import { Entity } from "./Entity.js";
import type { MobTemplate } from "../world/Zone.js";
import { Item, type ItemType } from "./Item.js";

export enum NPCBehavior {
  Passive = "passive",
  Aggressive = "aggressive",
  Sentinel = "sentinel",
  Wandering = "wandering",
  Shopkeeper = "shopkeeper",
  Healer = "healer",
  Trainer = "trainer",
}

export class NPC extends Entity {
  readonly templateVnum: number;
  shortDesc: string;
  longDesc: string;
  description: string;
  behaviors: Set<NPCBehavior>;
  gold: number;
  xpReward: number;
  spawnRoomVnum: number;
  respawnTime: number;      // ms until respawn after death
  respawnTimer: number;     // current countdown

  /** Loot table: templateVnum -> drop chance (0-1) */
  lootTable: Map<number, number> = new Map();

  /** Wander radius from spawn room */
  wanderRadius = 3;
  /** Tick counter for wander AI */
  wanderTimer = 0;

  /** Aggro range (rooms) */
  aggroRange = 0;

  /** Items this NPC carries */
  inventory: Item[] = [];

  constructor(template: MobTemplate, spawnRoomVnum: number) {
    super("npc", template.name, template.level, spawnRoomVnum);
    this.templateVnum = template.vnum;
    this.shortDesc = template.shortDesc;
    this.longDesc = template.longDesc;
    this.description = template.description;
    this.alignment = template.alignment;
    this.spawnRoomVnum = spawnRoomVnum;
    this.gold = template.gold;
    this.xpReward = template.xp || this.calculateXpReward(template.level);
    this.respawnTime = 60_000 + template.level * 1000;
    this.respawnTimer = 0;

    // Calculate HP from hit dice
    const { count, sides, bonus } = template.hitDice;
    const avgHp = Math.floor(count * ((sides + 1) / 2)) + bonus;
    this.combat.maxHp = Math.max(avgHp, 10);
    this.combat.hp = this.combat.maxHp;

    // Set damage from template
    this.combat.damroll = Math.floor(template.damageDice.count * ((template.damageDice.sides + 1) / 2)) + template.damageDice.bonus;
    this.combat.hitroll = template.level + 2;
    this.combat.armor = template.armor;

    // Set mana based on level
    this.combat.maxMana = 50 + template.level * 5;
    this.combat.mana = this.combat.maxMana;
    this.combat.maxMove = 100;
    this.combat.move = 100;

    // Determine behaviors
    this.behaviors = new Set<NPCBehavior>();
    if (template.flags.has("aggressive")) {
      this.behaviors.add(NPCBehavior.Aggressive);
      this.aggroRange = 0; // same room
    }
    if (template.flags.has("sentinel")) {
      this.behaviors.add(NPCBehavior.Sentinel);
    } else {
      this.behaviors.add(NPCBehavior.Wandering);
    }
    if (template.flags.has("shopkeeper")) {
      this.behaviors.add(NPCBehavior.Shopkeeper);
    }
    if (template.flags.has("healer")) {
      this.behaviors.add(NPCBehavior.Healer);
    }
    if (template.flags.has("train") || template.flags.has("practice")) {
      this.behaviors.add(NPCBehavior.Trainer);
    }

    // Generate loot table based on level
    this.generateLootTable(template);
  }

  private calculateXpReward(level: number): number {
    return Math.floor(50 * level + level * level * 2);
  }

  private generateLootTable(template: MobTemplate): void {
    // Base gold drop
    if (template.gold > 0) {
      this.gold = template.gold;
    }

    // Higher level mobs have better loot chances
    // Actual loot assignment happens at world level based on obj templates
  }

  get isAggressive(): boolean {
    return this.behaviors.has(NPCBehavior.Aggressive);
  }

  get isShopkeeper(): boolean {
    return this.behaviors.has(NPCBehavior.Shopkeeper);
  }

  get isHealer(): boolean {
    return this.behaviors.has(NPCBehavior.Healer);
  }

  getWeaponDamage(): { min: number; max: number } {
    return {
      min: Math.max(1, Math.floor(this.combat.damroll * 0.7)),
      max: Math.max(2, this.combat.damroll),
    };
  }

  die(): void {
    this.alive = false;
    this.target = null;
    this.respawnTimer = this.respawnTime;
  }

  respawn(): void {
    this.alive = true;
    this.combat.hp = this.combat.maxHp;
    this.combat.mana = this.combat.maxMana;
    this.combat.move = this.combat.maxMove;
    this.roomVnum = this.spawnRoomVnum;
    this.target = null;
    this.attackCooldown = 0;
    this.respawnTimer = 0;
  }

  serialize() {
    return {
      id: this.id,
      type: this.type,
      templateVnum: this.templateVnum,
      name: this.name,
      shortDesc: this.shortDesc,
      longDesc: this.longDesc,
      level: this.level,
      roomVnum: this.roomVnum,
      hpPercent: this.hpPercent,
      alive: this.alive,
      isInCombat: this.isInCombat,
      behaviors: Array.from(this.behaviors),
      gold: this.gold,
      alignment: this.alignment,
    };
  }
}
