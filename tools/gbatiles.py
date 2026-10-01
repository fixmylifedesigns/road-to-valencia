"""Turns RGB images into GBA 4bpp tiles, 16-colour palettes and screen entries."""


def rgb555(c):
    r, g, b = c[:3]
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def image_tiles(img):
    """Split an RGB image into 8x8 tiles of RGB555 values, row-major."""
    w, h = img.size
    px = img.load()
    tiles = []
    for ty in range(h // 8):
        for tx in range(w // 8):
            tiles.append([rgb555(px[tx * 8 + x, ty * 8 + y]) for y in range(8) for x in range(8)])
    return tiles, w // 8, h // 8


def pack_palettes(tiles, max_pals):
    """Greedily group each tile's colours into palettes of up to 15 colours (index 0 stays unused)."""
    sets = [frozenset(t) for t in tiles]
    unique = sorted(set(sets), key=lambda s: -len(s))
    pals = []
    assign = {}
    for s in unique:
        if len(s) > 15:
            raise ValueError(f"tile uses {len(s)} colours")
        best, best_score = None, None
        for i, p in enumerate(pals):
            union = p | s
            if len(union) <= 15:
                score = len(union) - len(p)
                if best is None or score < best_score:
                    best, best_score = i, score
        if best is None:
            pals.append(set(s))
            best = len(pals) - 1
        else:
            pals[best] |= s
        assign[s] = best
    if len(pals) > max_pals:
        raise ValueError(f"needs {len(pals)} palettes, limit {max_pals}")
    return [[0] + sorted(p) for p in pals], [assign[s] for s in sets]


def encode_tile(indices):
    out = bytearray()
    for i in range(0, 64, 2):
        out.append((indices[i] & 15) | ((indices[i + 1] & 15) << 4))
    return bytes(out)


def flips(indices):
    rows = [indices[y * 8:(y + 1) * 8] for y in range(8)]
    h = [v for r in rows for v in reversed(r)]
    v = [v for r in reversed(rows) for v in r]
    hv = [v for r in reversed(rows) for v in reversed(r)]
    return {0: indices, 1: h, 2: v, 3: hv}


def build_bg(img, max_pals, pal_base=0, max_tiles=1024):
    """Returns (tile_bytes, palettes, row-major screen entries, width, height) in tiles."""
    tiles, tw, th = image_tiles(img)
    pals, assign = pack_palettes(tiles, max_pals)
    lookup = [{c: i for i, c in enumerate(p)} for p in pals]
    uniq = {}
    tile_data = []
    entries = []
    for t, pi in zip(tiles, assign):
        idx = [lookup[pi][c] for c in t]
        key, flip = None, 0
        for f, variant in flips(idx).items():
            k = bytes(variant)
            if k in uniq:
                key, flip = uniq[k], f
                break
        if key is None:
            key = len(tile_data)
            uniq[bytes(idx)] = key
            tile_data.append(encode_tile(idx))
        entries.append(key | (flip << 10) | ((pal_base + pi) << 12))
    if len(tile_data) > max_tiles:
        raise ValueError(f"{len(tile_data)} tiles > {max_tiles}")
    return b"".join(tile_data), pals, entries, tw, th


def to_sbb_order(entries, tw, th):
    """Row-major entries of a 64x64 map -> GBA screenblock order."""
    if tw <= 32:
        return entries
    out = []
    for sby in range(th // 32):
        for sbx in range(tw // 32):
            for y in range(32):
                for x in range(32):
                    out.append(entries[(sby * 32 + y) * tw + sbx * 32 + x])
    return out


def c_array(name, data, ctype="unsigned char", per_line=16, fmt="0x{:02X}"):
    items = [fmt.format(v) for v in data]
    lines = [", ".join(items[i:i + per_line]) for i in range(0, len(items), per_line)]
    return f"const {ctype} {name}[{len(data)}] __attribute__((aligned(4))) = {{\n  " + ",\n  ".join(lines) + "\n};\n"
