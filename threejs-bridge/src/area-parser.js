import fs from "node:fs";
import path from "node:path";

const DIR_INDEX_TO_NAME = ["north", "east", "south", "west", "up", "down"];

const DIR_OFFSETS = {
  north: { x: 0, y: 0, z: -1 },
  east: { x: 1, y: 0, z: 0 },
  south: { x: 0, y: 0, z: 1 },
  west: { x: -1, y: 0, z: 0 },
  up: { x: 0, y: 1, z: 0 },
  down: { x: 0, y: -1, z: 0 },
};

function parseInteger(value) {
  const parsed = Number.parseInt(value, 10);
  return Number.isFinite(parsed) ? parsed : null;
}

function readTildeBlock(lines, startIndex) {
  const out = [];
  let index = startIndex;

  while (index < lines.length) {
    const line = lines[index];
    const marker = line.indexOf("~");
    if (marker !== -1) {
      out.push(line.slice(0, marker));
      index += 1;
      break;
    }
    out.push(line);
    index += 1;
  }

  return {
    text: out.join("\n").trim(),
    nextIndex: index,
  };
}

function parseAreaName(lines, fallbackName) {
  for (const rawLine of lines) {
    const line = rawLine.trim();
    if (!line.startsWith("Name ")) {
      continue;
    }
    const name = line.slice(5).trim();
    if (name.endsWith("~")) {
      return name.slice(0, -1).trim() || fallbackName;
    }
    return name || fallbackName;
  }
  return fallbackName;
}

function parseRoom(lines, startIndex, areaName, areaFile) {
  const vnum = parseInteger(lines[startIndex].trim().slice(1));
  if (vnum === null) {
    return { room: null, nextIndex: startIndex + 1 };
  }

  let index = startIndex + 1;
  const roomNameBlock = readTildeBlock(lines, index);
  const name = roomNameBlock.text;
  index = roomNameBlock.nextIndex;

  const roomDescriptionBlock = readTildeBlock(lines, index);
  const description = roomDescriptionBlock.text;
  index = roomDescriptionBlock.nextIndex;

  const roomDataLine = (lines[index] ?? "").trim();
  const roomDataTokens = roomDataLine
    .split(/\s+/)
    .map(parseInteger)
    .filter((value) => value !== null);
  const sector = roomDataTokens.length > 0 ? roomDataTokens[roomDataTokens.length - 1] : 0;
  index += 1;

  const exits = {};

  while (index < lines.length) {
    const token = (lines[index] ?? "").trim();

    if (token === "S") {
      index += 1;
      break;
    }

    if (/^D[0-5]$/.test(token)) {
      const door = parseInteger(token.slice(1));
      const direction = door === null ? null : DIR_INDEX_TO_NAME[door];

      index += 1;
      const exitDescription = readTildeBlock(lines, index);
      index = exitDescription.nextIndex;

      const exitKeyword = readTildeBlock(lines, index);
      index = exitKeyword.nextIndex;

      const destinationLine = (lines[index] ?? "").trim();
      const destinationTokens = destinationLine.split(/\s+/);
      const destinationVnum = parseInteger(destinationTokens[destinationTokens.length - 1]);
      if (direction && destinationVnum !== null && destinationVnum > 0) {
        exits[direction] = destinationVnum;
      }
      index += 1;
      continue;
    }

    if (token === "E") {
      index += 1;
      const extraKeyword = readTildeBlock(lines, index);
      index = extraKeyword.nextIndex;
      const extraDescription = readTildeBlock(lines, index);
      index = extraDescription.nextIndex;
      continue;
    }

    index += 1;
  }

  return {
    room: {
      vnum,
      name,
      description,
      area: areaName,
      areaFile,
      sector,
      exits,
      mobCount: 0,
      coord: null,
    },
    nextIndex: index,
  };
}

function parseMobCountsFromResets(lines) {
  const mobCountsByRoom = {};
  const resetsStart = lines.findIndex((line) => line.trim() === "#RESETS");

  if (resetsStart === -1) {
    return mobCountsByRoom;
  }

  let index = resetsStart + 1;
  while (index < lines.length) {
    const token = (lines[index] ?? "").trim();

    if (token === "S") {
      break;
    }

    if (!token || token.startsWith("*")) {
      index += 1;
      continue;
    }

    const tokens = token.split(/\s+/);
    if (tokens[0] === "M" && tokens.length >= 6) {
      const roomVnum = parseInteger(tokens[4]);
      const roomMax = parseInteger(tokens[5]);

      if (roomVnum !== null && roomVnum > 0 && roomMax !== null && roomMax > 0) {
        const key = String(roomVnum);
        mobCountsByRoom[key] = (mobCountsByRoom[key] ?? 0) + roomMax;
      }
    }

    index += 1;
  }

  return mobCountsByRoom;
}

function parseRoomsFromAreaFile(filePath) {
  const fileName = path.basename(filePath);
  const raw = fs.readFileSync(filePath, "utf8");
  const lines = raw.split(/\r?\n/);
  const areaName = parseAreaName(lines, fileName);
  const rooms = [];
  const mobCountsByRoom = parseMobCountsFromResets(lines);

  const roomsStart = lines.findIndex((line) => line.trim() === "#ROOMS");
  if (roomsStart === -1) {
    return { rooms, mobCountsByRoom };
  }

  let index = roomsStart + 1;
  while (index < lines.length) {
    const token = (lines[index] ?? "").trim();

    if (token === "#0") {
      break;
    }

    if (/^#\d+$/.test(token)) {
      const parsed = parseRoom(lines, index, areaName, fileName);
      if (parsed.room) {
        rooms.push(parsed.room);
      }
      index = parsed.nextIndex;
      continue;
    }

    index += 1;
  }

  return { rooms, mobCountsByRoom };
}

function assignCoordinates(roomsByVnum) {
  const visited = new Set();
  const rooms = Object.values(roomsByVnum);

  for (const startRoom of rooms) {
    if (visited.has(startRoom.vnum)) {
      continue;
    }

    if (!startRoom.coord) {
      startRoom.coord = { x: 0, y: 0, z: 0 };
    }

    const queue = [startRoom];
    visited.add(startRoom.vnum);

    while (queue.length > 0) {
      const room = queue.shift();

      for (const [direction, targetVnum] of Object.entries(room.exits)) {
        const targetRoom = roomsByVnum[String(targetVnum)];
        const offset = DIR_OFFSETS[direction];
        if (!targetRoom || !offset) {
          continue;
        }

        if (!targetRoom.coord) {
          targetRoom.coord = {
            x: room.coord.x + offset.x,
            y: room.coord.y + offset.y,
            z: room.coord.z + offset.z,
          };
        }

        if (!visited.has(targetRoom.vnum)) {
          visited.add(targetRoom.vnum);
          queue.push(targetRoom);
        }
      }
    }
  }
}

export function buildWorldFromAreaDirectory(areaDir) {
  const files = fs
    .readdirSync(areaDir, { withFileTypes: true })
    .filter((entry) => entry.isFile() && entry.name.toLowerCase().endsWith(".are"))
    .map((entry) => entry.name)
    .sort((a, b) => a.localeCompare(b));

  const roomsByVnum = {};
  const mobCountsByRoom = {};

  for (const file of files) {
    const fullPath = path.join(areaDir, file);
    const { rooms, mobCountsByRoom: mobCountsForFile } = parseRoomsFromAreaFile(fullPath);
    for (const room of rooms) {
      if (roomsByVnum[String(room.vnum)]) {
        continue;
      }
      roomsByVnum[String(room.vnum)] = room;
    }

    for (const [roomVnum, mobCount] of Object.entries(mobCountsForFile)) {
      mobCountsByRoom[roomVnum] = (mobCountsByRoom[roomVnum] ?? 0) + mobCount;
    }
  }

  for (const [roomVnum, mobCount] of Object.entries(mobCountsByRoom)) {
    const mappedRoom = roomsByVnum[roomVnum];
    if (!mappedRoom) {
      continue;
    }
    mappedRoom.mobCount = (mappedRoom.mobCount ?? 0) + mobCount;
  }

  assignCoordinates(roomsByVnum);

  return {
    generatedAt: new Date().toISOString(),
    areaDir,
    roomCount: Object.keys(roomsByVnum).length,
    rooms: roomsByVnum,
  };
}

export function serializeRoom(room) {
  return {
    vnum: room.vnum,
    name: room.name,
    description: room.description,
    area: room.area,
    areaFile: room.areaFile,
    sector: room.sector,
    mobCount: room.mobCount ?? 0,
    coord: room.coord,
    exits: room.exits,
  };
}
