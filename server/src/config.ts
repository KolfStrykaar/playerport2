export const Config = {
  server: {
    port: parseInt(process.env.PORT ?? "9000", 10),
    host: process.env.HOST ?? "0.0.0.0",
    tickRate: 20,               // game ticks per second
    tickInterval: 50,           // ms per tick (1000/20)
    saveInterval: 300_000,      // auto-save every 5 min
    maxPlayers: 500,
  },
  world: {
    areaDir: process.env.AREA_DIR ?? "../area",
    playerDir: process.env.PLAYER_DIR ?? "../player",
    spawnInterval: 60_000,      // NPC respawn check every 60s
    regenTickRate: 4,           // HP/mana regen every 4 ticks (200ms)
  },
  combat: {
    globalCooldown: 1500,       // ms between attacks
    meleeRange: 1,
    xpMultiplier: 1.0,
  },
  auth: {
    tokenExpiry: 86400_000,     // 24h
    minPasswordLength: 6,
    maxNameLength: 16,
  },
} as const;
