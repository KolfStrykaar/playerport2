import { v4 as uuidv4 } from "uuid";

export enum ItemType {
  Trash = 0,
  Light = 1,
  Scroll = 2,
  Wand = 3,
  Staff = 4,
  Weapon = 5,
  Treasure = 8,
  Armor = 9,
  Potion = 10,
  Clothing = 11,
  Furniture = 12,
  Container = 15,
  DrinkContainer = 17,
  Key = 18,
  Food = 19,
  Money = 20,
  Boat = 22,
  Pill = 26,
  Map = 28,
}

export enum WearSlot {
  None = "none",
  Light = "light",
  Head = "head",
  Neck = "neck",
  Torso = "torso",
  Legs = "legs",
  Feet = "feet",
  Hands = "hands",
  Arms = "arms",
  Shield = "shield",
  Body = "body",
  Waist = "waist",
  Wrist = "wrist",
  Wield = "wield",
  Hold = "hold",
  Float = "float",
}

export interface ItemEffect {
  stat: string;
  modifier: number;
}

export class Item {
  readonly instanceId: string;
  readonly templateVnum: number;
  name: string;
  shortDesc: string;
  longDesc: string;
  itemType: ItemType;
  level: number;
  weight: number;
  cost: number;
  wearSlot: WearSlot;
  effects: ItemEffect[];
  values: number[];

  /** For weapons: min/max damage */
  minDamage: number;
  maxDamage: number;

  /** For armor: AC bonus */
  armorValue: number;

  constructor(templateVnum: number, name: string, shortDesc: string, longDesc: string, itemType: ItemType, level: number) {
    this.instanceId = uuidv4();
    this.templateVnum = templateVnum;
    this.name = name;
    this.shortDesc = shortDesc;
    this.longDesc = longDesc;
    this.itemType = itemType;
    this.level = level;
    this.weight = 1;
    this.cost = 0;
    this.wearSlot = WearSlot.None;
    this.effects = [];
    this.values = [];
    this.minDamage = 1;
    this.maxDamage = 4;
    this.armorValue = 0;
  }

  serialize() {
    return {
      instanceId: this.instanceId,
      templateVnum: this.templateVnum,
      name: this.name,
      shortDesc: this.shortDesc,
      longDesc: this.longDesc,
      itemType: this.itemType,
      level: this.level,
      weight: this.weight,
      cost: this.cost,
      wearSlot: this.wearSlot,
      effects: this.effects,
      minDamage: this.minDamage,
      maxDamage: this.maxDamage,
      armorValue: this.armorValue,
    };
  }

  static fromTemplate(template: { vnum: number; name: string; shortDesc: string; longDesc: string; itemType: number; level: number; weight: number; cost: number; values: number[]; wearFlags: number }): Item {
    const item = new Item(template.vnum, template.name, template.shortDesc, template.longDesc, template.itemType as ItemType, template.level);
    item.weight = template.weight;
    item.cost = template.cost;
    item.values = [...template.values];

    // Determine wear slot from flags
    if (template.wearFlags & 2) item.wearSlot = WearSlot.Head;
    else if (template.wearFlags & 4) item.wearSlot = WearSlot.Neck;
    else if (template.wearFlags & 8) item.wearSlot = WearSlot.Torso;
    else if (template.wearFlags & 16) item.wearSlot = WearSlot.Legs;
    else if (template.wearFlags & 32) item.wearSlot = WearSlot.Feet;
    else if (template.wearFlags & 64) item.wearSlot = WearSlot.Hands;
    else if (template.wearFlags & 128) item.wearSlot = WearSlot.Arms;
    else if (template.wearFlags & 256) item.wearSlot = WearSlot.Shield;
    else if (template.wearFlags & 8192) item.wearSlot = WearSlot.Wield;
    else if (template.wearFlags & 16384) item.wearSlot = WearSlot.Hold;

    // Parse weapon damage from values
    if (template.itemType === ItemType.Weapon && template.values.length >= 3) {
      const diceCount = template.values[1] ?? 1;
      const diceSides = template.values[2] ?? 4;
      item.minDamage = diceCount;
      item.maxDamage = diceCount * diceSides;
    }

    // Parse armor value
    if (template.itemType === ItemType.Armor && template.values.length >= 1) {
      item.armorValue = template.values[0] ?? 0;
    }

    return item;
  }
}
