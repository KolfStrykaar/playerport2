import type { WorldManager } from "./WorldManager.js";
import type { EntityManager } from "../entities/EntityManager.js";
import { NPC } from "../entities/NPC.js";
import { Item } from "../entities/Item.js";
import type { Session } from "../net/Session.js";
import type { ServerMessage } from "../net/Protocol.js";

export class SpawnManager {
  private spawnedNPCs: Map<string, { zoneFile: string; spawnIndex: number }> = new Map();
  private deadNPCTimers: Map<string, { npc: NPC; timer: number; zoneFile: string; spawnIndex: number }> = new Map();

  constructor(
    private world: WorldManager,
    private entities: EntityManager,
  ) {}

  /** Initial world population */
  populateWorld(): void {
    let npcCount = 0;
    let itemCount = 0;

    for (const [fileName, zone] of this.world.zones) {
      for (let si = 0; si < zone.mobSpawns.length; si++) {
        const spawn = zone.mobSpawns[si];
        const template = this.world.getMobTemplate(spawn.mobVnum);
        const room = this.world.getRoom(spawn.roomVnum);
        if (!template || !room) continue;

        for (let i = 0; i < spawn.maxCount; i++) {
          const npc = new NPC(template, spawn.roomVnum);
          this.entities.addNPC(npc);
          this.spawnedNPCs.set(npc.id, { zoneFile: fileName, spawnIndex: si });
          spawn.currentCount++;
          npcCount++;
        }
      }

      for (const spawn of zone.objSpawns) {
        const template = this.world.getObjTemplate(spawn.objVnum);
        const room = this.world.getRoom(spawn.roomVnum);
        if (!template || !room) continue;

        for (let i = 0; i < spawn.maxCount; i++) {
          const item = Item.fromTemplate(template);
          this.entities.addItem(item);
          room.items.add(item.instanceId);
          spawn.currentCount++;
          itemCount++;
        }
      }
    }

    console.log(`[Spawn] Populated world: ${npcCount} NPCs, ${itemCount} items`);
  }

  /** Called when an NPC dies - start respawn timer */
  onNPCDeath(npc: NPC): void {
    const spawnInfo = this.spawnedNPCs.get(npc.id);
    if (!spawnInfo) return;

    const zone = this.world.zones.get(spawnInfo.zoneFile);
    if (zone) {
      const spawn = zone.mobSpawns[spawnInfo.spawnIndex];
      if (spawn) spawn.currentCount = Math.max(0, spawn.currentCount - 1);
    }

    // Remove dead NPC from room
    const room = this.world.getRoom(npc.roomVnum);
    room?.removeOccupant(npc.id);

    this.deadNPCTimers.set(npc.id, {
      npc,
      timer: npc.respawnTime,
      zoneFile: spawnInfo.zoneFile,
      spawnIndex: spawnInfo.spawnIndex,
    });
  }

  /** Tick update - process respawn timers */
  update(deltaMs: number, broadcastToRoom: (roomVnum: number, msg: ServerMessage) => void): void {
    const toRespawn: string[] = [];

    for (const [id, entry] of this.deadNPCTimers) {
      entry.timer -= deltaMs;
      if (entry.timer <= 0) {
        toRespawn.push(id);
      }
    }

    for (const id of toRespawn) {
      const entry = this.deadNPCTimers.get(id);
      if (!entry) continue;

      this.deadNPCTimers.delete(id);
      this.spawnedNPCs.delete(id);

      // Respawn a new NPC
      const zone = this.world.zones.get(entry.zoneFile);
      if (!zone) continue;

      const spawn = zone.mobSpawns[entry.spawnIndex];
      if (!spawn) continue;

      if (spawn.currentCount >= spawn.maxCount) continue;

      const template = this.world.getMobTemplate(spawn.mobVnum);
      const room = this.world.getRoom(spawn.roomVnum);
      if (!template || !room) continue;

      const newNPC = new NPC(template, spawn.roomVnum);
      this.entities.addNPC(newNPC);
      this.spawnedNPCs.set(newNPC.id, { zoneFile: entry.zoneFile, spawnIndex: entry.spawnIndex });
      spawn.currentCount++;

      // Notify players in the room
      broadcastToRoom(spawn.roomVnum, {
        type: "s_spawn_effect",
        entityId: newNPC.id,
        entity: newNPC.serialize(),
      });
    }
  }

  /** Generate loot drops when an NPC dies */
  generateLoot(npc: NPC): Item[] {
    const loot: Item[] = [];

    // Gold drop
    if (npc.gold > 0) {
      const goldVariance = Math.floor(npc.gold * 0.3);
      const goldAmount = npc.gold + Math.floor(Math.random() * goldVariance * 2) - goldVariance;
      if (goldAmount > 0) {
        const goldItem = new Item(0, "gold coins", `${goldAmount} gold coins`, `A pile of ${goldAmount} gold coins lies here.`, 20 /* Money */, 1);
        goldItem.cost = goldAmount;
        loot.push(goldItem);
      }
    }

    // Random item drops based on NPC level
    const dropChance = Math.min(0.5, 0.1 + npc.level * 0.005);
    if (Math.random() < dropChance) {
      // Find a suitable item template from the same zone
      const spawnInfo = this.spawnedNPCs.get(npc.id) ?? this.deadNPCTimers.get(npc.id);
      if (spawnInfo) {
        const zone = this.world.zones.get(spawnInfo.zoneFile);
        if (zone && zone.objTemplates.size > 0) {
          const templates = Array.from(zone.objTemplates.values())
            .filter((t) => t.level <= npc.level + 5 && t.level >= Math.max(1, npc.level - 10));

          if (templates.length > 0) {
            const template = templates[Math.floor(Math.random() * templates.length)];
            loot.push(Item.fromTemplate(template));
          }
        }
      }
    }

    return loot;
  }
}
