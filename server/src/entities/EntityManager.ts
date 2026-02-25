import type { Entity } from "./Entity.js";
import type { Player } from "./Player.js";
import type { NPC } from "./NPC.js";
import type { Item } from "./Item.js";
import type { WorldManager } from "../world/WorldManager.js";

export class EntityManager {
  private entities: Map<string, Entity> = new Map();
  private players: Map<string, Player> = new Map();
  private npcs: Map<string, NPC> = new Map();
  private items: Map<string, Item> = new Map();

  /** Map accountId -> playerId for quick lookup */
  private accountToPlayer: Map<string, string> = new Map();

  constructor(private world: WorldManager) {}

  // --- Entity lifecycle ---

  addPlayer(player: Player): void {
    this.entities.set(player.id, player);
    this.players.set(player.id, player);
    this.accountToPlayer.set(player.accountId, player.id);

    const room = this.world.getRoom(player.roomVnum);
    room?.addOccupant(player.id);
  }

  removePlayer(playerId: string): Player | undefined {
    const player = this.players.get(playerId);
    if (!player) return undefined;

    const room = this.world.getRoom(player.roomVnum);
    room?.removeOccupant(player.id);

    this.entities.delete(playerId);
    this.players.delete(playerId);
    this.accountToPlayer.delete(player.accountId);
    return player;
  }

  addNPC(npc: NPC): void {
    this.entities.set(npc.id, npc);
    this.npcs.set(npc.id, npc);

    const room = this.world.getRoom(npc.roomVnum);
    room?.addOccupant(npc.id);
  }

  removeNPC(npcId: string): NPC | undefined {
    const npc = this.npcs.get(npcId);
    if (!npc) return undefined;

    const room = this.world.getRoom(npc.roomVnum);
    room?.removeOccupant(npc.id);

    this.entities.delete(npcId);
    this.npcs.delete(npcId);
    return npc;
  }

  addItem(item: Item): void {
    this.items.set(item.instanceId, item);
  }

  removeItem(instanceId: string): Item | undefined {
    const item = this.items.get(instanceId);
    if (item) this.items.delete(instanceId);
    return item;
  }

  // --- Lookups ---

  getEntity(id: string): Entity | undefined {
    return this.entities.get(id);
  }

  getPlayer(id: string): Player | undefined {
    return this.players.get(id);
  }

  getPlayerByAccount(accountId: string): Player | undefined {
    const playerId = this.accountToPlayer.get(accountId);
    return playerId ? this.players.get(playerId) : undefined;
  }

  getNPC(id: string): NPC | undefined {
    return this.npcs.get(id);
  }

  getItem(instanceId: string): Item | undefined {
    return this.items.get(instanceId);
  }

  getAllPlayers(): IterableIterator<Player> {
    return this.players.values();
  }

  getAllNPCs(): IterableIterator<NPC> {
    return this.npcs.values();
  }

  get playerCount(): number {
    return this.players.size;
  }

  get npcCount(): number {
    return this.npcs.size;
  }

  get entityCount(): number {
    return this.entities.size;
  }

  // --- Spatial queries ---

  /** Get all entities in a room */
  getEntitiesInRoom(roomVnum: number): Entity[] {
    const room = this.world.getRoom(roomVnum);
    if (!room) return [];

    const entities: Entity[] = [];
    for (const id of room.occupants) {
      const entity = this.entities.get(id);
      if (entity && entity.alive) entities.push(entity);
    }
    return entities;
  }

  /** Get all players in a room */
  getPlayersInRoom(roomVnum: number): Player[] {
    return this.getEntitiesInRoom(roomVnum).filter((e): e is Player => e.type === "player") as Player[];
  }

  /** Get all NPCs in a room */
  getNPCsInRoom(roomVnum: number): NPC[] {
    return this.getEntitiesInRoom(roomVnum).filter((e): e is NPC => e.type === "npc") as NPC[];
  }

  /** Move entity to a new room */
  moveEntity(entityId: string, fromVnum: number, toVnum: number): boolean {
    const entity = this.entities.get(entityId);
    if (!entity) return false;

    const fromRoom = this.world.getRoom(fromVnum);
    const toRoom = this.world.getRoom(toVnum);
    if (!toRoom) return false;

    fromRoom?.removeOccupant(entityId);
    toRoom.addOccupant(entityId);
    entity.roomVnum = toVnum;
    return true;
  }

  /** Find a player by name (case-insensitive, prefix match) */
  findPlayerByName(name: string): Player | undefined {
    const lower = name.toLowerCase();
    for (const player of this.players.values()) {
      if (player.name.toLowerCase() === lower) return player;
    }
    for (const player of this.players.values()) {
      if (player.name.toLowerCase().startsWith(lower)) return player;
    }
    return undefined;
  }

  /** Find an NPC in a room by name */
  findNPCInRoom(roomVnum: number, name: string): NPC | undefined {
    const lower = name.toLowerCase();
    const npcs = this.getNPCsInRoom(roomVnum);
    return npcs.find((n) => n.name.toLowerCase().includes(lower) || n.shortDesc.toLowerCase().includes(lower));
  }
}
