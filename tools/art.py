"""Draws the overworld, interiors and title screen as RGB images (ported from the original canvas art)."""
from PIL import Image, ImageDraw

import sprite_data as S
from world import GRID, BUILDINGS, MAP_W, MAP_H, GRASS, ROAD, PAVE, WATER, TREE, FLOWER, BRIDGE, SOLID, DOOR, CROSS

T16 = 16
C = dict(
    grass="#86c96b", grassDark="#63aa55", grassLight="#b2df8e",
    road="#5d6170", roadLine="#ece6d6", roadDark="#4f5361",
    pave="#dcd4c4", paveLine="#bdb2a0",
    water="#3d8ed4", waterDeep="#2f74b6", waterLight="#8cc9f2",
    leaf="#2f8a4c", leafDark="#1f6438", leafLight="#52b067", trunk="#7a4f2e",
    red="#e8586a", yellow="#f4cf49", white="#f7f3ea",
    wood="#a86f45", woodDark="#6e4428", outline="#23202e",
)
# Canal sparkle colours; the ROM swaps them every few frames so the water shimmers.
SPARK_A = "#90d0f8"
SPARK_B = "#3890d8"


class Pen:
    def __init__(self, img):
        self.d = ImageDraw.Draw(img)

    def __call__(self, color, x, y, w=1, h=1):
        x, y, w, h = int(x), int(y), int(w), int(h)
        if w <= 0 or h <= 0:
            return
        self.d.rectangle([x, y, x + w - 1, y + h - 1], fill=color)


def g(x, y):
    if 0 <= x < MAP_W and 0 <= y < MAP_H:
        return GRID[y][x]
    return None


def grass(px, X, Y, tx, ty):
    px(C["grass"], X, Y, 16, 16)
    v = (tx * 7 + ty * 13) % 5
    if v < 2:
        ox, oy = (3, 5) if v == 0 else (9, 9)
        px(C["grassDark"], X + ox, Y + oy, 1, 2)
        px(C["grassDark"], X + ox + 2, Y + oy, 1, 2)
        px(C["grassLight"], X + ox + 1, Y + oy - 1)


def tile(px, t, X, Y, tx, ty):
    if t in (GRASS, SOLID):
        grass(px, X, Y, tx, ty)
    elif t == ROAD:
        px(C["road"], X, Y, 16, 16)
        horiz = g(tx - 1, ty) == ROAD or g(tx + 1, ty) == ROAD
        if horiz and g(tx, ty + 1) == ROAD and g(tx, ty - 1) != ROAD and tx % 2 == 0:
            px(C["roadLine"], X + 2, Y + 15, 10, 1)
        if g(tx, ty - 1) == ROAD and g(tx, ty + 1) == ROAD and g(tx + 1, ty) == ROAD and g(tx - 1, ty) != ROAD and ty % 2 == 0:
            px(C["roadLine"], X + 15, Y + 2, 1, 10)
    elif t == CROSS:
        px(C["road"], X, Y, 16, 16)
        vertical = g(tx, ty - 1) == CROSS or g(tx, ty + 1) == CROSS
        for i in range(1, 16, 4):
            if vertical:
                px(C["roadLine"], X + 2, Y + i, 12, 2)
            else:
                px(C["roadLine"], X + i, Y + 2, 2, 12)
    elif t == PAVE:
        px(C["pave"], X, Y, 16, 16)
        px(C["paveLine"], X, Y + 15, 16, 1)
        px(C["paveLine"], X + (ty % 2) * 8, Y, 1, 15)
    elif t == WATER:
        px(C["water"], X, Y, 16, 16)
        if g(tx, ty - 1) not in (WATER, BRIDGE):
            px(C["waterDeep"], X, Y, 16, 2)
        v = (tx * 5 + ty * 3) % 4
        px(SPARK_A, X + 2 + v * 2, Y + 5 + (v % 2) * 3, 3, 1)
        px(SPARK_B, X + 9 - v, Y + 11, 3, 1)
    elif t == BRIDGE:
        px(C["wood"], X, Y, 16, 16)
        for i in range(0, 16, 4):
            px(C["woodDark"], X + i, Y, 1, 16)
        if g(tx, ty - 1) != BRIDGE:
            px(C["woodDark"], X, Y, 16, 2)
        if g(tx, ty + 1) != BRIDGE:
            px(C["woodDark"], X, Y + 14, 16, 2)
    elif t == TREE:
        grass(px, X, Y, tx, ty)
        px(C["trunk"], X + 6, Y + 11, 4, 4)
        px(C["leafDark"], X + 2, Y + 3, 12, 9)
        px(C["leafDark"], X + 4, Y + 1, 8, 12)
        px(C["leaf"], X + 3, Y + 3, 10, 7)
        px(C["leaf"], X + 5, Y + 2, 6, 9)
        px(C["leafLight"], X + 5, Y + 3, 3, 2)
        px(C["leafLight"], X + 9, Y + 6, 2, 1)
    elif t == FLOWER:
        grass(px, X, Y, tx, ty)
        for fx, fy, c in ((3, 4, C["red"]), (10, 3, C["yellow"]), (6, 10, C["white"]), (12, 11, C["red"])):
            px(c, X + fx, Y + fy, 2, 2)
            px(C["grassDark"], X + fx, Y + fy + 2, 1, 1)
    else:
        grass(px, X, Y, tx, ty)


def window_row(px, x, y, w, color, frame, gap=6, size=4):
    i = x + 3
    while i + size <= x + w - 3:
        px(frame, i - 1, y - 1, size + 2, size + 2)
        px(color, i, y, size, size)
        px("#ffffff", i, y, 1, 1)
        i += gap


def door(px, b, glass):
    X, Y = b["door"][0] * 16, b["door"][1] * 16
    px(C["outline"], X + 2, Y + 1, 12, 15)
    px("#8fc4e0" if glass else "#6b4a33", X + 3, Y + 2, 10, 14)
    if glass:
        px("#cfe8f5", X + 4, Y + 3, 2, 8)
        px(C["outline"], X + 7, Y + 2, 1, 14)
    else:
        px(C["yellow"], X + 10, Y + 9, 1, 1)


def building(px, b):
    X, Y, W, H = b["x"] * 16, b["y"] * 16, b["w"] * 16, b["h"] * 16
    s = b["style"]
    if s == "consulate":
        px("#b9b4a8", X, Y + 14, W, H - 14)
        px("#ece8df", X + 2, Y + 16, W - 4, H - 18)
        px("#56627f", X - 1, Y + 4, W + 2, 12)
        px("#6f7c9c", X - 1, Y + 4, W + 2, 3)
        px(C["outline"], X - 1, Y + 15, W + 2, 1)
        for i in range(X + 8, X + W - 8, 14):
            px("#d6d1c6", i, Y + 18, 5, H - 22)
            px("#ffffff", i, Y + 18, 1, H - 22)
        window_row(px, X + 4, Y + 24, W - 8, "#7aa6c9", "#56627f", 14, 5)
        window_row(px, X + 4, Y + 40, W - 8, "#7aa6c9", "#56627f", 14, 5)
        px("#9aa0aa", X + W - 16, Y - 14, 1, 20)
        fx, fy = X + W - 15, Y - 14
        for r in range(7):
            px("#ffffff" if r % 2 else "#c8323a", fx, fy + r, 11, 1)
        px("#2b3a78", fx, fy, 5, 4)
        px("#ffffff", fx + 1, fy + 1)
        px("#ffffff", fx + 3, fy + 2)
        door(px, b, True)
    elif s == "station":
        px("#8e9aa8", X, Y + 10, W, H - 10)
        px("#bcd3e3", X + 2, Y + 12, W - 4, H - 14)
        for i in range(X + 2, X + W - 2, 10):
            px("#8e9aa8", i, Y + 12, 1, H - 14)
        for j in range(Y + 22, Y + H, 12):
            px("#8e9aa8", X + 2, j, W - 4, 1)
        px("#36486a", X - 2, Y + 2, W + 4, 10)
        px("#4c628a", X - 2, Y + 2, W + 4, 2)
        cx = X + W // 2 - 8
        px(C["outline"], cx, Y - 6, 16, 16)
        px("#f7f3ea", cx + 1, Y - 5, 14, 14)
        px(C["outline"], cx + 7, Y - 3, 1, 6)
        px(C["outline"], cx + 7, Y + 2, 5, 1)
        door(px, b, True)
    elif s == "home":
        px("#e9cfa6", X + 2, Y + 14, W - 4, H - 14)
        px("#a4462f", X - 2, Y + 2, W + 4, 14)
        for i in range(X - 2, X + W + 2, 4):
            px("#86361f", i, Y + 2, 1, 14)
        px("#c8674b", X - 2, Y + 2, W + 4, 2)
        window_row(px, X + 2, Y + 22, W - 4, "#8fc4e0", "#7d5a3c", 12, 6)
        px("#7d5a3c", X + 4, Y + 34, W - 8, 1)
        px("#f7f3ea", X + 8, Y + 35, 4, 5)
        px("#56b3ef", X + 14, Y + 35, 5, 4)
        door(px, b, False)
    elif s == "cafe":
        px("#6a4535", X + 1, Y + 8, W - 2, H - 8)
        px("#543428", X + 1, Y + 8, W - 2, 2)
        for n, i in enumerate(range(0, W + 4, 6)):
            px("#2f7d6d" if n % 2 else "#f3ead8", X - 2 + i, Y + 20, 6, 8)
        px(C["outline"], X - 2, Y + 28, W + 4, 1)
        px("#f3ead8", X + 6, Y + 11, W - 12, 7)
        px("#6a4535", X + W // 2 - 3, Y + 12, 5, 4)
        px("#6a4535", X + W // 2 + 2, Y + 13, 1, 2)
        px("#fbe9a8", X + 5, Y + 34, 16, 14)
        px("#fbe9a8", X + W - 21, Y + 34, 16, 14)
        px("#e9cf7c", X + 5, Y + 44, 16, 4)
        door(px, b, True)
    elif s == "prints":
        px("#d7dce5", X + 2, Y + 12, W - 4, H - 12)
        px("#2e7d8c", X - 1, Y + 2, W + 2, 12)
        px("#4aa0ad", X - 1, Y + 2, W + 2, 2)
        window_row(px, X + 2, Y + 20, W - 4, "#9fc6d6", "#6f7f93", 10, 5)
        window_row(px, X + 2, Y + 34, W - 4, "#9fc6d6", "#6f7f93", 10, 5)
        sx, sy = X + W - 26, Y + 50
        px("#f7f3ea", sx, sy, 14, 14)
        px(C["outline"], sx, sy, 14, 1)
        for r in range(4):
            px("#2e7d8c", sx + 3 + r, sy + 3 + r, 8 - r * 2, 1)
            px("#2e7d8c", sx + 3 + r, sy + 3 + r, 1, 8 - r * 2)
            px("#2e7d8c", sx + 10 - r, sy + 3 + r, 1, 8 - r * 2)
        door(px, b, True)
    elif s == "castle":
        px("#8b8d86", X, Y + H - 14, W, 14)
        for i in range(X, X + W, 6):
            px("#6e706a", i, Y + H - 14, 1, 14)
        px("#f4f1ea", X + 10, Y + 22, W - 20, H - 36)
        px("#3f8f7a", X + 4, Y + 16, W - 8, 7)
        px("#3f8f7a", X + 14, Y + 4, W - 28, 7)
        px("#5fb39b", X + 4, Y + 16, W - 8, 2)
        px("#f4f1ea", X + 18, Y + 11, W - 36, 5)
        px("#d9a93a", X + 15, Y + 1, 3, 3)
        px("#d9a93a", X + W - 18, Y + 1, 3, 3)
        window_row(px, X + 10, Y + 30, W - 20, "#2f3d45", "#f4f1ea", 8, 3)
    elif s == "tower":
        px("#8995a1", X + 8, Y + 6, W - 16, H - 6)
        for j in range(Y + 10, Y + H, 8):
            px("#b8c2cb", X + 8, j, W - 16, 1)
        px("#8995a1", X + 2, Y + H - 12, W - 4, 12)
        px("#6d7883", X + 14, Y + H - 12, W - 28, 12)
        px("#e8586a", X + 10, Y + 16, W - 20, 6)
        px("#9aa4ad", X + W // 2, Y - 10, 1, 16)
    elif s == "stand":
        px("#7a4f2e", X + 2, Y + 10, W - 4, H - 10)
        px("#f3ead8", X + 2, Y + 16, W - 4, 8)
        px("#c8323a", X, Y + 2, W, 8)
        for i in range(X + 4, X + W, 12):
            px("#f39a3a", i, Y + 11, 5, 5)
            px(C["outline"], i + 2, Y + 16, 1, 1)
        for i in range(X + 6, X + W - 4, 7):
            px("#8a5a2e", i, Y + 19, 4, 3)


def overworld():
    img = Image.new("RGB", (MAP_W * 16, MAP_H * 16))
    px = Pen(img)
    for y in range(MAP_H):
        for x in range(MAP_W):
            tile(px, GRID[y][x], x * 16, y * 16, x, y)
    for b in BUILDINGS:
        building(px, b)
    return img


def paste_sprite(img, rows, palette, x, y, flip=False):
    p = img.load()
    for j, row in enumerate(rows):
        for i, ch in enumerate(row):
            if ch != ".":
                p[x + (15 - i if flip else i), y + j] = hex_rgb(palette[ch])


def hex_rgb(h):
    h = h.lstrip("#")
    return tuple(int(h[i:i + 2], 16) for i in (0, 2, 4))


def person_rows(set_, facing, frame="stand"):
    side = facing in ("left", "right")
    top = set_["side" if side else facing]
    legs = (S.SIDE_LEGS if side else S.LEGS)[frame]
    return list(top) + list(legs) + [S.BLANK]


INTERIORS = {
    "consulate": dict(wall="#e8e4da", trim="#56627f", floor=("#c9c2b2", "#bdb5a4"), counter="#56627f", npc="guard", deco="flag"),
    "cafe": dict(wall="#6a4535", trim="#2f7d6d", floor=("#b98b5e", "#a97b50"), counter="#8a5a3c", npc="barista", deco="laptop"),
    "home": dict(wall="#efdcbc", trim="#a4462f", floor=("#d9b98c", "#c9a97c"), counter=None, npc=None, deco="kotatsu"),
    "prints": dict(wall="#dfe4ec", trim="#2e7d8c", floor=("#c5ccd6", "#b7bfca"), counter="#2e7d8c", npc="clerk", deco="scanner"),
    "station": dict(wall="#c7d3de", trim="#36486a", floor=("#aeb6c0", "#a0a8b3"), counter="#36486a", npc="traveler", deco="board"),
}
# Where the player sprite stands in each interior (pixels) and which way they face.
INTERIOR_PLAYER = {k: (112, 74, "up") for k in INTERIORS}
INTERIOR_PLAYER["home"] = (76, 58, "right")


def interior(name):
    s = INTERIORS[name]
    img = Image.new("RGB", (256, 256), hex_rgb(s["floor"][0]))
    px = Pen(img)
    for y in range(0, 256, 16):
        for x in range(0, 256, 16):
            px(s["floor"][((x + y) // 16) % 2], x, y, 16, 16)
    px(s["wall"], 0, 0, 240, 36)
    px(s["trim"], 0, 32, 240, 4)
    window_row(px, 0, 8, 80, "#9fd0ee", s["trim"], 26, 14)
    window_row(px, 160, 8, 80, "#9fd0ee", s["trim"], 26, 14)
    if s["deco"] == "flag":
        px("#9aa0aa", 118, 2, 1, 28)
        for r in range(9):
            px("#ffffff" if r % 2 else "#c8323a", 119, 4 + r, 16, 1)
        px("#2b3a78", 119, 4, 7, 5)
    if s["deco"] == "board":
        px(C["outline"], 96, 4, 48, 24)
        for r in range(3):
            px("#f4cf49", 100, 8 + r * 6, 18 + (r * 11) % 20, 2)
    if s["deco"] == "scanner":
        px("#6f7f93", 190, 46, 20, 14)
        px("#4ee0a0", 194, 49, 12, 3)
    if s["counter"]:
        paste_sprite(img, person_rows(S.TOWNSFOLK, "down"), S.PALETTES[s["npc"]], 112, 32)
        px(s["counter"], 64, 50, 112, 12)
        px("#ffffff", 64, 50, 112, 2)
    if s["deco"] in ("laptop", "kotatsu"):
        home = s["deco"] == "kotatsu"
        tx, ty = (104, 60) if home else (150, 72)
        px("#c44a3a" if home else "#6a4535", tx - 4, ty + 6, 72, 18)
        px("#e8d2a8", tx, ty, 64, 10)
        px("#3a3f4b", tx + 20, ty - 14, 24, 15)
        px("#8fd0a8", tx + 22, ty - 12, 10, 11)
        px("#f4b98a", tx + 33, ty - 12, 9, 11)
        px("#2e7d8c", tx + 26, ty - 8, 2, 2)
        px("#5a3a26", tx + 36, ty - 8, 2, 2)
        px("#9aa0aa", tx + 18, ty + 1, 28, 2)
    return img


def load_font():
    import re
    text = open(__file__.replace("art.py", "font8x8_basic.h")).read()
    rows = re.findall(r"\{\s*((?:0x[0-9A-Fa-f]{2},?\s*){8})\}", text)
    return [[int(v, 16) for v in re.findall(r"0x[0-9A-Fa-f]{2}", r)] for r in rows]


def big_text(img, text, x, y, scale, color, shadow=None):
    font = load_font()
    px = Pen(img)
    for n, ch in enumerate(text):
        glyph = font[ord(ch)]
        for gy, bits in enumerate(glyph):
            for gx in range(8):
                if bits >> gx & 1:
                    X, Y = x + (n * 8 + gx) * scale, y + gy * scale
                    if shadow:
                        px(shadow, X + scale, Y + scale, scale, scale)
                    px(color, X, Y, scale, scale)


def title():
    img = Image.new("RGB", (256, 256), (0, 0, 0))
    px = Pen(img)
    for i, c in enumerate(["#2b2d5a", "#4a3f7a", "#8a4f86", "#d9677a", "#f39a6b", "#f7c46b"]):
        px(c, 0, i * 18, 240, 18)
    px("#fbe7a1", 170, 76, 24, 24)
    heights = [30, 46, 38, 60, 34, 52, 70, 40, 58, 36, 48, 64, 42, 30, 54]
    for i, h in enumerate(heights):
        px("#23202e", i * 16, 124 - h, 15, h + 10)
    px("#23202e", 60, 68, 30, 8)
    px("#23202e", 66, 60, 18, 8)
    px("#23202e", 70, 54, 10, 6)
    for i in range(40):
        x, y = (i * 37) % 236, 76 + (i * 53) % 40
        if i % 3:
            px("#f4cf49", x, y, 2, 2)
    px("#3d8ed4", 0, 132, 240, 28)
    for i in range(10):
        px(SPARK_A if i % 2 else SPARK_B, (i * 29) % 230, 138 + (i % 3) * 6, 8, 1)
    big_text(img, "Road to", 36, 12, 3, "#ffd166", shadow="#b8323a")
    big_text(img, "Valencia", 24, 40, 3, "#ffd166", shadow="#b8323a")
    return img
