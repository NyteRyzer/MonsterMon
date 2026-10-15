"""Battle effect sprites, emotes and the overworld relic (16x16, one shared OBJ palette)."""

PAL = ["#000000", "#140c18", "#ffffff", "#ffe060", "#ff9030", "#e8283c", "#7a1020", "#5a2a90",
       "#c890ff", "#2a6a2a", "#6ad050", "#8a5a2a", "#e8dcc0", "#60f0ff", "#fff4b0", "#8a8a9a"]

K = {'.': 0, 'o': 1, 'w': 2, 'y': 3, 'O': 4, 'r': 5, 'R': 6, 'p': 7, 'P': 8, 'g': 9, 'G': 10, 'b': 11,
     'B': 12, 'c': 13, 'l': 14, 's': 15}

ART = {
    "slash": [
        "..........w.....", ".........ww....w", "........ww....ww", ".......ww....ww.",
        "......ww....ww..", ".....ww....ww...", "....ww....ww....", "...ww....ww....w",
        "..ww....ww....ww", ".ww....ww....ww.", "ww....ww....ww..", "w....ww....ww...",
        "....ww....ww....", "...ww....ww.....", "..ww....w.......", ".w..............",
    ],
    "blood": [
        "................", "......r.........", ".....rrr........", ".....rrr....r...",
        "....rrwrr..rrr..", "....rrrrr..rrr..", "....RrrrR.rrwrr.", ".....RRR..rrrrr.",
        "..r.......RrrrR.", ".rrr.......RRR..", ".rwr.....r......", ".rrr....rrr.....",
        "..R.....rwr.....", "........rrr.....", ".........R......", "................",
    ],
    "shadow": [
        "................", ".....pppppp.....", "...ppPPPPPPpp...", "..pPPoooooPPp...",
        ".pPPooooooooPp..", ".pPoooooooooPp..", "pPoooPPoooooooPp", "pPooPwPoooooooPp",
        "pPoooPPoooooooPp", "pPoooooooooooPp.", ".pPoooooooooPp..", ".pPPooooooooPp..",
        "..pPPoooooPPp...", "...ppPPPPPPpp...", ".....pppppp.....", "................",
    ],
    "flame": [
        ".......r........", "......rr........", "......rOr....r..", ".....rOOr...rr..",
        "..r..rOyOr..rOr.", "..rr.rOyyOrrOOr.", ".rOOrrOyyyOrOyOr", ".rOyOrOywyyOOyOr",
        "rOyyyOyywwyyOyOr", "rOyywyywwwyyyyOr", "rOyywwwwwwwyyyOr", ".rOyywwwwwwyyOr.",
        ".rOOyywwwwyyyOr.", "..rrOOyyyyyOOr..", "....rrOOOOOrr...", "......rrrrr.....",
    ],
    "nature": [
        "................", "...........GG...", "..........GGGG..", ".........GGgGG..",
        "........GGgGG...", "........GgGG....", ".......GgG......", "......gg........",
        "...bb...........", "..bbBb.......bb..", ".bbbbbb.....bBbb.", ".bbbbbb.....bbbb.",
        "..bbbb.......bb.", "......bb........", ".....bBbb.......", "......bb........",
    ],
    "star": [
        ".......P........", ".......P........", "......PcP.......", "......PcP.......",
        "PPPPPPcwcPPPPPP.", ".PPccccwccccPP..", "...PPcwwwcPP....", "....PcwwwcP.....",
        "....PcwcwcP.....", "...PcwP.PwcP....", "...PcP...PcP....", "..PcP.....PcP...",
        "..PP.......PP...", "................", "..............c.", ".............cwc",
    ],
    "bone": [
        "................", ".BB.............", "BBBB............", "BBBBB...........",
        ".BBBBB..........", "..BBBBB.........", "...BBBBB........", "....BBBBB.......",
        ".....BBBBB......", "......BBBBB.....", ".......BBBBB....", "........BBBBB...",
        ".........BBBBB..", "..........BBBBBB", "...........BBBB.", "............BB..",
    ],
    "light": [
        ".......w........", ".......l........", "..w....l....w...", "...l...l...l....",
        "....l..y..l.....", ".....lyyyl......", "......ywy.......", "wlllyyywwyyylllw",
        "......ywy.......", ".....lyyyl......", "....l..y..l.....", "...l...l...l....",
        "..w....l....w...", ".......l........", ".......w........", "................",
    ],
    "heal": [
        "................", "......GGG.......", "......GwG.......", "......GwG.......",
        "...GGGGwGGGG....", "...GwwwwwwwG....", "...GGGGwGGGG....", "......GwG.......",
        "......GwG.......", "......GGG...G...", "...........GwG..", "............G...",
        "..G.............", ".GwG............", "..G.............", "................",
    ],
    "emote": [
        "................", "...oooooooooo...", "..owwwwwwwwwwo..", "..owwwwrrwwwwo..",
        "..owwwwrrwwwwo..", "..owwwwrrwwwwo..", "..owwwwrrwwwwo..", "..owwwwrrwwwwo..",
        "..owwwwwwwwwwo..", "..owwwwrrwwwwo..", "..owwwwrrwwwwo..", "..owwwwwwwwwwo..",
        "...ooooowoooo...", "........wo......", ".........o......", "................",
    ],
    "relic": [
        "................", "................", "......llll......", ".....lyyyyl.....",
        "....lyywwyyl....", "...lyywwwwyyl...", "...lyyywwyyyl...", "...lOyyyyyyOl...",
        "...lOOyyyyOOl...", "....lOOOOOOl....", ".....llOOll.....", "......llll......",
        "................", "...l........l...", "................", "................",
    ],
}
ORDER = ["slash", "blood", "shadow", "flame", "nature", "star", "bone", "light", "heal", "emote", "relic"]


def grids():
    out = []
    for name in ORDER:
        rows = ART[name]
        rows = [(r + "." * 16)[:16] for r in rows][:16]
        assert len(rows) == 16, name
        out.append([[K[c] for c in r] for r in rows])
    return out
