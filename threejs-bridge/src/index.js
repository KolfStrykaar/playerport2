import fs from "node:fs";
import http from "node:http";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { WebSocketServer } from "ws";
import { buildWorldFromAreaDirectory, serializeRoom } from "./area-parser.js";
import { LokClient } from "./lok-client.js";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const BRIDGE_PORT = Number.parseInt(process.env.BRIDGE_PORT ?? "6102", 10);
const MUD_HOST = process.env.MUD_HOST ?? "176.9.151.147";
const MUD_PORT = Number.parseInt(process.env.MUD_PORT ?? "6101", 10);
const BASE_ROOM_MAP_RADIUS = 1;
const ROOM_MAP_RADIUS = (() => {
  const configured = Number.parseInt(
    process.env.ROOM_MAP_RADIUS ?? String(BASE_ROOM_MAP_RADIUS + 4),
    10,
  );
  if (Number.isFinite(configured) && configured >= 1) {
    return configured;
  }
  return BASE_ROOM_MAP_RADIUS + 4;
})();

const bridgeRoot = path.resolve(__dirname, "..");
const staticRoot = path.resolve(bridgeRoot, "public");
const vendorRoot = path.resolve(bridgeRoot, "node_modules", "three", "build");
const areaRoot = path.resolve(bridgeRoot, "..", "area");

const world = buildWorldFromAreaDirectory(areaRoot);

function sendJson(response, statusCode, payload) {
  response.writeHead(statusCode, {
    "Content-Type": "application/json; charset=utf-8",
    "Cache-Control": "no-store",
  });
  response.end(JSON.stringify(payload));
}

function mimeType(filePath) {
  const extension = path.extname(filePath).toLowerCase();
  switch (extension) {
    case ".html":
      return "text/html; charset=utf-8";
    case ".css":
      return "text/css; charset=utf-8";
    case ".js":
      return "application/javascript; charset=utf-8";
    case ".json":
      return "application/json; charset=utf-8";
    case ".svg":
      return "image/svg+xml";
    case ".png":
      return "image/png";
    default:
      return "text/plain; charset=utf-8";
  }
}

function serveFile(response, filePath) {
  fs.readFile(filePath, (error, data) => {
    if (error) {
      if (error.code === "ENOENT") {
        response.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
        response.end("Not found");
        return;
      }

      response.writeHead(500, { "Content-Type": "text/plain; charset=utf-8" });
      response.end("Server error");
      return;
    }

    response.writeHead(200, {
      "Content-Type": mimeType(filePath),
      "Cache-Control": "no-store",
    });
    response.end(data);
  });
}

function resolveStaticPath(rootDir, requestPath, fallbackFile = "index.html") {
  const normalized = requestPath === "/" ? `/${fallbackFile}` : requestPath;
  const absolute = path.resolve(rootDir, `.${normalized}`);
  if (!absolute.startsWith(rootDir)) {
    return null;
  }
  return absolute;
}

const server = http.createServer((request, response) => {
  if (!request.url) {
    response.writeHead(400, { "Content-Type": "text/plain; charset=utf-8" });
    response.end("Bad request");
    return;
  }

  const url = new URL(request.url, `http://${request.headers.host || "174.101.0.43"}`);

  if (url.pathname === "/api/health") {
    sendJson(response, 200, {
      ok: true,
      mudHost: MUD_HOST,
      mudPort: MUD_PORT,
      roomCount: world.roomCount,
      roomMapRadius: ROOM_MAP_RADIUS,
    });
    return;
  }

  if (url.pathname === "/api/world-meta") {
    sendJson(response, 200, {
      generatedAt: world.generatedAt,
      areaDir: world.areaDir,
      roomCount: world.roomCount,
      roomMapRadius: ROOM_MAP_RADIUS,
    });
    return;
  }

  if (url.pathname.startsWith("/vendor/")) {
    const vendorPath = url.pathname.replace("/vendor/", "/");
    const filePath = resolveStaticPath(vendorRoot, vendorPath);
    if (!filePath) {
      response.writeHead(403, { "Content-Type": "text/plain; charset=utf-8" });
      response.end("Forbidden");
      return;
    }
    serveFile(response, filePath);
    return;
  }

  const filePath = resolveStaticPath(staticRoot, url.pathname);
  if (!filePath) {
    response.writeHead(403, { "Content-Type": "text/plain; charset=utf-8" });
    response.end("Forbidden");
    return;
  }

  serveFile(response, filePath);
});

const wss = new WebSocketServer({ server });

const DIRECTIONS = new Set(["north", "east", "south", "west", "up", "down"]);

function buildRoomMap(centerVnum, radius = ROOM_MAP_RADIUS) {
  const centerRoom = world.rooms[String(centerVnum)];
  if (!centerRoom) {
    return null;
  }

  const includedRooms = {};
  const queue = [{ room: centerRoom, depth: 0 }];
  const seen = new Set([String(centerRoom.vnum)]);

  while (queue.length > 0) {
    const entry = queue.shift();
    includedRooms[String(entry.room.vnum)] = entry.room;

    if (entry.depth >= radius) {
      continue;
    }

    for (const targetVnum of Object.values(entry.room.exits || {})) {
      const nextRoom = world.rooms[String(targetVnum)];
      if (!nextRoom) {
        continue;
      }

      const key = String(nextRoom.vnum);
      if (seen.has(key)) {
        continue;
      }

      seen.add(key);
      queue.push({ room: nextRoom, depth: entry.depth + 1 });
    }
  }

  const serializedRooms = Object.values(includedRooms)
    .map((room) => serializeRoom(room))
    .sort((a, b) => a.vnum - b.vnum);

  const links = [];
  const seenLinks = new Set();

  for (const room of Object.values(includedRooms)) {
    for (const [direction, targetVnum] of Object.entries(room.exits || {})) {
      if (!includedRooms[String(targetVnum)]) {
        continue;
      }

      const low = room.vnum < targetVnum ? room.vnum : targetVnum;
      const high = room.vnum < targetVnum ? targetVnum : room.vnum;
      const linkKey = `${low}:${high}`;

      if (seenLinks.has(linkKey)) {
        continue;
      }

      seenLinks.add(linkKey);
      links.push({
        from: room.vnum,
        to: targetVnum,
        direction,
      });
    }
  }

  return {
    centerVnum: centerRoom.vnum,
    radius,
    rooms: serializedRooms,
    links,
  };
}

wss.on("connection", (socket) => {
  const mud = new LokClient({ host: MUD_HOST, port: MUD_PORT });

  const send = (payload) => {
    if (socket.readyState === 1) {
      socket.send(JSON.stringify(payload));
    }
  };

  send({
    type: "server_info",
    message: `Bridge connected. Target MUD socket is ${MUD_HOST}:${MUD_PORT}.`,
  });

  send({
    type: "world_meta",
    generatedAt: world.generatedAt,
    roomCount: world.roomCount,
    roomMapRadius: ROOM_MAP_RADIUS,
  });

  mud.on("connect", () => {
    send({ type: "server_info", message: "Connected to MUD." });
  });

  mud.on("close", () => {
    send({ type: "server_info", message: "MUD connection closed." });
  });

  mud.on("error", (error) => {
    send({ type: "server_error", message: error.message });
  });

  mud.on("text", (text) => {
    send({ type: "mud_text", text });
  });

  mud.on("stats", (stats) => {
    send({ type: "stats_update", ...stats });
  });

  mud.on("move", ({ direction }) => {
    send({
      type: "player_move",
      direction,
      duration: 260,
    });
  });

  mud.on("room", ({ vnum }) => {
    const room = world.rooms[String(vnum)];
    send({
      type: "room_change",
      roomId: vnum,
      room: room ? serializeRoom(room) : null,
      roomMap: room ? buildRoomMap(room.vnum) : null,
    });

    if (!room) {
      send({
        type: "server_error",
        message: `Room ${vnum} was not found in parsed area files.`,
      });
    }
  });

  mud.connect();

  socket.on("message", (data) => {
    const raw = data.toString();
    let payload;

    try {
      payload = JSON.parse(raw);
    } catch {
      mud.sendCommand(raw);
      return;
    }

    switch (payload.type) {
      case "mud_command": {
        if (typeof payload.command === "string") {
          mud.sendCommand(payload.command);
        }
        break;
      }
      case "move": {
        if (typeof payload.direction === "string" && DIRECTIONS.has(payload.direction)) {
          mud.sendCommand(payload.direction);
        }
        break;
      }
      case "enable_protocol": {
        mud.sendCommand("initializelokprot75");
        mud.sendCommand("look");
        break;
      }
      case "refresh_room": {
        mud.sendCommand("look");
        break;
      }
      case "ping": {
        send({ type: "pong" });
        break;
      }
      default: {
        send({ type: "server_error", message: `Unknown command type: ${payload.type}` });
      }
    }
  });

  socket.on("close", () => {
    mud.close();
  });
});

server.listen(BRIDGE_PORT, () => {
  process.stdout.write(
    [
      `threejs-bridge listening on http://127.0.0.1:${BRIDGE_PORT}`,
      `MUD target: ${MUD_HOST}:${MUD_PORT}`,
      `Parsed rooms: ${world.roomCount}`,
      `Map radius: ${ROOM_MAP_RADIUS}`,
      "",
    ].join("\n"),
  );
});
