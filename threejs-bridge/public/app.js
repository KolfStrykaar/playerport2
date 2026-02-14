import * as THREE from "/vendor/three.module.js";

const ROOM_SCALE = 3;
const ROOM_WIDTH = 18 * ROOM_SCALE;
const ROOM_DEPTH = 18 * ROOM_SCALE;
const ROOM_HEIGHT = 8 * ROOM_SCALE;
const WALL_THICKNESS = 0.44 * ROOM_SCALE;
const FLOOR_THICKNESS = 0.42 * ROOM_SCALE;
const EXIT_WIDTH = 4.8 * ROOM_SCALE;
const EXIT_HEIGHT = 4.2 * ROOM_SCALE;

const CAMERA_BASE_HEIGHT = 2.6 * ROOM_SCALE;
const CAMERA_MIN_DISTANCE = 24;
const CAMERA_MAX_DISTANCE = 96;
const CAMERA_DAMPING = 9;
const CAMERA_ROTATE_SPEED = 0.0062;
const CAMERA_ZOOM_SPEED = 0.015;
const WALK_SPEED = 17;
const SPRINT_MULTIPLIER = 1.5;
const TURN_SPEED = 9;
const CHARACTER_RADIUS = 1.2;
const EXIT_TRIGGER_DEPTH = 2.8 * ROOM_SCALE;
const NPC_MAX_COUNT = 14;
const NPC_GUARD_KEYWORDS = /guard|watch|sentry|gate|checkpoint|barracks/i;
const BUILDING_INTERIOR_KEYWORDS =
  /shop|store|tavern|inn|bar|temple|church|chapel|shrine|sanctuary|guild|hall|library|bank|house|cottage|hut|tower|keep|palace|castle|forge|smithy|academy|school|monastery|cathedral/i;
const ROOM_TILE_SIZE = 4.6;
const ROOM_TILE_HEIGHT = 0.9;
const ROOM_SPACING = 8;
const LEVEL_HEIGHT = 3;
const MAP_CAMERA_HALF_HEIGHT = 26;
const MAP_TRANSITION_SPEED = 11;
const MARKER_FOLLOW_SPEED = 14;
const MOB_INDICATOR_LIMIT = 6;

const EXIT_ORDER = ["north", "east", "south", "west", "up", "down"];
const DIRECTION_LABELS = {
  north: "North",
  east: "East",
  south: "South",
  west: "West",
  up: "Up",
  down: "Down",
};

const DIR_OFFSETS = {
  north: { x: 0, y: 0, z: -1 },
  east: { x: 1, y: 0, z: 0 },
  south: { x: 0, y: 0, z: 1 },
  west: { x: -1, y: 0, z: 0 },
  up: { x: 0, y: 1, z: 0 },
  down: { x: 0, y: -1, z: 0 },
};

const SECTOR_NAMES = {
  0: "Inside",
  1: "City",
  2: "Field",
  3: "Forest",
  4: "Hills",
  5: "Mountain",
  6: "Water (Swim)",
  7: "Water (No Swim)",
  9: "Air",
  10: "Desert",
  11: "Graveyard",
  12: "Polar",
  13: "Swamp",
  14: "Transport",
  15: "Underwater",
  16: "Knights of Takhisis",
  17: "Solamnic",
  18: "Conclave",
  19: "Aesthetic",
  20: "Thieves",
  21: "Holy Order",
  22: "Mercenary",
  23: "Undead",
  24: "Forester",
  25: "Artisans",
  26: "Black Order",
  27: "Corruption",
  28: "Death",
  29: "Blessing",
  30: "Knowledge",
  31: "Virtue",
  32: "Chivalry",
  33: "Wealth",
  34: "Wisdom",
  35: "Beauty",
  36: "Disease",
  37: "Nature",
  38: "Flame",
  39: "Destruction",
  40: "Justice",
  41: "Life",
  42: "Hatred",
  43: "Storm",
  44: "Battle",
  45: "Red Magic",
  46: "Black Magic",
  47: "White Magic",
};

const STYLE_PRESETS = {
  stone: {
    background: 0x101726,
    fog: 0x101726,
    floor: 0x3b322d,
    wall: 0x4f453d,
    ceiling: 0x273141,
    prop: 0x5b4f45,
    accent: 0xffb46a,
    portal: 0x6ebdf4,
    portalEmissive: 0x183951,
    hemiSky: 0xaac5e4,
    hemiGround: 0x181613,
    hemiIntensity: 0.75,
    sunColor: 0xffedcf,
    sunIntensity: 0.82,
  },
  city: {
    background: 0x171e2b,
    fog: 0x171e2b,
    floor: 0x4f5258,
    wall: 0x636972,
    ceiling: 0x2f3947,
    prop: 0x7f8793,
    accent: 0xf6c47d,
    portal: 0x7ec2ff,
    portalEmissive: 0x1a3f5f,
    hemiSky: 0xb4c5dd,
    hemiGround: 0x1e232e,
    hemiIntensity: 0.76,
    sunColor: 0xffe4c0,
    sunIntensity: 0.86,
  },
  field: {
    background: 0x102114,
    fog: 0x102114,
    floor: 0x314a2d,
    wall: 0x3e5d33,
    ceiling: 0x1d2f22,
    prop: 0x5a7a4c,
    accent: 0xe9bf65,
    portal: 0x7bd4f0,
    portalEmissive: 0x18454f,
    hemiSky: 0x9ac8a5,
    hemiGround: 0x192018,
    hemiIntensity: 0.78,
    sunColor: 0xffefc9,
    sunIntensity: 0.88,
  },
  forest: {
    background: 0x0d1d18,
    fog: 0x0d1d18,
    floor: 0x2e4a36,
    wall: 0x355640,
    ceiling: 0x1b2f26,
    prop: 0x4d6c4f,
    accent: 0xd5b46e,
    portal: 0x80c8f2,
    portalEmissive: 0x17435a,
    hemiSky: 0x96bea6,
    hemiGround: 0x171e16,
    hemiIntensity: 0.74,
    sunColor: 0xffdfbf,
    sunIntensity: 0.8,
  },
  mountain: {
    background: 0x121a1f,
    fog: 0x121a1f,
    floor: 0x434c54,
    wall: 0x56616b,
    ceiling: 0x2f3b43,
    prop: 0x6c7783,
    accent: 0xe2c181,
    portal: 0x8bb9ed,
    portalEmissive: 0x1e3a58,
    hemiSky: 0xa5bdd0,
    hemiGround: 0x1a2128,
    hemiIntensity: 0.75,
    sunColor: 0xffe5cb,
    sunIntensity: 0.82,
  },
  water: {
    background: 0x0f2033,
    fog: 0x0f2033,
    floor: 0x244b66,
    wall: 0x30607a,
    ceiling: 0x1a3d54,
    prop: 0x4f7a9b,
    accent: 0xffca72,
    portal: 0x84d7ff,
    portalEmissive: 0x195072,
    hemiSky: 0x95c6e8,
    hemiGround: 0x102136,
    hemiIntensity: 0.78,
    sunColor: 0xeed8c5,
    sunIntensity: 0.74,
  },
  swamp: {
    background: 0x151d12,
    fog: 0x151d12,
    floor: 0x3b4b2a,
    wall: 0x455a34,
    ceiling: 0x262f1d,
    prop: 0x637748,
    accent: 0xe3be67,
    portal: 0x83cfc8,
    portalEmissive: 0x1b4b46,
    hemiSky: 0xb2c98a,
    hemiGround: 0x221f14,
    hemiIntensity: 0.74,
    sunColor: 0xffddb6,
    sunIntensity: 0.7,
  },
  desert: {
    background: 0x2a1b13,
    fog: 0x2a1b13,
    floor: 0x8a6841,
    wall: 0xa07a4d,
    ceiling: 0x60422b,
    prop: 0xb89062,
    accent: 0xf6d38f,
    portal: 0x91d0ef,
    portalEmissive: 0x1f4d67,
    hemiSky: 0xd8bf95,
    hemiGround: 0x2a2116,
    hemiIntensity: 0.8,
    sunColor: 0xfff0ce,
    sunIntensity: 0.94,
  },
  polar: {
    background: 0x0f1a23,
    fog: 0x0f1a23,
    floor: 0x5b7086,
    wall: 0x7590a8,
    ceiling: 0x3d5568,
    prop: 0x9db5c7,
    accent: 0xd8e6f3,
    portal: 0x9ce1ff,
    portalEmissive: 0x27566f,
    hemiSky: 0xc8e2f9,
    hemiGround: 0x1d2934,
    hemiIntensity: 0.83,
    sunColor: 0xf9f8ff,
    sunIntensity: 0.95,
  },
  graveyard: {
    background: 0x120f1c,
    fog: 0x120f1c,
    floor: 0x3f3a4d,
    wall: 0x4f4b63,
    ceiling: 0x2b2a3c,
    prop: 0x696480,
    accent: 0xd2b6ff,
    portal: 0x8fb9ff,
    portalEmissive: 0x2a315f,
    hemiSky: 0x9aa1d1,
    hemiGround: 0x1d1726,
    hemiIntensity: 0.72,
    sunColor: 0xe6d9ff,
    sunIntensity: 0.66,
  },
  arcane: {
    background: 0x120f2a,
    fog: 0x120f2a,
    floor: 0x3c3472,
    wall: 0x4c4490,
    ceiling: 0x28205d,
    prop: 0x6e5db7,
    accent: 0x9de0ff,
    portal: 0x74d0ff,
    portalEmissive: 0x1e4372,
    hemiSky: 0xa5b0ff,
    hemiGround: 0x1a1538,
    hemiIntensity: 0.8,
    sunColor: 0xd7d9ff,
    sunIntensity: 0.75,
  },
  inferno: {
    background: 0x2a100c,
    fog: 0x2a100c,
    floor: 0x623327,
    wall: 0x7a3c2e,
    ceiling: 0x432118,
    prop: 0xac5635,
    accent: 0xff8b57,
    portal: 0xffb26d,
    portalEmissive: 0x6c2d15,
    hemiSky: 0xf18f66,
    hemiGround: 0x2d170e,
    hemiIntensity: 0.76,
    sunColor: 0xffc48f,
    sunIntensity: 0.9,
  },
  storm: {
    background: 0x101723,
    fog: 0x101723,
    floor: 0x3a4659,
    wall: 0x4b5b73,
    ceiling: 0x273347,
    prop: 0x607690,
    accent: 0x99d7ff,
    portal: 0x88d3ff,
    portalEmissive: 0x204667,
    hemiSky: 0xa6c7e2,
    hemiGround: 0x1b232f,
    hemiIntensity: 0.8,
    sunColor: 0xecf4ff,
    sunIntensity: 0.85,
  },
};

const SECTOR_STYLE_HINT = {
  0: "stone",
  1: "city",
  2: "field",
  3: "forest",
  4: "field",
  5: "mountain",
  6: "water",
  7: "water",
  9: "storm",
  10: "desert",
  11: "graveyard",
  12: "polar",
  13: "swamp",
  15: "water",
  16: "inferno",
  17: "stone",
  18: "arcane",
  20: "city",
  21: "stone",
  23: "graveyard",
  24: "forest",
  25: "city",
  26: "inferno",
  27: "inferno",
  28: "graveyard",
  29: "field",
  30: "arcane",
  31: "field",
  32: "stone",
  33: "city",
  34: "arcane",
  35: "field",
  36: "swamp",
  37: "forest",
  38: "inferno",
  39: "inferno",
  40: "stone",
  41: "field",
  42: "graveyard",
  43: "storm",
  44: "stone",
  45: "arcane",
  46: "arcane",
  47: "arcane",
};
const KEYWORD_STYLE_HINTS = [
  {
    style: "water",
    keywords: ["water", "river", "ocean", "sea", "lake", "dock", "harbor", "shore", "underwater"],
  },
  {
    style: "swamp",
    keywords: ["swamp", "bog", "marsh", "mire", "mud", "rot", "moss"],
  },
  {
    style: "forest",
    keywords: ["forest", "tree", "grove", "wood", "jungle", "leaf", "vine", "thicket"],
  },
  {
    style: "mountain",
    keywords: ["mountain", "cave", "rock", "stone", "cliff", "tunnel", "ridge", "grotto"],
  },
  {
    style: "desert",
    keywords: ["desert", "sand", "dune", "arid", "dust", "sun-scorched", "wastes"],
  },
  {
    style: "polar",
    keywords: ["ice", "frost", "snow", "glacier", "frozen", "blizzard"],
  },
  {
    style: "graveyard",
    keywords: ["grave", "crypt", "tomb", "bones", "undead", "sepulch", "mausoleum", "cemetery"],
  },
  {
    style: "arcane",
    keywords: ["arcane", "rune", "magic", "sigil", "conclave", "mystic", "enchant", "sorcery"],
  },
  {
    style: "inferno",
    keywords: ["fire", "flame", "lava", "ember", "burn", "inferno", "smoke", "ash"],
  },
  {
    style: "storm",
    keywords: ["storm", "thunder", "lightning", "tempest", "wind", "rain", "squall"],
  },
  {
    style: "city",
    keywords: ["street", "market", "town", "city", "alley", "shop", "inn", "cobbled"],
  },
];

const sceneCanvas = document.getElementById("scene");
const minimapCanvas = document.getElementById("minimap-scene");
const connStatus = document.getElementById("conn-status");
const roomName = document.getElementById("room-name");
const roomArea = document.getElementById("room-area");
const roomSector = document.getElementById("room-sector");
const mudLog = document.getElementById("mud-log");
const commandForm = document.getElementById("command-form");
const commandInput = document.getElementById("command-input");
const protocolButton = document.getElementById("enable-protocol");
const refreshButton = document.getElementById("refresh-room");
const infoPanel = document.getElementById("info-panel");
const infoPanelTitle = document.getElementById("info-panel-title");
const infoPanelSubtitle = document.getElementById("info-panel-subtitle");
const infoPanelContent = document.getElementById("info-panel-content");
const infoPanelRefresh = document.getElementById("info-refresh");
const infoPanelClose = document.getElementById("info-close");
const panelOpenButtons = Array.from(document.querySelectorAll("[data-panel-id]"));
const hpFill = document.getElementById("hp-fill");
const hpText = document.getElementById("hp-text");
const manaFill = document.getElementById("mana-fill");
const manaText = document.getElementById("mana-text");
const moveFill = document.getElementById("move-fill");
const moveText = document.getElementById("move-text");

const moveButtonsByDirection = new Map();
for (const button of document.querySelectorAll("[data-direction]")) {
  moveButtonsByDirection.set(button.dataset.direction, button);
}

const scene = new THREE.Scene();
scene.background = new THREE.Color(0x101726);
scene.fog = new THREE.Fog(0x101726, 40, 220);

const camera = new THREE.PerspectiveCamera(68, window.innerWidth / window.innerHeight, 0.1, 250);
camera.position.set(0, CAMERA_BASE_HEIGHT + 18, 56);
camera.lookAt(0, CAMERA_BASE_HEIGHT, 0);

const renderer = new THREE.WebGLRenderer({
  canvas: sceneCanvas,
  antialias: true,
});
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
renderer.setSize(window.innerWidth, window.innerHeight);

const hemiLight = new THREE.HemisphereLight(0xaec9e8, 0x181613, 0.75);
scene.add(hemiLight);

const dirLight = new THREE.DirectionalLight(0xffeecf, 0.82);
dirLight.position.set(13, 20, 11);
scene.add(dirLight);

const accentLight = new THREE.PointLight(0xffba72, 0.4, 48, 2);
accentLight.position.set(0, ROOM_HEIGHT - 1.4, 0);
scene.add(accentLight);

const roomGroup = new THREE.Group();
scene.add(roomGroup);

const minimapScene = new THREE.Scene();
minimapScene.background = new THREE.Color(0x101725);
minimapScene.fog = new THREE.Fog(0x101725, 55, 180);

const minimapCamera = new THREE.OrthographicCamera(-1, 1, 1, -1, 0.1, 300);
minimapCamera.position.set(0, 48, 18);
minimapCamera.lookAt(0, 0, 0);

const minimapRenderer = new THREE.WebGLRenderer({
  canvas: minimapCanvas,
  antialias: true,
  alpha: true,
});
minimapRenderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));

const minimapHemiLight = new THREE.HemisphereLight(0xa2c2ff, 0x0f1621, 0.7);
minimapScene.add(minimapHemiLight);

const minimapDirLight = new THREE.DirectionalLight(0xfff2db, 0.85);
minimapDirLight.position.set(18, 30, 12);
minimapScene.add(minimapDirLight);

const minimapGrid = new THREE.GridHelper(260, 52, 0x233c59, 0x172334);
minimapGrid.position.y = -0.05;
minimapScene.add(minimapGrid);

const minimapGroup = new THREE.Group();
minimapScene.add(minimapGroup);

const minimapMarker = new THREE.Mesh(
  new THREE.RingGeometry(1.7, 2.6, 40),
  new THREE.MeshBasicMaterial({
    color: 0xffbf61,
    transparent: true,
    opacity: 0.85,
    side: THREE.DoubleSide,
  }),
);
minimapMarker.rotation.x = -Math.PI / 2;
minimapMarker.visible = false;
minimapScene.add(minimapMarker);

const orbitState = {
  yaw: Math.PI * 0.2,
  pitch: 0.35,
  distance: 56,
  desiredDistance: 56,
  dragging: false,
  pointerId: -1,
  lastX: 0,
  lastY: 0,
};

const orbitTarget = new THREE.Vector3(0, CAMERA_BASE_HEIGHT, 0);
const orbitCurrentTarget = orbitTarget.clone();
const desiredCameraPosition = new THREE.Vector3();
const forwardVector = new THREE.Vector3();
const rightVector = new THREE.Vector3();

let currentRoom = null;
let movePulse = 0;
let frameClock = performance.now();
let minimapPreviousCenterCoord = null;
const minimapMarkerTargetPosition = new THREE.Vector3();
const minimapMarkerCurrentPosition = new THREE.Vector3();
const playerState = {
  position: new THREE.Vector3(0, FLOOR_THICKNESS / 2, 0),
  velocity: new THREE.Vector3(),
  heading: 0,
  lastExitAt: 0,
};
const inputState = {
  forward: false,
  backward: false,
  left: false,
  right: false,
  sprint: false,
};
const actorState = {
  playerRig: null,
  npcs: [],
};
let activeWalkableZones = [];
let activeExitTriggers = [];
let activeInfoPanelId = null;
let panelCapture = null;
let isImmortal = false;
const panelTextById = Object.create(null);

const PANEL_CONFIG = {
  room_description: {
    title: "Room Description",
    subtitle: "Current room details from backend parser",
    command: null,
  },
  who_list: {
    title: "Who List",
    subtitle: "Command: who",
    command: "who",
  },
  inventory: {
    title: "Inventory",
    subtitle: "Command: inventory",
    command: "inventory",
  },
  channels: {
    title: "Channels",
    subtitle: "Command: channels",
    command: "channels",
  },
  group: {
    title: "Group",
    subtitle: "Command: group",
    command: "group",
  },
  character_sheet: {
    title: "Character Sheet",
    subtitle: "Command: score",
    command: "score",
  },
  skills: {
    title: "Skills",
    subtitle: "Command: skills",
    command: "skills",
  },
  spells: {
    title: "Spells",
    subtitle: "Command: spells",
    command: "spells",
  },
  training: {
    title: "Training",
    subtitle: "Command: train",
    command: "train",
  },
  wiz: {
    title: "Wiz Commands",
    subtitle: "Immortal only | Command: wizhelp",
    command: "wizhelp",
    immortalOnly: true,
  },
};

function clamp(value, minValue, maxValue) {
  return Math.max(minValue, Math.min(maxValue, value));
}

function clampPct(value) {
  return clamp(value, 0, 100);
}

function toTitleCase(text) {
  if (!text) {
    return "";
  }
  return text.charAt(0).toUpperCase() + text.slice(1);
}

function normalizeAngle(angle) {
  let out = angle;
  while (out > Math.PI) {
    out -= Math.PI * 2;
  }
  while (out < -Math.PI) {
    out += Math.PI * 2;
  }
  return out;
}

function lerpAngle(current, target, t) {
  const delta = normalizeAngle(target - current);
  return current + delta * t;
}

function updateMeter(fillEl, textEl, value) {
  const pct = clampPct(value);
  fillEl.style.width = `${pct}%`;
  textEl.textContent = `${pct}%`;
}

function appendLog(text) {
  const normalized = text.replace(/\r/g, "");
  mudLog.textContent += normalized;
  if (mudLog.textContent.length > 25000) {
    mudLog.textContent = mudLog.textContent.slice(-18000);
  }
  mudLog.scrollTop = mudLog.scrollHeight;
}

function setConnectionLabel(text, color = "var(--accent)") {
  connStatus.textContent = text;
  connStatus.style.color = color;
}
function clearGroup(group) {
  while (group.children.length > 0) {
    const child = group.children[0];
    group.remove(child);

    if (child.geometry) {
      child.geometry.dispose();
    }

    if (child.material) {
      if (Array.isArray(child.material)) {
        for (const material of child.material) {
          material.dispose();
        }
      } else {
        child.material.dispose();
      }
    }
  }
}

function updateMinimapProjection() {
  const rect = minimapCanvas.getBoundingClientRect();
  const width = Math.max(1, Math.floor(rect.width));
  const height = Math.max(1, Math.floor(rect.height));
  minimapRenderer.setSize(width, height, false);

  const aspect = width / height;
  minimapCamera.left = -MAP_CAMERA_HALF_HEIGHT * aspect;
  minimapCamera.right = MAP_CAMERA_HALF_HEIGHT * aspect;
  minimapCamera.top = MAP_CAMERA_HALF_HEIGHT;
  minimapCamera.bottom = -MAP_CAMERA_HALF_HEIGHT;
  minimapCamera.updateProjectionMatrix();
}

function setImmortalState(nextState) {
  isImmortal = Boolean(nextState);
  for (const button of panelOpenButtons) {
    if (button.dataset.panelId === "wiz") {
      button.classList.toggle("immortal-off", !isImmortal);
      button.title = isImmortal
        ? "Wiz commands panel"
        : "Immortal only. Unlocks automatically when immortal output is detected.";
    }
  }
}

function buildRoomDescriptionPanelText(room) {
  if (!room) {
    return "Room data is not synchronized yet. Use protocol 75 and look/refresh.";
  }

  const exits = [];
  for (const direction of EXIT_ORDER) {
    const targetVnum = Number.parseInt(room.exits?.[direction] ?? "", 10);
    if (Number.isFinite(targetVnum) && targetVnum > 0) {
      exits.push(`${DIRECTION_LABELS[direction]} -> #${targetVnum}`);
    }
  }

  return [
    `${room.name} (#${room.vnum})`,
    `Area: ${room.area || room.areaFile || "Unknown"}`,
    `Sector: ${resolveSectorName(room.sector)}`,
    "",
    room.description || "(No room description found)",
    "",
    exits.length > 0 ? `Exits: ${exits.join(" | ")}` : "Exits: none",
  ].join("\n");
}

function setInfoPanelContent(text) {
  infoPanelContent.textContent = text && text.length > 0 ? text : "(No data returned)";
}

function updatePanelButtonSelection() {
  for (const button of panelOpenButtons) {
    button.classList.toggle("active", button.dataset.panelId === activeInfoPanelId);
  }
}

function updateImmortalStatusFromText(text) {
  const lower = text.toLowerCase();
  if (lower.includes("you are not an immortal")) {
    setImmortalState(false);
    return;
  }

  if (
    lower.includes("immortal commands") ||
    lower.includes("wizhelp") ||
    lower.includes("holylight") ||
    lower.includes("wiznet")
  ) {
    setImmortalState(true);
  }
}

function startPanelCapture(panelId) {
  const config = PANEL_CONFIG[panelId];
  if (!config?.command) {
    return;
  }

  panelCapture = {
    panelId,
    expiresAt: performance.now() + 1500,
  };

  panelTextById[panelId] = "";
  setInfoPanelContent(`[bridge] running '${config.command}'...`);
  send(socket, { type: "mud_command", command: config.command });
}

function refreshInfoPanel(panelId = activeInfoPanelId) {
  if (!panelId) {
    return;
  }

  const config = PANEL_CONFIG[panelId];
  if (!config) {
    return;
  }

  if (panelId === "room_description") {
    setInfoPanelContent(buildRoomDescriptionPanelText(currentRoom));
    return;
  }

  if (config.immortalOnly && !isImmortal) {
    setInfoPanelContent("Checking immortal access with 'wizhelp'...");
    startPanelCapture(panelId);
    return;
  }

  startPanelCapture(panelId);
}

function openInfoPanel(panelId) {
  const config = PANEL_CONFIG[panelId];
  if (!config) {
    return;
  }

  activeInfoPanelId = panelId;
  infoPanel.classList.remove("hidden");
  infoPanelTitle.textContent = config.title;
  infoPanelSubtitle.textContent = config.subtitle;
  updatePanelButtonSelection();

  if (panelId === "room_description") {
    setInfoPanelContent(buildRoomDescriptionPanelText(currentRoom));
    return;
  }

  if (config.immortalOnly && !isImmortal) {
    setInfoPanelContent("Checking immortal access with 'wizhelp'...");
    startPanelCapture(panelId);
    return;
  }

  const cached = panelTextById[panelId];
  if (cached && cached.trim().length > 0) {
    setInfoPanelContent(cached);
  }

  refreshInfoPanel(panelId);
}

function closeInfoPanel() {
  activeInfoPanelId = null;
  infoPanel.classList.add("hidden");
  updatePanelButtonSelection();
}

function capturePanelText(text) {
  if (!panelCapture) {
    return;
  }

  const panelId = panelCapture.panelId;
  panelTextById[panelId] = (panelTextById[panelId] ?? "") + text.replace(/\r/g, "");
  panelCapture.expiresAt = performance.now() + 340;

  if (panelId === "wiz") {
    const lower = panelTextById[panelId].toLowerCase();
    if (
      lower.includes("you are not an immortal") ||
      lower.includes("huh?") ||
      lower.includes("unknown command")
    ) {
      setImmortalState(false);
    } else if (lower.trim().length > 0) {
      setImmortalState(true);
    }
  }

  if (activeInfoPanelId === panelId) {
    setInfoPanelContent(panelTextById[panelId]);
  }
}

function clearRoomScene() {
  clearGroup(roomGroup);
  actorState.playerRig = null;
  actorState.npcs = [];
  activeWalkableZones = [];
  activeExitTriggers = [];
  playerState.position.set(0, FLOOR_THICKNESS / 2, 0);
  playerState.velocity.set(0, 0, 0);
  sceneCanvas.style.cursor = orbitState.dragging ? "grabbing" : "grab";
}

function seededRandom(seed) {
  const x = Math.sin(seed * 12.9898 + seed * seed * 0.1327) * 43758.5453;
  return x - Math.floor(x);
}

function keywordScore(text, keywords) {
  let score = 0;
  for (const keyword of keywords) {
    if (text.includes(keyword)) {
      score += 1;
    }
  }
  return score;
}

function resolveSectorName(sector) {
  const parsed = Number.parseInt(sector ?? "", 10);
  if (!Number.isFinite(parsed)) {
    return "Unknown";
  }
  return SECTOR_NAMES[parsed] ?? `Sector ${parsed}`;
}

function isInteriorBuildingRoom(room) {
  if (!room) {
    return false;
  }

  const sectorValue = Number.parseInt(room.sector ?? "", 10);
  if (Number.isFinite(sectorValue) && sectorValue === 0) {
    return true;
  }

  const text = `${room.name ?? ""} ${room.description ?? ""}`.toLowerCase();
  if (text.includes("inside")) {
    return true;
  }
  return BUILDING_INTERIOR_KEYWORDS.test(text);
}

function buildRoomMapLookup(roomMapPayload) {
  const lookup = Object.create(null);
  const rooms = roomMapPayload?.rooms;
  if (!Array.isArray(rooms)) {
    return lookup;
  }

  for (const room of rooms) {
    if (room?.vnum === undefined || room?.vnum === null) {
      continue;
    }
    lookup[String(room.vnum)] = room;
  }
  return lookup;
}

function shouldUseBuildingDoorway(currentRoom, targetRoom) {
  if (!targetRoom) {
    return false;
  }

  const currentInside = isInteriorBuildingRoom(currentRoom);
  const targetInside = isInteriorBuildingRoom(targetRoom);
  if (currentInside === targetInside) {
    return false;
  }

  const insideRoom = currentInside ? currentRoom : targetRoom;
  const insideText = `${insideRoom?.name ?? ""} ${insideRoom?.description ?? ""}`.toLowerCase();
  return BUILDING_INTERIOR_KEYWORDS.test(insideText) || currentInside || targetInside;
}

function chooseRoomStyle(room) {
  const sectorValue = Number.parseInt(room?.sector ?? "", 10);
  const sectorFallbackName = SECTOR_STYLE_HINT[sectorValue] ?? "stone";

  const description = (room?.description ?? "").toLowerCase();
  let chosenName = sectorFallbackName;
  let chosenScore = 0;

  for (const hint of KEYWORD_STYLE_HINTS) {
    const score = keywordScore(description, hint.keywords);
    if (score > chosenScore) {
      chosenScore = score;
      chosenName = hint.style;
    }
  }

  return {
    name: chosenName,
    style: STYLE_PRESETS[chosenName] ?? STYLE_PRESETS.stone,
  };
}

function applySceneStyle(style) {
  scene.background = new THREE.Color(style.background);
  scene.fog = new THREE.Fog(style.fog, 40, 220);

  hemiLight.color.setHex(style.hemiSky);
  hemiLight.groundColor.setHex(style.hemiGround);
  hemiLight.intensity = style.hemiIntensity;

  dirLight.color.setHex(style.sunColor);
  dirLight.intensity = style.sunIntensity;

  accentLight.color.setHex(style.accent);
  accentLight.intensity = 0.42 + movePulse * 0.35;
}

function makeStandardMaterial(color, overrides = {}) {
  return new THREE.MeshStandardMaterial({
    color,
    roughness: 0.78,
    metalness: 0.1,
    ...overrides,
  });
}

function addBox(width, height, depth, x, y, z, material) {
  const mesh = new THREE.Mesh(new THREE.BoxGeometry(width, height, depth), material);
  mesh.position.set(x, y, z);
  roomGroup.add(mesh);
  return mesh;
}

function isExitAvailable(exits, direction) {
  const targetVnum = Number.parseInt(exits?.[direction] ?? "", 10);
  return Number.isFinite(targetVnum) && targetVnum > 0;
}

function addWalkZone(minX, maxX, minZ, maxZ) {
  activeWalkableZones.push({
    minX,
    maxX,
    minZ,
    maxZ,
  });
}

function isPointInWalkZone(x, z) {
  for (const zone of activeWalkableZones) {
    if (x >= zone.minX && x <= zone.maxX && z >= zone.minZ && z <= zone.maxZ) {
      return true;
    }
  }
  return false;
}

function clampPointToNearestZone(x, z) {
  if (activeWalkableZones.length === 0) {
    return { x: 0, z: 0 };
  }

  let closest = null;
  let closestDistance = Number.POSITIVE_INFINITY;

  for (const zone of activeWalkableZones) {
    const clampedX = clamp(x, zone.minX, zone.maxX);
    const clampedZ = clamp(z, zone.minZ, zone.maxZ);
    const dx = x - clampedX;
    const dz = z - clampedZ;
    const distance = dx * dx + dz * dz;
    if (distance < closestDistance) {
      closestDistance = distance;
      closest = { x: clampedX, z: clampedZ };
    }
  }

  return closest ?? { x: 0, z: 0 };
}

function resolveWalkPosition(currentX, currentZ, nextX, nextZ) {
  if (isPointInWalkZone(nextX, nextZ)) {
    return { x: nextX, z: nextZ };
  }

  if (isPointInWalkZone(nextX, currentZ)) {
    return { x: nextX, z: currentZ };
  }

  if (isPointInWalkZone(currentX, nextZ)) {
    return { x: currentX, z: nextZ };
  }

  return clampPointToNearestZone(nextX, nextZ);
}

function samplePointInZone(zone, seed) {
  const randomX = zone.minX + (zone.maxX - zone.minX) * seededRandom(seed * 1.17 + 12.3);
  const randomZ = zone.minZ + (zone.maxZ - zone.minZ) * seededRandom(seed * 2.53 + 44.8);
  return new THREE.Vector3(randomX, FLOOR_THICKNESS / 2, randomZ);
}

function sampleWalkablePoint(seed = 1) {
  if (activeWalkableZones.length === 0) {
    return new THREE.Vector3(0, FLOOR_THICKNESS / 2, 0);
  }

  const zoneIndex = Math.floor(seededRandom(seed * 6.71 + 3.2) * activeWalkableZones.length);
  const zone = activeWalkableZones[zoneIndex] ?? activeWalkableZones[0];
  return samplePointInZone(zone, seed + zoneIndex * 5.13);
}

function createHumanoid({
  bodyColor = 0x8bc8ff,
  accentColor = 0xe7d9c1,
  limbColor = 0x2e394a,
  heightScale = 1,
} = {}) {
  const scale = ROOM_SCALE * 0.55 * heightScale;
  const group = new THREE.Group();

  const torso = new THREE.Mesh(
    new THREE.BoxGeometry(1.2 * scale, 1.8 * scale, 0.7 * scale),
    new THREE.MeshStandardMaterial({ color: bodyColor, roughness: 0.55, metalness: 0.12 }),
  );
  torso.position.y = FLOOR_THICKNESS / 2 + 2.45 * scale;

  const head = new THREE.Mesh(
    new THREE.SphereGeometry(0.46 * scale, 14, 14),
    new THREE.MeshStandardMaterial({ color: accentColor, roughness: 0.72, metalness: 0.03 }),
  );
  head.position.y = FLOOR_THICKNESS / 2 + 3.95 * scale;

  const armGeometry = new THREE.BoxGeometry(0.34 * scale, 1.3 * scale, 0.34 * scale);
  const legGeometry = new THREE.BoxGeometry(0.4 * scale, 1.55 * scale, 0.4 * scale);
  const limbMaterial = new THREE.MeshStandardMaterial({
    color: limbColor,
    roughness: 0.66,
    metalness: 0.08,
  });

  const leftArmPivot = new THREE.Group();
  const rightArmPivot = new THREE.Group();
  leftArmPivot.position.set(-0.86 * scale, FLOOR_THICKNESS / 2 + 3.18 * scale, 0);
  rightArmPivot.position.set(0.86 * scale, FLOOR_THICKNESS / 2 + 3.18 * scale, 0);

  const leftArm = new THREE.Mesh(armGeometry, limbMaterial);
  const rightArm = new THREE.Mesh(armGeometry, limbMaterial);
  leftArm.position.y = -0.68 * scale;
  rightArm.position.y = -0.68 * scale;
  leftArmPivot.add(leftArm);
  rightArmPivot.add(rightArm);

  const leftLegPivot = new THREE.Group();
  const rightLegPivot = new THREE.Group();
  leftLegPivot.position.set(-0.36 * scale, FLOOR_THICKNESS / 2 + 1.6 * scale, 0);
  rightLegPivot.position.set(0.36 * scale, FLOOR_THICKNESS / 2 + 1.6 * scale, 0);

  const leftLeg = new THREE.Mesh(legGeometry, limbMaterial);
  const rightLeg = new THREE.Mesh(legGeometry, limbMaterial);
  leftLeg.position.y = -0.8 * scale;
  rightLeg.position.y = -0.8 * scale;
  leftLegPivot.add(leftLeg);
  rightLegPivot.add(rightLeg);

  group.add(torso);
  group.add(head);
  group.add(leftArmPivot);
  group.add(rightArmPivot);
  group.add(leftLegPivot);
  group.add(rightLegPivot);

  roomGroup.add(group);

  return {
    group,
    leftArmPivot,
    rightArmPivot,
    leftLegPivot,
    rightLegPivot,
    baseY: FLOOR_THICKNESS / 2,
    stepPhase: seededRandom(bodyColor) * Math.PI * 2,
  };
}

function animateHumanoid(rig, speed, deltaSeconds) {
  if (!rig) {
    return;
  }

  const normalizedSpeed = clamp(speed / WALK_SPEED, 0, 1.65);
  const moving = normalizedSpeed > 0.05;
  const strideRate = moving ? 7.2 + normalizedSpeed * 3.2 : 2.8;
  rig.stepPhase += deltaSeconds * strideRate;

  const stride = moving ? Math.sin(rig.stepPhase * 2.2) : Math.sin(rig.stepPhase) * 0.08;
  const armSwing = stride * 0.9 * normalizedSpeed;
  const legSwing = stride * 1.1 * normalizedSpeed;
  const bob = moving ? Math.abs(Math.sin(rig.stepPhase * 2.2)) * 0.16 : 0;

  rig.leftArmPivot.rotation.x = armSwing;
  rig.rightArmPivot.rotation.x = -armSwing;
  rig.leftLegPivot.rotation.x = -legSwing;
  rig.rightLegPivot.rotation.x = legSwing;
  rig.group.position.y = rig.baseY + bob;
}

function isGuardRoom(room) {
  const roomText = `${room?.name ?? ""} ${room?.description ?? ""}`;
  return NPC_GUARD_KEYWORDS.test(roomText);
}

function collectGuardPosts(exits) {
  const posts = [new THREE.Vector3(0, FLOOR_THICKNESS / 2, 0)];

  if (isExitAvailable(exits, "north")) {
    posts.push(new THREE.Vector3(0, FLOOR_THICKNESS / 2, -ROOM_DEPTH / 2 + EXIT_TRIGGER_DEPTH * 2.2));
  }
  if (isExitAvailable(exits, "south")) {
    posts.push(new THREE.Vector3(0, FLOOR_THICKNESS / 2, ROOM_DEPTH / 2 - EXIT_TRIGGER_DEPTH * 2.2));
  }
  if (isExitAvailable(exits, "east")) {
    posts.push(new THREE.Vector3(ROOM_WIDTH / 2 - EXIT_TRIGGER_DEPTH * 2.2, FLOOR_THICKNESS / 2, 0));
  }
  if (isExitAvailable(exits, "west")) {
    posts.push(new THREE.Vector3(-ROOM_WIDTH / 2 + EXIT_TRIGGER_DEPTH * 2.2, FLOOR_THICKNESS / 2, 0));
  }

  return posts;
}

function updateNpcActor(actor, deltaSeconds) {
  if (!actor?.rig) {
    return;
  }

  if (actor.isGuard) {
    const toPost = new THREE.Vector3().subVectors(actor.guardPost, actor.position);
    const distance = toPost.length();
    if (distance > 0.2) {
      toPost.normalize();
      actor.position.addScaledVector(toPost, Math.min(distance, actor.speed * 0.45 * deltaSeconds));
      actor.heading = Math.atan2(toPost.x, toPost.z);
      actor.lastMoveSpeed = actor.speed * 0.45;
    } else {
      actor.lastMoveSpeed = 0;
      actor.heading += deltaSeconds * 0.2;
    }
  } else {
    actor.idleTimer -= deltaSeconds;
    const toTarget = new THREE.Vector3().subVectors(actor.target, actor.position);
    const distance = toTarget.length();

    if (distance < 0.5 || actor.idleTimer > 0) {
      actor.lastMoveSpeed = 0;
      if (actor.idleTimer <= 0) {
        actor.target.copy(sampleWalkablePoint(actor.seed + performance.now() * 0.0001));
        actor.idleTimer = seededRandom(actor.seed + actor.position.x * 0.13) * 2.2;
      }
    } else {
      toTarget.normalize();
      const stepSpeed = actor.speed * (0.65 + seededRandom(actor.seed + actor.position.z * 0.07) * 0.4);
      actor.position.addScaledVector(toTarget, Math.min(distance, stepSpeed * deltaSeconds));
      actor.heading = Math.atan2(toTarget.x, toTarget.z);
      actor.lastMoveSpeed = stepSpeed;
    }
  }

  const clamped = resolveWalkPosition(actor.position.x, actor.position.z, actor.position.x, actor.position.z);
  actor.position.set(clamped.x, FLOOR_THICKNESS / 2, clamped.z);

  actor.rig.group.position.x = actor.position.x;
  actor.rig.group.position.z = actor.position.z;
  actor.rig.group.rotation.y = lerpAngle(
    actor.rig.group.rotation.y,
    actor.heading,
    clamp(deltaSeconds * TURN_SPEED, 0, 1),
  );
  animateHumanoid(actor.rig, actor.lastMoveSpeed ?? 0, deltaSeconds);
}

function updatePlayerMovement(deltaSeconds, nowMs) {
  const hasInput = inputState.forward || inputState.backward || inputState.left || inputState.right;
  if (!hasInput) {
    playerState.velocity.multiplyScalar(Math.exp(-deltaSeconds * 7.5));
  } else {
    camera.getWorldDirection(forwardVector);
    forwardVector.y = 0;
    if (forwardVector.lengthSq() < 0.0001) {
      forwardVector.set(0, 0, -1);
    }
    forwardVector.normalize();
    rightVector.set(forwardVector.z, 0, -forwardVector.x).normalize();

    const moveVector = new THREE.Vector3();
    if (inputState.forward) {
      moveVector.add(forwardVector);
    }
    if (inputState.backward) {
      moveVector.sub(forwardVector);
    }
    if (inputState.left) {
      moveVector.sub(rightVector);
    }
    if (inputState.right) {
      moveVector.add(rightVector);
    }

    if (moveVector.lengthSq() > 0.0001) {
      moveVector.normalize();
      const targetSpeed = WALK_SPEED * (inputState.sprint ? SPRINT_MULTIPLIER : 1);
      const targetVelocity = moveVector.multiplyScalar(targetSpeed);
      playerState.velocity.lerp(targetVelocity, clamp(deltaSeconds * 10, 0, 1));
      playerState.heading = Math.atan2(playerState.velocity.x, playerState.velocity.z);
    }
  }

  const nextX = playerState.position.x + playerState.velocity.x * deltaSeconds;
  const nextZ = playerState.position.z + playerState.velocity.z * deltaSeconds;
  const resolved = resolveWalkPosition(playerState.position.x, playerState.position.z, nextX, nextZ);
  playerState.position.set(resolved.x, FLOOR_THICKNESS / 2, resolved.z);

  const speed = playerState.velocity.length();
  if (actorState.playerRig) {
    actorState.playerRig.group.position.x = playerState.position.x;
    actorState.playerRig.group.position.z = playerState.position.z;
    actorState.playerRig.group.rotation.y = lerpAngle(
      actorState.playerRig.group.rotation.y,
      playerState.heading,
      clamp(deltaSeconds * TURN_SPEED, 0, 1),
    );
    animateHumanoid(actorState.playerRig, speed, deltaSeconds);
  }

  orbitTarget.x = playerState.position.x;
  orbitTarget.y = CAMERA_BASE_HEIGHT;
  orbitTarget.z = playerState.position.z;

  if (nowMs - playerState.lastExitAt < 650) {
    return;
  }

  for (const trigger of activeExitTriggers) {
    if (
      playerState.position.x >= trigger.minX &&
      playerState.position.x <= trigger.maxX &&
      playerState.position.z >= trigger.minZ &&
      playerState.position.z <= trigger.maxZ
    ) {
      playerState.lastExitAt = nowMs;
      attemptMove(trigger.direction);
      break;
    }
  }
}

function spawnRoomActors(room, style, exits) {
  actorState.playerRig = createHumanoid({
    bodyColor: style.accent,
    accentColor: 0xf0dbc2,
    limbColor: 0x2a3344,
    heightScale: 1.08,
  });
  playerState.position.set(0, FLOOR_THICKNESS / 2, 0);
  playerState.velocity.set(0, 0, 0);
  playerState.heading = Math.PI;
  actorState.playerRig.group.position.copy(playerState.position);
  actorState.playerRig.group.rotation.y = playerState.heading;

  const totalMobs = clamp(Number.parseInt(room?.mobCount ?? 0, 10) || 0, 0, NPC_MAX_COUNT);
  const guardRoom = isGuardRoom(room);
  const guardPosts = collectGuardPosts(exits);
  let guardCount = 0;
  if (totalMobs > 0 && guardRoom) {
    guardCount = Math.min(guardPosts.length, Math.max(1, Math.floor(totalMobs * 0.45)));
  }

  const npcCount = Math.max(0, totalMobs);
  actorState.npcs = [];

  for (let index = 0; index < npcCount; index += 1) {
    const isGuard = index < guardCount;
    const seed = Number(room.vnum || 0) * 17.3 + index * 13.7;
    const startPos = isGuard
      ? guardPosts[index % guardPosts.length].clone()
      : sampleWalkablePoint(seed + 3.1);

    const rig = createHumanoid({
      bodyColor: isGuard ? 0x536f9f : 0x50756a,
      accentColor: isGuard ? 0xc6d1e4 : 0xd4c9af,
      limbColor: isGuard ? 0x2c3650 : 0x2d3d34,
      heightScale: isGuard ? 1.03 : 0.95 + seededRandom(seed) * 0.12,
    });

    const actor = {
      rig,
      isGuard,
      seed,
      position: startPos,
      heading: seededRandom(seed + 5.2) * Math.PI * 2,
      speed: isGuard ? WALK_SPEED * 0.58 : WALK_SPEED * (0.43 + seededRandom(seed + 8.9) * 0.32),
      target: sampleWalkablePoint(seed + 9.7),
      idleTimer: seededRandom(seed + 4.1) * 1.4,
      guardPost: isGuard ? guardPosts[index % guardPosts.length].clone() : null,
      lastMoveSpeed: 0,
    };

    actor.rig.group.position.copy(actor.position);
    actor.rig.group.rotation.y = actor.heading;
    actorState.npcs.push(actor);
  }
}

function addNorthSouthWall(direction, mode, wallMaterial, trimMaterial) {
  const z = direction === "north" ? -ROOM_DEPTH / 2 : ROOM_DEPTH / 2;
  const wallY = ROOM_HEIGHT / 2;

  if (mode === "open") {
    return;
  }

  if (mode === "solid") {
    addBox(ROOM_WIDTH, ROOM_HEIGHT, WALL_THICKNESS, 0, wallY, z, wallMaterial);
    return;
  }

  const sideWidth = (ROOM_WIDTH - EXIT_WIDTH) / 2;
  if (sideWidth > 0.1) {
    addBox(
      sideWidth,
      ROOM_HEIGHT,
      WALL_THICKNESS,
      -ROOM_WIDTH / 2 + sideWidth / 2,
      wallY,
      z,
      wallMaterial,
    );
    addBox(
      sideWidth,
      ROOM_HEIGHT,
      WALL_THICKNESS,
      ROOM_WIDTH / 2 - sideWidth / 2,
      wallY,
      z,
      wallMaterial,
    );
  }

  const topHeight = ROOM_HEIGHT - EXIT_HEIGHT;
  if (topHeight > 0.1) {
    addBox(EXIT_WIDTH, topHeight, WALL_THICKNESS, 0, EXIT_HEIGHT + topHeight / 2, z, wallMaterial);
  }

  const doorWidth = EXIT_WIDTH * 0.42;
  const doorHeight = EXIT_HEIGHT * 0.82;
  const doorDepth = WALL_THICKNESS * 0.45;
  addBox(
    doorWidth,
    doorHeight,
    doorDepth,
    -EXIT_WIDTH * 0.16,
    doorHeight / 2,
    z + (direction === "north" ? WALL_THICKNESS * 0.18 : -WALL_THICKNESS * 0.18),
    trimMaterial,
  );
}

function addEastWestWall(direction, mode, wallMaterial, trimMaterial) {
  const x = direction === "east" ? ROOM_WIDTH / 2 : -ROOM_WIDTH / 2;
  const wallY = ROOM_HEIGHT / 2;

  if (mode === "open") {
    return;
  }

  if (mode === "solid") {
    addBox(WALL_THICKNESS, ROOM_HEIGHT, ROOM_DEPTH, x, wallY, 0, wallMaterial);
    return;
  }

  const sideDepth = (ROOM_DEPTH - EXIT_WIDTH) / 2;
  if (sideDepth > 0.1) {
    addBox(
      WALL_THICKNESS,
      ROOM_HEIGHT,
      sideDepth,
      x,
      wallY,
      -ROOM_DEPTH / 2 + sideDepth / 2,
      wallMaterial,
    );
    addBox(
      WALL_THICKNESS,
      ROOM_HEIGHT,
      sideDepth,
      x,
      wallY,
      ROOM_DEPTH / 2 - sideDepth / 2,
      wallMaterial,
    );
  }

  const topHeight = ROOM_HEIGHT - EXIT_HEIGHT;
  if (topHeight > 0.1) {
    addBox(WALL_THICKNESS, topHeight, EXIT_WIDTH, x, EXIT_HEIGHT + topHeight / 2, 0, wallMaterial);
  }

  const doorWidth = EXIT_WIDTH * 0.42;
  const doorHeight = EXIT_HEIGHT * 0.82;
  const doorDepth = WALL_THICKNESS * 0.45;
  addBox(
    doorDepth,
    doorHeight,
    doorWidth,
    x + (direction === "east" ? -WALL_THICKNESS * 0.18 : WALL_THICKNESS * 0.18),
    doorHeight / 2,
    -EXIT_WIDTH * 0.16,
    trimMaterial,
  );
}

function addExteriorBuilding(direction, style, trimMaterial) {
  const buildingWidth = EXIT_WIDTH * 2.05;
  const buildingDepth = ROOM_SCALE * 11;
  const buildingHeight = ROOM_HEIGHT * 0.86;
  const facadeThickness = WALL_THICKNESS * 1.6;
  const bodyMaterial = makeStandardMaterial(style.wall, { roughness: 0.66, metalness: 0.14 });
  const roofMaterial = makeStandardMaterial(style.prop, { roughness: 0.74, metalness: 0.1 });

  if (direction === "north" || direction === "south") {
    const sign = direction === "north" ? -1 : 1;
    const centerZ = sign * (ROOM_DEPTH / 2 + buildingDepth / 2 + WALL_THICKNESS * 0.5);
    addBox(buildingWidth, buildingHeight, buildingDepth, 0, buildingHeight / 2, centerZ, bodyMaterial);
    addBox(buildingWidth + ROOM_SCALE * 2.3, ROOM_SCALE * 1.05, buildingDepth + ROOM_SCALE * 1.7, 0, buildingHeight + ROOM_SCALE * 0.55, centerZ, roofMaterial);
    addBox(buildingWidth * 0.42, EXIT_HEIGHT * 0.78, facadeThickness, 0, EXIT_HEIGHT * 0.39, centerZ - sign * (buildingDepth / 2 - facadeThickness / 2), trimMaterial);
    return;
  }

  const sign = direction === "east" ? 1 : -1;
  const centerX = sign * (ROOM_WIDTH / 2 + buildingDepth / 2 + WALL_THICKNESS * 0.5);
  addBox(buildingDepth, buildingHeight, buildingWidth, centerX, buildingHeight / 2, 0, bodyMaterial);
  addBox(buildingDepth + ROOM_SCALE * 1.7, ROOM_SCALE * 1.05, buildingWidth + ROOM_SCALE * 2.3, centerX, buildingHeight + ROOM_SCALE * 0.55, 0, roofMaterial);
  addBox(facadeThickness, EXIT_HEIGHT * 0.78, buildingWidth * 0.42, centerX - sign * (buildingDepth / 2 - facadeThickness / 2), EXIT_HEIGHT * 0.39, 0, trimMaterial);
}

function addCardinalExitNavigation(direction, doorway) {
  const openHalfWidth = doorway ? EXIT_WIDTH * 0.34 : EXIT_WIDTH * 0.48;
  const outerDepth = EXIT_TRIGGER_DEPTH * 1.45;

  if (direction === "north") {
    addWalkZone(
      -openHalfWidth,
      openHalfWidth,
      -ROOM_DEPTH / 2 - outerDepth,
      -ROOM_DEPTH / 2 + CHARACTER_RADIUS,
    );
    activeExitTriggers.push({
      direction,
      minX: -openHalfWidth + CHARACTER_RADIUS * 0.3,
      maxX: openHalfWidth - CHARACTER_RADIUS * 0.3,
      minZ: -ROOM_DEPTH / 2 - outerDepth,
      maxZ: -ROOM_DEPTH / 2 - EXIT_TRIGGER_DEPTH * 0.28,
    });
    return;
  }

  if (direction === "south") {
    addWalkZone(
      -openHalfWidth,
      openHalfWidth,
      ROOM_DEPTH / 2 - CHARACTER_RADIUS,
      ROOM_DEPTH / 2 + outerDepth,
    );
    activeExitTriggers.push({
      direction,
      minX: -openHalfWidth + CHARACTER_RADIUS * 0.3,
      maxX: openHalfWidth - CHARACTER_RADIUS * 0.3,
      minZ: ROOM_DEPTH / 2 + EXIT_TRIGGER_DEPTH * 0.28,
      maxZ: ROOM_DEPTH / 2 + outerDepth,
    });
    return;
  }

  if (direction === "east") {
    addWalkZone(
      ROOM_WIDTH / 2 - CHARACTER_RADIUS,
      ROOM_WIDTH / 2 + outerDepth,
      -openHalfWidth,
      openHalfWidth,
    );
    activeExitTriggers.push({
      direction,
      minX: ROOM_WIDTH / 2 + EXIT_TRIGGER_DEPTH * 0.28,
      maxX: ROOM_WIDTH / 2 + outerDepth,
      minZ: -openHalfWidth + CHARACTER_RADIUS * 0.3,
      maxZ: openHalfWidth - CHARACTER_RADIUS * 0.3,
    });
    return;
  }

  addWalkZone(
    -ROOM_WIDTH / 2 - outerDepth,
    -ROOM_WIDTH / 2 + CHARACTER_RADIUS,
    -openHalfWidth,
    openHalfWidth,
  );
  activeExitTriggers.push({
    direction,
    minX: -ROOM_WIDTH / 2 - outerDepth,
    maxX: -ROOM_WIDTH / 2 - EXIT_TRIGGER_DEPTH * 0.28,
    minZ: -openHalfWidth + CHARACTER_RADIUS * 0.3,
    maxZ: openHalfWidth - CHARACTER_RADIUS * 0.3,
  });
}

function addExitApron(direction, floorMaterial) {
  const apronDepth = EXIT_TRIGGER_DEPTH * 1.7;
  const apronWidth = EXIT_WIDTH * 1.05;

  if (direction === "north") {
    addBox(
      apronWidth,
      FLOOR_THICKNESS,
      apronDepth,
      0,
      -FLOOR_THICKNESS / 2,
      -ROOM_DEPTH / 2 - apronDepth / 2 + FLOOR_THICKNESS * 0.12,
      floorMaterial,
    );
    return;
  }

  if (direction === "south") {
    addBox(
      apronWidth,
      FLOOR_THICKNESS,
      apronDepth,
      0,
      -FLOOR_THICKNESS / 2,
      ROOM_DEPTH / 2 + apronDepth / 2 - FLOOR_THICKNESS * 0.12,
      floorMaterial,
    );
    return;
  }

  if (direction === "east") {
    addBox(
      apronDepth,
      FLOOR_THICKNESS,
      apronWidth,
      ROOM_WIDTH / 2 + apronDepth / 2 - FLOOR_THICKNESS * 0.12,
      -FLOOR_THICKNESS / 2,
      0,
      floorMaterial,
    );
    return;
  }

  addBox(
    apronDepth,
    FLOOR_THICKNESS,
    apronWidth,
    -ROOM_WIDTH / 2 - apronDepth / 2 + FLOOR_THICKNESS * 0.12,
    -FLOOR_THICKNESS / 2,
    0,
    floorMaterial,
  );
}

function addVerticalExitFeature(direction, style) {
  const shaftRadius = ROOM_SCALE * 1.4;
  const centerX = direction === "up" ? -ROOM_SCALE * 2.4 : ROOM_SCALE * 2.4;
  const centerZ = 0;

  const rim = new THREE.Mesh(
    new THREE.TorusGeometry(shaftRadius, ROOM_SCALE * 0.22, 8, 24),
    new THREE.MeshStandardMaterial({
      color: style.accent,
      emissive: style.portalEmissive,
      emissiveIntensity: 0.35,
      roughness: 0.5,
      metalness: 0.24,
    }),
  );
  rim.rotation.x = Math.PI / 2;
  rim.position.set(centerX, direction === "up" ? ROOM_HEIGHT - 0.2 : 0.26, centerZ);
  roomGroup.add(rim);

  const ladderMaterial = new THREE.MeshStandardMaterial({
    color: style.prop,
    roughness: 0.66,
    metalness: 0.2,
  });
  const railOffset = shaftRadius * 0.45;
  const railHeight = ROOM_HEIGHT * 0.75;
  addBox(ROOM_SCALE * 0.14, railHeight, ROOM_SCALE * 0.14, centerX - railOffset, railHeight / 2, centerZ, ladderMaterial);
  addBox(ROOM_SCALE * 0.14, railHeight, ROOM_SCALE * 0.14, centerX + railOffset, railHeight / 2, centerZ, ladderMaterial);

  for (let rung = 0; rung < 8; rung += 1) {
    const y = ROOM_SCALE * 1.4 + rung * ROOM_SCALE * 2.3;
    addBox(shaftRadius * 0.8, ROOM_SCALE * 0.1, ROOM_SCALE * 0.1, centerX, y, centerZ, ladderMaterial);
  }
}

function isExitLane(x, z) {
  const nsLane = Math.abs(x) < EXIT_WIDTH * 0.65 && Math.abs(z) > ROOM_DEPTH * 0.3;
  const ewLane = Math.abs(z) < EXIT_WIDTH * 0.65 && Math.abs(x) > ROOM_WIDTH * 0.3;
  return nsLane || ewLane;
}

function sampleInteriorPoint(seed, index) {
  let x = (seededRandom(seed + index * 11.37) - 0.5) * (ROOM_WIDTH - 4);
  let z = (seededRandom(seed + index * 23.91) - 0.5) * (ROOM_DEPTH - 4);

  if (isExitLane(x, z)) {
    x *= 0.45;
    z *= 0.45;
  }

  return { x, z };
}

function addTree(x, z, size, style) {
  const trunk = new THREE.Mesh(
    new THREE.CylinderGeometry(0.16 * size, 0.2 * size, 1.6 * size, 10),
    makeStandardMaterial(style.wall, { roughness: 0.86, metalness: 0.05 }),
  );
  trunk.position.set(x, 0.8 * size, z);

  const canopy = new THREE.Mesh(
    new THREE.SphereGeometry(0.62 * size, 10, 10),
    makeStandardMaterial(style.prop, { roughness: 0.92, metalness: 0.03 }),
  );
  canopy.position.set(x, 1.9 * size, z);

  roomGroup.add(trunk);
  roomGroup.add(canopy);
}

function addPillar(x, z, size, style) {
  const pillar = new THREE.Mesh(
    new THREE.CylinderGeometry(0.26 * size, 0.3 * size, 2.2 * size, 12),
    makeStandardMaterial(style.prop, { roughness: 0.72, metalness: 0.18 }),
  );
  pillar.position.set(x, 1.1 * size, z);
  roomGroup.add(pillar);
}

function addRock(x, z, size, style) {
  const rock = new THREE.Mesh(
    new THREE.DodecahedronGeometry(0.45 * size, 0),
    makeStandardMaterial(style.prop, { roughness: 0.88, metalness: 0.05 }),
  );
  rock.position.set(x, 0.38 * size, z);
  rock.scale.set(1.1, 0.7, 0.9);
  roomGroup.add(rock);
}

function addGravestone(x, z, size, style) {
  const base = new THREE.Mesh(
    new THREE.BoxGeometry(0.8 * size, 1.5 * size, 0.28 * size),
    makeStandardMaterial(style.prop, { roughness: 0.82, metalness: 0.07 }),
  );
  base.position.set(x, 0.75 * size, z);
  roomGroup.add(base);
}
function addDescriptionDecor(room, styleName, style) {
  const seed = Number(room.vnum) || 1;
  const description = (room.description ?? "").toLowerCase();

  const hasWaterFeature = /water|river|lake|sea|ocean|pool|underwater/.test(description);
  if (hasWaterFeature || styleName === "water") {
    const pool = new THREE.Mesh(
      new THREE.CircleGeometry(2.2, 24),
      new THREE.MeshStandardMaterial({
        color: style.portal,
        emissive: style.portalEmissive,
        emissiveIntensity: 0.34,
        transparent: true,
        opacity: 0.56,
        roughness: 0.18,
        metalness: 0.12,
        side: THREE.DoubleSide,
      }),
    );
    pool.rotation.x = -Math.PI / 2;
    pool.position.set(0, 0.05, 0);
    roomGroup.add(pool);
  }

  const hasTorches = /torch|lantern|flame|brazier|candle/.test(description);
  if (hasTorches || styleName === "inferno") {
    const leftTorch = new THREE.PointLight(0xffa46a, 0.58, 21, 2);
    leftTorch.position.set(-ROOM_WIDTH / 2 + 1.1, ROOM_HEIGHT * 0.62, 0);
    const rightTorch = new THREE.PointLight(0xffa46a, 0.58, 21, 2);
    rightTorch.position.set(ROOM_WIDTH / 2 - 1.1, ROOM_HEIGHT * 0.62, 0);
    roomGroup.add(leftTorch);
    roomGroup.add(rightTorch);
  }

  const scatterCount = 12 + Math.floor(seededRandom(seed * 1.71) * 9);
  for (let index = 0; index < scatterCount; index += 1) {
    const point = sampleInteriorPoint(seed, index);
    const size = 0.8 + seededRandom(seed + index * 37.4) * 0.75;

    if (styleName === "forest" || styleName === "field" || styleName === "swamp") {
      if (seededRandom(seed + index * 8.1) > 0.35) {
        addTree(point.x, point.z, size, style);
      } else {
        addRock(point.x, point.z, size * 0.8, style);
      }
      continue;
    }

    if (styleName === "city" || styleName === "stone" || styleName === "arcane") {
      addPillar(point.x, point.z, size, style);
      continue;
    }

    if (styleName === "graveyard") {
      if (seededRandom(seed + index * 6.9) > 0.45) {
        addGravestone(point.x, point.z, size, style);
      } else {
        addRock(point.x, point.z, size * 0.8, style);
      }
      continue;
    }

    addRock(point.x, point.z, size, style);
  }
}

function buildRoomGeometry(room, styleName, style, roomMapPayload) {
  clearRoomScene();
  applySceneStyle(style);

  const floorMaterial = makeStandardMaterial(style.floor, { roughness: 0.9, metalness: 0.05 });
  const wallMaterial = makeStandardMaterial(style.wall, { roughness: 0.82, metalness: 0.09 });
  const ceilingMaterial = makeStandardMaterial(style.ceiling, { roughness: 0.75, metalness: 0.11 });

  addBox(ROOM_WIDTH, FLOOR_THICKNESS, ROOM_DEPTH, 0, -FLOOR_THICKNESS / 2, 0, floorMaterial);
  addBox(
    ROOM_WIDTH,
    FLOOR_THICKNESS,
    ROOM_DEPTH,
    0,
    ROOM_HEIGHT + FLOOR_THICKNESS / 2,
    0,
    ceilingMaterial,
  );

  const exits = room.exits ?? {};
  const roomLookup = buildRoomMapLookup(roomMapPayload);
  const currentInside = isInteriorBuildingRoom(room);
  const exitModes = {
    north: "solid",
    east: "solid",
    south: "solid",
    west: "solid",
  };

  const trimMaterial = makeStandardMaterial(style.prop, {
    roughness: 0.5,
    metalness: 0.32,
    emissive: style.portalEmissive,
    emissiveIntensity: 0.18,
  });

  for (const direction of ["north", "east", "south", "west"]) {
    if (!isExitAvailable(exits, direction)) {
      exitModes[direction] = "solid";
      continue;
    }

    const targetVnum = exits[direction];
    const targetRoom = roomLookup[String(targetVnum)];
    exitModes[direction] = shouldUseBuildingDoorway(room, targetRoom) ? "doorway" : "open";
  }

  addNorthSouthWall("north", exitModes.north, wallMaterial, trimMaterial);
  addNorthSouthWall("south", exitModes.south, wallMaterial, trimMaterial);
  addEastWestWall("east", exitModes.east, wallMaterial, trimMaterial);
  addEastWestWall("west", exitModes.west, wallMaterial, trimMaterial);

  if (exitModes.north !== "open") {
    addBox(ROOM_WIDTH + 0.2, 0.22, 0.22, 0, 0.08, -ROOM_DEPTH / 2 + 0.11, trimMaterial);
  }
  if (exitModes.south !== "open") {
    addBox(ROOM_WIDTH + 0.2, 0.22, 0.22, 0, 0.08, ROOM_DEPTH / 2 - 0.11, trimMaterial);
  }
  if (exitModes.west !== "open") {
    addBox(0.22, 0.22, ROOM_DEPTH + 0.2, -ROOM_WIDTH / 2 + 0.11, 0.08, 0, trimMaterial);
  }
  if (exitModes.east !== "open") {
    addBox(0.22, 0.22, ROOM_DEPTH + 0.2, ROOM_WIDTH / 2 - 0.11, 0.08, 0, trimMaterial);
  }

  activeWalkableZones = [];
  activeExitTriggers = [];
  addWalkZone(
    -ROOM_WIDTH / 2 + CHARACTER_RADIUS,
    ROOM_WIDTH / 2 - CHARACTER_RADIUS,
    -ROOM_DEPTH / 2 + CHARACTER_RADIUS,
    ROOM_DEPTH / 2 - CHARACTER_RADIUS,
  );

  for (const direction of ["north", "east", "south", "west"]) {
    if (isExitAvailable(exits, direction)) {
      addExitApron(direction, floorMaterial);
      addCardinalExitNavigation(direction, exitModes[direction] === "doorway");

      const targetVnum = exits[direction];
      const targetRoom = roomLookup[String(targetVnum)];
      const targetInside = isInteriorBuildingRoom(targetRoom);
      if (exitModes[direction] === "doorway" && !currentInside && targetInside) {
        addExteriorBuilding(direction, style, trimMaterial);
      }
    }
  }

  if (isExitAvailable(exits, "up")) {
    addVerticalExitFeature("up", style);
  }
  if (isExitAvailable(exits, "down")) {
    addVerticalExitFeature("down", style);
  }

  addDescriptionDecor(room, styleName, style);
  spawnRoomActors(room, style, exits);
}

function updateMovementButtons(room) {
  for (const [direction, button] of moveButtonsByDirection.entries()) {
    const targetVnum = Number.parseInt(room?.exits?.[direction] ?? "", 10);
    const enabled = isExitAvailable(room?.exits, direction);
    button.disabled = !enabled;
    button.classList.toggle("available", enabled);
    button.title = enabled
      ? `${DIRECTION_LABELS[direction]} to room #${targetVnum}`
      : `${DIRECTION_LABELS[direction]} exit unavailable`;
  }
}

function toMinimapLocalPosition(coord, centerCoord) {
  return new THREE.Vector3(
    (coord.x - centerCoord.x) * ROOM_SPACING,
    (coord.y - centerCoord.y) * LEVEL_HEIGHT,
    (coord.z - centerCoord.z) * ROOM_SPACING,
  );
}

function buildFallbackMap(room) {
  const centerCoord = room.coord ?? { x: 0, y: 0, z: 0 };
  const rooms = [
    {
      ...room,
      coord: centerCoord,
    },
  ];
  const links = [];

  for (const [direction, targetVnum] of Object.entries(room.exits || {})) {
    const offset = DIR_OFFSETS[direction];
    if (!offset) {
      continue;
    }

    rooms.push({
      vnum: targetVnum,
      name: direction,
      coord: {
        x: centerCoord.x + offset.x,
        y: centerCoord.y + offset.y,
        z: centerCoord.z + offset.z,
      },
      exits: {},
      isSynthetic: true,
    });

    links.push({
      from: room.vnum,
      to: targetVnum,
      direction,
    });
  }

  return {
    centerVnum: room.vnum,
    rooms,
    links,
  };
}

function drawMinimapMobIndicators(room, localPos, tileHeight) {
  const mobCount = Number.parseInt(room.mobCount ?? 0, 10);
  if (!Number.isFinite(mobCount) || mobCount <= 0 || room.isSynthetic) {
    return;
  }

  const displayedCount = Math.min(MOB_INDICATOR_LIMIT, mobCount);
  const ringRadius = displayedCount === 1 ? 0 : 0.45 + displayedCount * 0.1;
  const indicatorY = localPos.y + tileHeight + 0.3;

  for (let index = 0; index < displayedCount; index += 1) {
    const angle = displayedCount === 1 ? 0 : (index / displayedCount) * Math.PI * 2;
    const pip = new THREE.Mesh(
      new THREE.SphereGeometry(0.24, 10, 10),
      new THREE.MeshStandardMaterial({
        color: 0xff6442,
        emissive: 0x4d1500,
        roughness: 0.38,
        metalness: 0.14,
        transparent: true,
        opacity: 0.92,
      }),
    );
    pip.position.set(
      localPos.x + Math.cos(angle) * ringRadius,
      indicatorY,
      localPos.z + Math.sin(angle) * ringRadius,
    );
    minimapGroup.add(pip);
  }

  if (mobCount > displayedCount) {
    const overflowRing = new THREE.Mesh(
      new THREE.RingGeometry(ringRadius + 0.12, ringRadius + 0.28, 18),
      new THREE.MeshBasicMaterial({
        color: 0xffbf61,
        transparent: true,
        opacity: 0.88,
        side: THREE.DoubleSide,
      }),
    );
    overflowRing.rotation.x = -Math.PI / 2;
    overflowRing.position.set(localPos.x, indicatorY + 0.05, localPos.z);
    minimapGroup.add(overflowRing);
  }
}

function drawMinimapRoomNode(room, centerCoord, currentRoomVnum, roomPositions) {
  const coord = room.coord ?? centerCoord;
  const localPos = toMinimapLocalPosition(coord, centerCoord);
  roomPositions.set(String(room.vnum), localPos);

  const isCurrent = Number(room.vnum) === Number(currentRoomVnum);
  const levelDelta = (coord.y ?? 0) - (centerCoord.y ?? 0);

  let color = 0x5278a1;
  if (isCurrent) {
    color = 0xffa24a;
  } else if (levelDelta > 0) {
    color = 0x62b58a;
  } else if (levelDelta < 0) {
    color = 0x6b77c9;
  }

  const tileHeight = isCurrent ? 1.3 : ROOM_TILE_HEIGHT;
  const tile = new THREE.Mesh(
    new THREE.BoxGeometry(ROOM_TILE_SIZE, tileHeight, ROOM_TILE_SIZE),
    new THREE.MeshStandardMaterial({
      color,
      roughness: 0.5,
      metalness: 0.12,
      transparent: true,
      opacity: room.isSynthetic ? 0.58 : 0.95,
      emissive: isCurrent ? 0x5b2600 : 0x000000,
    }),
  );
  tile.position.set(localPos.x, localPos.y + tileHeight / 2, localPos.z);
  minimapGroup.add(tile);

  const edges = new THREE.LineSegments(
    new THREE.EdgesGeometry(tile.geometry),
    new THREE.LineBasicMaterial({ color: 0x0e1622, transparent: true, opacity: 0.5 }),
  );
  edges.position.copy(tile.position);
  minimapGroup.add(edges);

  drawMinimapMobIndicators(room, localPos, tileHeight);
}

function drawMinimapLink(link, roomPositions) {
  const from = roomPositions.get(String(link.from));
  const to = roomPositions.get(String(link.to));
  if (!from || !to) {
    return;
  }

  const isVertical = link.direction === "up" || link.direction === "down" || Math.abs(from.y - to.y) > 0.1;
  const points = [
    new THREE.Vector3(from.x, from.y + 0.75, from.z),
    new THREE.Vector3(to.x, to.y + 0.75, to.z),
  ];

  const line = new THREE.Line(
    new THREE.BufferGeometry().setFromPoints(points),
    new THREE.LineBasicMaterial({
      color: isVertical ? 0xffcc78 : 0x8bc8ff,
      transparent: true,
      opacity: 0.86,
    }),
  );
  minimapGroup.add(line);
}

function renderMiniMap(room, roomMapPayload) {
  clearGroup(minimapGroup);

  if (!room) {
    minimapMarker.visible = false;
    minimapPreviousCenterCoord = null;
    minimapGroup.position.set(0, 0, 0);
    return;
  }

  const mapData =
    roomMapPayload && Array.isArray(roomMapPayload.rooms) && roomMapPayload.rooms.length > 0
      ? roomMapPayload
      : buildFallbackMap(room);

  const centerRoom =
    mapData.rooms.find((entry) => Number(entry.vnum) === Number(mapData.centerVnum)) ?? room;
  const centerCoord = centerRoom.coord ?? room.coord ?? { x: 0, y: 0, z: 0 };

  const carryX = minimapGroup.position.x;
  const carryY = minimapGroup.position.y;
  const carryZ = minimapGroup.position.z;
  if (minimapPreviousCenterCoord) {
    minimapGroup.position.set(
      carryX + (centerCoord.x - minimapPreviousCenterCoord.x) * ROOM_SPACING,
      carryY + (centerCoord.y - minimapPreviousCenterCoord.y) * LEVEL_HEIGHT,
      carryZ + (centerCoord.z - minimapPreviousCenterCoord.z) * ROOM_SPACING,
    );
  } else {
    minimapGroup.position.set(0, 0, 0);
  }
  minimapPreviousCenterCoord = {
    x: centerCoord.x,
    y: centerCoord.y,
    z: centerCoord.z,
  };

  const roomPositions = new Map();
  for (const mapRoom of mapData.rooms) {
    drawMinimapRoomNode(mapRoom, centerCoord, room.vnum, roomPositions);
  }

  const seenLinks = new Set();
  for (const link of mapData.links || []) {
    const fromKey = String(link.from);
    const toKey = String(link.to);
    const linkKey =
      Number(fromKey) < Number(toKey) ? `${fromKey}:${toKey}` : `${toKey}:${fromKey}`;

    if (seenLinks.has(linkKey)) {
      continue;
    }
    seenLinks.add(linkKey);
    drawMinimapLink(link, roomPositions);
  }

  const currentPosition = roomPositions.get(String(room.vnum)) ?? new THREE.Vector3(0, 0.78, 0);
  const markerHeight = currentPosition.y + 0.78;
  minimapMarkerTargetPosition.set(currentPosition.x, markerHeight, currentPosition.z);
  if (!minimapMarker.visible) {
    minimapMarkerCurrentPosition.copy(minimapMarkerTargetPosition);
  }
  minimapMarker.position.copy(minimapMarkerCurrentPosition);
  minimapMarker.visible = true;
}

function renderUnknownRoom(roomId) {
  currentRoom = null;
  clearRoomScene();
  inputState.forward = false;
  inputState.backward = false;
  inputState.left = false;
  inputState.right = false;
  inputState.sprint = false;
  playerState.lastExitAt = performance.now();
  renderMiniMap(null, null);
  roomName.textContent = `Unknown room #${roomId}`;
  roomArea.textContent = "Area: Not parsed";
  roomSector.textContent = "Sector: Unknown";
  updateMovementButtons(null);

  if (activeInfoPanelId === "room_description") {
    setInfoPanelContent(buildRoomDescriptionPanelText(null));
  }
}

function renderKnownRoom(room, roomMapPayload) {
  currentRoom = room;

  const chosen = chooseRoomStyle(room);

  buildRoomGeometry(room, chosen.name, chosen.style, roomMapPayload);
  inputState.forward = false;
  inputState.backward = false;
  inputState.left = false;
  inputState.right = false;
  inputState.sprint = false;
  playerState.lastExitAt = performance.now();
  renderMiniMap(room, roomMapPayload);

  roomName.textContent = `${room.name} (#${room.vnum})`;
  roomArea.textContent = `Area: ${room.area || room.areaFile || "Unknown"}`;
  roomSector.textContent = `Sector: ${resolveSectorName(room.sector)} | Theme: ${toTitleCase(chosen.name)}`;

  updateMovementButtons(room);

  if (activeInfoPanelId === "room_description") {
    setInfoPanelContent(buildRoomDescriptionPanelText(room));
  }

  orbitTarget.set(playerState.position.x, CAMERA_BASE_HEIGHT, playerState.position.z);
}

function send(ws, payload) {
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify(payload));
  }
}

function attemptMove(direction) {
  const targetVnum = Number.parseInt(currentRoom?.exits?.[direction] ?? "", 10);
  if (!Number.isFinite(targetVnum) || targetVnum <= 0) {
    appendLog(`[bridge] no ${direction} exit from this room\n`);
    return;
  }

  send(socket, { type: "move", direction });
}

function startMoveAnimation(duration) {
  movePulse = Math.max(movePulse, Math.min(1, (duration || 260) / 260));
}

function animate(now) {
  requestAnimationFrame(animate);

  const deltaSeconds = Math.min(0.08, Math.max(0.001, (now - frameClock) / 1000));
  frameClock = now;

  updatePlayerMovement(deltaSeconds, now);
  for (const actor of actorState.npcs) {
    updateNpcActor(actor, deltaSeconds);
  }

  const damping = 1 - Math.exp(-CAMERA_DAMPING * deltaSeconds);
  orbitState.distance += (orbitState.desiredDistance - orbitState.distance) * damping;
  orbitCurrentTarget.lerp(orbitTarget, damping);

  const horizontal = Math.cos(orbitState.pitch) * orbitState.distance;
  desiredCameraPosition.set(
    orbitCurrentTarget.x + Math.sin(orbitState.yaw) * horizontal,
    orbitCurrentTarget.y + Math.sin(orbitState.pitch) * orbitState.distance + CAMERA_BASE_HEIGHT * 0.18,
    orbitCurrentTarget.z + Math.cos(orbitState.yaw) * horizontal,
  );

  camera.position.lerp(desiredCameraPosition, damping);
  camera.lookAt(orbitCurrentTarget);

  movePulse = Math.max(0, movePulse - deltaSeconds * 1.75);

  accentLight.intensity = 0.4 + movePulse * 0.35;

  const mapLerp = 1 - Math.exp(-MAP_TRANSITION_SPEED * deltaSeconds);
  minimapGroup.position.multiplyScalar(1 - mapLerp);
  if (minimapGroup.position.lengthSq() < 0.0001) {
    minimapGroup.position.set(0, 0, 0);
  }

  if (minimapMarker.visible) {
    const markerLerp = 1 - Math.exp(-MARKER_FOLLOW_SPEED * deltaSeconds);
    minimapMarkerCurrentPosition.lerp(minimapMarkerTargetPosition, markerLerp);
    const idlePulse = 1 + Math.sin(now * 0.006) * 0.05;
    const boostedScale = idlePulse + movePulse * 0.2;
    const hover = Math.sin(now * 0.004) * 0.07;
    minimapMarker.position.set(
      minimapMarkerCurrentPosition.x,
      minimapMarkerCurrentPosition.y + hover,
      minimapMarkerCurrentPosition.z,
    );
    minimapMarker.scale.set(boostedScale, boostedScale, boostedScale);
    minimapMarker.material.opacity = Math.min(1, 0.78 + movePulse * 0.35);
  }

  if (panelCapture && now >= panelCapture.expiresAt) {
    panelCapture = null;
  }

  renderer.render(scene, camera);
  minimapRenderer.render(minimapScene, minimapCamera);
}
sceneCanvas.style.cursor = "grab";

sceneCanvas.addEventListener("pointerdown", (event) => {
  if (event.button !== 0) {
    return;
  }

  orbitState.dragging = true;
  orbitState.pointerId = event.pointerId;
  orbitState.lastX = event.clientX;
  orbitState.lastY = event.clientY;

  sceneCanvas.setPointerCapture(event.pointerId);
  sceneCanvas.style.cursor = "grabbing";
});

sceneCanvas.addEventListener("pointermove", (event) => {
  if (orbitState.dragging && event.pointerId === orbitState.pointerId) {
    const dx = event.clientX - orbitState.lastX;
    const dy = event.clientY - orbitState.lastY;

    orbitState.yaw -= dx * CAMERA_ROTATE_SPEED;
    orbitState.pitch = clamp(orbitState.pitch - dy * CAMERA_ROTATE_SPEED, -0.2, 1.18);

    orbitState.lastX = event.clientX;
    orbitState.lastY = event.clientY;
  }
});

function finishPointerInteraction(event) {
  if (!orbitState.dragging || event.pointerId !== orbitState.pointerId) {
    return;
  }

  orbitState.dragging = false;
  orbitState.pointerId = -1;

  if (sceneCanvas.hasPointerCapture(event.pointerId)) {
    sceneCanvas.releasePointerCapture(event.pointerId);
  }

  sceneCanvas.style.cursor = "grab";
}

sceneCanvas.addEventListener("pointerup", finishPointerInteraction);
sceneCanvas.addEventListener("pointercancel", finishPointerInteraction);

sceneCanvas.addEventListener("pointerleave", () => {
  if (!orbitState.dragging) {
    sceneCanvas.style.cursor = "grab";
  }
});

sceneCanvas.addEventListener(
  "wheel",
  (event) => {
    event.preventDefault();
    orbitState.desiredDistance = clamp(
      orbitState.desiredDistance + event.deltaY * CAMERA_ZOOM_SPEED,
      CAMERA_MIN_DISTANCE,
      CAMERA_MAX_DISTANCE,
    );
  },
  { passive: false },
);

window.addEventListener("resize", () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
  updateMinimapProjection();
});

const wsProtocol = window.location.protocol === "https:" ? "wss" : "ws";
const socket = new WebSocket(`${wsProtocol}://${window.location.host}`);

socket.addEventListener("open", () => {
  setConnectionLabel("Bridge Online", "var(--move)");
  appendLog("[bridge] connected\n");
});

socket.addEventListener("close", () => {
  setConnectionLabel("Disconnected", "var(--hp)");
  appendLog("[bridge] disconnected\n");
});

socket.addEventListener("error", () => {
  setConnectionLabel("Error", "var(--hp)");
  appendLog("[bridge] socket error\n");
});

socket.addEventListener("message", (event) => {
  let payload;
  try {
    payload = JSON.parse(event.data);
  } catch {
    return;
  }

  switch (payload.type) {
    case "server_info":
      appendLog(`[bridge] ${payload.message}\n`);
      break;
    case "server_error":
      appendLog(`[error] ${payload.message}\n`);
      break;
    case "mud_text":
      appendLog(payload.text);
      updateImmortalStatusFromText(payload.text);
      capturePanelText(payload.text);
      break;
    case "stats_update":
      updateMeter(hpFill, hpText, payload.hpPct ?? 0);
      updateMeter(manaFill, manaText, payload.manaPct ?? 0);
      updateMeter(moveFill, moveText, payload.movePct ?? 0);
      break;
    case "player_move":
      startMoveAnimation(payload.duration);
      break;
    case "room_change":
      if (payload.room) {
        renderKnownRoom(payload.room, payload.roomMap);
      } else {
        renderUnknownRoom(payload.roomId);
      }
      break;
    default:
      break;
  }
});
commandForm.addEventListener("submit", (event) => {
  event.preventDefault();
  const command = commandInput.value.trim();
  if (!command) {
    return;
  }
  send(socket, { type: "mud_command", command });
  commandInput.value = "";
});

protocolButton.addEventListener("click", () => {
  send(socket, { type: "enable_protocol" });
});

refreshButton.addEventListener("click", () => {
  send(socket, { type: "refresh_room" });
});

for (const [direction, button] of moveButtonsByDirection.entries()) {
  button.addEventListener("click", () => {
    attemptMove(direction);
  });
}

for (const button of panelOpenButtons) {
  button.addEventListener("click", () => {
    const panelId = button.dataset.panelId;
    if (!panelId) {
      return;
    }
    openInfoPanel(panelId);
  });
}

infoPanelRefresh.addEventListener("click", () => {
  refreshInfoPanel();
});

infoPanelClose.addEventListener("click", () => {
  closeInfoPanel();
});

window.addEventListener("keydown", (event) => {
  if (
    document.activeElement === commandInput ||
    document.activeElement?.tagName === "INPUT" ||
    document.activeElement?.tagName === "TEXTAREA"
  ) {
    return;
  }

  if (event.key === "Escape" && !infoPanel.classList.contains("hidden")) {
    event.preventDefault();
    closeInfoPanel();
    return;
  }

  const normalizedKey = event.key.length === 1 ? event.key.toLowerCase() : event.key;
  const localMoveByKey = {
    w: "forward",
    s: "backward",
    a: "right",
    d: "left",
    Shift: "sprint",
  };
  const localMove = localMoveByKey[normalizedKey];
  if (localMove) {
    event.preventDefault();
    inputState[localMove] = true;
    return;
  }

  const exitByKey = {
    ArrowUp: "north",
    ArrowRight: "east",
    ArrowDown: "south",
    ArrowLeft: "west",
    q: "up",
    z: "down",
  };

  const direction = exitByKey[normalizedKey];
  if (!direction) {
    return;
  }

  event.preventDefault();
  attemptMove(direction);
});

window.addEventListener("keyup", (event) => {
  const normalizedKey = event.key.length === 1 ? event.key.toLowerCase() : event.key;
  const localMoveByKey = {
    w: "forward",
    s: "backward",
    a: "right",
    d: "left",
    Shift: "sprint",
  };
  const localMove = localMoveByKey[normalizedKey];
  if (!localMove) {
    return;
  }
  inputState[localMove] = false;
});

setImmortalState(false);
updatePanelButtonSelection();
setInfoPanelContent("Select an information panel to view details.");
updateMinimapProjection();
requestAnimationFrame(() => {
  updateMinimapProjection();
});
updateMovementButtons(null);
animate(performance.now());
