import http from "node:http";
import { WebSocketServer, type WebSocket } from "ws";
import { v4 as uuidv4 } from "uuid";
import { Config } from "./config.js";
import { GameLoop } from "./GameLoop.js";
import { WorldManager } from "./world/WorldManager.js";
import { SpawnManager } from "./world/SpawnManager.js";
import { EntityManager } from "./entities/EntityManager.js";
import { CombatManager } from "./combat/CombatManager.js";
import { ChatManager } from "./chat/ChatManager.js";
import { AuthManager } from "./auth/AuthManager.js";
import { Session, SessionState } from "./net/Session.js";
import { MessageHandler } from "./net/MessageHandler.js";
import type { ClientMessage, ServerMessage } from "./net/Protocol.js";

export class GameServer {
  readonly world: WorldManager;
  readonly entities: EntityManager;
  readonly spawns: SpawnManager;
  readonly combat: CombatManager;
  readonly chat: ChatManager;
  readonly auth: AuthManager;
  readonly gameLoop: GameLoop;
  readonly messageHandler: MessageHandler;

  private httpServer: http.Server;
  private wss: WebSocketServer;
  private sessions: Map<string, Session> = new Map();
  private startTime = Date.now();

  // Tick-based counters
  private regenCounter = 0;
  private saveCounter = 0;
  private npcAICounter = 0;

  constructor() {
    this.world = new WorldManager();
    this.entities = new EntityManager(this.world);
    this.spawns = new SpawnManager(this.world, this.entities);
    this.combat = new CombatManager(this.entities, this.world, this.spawns);
    this.chat = new ChatManager(this.entities);
    this.auth = new AuthManager();
    this.gameLoop = new GameLoop();

    this.messageHandler = new MessageHandler(
      this.world,
      this.entities,
      this.combat,
      this.chat,
      this.auth,
      this.sessions,
      this.startTime,
    );

    // Wire up cross-system callbacks
    this.combat.setBroadcasters(
      (roomVnum, msg, excludeId) => this.broadcastToRoom(roomVnum, msg, excludeId),
      (playerId) => this.findSessionByPlayerId(playerId),
    );

    this.chat.setSessionFinder(
      (playerId) => this.findSessionByPlayerId(playerId),
    );

    // HTTP server
    this.httpServer = http.createServer((req, res) => this.handleHTTP(req, res));
    this.wss = new WebSocketServer({ server: this.httpServer });
  }

  async start(): Promise<void> {
    console.log("╔══════════════════════════════════════════╗");
    console.log("║       LOK MMORPG Server v1.0.0          ║");
    console.log("║     Legends of Krynn - 3D Backend       ║");
    console.log("╚══════════════════════════════════════════╝");
    console.log();

    // 1. Load world data
    this.world.load();

    // 2. Initialize auth
    this.auth.initialize();

    // 3. Populate world with NPCs and items
    this.spawns.populateWorld();

    // 4. Set up WebSocket handlers
    this.wss.on("connection", (ws) => this.handleConnection(ws));

    // 5. Start game loop
    this.gameLoop.onTick((tick, deltaMs) => this.onTick(tick, deltaMs));
    this.gameLoop.start();

    // 6. Start listening
    return new Promise((resolve) => {
      this.httpServer.listen(Config.server.port, Config.server.host, () => {
        console.log();
        console.log(`[Server] Listening on ${Config.server.host}:${Config.server.port}`);
        console.log(`[Server] WebSocket endpoint: ws://${Config.server.host}:${Config.server.port}`);
        console.log(`[Server] HTTP API: http://${Config.server.host}:${Config.server.port}/api/`);
        console.log();
        resolve();
      });
    });
  }

  stop(): void {
    console.log("[Server] Shutting down...");

    this.gameLoop.stop();

    // Save all players
    for (const player of this.entities.getAllPlayers()) {
      this.auth.savePlayer(player);
    }

    // Close all sessions
    for (const session of this.sessions.values()) {
      session.send({ type: "s_system_message", message: "Server shutting down.", severity: "warning" });
      session.close(1001, "Server shutdown");
    }

    this.wss.close();
    this.httpServer.close();
    console.log("[Server] Shutdown complete.");
  }

  // --- WebSocket ---

  private handleConnection(ws: WebSocket): void {
    const sessionId = uuidv4();
    const session = new Session(sessionId, ws);
    this.sessions.set(sessionId, session);

    console.log(`[Net] Client connected: ${sessionId} (${this.sessions.size} total)`);

    session.send({
      type: "s_world_info",
      playerCount: this.entities.playerCount,
      npcCount: this.entities.npcCount,
      roomCount: this.world.rooms.size,
      zoneCount: this.world.zones.size,
      uptime: Date.now() - this.startTime,
    });

    ws.on("message", async (data) => {
      if (!session.checkRateLimit()) {
        session.send({ type: "s_system_message", message: "Rate limited. Slow down.", severity: "warning" });
        return;
      }

      let message: ClientMessage;
      try {
        message = JSON.parse(data.toString()) as ClientMessage;
      } catch {
        session.send({ type: "s_system_message", message: "Invalid message format.", severity: "error" });
        return;
      }

      if (!message.type) {
        session.send({ type: "s_system_message", message: "Missing message type.", severity: "error" });
        return;
      }

      try {
        await this.messageHandler.handle(session, message);
      } catch (err) {
        console.error(`[Net] Error handling message from ${sessionId}:`, err);
        session.send({ type: "s_system_message", message: "Internal server error.", severity: "error" });
      }
    });

    ws.on("close", () => {
      this.handleDisconnect(session);
    });

    ws.on("error", (err) => {
      console.error(`[Net] WebSocket error for ${sessionId}:`, err.message);
    });
  }

  private handleDisconnect(session: Session): void {
    if (session.playerId) {
      const player = this.entities.getPlayer(session.playerId);
      if (player) {
        // Save player state
        this.auth.savePlayer(player);

        // Notify room
        this.broadcastToRoom(player.roomVnum, {
          type: "s_entity_leave",
          entityId: player.id,
          entityName: player.name,
        }, player.id);

        // Remove from world
        this.entities.removePlayer(player.id);
        console.log(`[Game] ${player.name} disconnected`);
      }
    }

    this.sessions.delete(session.id);
    session.state = SessionState.Disconnected;
    console.log(`[Net] Client disconnected: ${session.id} (${this.sessions.size} remaining)`);
  }

  // --- HTTP API ---

  private handleHTTP(req: http.IncomingMessage, res: http.ServerResponse): void {
    const url = new URL(req.url ?? "/", `http://${req.headers.host ?? "localhost"}`);

    // CORS headers for development
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");

    if (req.method === "OPTIONS") {
      res.writeHead(204);
      res.end();
      return;
    }

    switch (url.pathname) {
      case "/api/health": {
        this.sendJson(res, 200, {
          ok: true,
          uptime: Date.now() - this.startTime,
          players: this.entities.playerCount,
          npcs: this.entities.npcCount,
          rooms: this.world.rooms.size,
          zones: this.world.zones.size,
          tickRate: Config.server.tickRate,
          activeFights: this.combat.activeFightCount,
        });
        break;
      }

      case "/api/world": {
        this.sendJson(res, 200, {
          roomCount: this.world.rooms.size,
          zoneCount: this.world.zones.size,
          zones: Array.from(this.world.zones.values()).map((z) => ({
            name: z.name,
            fileName: z.fileName,
            rooms: z.rooms.size,
            mobTemplates: z.mobTemplates.size,
            objTemplates: z.objTemplates.size,
          })),
          startRoom: this.world.startRoom,
        });
        break;
      }

      case "/api/players": {
        const players = Array.from(this.entities.getAllPlayers()).map((p) => p.serializePublic());
        this.sendJson(res, 200, { count: players.length, players });
        break;
      }

      case "/api/room": {
        const vnum = parseInt(url.searchParams.get("vnum") ?? "0", 10);
        const room = this.world.getRoom(vnum);
        if (!room) {
          this.sendJson(res, 404, { error: "Room not found" });
        } else {
          const entities = this.entities.getEntitiesInRoom(vnum).map((e) => e.serialize());
          this.sendJson(res, 200, { room: room.serialize(), entities });
        }
        break;
      }

      case "/api/stats": {
        this.sendJson(res, 200, {
          uptime: Date.now() - this.startTime,
          connections: this.sessions.size,
          players: this.entities.playerCount,
          npcs: this.entities.npcCount,
          entities: this.entities.entityCount,
          rooms: this.world.rooms.size,
          zones: this.world.zones.size,
          tick: this.gameLoop.currentTick,
          activeFights: this.combat.activeFightCount,
          memoryMB: Math.round(process.memoryUsage().heapUsed / 1024 / 1024),
        });
        break;
      }

      default: {
        this.sendJson(res, 200, {
          name: "LOK MMORPG Server",
          version: "1.0.0",
          endpoints: ["/api/health", "/api/world", "/api/players", "/api/room?vnum=3001", "/api/stats"],
          websocket: `ws://${req.headers.host ?? "localhost:9000"}`,
        });
      }
    }
  }

  private sendJson(res: http.ServerResponse, status: number, data: unknown): void {
    res.writeHead(status, { "Content-Type": "application/json; charset=utf-8", "Cache-Control": "no-store" });
    res.end(JSON.stringify(data));
  }

  // --- Game Loop Tick ---

  private onTick(tick: number, deltaMs: number): void {
    const now = Date.now();

    // Combat updates every tick
    this.combat.update(now);

    // NPC AI every 10 ticks (500ms)
    this.npcAICounter += deltaMs;
    if (this.npcAICounter >= 500) {
      this.npcAICounter = 0;
      this.updateNPCAI();
    }

    // Spawns update every 2 seconds
    this.spawns.update(deltaMs * 40, (roomVnum, msg) => this.broadcastToRoom(roomVnum, msg));

    // Regen every ~4 ticks (200ms)
    this.regenCounter += deltaMs;
    if (this.regenCounter >= 5000) {
      this.regenCounter = 0;
      this.regenTick();
    }

    // Auto-save every 5 minutes
    this.saveCounter += deltaMs;
    if (this.saveCounter >= Config.server.saveInterval) {
      this.saveCounter = 0;
      this.autoSave();
    }
  }

  private regenTick(): void {
    for (const player of this.entities.getAllPlayers()) {
      if (!player.isDead && !player.isInCombat) {
        const prevHp = player.combat.hp;
        const prevMana = player.combat.mana;
        player.regen();

        // Only send update if something changed
        if (player.combat.hp !== prevHp || player.combat.mana !== prevMana) {
          const session = this.findSessionByPlayerId(player.id);
          session?.send({ type: "s_player_update", player: player.serialize() });
        }
      }
    }

    for (const npc of this.entities.getAllNPCs()) {
      if (!npc.isDead && !npc.isInCombat) {
        npc.regen();
      }
    }
  }

  private updateNPCAI(): void {
    for (const npc of this.entities.getAllNPCs()) {
      if (npc.isDead || npc.isInCombat) continue;

      // Wandering behavior
      if (npc.behaviors.has("wandering" as any) && !npc.behaviors.has("sentinel" as any)) {
        npc.wanderTimer++;
        if (npc.wanderTimer >= 20) {  // ~10 seconds
          npc.wanderTimer = 0;

          // 30% chance to wander
          if (Math.random() < 0.3) {
            const room = this.world.getRoom(npc.roomVnum);
            if (room) {
              const exits = Array.from(room.exits.entries());
              if (exits.length > 0) {
                const [dir, exit] = exits[Math.floor(Math.random() * exits.length)];
                const targetRoom = this.world.getRoom(exit.targetVnum);

                // Don't wander too far from spawn
                if (targetRoom && !targetRoom.flags.has("no_mob")) {
                  const spawnRoom = this.world.getRoom(npc.spawnRoomVnum);
                  if (spawnRoom?.coord && targetRoom.coord) {
                    const dist = Math.abs(targetRoom.coord.x - spawnRoom.coord.x) +
                                 Math.abs(targetRoom.coord.y - spawnRoom.coord.y) +
                                 Math.abs(targetRoom.coord.z - spawnRoom.coord.z);
                    if (dist <= npc.wanderRadius) {
                      // Notify old room
                      this.broadcastToRoom(npc.roomVnum, {
                        type: "s_entity_leave",
                        entityId: npc.id,
                        entityName: npc.shortDesc,
                        direction: dir,
                      });

                      this.entities.moveEntity(npc.id, npc.roomVnum, exit.targetVnum);

                      // Notify new room
                      this.broadcastToRoom(npc.roomVnum, {
                        type: "s_entity_enter",
                        entity: npc.serialize(),
                        direction: dir,
                      });
                    }
                  }
                }
              }
            }
          }
        }
      }

      // Aggressive behavior - check for players in room
      if (npc.isAggressive && !npc.isInCombat) {
        const players = this.entities.getPlayersInRoom(npc.roomVnum);
        if (players.length > 0) {
          const target = players[Math.floor(Math.random() * players.length)];
          if (!target.isDead) {
            const room = this.world.getRoom(npc.roomVnum);
            if (!room?.flags.has("safe")) {
              this.combat.initiateCombat(npc, target);
            }
          }
        }
      }
    }
  }

  private autoSave(): void {
    let saved = 0;
    for (const player of this.entities.getAllPlayers()) {
      this.auth.savePlayer(player);
      saved++;
    }
    if (saved > 0) {
      console.log(`[Server] Auto-saved ${saved} player(s)`);
    }
  }

  // --- Broadcasting ---

  broadcastToRoom(roomVnum: number, message: ServerMessage, excludeId?: string): void {
    const players = this.entities.getPlayersInRoom(roomVnum);
    for (const player of players) {
      if (player.id === excludeId) continue;
      const session = this.findSessionByPlayerId(player.id);
      session?.send(message);
    }
  }

  broadcastAll(message: ServerMessage, excludeId?: string): void {
    for (const session of this.sessions.values()) {
      if (!session.isInGame) continue;
      if (session.playerId === excludeId) continue;
      session.send(message);
    }
  }

  findSessionByPlayerId(playerId: string): Session | undefined {
    for (const session of this.sessions.values()) {
      if (session.playerId === playerId) return session;
    }
    return undefined;
  }
}
