# LOK 1.1 Three.js Bridge (playerport2)

This folder adds a 3D frontend bridge while keeping the LOK gameplay engine in C.

## What This Includes

- A small C protocol extension in `playerport2/src/act_info.c`:
  - When `initializelokprot75` is active, `look` now emits `lokprotroomvnum<id>`.
- A Node.js bridge:
  - Connects to the existing MUD TCP port.
  - Parses `lokprot` tokens into JSON events.
  - Parses `playerport2/area/*.are` into a room graph for rendering data.
  - Serves a three.js browser client + WebSocket endpoint.
- A browser client:
  - Renders primitive 3D rooms.
  - Animates movement events.
  - Preserves text command/chat workflow.

## Directory Layout

- `threejs-bridge/src/index.js`: HTTP + WebSocket bridge server.
- `threejs-bridge/src/lok-client.js`: TCP client + token parser.
- `threejs-bridge/src/area-parser.js`: `.are` parser and room graph builder.
- `threejs-bridge/public/`: three.js frontend.

## Run Steps

1. Rebuild the MUD server after the C patch.
   - `cd playerport2/src`
   - `make`
2. Start your MUD as usual.
3. Start this bridge.
   - `cd playerport2/threejs-bridge`
   - `npm install`
   - `npm run start`
4. Open `http://127.0.0.1:8080`.
5. Log in from the command field.
6. Click `Enable Protocol 75` once you are in-game.
7. Use `look` or `Refresh Room` if the room is not synced yet.

## Environment Variables

- `BRIDGE_PORT` (default `8080`)
- `MUD_HOST` (default `127.0.0.1`)
- `MUD_PORT` (default `4000`)

Example:

```powershell
$env:MUD_HOST="127.0.0.1"
$env:MUD_PORT="4000"
$env:BRIDGE_PORT="8080"
npm run start
```

## Notes

- This is a foundational bridge, not a full renderer for every gameplay mechanic yet.
- Rooms are currently primitive geometry generated from exit topology.
- If a room vnum is emitted but not found in `area/*.are`, the client will report it as unknown.
