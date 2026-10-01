"""Osaka overworld layout: 32 x 32 metatiles of 16px (one 512x512 GBA background)."""

MAP_W = MAP_H = 32
GRASS, ROAD, PAVE, WATER, TREE, FLOWER, BRIDGE, SOLID, DOOR, CROSS = range(10)
WALKABLE = {GRASS, ROAD, PAVE, FLOWER, BRIDGE, DOOR, CROSS}

BUILDINGS = [
    dict(id="consulate", name="U.S. Consulate", x=2, y=2, w=8, h=5, door=(5, 6), style="consulate"),
    dict(id="station", name="Osaka Station", x=20, y=2, w=9, h=5, door=(24, 6), style="station"),
    dict(id="home", name="Home", x=2, y=11, w=5, h=4, door=(4, 14), style="home"),
    dict(id="cafe", name="Cafe Nomado", x=8, y=11, w=5, h=4, door=(10, 14), style="cafe"),
    dict(id="prints", name="Fingerprint Service", x=2, y=20, w=7, h=5, door=(5, 24), style="prints"),
    dict(id="castle", name="Osaka Castle", x=22, y=11, w=5, h=4, door=None, style="castle"),
    dict(id="tower", name="Tower", x=25, y=21, w=3, h=4, door=None, style="tower"),
    dict(id="takoyaki", name="Takoyaki stand", x=10, y=21, w=4, h=2, door=None, style="stand"),
]

# id, x, y, look
NPCS = [
    ("salaryman", 13, 15, "salaryman"),
    ("obaachan", 3, 19, "obaachan"),
    ("traveler", 28, 7, "traveler"),
    ("vendor", 11, 23, "vendor"),
]

TREES = [(7, 11), (14, 12), (20, 11), (21, 13), (27, 11), (28, 13), (20, 15), (28, 15), (1, 12),
         (9, 20), (14, 21), (20, 20), (22, 23), (29, 21), (13, 25), (1, 25), (29, 24)]
FLOWERS = [(21, 12), (27, 12), (3, 15), (6, 15), (9, 15), (12, 15), (23, 15), (25, 15),
           (3, 25), (7, 25), (24, 25), (28, 25), (21, 21)]

START = (4, 15)


def build():
    g = [[GRASS] * MAP_W for _ in range(MAP_H)]

    def rect(x, y, w, h, t):
        for j in range(y, y + h):
            for i in range(x, x + w):
                if 0 <= i < MAP_W and 0 <= j < MAP_H:
                    g[j][i] = t

    rect(1, 7, 30, 1, PAVE)
    rect(1, 8, 30, 2, ROAD)
    rect(1, 10, 30, 1, PAVE)
    rect(15, 2, 1, 28, PAVE)
    rect(18, 2, 1, 28, PAVE)
    rect(16, 2, 2, 28, ROAD)
    rect(1, 26, 30, 1, PAVE)
    rect(1, 27, 30, 1, ROAD)
    rect(1, 28, 30, 1, PAVE)
    rect(10, 2, 5, 5, PAVE)
    rect(1, 2, 1, 5, PAVE)
    rect(19, 2, 1, 5, PAVE)
    rect(29, 2, 2, 5, PAVE)
    rect(8, 8, 1, 2, CROSS)
    rect(24, 8, 1, 2, CROSS)
    rect(16, 11, 2, 1, CROSS)
    rect(16, 25, 2, 1, CROSS)
    # Dotonbori canal
    rect(1, 16, 30, 1, PAVE)
    rect(1, 19, 30, 1, PAVE)
    rect(1, 17, 30, 2, WATER)
    rect(6, 17, 2, 2, BRIDGE)
    rect(15, 17, 4, 2, BRIDGE)
    rect(26, 17, 2, 2, BRIDGE)
    # Border
    rect(0, 0, MAP_W, 2, TREE)
    rect(0, 30, MAP_W, 2, TREE)
    rect(0, 0, 1, MAP_H, TREE)
    rect(MAP_W - 1, 0, 1, MAP_H, TREE)
    for x, y in TREES:
        g[y][x] = TREE
    for x, y in FLOWERS:
        g[y][x] = FLOWER
    for b in BUILDINGS:
        rect(b["x"], b["y"], b["w"], b["h"], SOLID)
        if b["door"]:
            dx, dy = b["door"]
            g[dy][dx] = DOOR
    return g


GRID = build()


def check_reachable():
    npc_spots = {(x, y) for _, x, y, _ in NPCS}
    seen = {START}
    todo = [START]
    while todo:
        x, y = todo.pop()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if n in seen or not (0 <= n[0] < MAP_W and 0 <= n[1] < MAP_H):
                continue
            if GRID[n[1]][n[0]] in WALKABLE and n not in npc_spots:
                seen.add(n)
                todo.append(n)
    for b in BUILDINGS:
        if b["door"]:
            assert b["door"] in seen, f"{b['id']} door unreachable"
    for _, x, y, _ in NPCS:
        assert GRID[y][x] in WALKABLE, "npc on solid tile"
