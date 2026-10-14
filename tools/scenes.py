"""Hunters / NPC sprites, title screen and battle backdrops for MonsterMons."""
import math
import random

from PIL import Image, ImageDraw

from font5x7 import FONT

ROLE = {'.': 0, 'o': 1, 's': 2, 'S': 3, 'h': 4, 'H': 5, 'c': 6, 'C': 7, 'p': 8, 'b': 9, 'a': 10, 'w': 11, 'e': 1, 'g': 12}

HEAD_DOWN = [
    "................",
    ".....oooooo.....",
    "....ohhhhhho....",
    "...ohhhhhhhho...",
    "...ohhhhhhhho...",
    "...ohssssssho...",
    "...oseSsSSeso...",
    "....ossSSsso....",
    ".....oSSSSo.....",
]
HEAD_UP = [
    "................",
    ".....oooooo.....",
    "....ohhhhhho....",
    "...ohhhhhhhho...",
    "...ohhhhhhhho...",
    "...ohhhhhhhho...",
    "...ohhhhHhhho...",
    "....oHHHHHHo....",
    ".....oSSSSo.....",
]
HEAD_SIDE = [
    "................",
    ".....oooooo.....",
    "....ohhhhhho....",
    "...ohhhhhhhho...",
    "...ohhhhhhhho...",
    "...osshhhhhho...",
    "...oeshhhhhho...",
    "....ossShhho....",
    ".....oSSSSo.....",
]
BODY_DOWN = [
    "...occcaaccco...",
    "..ococcccccoco..",
    "..osoCccccCoso..",
    "...ooaaaaaaoo...",
    "....oppppppo....",
]
BODY_UP = [
    "...occcccccco...",
    "..ococcccccoco..",
    "..osoCccccCoso..",
    "...ooaaaaaaoo...",
    "....oppppppo....",
]
BODY_SIDE = [
    "....occcccco....",
    "....occccCco....",
    "....ocssCCco....",
    "....oaaaaaao....",
    "....oppppppo....",
]
LEGS_FB = [
    ["....oppooppo....", "....obboobbo...."],
    ["....oppoobbo....", "....obbo.oo....."],
    ["....obbooppo....", ".....oo.obbo...."],
]
LEGS_SIDE = [
    [".....oppppo.....", ".....obbbbo....."],
    ["....opp.oppo....", "...obbo..obbo..."],
    [".....oppppo.....", "....obboobbo...."],
]

PLAYER_COATS = [("#8a1a2a", "#5a0e1a"), ("#1a7a7a", "#0e4a50"), ("#5a2a8a", "#38185a"), ("#5a5a66", "#3a3a44")]
PLAYER_HAIRS = [("#5a3a24", "#3a2414"), ("#e8e4f0", "#a8a4b8"), ("#1a1a24", "#0a0a10")]


def ppal(skin, hair, coat, pants, boots, accent, glow="#ffd040"):
    skin_shade = {"#f0c8a0": "#c89870", "#c08060": "#905840", "#8a5a3a": "#603a24", "#e8e0e8": "#b0a8b8"}.get(skin, "#b08a70")
    return ["#000000", "#100c14", skin, skin_shade, hair[0], hair[1], coat[0], coat[1], pants, boots, accent, "#f8f4ff",
            glow, "#000000", "#000000", "#000000"]


PEOPLE = [
    {"id": "player", "pal": ppal("#f0c8a0", PLAYER_HAIRS[0], PLAYER_COATS[0], "#2a2430", "#1a1418", "#d0a040")},
    {"id": "vex", "pal": ppal("#e8e0e8", ("#e8f0f8", "#a8b0c8"), ("#1a2a3a", "#0e1824"), "#141820", "#0a0c10", "#30d0e0"),
     "spiky": True},
    {"id": "ash", "pal": ppal("#c08060", ("#a8a8b0", "#78787e"), ("#5a5048", "#3a3430"), "#2a2420", "#1a1410", "#6a4a2a"),
     "hat": True, "longcoat": True},
    {"id": "man", "pal": ppal("#f0c8a0", ("#2a2018", "#1a1410"), ("#4a5a3a", "#323e28"), "#3a3028", "#1a1410", "#8a6a3a")},
    {"id": "woman", "pal": ppal("#c08060", ("#8a3a2a", "#5a2418"), ("#6a3a6a", "#482448"), "#3a2a3a", "#1a1418", "#d0a0c0"),
     "long": True},
    {"id": "grunt", "pal": ppal("#f0c8a0", ("#e8e4d8", "#b8b4a8"), ("#e8e4d8", "#a8a498"), "#d8d4c8", "#6a6050", "#d0a020",
                                glow="#ffe040"), "hat": True, "visor": True},
    {"id": "mira", "pal": ppal("#8a5a3a", ("#2a1a3a", "#1a1028"), ("#4a2a6a", "#2e1a46"), "#2a1a3a", "#140c1c", "#b080ff"),
     "hood": True, "long": True},
    {"id": "wren", "pal": ppal("#f0c8a0", ("#f0ecf4", "#c0bcc8"), ("#f0ecf4", "#b8b4c4"), "#e0dce8", "#4a3a4a", "#c02838"),
     "hood": True, "longcoat": True},
    {"id": "solenne", "pal": ppal("#e8e0e8", ("#f0d890", "#c0a860"), ("#f4f0e8", "#c8c0b0"), "#e8e0d0", "#8a7040", "#e0b020"),
     "long": True, "longcoat": True},
    {"id": "kid", "pal": ppal("#f0c8a0", ("#6a4a2a", "#4a3018"), ("#3a5a8a", "#28406a"), "#3a3028", "#1a1410", "#c02a2a"),
     "hat": True},
    {"id": "hunter", "pal": ppal("#c08060", ("#3a2a1a", "#241a10"), ("#6a4a2a", "#4a321c"), "#2a2420", "#1a1410", "#8a8a8a"),
     "hat": True},
    {"id": "keeper", "pal": ppal("#f0c8a0", ("#8a8a90", "#5a5a60"), ("#2a3a5a", "#1a2640"), "#2a2a3a", "#141418", "#d0a040")},
]


def grid_from(rows):
    return [[ROLE[ch] for ch in r] for r in rows]


def apply_style(rows, p, facing):
    rows = [list(r) for r in rows]
    if p.get("hood"):
        for y in range(1, 8):
            for x in range(16):
                if rows[y][x] in "hH":
                    rows[y][x] = 'c' if rows[y][x] == 'h' else 'C'
    if p.get("hat"):
        rows[1] = list("....oooooooo....")
        rows[2] = list("...oaaaaaaaao...")
        rows[3] = list("..oaaaaaaaaaao..")
        rows[4] = list("...ohhhhhhhho...") if facing != "up" else list("...ohhhhhhhho...")
    if p.get("spiky"):
        rows[0] = list("....o.o..o.o....")
        rows[1] = list("...ohohoohoho...")
    if p.get("long"):
        for y in range(5, 11):
            if rows[y][3] in "o.":
                rows[y][3] = 'h'
                rows[y][2] = 'o'
            if rows[y][12] in "o.":
                rows[y][12] = 'h'
                rows[y][13] = 'o'
    if p.get("visor") and facing != "up":
        for x in range(16):
            if rows[6][x] in "eSs" and 4 <= x <= 11:
                rows[6][x] = 'g'
    if p.get("longcoat"):
        rows[13] = [('c' if ch == 'p' else ch) for ch in rows[13]]
    return ["".join(r) for r in rows]


def build_people():
    out = []
    for p in PEOPLE:
        frames = []
        for facing, head, body, legs in [("down", HEAD_DOWN, BODY_DOWN, LEGS_FB), ("up", HEAD_UP, BODY_UP, LEGS_FB),
                                         ("side", HEAD_SIDE, BODY_SIDE, LEGS_SIDE)]:
            for f in range(3):
                rows = head + body + legs[f]
                rows = rows[:16]
                assert len(rows) == 16 and all(len(r) == 16 for r in rows), (p["id"], facing, [len(r) for r in rows])
                rows = apply_style(rows, p, facing)
                frames.append(grid_from(rows))
        out.append({"id": p["id"], "pal": p["pal"], "frames": frames})
    return out


# ------------------------------------------------------------------ scenes

def band_gradient(d, w, y0, y1, c0, c1, band=8):
    for y in range(y0, y1, band):
        t = (y - y0) / max(1, (y1 - y0))
        col = tuple(int(c0[i] + (c1[i] - c0[i]) * t) for i in range(3))
        d.rectangle([0, y, w, min(y1, y + band) - 1], fill=col)


def moon(d, cx, cy, r, col=(232, 226, 204), shade=(190, 184, 168), halo=None):
    if halo:
        d.ellipse([cx - r - 6, cy - r - 6, cx + r + 6, cy + r + 6], fill=halo)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=col)
    for (dx, dy, rr) in [(-r * 0.3, -r * 0.2, r * 0.25), (r * 0.35, r * 0.25, r * 0.18), (-r * 0.1, r * 0.45, r * 0.12)]:
        d.ellipse([cx + dx - rr, cy + dy - rr, cx + dx + rr, cy + dy + rr], fill=shade)


def stars(d, rng, w, h, n, col=(220, 220, 255)):
    for _ in range(n):
        x, y = rng.randrange(w), rng.randrange(h)
        d.point((x, y), fill=col)


def big_text(d, text, x, y, scale, fill, outline, shadow=None):
    cw = 6 * scale
    for i, ch in enumerate(text):
        rows = FONT[ch]
        for yy in range(7):
            for xx in range(5):
                if rows[yy][xx] == '#':
                    px, py = x + i * cw + xx * scale, y + yy * scale
                    for ox, oy in [(-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (1, 1), (1, -1), (-1, 1)]:
                        d.rectangle([px + ox * 2, py + oy * 2, px + scale - 1 + ox * 2, py + scale - 1 + oy * 2], fill=outline)
    if shadow:
        for i, ch in enumerate(text):
            rows = FONT[ch]
            for yy in range(7):
                for xx in range(5):
                    if rows[yy][xx] == '#':
                        px, py = x + i * cw + xx * scale + 2, y + yy * scale + 2
                        d.rectangle([px, py, px + scale - 1, py + scale - 1], fill=shadow)
    for i, ch in enumerate(text):
        rows = FONT[ch]
        for yy in range(7):
            for xx in range(5):
                if rows[yy][xx] == '#':
                    px, py = x + i * cw + xx * scale, y + yy * scale
                    f = fill[min(len(fill) - 1, yy * len(fill) // 7)]
                    d.rectangle([px, py, px + scale - 1, py + scale - 1], fill=f)


def title_scene():
    rng = random.Random(7)
    im = Image.new("RGB", (240, 160))
    d = ImageDraw.Draw(im)
    band_gradient(d, 240, 0, 160, (6, 4, 16), (70, 20, 50), 8)
    stars(d, rng, 240, 90, 40)
    moon(d, 186, 66, 26, col=(226, 210, 196), shade=(190, 160, 150), halo=(110, 30, 50))
    # castle silhouette
    sil = (10, 6, 14)
    d.polygon([(0, 160), (0, 128), (40, 120), (90, 126), (130, 116), (170, 124), (240, 112), (240, 160)], fill=(22, 10, 26))
    d.rectangle([140, 86, 200, 124], fill=sil)
    for tx, th in [(140, 70), (160, 60), (184, 74), (200, 66)]:
        d.rectangle([tx, th, tx + 10, 124], fill=sil)
        d.polygon([(tx - 2, th), (tx + 5, th - 12), (tx + 12, th)], fill=sil)
    for wx, wy in [(150, 96), (172, 92), (190, 100), (164, 72)]:
        d.rectangle([wx, wy, wx + 2, wy + 4], fill=(255, 200, 90))
    # dead trees
    for tx in (20, 60, 100):
        d.line([(tx, 160), (tx, 118)], fill=sil, width=3)
        for k in range(4):
            y = 126 - k * 6
            d.line([(tx, y), (tx - 8 + k, y - 8)], fill=sil, width=1)
            d.line([(tx, y + 2), (tx + 8 - k, y - 6)], fill=sil, width=1)
    # bats
    for bx, by in [(120, 50), (100, 40), (210, 30), (60, 70)]:
        d.polygon([(bx - 5, by), (bx - 2, by - 2), (bx, by + 1), (bx + 2, by - 2), (bx + 5, by), (bx, by + 3)], fill=sil)
    big_text(d, "MONSTER", 15, 10, 3, [(255, 120, 130), (230, 60, 80), (180, 30, 60), (120, 16, 40)], (14, 4, 10),
             shadow=(60, 10, 30))
    big_text(d, "MONS", 152, 10, 3, [(200, 160, 255), (160, 110, 240), (120, 70, 200), (80, 40, 150)], (14, 4, 10),
             shadow=(40, 10, 60))
    return im


def battle_scene(kind):
    rng = random.Random(sum(map(ord, kind)) * 31)
    W, H = 240, 112
    im = Image.new("RGB", (W, H))
    d = ImageDraw.Draw(im)
    skies = {
        "town": ((14, 10, 30), (60, 40, 70)), "woods": ((6, 14, 16), (30, 50, 44)), "grave": ((10, 10, 18), (50, 50, 64)),
        "city": ((10, 4, 24), (70, 20, 70)), "manor": ((20, 8, 12), (60, 24, 30)), "castle": ((8, 6, 14), (44, 30, 56)),
        "rift": ((10, 2, 24), (90, 30, 140)),
    }
    s0, s1 = skies[kind]
    band_gradient(d, W, 0, 70, s0, s1, 7)
    if kind in ("town", "woods", "grave", "castle"):
        stars(d, rng, W, 40, 25)
        moon(d, 200, 22, 12)
    sil = tuple(max(0, c // 3) for c in s0)
    gy = 64
    if kind == "town":
        for x in range(0, W, 40):
            h = rng.randrange(16, 30)
            d.rectangle([x + 4, gy - h, x + 34, gy], fill=(24, 18, 34))
            d.polygon([(x + 1, gy - h), (x + 19, gy - h - 12), (x + 37, gy - h)], fill=(30, 20, 40))
            d.rectangle([x + 14, gy - h + 6, x + 18, gy - h + 11], fill=(255, 208, 112))
        ground = [(46, 70, 46), (36, 56, 38), (60, 86, 56)]
    elif kind == "woods":
        for x in range(-10, W, 18):
            h = rng.randrange(30, 52)
            d.polygon([(x, gy), (x + 10, gy - h), (x + 20, gy)], fill=(12, 28, 22))
        for x in range(-6, W, 26):
            h = rng.randrange(20, 34)
            d.polygon([(x, gy + 2), (x + 13, gy - h), (x + 26, gy + 2)], fill=(20, 42, 30))
        ground = [(40, 64, 44), (30, 50, 36), (58, 86, 58)]
    elif kind == "grave":
        for x in range(4, W, 28):
            h = rng.randrange(10, 18)
            d.rectangle([x, gy - h, x + 10, gy], fill=(60, 58, 74))
            d.ellipse([x, gy - h - 5, x + 10, gy - h + 5], fill=(60, 58, 74))
        for x in (30, 150):
            d.line([(x, gy), (x, gy - 40)], fill=sil, width=3)
            for k in range(4):
                d.line([(x, gy - 14 - k * 6), (x - 12 + k * 2, gy - 24 - k * 6)], fill=sil, width=1)
                d.line([(x, gy - 12 - k * 6), (x + 12 - k * 2, gy - 22 - k * 6)], fill=sil, width=1)
        ground = [(54, 60, 50), (40, 46, 40), (74, 84, 66)]
    elif kind == "city":
        for x in range(0, W, 30):
            h = rng.randrange(30, 60)
            d.rectangle([x, gy - h, x + 26, gy], fill=(20, 14, 34))
            for wy in range(gy - h + 4, gy - 4, 8):
                for wx in range(x + 4, x + 22, 6):
                    if rng.random() < 0.4:
                        d.rectangle([wx, wy, wx + 2, wy + 3], fill=rng.choice([(255, 63, 164), (34, 224, 255), (255, 208, 58)]))
        d.rectangle([60, 20, 110, 30], fill=(40, 10, 40))
        d.rectangle([62, 22, 108, 28], fill=(255, 63, 164))
        ground = [(34, 28, 50), (26, 20, 40), (60, 50, 90)]
    elif kind == "manor":
        d.rectangle([0, 0, W, 70], fill=(40, 12, 20))
        for x in range(0, W, 16):
            d.rectangle([x + 2, 0, x + 6, 70], fill=(60, 20, 30))
        for x in (40, 180):
            d.rectangle([x, 10, x + 24, 50], fill=(20, 10, 14))
            d.rectangle([x + 3, 13, x + 21, 47], fill=(36, 50, 80))
            d.line([(x + 12, 13), (x + 12, 47)], fill=(20, 10, 14))
        for x in (110,):
            d.rectangle([x, 30, x + 4, 60], fill=(200, 160, 80))
            d.ellipse([x - 2, 22, x + 6, 32], fill=(255, 220, 120))
        ground = [(70, 30, 30), (50, 20, 24), (110, 46, 50)]
    elif kind == "castle":
        for x in range(0, W, 40):
            d.rectangle([x + 10, 10, x + 22, gy], fill=(40, 36, 54))
            d.rectangle([x + 8, 6, x + 24, 12], fill=(50, 46, 66))
        d.rectangle([0, 50, W, gy], fill=(30, 26, 40))
        ground = [(62, 58, 78), (46, 42, 60), (90, 86, 110)]
    else:
        for _ in range(8):
            x, y, r = rng.randrange(W), rng.randrange(60), rng.randrange(4, 14)
            d.ellipse([x - r, y - r, x + r, y + r], fill=(60, 20, 110))
            d.ellipse([x - r // 2, y - r // 2, x + r // 2, y + r // 2], fill=(120, 60, 200))
        stars(d, rng, W, 64, 40, col=(200, 240, 255))
        ground = [(40, 16, 80), (26, 10, 56), (80, 40, 160)]
    band_gradient(d, W, gy, H, ground[0], ground[1], 6)
    for _ in range(60):
        x, y = rng.randrange(W), rng.randrange(gy + 2, H)
        d.point((x, y), fill=ground[2])
    # platforms (enemy right/up, player left/down)
    plat = tuple(max(0, c - 18) for c in ground[1])
    rim = ground[2]
    d.ellipse([138, 40, 236, 60], fill=plat, outline=rim)
    d.ellipse([4, 92, 120, 112], fill=plat, outline=rim)
    return im


def build_scenes():
    out = [("title", title_scene())]
    for k in ("town", "woods", "grave", "city", "manor", "castle", "rift"):
        out.append((k, battle_scene(k)))
    return out
