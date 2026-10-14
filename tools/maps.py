"""Area maps for MonsterMons. Each map is a grid of metatile characters
(legend in gen_assets.build_metatiles). Warps, NPCs and events live in C
(src/world_data.c) and reference the coordinates noted here."""


class Grid:
    def __init__(self, w, h, fill):
        self.w, self.h = w, h
        self.g = [[fill] * w for _ in range(h)]

    def rect(self, x0, y0, x1, y1, ch):  # inclusive
        for y in range(max(0, y0), min(self.h, y1 + 1)):
            for x in range(max(0, x0), min(self.w, x1 + 1)):
                self.g[y][x] = ch
        return self

    def border(self, ch, t=1):
        self.rect(0, 0, self.w - 1, t - 1, ch)
        self.rect(0, self.h - t, self.w - 1, self.h - 1, ch)
        self.rect(0, 0, t - 1, self.h - 1, ch)
        self.rect(self.w - t, 0, self.w - 1, self.h - 1, ch)
        return self

    def put(self, x, y, s):
        for i, ch in enumerate(s):
            if 0 <= x + i < self.w and 0 <= y < self.h:
                self.g[y][x + i] = ch
        return self

    def dots(self, pts, ch):
        for x, y in pts:
            self.g[y][x] = ch
        return self

    def rows(self):
        return ["".join(r) for r in self.g]


def hollowmere():
    return [
        "TTTTTTTTTTT==TTTTTTTTTTT",
        "TT.......f.==.........TT",
        "T..RRRRR...==..RRRRRR..T",
        "T..RRRRR..L==L.RRRRRR..T",
        "T..VWDWV...==..VWVDWV..T",
        "T....=.....==.....=....T",
        "T.f..=ccccccccccccc..f.T",
        "T....=c...........c....T",
        "Tz...=c..~~...~~..c....T",
        "T....=c..~~.+.~~..c...fT",
        "T....=c...........c....T",
        "T..L.=ccccccccccccc..L.T",
        "T....=.....cc..........T",
        "T.f..=.RRRRRRRRRR......T",
        "T....=.RRRRRRRRRR......T",
        "T....=.VWVWDDWVWV....ffT",
        "TFFFF=.....cc.......FFFT",
        "T....=.....cc..........T",
        "TTT..=====ccc..f....TTTT",
        "TTTTTTTTTTTTTTTTTTTTTTTT",
    ]


def home():
    return [
        "mmmmmmmmmm",
        "mkkwwwwwZm",
        "mwwwwwwwwm",
        "mwwrrrrwBm",
        "mwwrrrrwwm",
        "mwwwwwwwwm",
        "mewwwwwwem",
        "mmmm^^mmmm",
    ]


def sanctuary():
    return [
        "mmmmmmmmmm",
        "mkwwwwwwkm",
        "mCCCCCCCCm",
        "mwwwwwwwwm",
        "mhwwwwwwhm",
        "mwwrrrrwwm",
        "mwwrrrrwwm",
        "mmmm^^mmmm",
    ]


def guild():
    return [
        "mmmmmmmmmmmm",
        "mkkkwwwwkkkm",
        "mwwwwwwwwwwm",
        "mwwBwwwwwBwm",
        "mwwwwrrwwwwm",
        "mwwwwrrwwwwm",
        "mPwwwrrwwwPm",
        "mwwwwrrwwwwm",
        "mmmmm^^mmmmm",
    ]


def woods():
    g = Grid(24, 30, '"').border('T')
    # winding path south (11-12) -> north (11-12)
    g.rect(11, 20, 12, 29, '=')
    g.rect(11, 19, 16, 20, '=')
    g.rect(15, 11, 16, 20, '=')
    g.rect(11, 10, 16, 11, '=')
    g.rect(11, 0, 12, 10, '=')
    # lake with tall grass shore (Lochling)
    g.rect(2, 4, 7, 8, '~')
    g.rect(2, 9, 8, 11, ',')
    g.rect(8, 4, 9, 8, ',')
    # tall grass fields
    g.rect(2, 22, 9, 27, ',')
    g.rect(14, 23, 21, 27, ',')
    g.rect(2, 14, 9, 18, ',')
    g.rect(13, 2, 17, 7, ',')
    # tree clusters
    g.rect(13, 13, 14, 17, 'T')
    g.rect(9, 12, 10, 13, 'T')
    g.rect(19, 2, 22, 5, 'T')
    g.rect(1, 19, 4, 20, 'T')
    g.rect(18, 20, 22, 21, 'T')
    g.dots([(6, 20), (8, 21), (17, 22), (20, 17), (2, 12), (10, 2)], 'T')
    # fog wall hiding the secret glade (needs Lantern)
    g.rect(18, 12, 18, 18, 'q')
    g.rect(19, 11, 22, 11, 'T')
    g.rect(19, 19, 22, 19, 'T')
    g.rect(19, 12, 22, 18, ',')
    g.dots([(21, 13)], 'M')
    g.rect(17, 14, 17, 16, '"')
    # flowered clearing where the wounded wolf rests
    g.rect(2, 1, 6, 2, 'f')
    g.dots([(3, 2), (4, 1)], '"')
    # investigation clues
    g.dots([(5, 25), (20, 24), (6, 16)], '*')
    g.dots([(13, 27)], 'z')
    return g.rows()


def grave():
    g = Grid(24, 20, '"').border('i')
    # manor facade on the hill
    g.rect(6, 0, 17, 0, 'R')
    g.rect(6, 1, 17, 1, 'W')
    g.rect(6, 2, 17, 2, 'V')
    g.dots([(8, 1), (15, 1)], 'V')
    g.dots([(11, 2), (12, 2)], 'D')
    g.rect(0, 0, 5, 2, 'i')
    g.rect(18, 0, 23, 2, 'i')
    # paths
    g.rect(11, 3, 12, 19, '=')
    g.rect(13, 9, 23, 10, '=')
    # grave rows with diggings between
    for y in (5, 8, 12, 15):
        for x in (2, 4, 6, 8):
            g.dots([(x, y)], '+')
        for x in (15, 17, 19, 21):
            if y not in (8,):
                g.dots([(x, y)], '+')
    g.rect(2, 6, 9, 6, 'x')
    g.rect(2, 13, 9, 14, 'x')
    g.rect(15, 13, 21, 14, 'x')
    g.rect(15, 5, 21, 6, 'x')
    g.rect(2, 16, 9, 17, 'x')
    g.dots([(1, 10), (9, 10), (20, 17), (14, 4), (2, 3), (21, 3)], 't')
    g.dots([(13, 17)], 'z')
    return g.rows()


def neon():
    g = Grid(30, 20, 'N')
    # top buildings with neon
    g.rect(0, 0, 29, 5, 'b')
    g.rect(13, 0, 16, 7, 'N')
    g.rect(14, 0, 14, 7, 'a')
    g.rect(15, 0, 15, 7, 'A')
    for x, ch in [(1, 'n'), (2, 'n'), (5, 'j'), (6, 'j'), (9, 'J'), (10, 'J'), (18, 'n'), (19, 'n'), (22, 'J'), (23, 'J'), (26, 'j'), (27, 'j')]:
        g.dots([(x, 5)], ch)
    # main road
    g.rect(0, 8, 29, 11, 'a')
    g.rect(14, 8, 14, 11, 'a')
    # bottom buildings + arena
    g.rect(0, 14, 29, 19, 'b')
    g.rect(18, 13, 27, 17, 'R')
    g.rect(18, 17, 27, 17, 'V')
    g.dots([(22, 17), (23, 17)], 'D')
    g.dots([(19, 17), (26, 17)], 'W')
    for x, ch in [(3, 'j'), (4, 'j'), (9, 'n'), (10, 'n'), (12, 'J')]:
        g.dots([(x, 14)], ch)
    g.rect(18, 18, 27, 19, 'N')
    g.rect(17, 18, 17, 19, 'b'); g.rect(28, 18, 29, 19, 'b')
    # alleys (encounters)
    g.rect(1, 15, 6, 18, 'u')
    g.rect(0, 15, 0, 19, 'b'); g.rect(7, 15, 7, 19, 'b'); g.rect(1, 19, 6, 19, 'b')
    g.rect(25, 1, 28, 4, 'u')
    g.rect(24, 1, 24, 4, 'b'); g.rect(29, 0, 29, 5, 'b')
    g.dots([(26, 5), (27, 5)], 'N')
    # lamps
    g.dots([(3, 7), (9, 7), (20, 7), (26, 7), (3, 12), (11, 12), (16, 12), (28, 12)], 'L')
    g.dots([(11, 13)], 'z')
    return g.rows()


def arena():
    g = Grid(12, 12, 's').border('S')
    g.rect(5, 2, 6, 10, 'r')
    g.dots([(1, 2), (10, 2), (1, 6), (10, 6)], 'P')
    g.dots([(5, 11), (6, 11)], '^')
    g.dots([(3, 1), (8, 1)], 'Y')
    return g.rows()


def manor():
    g = Grid(20, 16, 'w').border('m')
    # study behind iron gate
    g.rect(1, 1, 18, 3, 'm')
    g.rect(7, 1, 12, 2, 'w')
    g.dots([(7, 1), (12, 1)], 'k')
    g.dots([(9, 1), (10, 1)], 'B')
    g.dots([(9, 3), (10, 3)], 'G')
    # grand hall
    g.rect(8, 4, 11, 14, 'r')
    g.rect(3, 9, 16, 9, 'r')
    g.dots([(9, 5)], 'o'); g.dots([(15, 8)], 'o'); g.dots([(9, 12)], 'o'); g.dots([(3, 8)], 'o')
    # dusty corners (encounters)
    g.rect(1, 4, 4, 6, 'u')
    g.rect(15, 4, 18, 6, 'u')
    g.rect(1, 11, 5, 14, 'u')
    g.rect(14, 11, 18, 14, 'u')
    g.dots([(6, 4), (13, 4), (6, 14), (13, 14)], 'P')
    g.dots([(9, 15), (10, 15)], '^')
    g.dots([(17, 9)], 'k')
    return g.rows()


def castle():
    g = Grid(20, 20, 's').border('S')
    g.rect(1, 1, 18, 1, 'S')
    g.dots([(9, 2), (10, 2)], 'K')
    g.rect(8, 3, 11, 18, 'r')
    for y in (5, 9, 13, 16):
        g.dots([(6, y), (13, y)], 'P')
    g.dots([(2, 3), (17, 3)], 'Y')
    g.rect(1, 6, 4, 12, 'u')
    g.rect(15, 6, 18, 12, 'u')
    g.rect(1, 15, 4, 18, 'u')
    g.rect(15, 15, 18, 18, 'u')
    g.dots([(9, 19), (10, 19)], '^')
    g.dots([(14, 2)], 's')
    return g.rows()


def rift():
    g = Grid(20, 20, 'v')
    g.rect(9, 3, 10, 18, 'p')
    g.rect(3, 13, 16, 15, 'p')
    g.rect(3, 6, 16, 8, 'p')
    g.rect(2, 6, 5, 15, 'g')
    g.rect(14, 6, 17, 15, 'g')
    g.rect(7, 1, 12, 4, 'p')
    g.rect(6, 10, 13, 11, 'g')
    g.dots([(9, 19)], '@')
    return g.rows()


# id, title, rows, bgm, battle backdrop (0 town 1 woods 2 grave 3 city 4 manor 5 castle 6 rift)
MAPS = [
    {"id": "hollowmere", "title": "HOLLOWMERE", "rows": hollowmere(), "bgm": 0, "area": 0},
    {"id": "home", "title": "YOUR HOME", "rows": home(), "bgm": 0, "area": 0},
    {"id": "sanctuary", "title": "SANCTUARY", "rows": sanctuary(), "bgm": 0, "area": 0},
    {"id": "guild", "title": "HUNTER GUILD", "rows": guild(), "bgm": 0, "area": 0},
    {"id": "woods", "title": "WHISPER WOODS", "rows": woods(), "bgm": 1, "area": 1},
    {"id": "grave", "title": "GRAVE HILL", "rows": grave(), "bgm": 1, "area": 2},
    {"id": "neon", "title": "NEON ROW", "rows": neon(), "bgm": 1, "area": 3},
    {"id": "arena", "title": "TRIAL ARENA", "rows": arena(), "bgm": 2, "area": 5},
    {"id": "manor", "title": "RAVENCREST MANOR", "rows": manor(), "bgm": 2, "area": 4},
    {"id": "castle", "title": "DUSKHOLD CASTLE", "rows": castle(), "bgm": 2, "area": 5},
    {"id": "rift", "title": "THE RIFT", "rows": rift(), "bgm": 3, "area": 6},
]
