import type { Room } from "./Room.js";

export interface MobSpawn {
  mobVnum: number;
  roomVnum: number;
  maxCount: number;
  currentCount: number;
  respawnTimer: number;
}

export interface ObjSpawn {
  objVnum: number;
  roomVnum: number;
  maxCount: number;
  currentCount: number;
}

export interface MobTemplate {
  vnum: number;
  name: string;
  shortDesc: string;
  longDesc: string;
  description: string;
  level: number;
  alignment: number;
  hitDice: { count: number; sides: number; bonus: number };
  damageDice: { count: number; sides: number; bonus: number };
  armor: number;
  gold: number;
  xp: number;
  flags: Set<string>;
}

export interface ObjTemplate {
  vnum: number;
  name: string;
  shortDesc: string;
  longDesc: string;
  itemType: number;
  level: number;
  weight: number;
  cost: number;
  values: number[];
  wearFlags: number;
  extraFlags: number;
}

export class Zone {
  readonly name: string;
  readonly fileName: string;
  rooms: Map<number, Room> = new Map();
  mobTemplates: Map<number, MobTemplate> = new Map();
  objTemplates: Map<number, ObjTemplate> = new Map();
  mobSpawns: MobSpawn[] = [];
  objSpawns: ObjSpawn[] = [];

  minLevel = 0;
  maxLevel = 110;
  builders = "";
  security = 1;

  constructor(name: string, fileName: string) {
    this.name = name;
    this.fileName = fileName;
  }

  addRoom(room: Room): void {
    this.rooms.set(room.vnum, room);
  }

  addMobTemplate(template: MobTemplate): void {
    this.mobTemplates.set(template.vnum, template);
  }

  addObjTemplate(template: ObjTemplate): void {
    this.objTemplates.set(template.vnum, template);
  }

  addMobSpawn(mobVnum: number, roomVnum: number, maxCount: number): void {
    this.mobSpawns.push({ mobVnum, roomVnum, maxCount, currentCount: 0, respawnTimer: 0 });
  }

  addObjSpawn(objVnum: number, roomVnum: number, maxCount: number): void {
    this.objSpawns.push({ objVnum, roomVnum, maxCount, currentCount: 0 });
  }
}
