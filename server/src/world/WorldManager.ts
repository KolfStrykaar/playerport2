import path from "node:path";
import { Config } from "../config.js";
import { loadAllAreas } from "./AreaLoader.js";
import { Room, DIR_VECTORS, type Direction, type RoomCoord } from "./Room.js";
import { Zone, type MobTemplate, type ObjTemplate } from "./Zone.js";

export class WorldManager {
  rooms: Map<number, Room> = new Map();
  zones: Map<string, Zone> = new Map();
  mobTemplates: Map<number, MobTemplate> = new Map();
  objTemplates: Map<number, ObjTemplate> = new Map();

  /** Room VNum where new players start */
  startRoom = 3001;

  load(): void {
    const areaDir = path.resolve(Config.world.areaDir);
    console.log(`[World] Loading areas from ${areaDir}...`);

    const zones = loadAllAreas(areaDir);

    for (const zone of zones) {
      this.zones.set(zone.fileName, zone);

      for (const [vnum, room] of zone.rooms) {
        if (!this.rooms.has(vnum)) {
          this.rooms.set(vnum, room);
        }
      }

      for (const [vnum, mob] of zone.mobTemplates) {
        if (!this.mobTemplates.has(vnum)) {
          this.mobTemplates.set(vnum, mob);
        }
      }

      for (const [vnum, obj] of zone.objTemplates) {
        if (!this.objTemplates.has(vnum)) {
          this.objTemplates.set(vnum, obj);
        }
      }
    }

    this.assignCoordinates();

    // Find a valid start room
    if (!this.rooms.has(this.startRoom)) {
      const firstVnum = this.rooms.keys().next().value;
      if (firstVnum !== undefined) {
        this.startRoom = firstVnum;
      }
    }

    console.log(`[World] Loaded ${this.rooms.size} rooms, ${this.mobTemplates.size} mob templates, ${this.objTemplates.size} object templates across ${this.zones.size} zones`);
    console.log(`[World] Start room: ${this.startRoom}`);
  }

  getRoom(vnum: number): Room | undefined {
    return this.rooms.get(vnum);
  }

  getMobTemplate(vnum: number): MobTemplate | undefined {
    return this.mobTemplates.get(vnum);
  }

  getObjTemplate(vnum: number): ObjTemplate | undefined {
    return this.objTemplates.get(vnum);
  }

  /** Get rooms within BFS radius of a center room (for map rendering) */
  getRoomMap(centerVnum: number, radius = 5): { rooms: Room[]; links: Array<{ from: number; to: number; direction: string }> } {
    const center = this.rooms.get(centerVnum);
    if (!center) return { rooms: [], links: [] };

    const visited = new Map<number, Room>();
    const queue: Array<{ room: Room; depth: number }> = [{ room: center, depth: 0 }];
    visited.set(center.vnum, center);

    while (queue.length > 0) {
      const entry = queue.shift()!;
      if (entry.depth >= radius) continue;

      for (const [, exit] of entry.room.exits) {
        const target = this.rooms.get(exit.targetVnum);
        if (!target || visited.has(target.vnum)) continue;
        visited.set(target.vnum, target);
        queue.push({ room: target, depth: entry.depth + 1 });
      }
    }

    const rooms = Array.from(visited.values());
    const links: Array<{ from: number; to: number; direction: string }> = [];
    const seenLinks = new Set<string>();

    for (const room of rooms) {
      for (const [dir, exit] of room.exits) {
        if (!visited.has(exit.targetVnum)) continue;
        const low = Math.min(room.vnum, exit.targetVnum);
        const high = Math.max(room.vnum, exit.targetVnum);
        const key = `${low}:${high}`;
        if (seenLinks.has(key)) continue;
        seenLinks.add(key);
        links.push({ from: room.vnum, to: exit.targetVnum, direction: dir });
      }
    }

    return { rooms, links };
  }

  /** Get all mob spawns across all zones */
  getAllMobSpawns(): Array<{ zone: Zone; mobVnum: number; roomVnum: number; maxCount: number }> {
    const spawns: Array<{ zone: Zone; mobVnum: number; roomVnum: number; maxCount: number }> = [];
    for (const zone of this.zones.values()) {
      for (const spawn of zone.mobSpawns) {
        spawns.push({ zone, mobVnum: spawn.mobVnum, roomVnum: spawn.roomVnum, maxCount: spawn.maxCount });
      }
    }
    return spawns;
  }

  private assignCoordinates(): void {
    const visited = new Set<number>();

    for (const room of this.rooms.values()) {
      if (visited.has(room.vnum)) continue;

      if (!room.coord) {
        room.coord = { x: 0, y: 0, z: 0 };
      }

      const queue: Room[] = [room];
      visited.add(room.vnum);

      while (queue.length > 0) {
        const current = queue.shift()!;

        for (const [dir, exit] of current.exits) {
          const target = this.rooms.get(exit.targetVnum);
          const offset = DIR_VECTORS[dir];
          if (!target || !offset) continue;

          if (!target.coord) {
            target.coord = {
              x: current.coord!.x + offset.x,
              y: current.coord!.y + offset.y,
              z: current.coord!.z + offset.z,
            };
          }

          if (!visited.has(target.vnum)) {
            visited.add(target.vnum);
            queue.push(target);
          }
        }
      }
    }
  }
}
