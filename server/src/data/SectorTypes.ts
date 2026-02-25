export interface SectorDefinition {
  id: number;
  name: string;
  color: string;
  moveCost: number;
  flags: Set<string>;
}

const SECTOR_DATA: Array<[number, string, string, number, string[]]> = [
  [0, "Inside", "#8B8682", 1, ["shelter"]],
  [1, "City", "#C0B8A0", 1, ["shelter"]],
  [2, "Field", "#7CCD7C", 2, []],
  [3, "Forest", "#228B22", 3, ["cover"]],
  [4, "Hills", "#BDB76B", 4, []],
  [5, "Mountain", "#8B7355", 6, ["difficult"]],
  [6, "Water (Swim)", "#4682B4", 4, ["water", "swim"]],
  [7, "Water (No Swim)", "#191970", 6, ["water", "deep_water"]],
  [8, "Underwater", "#00008B", 8, ["water", "underwater"]],
  [9, "Air", "#87CEEB", 4, ["flying"]],
  [10, "Desert", "#EDC9AF", 5, ["hot"]],
  [11, "Graveyard", "#696969", 2, ["undead"]],
  [12, "Underground", "#4A4A4A", 2, ["dark"]],
  [13, "Polar", "#F0F8FF", 4, ["cold"]],
  [14, "Swamp", "#556B2F", 4, ["poison", "difficult"]],
  [15, "Jungle", "#006400", 4, ["cover", "difficult"]],
  [16, "Road", "#D2B48C", 1, []],
  [17, "Beach", "#F5DEB3", 2, []],
  [18, "Lava", "#FF4500", 10, ["fire", "lethal"]],
  [19, "Snow", "#FFFAFA", 3, ["cold"]],
  [20, "Ruins", "#A0522D", 2, ["cover"]],
  [21, "Bridge", "#DEB887", 1, []],
  [22, "Cave", "#3B3B3B", 2, ["dark", "shelter"]],
  [23, "Arena", "#CD853F", 1, ["pvp"]],
  [24, "Temple", "#FFD700", 1, ["holy", "shelter"]],
  [25, "Tower", "#B0C4DE", 1, ["shelter"]],
  [26, "Dungeon", "#2F2F2F", 2, ["dark", "shelter"]],
  [27, "Corruption", "#8B0000", 3, ["evil"]],
  [28, "Death", "#1C1C1C", 3, ["undead", "evil"]],
  [29, "Blessing", "#FFFACD", 2, ["holy"]],
  [30, "Knowledge", "#DAA520", 1, ["shelter"]],
  [31, "Virtue", "#F0E68C", 2, ["holy"]],
  [32, "Justice", "#C0C0C0", 1, ["shelter"]],
  [33, "Life", "#90EE90", 2, ["holy", "regen"]],
  [34, "Storm", "#483D8B", 4, ["lightning"]],
  [35, "Nature", "#32CD32", 2, ["regen"]],
  [36, "Flame", "#FF6347", 5, ["fire"]],
  [37, "Portal", "#9400D3", 1, ["magical"]],
  [38, "Tavern", "#CD853F", 1, ["shelter", "regen"]],
  [39, "Shop", "#DAA520", 1, ["shelter"]],
  [40, "Guild", "#B8860B", 1, ["shelter"]],
  [41, "Dock", "#8FBC8F", 1, []],
  [42, "Ship", "#D2691E", 1, ["shelter"]],
  [43, "Throne", "#FFD700", 1, ["shelter"]],
  [44, "Garden", "#98FB98", 1, ["regen"]],
  [45, "Crypt", "#2F4F4F", 2, ["dark", "undead"]],
  [46, "Battlefield", "#8B4513", 2, ["pvp"]],
];

export const SectorTypes = new Map<number, SectorDefinition>();

for (const [id, name, color, moveCost, flags] of SECTOR_DATA) {
  SectorTypes.set(id, { id, name, color, moveCost, flags: new Set(flags) });
}

export function getSector(id: number): SectorDefinition {
  return SectorTypes.get(id) ?? { id, name: "Unknown", color: "#808080", moveCost: 2, flags: new Set() };
}
