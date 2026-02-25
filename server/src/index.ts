import { GameServer } from "./GameServer.js";

const server = new GameServer();

// Graceful shutdown
process.on("SIGINT", () => {
  console.log("\nReceived SIGINT");
  server.stop();
  process.exit(0);
});

process.on("SIGTERM", () => {
  console.log("\nReceived SIGTERM");
  server.stop();
  process.exit(0);
});

process.on("uncaughtException", (err) => {
  console.error("[FATAL] Uncaught exception:", err);
  server.stop();
  process.exit(1);
});

// Start
server.start().catch((err) => {
  console.error("[FATAL] Failed to start server:", err);
  process.exit(1);
});
