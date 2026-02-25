import type { WebSocket } from "ws";
import type { ServerMessage } from "./Protocol.js";

export enum SessionState {
  Connected = "connected",
  Authenticated = "authenticated",
  InGame = "in_game",
  Disconnected = "disconnected",
}

export class Session {
  readonly id: string;
  readonly ws: WebSocket;
  readonly connectedAt: number;
  state: SessionState;
  accountId: string | null = null;
  playerId: string | null = null;
  lastPing = Date.now();

  /** Rate limiting */
  private messageCount = 0;
  private messageWindowStart = Date.now();
  private readonly maxMessagesPerSecond = 20;

  constructor(id: string, ws: WebSocket) {
    this.id = id;
    this.ws = ws;
    this.connectedAt = Date.now();
    this.state = SessionState.Connected;
  }

  send(message: ServerMessage): void {
    if (this.ws.readyState === 1) {
      this.ws.send(JSON.stringify(message));
    }
  }

  sendRaw(data: string): void {
    if (this.ws.readyState === 1) {
      this.ws.send(data);
    }
  }

  /** Returns true if the message should be processed, false if rate limited */
  checkRateLimit(): boolean {
    const now = Date.now();
    if (now - this.messageWindowStart > 1000) {
      this.messageCount = 0;
      this.messageWindowStart = now;
    }
    this.messageCount++;
    return this.messageCount <= this.maxMessagesPerSecond;
  }

  get isAuthenticated(): boolean {
    return this.state === SessionState.Authenticated || this.state === SessionState.InGame;
  }

  get isInGame(): boolean {
    return this.state === SessionState.InGame;
  }

  close(code = 1000, reason = ""): void {
    this.state = SessionState.Disconnected;
    try {
      this.ws.close(code, reason);
    } catch {
      // already closed
    }
  }
}
