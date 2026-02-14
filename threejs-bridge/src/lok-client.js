import net from "node:net";
import { EventEmitter } from "node:events";

const DIRECTION_PATTERN = /lokprotmoved(north|east|south|west|up|down)/g;
const ROOM_PATTERN = /lokprotroomvnum(\d+)/g;
const STATS_PATTERN =
  /locprothp\/mana\/move(\d{1,3})locprothp\/mana\/move(\d{1,3})locprothp\/mana\/move(\d{1,3})/g;

const ANSI_PATTERN = /\x1b\[[0-9;?]*[ -/]*[@-~]/g;

function stripAnsi(text) {
  return text.replace(ANSI_PATTERN, "");
}

function decodeTelnetChunk(buffer, socket) {
  const out = [];

  for (let index = 0; index < buffer.length; index += 1) {
    const byte = buffer[index];

    if (byte !== 255) {
      out.push(byte);
      continue;
    }

    const command = buffer[index + 1];
    if (command === undefined) {
      break;
    }
    index += 1;

    if (command === 255) {
      out.push(255);
      continue;
    }

    if (command === 251 || command === 252 || command === 253 || command === 254) {
      const option = buffer[index + 1];
      if (option === undefined) {
        break;
      }
      index += 1;

      const response = command === 251 || command === 252 ? 254 : 252;
      if (socket && !socket.destroyed) {
        socket.write(Buffer.from([255, response, option]));
      }
      continue;
    }

    if (command === 250) {
      while (index < buffer.length - 1) {
        index += 1;
        if (buffer[index] === 255 && buffer[index + 1] === 240) {
          index += 1;
          break;
        }
      }
      continue;
    }
  }

  return Buffer.from(out).toString("utf8");
}

export class LokClient extends EventEmitter {
  constructor({ host, port }) {
    super();
    this.host = host;
    this.port = port;
    this.socket = null;
    this.tail = "";
    this.flushTimer = null;
  }

  connect() {
    if (this.socket) {
      return;
    }

    this.socket = net.createConnection({ host: this.host, port: this.port }, () => {
      this.emit("connect");
    });

    this.socket.on("data", (chunk) => {
      const text = decodeTelnetChunk(chunk, this.socket);
      if (!text) {
        return;
      }

      const combined = this.tail + text;
      if (combined.length <= 128) {
        this.tail = combined;
      } else {
        const processText = combined.slice(0, -128);
        this.tail = combined.slice(-128);
        this.parseAndEmit(processText);
      }

      if (this.flushTimer) {
        clearTimeout(this.flushTimer);
      }
      this.flushTimer = setTimeout(() => {
        if (this.tail.length > 0) {
          this.parseAndEmit(this.tail);
          this.tail = "";
        }
      }, 35);
    });

    this.socket.on("error", (error) => {
      this.emit("error", error);
    });

    this.socket.on("close", () => {
      if (this.flushTimer) {
        clearTimeout(this.flushTimer);
        this.flushTimer = null;
      }
      if (this.tail.length > 0) {
        this.parseAndEmit(this.tail);
        this.tail = "";
      }
      this.emit("close");
      this.socket = null;
    });
  }

  parseAndEmit(rawText) {
    let visibleText = rawText;

    visibleText = visibleText.replace(STATS_PATTERN, (_match, hp, mana, move) => {
      this.emit("stats", {
        hpPct: Number.parseInt(hp, 10),
        manaPct: Number.parseInt(mana, 10),
        movePct: Number.parseInt(move, 10),
      });
      return "";
    });

    visibleText = visibleText.replace(DIRECTION_PATTERN, (_match, direction) => {
      this.emit("move", { direction });
      return "";
    });

    visibleText = visibleText.replace(ROOM_PATTERN, (_match, vnum) => {
      this.emit("room", { vnum: Number.parseInt(vnum, 10) });
      return "";
    });

    visibleText = stripAnsi(visibleText).replace(/\0/g, "");
    if (visibleText.length > 0) {
      this.emit("text", visibleText);
    }
  }

  sendCommand(command) {
    if (!this.socket || this.socket.destroyed) {
      return;
    }

    const trimmed = command.replace(/[\r\n]+/g, "").trim();
    if (!trimmed) {
      return;
    }

    this.socket.write(`${trimmed}\n`);
  }

  close() {
    if (!this.socket) {
      return;
    }
    if (this.flushTimer) {
      clearTimeout(this.flushTimer);
      this.flushTimer = null;
    }
    this.socket.end();
    this.socket.destroy();
    this.socket = null;
  }
}
