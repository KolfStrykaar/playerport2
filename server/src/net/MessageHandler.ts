import type { Session } from "./Session.js";
import { SessionState } from "./Session.js";
import type { ClientMessage, ServerMessage } from "./Protocol.js";
import type { WorldManager } from "../world/WorldManager.js";
import type { EntityManager } from "../entities/EntityManager.js";
import type { CombatManager } from "../combat/CombatManager.js";
import type { ChatManager } from "../chat/ChatManager.js";
import type { AuthManager } from "../auth/AuthManager.js";
import type { SpawnManager } from "../world/SpawnManager.js";
import type { Player } from "../entities/Player.js";
import { Item } from "../entities/Item.js";
import type { Direction } from "../world/Room.js";

const VALID_DIRECTIONS = new Set(["north", "east", "south", "west", "up", "down"]);

export class MessageHandler {
  constructor(
    private world: WorldManager,
    private entities: EntityManager,
    private combat: CombatManager,
    private chat: ChatManager,
    private auth: AuthManager,
    private sessions: Map<string, Session>,
    private startTime: number,
  ) {}

  async handle(session: Session, message: ClientMessage): Promise<void> {
    switch (message.type) {
      case "c_login": return this.handleLogin(session, message.username, message.password);
      case "c_register": return this.handleRegister(session, message.username, message.password, message.characterName, message.playerClass, message.race);
      case "c_move": return this.handleMove(session, message.direction);
      case "c_look": return this.handleLook(session, message.target);
      case "c_attack": return this.handleAttack(session, message.targetId);
      case "c_use_skill": return this.handleUseSkill(session, message.skillName, message.targetId);
      case "c_chat": return this.handleChat(session, message.channel, message.message, message.targetName);
      case "c_pickup_item": return this.handlePickupItem(session, message.itemId);
      case "c_drop_item": return this.handleDropItem(session, message.itemId);
      case "c_equip_item": return this.handleEquipItem(session, message.itemId);
      case "c_unequip_item": return this.handleUnequipItem(session, message.slot);
      case "c_interact": return this.handleInteract(session, message.npcId, message.action);
      case "c_ping": return this.handlePing(session, message.timestamp);
      case "c_respawn": return this.handleRespawn(session);
      default: {
        session.send({ type: "s_system_message", message: `Unknown message type`, severity: "error" });
      }
    }
  }

  // --- Auth ---

  private async handleLogin(session: Session, username: string, password: string): Promise<void> {
    if (session.isAuthenticated) {
      session.send({ type: "s_login_failed", reason: "Already logged in" });
      return;
    }

    const result = await this.auth.login(username, password);
    if (!result.success || !result.player) {
      session.send({ type: "s_login_failed", reason: result.reason ?? "Invalid credentials" });
      return;
    }

    // Check if already logged in
    const existing = this.entities.getPlayerByAccount(result.player.accountId);
    if (existing) {
      session.send({ type: "s_login_failed", reason: "Character already logged in" });
      return;
    }

    const player = result.player;
    session.accountId = player.accountId;
    session.playerId = player.id;
    session.state = SessionState.InGame;

    this.entities.addPlayer(player);

    const room = this.world.getRoom(player.roomVnum);
    const roomMap = this.world.getRoomMap(player.roomVnum);

    session.send({
      type: "s_login_success",
      player: player.serialize(),
      room: room?.serialize() ?? {},
      roomMap,
    });

    // Notify room
    this.broadcastToRoom(player.roomVnum, {
      type: "s_entity_enter",
      entity: player.serializePublic(),
    }, player.id);

    // Send room state
    this.sendRoomState(session, player);

    console.log(`[Game] ${player.name} (${player.playerClass} ${player.race} L${player.level}) logged in to room ${player.roomVnum}`);
  }

  private async handleRegister(session: Session, username: string, password: string, characterName: string, playerClass: string, race: string): Promise<void> {
    if (session.isAuthenticated) {
      session.send({ type: "s_register_failed", reason: "Already logged in" });
      return;
    }

    const result = await this.auth.register(username, password, characterName, playerClass, race, this.world.startRoom);
    if (!result.success) {
      session.send({ type: "s_register_failed", reason: result.reason ?? "Registration failed" });
      return;
    }

    session.send({ type: "s_register_success", message: "Account created! You can now log in." });
  }

  // --- Movement ---

  private handleMove(session: Session, direction: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    if (player.isDead) {
      session.send({ type: "s_system_message", message: "You are dead. Use respawn first.", severity: "warning" });
      return;
    }

    if (player.isInCombat) {
      session.send({ type: "s_system_message", message: "You can't move while in combat!", severity: "warning" });
      return;
    }

    if (!VALID_DIRECTIONS.has(direction)) {
      session.send({ type: "s_system_message", message: "Invalid direction.", severity: "error" });
      return;
    }

    const dir = direction as Direction;
    const room = this.world.getRoom(player.roomVnum);
    if (!room) return;

    const exit = room.getExit(dir);
    if (!exit) {
      session.send({ type: "s_system_message", message: `There is no exit ${direction}.`, severity: "info" });
      return;
    }

    if (exit.closed) {
      session.send({ type: "s_system_message", message: "The door is closed.", severity: "info" });
      return;
    }

    const targetRoom = this.world.getRoom(exit.targetVnum);
    if (!targetRoom) {
      session.send({ type: "s_system_message", message: "That exit leads nowhere.", severity: "error" });
      return;
    }

    // Check movement points
    const cost = targetRoom.moveCost;
    if (player.combat.move < cost) {
      session.send({ type: "s_system_message", message: "You are too exhausted to move.", severity: "warning" });
      return;
    }

    player.combat.move -= cost;
    const fromVnum = player.roomVnum;

    // Notify old room
    this.broadcastToRoom(fromVnum, {
      type: "s_entity_leave",
      entityId: player.id,
      entityName: player.name,
      direction,
    }, player.id);

    // Move entity
    this.entities.moveEntity(player.id, fromVnum, exit.targetVnum);

    // Notify new room
    this.broadcastToRoom(exit.targetVnum, {
      type: "s_entity_enter",
      entity: player.serializePublic(),
      direction,
    }, player.id);

    // Send room data to player
    this.sendRoomState(session, player);

    // Check for aggressive NPCs
    this.checkAggroNPCs(player);

    player.lastActivity = Date.now();
  }

  // --- Look ---

  private handleLook(session: Session, target?: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    if (target) {
      // Look at a specific entity or item
      const npc = this.entities.findNPCInRoom(player.roomVnum, target);
      if (npc) {
        session.send({
          type: "s_system_message",
          message: `${npc.shortDesc}\n${npc.description}\nLevel ${npc.level} - ${npc.hpPercent}% HP`,
          severity: "info",
        });
        return;
      }

      const otherPlayer = this.entities.findPlayerByName(target);
      if (otherPlayer && otherPlayer.roomVnum === player.roomVnum) {
        session.send({
          type: "s_system_message",
          message: `${otherPlayer.name} the ${otherPlayer.race} ${otherPlayer.playerClass}\nLevel ${otherPlayer.level} - ${otherPlayer.hpPercent}% HP`,
          severity: "info",
        });
        return;
      }

      session.send({ type: "s_system_message", message: "You don't see that here.", severity: "info" });
      return;
    }

    this.sendRoomState(session, player);
  }

  // --- Combat ---

  private handleAttack(session: Session, targetId: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    if (player.isDead) {
      session.send({ type: "s_system_message", message: "You are dead.", severity: "warning" });
      return;
    }

    const target = this.entities.getEntity(targetId);
    if (!target || target.roomVnum !== player.roomVnum || target.isDead) {
      session.send({ type: "s_system_message", message: "Target not found.", severity: "error" });
      return;
    }

    if (target.type === "player") {
      const room = this.world.getRoom(player.roomVnum);
      if (room?.flags.has("safe")) {
        session.send({ type: "s_system_message", message: "You can't fight here - this is a safe zone.", severity: "warning" });
        return;
      }
    }

    this.combat.initiateCombat(player, target);
    player.lastActivity = Date.now();
  }

  private handleUseSkill(session: Session, skillName: string, targetId?: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    if (player.isDead) {
      session.send({ type: "s_system_message", message: "You are dead.", severity: "warning" });
      return;
    }

    const target = targetId ? this.entities.getEntity(targetId) : null;
    const result = this.combat.useSkill(player, skillName, target ?? undefined);

    if (!result.success) {
      session.send({ type: "s_system_message", message: result.message ?? "Skill failed.", severity: "warning" });
    }

    player.lastActivity = Date.now();
  }

  // --- Chat ---

  private handleChat(session: Session, channel: string, message: string, targetName?: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    this.chat.handleMessage(player, channel, message, targetName);
    player.lastActivity = Date.now();
  }

  // --- Inventory ---

  private handlePickupItem(session: Session, itemId: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    const room = this.world.getRoom(player.roomVnum);
    if (!room || !room.items.has(itemId)) {
      session.send({ type: "s_system_message", message: "Item not found.", severity: "error" });
      return;
    }

    const item = this.entities.getItem(itemId);
    if (!item) {
      room.items.delete(itemId);
      return;
    }

    if (!player.addItem(item)) {
      session.send({ type: "s_system_message", message: "Your inventory is full.", severity: "warning" });
      return;
    }

    room.items.delete(itemId);
    session.send({ type: "s_player_update", player: player.serialize() });

    this.broadcastToRoom(player.roomVnum, {
      type: "s_system_message",
      message: `${player.name} picks up ${item.shortDesc}.`,
      severity: "info",
    }, player.id);

    player.lastActivity = Date.now();
  }

  private handleDropItem(session: Session, itemId: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    const item = player.removeItem(itemId);
    if (!item) {
      session.send({ type: "s_system_message", message: "Item not found in inventory.", severity: "error" });
      return;
    }

    const room = this.world.getRoom(player.roomVnum);
    room?.items.add(item.instanceId);
    this.entities.addItem(item);

    session.send({ type: "s_player_update", player: player.serialize() });
    player.lastActivity = Date.now();
  }

  private handleEquipItem(session: Session, itemId: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    const item = player.inventory.find((i) => i.instanceId === itemId);
    if (!item) {
      session.send({ type: "s_system_message", message: "Item not found.", severity: "error" });
      return;
    }

    const unequipped = player.equip(item);
    if (unequipped === null && !player.equipment.has(item.wearSlot)) {
      session.send({ type: "s_system_message", message: "You can't equip that.", severity: "warning" });
      return;
    }

    session.send({ type: "s_player_update", player: player.serialize() });
    player.lastActivity = Date.now();
  }

  private handleUnequipItem(session: Session, slot: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    const item = player.unequip(slot as any);
    if (!item) {
      session.send({ type: "s_system_message", message: "Nothing equipped in that slot.", severity: "error" });
      return;
    }

    session.send({ type: "s_player_update", player: player.serialize() });
    player.lastActivity = Date.now();
  }

  // --- NPC Interaction ---

  private handleInteract(session: Session, npcId: string, action: string): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    const npc = this.entities.getNPC(npcId);
    if (!npc || npc.roomVnum !== player.roomVnum || npc.isDead) {
      session.send({ type: "s_system_message", message: "NPC not found.", severity: "error" });
      return;
    }

    switch (action) {
      case "talk": {
        if (npc.isShopkeeper) {
          session.send({ type: "s_system_message", message: `${npc.shortDesc} says: "Welcome! What would you like to buy?"`, severity: "info" });
        } else if (npc.isHealer) {
          session.send({ type: "s_system_message", message: `${npc.shortDesc} says: "I can heal your wounds for a price."`, severity: "info" });
        } else {
          session.send({ type: "s_system_message", message: `${npc.shortDesc} nods at you.`, severity: "info" });
        }
        break;
      }
      case "heal": {
        if (!npc.isHealer) {
          session.send({ type: "s_system_message", message: "This NPC can't heal you.", severity: "warning" });
          return;
        }
        const cost = (player.combat.maxHp - player.combat.hp) * 2;
        if (player.gold < cost) {
          session.send({ type: "s_system_message", message: `You need ${cost} gold to be fully healed.`, severity: "warning" });
          return;
        }
        player.gold -= cost;
        player.combat.hp = player.combat.maxHp;
        player.combat.mana = player.combat.maxMana;
        session.send({ type: "s_player_update", player: player.serialize() });
        session.send({ type: "s_system_message", message: `${npc.shortDesc} heals you for ${cost} gold.`, severity: "info" });
        break;
      }
      default: {
        session.send({ type: "s_system_message", message: "Unknown interaction.", severity: "error" });
      }
    }

    player.lastActivity = Date.now();
  }

  // --- Respawn ---

  private handleRespawn(session: Session): void {
    const player = this.getSessionPlayer(session);
    if (!player) return;

    if (!player.isDead) {
      session.send({ type: "s_system_message", message: "You are not dead.", severity: "info" });
      return;
    }

    const oldRoom = player.roomVnum;
    player.respawn(this.world.startRoom);
    this.entities.moveEntity(player.id, oldRoom, this.world.startRoom);

    this.sendRoomState(session, player);
    session.send({ type: "s_player_update", player: player.serialize() });
    session.send({ type: "s_system_message", message: "You have been resurrected.", severity: "info" });

    this.broadcastToRoom(this.world.startRoom, {
      type: "s_entity_enter",
      entity: player.serializePublic(),
    }, player.id);
  }

  // --- Ping ---

  private handlePing(session: Session, timestamp: number): void {
    session.lastPing = Date.now();
    session.send({ type: "s_pong", clientTimestamp: timestamp, serverTimestamp: Date.now() });
  }

  // --- Helpers ---

  private getSessionPlayer(session: Session): Player | null {
    if (!session.isInGame || !session.playerId) {
      session.send({ type: "s_system_message", message: "You must be logged in.", severity: "error" });
      return null;
    }
    return this.entities.getPlayer(session.playerId) ?? null;
  }

  private sendRoomState(session: Session, player: Player): void {
    const room = this.world.getRoom(player.roomVnum);
    if (!room) return;

    const entities = this.entities.getEntitiesInRoom(player.roomVnum)
      .filter((e) => e.id !== player.id)
      .map((e) => e.serialize());

    const items: Array<Record<string, unknown>> = [];
    for (const itemId of room.items) {
      const item = this.entities.getItem(itemId);
      if (item) items.push(item.serialize());
    }

    const roomMap = this.world.getRoomMap(player.roomVnum);

    session.send({
      type: "s_room_change",
      room: room.serialize(),
      roomMap,
      entities,
      items,
    });

    session.send({ type: "s_player_update", player: player.serialize() });
  }

  private checkAggroNPCs(player: Player): void {
    const npcs = this.entities.getNPCsInRoom(player.roomVnum);
    for (const npc of npcs) {
      if (npc.isAggressive && npc.alive && !npc.isInCombat) {
        this.combat.initiateCombat(npc, player);
        break; // Only one aggro at a time
      }
    }
  }

  /** Broadcast a message to all players in a room, optionally excluding one */
  broadcastToRoom(roomVnum: number, message: ServerMessage, excludeId?: string): void {
    const players = this.entities.getPlayersInRoom(roomVnum);
    for (const player of players) {
      if (player.id === excludeId) continue;
      const session = this.findSessionByPlayerId(player.id);
      session?.send(message);
    }
  }

  /** Broadcast to all connected, in-game sessions */
  broadcastAll(message: ServerMessage, excludeId?: string): void {
    for (const session of this.sessions.values()) {
      if (!session.isInGame) continue;
      if (session.playerId === excludeId) continue;
      session.send(message);
    }
  }

  private findSessionByPlayerId(playerId: string): Session | undefined {
    for (const session of this.sessions.values()) {
      if (session.playerId === playerId) return session;
    }
    return undefined;
  }
}
