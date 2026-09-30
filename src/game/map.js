// Osaka overworld. 40 x 30 tiles of 16px.
export const MAP_W = 40;
export const MAP_H = 30;

export const T = {
  GRASS: 0,
  ROAD: 1,
  PAVE: 2,
  WATER: 3,
  TREE: 4,
  FLOWER: 5,
  BRIDGE: 6,
  SOLID: 7,
  DOOR: 8,
  CROSS: 9,
};

const WALKABLE = new Set([T.GRASS, T.ROAD, T.PAVE, T.FLOWER, T.BRIDGE, T.DOOR, T.CROSS]);

// Buildings. `door` is the tile you walk into; buildings without a door are landmarks you can read.
export const BUILDINGS = [
  {
    id: "consulate",
    name: "U.S. Consulate General Osaka-Kobe",
    short: "U.S. Consulate",
    x: 3, y: 2, w: 9, h: 5,
    door: { x: 7, y: 6 },
    style: "consulate",
  },
  {
    id: "station",
    name: "Osaka Station",
    short: "Osaka Station",
    x: 26, y: 2, w: 10, h: 5,
    door: { x: 30, y: 6 },
    style: "station",
  },
  {
    id: "home",
    name: "Home",
    short: "Home",
    x: 2, y: 11, w: 6, h: 4,
    door: { x: 4, y: 14 },
    style: "home",
  },
  {
    id: "cafe",
    name: "Cafe Nomado",
    short: "Cafe Nomado",
    x: 12, y: 11, w: 6, h: 4,
    door: { x: 14, y: 14 },
    style: "cafe",
  },
  {
    id: "prints",
    name: "Fingerprint Service Osaka",
    short: "Fingerprint Service",
    x: 3, y: 20, w: 8, h: 5,
    door: { x: 6, y: 24 },
    style: "prints",
  },
  {
    id: "castle",
    name: "Osaka Castle",
    short: "Osaka Castle",
    x: 30, y: 11, w: 5, h: 4,
    door: null,
    style: "castle",
    sign: "Osaka Castle. Tall walls, taller paperwork piles back home.",
  },
  {
    id: "tower",
    name: "Tower",
    short: "Tower",
    x: 30, y: 21, w: 3, h: 4,
    door: null,
    style: "tower",
    sign: "An old steel tower over the south side. From the top you can almost see Valencia. Almost.",
  },
  {
    id: "takoyaki",
    name: "Takoyaki stand",
    short: "Takoyaki",
    x: 14, y: 21, w: 4, h: 2,
    door: null,
    style: "stand",
    sign: "Takoyaki, 8 pieces. Fuel for paperwork.",
  },
];

export const NPCS = [
  {
    id: "salaryman",
    x: 19, y: 15,
    look: "salaryman",
    lines: [
      "Social security certificate? My company files ours through HR too.",
      "It stops you paying into two systems at once. Worth chasing!",
    ],
  },
  {
    id: "obaachan",
    x: 7, y: 19,
    look: "obaachan",
    lines: [
      "Spain? Ara, how exciting.",
      "Bring me back some jamon. And eat properly over there!",
    ],
  },
  {
    id: "traveler",
    x: 33, y: 8,
    look: "traveler",
    lines: [
      "Trains to Kansai Airport leave from Osaka Station.",
      "Finish your Osaka list first. The airport won't wait for paperwork.",
    ],
  },
  {
    id: "vendor",
    x: 15, y: 23,
    look: "vendor",
    lines: ["Irasshai! Hot takoyaki!", "Careful, the inside is lava."],
  },
];

const TREES = [
  [27, 11], [28, 12], [36, 11], [37, 13], [27, 15], [36, 15], [29, 15], [35, 15],
  [1, 12], [9, 11], [9, 13], [19, 11], [20, 13],
  [12, 20], [19, 21], [26, 20], [27, 23], [35, 21], [36, 24], [13, 25], [18, 25],
  [2, 25], [11, 23],
];

const FLOWERS = [
  [28, 13], [29, 13], [35, 12], [36, 13], [33, 16], [31, 16],
  [3, 15], [6, 15], [12, 15], [17, 15],
  [4, 25], [8, 25], [29, 25], [33, 25],
];

function build() {
  const g = Array.from({ length: MAP_H }, () => Array(MAP_W).fill(T.GRASS));
  const rect = (x, y, w, h, t) => {
    for (let j = y; j < y + h; j++) {
      for (let i = x; i < x + w; i++) {
        if (j >= 0 && j < MAP_H && i >= 0 && i < MAP_W) g[j][i] = t;
      }
    }
  };

  // Streets
  rect(1, 7, 38, 1, T.PAVE);
  rect(1, 8, 38, 2, T.ROAD);
  rect(1, 10, 38, 1, T.PAVE);
  rect(21, 2, 1, 26, T.PAVE);
  rect(24, 2, 1, 26, T.PAVE);
  rect(22, 2, 2, 26, T.ROAD);
  rect(1, 26, 38, 1, T.PAVE);
  rect(1, 27, 38, 1, T.ROAD);
  rect(12, 2, 9, 5, T.PAVE); // Umeda plaza
  rect(1, 2, 2, 5, T.PAVE);
  rect(36, 2, 3, 5, T.PAVE);
  rect(25, 2, 1, 5, T.PAVE);

  // Crosswalks
  rect(10, 8, 1, 2, T.CROSS);
  rect(30, 8, 1, 2, T.CROSS);
  rect(22, 11, 2, 1, T.CROSS);
  rect(22, 25, 2, 1, T.CROSS);

  // Dotonbori canal with walkways and bridges
  rect(1, 16, 38, 1, T.PAVE);
  rect(1, 19, 38, 1, T.PAVE);
  rect(1, 17, 38, 2, T.WATER);
  rect(9, 17, 3, 2, T.BRIDGE);
  rect(21, 17, 4, 2, T.BRIDGE);
  rect(31, 17, 2, 2, T.BRIDGE);

  // Border trees
  rect(0, 0, MAP_W, 2, T.TREE);
  rect(0, 28, MAP_W, 2, T.TREE);
  rect(0, 0, 1, MAP_H, T.TREE);
  rect(MAP_W - 1, 0, 1, MAP_H, T.TREE);

  for (const [x, y] of TREES) g[y][x] = T.TREE;
  for (const [x, y] of FLOWERS) g[y][x] = T.FLOWER;

  for (const b of BUILDINGS) {
    rect(b.x, b.y, b.w, b.h, T.SOLID);
    if (b.door) g[b.door.y][b.door.x] = T.DOOR;
  }
  return g;
}

export const GRID = build();

export function tileAt(x, y) {
  if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return T.TREE;
  return GRID[y][x];
}

export function isWalkable(x, y) {
  return WALKABLE.has(tileAt(x, y)) && !NPCS.some((n) => n.x === x && n.y === y);
}

export function buildingAt(x, y) {
  return BUILDINGS.find((b) => x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h) || null;
}

export function doorAt(x, y) {
  return BUILDINGS.find((b) => b.door && b.door.x === x && b.door.y === y) || null;
}

export function npcAt(x, y) {
  return NPCS.find((n) => n.x === x && n.y === y) || null;
}

export const START = { x: 4, y: 15, facing: "down" };
