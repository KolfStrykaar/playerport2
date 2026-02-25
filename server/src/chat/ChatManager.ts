import type { Player } from "../entities/Player.js";
import type { EntityManager } from "../entities/EntityManager.js";
import type { Session } from "../net/Session.js";
import type { ServerMessage } from "../net/Protocol.js";

export type ChatChannel = "global" | "area" | "party" | "whisper" | "trade" | "clan";

const VALID_CHANNELS = new Set<ChatChannel>(["global", "area", "party", "whisper", "trade", "clan"]);

export class ChatManager {
  private findSession: ((playerId: string) => Session | undefined) | null = null;

  constructor(private entities: EntityManager) {}

  setSessionFinder(finder: (playerId: string) => Session | undefined): void {
    this.findSession = finder;
  }

  handleMessage(sender: Player, channel: string, message: string, targetName?: string): void {
    if (!VALID_CHANNELS.has(channel as ChatChannel)) {
      const session = this.findSession?.(sender.id);
      session?.send({ type: "s_system_message", message: `Invalid channel: ${channel}`, severity: "error" });
      return;
    }

    // Sanitize message
    const sanitized = message.trim().slice(0, 500);
    if (!sanitized) return;

    const chatMsg: ServerMessage = {
      type: "s_chat",
      channel,
      senderName: sender.name,
      message: sanitized,
      timestamp: Date.now(),
    };

    switch (channel as ChatChannel) {
      case "global":
        this.broadcastToAll(chatMsg);
        break;

      case "area":
        this.broadcastToArea(sender, chatMsg);
        break;

      case "whisper":
        this.sendWhisper(sender, targetName ?? "", chatMsg);
        break;

      case "trade":
        this.broadcastToAll(chatMsg);
        break;

      case "clan":
        this.broadcastToClan(sender, chatMsg);
        break;

      default:
        this.broadcastToAll(chatMsg);
    }
  }

  private broadcastToAll(msg: ServerMessage): void {
    for (const player of this.entities.getAllPlayers()) {
      this.findSession?.(player.id)?.send(msg);
    }
  }

  private broadcastToArea(sender: Player, msg: ServerMessage): void {
    // Send to all players in rooms within the same area
    for (const player of this.entities.getAllPlayers()) {
      if (player.roomVnum === sender.roomVnum || this.isSameArea(sender, player)) {
        this.findSession?.(player.id)?.send(msg);
      }
    }
  }

  private sendWhisper(sender: Player, targetName: string, msg: ServerMessage): void {
    const target = this.entities.findPlayerByName(targetName);
    if (!target) {
      this.findSession?.(sender.id)?.send({
        type: "s_system_message",
        message: `Player "${targetName}" not found.`,
        severity: "warning",
      });
      return;
    }

    // Send to target
    this.findSession?.(target.id)?.send(msg);
    // Echo to sender
    this.findSession?.(sender.id)?.send(msg);
  }

  private broadcastToClan(sender: Player, msg: ServerMessage): void {
    if (!sender.clan) {
      this.findSession?.(sender.id)?.send({
        type: "s_system_message",
        message: "You are not in a clan.",
        severity: "warning",
      });
      return;
    }

    for (const player of this.entities.getAllPlayers()) {
      if (player.clan === sender.clan) {
        this.findSession?.(player.id)?.send(msg);
      }
    }
  }

  private isSameArea(a: Player, b: Player): boolean {
    // Simple heuristic: rooms in the same area file
    // In a full implementation, we'd track area assignments
    return Math.abs(a.roomVnum - b.roomVnum) < 100;
  }
}
