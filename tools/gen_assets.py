#!/usr/bin/env python3
"""MonsterMons asset generator.

Builds every graphic in the game procedurally (font, UI, overworld tiles,
hunters/NPCs, 36 Monstermon sprites, title screen, battle backdrops) plus the
area maps, and writes them as C arrays into <outdir>/assets.c / assets.h.
Pass --preview to also dump PNG previews into <outdir>/preview.
"""
import os
import random
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from font5x7 import FONT  # noqa: E402
import monsters  # noqa: E402
import maps as mapdata  # noqa: E402
import scenes  # noqa: E402


def rgb15(h):
    h = h.lstrip("#")
    r, g, b = int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16)
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def hex2rgb(h):
    h = h.lstrip("#")
    return (int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16))


def tile4(grid, x0, y0):
    """Encode an 8x8 region of an index grid as 8 u32 words (4bpp)."""
    words = []
    for y in range(8):
        w = 0
        for x in range(8):
            w |= (grid[y0 + y][x0 + x] & 15) << (4 * x)
        words.append(w)
    return tuple(words)


class Out:
    def __init__(self):
        self.c = ['#include "assets.h"', ""]
        self.h = ["#ifndef ASSETS_H", "#define ASSETS_H", "#include <stdint.h>", ""]

    def arr(self, ctype, name, data, per_line=12, fmt="0x%X", dims=""):
        self.h.append("extern const %s %s[]%s;" % (ctype, name, dims))
        lines = []
        for i in range(0, len(data), per_line):
            lines.append("    " + ",".join(fmt % v for v in data[i:i + per_line]) + ",")
        self.c.append("const %s %s[]%s __attribute__((aligned(4))) = {\n%s\n};" % (ctype, name, dims, "\n".join(lines)))

    def raw_h(self, s):
        self.h.append(s)

    def raw_c(self, s):
        self.c.append(s)

    def write(self, outdir):
        self.h.append("#endif")
        open(os.path.join(outdir, "assets.c"), "w").write("\n".join(self.c) + "\n")
        open(os.path.join(outdir, "assets.h"), "w").write("\n".join(self.h) + "\n")


# ---------------------------------------------------------------- palettes
UI_BASE = ["#000000", "#140f24", "#8c7cb8", "#f4f0ff", "#4a4068", "#e8c060", "#d83848",
           "#48d070", "#f0d040", "#4890f0", "#9058c0", "#06040c", "#2a2044", "#c0b0e0",
           "#ff7090", "#ffffff"]


def ui_palettes():
    pals = []
    variants = [("#f4f0ff", "#48d070"), ("#e8c060", "#f0d040"), ("#7a7290", "#d83848"),
                ("#ff7080", "#48d070")]
    for fg, bar in variants:
        p = list(UI_BASE)
        p[3] = fg
        p[7] = bar
        pals.append(p)
    return pals


BG_PALS = {
    4: ["#000000", "#0b0f14", "#14261c", "#1f3a28", "#2d5236", "#3f6b45", "#5a8a55", "#2a1e16",
        "#4a3424", "#6b5034", "#8a6c48", "#c8c0d8", "#8c6cb0", "#e8d890", "#3a4a5a", "#9aa8b8"],
    5: ["#000000", "#0c0a12", "#1e1a2a", "#2e2a3e", "#444058", "#5e5a74", "#7c7894", "#a09cb4",
        "#3a2a30", "#5c3a3a", "#7a4c44", "#e8c070", "#fff0b0", "#6a2030", "#2a3a2a", "#4a5a3a"],
    6: ["#000000", "#08060e", "#1a1428", "#2a2040", "#ff3fa4", "#ff9ad4", "#22e0ff", "#a8f4ff",
        "#b04cff", "#e0b0ff", "#ffd03a", "#fff2a8", "#3a3050", "#4c4068", "#18ff8c", "#ffffff"],
    7: ["#000000", "#06101c", "#0c2034", "#123050", "#1c4670", "#2c6090", "#5a90c0", "#a8d0f0",
        "#14261c", "#1f3a28", "#2d5236", "#3f6b45", "#5a8a55", "#4a3424", "#6b5034", "#c8c0d8"],
    8: ["#000000", "#120a08", "#2a1810", "#44281a", "#5e3a24", "#7a5030", "#9a6c44", "#5a1420",
        "#8a2030", "#b03a40", "#c8a050", "#e8d8b0", "#2a3040", "#3a5a3a", "#e8c070", "#fff0b0"],
    9: ["#000000", "#05020a", "#120624", "#200a3e", "#34105e", "#4c1a86", "#6c2cb4", "#9a50e0",
        "#d090ff", "#10303a", "#1a5a60", "#30a0a0", "#80f0e0", "#ff60a0", "#ffffff", "#2a2a40"],
    10: ["#000000", "#0a0c0e", "#1a1e1c", "#262c28", "#343c34", "#46503e", "#5e6a50", "#3a3440",
         "#5a5468", "#7e7890", "#a8a2b8", "#2a2018", "#3e3024", "#54402e", "#8a2030", "#c8d0c0"],
    11: ["#000000", "#0c0a10", "#2a1a2a", "#3e2440", "#583458", "#74487a", "#3a3436", "#58504e",
         "#7a706a", "#4a2e1e", "#6e4428", "#ffd070", "#fff4c0", "#8a7a50", "#1e2838", "#b0a8a0"],
    13: ["#000000", "#5a6478", "#7a8498", "#9aa4b8", "#bac4d8", "#000000", "#000000", "#000000",
         "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000"],
}
# palette entries that keep glowing at night (bitmask per bank)
EMISSIVE = {4: [13], 5: [11, 12], 6: list(range(1, 16)), 8: [14, 15], 9: list(range(1, 16)),
            11: [11, 12], 13: list(range(1, 16))}

# ---------------------------------------------------------------- font / ui tiles


def build_ui_tiles():
    tiles = []
    for c in range(32, 127):
        rows = FONT[chr(c)]
        g = [[1] * 8 for _ in range(8)]
        for y in range(7):
            for x in range(5):
                if rows[y][x] == '#':
                    g[y][x + 1] = 3
        # drop shadow
        for y in range(7):
            for x in range(5):
                if rows[y][x] == '#':
                    sx, sy = x + 2, y + 1
                    if sx < 8 and sy < 8 and g[sy][sx] == 1:
                        g[sy][sx] = 4
        tiles.append(tile4(g, 0, 0))
    ui = {}

    def add(name, g):
        ui[name] = len(tiles)
        tiles.append(tile4(g, 0, 0))

    def blank(c):
        return [[c] * 8 for _ in range(8)]

    # window frame pieces (bg 1, frame 2 light, 11 dark edge, 12 inner shadow)
    tl = blank(1)
    for i in range(8):
        tl[0][i] = 0; tl[i][0] = 0
    for i in range(1, 8):
        tl[1][i] = 11; tl[i][1] = 11
        tl[2][i] = 2; tl[i][2] = 2
    for i in range(3, 8):
        tl[3][i] = 12; tl[i][3] = 12
    tl[1][1] = 0
    tl[2][2] = 13
    add("win_tl", tl)
    top = blank(1)
    for i in range(8):
        top[0][i] = 0; top[1][i] = 11; top[2][i] = 2; top[3][i] = 12
    for i in range(0, 8, 4):
        top[2][i] = 13
    add("win_t", top)
    add("win_fill", blank(1))
    left = blank(1)
    for y in range(8):
        left[y][0] = 0; left[y][1] = 11; left[y][2] = 2; left[y][3] = 12
    for y in range(0, 8, 4):
        left[y][2] = 13
    add("win_l", left)
    # flipped pieces are produced with tile flip bits in C
    # cursor arrow
    cur = blank(1)
    arrow = ["        ", " #      ", " ##     ", " ###    ", " ####   ", " ###    ", " ##     ", " #      "]
    for y, r in enumerate(arrow):
        for x, ch in enumerate(r):
            if ch == '#':
                cur[y][x] = 5
    add("cursor", cur)
    # down-arrow (continue prompt)
    da = blank(1)
    for y, r in enumerate(["        ", "        ", " ###### ", "  ####  ", "   ##   ", "        ", "        ", "        "]):
        for x, ch in enumerate(r):
            if ch == '#':
                da[y][x] = 5
    add("more", da)
    # HP bar: left cap, 0..8 fill segments, right cap
    cap = blank(1)
    for y in range(2, 6):
        cap[y][7] = 13
    cap[1][7] = cap[6][7] = 11
    add("bar_l", cap)
    for n in range(9):
        b = blank(1)
        for x in range(8):
            b[1][x] = 13; b[6][x] = 13
            for y in range(2, 6):
                b[y][x] = 7 if x < n else 12
            if x < n:
                b[2][x] = 15 if n > 0 else 12
        add("bar%d" % n, b)
    capr = blank(1)
    for y in range(2, 6):
        capr[y][0] = 13
    add("bar_r", capr)
    # transparent tile (for clearing BG0 over maps)
    add("clear", blank(0))
    # small icons: heart(bond), moon, sun, rain, fog, star(rare), skull(fainted), check(caught), eye(seen)
    icons = {
        "ic_heart": ["        ", " ## ##  ", "####### ", "####### ", " #####  ", "  ###   ", "   #    ", "        "],
        "ic_moon": ["  ###   ", " ##     ", "##      ", "##      ", "##      ", " ##     ", "  ###   ", "        "],
        "ic_sun": ["   #    ", " # # #  ", "  ###   ", "###### ", "  ###   ", " # # #  ", "   #    ", "        "],
        "ic_rain": ["  ##    ", " #### # ", "####### ", "        ", " # # #  ", "# # #   ", "        ", "        "],
        "ic_fog": ["        ", " ###### ", "        ", "####### ", "        ", " ###### ", "        ", "        "],
        "ic_star": ["   #    ", "   #    ", "#######", " ##### ", "  ###   ", " ## ##  ", " #   #  ", "        "],
        "ic_skull": [" #####  ", "#######", "# ## ##", "#######", " ## ##  ", " #####  ", " # # #  ", "        "],
        "ic_ball": ["  ###   ", " #   #  ", "# ### # ", "##   ## ", "# ### # ", " #   #  ", "  ###   ", "        "],
        "ic_eye": ["        ", "  ###   ", " #   #  ", "#  #  # ", " #   #  ", "  ###   ", "        ", "        "],
        "ic_blood": ["   #    ", "  ###   ", "  ###   ", " ##### ", " ##### ", "  ###   ", "        ", "        "],
    }
    colors = {"ic_heart": 14, "ic_moon": 8, "ic_sun": 5, "ic_rain": 9, "ic_fog": 13, "ic_star": 8,
              "ic_skull": 13, "ic_ball": 6, "ic_eye": 9, "ic_blood": 6}
    for name, rows in icons.items():
        g = blank(1)
        for y, r in enumerate(rows):
            for x, ch in enumerate(r[:8]):
                if ch == '#':
                    g[y][x] = colors[name]
        add(name, g)
    # type badge tiles are drawn as text in C
    return tiles, ui


# ---------------------------------------------------------------- metatiles
A_WALK, A_SOLID, A_GRASS, A_WATER, A_DOOR, A_FOG, A_COUNTER, A_SIGN = range(8)


def build_metatiles():
    rng = random.Random(1337)
    mts = []  # (char, name, bank, grid, attr)

    def new(c):
        return [[c] * 16 for _ in range(16)]

    def speck(g, cols, n, area=(0, 0, 16, 16)):
        x0, y0, x1, y1 = area
        for _ in range(n):
            g[rng.randrange(y0, y1)][rng.randrange(x0, x1)] = rng.choice(cols)

    def rect(g, x0, y0, x1, y1, c):
        for y in range(max(0, y0), min(16, y1)):
            for x in range(max(0, x0), min(16, x1)):
                g[y][x] = c

    def ell(g, cx, cy, rx, ry, c):
        for y in range(16):
            for x in range(16):
                if ((x + .5 - cx) / rx) ** 2 + ((y + .5 - cy) / ry) ** 2 <= 1:
                    g[y][x] = c

    def add(ch, name, bank, g, attr):
        mts.append((ch, name, bank, g, attr))
        return g

    def grass(dark=False):
        g = new(3 if dark else 4)
        speck(g, [2, 3] if dark else [3, 5], 40)
        for _ in range(6):
            x, y = rng.randrange(1, 15), rng.randrange(1, 15)
            g[y][x] = 4 if dark else 5
            g[y - 1][x] = 5 if dark else 6
        return g

    add('.', "grass", 4, grass(), A_WALK)
    add('"', "darkgrass", 4, grass(True), A_WALK)
    # tall grass: clumps of blades
    g = new(3)
    speck(g, [2], 20)
    for (bx, by) in [(1, 7), (9, 7), (5, 15), (13, 15)]:
        for dx, h in [(0, 5), (2, 6), (4, 5), (6, 4)]:
            for k in range(h):
                y = by - k
                x = bx + dx + (1 if k > 3 else 0)
                if 0 <= y < 16 and 0 <= x < 16:
                    g[y][x] = 6 if k == h - 1 else (5 if k > 1 else 2)
    add(',', "tallgrass", 4, g, A_GRASS)
    g = new(9)
    speck(g, [8, 10], 50)
    for _ in range(4):
        x, y = rng.randrange(1, 15), rng.randrange(1, 15)
        g[y][x] = 10; g[y + 1][x] = 7
    add('=', "path", 4, g, A_WALK)
    # pine tree
    g = grass()
    for y in range(16):
        for x in range(16):
            w = (y + 1) * 0.5 if y < 13 else 0
            if y < 13 and abs(x + .5 - 8) <= w:
                g[y][x] = 2 if x > 8 else 3
                if (y % 4 == 3) and abs(x + .5 - 8) > w - 2:
                    g[y][x] = 4
    rect(g, 7, 13, 9, 16, 7)
    g[13][8] = 8
    for y in range(13):
        for x in range(16):
            if g[y][x] in (2, 3, 4):
                nb = [(x - 1, y), (x + 1, y), (x, y + 1)]
                if any(not (0 <= a < 16 and 0 <= b < 16) or g[b][a] in (5, 6) for a, b in nb):
                    g[y][x] = 1
    add('T', "tree", 4, g, A_SOLID)
    # dead tree on grave ground
    g = new(5)
    speck(g, [4, 6], 30)
    rect(g, 7, 6, 9, 16, 3)
    for x0, y0, dx in [(7, 7, -1), (8, 5, 1), (7, 9, -1), (8, 10, 1), (7, 3, -1)]:
        x, y = x0, y0
        for k in range(5):
            if 0 <= x < 16 and 0 <= y < 16:
                g[y][x] = 2
            x += dx
            y -= 1 if k % 2 == 0 else 0
    rect(g, 7, 2, 9, 7, 2)
    g[15][6] = 2; g[15][9] = 2
    add('t', "deadtree", 10, g, A_SOLID)
    # water
    g = new(4)
    for y in range(0, 16, 4):
        off = (y // 4) * 3
        for x in range(16):
            if (x + off) % 8 in (0, 1, 2):
                g[y + 1][x] = 5
            if (x + off) % 8 == 1:
                g[y][x] = 6
    speck(g, [7], 3)
    speck(g, [3], 10)
    add('~', "water", 7, g, A_WATER)
    g = grass()
    for _ in range(5):
        x, y = rng.randrange(2, 14), rng.randrange(2, 14)
        c = rng.choice([11, 12])
        for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
            g[y + dy][x + dx] = c
        g[y][x] = 13
    add('f', "flowers", 4, g, A_WALK)
    g = grass()
    rect(g, 0, 5, 16, 7, 8); rect(g, 0, 10, 16, 12, 8)
    for y in (5, 10):
        for x in range(16):
            g[y][x] = 9
    for px in (2, 13):
        rect(g, px, 3, px + 2, 15, 7)
        g[3][px] = 9
    add('F', "fence", 4, g, A_SOLID)
    # gothic brick wall
    g = new(2)
    for row in range(4):
        y0 = row * 4
        off = 0 if row % 2 == 0 else 4
        for bx in range(-1, 3):
            x0 = bx * 8 + off
            rect(g, x0 + 1, y0 + 1, x0 + 8, y0 + 4, rng.choice([3, 4, 4]))
            for x in range(x0 + 1, x0 + 8):
                if 0 <= x < 16:
                    g[y0 + 1][x] = 5
    speck(g, [14, 15], 6)
    add('#', "wall", 5, g, A_SOLID)
    # roof shingles
    g = new(3)
    for row in range(4):
        y0 = row * 4
        off = 0 if row % 2 == 0 else 2
        for x in range(16):
            g[y0][x] = 2
            if (x + off) % 4 == 0:
                g[y0 + 1][x] = 2; g[y0 + 2][x] = 2
            g[y0 + 3][x] = 4 if (x + off) % 4 == 2 else g[y0 + 3][x]
        for x in range(16):
            if g[y0 + 1][x] == 3:
                g[y0 + 1][x] = 5 if (x + off) % 4 == 1 else 4
    add('R', "roof", 11, g, A_SOLID)

    def wall_base():
        g = new(7)
        for y in range(0, 16, 4):
            for x in range(16):
                g[y][x] = 6
        speck(g, [8], 12)
        return g

    g = wall_base()
    rect(g, 4, 3, 12, 13, 1)
    rect(g, 5, 4, 11, 12, 11)
    rect(g, 5, 4, 8, 7, 12)
    rect(g, 7, 4, 9, 12, 9)
    rect(g, 5, 7, 11, 8, 9)
    rect(g, 3, 13, 13, 14, 13)
    add('W', "window", 11, g, A_SOLID)
    add('V', "housewall", 11, wall_base(), A_SOLID)
    g = wall_base()
    rect(g, 3, 2, 13, 16, 1)
    rect(g, 4, 3, 12, 16, 10)
    for x in range(4, 12, 2):
        rect(g, x, 3, x + 1, 16, 9)
    rect(g, 4, 3, 12, 4, 13)
    g[9][10] = 13; g[9][11] = 12
    add('D', "door", 11, g, A_DOOR)
    # gravestone
    g = new(5)
    speck(g, [4, 6], 30)
    ell(g, 8, 6, 5, 5, 8)
    rect(g, 3, 6, 13, 15, 8)
    for y in range(16):
        for x in range(16):
            if g[y][x] == 8 and x >= 10:
                g[y][x] = 7
            if g[y][x] == 8 and x <= 4:
                g[y][x] = 9
    rect(g, 7, 4, 9, 11, 7); rect(g, 5, 6, 11, 8, 7)
    rect(g, 2, 15, 14, 16, 4)
    for y in range(16):
        for x in range(16):
            if g[y][x] in (7, 8, 9):
                for a, b in [(x - 1, y), (x + 1, y), (x, y - 1)]:
                    if 0 <= a < 16 and 0 <= b < 16 and g[b][a] in (4, 5, 6):
                        g[b][a] = 1
    add('+', "grave", 10, g, A_SOLID)
    g = new(12)
    speck(g, [11, 13], 50)
    for _ in range(5):
        x, y = rng.randrange(1, 15), rng.randrange(2, 15)
        g[y][x] = 5; g[y - 1][x] = 6; g[y][x + 1 if x < 15 else x] = 5
    speck(g, [15], 2)
    add('x', "gravedirt", 10, g, A_GRASS)
    # cobblestones
    g = new(3)
    for (cx, cy) in [(3, 3), (10, 2), (6, 8), (13, 9), (2, 13), (9, 14), (15, 4)]:
        ell(g, cx, cy, 3.4, 2.6, rng.choice([4, 5]))
        if 0 <= cy - 2 < 16:
            for x in range(max(0, cx - 2), min(16, cx + 2)):
                if g[cy - 2][x] in (4, 5):
                    g[cy - 2][x] = 6
    add('c', "cobble", 5, g, A_WALK)
    g = new(4)
    for x in range(16):
        g[0][x] = 3; g[8][x] = 3
    for y in range(16):
        g[y][0] = 3; g[y][8] = 3
    speck(g, [5], 10)
    add('N', "sidewalk", 5, g, A_WALK)

    def neon(color, light, shape):
        g = new(12)
        for y in range(0, 16, 5):
            for x in range(16):
                g[y][x] = 13
        rect(g, 1, 3, 15, 13, 2)
        for y, r in enumerate(shape):
            for x, ch in enumerate(r):
                if ch == '#':
                    g[4 + y][2 + x] = color
                    if y > 0 and g[3 + y][2 + x] == 2:
                        g[3 + y][2 + x] = 3
        for y in range(16):
            for x in range(16):
                if g[y][x] == color and (x + y) % 3 == 0:
                    g[y][x] = light
        return g

    add('n', "neonpink", 6, neon(4, 5, ["            ", " ##  # #  # ", "#  # # ## # ", "#  # # # ## ", " ##  # #  # ", "            ", "            ", " ########## "]), A_SOLID)
    add('j', "neoncyan", 6, neon(6, 7, ["   ####     ", "  #    #    ", " # #  # #   ", " #  ##  #   ", "  #    #    ", "   ####     ", "            ", " ########## "]), A_SOLID)
    add('J', "neonviolet", 6, neon(8, 9, ["  #     #   ", "  ##   ##   ", "  # # # #   ", "  #  #  #   ", "  #     #   ", "  #     #   ", "            ", " ########## "]), A_SOLID)
    g = new(2)
    speck(g, [3], 30)
    speck(g, [4, 6, 8], 4)
    for x in range(2, 14):
        if x % 6 < 3:
            g[15][x] = 10
    add('a', "asphalt", 6, g, A_WALK)
    g = new(2)
    speck(g, [3], 30)
    for y in range(16):
        if y % 6 < 3:
            g[y][7] = 10
            g[y][8] = 11
    add('A', "roadline", 6, g, A_WALK)
    # lamp post
    g = new(3)
    for (cx, cy) in [(3, 3), (12, 12), (3, 13)]:
        ell(g, cx, cy, 3, 2.5, 4)
    rect(g, 7, 5, 9, 15, 1)
    rect(g, 6, 14, 10, 16, 2)
    rect(g, 5, 1, 11, 5, 1)
    rect(g, 6, 2, 10, 4, 11)
    g[2][7] = 12; g[2][8] = 12
    g[0][7] = 1; g[0][8] = 1
    add('L', "lamp", 5, g, A_SOLID)
    g = new(2)
    for y in range(16):
        for x in range(16):
            if x % 8 == 0 or y % 8 == 0:
                g[y][x] = 1
    for (wx, wy) in [(2, 2), (10, 2), (2, 10), (10, 10)]:
        lit = rng.random() < 0.5
        rect(g, wx, wy, wx + 4, wy + 4, 11 if lit else 3)
        if lit:
            g[wy][wx] = 12
    add('b', "building", 5, g, A_SOLID)
    # interior
    g = new(4)
    for y in range(16):
        if y % 4 == 0:
            for x in range(16):
                g[y][x] = 3
    for y in range(16):
        x = (y // 4) * 5 % 16
        g[y][x] = 3
    speck(g, [5], 12)
    add('w', "woodfloor", 8, g, A_WALK)
    g = new(8)
    for i in range(16):
        g[0][i] = 10; g[15][i] = 10
    for y in range(2, 14, 4):
        for x in range(2, 14, 4):
            g[y][x] = 9; g[y + 1][x + 1] = 7
    add('r', "carpet", 8, g, A_WALK)
    g = new(5)
    rect(g, 0, 0, 16, 6, 6)
    rect(g, 0, 0, 16, 1, 1)
    rect(g, 0, 5, 16, 6, 10)
    rect(g, 0, 6, 16, 16, 3)
    for x in range(0, 16, 4):
        rect(g, x, 7, x + 1, 16, 2)
    add('C', "counter", 8, g, A_COUNTER)
    g = new(3)
    for shelf in (0, 8):
        rect(g, 0, shelf + 7, 16, shelf + 8, 2)
        x = 1
        while x < 15:
            w = rng.choice([1, 2])
            c = rng.choice([12, 13, 7, 10, 11])
            h = rng.choice([5, 6])
            rect(g, x, shelf + 7 - h, x + w, shelf + 7, c)
            x += w + (1 if rng.random() < 0.3 else 0)
    for y in range(16):
        g[y][0] = 1; g[y][15] = 1
    add('k', "bookshelf", 8, g, A_SOLID)
    g = new(7)
    for x in range(16):
        if x % 4 == 1:
            for y in range(12):
                g[y][x] = 8
    rect(g, 0, 11, 16, 13, 10)
    rect(g, 0, 13, 16, 16, 3)
    for x in range(1, 16, 4):
        g[5][x] = 10
    add('m', "wallpaper", 8, g, A_SOLID)
    g = new(5)
    rect(g, 1, 3, 15, 10, 6)
    rect(g, 1, 3, 15, 4, 10)
    rect(g, 2, 10, 4, 16, 3); rect(g, 12, 10, 14, 16, 3)
    rect(g, 6, 1, 7, 4, 11); g[0][6] = 14
    add('B', "table", 8, g, A_SOLID)
    g = new(4)
    rect(g, 2, 2, 14, 14, 5)
    for i in range(2, 14):
        g[i][i] = 3; g[i][15 - i] = 3
    for i in range(2, 14):
        g[2][i] = 6; g[i][2] = 6; g[13][i] = 2; g[i][13] = 2
    add('e', "crate", 8, g, A_SOLID)
    g = new(4)
    for y in range(16):
        for x in range(16):
            if x % 4 == 0:
                g[y][x] = 3
    for x in range(16):
        g[0][x] = 2; g[15][x] = 2
    speck(g, [5], 8)
    add('|', "bridge", 8, g, A_WALK)
    # rift
    g = new(1)
    speck(g, [2, 3], 40)
    speck(g, [6, 8, 14], 4)
    add('v', "void", 9, g, A_SOLID)
    g = new(4)
    for y in range(16):
        for x in range(16):
            if (x + y) % 8 == 0 or (x - y) % 8 == 0:
                g[y][x] = 5
    for i in range(16):
        g[0][i] = 11; g[i][0] = 10
    speck(g, [6], 6)
    add('p', "riftfloor", 9, g, A_WALK)
    g = new(3)
    ell(g, 8, 8, 7, 7, 4)
    ell(g, 8, 8, 5, 5, 3)
    for k in range(8):
        import math
        a = k * math.pi / 4
        x = int(8 + math.cos(a) * 6)
        y = int(8 + math.sin(a) * 6)
        g[min(15, y)][min(15, x)] = 12
    rect(g, 7, 4, 9, 12, 13); rect(g, 4, 7, 12, 9, 13)
    g[8][8] = 14
    add('g', "riftglyph", 9, g, A_GRASS)
    # stone floor / castle
    g = new(3)
    for y in range(16):
        g[y][0] = 2
        g[y][8 if y < 8 else 4] = 2
    for x in range(16):
        g[0][x] = 2; g[8][x] = 2
    speck(g, [4], 14)
    x, y = 11, 2
    for _ in range(5):
        g[y][x] = 2; x += rng.choice([-1, 0, 1]); y += 1
    add('s', "stonefloor", 5, g, A_WALK)
    g = new(4)
    for row in range(2):
        y0 = row * 8
        off = 0 if row == 0 else 6
        for x in range(16):
            g[y0][x] = 2
            g[y0 + 1][x] = 6
        for x in (off % 16, (off + 8) % 16):
            rect(g, x, y0, x + 1, y0 + 8, 2)
        rect(g, 0, y0 + 6, 16, y0 + 8, 3)
    speck(g, [5], 8)
    add('S', "castlewall", 5, g, A_SOLID)
    g = new(3)
    for x in range(1, 16, 3):
        rect(g, x, 0, x + 2, 16, 1)
        g[0][x] = 6
        for y in range(1, 16):
            g[y][x + 1 if x + 1 < 16 else x] = 4
    rect(g, 0, 3, 16, 4, 1); rect(g, 0, 12, 16, 13, 1)
    add('G', "gate", 5, g, A_SOLID)
    g = grass(True)
    for y in range(16):
        for x in range(16):
            v = (x * 3 + y * 7 + (x * y) % 5) % 9
            if v < 4:
                g[y][x] = 14
            elif v < 6:
                g[y][x] = 15
    add('q', "fog", 4, g, A_FOG)
    g = new(3)
    ell(g, 8, 8, 7.5, 7.5, 2)
    ell(g, 8, 8, 6, 6, 11)
    ell(g, 8, 8, 4, 4, 12)
    rect(g, 7, 3, 9, 13, 11); rect(g, 3, 7, 13, 9, 11)
    add('h', "altar", 5, g, A_SOLID)
    g = new(3)
    speck(g, [2], 20)
    for (cx, cy, r) in [(4, 4, 3), (11, 6, 2.5), (6, 12, 2.5), (13, 13, 2)]:
        ell(g, cx, cy, r, r * 0.8, 4)
        ell(g, cx - 0.7, cy - 0.7, r * 0.5, r * 0.4, 6)
    add('u', "rubble", 5, g, A_GRASS)
    g = new(3)
    for y in range(16):
        c = [6, 5, 4, 3][y // 4]
        for x in range(16):
            g[y][x] = c if y % 4 else 2
    add('^', "stairs", 5, g, A_DOOR)
    add('_', "black", 5, new(1), A_SOLID)
    g = new(3)
    rect(g, 3, 0, 13, 16, 5)
    rect(g, 3, 0, 6, 16, 7)
    rect(g, 10, 0, 13, 16, 4)
    rect(g, 2, 0, 14, 2, 6); rect(g, 2, 14, 14, 16, 4)
    for y in range(16):
        g[y][2 if y > 1 and y < 14 else 1] = 1; g[y][13 if y > 1 and y < 14 else 14] = 1
    add('P', "pillar", 5, g, A_SOLID)
    g = new(3)
    rect(g, 3, 12, 13, 16, 4)
    ell(g, 8, 8, 4, 4.5, 5)
    for (x, y) in [(3, 3), (4, 4), (5, 5), (12, 3), (11, 4), (10, 5)]:
        g[y][x] = 4
    ell(g, 8, 9, 2, 1, 4)
    g[7][6] = 12; g[7][9] = 12
    for (x, y) in [(2, 6), (3, 7), (13, 6), (12, 7), (1, 5), (14, 5)]:
        g[y][x] = 4
    add('Y', "statue", 5, g, A_SOLID)
    g = grass()
    rect(g, 7, 9, 9, 16, 7)
    rect(g, 2, 3, 14, 10, 8)
    rect(g, 2, 3, 14, 4, 10)
    for x in range(4, 12, 2):
        g[6][x] = 7
    add('z', "sign", 4, g, A_SIGN)
    g = new(5)
    speck(g, [4, 6], 30)
    for x in range(1, 16, 3):
        rect(g, x, 2, x + 1, 15, 1)
        g[1][x] = 9
    rect(g, 0, 4, 16, 5, 1); rect(g, 0, 12, 16, 13, 1)
    add('i', "ironfence", 10, g, A_SOLID)
    g = grass()
    for (x, y) in [(4, 4), (11, 9), (6, 12)]:
        g[y][x] = 13
        for d in (-1, 1):
            g[y + d][x] = 11; g[y][x + d] = 11
    add('*', "clue", 4, g, A_SIGN)

    def brazier(lit):
        g = new(3)
        speck(g, [2, 4], 15)
        rect(g, 4, 9, 12, 12, 5)
        rect(g, 5, 12, 11, 13, 4)
        rect(g, 7, 13, 9, 15, 4)
        rect(g, 5, 15, 11, 16, 2)
        rect(g, 4, 9, 12, 10, 6)
        if lit:
            for y, r in enumerate(["   #    ", "  ##  # ", " ###  ##", " ####### ", "#########", " ####### ", "  #####  "]):
                for x, ch in enumerate(r):
                    if ch == '#' and 3 + x < 16:
                        g[2 + y][4 + x - 0] = 11 if (x + y) % 3 else 12
            g[7][7] = 13
        else:
            rect(g, 5, 8, 11, 9, 1)
        return g

    add('o', "brazier", 5, brazier(False), A_SOLID)
    add('O', "brazierlit", 5, brazier(True), A_SOLID)
    # throne
    g = new(3)
    rect(g, 3, 1, 13, 16, 13)
    rect(g, 4, 2, 12, 12, 8)
    rect(g, 3, 1, 13, 2, 11)
    for x in (3, 8, 12):
        g[0][x] = 11
    rect(g, 2, 12, 14, 16, 4)
    add('K', "throne", 5, g, A_SOLID)
    # portal
    g = new(1)
    ell(g, 8, 8, 7.5, 7.5, 6)
    ell(g, 8, 8, 6, 6, 8)
    ell(g, 8, 8, 4.5, 4.5, 13)
    ell(g, 8, 8, 2.5, 2.5, 14)
    add('@', "portal", 9, g, A_DOOR)
    # cliff/hedge in the woods: dark thicket
    g = new(2)
    for _ in range(9):
        cx, cy = rng.uniform(1, 15), rng.uniform(1, 15)
        ell(g, cx, cy, 3, 3, rng.choice([3, 4]))
        g[int(cy) - 1][int(cx)] = 5
    speck(g, [1], 10)
    add('H', "thicket", 4, g, A_SOLID)
    # moon pool (glowing water)
    g = new(4)
    ell(g, 8, 8, 6, 5, 6)
    ell(g, 7, 7, 2.5, 2, 7)
    add('M', "moonpool", 7, g, A_WATER)
    # bed
    g = new(4)
    rect(g, 1, 0, 15, 16, 2)
    rect(g, 2, 1, 14, 5, 11)
    rect(g, 2, 5, 14, 15, 8)
    rect(g, 2, 5, 14, 6, 9)
    add('Z', "bed", 8, g, A_SOLID)
    return mts


# ---------------------------------------------------------------- main build


def quantize_scene(img, ncolors=15):
    """Quantize an RGB image to <=15 colors + index 0 (unused/transparent)."""
    q = img.convert("RGB").quantize(colors=ncolors, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = q.getpalette()[:ncolors * 3]
    w, h = q.size
    px = q.load()
    grid = [[px[x, y] + 1 for x in range(w)] for y in range(h)]
    colors = ["#000000"] + ["#%02x%02x%02x" % tuple(pal[i * 3:i * 3 + 3]) for i in range(ncolors)]
    while len(colors) < 16:
        colors.append("#000000")
    return grid, colors


def tilemap_from_grid(grid, bank, base_tile=0):
    """Deduplicate tiles (with flips) and produce map entries."""
    h, w = len(grid), len(grid[0])
    tiles, index, entries = [], {}, []
    for ty in range(h // 8):
        for tx in range(w // 8):
            t = tile4(grid, tx * 8, ty * 8)
            rows = [[(t[y] >> (4 * x)) & 15 for x in range(8)] for y in range(8)]
            found = None
            for flip, (fx, fy) in enumerate([(0, 0), (1, 0), (0, 1), (1, 1)]):
                rr = rows
                if fx:
                    rr = [r[::-1] for r in rr]
                if fy:
                    rr = rr[::-1]
                key = tuple(tuple(r) for r in rr)
                if key in index:
                    found = (index[key], fx, fy)
                    break
            if found is None:
                key = tuple(tuple(r) for r in rows)
                index[key] = len(tiles)
                tiles.append(t)
                found = (index[key], 0, 0)
            ti, fx, fy = found
            entries.append((base_tile + ti) | (fx << 10) | (fy << 11) | (bank << 12))
    return tiles, entries


def main():
    outdir = sys.argv[1]
    preview = "--preview" in sys.argv
    os.makedirs(outdir, exist_ok=True)
    out = Out()

    # --- UI
    ui_tiles, ui = build_ui_tiles()
    out.arr("uint32_t", "ui_tiles", [w for t in ui_tiles for w in t], 8)
    out.raw_h("#define UI_TILE_COUNT %d" % len(ui_tiles))
    for name, idx in ui.items():
        out.raw_h("#define UI_%s %d" % (name.upper(), idx))
    out.arr("uint16_t", "ui_pals", [rgb15(c) for p in ui_palettes() for c in p], 16)

    # --- BG palettes (banks 4..13)
    bgp = []
    emis = []
    for bank in range(4, 14):
        p = BG_PALS.get(bank, ["#000000"] * 16)
        bgp += [rgb15(c) for c in p]
        m = 0
        for i in EMISSIVE.get(bank, []):
            m |= 1 << i
        emis.append(m)
    out.arr("uint16_t", "bg_pals", bgp, 16)
    out.arr("uint16_t", "bg_emissive", emis, 16)

    # --- metatiles
    mts = build_metatiles()
    mt_tiles, mt_index, mt_entries, mt_attr = [], {}, [], []
    for ch, name, bank, g, attr in mts:
        for (tx, ty) in [(0, 0), (8, 0), (0, 8), (8, 8)]:
            t = tile4(g, tx, ty)
            if t not in mt_index:
                mt_index[t] = len(mt_tiles)
                mt_tiles.append(t)
            mt_entries.append(mt_index[t] | (bank << 12))
        mt_attr.append(attr)
    out.arr("uint32_t", "mt_tiles", [w for t in mt_tiles for w in t], 8)
    out.raw_h("#define MT_TILE_COUNT %d" % len(mt_tiles))
    out.arr("uint16_t", "mt_entries", mt_entries, 16)
    out.arr("uint8_t", "mt_attr", mt_attr, 32)
    out.raw_h("#define MT_COUNT %d" % len(mts))
    chmap = {}
    for i, (ch, name, bank, g, attr) in enumerate(mts):
        chmap[ch] = i
        out.raw_h("#define MT_%s %d" % (name.upper(), i))
    out.raw_h("enum { A_WALK, A_SOLID, A_GRASS, A_WATER, A_DOOR, A_FOG, A_COUNTER, A_SIGN };")

    # --- maps
    map_names = []
    out.raw_c("")
    for mi, m in enumerate(mapdata.MAPS):
        rows = m["rows"]
        w = len(rows[0])
        for r in rows:
            assert len(r) == w, (m["name"], r, len(r), w)
        data = [chmap[c] for r in rows for c in r]
        out.arr("uint8_t", "map_%s" % m["id"], data, w)
        map_names.append(m)
        out.raw_h("#define MAP_%s %d" % (m["id"].upper(), mi))
    out.raw_h("#define MAP_COUNT %d" % len(mapdata.MAPS))
    out.raw_h("typedef struct { const uint8_t *data; uint8_t w, h, bgm, area; const char *name; } MapDef;")
    out.raw_h("extern const MapDef map_defs[];")
    out.raw_c("const MapDef map_defs[] = {\n%s\n};" % "\n".join(
        '    { map_%s, %d, %d, %d, %d, "%s" },' % (m["id"], len(m["rows"][0]), len(m["rows"]), m["bgm"], m["area"], m["title"])
        for m in mapdata.MAPS))

    # --- people
    ppl_tiles, ppl_pals = [], []
    for p in scenes.build_people():
        for frame in p["frames"]:
            for (tx, ty) in [(0, 0), (8, 0), (0, 8), (8, 8)]:
                ppl_tiles.append(tile4(frame, tx, ty))
        ppl_pals += [rgb15(c) for c in p["pal"]]
    out.arr("uint32_t", "person_tiles", [w for t in ppl_tiles for w in t], 8)
    out.arr("uint16_t", "person_pals", ppl_pals, 16)
    out.raw_h("#define PERSON_COUNT %d" % len(scenes.PEOPLE))
    for i, p in enumerate(scenes.PEOPLE):
        out.raw_h("#define PS_%s %d" % (p["id"].upper(), i))
    out.arr("uint16_t", "player_coat_pals", [rgb15(c) for pair in scenes.PLAYER_COATS for c in pair], 16)
    out.arr("uint16_t", "player_hair_pals", [rgb15(c) for pair in scenes.PLAYER_HAIRS for c in pair], 16)

    # --- monsters
    mon_tiles, mon_pals, mon_vpals, icon_tiles = [], [], [], []
    previews = []
    for sp in monsters.SPECIES:
        g = sp["sprite"]
        for ty in range(4):
            for tx in range(4):
                mon_tiles.append(tile4(g, tx * 8, ty * 8))
        ic = monsters.make_icon(g)
        for (tx, ty) in [(0, 0), (8, 0), (0, 8), (8, 8)]:
            icon_tiles.append(tile4(ic, tx, ty))
        mon_pals += [rgb15(c) for c in sp["pal"]]
        mon_vpals += [rgb15(c) for c in sp["vpal"]]
        previews.append((g, sp["pal"], sp["vpal"], ic, sp["name"]))
    out.arr("uint32_t", "mon_tiles", [w for t in mon_tiles for w in t], 8)
    out.arr("uint32_t", "mon_icon_tiles", [w for t in icon_tiles for w in t], 8)
    out.arr("uint16_t", "mon_pals", mon_pals, 16)
    out.arr("uint16_t", "mon_vpals", mon_vpals, 16)
    out.raw_h("#define SPECIES_COUNT %d" % len(monsters.SPECIES))
    for i, sp in enumerate(monsters.SPECIES):
        out.raw_h("#define SP_%s %d" % (sp["name"].upper(), i))

    # --- title + battle backdrops (full-screen tiled images)
    scene_imgs = scenes.build_scenes()
    out.raw_h("typedef struct { const uint32_t *tiles; uint16_t ntiles; const uint16_t *map; const uint16_t *pal; uint8_t w, h; } SceneDef;")
    out.raw_h("extern const SceneDef scene_defs[];")
    sdefs = []
    for si, (sid, img) in enumerate(scene_imgs):
        grid, cols = quantize_scene(img)
        tiles, entries = tilemap_from_grid(grid, 0)
        assert len(tiles) <= 512, (sid, len(tiles))
        out.arr("uint32_t", "scene_%s_tiles" % sid, [w for t in tiles for w in t], 8)
        out.arr("uint16_t", "scene_%s_map" % sid, entries, img.size[0] // 8)
        out.arr("uint16_t", "scene_%s_pal" % sid, [rgb15(c) for c in cols], 16)
        out.raw_h("#define SCENE_%s %d" % (sid.upper(), si))
        sdefs.append("    { scene_%s_tiles, %d, scene_%s_map, scene_%s_pal, %d, %d }," % (
            sid, len(tiles), sid, sid, img.size[0] // 8, img.size[1] // 8))
        if preview:
            os.makedirs(os.path.join(outdir, "preview"), exist_ok=True)
            img.save(os.path.join(outdir, "preview", "scene_%s.png" % sid))
        print("scene %-8s %3d tiles" % (sid, len(tiles)))
    out.raw_c("const SceneDef scene_defs[] = {\n%s\n};" % "\n".join(sdefs))

    out.write(outdir)
    print("ui tiles %d, metatile tiles %d (%d metatiles), species %d" % (
        len(ui_tiles), len(mt_tiles), len(mts), len(monsters.SPECIES)))

    if preview:
        pv = os.path.join(outdir, "preview")
        os.makedirs(pv, exist_ok=True)
        # monster sheet
        cols = 9
        rows = (len(previews) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * 72, rows * 84), (30, 30, 40))
        d = ImageDraw.Draw(sheet)
        for i, (g, pal, vpal, ic, name) in enumerate(previews):
            ox, oy = (i % cols) * 72, (i // cols) * 84
            for (pp, dx) in [(pal, 0)]:
                for y in range(32):
                    for x in range(32):
                        if g[y][x]:
                            c = hex2rgb(pp[g[y][x]])
                            d.rectangle([ox + x * 2, oy + y * 2, ox + x * 2 + 1, oy + y * 2 + 1], fill=c)
            for y in range(16):
                for x in range(16):
                    if ic[y][x]:
                        sheet.putpixel((ox + 52 + x, oy + y), hex2rgb(pal[ic[y][x]]))
            for y in range(32):
                for x in range(32):
                    if g[y][x]:
                        sheet.putpixel((ox + 52 + x // 2, oy + 20 + y // 2), hex2rgb(vpal[g[y][x]]))
            d.text((ox, oy + 66), name[:11], fill=(220, 220, 220))
        sheet.save(os.path.join(pv, "monsters.png"))
        # metatile sheet
        allp = {b: [hex2rgb(c) for c in p] for b, p in BG_PALS.items()}
        ms = Image.new("RGB", (16 * 18, ((len(mts) + 15) // 16) * 18), (0, 0, 0))
        for i, (ch, name, bank, g, attr) in enumerate(mts):
            ox, oy = (i % 16) * 18, (i // 16) * 18
            for y in range(16):
                for x in range(16):
                    ms.putpixel((ox + x, oy + y), allp[bank][g[y][x]])
        ms = ms.resize((ms.size[0] * 3, ms.size[1] * 3), Image.NEAREST)
        ms.save(os.path.join(pv, "metatiles.png"))
        # maps
        mtimg = {}
        for i, (ch, name, bank, g, attr) in enumerate(mts):
            im = Image.new("RGB", (16, 16))
            for y in range(16):
                for x in range(16):
                    im.putpixel((x, y), allp[bank][g[y][x]])
            mtimg[ch] = im
        for m in mapdata.MAPS:
            rws = m["rows"]
            im = Image.new("RGB", (len(rws[0]) * 16, len(rws) * 16))
            for y, r in enumerate(rws):
                for x, ch in enumerate(r):
                    im.paste(mtimg[ch], (x * 16, y * 16))
            im.save(os.path.join(pv, "map_%s.png" % m["id"]))
        # people
        ps = Image.new("RGB", (9 * 18, len(scenes.PEOPLE) * 18), (60, 60, 70))
        for pi, p in enumerate(scenes.build_people()):
            pal = [hex2rgb(c) for c in p["pal"]]
            for fi, f in enumerate(p["frames"]):
                for y in range(16):
                    for x in range(16):
                        if f[y][x]:
                            ps.putpixel((fi * 18 + x, pi * 18 + y), pal[f[y][x]])
        ps.resize((ps.size[0] * 4, ps.size[1] * 4), Image.NEAREST).save(os.path.join(pv, "people.png"))


if __name__ == "__main__":
    main()
