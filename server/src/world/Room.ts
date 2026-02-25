import { getSector, type SectorDefinition } from "../data/SectorTypes.js";

export type Direction = "north" | "east" | "south" | "west" | "up" | "down";

export const ALL_DIRECTIONS: Direction[] = ["north", "east", "south", "west", "up", "down"];

export const OPPOSITE_DIR: Record<Direction, Direction> = {
  north: "south",
  south: "north",
  east: "west",
  west: "east",
  up: "down",
  down: "up",
};

export const DIR_VECTORS: Record<Direction, { x: number; y: number; z: number }> = {
  north: { x: 0, y: 0, z: -1 },
  east:  { x: 1, y: 0, z: 0 },
  south: { x: 0, y: 0, z: 1 },
  west:  { x: -1, y: 0, z: 0 },
  up:    { x: 0, y: 1, z: 0 },
  down:  { x: 0, y: -1, z: 0 },
};

export interface RoomExit {
  direction: Direction;
  targetVnum: number;
  doorFlags: number;
  keyVnum: number;
  locked: boolean;
  closed: boolean;
}

export interface RoomCoord {
  x: number;
  y: number;
  z: number;
}

export class Room {
  readonly vnum: number;
  name: string;
  description: string;
  areaName: string;
  areaFile: string;
  sectorType: number;
  exits: Map<Direction, RoomExit> = new Map();
  coord: RoomCoord | null = null;

  /** Set of entity IDs currently in this room */
  occupants: Set<string> = new Set();
  /** Set of item instance IDs on the ground */
  items: Set<string> = new Set();

  /** Room flags (dark, no_mob, safe, etc.) */
  flags: Set<string> = new Set();

  constructor(vnum: number, name: string, description: string, areaName: string, areaFile: string, sectorType: number) {
    this.vnum = vnum;
    this.name = name;
    this.description = description;
    this.areaName = areaName;
    this.areaFile = areaFile;
    this.sectorType = sectorType;
  }

  get sector(): SectorDefinition {
    return getSector(this.sectorType);
  }

  get moveCost(): number {
    return this.sector.moveCost;
  }

  hasExit(dir: Direction): boolean {
    return this.exits.has(dir);
  }

  getExit(dir: Direction): RoomExit | undefined {
    return this.exits.get(dir);
  }

  addExit(dir: Direction, targetVnum: number, doorFlags = 0, keyVnum = 0): void {
    this.exits.set(dir, {
      direction: dir,
      targetVnum,
      doorFlags,
      keyVnum,
      locked: (doorFlags & 2) !== 0,
      closed: (doorFlags & 1) !== 0,
    });
  }

  addOccupant(entityId: string): void {
    this.occupants.add(entityId);
  }

  removeOccupant(entityId: string): void {
    this.occupants.delete(entityId);
  }

  serialize() {
    const exits: Record<string, number> = {};
    for (const [dir, exit] of this.exits) {
      exits[dir] = exit.targetVnum;
    }
    return {
      vnum: this.vnum,
      name: this.name,
      description: this.description,
      area: this.areaName,
      sector: this.sectorType,
      sectorName: this.sector.name,
      sectorColor: this.sector.color,
      coord: this.coord,
      exits,
      occupantCount: this.occupants.size,
      itemCount: this.items.size,
    };
  }
}
