/**
 * MMORPG WebSocket Protocol
 *
 * All messages are JSON with a "type" field.
 * Client -> Server messages start with "c_"
 * Server -> Client messages start with "s_"
 */

// ============================================================
// Client -> Server Messages
// ============================================================

export interface C_Login {
  type: "c_login";
  username: string;
  password: string;
}

export interface C_Register {
  type: "c_register";
  username: string;
  password: string;
  characterName: string;
  playerClass: string;
  race: string;
}

export interface C_Move {
  type: "c_move";
  direction: string;
}

export interface C_Look {
  type: "c_look";
  target?: string;
}

export interface C_Attack {
  type: "c_attack";
  targetId: string;
}

export interface C_UseSkill {
  type: "c_use_skill";
  skillName: string;
  targetId?: string;
}

export interface C_Chat {
  type: "c_chat";
  channel: string;
  message: string;
  targetName?: string;
}

export interface C_PickupItem {
  type: "c_pickup_item";
  itemId: string;
}

export interface C_DropItem {
  type: "c_drop_item";
  itemId: string;
}

export interface C_EquipItem {
  type: "c_equip_item";
  itemId: string;
}

export interface C_UnequipItem {
  type: "c_unequip_item";
  slot: string;
}

export interface C_Interact {
  type: "c_interact";
  npcId: string;
  action: string;
}

export interface C_Ping {
  type: "c_ping";
  timestamp: number;
}

export interface C_Respawn {
  type: "c_respawn";
}

export type ClientMessage =
  | C_Login
  | C_Register
  | C_Move
  | C_Look
  | C_Attack
  | C_UseSkill
  | C_Chat
  | C_PickupItem
  | C_DropItem
  | C_EquipItem
  | C_UnequipItem
  | C_Interact
  | C_Ping
  | C_Respawn;

// ============================================================
// Server -> Client Messages
// ============================================================

export interface S_LoginSuccess {
  type: "s_login_success";
  player: Record<string, unknown>;
  room: Record<string, unknown>;
  roomMap: Record<string, unknown>;
}

export interface S_LoginFailed {
  type: "s_login_failed";
  reason: string;
}

export interface S_RegisterSuccess {
  type: "s_register_success";
  message: string;
}

export interface S_RegisterFailed {
  type: "s_register_failed";
  reason: string;
}

export interface S_RoomChange {
  type: "s_room_change";
  room: Record<string, unknown>;
  roomMap: Record<string, unknown>;
  entities: Array<Record<string, unknown>>;
  items: Array<Record<string, unknown>>;
}

export interface S_EntityEnter {
  type: "s_entity_enter";
  entity: Record<string, unknown>;
  direction?: string;
}

export interface S_EntityLeave {
  type: "s_entity_leave";
  entityId: string;
  entityName: string;
  direction?: string;
}

export interface S_PlayerUpdate {
  type: "s_player_update";
  player: Record<string, unknown>;
}

export interface S_CombatAction {
  type: "s_combat_action";
  attackerId: string;
  attackerName: string;
  targetId: string;
  targetName: string;
  damage: number;
  skill: string;
  targetHpPercent: number;
}

export interface S_EntityDeath {
  type: "s_entity_death";
  entityId: string;
  entityName: string;
  killerId: string;
  killerName: string;
  xpGained?: number;
  goldGained?: number;
  loot?: Array<Record<string, unknown>>;
}

export interface S_LevelUp {
  type: "s_level_up";
  newLevel: number;
  player: Record<string, unknown>;
}

export interface S_Chat {
  type: "s_chat";
  channel: string;
  senderName: string;
  message: string;
  timestamp: number;
}

export interface S_SystemMessage {
  type: "s_system_message";
  message: string;
  severity: "info" | "warning" | "error";
}

export interface S_Pong {
  type: "s_pong";
  clientTimestamp: number;
  serverTimestamp: number;
}

export interface S_SpawnEffect {
  type: "s_spawn_effect";
  entityId: string;
  entity: Record<string, unknown>;
}

export interface S_WorldInfo {
  type: "s_world_info";
  playerCount: number;
  npcCount: number;
  roomCount: number;
  zoneCount: number;
  uptime: number;
}

export type ServerMessage =
  | S_LoginSuccess
  | S_LoginFailed
  | S_RegisterSuccess
  | S_RegisterFailed
  | S_RoomChange
  | S_EntityEnter
  | S_EntityLeave
  | S_PlayerUpdate
  | S_CombatAction
  | S_EntityDeath
  | S_LevelUp
  | S_Chat
  | S_SystemMessage
  | S_Pong
  | S_SpawnEffect
  | S_WorldInfo;
