import { Config } from "../config.js";
import type { Entity } from "../entities/Entity.js";
import type { Player } from "../entities/Player.js";
import type { NPC } from "../entities/NPC.js";
import type { EntityManager } from "../entities/EntityManager.js";
import type { WorldManager } from "../world/WorldManager.js";
import type { SpawnManager } from "../world/SpawnManager.js";
import type { Session } from "../net/Session.js";
import type { ServerMessage } from "../net/Protocol.js";

export interface SkillResult {
  success: boolean;
  message?: string;
}

interface CombatPair {
  attacker: Entity;
  defender: Entity;
  startedAt: number;
}

export class CombatManager {
  private activeFights: Map<string, CombatPair> = new Map();
  private broadcastToRoom: ((roomVnum: number, msg: ServerMessage, excludeId?: string) => void) | null = null;
  private findSession: ((playerId: string) => Session | undefined) | null = null;

  constructor(
    private entities: EntityManager,
    private world: WorldManager,
    private spawns: SpawnManager,
  ) {}

  setBroadcasters(
    broadcastToRoom: (roomVnum: number, msg: ServerMessage, excludeId?: string) => void,
    findSession: (playerId: string) => Session | undefined,
  ): void {
    this.broadcastToRoom = broadcastToRoom;
    this.findSession = findSession;
  }

  initiateCombat(attacker: Entity, defender: Entity): void {
    if (attacker.isDead || defender.isDead) return;
    if (attacker.id === defender.id) return;

    attacker.target = defender.id;
    defender.target = attacker.id;

    const key = this.fightKey(attacker.id, defender.id);
    if (!this.activeFights.has(key)) {
      this.activeFights.set(key, { attacker, defender, startedAt: Date.now() });
    }

    // Notify room
    this.broadcastToRoom?.(attacker.roomVnum, {
      type: "s_system_message",
      message: `${attacker.name} attacks ${defender.name}!`,
      severity: "info",
    });
  }

  /** Main combat tick - process all active fights */
  update(now: number): void {
    const toRemove: string[] = [];

    for (const [key, fight] of this.activeFights) {
      const { attacker, defender } = fight;

      // Check if fight should end
      if (attacker.isDead || defender.isDead || attacker.roomVnum !== defender.roomVnum) {
        this.endCombat(attacker, defender);
        toRemove.push(key);
        continue;
      }

      // Process attacker's turn
      if (now >= attacker.attackCooldown) {
        this.processAttack(attacker, defender, now);
      }

      // Process defender's turn
      if (now >= defender.attackCooldown) {
        this.processAttack(defender, attacker, now);
      }
    }

    for (const key of toRemove) {
      this.activeFights.delete(key);
    }
  }

  private processAttack(attacker: Entity, defender: Entity, now: number): void {
    if (attacker.isDead || defender.isDead) return;

    const gcd = Config.combat.globalCooldown;
    attacker.attackCooldown = now + gcd;

    // Hit calculation
    const hitChance = 70 + (attacker.combat.hitroll - defender.combat.armor / 10);
    const roll = Math.random() * 100;

    if (roll > hitChance) {
      // Miss
      this.broadcastToRoom?.(attacker.roomVnum, {
        type: "s_combat_action",
        attackerId: attacker.id,
        attackerName: attacker.name,
        targetId: defender.id,
        targetName: defender.name,
        damage: 0,
        skill: "miss",
        targetHpPercent: defender.hpPercent,
      });
      return;
    }

    // Damage calculation
    let damage: number;
    if (attacker.type === "player") {
      const { min, max } = (attacker as Player).getWeaponDamage();
      damage = min + Math.floor(Math.random() * (max - min + 1));
    } else {
      const { min, max } = (attacker as NPC).getWeaponDamage();
      damage = min + Math.floor(Math.random() * (max - min + 1));
    }

    // Critical hit (10% chance, 1.5x damage)
    let skill = "hit";
    if (Math.random() < 0.1) {
      damage = Math.floor(damage * 1.5);
      skill = "critical";
    }

    // Apply armor reduction
    const armorReduction = Math.max(0, -defender.combat.armor) / 200;
    damage = Math.max(1, Math.floor(damage * (1 - armorReduction)));

    const actualDamage = defender.takeDamage(damage);

    // Broadcast combat action
    this.broadcastToRoom?.(attacker.roomVnum, {
      type: "s_combat_action",
      attackerId: attacker.id,
      attackerName: attacker.name,
      targetId: defender.id,
      targetName: defender.name,
      damage: actualDamage,
      skill,
      targetHpPercent: defender.hpPercent,
    });

    // Check for death
    if (defender.isDead) {
      this.handleDeath(attacker, defender);
    }
  }

  private handleDeath(killer: Entity, victim: Entity): void {
    this.endCombat(killer, victim);

    let xpGained = 0;
    let goldGained = 0;
    let loot: Array<Record<string, unknown>> = [];

    if (victim.type === "npc") {
      const npc = victim as NPC;

      // XP reward
      xpGained = Math.floor(npc.xpReward * Config.combat.xpMultiplier);
      goldGained = npc.gold;

      // Generate loot
      const lootItems = this.spawns.generateLoot(npc);
      loot = lootItems.map((i) => i.serialize());

      // Grant rewards to killer
      if (killer.type === "player") {
        const player = killer as Player;
        player.kills++;

        const { leveled, newLevel } = player.gainXp(xpGained);
        player.gold += goldGained;

        // Add loot to room
        const room = this.world.getRoom(victim.roomVnum);
        if (room) {
          for (const item of lootItems) {
            this.entities.addItem(item);
            room.items.add(item.instanceId);
          }
        }

        // Send level up notification
        if (leveled) {
          const session = this.findSession?.(player.id);
          session?.send({
            type: "s_level_up",
            newLevel,
            player: player.serialize(),
          });

          this.broadcastToRoom?.(player.roomVnum, {
            type: "s_system_message",
            message: `${player.name} has reached level ${newLevel}!`,
            severity: "info",
          });
        }

        // Update player
        const session = this.findSession?.(player.id);
        session?.send({ type: "s_player_update", player: player.serialize() });
      }

      // NPC death handling
      npc.die();
      this.spawns.onNPCDeath(npc);
      this.entities.removeNPC(npc.id);
    } else {
      // Player death
      const deadPlayer = victim as Player;
      deadPlayer.die();

      const session = this.findSession?.(deadPlayer.id);
      session?.send({ type: "s_player_update", player: deadPlayer.serialize() });
    }

    // Broadcast death
    this.broadcastToRoom?.(victim.roomVnum, {
      type: "s_entity_death",
      entityId: victim.id,
      entityName: victim.name,
      killerId: killer.id,
      killerName: killer.name,
      xpGained,
      goldGained,
      loot,
    });
  }

  private endCombat(a: Entity, b: Entity): void {
    if (a.target === b.id) a.target = null;
    if (b.target === a.id) b.target = null;

    const key = this.fightKey(a.id, b.id);
    this.activeFights.delete(key);
  }

  /** Flee from combat - 60% chance of success */
  flee(entity: Entity): boolean {
    if (!entity.isInCombat) return false;

    if (Math.random() > 0.6) return false;

    const opponent = entity.target ? this.entities.getEntity(entity.target) : null;
    if (opponent) {
      this.endCombat(entity, opponent);
    }

    return true;
  }

  useSkill(player: Player, skillName: string, target?: Entity): SkillResult {
    const now = Date.now();

    // Check cooldown
    const cdReady = player.cooldowns.get(skillName) ?? 0;
    if (now < cdReady) {
      const remaining = Math.ceil((cdReady - now) / 1000);
      return { success: false, message: `${skillName} is on cooldown (${remaining}s)` };
    }

    switch (skillName) {
      case "heal": {
        if (player.combat.mana < 30) return { success: false, message: "Not enough mana." };
        player.combat.mana -= 30;
        const healAmount = 20 + player.level * 3 + player.stats.wis * 2;
        player.heal(healAmount);
        player.cooldowns.set(skillName, now + 4000);

        this.broadcastToRoom?.(player.roomVnum, {
          type: "s_combat_action",
          attackerId: player.id,
          attackerName: player.name,
          targetId: player.id,
          targetName: player.name,
          damage: -healAmount,
          skill: "heal",
          targetHpPercent: player.hpPercent,
        });
        return { success: true };
      }

      case "fireball": {
        if (player.combat.mana < 50) return { success: false, message: "Not enough mana." };
        if (!target || target.roomVnum !== player.roomVnum) return { success: false, message: "No valid target." };

        player.combat.mana -= 50;
        const damage = 30 + player.level * 4 + player.stats.int * 3;
        target.takeDamage(damage);
        player.cooldowns.set(skillName, now + 6000);

        this.broadcastToRoom?.(player.roomVnum, {
          type: "s_combat_action",
          attackerId: player.id,
          attackerName: player.name,
          targetId: target.id,
          targetName: target.name,
          damage,
          skill: "fireball",
          targetHpPercent: target.hpPercent,
        });

        if (target.isDead) {
          this.handleDeath(player, target);
        } else if (!player.isInCombat) {
          this.initiateCombat(player, target);
        }

        return { success: true };
      }

      case "backstab": {
        if (!target || target.roomVnum !== player.roomVnum) return { success: false, message: "No valid target." };
        if (target.isInCombat) return { success: false, message: "Target is already in combat." };

        const damage = Math.floor((20 + player.level * 3 + player.stats.dex * 2) * 2.5);
        target.takeDamage(damage);
        player.cooldowns.set(skillName, now + 12000);

        this.broadcastToRoom?.(player.roomVnum, {
          type: "s_combat_action",
          attackerId: player.id,
          attackerName: player.name,
          targetId: target.id,
          targetName: target.name,
          damage,
          skill: "backstab",
          targetHpPercent: target.hpPercent,
        });

        if (target.isDead) {
          this.handleDeath(player, target);
        } else {
          this.initiateCombat(player, target);
        }

        return { success: true };
      }

      case "bash": {
        if (!target || target.roomVnum !== player.roomVnum) return { success: false, message: "No valid target." };

        const damage = 10 + player.level * 2 + player.stats.str * 2;
        target.takeDamage(damage);
        player.cooldowns.set(skillName, now + 3000);

        // Stun: delay target's next attack
        target.attackCooldown = Math.max(target.attackCooldown, now + 2000);

        this.broadcastToRoom?.(player.roomVnum, {
          type: "s_combat_action",
          attackerId: player.id,
          attackerName: player.name,
          targetId: target.id,
          targetName: target.name,
          damage,
          skill: "bash",
          targetHpPercent: target.hpPercent,
        });

        if (target.isDead) {
          this.handleDeath(player, target);
        } else if (!player.isInCombat) {
          this.initiateCombat(player, target);
        }

        return { success: true };
      }

      default:
        return { success: false, message: `Unknown skill: ${skillName}` };
    }
  }

  private fightKey(a: string, b: string): string {
    return a < b ? `${a}:${b}` : `${b}:${a}`;
  }

  get activeFightCount(): number {
    return this.activeFights.size;
  }
}
