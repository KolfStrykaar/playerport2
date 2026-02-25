import { Config } from "./config.js";

export type TickCallback = (tick: number, deltaMs: number) => void;

export class GameLoop {
  private running = false;
  private tick = 0;
  private timer: ReturnType<typeof setInterval> | null = null;
  private callbacks: TickCallback[] = [];
  private lastTime = 0;

  onTick(callback: TickCallback): void {
    this.callbacks.push(callback);
  }

  start(): void {
    if (this.running) return;
    this.running = true;
    this.lastTime = Date.now();

    console.log(`[GameLoop] Started at ${Config.server.tickRate} ticks/sec (${Config.server.tickInterval}ms interval)`);

    this.timer = setInterval(() => {
      const now = Date.now();
      const delta = now - this.lastTime;
      this.lastTime = now;
      this.tick++;

      for (const cb of this.callbacks) {
        try {
          cb(this.tick, delta);
        } catch (err) {
          console.error("[GameLoop] Tick error:", err);
        }
      }
    }, Config.server.tickInterval);
  }

  stop(): void {
    if (!this.running) return;
    this.running = false;
    if (this.timer) {
      clearInterval(this.timer);
      this.timer = null;
    }
    console.log("[GameLoop] Stopped");
  }

  get currentTick(): number {
    return this.tick;
  }

  get isRunning(): boolean {
    return this.running;
  }
}
