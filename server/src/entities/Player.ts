import { Entity, type Stats, type CombatStats } from "./Entity.js";
import { Item, WearSlot } from "./Item.js";

export enum PlayerClass {
  Warrior = "warrior",
  Mage = "mage",
  Cleric = "cleric",
  Thief = "thief",
  Ranger = "ranger",
  Paladin = "paladin",
}

export enum PlayerRace {
  Human = "human",
  Elf = "elf",
  Dwarf = "dwarf",
  HalfElf = "half-elf",
  Kender = "kender",
  Minotaur = "minotaur",
  Gnome = "gnome",
}

export const CLASS_BASE_STATS: Record<PlayerClass, Stats> = {
  [PlayerClass.Warrior]:  { str: 16, dex: 13, con: 15, int: 10, wis: 10, cha: 12 },
  [PlayerClass.Mage]:     { str: 10, dex: 13, con: 11, int: 17, wis: 14, cha: 11 },
  [PlayerClass.Cleric]:   { str: 12, dex: 11, con: 13, int: 13, wis: 17, cha: 14 },
  [PlayerClass.Thief]:    { str: 12, dex: 17, con: 12, int: 14, wis: 10, cha: 11 },
  [PlayerClass.Ranger]:   { str: 14, dex: 15, con: 14, int: 12, wis: 13, cha: 10 },
  [PlayerClass.Paladin]:  { str: 15, dex: 11, con: 14, int: 11, wis: 15, cha: 16 },
};

export const RACE_STAT_BONUS: Record<PlayerRace, Partial<Stats>> = {
  [PlayerRace.Human]:    {},
  [PlayerRace.Elf]:      { dex: 2, int: 1, con: -1 },
  [PlayerRace.Dwarf]:    { str: 1, con: 2, cha: -1 },
  [PlayerRace.HalfElf]: { dex: 1, cha: 1 },
  [PlayerRace.Kender]:   { dex: 3, str: -1, wis: -1 },
  [PlayerRace.Minotaur]: { str: 3, con: 1, int: -2 },
  [PlayerRace.Gnome]:    { int: 2, dex: 1, str: -1 },
};

export class Player extends Entity {
  readonly accountId: string;
  playerClass: PlayerClass;
  race: PlayerRace;
  xp: number;
  xpToLevel: number;
  gold: number;
  bank: number;
  kills: number;
  deaths: number;
  clan: string | null;

  inventory: Item[] = [];
  equipment: Map<WearSlot, Item> = new Map();
  maxInventorySize = 30;

  /** Skill cooldowns: skillName -> ready timestamp */
  cooldowns: Map<string, number> = new Map();

  /** Last activity timestamp for AFK detection */
  lastActivity = Date.now();

  constructor(accountId: string, name: string, playerClass: PlayerClass, race: PlayerRace) {
    super("player", name, 1, 3001);
    this.accountId = accountId;
    this.playerClass = playerClass;
    this.race = race;
    this.xp = 0;
    this.xpToLevel = 1000;
    this.gold = 100;
    this.bank = 0;
    this.kills = 0;
    this.deaths = 0;
    this.clan = null;

    // Apply class base stats
    const baseStats = CLASS_BASE_STATS[playerClass];
    this.stats = { ...baseStats };

    // Apply race bonuses
    const raceBonus = RACE_STAT_BONUS[race];
    for (const [key, value] of Object.entries(raceBonus)) {
      (this.stats as unknown as Record<string, number>)[key] += value as number;
    }

    this.recalculateCombat();
  }

  recalculateCombat(): void {
    const level = this.level;
    const { str, dex, con, int, wis } = this.stats;

    this.combat.maxHp = 20 + (con * 2) + (level * (6 + Math.floor(con / 4)));
    this.combat.maxMana = 50 + (int * 3) + (wis * 2) + (level * (3 + Math.floor(int / 5)));
    this.combat.maxMove = 80 + (dex * 2) + (level * 3);

    this.combat.hitroll = Math.floor(str / 3) + Math.floor(dex / 5) + level;
    this.combat.damroll = Math.floor(str / 2) + level;

    // Base armor class (lower is better)
    let ac = 100 - (dex * 2) - (level * 2);
    for (const [, item] of this.equipment) {
      ac -= item.armorValue;
    }
    this.combat.armor = ac;

    // Equipment bonuses
    for (const [, item] of this.equipment) {
      for (const effect of item.effects) {
        switch (effect.stat) {
          case "hitroll": this.combat.hitroll += effect.modifier; break;
          case "damroll": this.combat.damroll += effect.modifier; break;
          case "hp": this.combat.maxHp += effect.modifier; break;
          case "mana": this.combat.maxMana += effect.modifier; break;
        }
      }
    }

    // Clamp HP/mana/move to max
    this.combat.hp = Math.min(this.combat.hp, this.combat.maxHp);
    this.combat.mana = Math.min(this.combat.mana, this.combat.maxMana);
    this.combat.move = Math.min(this.combat.move, this.combat.maxMove);
  }

  gainXp(amount: number): { leveled: boolean; newLevel: number } {
    this.xp += amount;
    let leveled = false;

    while (this.xp >= this.xpToLevel && this.level < 110) {
      this.xp -= this.xpToLevel;
      this.level++;
      this.xpToLevel = Math.floor(this.xpToLevel * 1.15);
      leveled = true;

      // Level up: full heal and stat boost
      this.recalculateCombat();
      this.combat.hp = this.combat.maxHp;
      this.combat.mana = this.combat.maxMana;
      this.combat.move = this.combat.maxMove;
    }

    return { leveled, newLevel: this.level };
  }

  canEquip(item: Item): boolean {
    if (item.wearSlot === WearSlot.None) return false;
    if (item.level > this.level) return false;
    return true;
  }

  equip(item: Item): Item | null {
    if (!this.canEquip(item)) return null;

    const existing = this.equipment.get(item.wearSlot);
    this.equipment.set(item.wearSlot, item);

    // Remove from inventory if present
    const idx = this.inventory.indexOf(item);
    if (idx >= 0) this.inventory.splice(idx, 1);

    this.recalculateCombat();
    return existing ?? null;  // return unequipped item
  }

  unequip(slot: WearSlot): Item | null {
    const item = this.equipment.get(slot);
    if (!item) return null;
    if (this.inventory.length >= this.maxInventorySize) return null;

    this.equipment.delete(slot);
    this.inventory.push(item);
    this.recalculateCombat();
    return item;
  }

  addItem(item: Item): boolean {
    if (this.inventory.length >= this.maxInventorySize) return false;
    this.inventory.push(item);
    return true;
  }

  removeItem(instanceId: string): Item | null {
    const idx = this.inventory.findIndex((i) => i.instanceId === instanceId);
    if (idx < 0) return null;
    return this.inventory.splice(idx, 1)[0];
  }

  getWeaponDamage(): { min: number; max: number } {
    const weapon = this.equipment.get(WearSlot.Wield);
    if (weapon) {
      return { min: weapon.minDamage + this.combat.damroll, max: weapon.maxDamage + this.combat.damroll };
    }
    // Unarmed
    return { min: 1 + Math.floor(this.combat.damroll / 2), max: 3 + this.combat.damroll };
  }

  die(): void {
    this.alive = false;
    this.target = null;
    this.deaths++;

    // Lose 10% gold on death
    const goldLost = Math.floor(this.gold * 0.1);
    this.gold -= goldLost;

    // Lose some XP
    const xpLost = Math.floor(this.xpToLevel * 0.05);
    this.xp = Math.max(0, this.xp - xpLost);
  }

  respawn(roomVnum: number): void {
    this.alive = true;
    this.combat.hp = Math.floor(this.combat.maxHp * 0.5);
    this.combat.mana = Math.floor(this.combat.maxMana * 0.5);
    this.combat.move = this.combat.maxMove;
    this.roomVnum = roomVnum;
    this.target = null;
    this.attackCooldown = 0;
  }

  serialize() {
    return {
      id: this.id,
      type: this.type,
      accountId: this.accountId,
      name: this.name,
      level: this.level,
      playerClass: this.playerClass,
      race: this.race,
      roomVnum: this.roomVnum,
      stats: { ...this.stats },
      combat: { ...this.combat },
      alignment: this.alignment,
      xp: this.xp,
      xpToLevel: this.xpToLevel,
      gold: this.gold,
      bank: this.bank,
      kills: this.kills,
      deaths: this.deaths,
      clan: this.clan,
      inventory: this.inventory.map((i) => i.serialize()),
      equipment: Object.fromEntries(
        Array.from(this.equipment.entries()).map(([slot, item]) => [slot, item.serialize()])
      ),
      alive: this.alive,
      hpPercent: this.hpPercent,
      manaPercent: this.manaPercent,
      movePercent: this.movePercent,
    };
  }

  /** Minimal state sent to other players */
  serializePublic() {
    return {
      id: this.id,
      type: this.type,
      name: this.name,
      level: this.level,
      playerClass: this.playerClass,
      race: this.race,
      roomVnum: this.roomVnum,
      hpPercent: this.hpPercent,
      clan: this.clan,
      alive: this.alive,
      isInCombat: this.isInCombat,
    };
  }
}
