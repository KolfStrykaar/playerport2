import { v4 as uuidv4 } from "uuid";

export interface Stats {
  str: number;
  dex: number;
  con: number;
  int: number;
  wis: number;
  cha: number;
}

export interface CombatStats {
  hp: number;
  maxHp: number;
  mana: number;
  maxMana: number;
  move: number;
  maxMove: number;
  armor: number;
  hitroll: number;
  damroll: number;
}

export type EntityType = "player" | "npc";

export abstract class Entity {
  readonly id: string;
  readonly type: EntityType;
  name: string;
  level: number;
  roomVnum: number;
  stats: Stats;
  combat: CombatStats;
  alignment: number;

  /** Entity currently in combat with (null if not fighting) */
  target: string | null = null;
  /** Cooldown timestamp for next attack */
  attackCooldown = 0;
  /** Is this entity alive? */
  alive = true;

  constructor(type: EntityType, name: string, level: number, roomVnum: number) {
    this.id = uuidv4();
    this.type = type;
    this.name = name;
    this.level = level;
    this.roomVnum = roomVnum;
    this.alignment = 0;

    this.stats = { str: 13, dex: 13, con: 13, int: 13, wis: 13, cha: 13 };
    this.combat = {
      hp: 20 + level * 8,
      maxHp: 20 + level * 8,
      mana: 100 + level * 5,
      maxMana: 100 + level * 5,
      move: 100 + level * 2,
      maxMove: 100 + level * 2,
      armor: 100 - level * 2,
      hitroll: level,
      damroll: level,
    };
  }

  get isDead(): boolean {
    return this.combat.hp <= 0 || !this.alive;
  }

  get isInCombat(): boolean {
    return this.target !== null;
  }

  get hpPercent(): number {
    return Math.round((this.combat.hp / this.combat.maxHp) * 100);
  }

  get manaPercent(): number {
    return Math.round((this.combat.mana / this.combat.maxMana) * 100);
  }

  get movePercent(): number {
    return Math.round((this.combat.move / this.combat.maxMove) * 100);
  }

  takeDamage(amount: number): number {
    const actual = Math.min(this.combat.hp, Math.max(0, amount));
    this.combat.hp -= actual;
    if (this.combat.hp <= 0) {
      this.combat.hp = 0;
      this.alive = false;
    }
    return actual;
  }

  heal(amount: number): number {
    const actual = Math.min(this.combat.maxHp - this.combat.hp, Math.max(0, amount));
    this.combat.hp += actual;
    return actual;
  }

  regen(): void {
    if (this.isDead) return;

    // HP regen: ~2% per tick
    const hpRegen = Math.max(1, Math.floor(this.combat.maxHp * 0.02));
    this.combat.hp = Math.min(this.combat.maxHp, this.combat.hp + hpRegen);

    // Mana regen: ~3% per tick
    const manaRegen = Math.max(1, Math.floor(this.combat.maxMana * 0.03));
    this.combat.mana = Math.min(this.combat.maxMana, this.combat.mana + manaRegen);

    // Move regen: ~5% per tick
    const moveRegen = Math.max(1, Math.floor(this.combat.maxMove * 0.05));
    this.combat.move = Math.min(this.combat.maxMove, this.combat.move + moveRegen);
  }

  abstract serialize(): Record<string, unknown>;
}
