import fs from "node:fs";
import path from "node:path";
import crypto from "node:crypto";
import { Config } from "../config.js";
import { Player, PlayerClass, PlayerRace } from "../entities/Player.js";

interface AccountRecord {
  id: string;
  username: string;
  passwordHash: string;
  salt: string;
  characterName: string;
  playerClass: PlayerClass;
  race: PlayerRace;
  level: number;
  xp: number;
  xpToLevel: number;
  gold: number;
  bank: number;
  kills: number;
  deaths: number;
  roomVnum: number;
  stats: Record<string, number>;
  createdAt: string;
  lastLogin: string;
}

const VALID_CLASSES = new Set(Object.values(PlayerClass));
const VALID_RACES = new Set(Object.values(PlayerRace));

export class AuthManager {
  private accounts: Map<string, AccountRecord> = new Map(); // username -> record
  private dataDir: string;

  constructor() {
    this.dataDir = path.resolve(Config.world.playerDir, "..", "accounts");
  }

  initialize(): void {
    if (!fs.existsSync(this.dataDir)) {
      fs.mkdirSync(this.dataDir, { recursive: true });
    }

    // Load existing accounts
    const files = fs.readdirSync(this.dataDir).filter((f) => f.endsWith(".json"));
    for (const file of files) {
      try {
        const raw = fs.readFileSync(path.join(this.dataDir, file), "utf8");
        const record = JSON.parse(raw) as AccountRecord;
        this.accounts.set(record.username.toLowerCase(), record);
      } catch (err) {
        console.error(`[Auth] Failed to load account ${file}:`, err);
      }
    }

    console.log(`[Auth] Loaded ${this.accounts.size} accounts`);
  }

  async register(
    username: string,
    password: string,
    characterName: string,
    playerClass: string,
    race: string,
    startRoom: number,
  ): Promise<{ success: boolean; reason?: string }> {
    // Validate
    const uname = username.trim().toLowerCase();
    const cname = characterName.trim();

    if (uname.length < 3 || uname.length > 20) {
      return { success: false, reason: "Username must be 3-20 characters." };
    }

    if (!/^[a-z0-9_]+$/.test(uname)) {
      return { success: false, reason: "Username must be alphanumeric." };
    }

    if (password.length < Config.auth.minPasswordLength) {
      return { success: false, reason: `Password must be at least ${Config.auth.minPasswordLength} characters.` };
    }

    if (cname.length < 2 || cname.length > Config.auth.maxNameLength) {
      return { success: false, reason: `Character name must be 2-${Config.auth.maxNameLength} characters.` };
    }

    if (!/^[A-Za-z]+$/.test(cname)) {
      return { success: false, reason: "Character name must contain only letters." };
    }

    if (!VALID_CLASSES.has(playerClass as PlayerClass)) {
      return { success: false, reason: `Invalid class. Choose from: ${[...VALID_CLASSES].join(", ")}` };
    }

    if (!VALID_RACES.has(race as PlayerRace)) {
      return { success: false, reason: `Invalid race. Choose from: ${[...VALID_RACES].join(", ")}` };
    }

    if (this.accounts.has(uname)) {
      return { success: false, reason: "Username already taken." };
    }

    // Check if character name is taken
    for (const account of this.accounts.values()) {
      if (account.characterName.toLowerCase() === cname.toLowerCase()) {
        return { success: false, reason: "Character name already taken." };
      }
    }

    // Hash password
    const salt = crypto.randomBytes(16).toString("hex");
    const passwordHash = this.hashPassword(password, salt);

    const record: AccountRecord = {
      id: crypto.randomUUID(),
      username: uname,
      passwordHash,
      salt,
      characterName: cname,
      playerClass: playerClass as PlayerClass,
      race: race as PlayerRace,
      level: 1,
      xp: 0,
      xpToLevel: 1000,
      gold: 100,
      bank: 0,
      kills: 0,
      deaths: 0,
      roomVnum: startRoom,
      stats: {},
      createdAt: new Date().toISOString(),
      lastLogin: new Date().toISOString(),
    };

    this.accounts.set(uname, record);
    this.saveAccount(record);

    console.log(`[Auth] New account registered: ${uname} (${cname} the ${race} ${playerClass})`);
    return { success: true };
  }

  async login(username: string, password: string): Promise<{ success: boolean; player?: Player; reason?: string }> {
    const uname = username.trim().toLowerCase();
    const record = this.accounts.get(uname);

    if (!record) {
      return { success: false, reason: "Account not found." };
    }

    const hash = this.hashPassword(password, record.salt);
    if (hash !== record.passwordHash) {
      return { success: false, reason: "Invalid password." };
    }

    // Create player from record
    const player = new Player(
      record.id,
      record.characterName,
      record.playerClass,
      record.race,
    );

    player.level = record.level;
    player.xp = record.xp;
    player.xpToLevel = record.xpToLevel;
    player.gold = record.gold;
    player.bank = record.bank;
    player.kills = record.kills;
    player.deaths = record.deaths;
    player.roomVnum = record.roomVnum;

    if (record.stats && Object.keys(record.stats).length > 0) {
      Object.assign(player.stats, record.stats);
    }

    player.recalculateCombat();
    player.combat.hp = player.combat.maxHp;
    player.combat.mana = player.combat.maxMana;
    player.combat.move = player.combat.maxMove;

    // Update last login
    record.lastLogin = new Date().toISOString();
    this.saveAccount(record);

    return { success: true, player };
  }

  /** Save player state back to account file */
  savePlayer(player: Player): void {
    // Find account by ID
    for (const [, record] of this.accounts) {
      if (record.id === player.accountId) {
        record.level = player.level;
        record.xp = player.xp;
        record.xpToLevel = player.xpToLevel;
        record.gold = player.gold;
        record.bank = player.bank;
        record.kills = player.kills;
        record.deaths = player.deaths;
        record.roomVnum = player.roomVnum;
        record.stats = { ...player.stats };
        this.saveAccount(record);
        return;
      }
    }
  }

  private hashPassword(password: string, salt: string): string {
    return crypto.pbkdf2Sync(password, salt, 10000, 64, "sha512").toString("hex");
  }

  private saveAccount(record: AccountRecord): void {
    const filePath = path.join(this.dataDir, `${record.username}.json`);
    fs.writeFileSync(filePath, JSON.stringify(record, null, 2));
  }
}
