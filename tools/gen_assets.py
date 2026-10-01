"""Builds every graphic and the map data into build/assets.c and build/assets.h."""
import os
import sys

from PIL import Image

import art
import sprite_data as S
import world as W
from gbatiles import build_bg, to_sbb_order, c_array, rgb555, encode_tile

OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..", "build")
os.makedirs(OUT, exist_ok=True)

MAP_PALS = 12  # banks 0-11; 12-15 belong to the text layer
c_src = ['#include "assets.h"\n']
h_src = ["#ifndef ASSETS_H\n#define ASSETS_H\n#include <stdint.h>\n"]


def emit_u8(name, data):
    c_src.append(c_array(name, data))
    h_src.append(f"extern const uint8_t {name}[{len(data)}];\n")


def emit_u16(name, data):
    c_src.append(c_array(name, data, "uint16_t", 12, "0x{:04X}"))
    h_src.append(f"extern const uint16_t {name}[{len(data)}];\n")


def define(name, value):
    h_src.append(f"#define {name} {value}\n")


def palettes_u16(pals, count=None):
    out = []
    for p in pals:
        p = list(p) + [0] * (16 - len(p))
        out.extend(p[:16])
    return out


def emit_bg(name, img, max_tiles=1024):
    tiles, pals, entries, tw, th = build_bg(img, MAP_PALS, max_tiles=max_tiles)
    emit_u8(f"{name}_tiles", tiles)
    emit_u16(f"{name}_pal", palettes_u16(pals))
    emit_u16(f"{name}_map", to_sbb_order(entries, tw, th))
    print(f"{name}: {len(tiles) // 32} tiles, {len(pals)} palettes", file=sys.stderr)
    return pals


def spark_slots(pals):
    a, b = rgb555(art.hex_rgb(art.SPARK_A)), rgb555(art.hex_rgb(art.SPARK_B))
    slots_a = [bank * 16 + i for bank, p in enumerate(pals) for i, c in enumerate(p) if c == a and i]
    slots_b = [bank * 16 + i for bank, p in enumerate(pals) for i, c in enumerate(p) if c == b and i]
    return slots_a, slots_b


# ---------- backgrounds ----------
W.check_reachable()
ow_pals = emit_bg("bg_overworld", art.overworld())
sa, sb = spark_slots(ow_pals)
emit_u16("ow_spark_a", sa or [0])
emit_u16("ow_spark_b", sb or [0])
define("OW_SPARK_A_COUNT", len(sa))
define("OW_SPARK_B_COUNT", len(sb))
define("SPARK_A_COLOR", hex(rgb555(art.hex_rgb(art.SPARK_A))))
define("SPARK_B_COLOR", hex(rgb555(art.hex_rgb(art.SPARK_B))))

title_pals = emit_bg("bg_title", art.title())
ta, tb = spark_slots(title_pals)
emit_u16("title_spark_a", ta or [0])
emit_u16("title_spark_b", tb or [0])
define("TITLE_SPARK_A_COUNT", len(ta))
define("TITLE_SPARK_B_COUNT", len(tb))

for name in ["consulate", "cafe", "home", "prints", "station"]:
    emit_bg(f"bg_in_{name}", art.interior(name))
    x, y, facing = art.INTERIOR_PLAYER[name]
    define(f"IN_{name.upper()}_PX", x)
    define(f"IN_{name.upper()}_PY", y)
    define(f"IN_{name.upper()}_FACE", {"down": 0, "up": 1, "right": 2, "left": 3}[facing])

# ---------- sprites ----------
obj_tiles = bytearray()
obj_pals = []


def obj_palette(pal_dict):
    colors = []
    for v in pal_dict.values():
        c = rgb555(art.hex_rgb(v))
        if c not in colors:
            colors.append(c)
    pal = [0] + colors
    obj_pals.append(pal)
    lookup = {k: pal.index(rgb555(art.hex_rgb(v))) for k, v in pal_dict.items()}
    return len(obj_pals) - 1, lookup


def add_obj(rows, lookup, w, h, flip=False, overlay=None, scale=1, top_pad=None):
    """rows are 16 wide; placed at the bottom of a w x h cell. Returns first tile index."""
    grid = [[0] * w for _ in range(h)]
    rh = len(rows) * scale
    oy = h - rh if top_pad is None else top_pad
    ox = (w - 16 * scale) // 2
    for j, row in enumerate(rows):
        for i, ch in enumerate(row):
            if ch == ".":
                continue
            x = 15 - i if flip else i
            for sy in range(scale):
                for sx in range(scale):
                    grid[oy + j * scale + sy][ox + x * scale + sx] = lookup[ch]
    if overlay:
        for x, y, ch in overlay:
            for sy in range(scale):
                for sx in range(scale):
                    grid[oy + y * scale + sy][ox + x * scale + sx] = lookup[ch]
    first = len(obj_tiles) // 32
    for ty in range(h // 8):
        for tx in range(w // 8):
            idx = [grid[ty * 8 + y][tx * 8 + x] for y in range(8) for x in range(8)]
            obj_tiles.extend(encode_tile(idx))
    return first


FRAMES = ["stand", "walkA", "walkB"]
DIRS = ["down", "up", "right", "left"]  # matches the C facing enum order

for who, set_ in (("irving", S.IRVING), ("moeno", S.MOENO)):
    pal, lookup = obj_palette(S.PALETTES[who])
    define(f"OBJPAL_{who.upper()}", pal)
    first = None
    for d in DIRS:
        for f in FRAMES:
            rows = art.person_rows(set_, "right" if d == "left" else d, f)
            overlay = S.MOENO["leftOverlay"] if (who == "moeno" and d == "left") else None
            t = add_obj(rows, lookup, 16, 32, flip=(d == "left"), overlay=overlay)
            first = t if first is None else first
    define(f"OBJ_{who.upper()}", first)  # 12 frames x 8 tiles
    t = add_obj(art.person_rows(set_, "down", "stand"), lookup, 32, 64, scale=2, top_pad=8)
    define(f"OBJ_{who.upper()}_BIG", t)

pal, lookup = obj_palette(S.PALETTES["mui"])
define("OBJPAL_MUI", pal)
first = None
for d in DIRS:
    facing = "side" if d in ("left", "right") else d
    rows = S.MUI[facing]
    for hop in (0, 1):
        r = rows if hop == 0 else [S.BLANK] + list(rows[:-1])
        t = add_obj(r, lookup, 16, 16, flip=(d == "left"))
        first = t if first is None else first
define("OBJ_MUI", first)  # 8 frames x 4 tiles

for look in ["salaryman", "obaachan", "traveler", "vendor"]:
    pal, lookup = obj_palette(S.PALETTES[look])
    t = add_obj(art.person_rows(S.TOWNSFOLK, "down"), lookup, 16, 32)
    define(f"OBJ_{look.upper()}", t)
    define(f"OBJPAL_{look.upper()}", pal)

emit_u8("obj_tiles", bytes(obj_tiles))
emit_u16("obj_pal", palettes_u16(obj_pals))
print(f"sprites: {len(obj_tiles) // 32} tiles, {len(obj_pals)} palettes", file=sys.stderr)

# ---------- text layer tiles ----------
font = art.load_font()
ui = [[0] * 64 for _ in range(128)]


def put(t, x, y, v):
    ui[t][y * 8 + x] = v


for i in range(64):
    ui[1][i] = 1


def border(t, top, bottom, left, right):
    for y in range(8):
        for x in range(8):
            v = 1
            dist = []
            if top:
                dist.append(y)
            if bottom:
                dist.append(7 - y)
            if left:
                dist.append(x)
            if right:
                dist.append(7 - x)
            d = min(dist) if dist else 9
            if d <= 1:
                v = 3
            elif d == 2:
                v = 4
            if top and left and x + y <= 1 or top and right and (7 - x) + y <= 1 or bottom and left and x + (7 - y) <= 1 or bottom and right and (7 - x) + (7 - y) <= 1:
                v = 0
            put(t, x, y, v)


border(2, True, False, True, False)
border(3, True, False, False, False)
border(4, True, False, False, True)
border(5, False, False, True, False)
border(6, False, False, False, True)
border(7, False, True, True, False)
border(8, False, True, False, False)
border(9, False, True, False, True)
# 10: cursor arrow
arrow = ["........", ".XX.....", ".XXXX...", ".XXXXXX.", ".XXXX...", ".XX.....", "........", "........"]
more = ["........", "........", ".XXXXXX.", "..XXXX..", "...XX...", "........", "........", "........"]
for y in range(8):
    for x in range(8):
        put(10, x, y, 2 if arrow[y][x] == "X" else 1)
        put(11, x, y, 7 if more[y][x] == "X" else 1)
box_empty = ["........", ".FFFFFF.", ".FWWWWF.", ".FWWWWF.", ".FWWWWF.", ".FWWWWF.", ".FFFFFF.", "........"]
box_full = ["........", ".FFFFFF.", ".FOOOOWF", ".FWOOWOF", ".FOWWOOF", ".FOOWOOF", ".FFFFFF.", "........"]
m = {".": 1, "F": 3, "W": 8, "O": 5}
for y in range(8):
    for x in range(8):
        put(12, x, y, m[box_empty[y][x]])
        put(13, x, y, m[box_full[y][x]])
for n in range(9):
    t = 14 + n
    for y in range(8):
        for x in range(8):
            v = 1
            if 2 <= y <= 5:
                v = (6 if y < 4 else 5) if x < n else 9
            put(t, x, y, v)
for i in range(64):
    ui[23][i] = 3
for ch in range(32, 127):
    for y, bits in enumerate(font[ch]):
        for x in range(8):
            put(ch, x, y, 2 if bits >> x & 1 else 1)
emit_u8("ui_tiles", b"".join(encode_tile(t) for t in ui))

UI_COLORS = {
    15: ["#fbf8f0", "#2a2740"],  # paper / ink
    14: ["#1f2a4d", "#fff7e6"],  # navy / cream (HUD, name tags)
    13: ["#fbf8f0", "#8a86a0"],  # paper / grey (done tasks)
    12: ["#ffe2b8", "#2a2740"],  # highlight / ink (selected row)
}
shared = ["#3a3f6b", "#9cc0e0", "#f28b1e", "#ffd166", "#d8383d", "#ffffff", "#1a1830"]
ui_pal = []
for bank in (12, 13, 14, 15):
    cols = UI_COLORS[bank] + shared
    ui_pal.extend([0] + [rgb555(art.hex_rgb(c)) for c in cols] + [0] * (15 - len(cols)))
emit_u16("ui_pal", ui_pal)  # banks 12-15

# ---------- world data ----------
emit_u8("world_grid", bytes(v for row in W.GRID for v in row))
define("MAP_W", W.MAP_W)
define("MAP_H", W.MAP_H)
define("START_X", W.START[0])
define("START_Y", W.START[1])
for i, b in enumerate(W.BUILDINGS):
    define(f"B_{b['id'].upper()}", i)
define("BUILDING_COUNT", len(W.BUILDINGS))
rects = []
for b in W.BUILDINGS:
    dx, dy = b["door"] if b["door"] else (255, 255)
    rects += [b["x"], b["y"], b["w"], b["h"], dx, dy]
emit_u8("building_rects", rects)
npcs = []
looks = {"salaryman": "OBJ_SALARYMAN", "obaachan": "OBJ_OBAACHAN", "traveler": "OBJ_TRAVELER", "vendor": "OBJ_VENDOR"}
for i, (nid, x, y, look) in enumerate(W.NPCS):
    define(f"NPC_{nid.upper()}", i)
    npcs += [x, y]
emit_u8("npc_pos", npcs)
define("NPC_COUNT", len(W.NPCS))
c_src.append("const uint16_t npc_tile[] = {" + ", ".join(f"OBJ_{n[3].upper()}" for n in W.NPCS) + "};\n")
c_src.append("const uint8_t npc_pal[] = {" + ", ".join(f"OBJPAL_{n[3].upper()}" for n in W.NPCS) + "};\n")
h_src.append("extern const uint16_t npc_tile[];\nextern const uint8_t npc_pal[];\n")
for name, val in (("T_GRASS", W.GRASS), ("T_ROAD", W.ROAD), ("T_PAVE", W.PAVE), ("T_WATER", W.WATER), ("T_TREE", W.TREE),
                  ("T_FLOWER", W.FLOWER), ("T_BRIDGE", W.BRIDGE), ("T_SOLID", W.SOLID), ("T_DOOR", W.DOOR), ("T_CROSS", W.CROSS)):
    define(name, val)

h_src.append("#endif\n")
open(os.path.join(OUT, "assets.c"), "w").write("\n".join(c_src))
open(os.path.join(OUT, "assets.h"), "w").write("".join(h_src))
print("assets written", file=sys.stderr)
