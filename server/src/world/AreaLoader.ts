import fs from "node:fs";
import path from "node:path";
import { Room, type Direction } from "./Room.js";
import { Zone, type MobTemplate, type ObjTemplate } from "./Zone.js";

const DIR_INDEX_TO_NAME: Direction[] = ["north", "east", "south", "west", "up", "down"];

function parseInt10(value: string): number | null {
  const n = Number.parseInt(value, 10);
  return Number.isFinite(n) ? n : null;
}

function readTildeBlock(lines: string[], start: number): { text: string; nextIndex: number } {
  const out: string[] = [];
  let i = start;
  while (i < lines.length) {
    const line = lines[i];
    const marker = line.indexOf("~");
    if (marker !== -1) {
      out.push(line.slice(0, marker));
      i++;
      break;
    }
    out.push(line);
    i++;
  }
  return { text: out.join("\n").trim(), nextIndex: i };
}

function parseAreaName(lines: string[], fallback: string): string {
  for (const raw of lines) {
    const line = raw.trim();
    if (!line.startsWith("Name ")) continue;
    const name = line.slice(5).trim();
    if (name.endsWith("~")) return name.slice(0, -1).trim() || fallback;
    return name || fallback;
  }
  return fallback;
}

function parseRooms(lines: string[], startIndex: number, areaName: string, areaFile: string): { rooms: Room[]; endIndex: number } {
  const rooms: Room[] = [];
  let i = startIndex;

  while (i < lines.length) {
    const token = (lines[i] ?? "").trim();
    if (token === "#0") { i++; break; }

    if (/^#\d+$/.test(token)) {
      const vnum = parseInt10(token.slice(1));
      if (vnum === null) { i++; continue; }

      i++;
      const nameBlock = readTildeBlock(lines, i);
      i = nameBlock.nextIndex;

      const descBlock = readTildeBlock(lines, i);
      i = descBlock.nextIndex;

      const dataLine = (lines[i] ?? "").trim();
      const dataTokens = dataLine.split(/\s+/).map(parseInt10).filter((v): v is number => v !== null);
      const sector = dataTokens.length > 0 ? dataTokens[dataTokens.length - 1] : 0;
      const roomFlags = dataTokens.length > 1 ? dataTokens[1] : 0;
      i++;

      const room = new Room(vnum, nameBlock.text, descBlock.text, areaName, areaFile, sector);

      // Parse room flags
      if (roomFlags & 1) room.flags.add("dark");
      if (roomFlags & 4) room.flags.add("no_mob");
      if (roomFlags & 8) room.flags.add("indoors");
      if (roomFlags & 512) room.flags.add("safe");
      if (roomFlags & 1024) room.flags.add("no_recall");
      if (roomFlags & 2048) room.flags.add("no_magic");

      // Parse exits and extra descriptions
      while (i < lines.length) {
        const t = (lines[i] ?? "").trim();
        if (t === "S") { i++; break; }

        if (/^D[0-5]$/.test(t)) {
          const dirIdx = parseInt10(t.slice(1));
          const direction = dirIdx !== null ? DIR_INDEX_TO_NAME[dirIdx] : null;
          i++;

          const exitDesc = readTildeBlock(lines, i);
          i = exitDesc.nextIndex;

          const exitKeyword = readTildeBlock(lines, i);
          i = exitKeyword.nextIndex;

          const destLine = (lines[i] ?? "").trim();
          const destTokens = destLine.split(/\s+/);
          const doorFlags = parseInt10(destTokens[0] ?? "0") ?? 0;
          const keyVnum = parseInt10(destTokens[1] ?? "0") ?? 0;
          const destVnum = parseInt10(destTokens[destTokens.length - 1] ?? "0");

          if (direction && destVnum !== null && destVnum > 0) {
            room.addExit(direction, destVnum, doorFlags, keyVnum);
          }
          i++;
          continue;
        }

        if (t === "E") {
          i++;
          const ek = readTildeBlock(lines, i);
          i = ek.nextIndex;
          const ed = readTildeBlock(lines, i);
          i = ed.nextIndex;
          continue;
        }

        i++;
      }

      rooms.push(room);
      continue;
    }

    i++;
  }

  return { rooms, endIndex: i };
}

function parseMobiles(lines: string[], startIndex: number): { mobs: MobTemplate[]; endIndex: number } {
  const mobs: MobTemplate[] = [];
  let i = startIndex;

  while (i < lines.length) {
    const token = (lines[i] ?? "").trim();
    if (token === "#0") { i++; break; }

    if (/^#\d+$/.test(token)) {
      const vnum = parseInt10(token.slice(1));
      if (vnum === null) { i++; continue; }

      i++;
      const nameBlock = readTildeBlock(lines, i);
      i = nameBlock.nextIndex;

      const shortBlock = readTildeBlock(lines, i);
      i = shortBlock.nextIndex;

      const longBlock = readTildeBlock(lines, i);
      i = longBlock.nextIndex;

      const descBlock = readTildeBlock(lines, i);
      i = descBlock.nextIndex;

      // ACT line
      const actLine = (lines[i] ?? "").trim();
      const actTokens = actLine.split(/\s+/);
      i++;

      // Alignment line
      const alignLine = (lines[i] ?? "").trim();
      const alignTokens = alignLine.split(/\s+/);
      const alignment = parseInt10(alignTokens[2] ?? "0") ?? 0;
      i++;

      // Level line
      const levelLine = (lines[i] ?? "").trim();
      const levelTokens = levelLine.split(/\s+/);
      const level = parseInt10(levelTokens[0] ?? "1") ?? 1;

      // Parse hit dice (e.g., "3d8+10")
      const hitStr = levelTokens[1] ?? "1d8+0";
      const hitMatch = hitStr.match(/(\d+)d(\d+)\+?(\d+)?/);
      const hitDice = {
        count: parseInt10(hitMatch?.[1] ?? "1") ?? 1,
        sides: parseInt10(hitMatch?.[2] ?? "8") ?? 8,
        bonus: parseInt10(hitMatch?.[3] ?? "0") ?? 0,
      };

      // Parse damage dice
      const dmgStr = levelTokens[3] ?? "1d4+0";
      const dmgMatch = dmgStr.match(/(\d+)d(\d+)\+?(\d+)?/);
      const damageDice = {
        count: parseInt10(dmgMatch?.[1] ?? "1") ?? 1,
        sides: parseInt10(dmgMatch?.[2] ?? "4") ?? 4,
        bonus: parseInt10(dmgMatch?.[3] ?? "0") ?? 0,
      };

      const armor = parseInt10(levelTokens[2] ?? "10") ?? 10;
      i++;

      // Gold / XP line
      const goldLine = (lines[i] ?? "").trim();
      const goldTokens = goldLine.split(/\s+/);
      const gold = parseInt10(goldTokens[0] ?? "0") ?? 0;
      const xp = parseInt10(goldTokens[1] ?? "0") ?? 0;
      i++;

      // Skip remaining mob data until next #vnum
      while (i < lines.length) {
        const peek = (lines[i] ?? "").trim();
        if (/^#\d+$/.test(peek) || peek === "#0") break;
        i++;
      }

      const flags = new Set<string>();
      const actFlag = parseInt10(actTokens[0] ?? "0") ?? 0;
      if (actFlag & 2) flags.add("sentinel");
      if (actFlag & 4) flags.add("scavenger");
      if (actFlag & 32) flags.add("aggressive");
      if (actFlag & 128) flags.add("wimpy");
      if (actFlag & 256) flags.add("pet");
      if (actFlag & 512) flags.add("train");
      if (actFlag & 1024) flags.add("practice");
      if (actFlag & 32768) flags.add("healer");
      if (actFlag & 16384) flags.add("shopkeeper");

      mobs.push({
        vnum,
        name: nameBlock.text,
        shortDesc: shortBlock.text,
        longDesc: longBlock.text,
        description: descBlock.text,
        level,
        alignment,
        hitDice,
        damageDice,
        armor,
        gold,
        xp,
        flags,
      });
      continue;
    }

    i++;
  }

  return { mobs, endIndex: i };
}

function parseObjects(lines: string[], startIndex: number): { objects: ObjTemplate[]; endIndex: number } {
  const objects: ObjTemplate[] = [];
  let i = startIndex;

  while (i < lines.length) {
    const token = (lines[i] ?? "").trim();
    if (token === "#0") { i++; break; }

    if (/^#\d+$/.test(token)) {
      const vnum = parseInt10(token.slice(1));
      if (vnum === null) { i++; continue; }

      i++;
      const nameBlock = readTildeBlock(lines, i);
      i = nameBlock.nextIndex;

      const shortBlock = readTildeBlock(lines, i);
      i = shortBlock.nextIndex;

      const longBlock = readTildeBlock(lines, i);
      i = longBlock.nextIndex;

      // Material line
      const _matBlock = readTildeBlock(lines, i);
      i = _matBlock.nextIndex;

      // Type/flags line
      const typeLine = (lines[i] ?? "").trim();
      const typeTokens = typeLine.split(/\s+/);
      const itemType = parseInt10(typeTokens[0] ?? "0") ?? 0;
      const extraFlags = parseInt10(typeTokens[1] ?? "0") ?? 0;
      const wearFlags = parseInt10(typeTokens[2] ?? "0") ?? 0;
      i++;

      // Values line
      const valLine = (lines[i] ?? "").trim();
      const values = valLine.split(/\s+/).map((v) => parseInt10(v) ?? 0);
      i++;

      // Level/weight/cost line
      const lwcLine = (lines[i] ?? "").trim();
      const lwcTokens = lwcLine.split(/\s+/);
      const level = parseInt10(lwcTokens[0] ?? "1") ?? 1;
      const weight = parseInt10(lwcTokens[1] ?? "1") ?? 1;
      const cost = parseInt10(lwcTokens[2] ?? "0") ?? 0;
      i++;

      // Skip remaining obj data until next #vnum
      while (i < lines.length) {
        const peek = (lines[i] ?? "").trim();
        if (/^#\d+$/.test(peek) || peek === "#0") break;
        i++;
      }

      objects.push({ vnum, name: nameBlock.text, shortDesc: shortBlock.text, longDesc: longBlock.text, itemType, level, weight, cost, values, wearFlags, extraFlags });
      continue;
    }

    i++;
  }

  return { objects, endIndex: i };
}

interface ResetEntry {
  type: "mob" | "obj" | "give" | "equip" | "door";
  mobVnum?: number;
  objVnum?: number;
  roomVnum?: number;
  maxCount?: number;
}

function parseResets(lines: string[], startIndex: number): { resets: ResetEntry[]; endIndex: number } {
  const resets: ResetEntry[] = [];
  let i = startIndex;
  let lastMobRoom = 0;

  while (i < lines.length) {
    const line = (lines[i] ?? "").trim();
    if (line === "S" || line.startsWith("#")) { i++; break; }
    if (!line || line.startsWith("*")) { i++; continue; }

    const tokens = line.split(/\s+/);
    const cmd = tokens[0];

    switch (cmd) {
      case "M": {
        if (tokens.length >= 6) {
          const mobVnum = parseInt10(tokens[2] ?? "0") ?? 0;
          const roomVnum = parseInt10(tokens[4] ?? "0") ?? 0;
          const maxCount = parseInt10(tokens[3] ?? "1") ?? 1;
          lastMobRoom = roomVnum;
          resets.push({ type: "mob", mobVnum, roomVnum, maxCount });
        }
        break;
      }
      case "O": {
        if (tokens.length >= 5) {
          const objVnum = parseInt10(tokens[2] ?? "0") ?? 0;
          const roomVnum = parseInt10(tokens[4] ?? "0") ?? 0;
          resets.push({ type: "obj", objVnum, roomVnum, maxCount: 1 });
        }
        break;
      }
      case "G": {
        if (tokens.length >= 4) {
          const objVnum = parseInt10(tokens[2] ?? "0") ?? 0;
          resets.push({ type: "give", objVnum, roomVnum: lastMobRoom });
        }
        break;
      }
      case "E": {
        if (tokens.length >= 5) {
          const objVnum = parseInt10(tokens[2] ?? "0") ?? 0;
          resets.push({ type: "equip", objVnum, roomVnum: lastMobRoom });
        }
        break;
      }
    }

    i++;
  }

  return { resets, endIndex: i };
}

export function loadAreaFile(filePath: string): Zone {
  const fileName = path.basename(filePath);
  const raw = fs.readFileSync(filePath, "utf8");
  const lines = raw.split(/\r?\n/);
  const areaName = parseAreaName(lines, fileName);
  const zone = new Zone(areaName, fileName);

  let i = 0;
  while (i < lines.length) {
    const line = (lines[i] ?? "").trim();

    if (line === "#ROOMS") {
      i++;
      const { rooms, endIndex } = parseRooms(lines, i, areaName, fileName);
      for (const room of rooms) zone.addRoom(room);
      i = endIndex;
      continue;
    }

    if (line === "#MOBILES" || line === "#MOBILE") {
      i++;
      const { mobs, endIndex } = parseMobiles(lines, i);
      for (const mob of mobs) zone.addMobTemplate(mob);
      i = endIndex;
      continue;
    }

    if (line === "#OBJECTS" || line === "#OBJECT") {
      i++;
      const { objects, endIndex } = parseObjects(lines, i);
      for (const obj of objects) zone.addObjTemplate(obj);
      i = endIndex;
      continue;
    }

    if (line === "#RESETS") {
      i++;
      const { resets, endIndex } = parseResets(lines, i);
      for (const reset of resets) {
        if (reset.type === "mob" && reset.mobVnum && reset.roomVnum) {
          zone.addMobSpawn(reset.mobVnum, reset.roomVnum, reset.maxCount ?? 1);
        }
        if (reset.type === "obj" && reset.objVnum && reset.roomVnum) {
          zone.addObjSpawn(reset.objVnum, reset.roomVnum, reset.maxCount ?? 1);
        }
      }
      i = endIndex;
      continue;
    }

    i++;
  }

  return zone;
}

export function loadAllAreas(areaDir: string): Zone[] {
  const absoluteDir = path.resolve(areaDir);
  const files = fs.readdirSync(absoluteDir, { withFileTypes: true })
    .filter((e) => e.isFile() && e.name.toLowerCase().endsWith(".are"))
    .map((e) => e.name)
    .sort();

  const zones: Zone[] = [];
  for (const file of files) {
    try {
      zones.push(loadAreaFile(path.join(absoluteDir, file)));
    } catch (err) {
      console.error(`[AreaLoader] Failed to parse ${file}:`, err);
    }
  }

  return zones;
}
