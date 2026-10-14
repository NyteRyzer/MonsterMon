"""Procedural pixel art for every Monstermon (32x32, 16-colour palettes).

Palette roles: 0 transparent, 1 outline, 2-4 body ramp (dark..light),
5-7 accent ramp, 8 white, 9 eye glow, 10 mouth/pupil dark, 11-13 extra
ramp, 14 glow, 15 highlight.
"""
import colorsys
import math

N = 32


class C:
    def __init__(self):
        self.g = [[0] * N for _ in range(N)]

    def px(self, x, y, c):
        x, y = int(round(x)), int(round(y))
        if 0 <= x < N and 0 <= y < N:
            self.g[y][x] = c

    def ell(self, cx, cy, rx, ry, c):
        for y in range(N):
            for x in range(N):
                if ((x + .5 - cx) / rx) ** 2 + ((y + .5 - cy) / ry) ** 2 <= 1:
                    self.g[y][x] = c

    def sell(self, cx, cy, rx, ry, ramp):
        d, m, l = ramp
        for y in range(N):
            for x in range(N):
                dx, dy = (x + .5 - cx) / rx, (y + .5 - cy) / ry
                if dx * dx + dy * dy <= 1:
                    s = -0.55 * dx - 0.85 * dy
                    self.g[y][x] = l if s > 0.45 else (d if s < -0.35 else m)

    def poly(self, pts, c):
        n = len(pts)
        for y in range(N):
            for x in range(N):
                px, py = x + .5, y + .5
                inside = False
                j = n - 1
                for i in range(n):
                    xi, yi = pts[i]
                    xj, yj = pts[j]
                    if (yi > py) != (yj > py) and px < (xj - xi) * (py - yi) / (yj - yi + 1e-9) + xi:
                        inside = not inside
                    j = i
                if inside:
                    self.g[y][x] = c

    def line(self, x0, y0, x1, y1, c, w=1):
        steps = int(max(abs(x1 - x0), abs(y1 - y0)) * 2) + 1
        for i in range(steps + 1):
            t = i / steps
            x, y = x0 + (x1 - x0) * t, y0 + (y1 - y0) * t
            if w == 1:
                self.px(x, y, c)
            else:
                for dy in range(-(w // 2), w - w // 2):
                    for dx in range(-(w // 2), w - w // 2):
                        self.px(x + dx, y + dy, c)

    def rect(self, x0, y0, x1, y1, c):
        for y in range(max(0, y0), min(N, y1)):
            for x in range(max(0, x0), min(N, x1)):
                self.g[y][x] = c

    def mirror(self):
        for y in range(N):
            for x in range(16):
                self.g[y][31 - x] = self.g[y][x]

    def outline(self, c=1):
        g = self.g
        add = []
        for y in range(N):
            for x in range(N):
                if g[y][x] == 0:
                    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                        a, b = x + dx, y + dy
                        if 0 <= a < N and 0 <= b < N and g[b][a] not in (0, c):
                            add.append((x, y))
                            break
        for x, y in add:
            g[y][x] = c

    def recolor(self, src, dst, region=None):
        for y in range(N):
            for x in range(N):
                if self.g[y][x] == src and (region is None or region(x, y)):
                    self.g[y][x] = dst


def eyes(c, x, y, w=1.5, h=2.0, iris=9, pupil=10, sym=True, shine=True):
    c.ell(x, y, w, h, 8)
    c.ell(x + 0.3, y + 0.4, max(0.8, w * 0.7), max(0.9, h * 0.7), iris)
    c.px(x + 0.3, y + 0.6, pupil)
    if shine:
        c.px(x - 0.6, y - 0.8, 8)
    if sym:
        xr = 31 - x
        c.ell(xr, y, w, h, 8)
        c.ell(xr - 0.3, y + 0.4, max(0.8, w * 0.7), max(0.9, h * 0.7), iris)
        c.px(xr - 0.3, y + 0.6, pupil)
        if shine:
            c.px(xr - 0.6, y - 0.8, 8)


def glow_eyes(c, x, y, col=9, sym=True, size=1):
    for dx in range(size):
        c.px(x + dx, y, col)
        if sym:
            c.px(31 - x - dx, y, col)


B = (2, 3, 4)
A = (5, 6, 7)
X = (11, 12, 13)

# ------------------------------------------------------------------ designs


def fangling():
    c = C()
    c.poly([(8, 17), (0, 10), (1, 17), (0, 21), (3, 20), (4, 24), (8, 22)], 5)
    c.line(8, 17, 1, 11, 6); c.line(7, 19, 1, 20, 6)
    c.poly([(10, 14), (6, 2), (14, 10)], 3)
    c.poly([(10, 12), (7, 5), (13, 10)], 6)
    c.sell(15.5, 19, 9, 8, B)
    c.ell(12, 27, 2.2, 1.5, 2)
    c.mirror()
    c.outline()
    eyes(c, 12, 18, 2, 2.5, iris=9)
    c.rect(14, 22, 18, 23, 10)
    c.px(14, 23, 8); c.px(17, 23, 8)
    c.px(9, 21, 6); c.px(22, 21, 6)
    return c


def countess():
    c = C()
    c.poly([(15.5, 7), (2, 31), (29, 31)], 5)
    c.poly([(15.5, 10), (6, 31), (25, 31)], 6)
    c.poly([(12, 13), (19, 13), (23, 31), (8, 31)], 2)
    c.poly([(13, 14), (18, 14), (16, 30)], 3)
    c.poly([(8, 7), (12, 13), (6, 15)], 7)
    c.poly([(23, 7), (19, 13), (25, 15)], 7)
    c.sell(15.5, 9, 4.2, 4.8, X)
    c.ell(15.5, 5.5, 5, 3, 2)
    c.poly([(11, 5), (15.5, 9), (20, 5)], 2)
    c.rect(10, 5, 12, 13, 2); c.rect(19, 5, 21, 13, 2)
    c.outline()
    glow_eyes(c, 13, 10, 9, size=2)
    c.px(15, 12, 10); c.px(16, 12, 10)
    c.px(15, 13, 8)
    c.ell(15.5, 18, 1, 1, 14)
    return c


def dawnfang():
    c = C()
    for i, (dy, r) in enumerate([(12, 6), (16, 6), (20, 5), (24, 4)]):
        c.ell(6 - i * 0.5, dy, r, 2.6, 12 if i % 2 else 13)
    c.poly([(10, 14), (8, 5), (14, 11)], 3)
    c.poly([(10, 13), (9, 7), (13, 11)], 6)
    c.sell(15.5, 19, 8.5, 8, B)
    c.ell(12, 27, 2, 1.4, 2)
    c.mirror()
    c.outline()
    c.ell(15.5, 3, 6, 1.6, 14)
    c.ell(15.5, 3, 4, 0.7, 0)
    eyes(c, 12, 18, 2, 2.3, iris=14)
    c.rect(14, 22, 18, 23, 10)
    c.px(14, 23, 8); c.px(17, 23, 8)
    c.ell(15.5, 14, 1.2, 1.2, 15)
    return c


def pupwolf():
    c = C()
    c.sell(15.5, 25, 7, 5.5, B)
    c.ell(11, 29.5, 2.4, 1.6, 4)
    c.poly([(9, 12), (7, 2), (14, 8)], 3)
    c.poly([(9.5, 10), (8, 5), (12.5, 8)], 6)
    c.sell(15.5, 15, 8.5, 7, B)
    c.ell(15.5, 19.5, 4.5, 3, 11)
    c.mirror()
    c.poly([(23, 26), (31, 18), (30, 24), (25, 28)], 3)
    c.outline()
    eyes(c, 11.5, 14, 1.6, 2.1, iris=10, pupil=10)
    c.rect(14, 17, 18, 19, 10)
    c.px(15, 21, 10); c.px(16, 21, 10)
    c.poly([(14.5, 8), (17, 8), (15.8, 11)], 14)
    return c


def direwolf():
    c = C()
    c.sell(15.5, 27, 13, 6, B)
    for i in range(7):
        a = math.pi * (0.9 + i * 0.2)
        c.poly([(15.5 + math.cos(a) * 6, 15 + math.sin(a) * 4), (15.5 + math.cos(a) * 13, 16 + math.sin(a) * 10),
                (15.5 + math.cos(a + 0.25) * 7, 15 + math.sin(a + 0.25) * 5)], 2)
    c.poly([(8, 10), (6, 0), (13, 6)], 3)
    c.poly([(8.5, 8), (7.5, 3), (11.5, 6)], 6)
    c.sell(15.5, 14, 8, 6.5, B)
    c.ell(15.5, 20, 4.5, 4, 11)
    c.mirror()
    c.outline()
    c.poly([(10, 12), (14, 13), (10, 14)], 9)
    c.poly([(21, 12), (17, 13), (21, 14)], 9)
    c.rect(14, 17, 18, 19, 10)
    c.rect(13, 22, 19, 23, 10)
    for x in (13, 15, 16, 18):
        c.px(x, 22, 8)
    return c


def moonhowl():
    c = direwolf()
    c.ell(26, 4, 4, 4, 14)
    c.ell(24.5, 3, 3.4, 3.4, 0)
    c.poly([(14.5, 7), (17, 7), (15.8, 10)], 14)
    return c


def wispette():
    c = C()
    c.poly([(7.5, 15), (8.5, 27), (12, 24), (15.5, 29), (19, 24), (22.5, 27), (23.5, 15)], 12)
    c.sell(15.5, 15, 8.2, 8.2, X)
    c.ell(6.5, 19, 2, 1.5, 12); c.ell(24.5, 19, 2, 1.5, 12)
    c.poly([(15.5, 0), (12, 7), (19, 7)], 14)
    c.outline()
    c.ell(12, 14, 1.6, 2.6, 10); c.ell(19, 14, 1.6, 2.6, 10)
    c.px(11, 13, 8); c.px(18, 13, 8)
    c.px(9, 17, 6); c.px(22, 17, 6)
    c.rect(15, 18, 17, 19, 10)
    c.ell(15.5, 4, 1.2, 1.4, 15)
    return c


def lanternis():
    c = C()
    c.poly([(6, 14), (7, 28), (11, 25), (15.5, 30), (20, 25), (24, 28), (25, 14)], 12)
    c.sell(15.5, 13, 9.5, 9, X)
    c.poly([(9, 6), (15.5, 0), (22, 6)], 5)
    c.ell(15.5, 6, 9, 1.5, 5)
    c.outline()
    c.ell(12, 13, 1.5, 2.4, 10); c.ell(19, 13, 1.5, 2.4, 10)
    c.px(11, 12, 8); c.px(18, 12, 8)
    c.rect(24, 18, 31, 28, 1)
    c.rect(25, 19, 30, 27, 14)
    c.rect(26, 20, 29, 26, 15)
    c.line(27.5, 18, 22, 15, 1)
    c.rect(15, 17, 17, 18, 10)
    return c


def banshriek():
    c = C()
    c.poly([(5, 8), (2, 30), (8, 26), (11, 31), (15.5, 27), (20, 31), (23, 26), (29, 30), (26, 8)], 3)
    c.poly([(9, 14), (7, 29), (12, 25), (15.5, 30), (19, 25), (24, 29), (22, 14)], 4)
    for x in (4, 7, 10):
        c.line(x + 3, 3, x - 1, 24, 2, 2)
    c.sell(15.5, 10, 5, 6.5, X)
    c.ell(15.5, 3.5, 7, 3, 2)
    c.mirror()
    c.outline()
    c.ell(13, 9, 1.4, 1.8, 10); c.ell(18, 9, 1.4, 1.8, 10)
    c.px(13, 9, 9); c.px(18, 9, 9)
    c.ell(15.5, 14, 1.6, 2.6, 10)
    c.px(15, 13, 6)
    return c


def batling():
    c = C()
    c.poly([(11, 15), (0, 7), (2, 15), (0, 22), (5, 20), (8, 24), (12, 20)], 5)
    c.line(11, 15, 1, 8, 6); c.line(10, 18, 1, 21, 6)
    c.poly([(12, 13), (10, 5), (15, 11)], 3)
    c.sell(15.5, 17, 5.5, 6, B)
    c.mirror()
    c.outline()
    eyes(c, 13.5, 16, 1.2, 1.5, iris=9)
    c.px(14, 20, 8); c.px(17, 20, 8)
    return c


def chirovamp():
    c = C()
    c.poly([(10, 12), (0, 2), (0, 12), (1, 22), (4, 19), (6, 26), (9, 21), (11, 25)], 5)
    for (x, y) in [(0, 12), (1, 22), (6, 26)]:
        c.line(10, 13, x + 1, y - 1, 6)
    c.poly([(12, 10), (9, 0), (15, 8)], 3)
    c.poly([(12, 9), (10, 3), (14, 8)], 6)
    c.sell(15.5, 16, 7, 8.5, B)
    c.ell(12, 28, 2, 1.5, 2)
    c.mirror()
    c.outline()
    c.poly([(11, 13), (14.5, 14.5), (11, 15.5)], 9)
    c.poly([(20, 13), (16.5, 14.5), (20, 15.5)], 9)
    c.ell(15.5, 20, 3, 2, 10)
    for x in (13, 15, 16, 18):
        c.px(x, 19, 8)
    c.px(14, 21, 8); c.px(17, 21, 8)
    return c


def ghoulet():
    c = C()
    c.sell(15.5, 24, 6.5, 6, A)
    c.poly([(9, 28), (11, 31), (13, 28), (15, 31), (17, 28), (19, 31), (22, 28)], 5)
    c.ell(7.5, 21, 2.2, 3, 3)
    c.sell(15.5, 12, 7.5, 7, B)
    c.mirror()
    c.poly([(12, 4), (14, 1), (15, 5)], 2)
    c.outline()
    c.ell(12, 11, 2.2, 2.4, 8); c.px(12, 11, 10)
    c.ell(19.5, 11.5, 1.4, 1.4, 8); c.px(19, 11, 10)
    c.line(12, 16, 19, 16, 10)
    for x in (13, 15, 17):
        c.line(x, 15, x, 17, 10)
    c.line(22, 5, 20, 8, 1)
    return c


def ghoulord():
    c = C()
    for x in (6, 25):
        c.poly([(x - 4, 12), (x - 2, 2), (x, 8), (x + 1, 0), (x + 3, 10), (x + 4, 12)], 14)
        c.poly([(x - 2, 12), (x, 5), (x + 2, 12)], 15)
    c.sell(15.5, 21, 12, 10, B)
    c.sell(15.5, 9, 5, 5, B)
    c.mirror()
    c.outline()
    c.line(4, 15, 27, 27, 8); c.line(4, 16, 27, 28, 13)
    glow_eyes(c, 13, 9, 14, size=2)
    c.rect(13, 12, 19, 13, 10)
    c.rect(9, 20, 12, 21, 10); c.rect(19, 23, 23, 24, 10)
    return c


def mothkin():
    c = C()
    c.poly([(13, 14), (1, 4), (0, 13), (8, 17)], 5)
    c.poly([(13, 18), (2, 20), (4, 28), (12, 23)], 6)
    c.ell(6, 10, 2.2, 2.2, 7); c.px(6, 10, 9)
    c.ell(6.5, 23, 1.6, 1.6, 7)
    c.sell(15.5, 21, 5, 7.5, B)
    c.ell(15.5, 15, 6.5, 3, 13)
    c.line(13, 8, 9, 1, 2); c.line(9, 1, 7, 2, 2)
    c.mirror()
    c.sell(15.5, 11, 5, 4.5, (12, 13, 8))
    c.outline()
    c.ell(13, 11, 1.8, 1.8, 9); c.ell(18, 11, 1.8, 1.8, 9)
    c.px(12, 10, 8); c.px(17, 10, 8)
    for y in (21, 24, 27):
        c.line(12, y, 19, y, 2)
    return c


def mothman():
    c = C()
    c.poly([(12, 8), (0, 0), (1, 14), (0, 28), (6, 24), (11, 30), (13, 20)], 5)
    c.poly([(12, 9), (3, 4), (4, 14), (3, 24), (11, 22)], 6)
    c.poly([(11, 6), (20, 6), (19, 31), (12, 31)], 2)
    c.sell(15.5, 9, 6, 6, B)
    c.mirror()
    c.outline()
    c.ell(12.5, 8.5, 2.3, 1.7, 9); c.ell(18.5, 8.5, 2.3, 1.7, 9)
    c.px(12, 8, 14); c.px(19, 8, 14)
    c.line(7, 2, 10, 4, 2); c.line(24, 2, 21, 4, 2)
    return c


def impish():
    c = C()
    c.poly([(10, 18), (2, 12), (4, 20), (9, 22)], 5)
    c.sell(15.5, 23, 5, 5.5, B)
    c.ell(11, 29, 2, 1.5, 2)
    c.sell(15.5, 13, 7.5, 6.5, B)
    c.poly([(9, 9), (5, 1), (12, 7)], 12)
    c.poly([(8, 6), (5, 1), (9, 5)], 13)
    c.mirror()
    c.line(21, 25, 28, 21, 3, 2); c.poly([(27, 18), (31, 20), (28, 23)], 3)
    c.outline()
    eyes(c, 12, 12, 1.7, 1.9, iris=14)
    c.poly([(11, 16), (20, 16), (15.5, 19)], 10)
    c.px(13, 16, 8); c.px(18, 16, 8); c.px(15, 16, 8); c.px(16, 16, 8)
    return c


def hellion():
    c = C()
    c.poly([(9, 14), (0, 6), (2, 18), (0, 26), (7, 22), (10, 24)], 5)
    c.sell(15.5, 21, 9.5, 9, B)
    c.sell(15.5, 10, 6, 6, B)
    c.poly([(9, 7), (3, 6), (1, 0), (5, 3), (11, 4)], 12)
    c.poly([(10, 4), (12, -1), (15, 4)], 14)
    c.poly([(12, 4), (15.5, -2), (16, 4)], 15)
    c.mirror()
    c.outline()
    c.poly([(11, 9), (14.5, 10.5), (11, 11.5)], 14)
    c.poly([(20, 9), (16.5, 10.5), (20, 11.5)], 14)
    c.rect(12, 13, 20, 14, 10)
    for x in (12, 14, 17, 19):
        c.px(x, 13, 8)
    c.line(11, 20, 15, 25, 3); c.line(20, 20, 16, 25, 3)
    for x in (6, 8, 23, 25):
        c.px(x, 29, 8)
    return c


def hexcat():
    c = C()
    c.sell(15.5, 24, 6.5, 5.5, B)
    c.ell(11.5, 29, 2, 1.4, 3)
    c.sell(15.5, 15, 7.5, 6, B)
    c.poly([(8.5, 12), (7, 4), (12, 9)], 3)
    c.poly([(15.5, 0), (10, 8), (21, 8)], 5)
    c.ell(15.5, 8.5, 10, 1.6, 6)
    c.mirror()
    c.poly([(22, 26), (28, 25), (30, 18), (28, 15), (27, 22), (22, 23)], 3)
    c.outline()
    c.rect(11, 6, 21, 7, 14)
    c.ell(12, 15, 1.8, 1.8, 9); c.line(12, 14, 12, 16, 10)
    c.ell(19, 15, 1.8, 1.8, 9); c.line(19, 14, 19, 16, 10)
    c.px(15, 18, 6); c.px(16, 18, 6)
    c.line(5, 17, 9, 18, 13); c.line(26, 17, 22, 18, 13)
    c.px(17, 2, 14)
    return c


def grimalkin():
    c = C()
    c.poly([(21, 24), (30, 20), (31, 10), (29, 9), (28, 18), (21, 21)], 3)
    c.poly([(20, 26), (27, 29), (31, 26), (29, 25), (27, 27), (20, 24)], 3)
    c.sell(14.5, 22, 7, 8, B)
    c.sell(14.5, 11, 6.5, 6, B)
    c.poly([(9, 8), (8, 0), (13, 5)], 3)
    c.poly([(20, 8), (21, 0), (16, 5)], 3)
    c.outline()
    c.poly([(10, 10), (13, 11), (10, 12)], 9)
    c.poly([(19, 10), (16, 11), (19, 12)], 9)
    c.px(14, 14, 6); c.px(15, 14, 6)
    for (x, y) in [(2, 5), (4, 22), (26, 3), (1, 14)]:
        c.px(x, y, 14); c.px(x + 1, y, 15); c.px(x, y + 1, 15)
    c.ell(14.5, 19, 1.2, 1.2, 14)
    return c


def bonehop():
    c = C()
    c.ell(11, 6, 2.4, 7, 12); c.ell(11, 6, 1, 5, 11)
    c.sell(15.5, 24, 5, 5, X)
    c.ell(11, 29.5, 3, 1.5, 12)
    c.sell(15.5, 15, 7, 6, X)
    c.mirror()
    c.outline()
    c.ell(12, 14, 2, 2.2, 10); c.ell(19, 14, 2, 2.2, 10)
    c.px(12, 14, 9); c.px(19, 14, 9)
    c.poly([(14.5, 17), (16.5, 17), (15.5, 19)], 10)
    for y in (22, 24, 26):
        c.line(12, y, 19, y, 11)
    c.line(15.5, 21, 15.5, 27, 10)
    return c


def squatchy():
    c = C()
    for i in range(10):
        a = math.pi * (0.5 + i / 9.0)
        c.poly([(15.5 + math.cos(a) * 7, 18 + math.sin(a) * 9), (15.5 + math.cos(a) * 10.5, 18 + math.sin(a) * 11.5),
                (15.5 + math.cos(a + 0.18) * 7, 18 + math.sin(a + 0.18) * 9)], 2)
    c.sell(15.5, 19, 9, 10, B)
    c.ell(10, 29.5, 4, 2, 4)
    c.ell(15.5, 13, 5, 4.2, 12)
    c.ell(15.5, 22, 5, 5, 3)
    c.mirror()
    c.outline()
    c.ell(13, 12, 1.1, 1.3, 10); c.ell(18, 12, 1.1, 1.3, 10)
    c.px(13, 11, 8); c.px(18, 11, 8)
    c.rect(14, 15, 18, 16, 11)
    return c


def sasquatch():
    c = C()
    c.sell(15.5, 19, 11, 11, B)
    c.ell(5, 21, 3, 8, 3)
    c.ell(4, 29, 3, 2, 2)
    c.ell(11, 30, 4, 2, 4)
    c.sell(15.5, 7, 5.5, 5.5, B)
    c.ell(15.5, 8, 3.5, 3, 12)
    c.ell(15.5, 20, 6, 6, 3)
    c.mirror()
    c.outline()
    c.ell(14, 7, 0.9, 1, 10); c.ell(17, 7, 0.9, 1, 10)
    c.rect(14, 10, 18, 11, 11)
    c.line(13, 3, 15, 4, 2); c.line(18, 3, 16, 4, 2)
    return c


def gargoyl():
    c = C()
    c.poly([(11, 14), (0, 3), (2, 12), (0, 20), (5, 18), (9, 23)], 5)
    c.line(11, 14, 1, 4, 6)
    c.sell(15.5, 22, 8, 8, B)
    c.ell(10, 29, 3, 2, 2)
    c.sell(15.5, 11, 6, 5.5, B)
    c.poly([(11, 8), (8, 1), (13, 6)], 3)
    c.mirror()
    c.outline()
    c.rect(11, 10, 14, 12, 9); c.rect(18, 10, 21, 12, 9)
    c.px(11, 10, 15); c.px(18, 10, 15)
    c.rect(13, 14, 19, 15, 10)
    c.px(13, 15, 8); c.px(18, 15, 8)
    c.line(19, 19, 22, 24, 1); c.line(22, 24, 21, 27, 1)
    c.px(12, 24, 11); c.px(13, 25, 11); c.px(20, 18, 11)
    return c


def cerberpup(big=False):
    c = C()
    s = 1.15 if big else 1.0
    c.sell(15.5, 25, 9 * s, 6, B)
    c.ell(9, 30, 2.5, 1.5, 2); c.ell(22, 30, 2.5, 1.5, 2)
    for (hx, hy) in [(7, 15), (24, 15), (15.5, 11)]:
        if big:
            c.poly([(hx - 5, hy - 2), (hx - 3, hy - 9), (hx, hy - 4), (hx + 3, hy - 10), (hx + 5, hy - 2)], 14)
        c.poly([(hx - 4, hy - 1), (hx - 4, hy - 7), (hx - 1, hy - 4)], 3)
        c.poly([(hx + 4, hy - 1), (hx + 4, hy - 7), (hx + 1, hy - 4)], 3)
        c.sell(hx, hy, 5 * s, 4.5 * s, B)
        c.ell(hx, hy + 3, 2.5, 1.8, 11)
    c.outline()
    for (hx, hy) in [(7, 15), (24, 15), (15.5, 11)]:
        c.px(hx - 2, hy, 14); c.px(hx + 2, hy, 14)
        c.px(hx, hy + 2, 10); c.px(hx - 0.5, hy + 2, 10)
        if big:
            c.px(hx - 1, hy + 4, 8); c.px(hx + 1, hy + 4, 8)
    c.rect(9, 21, 23, 22, 14)
    return c


def cerberus():
    return cerberpup(True)


def glitchost():
    c = C()
    c.poly([(7.5, 14), (8, 28), (12, 25), (15.5, 29), (19, 25), (23, 28), (23.5, 14)], 12)
    c.sell(15.5, 14, 8.5, 8.5, X)
    c.mirror()
    c.outline()
    c.rect(10, 11, 14, 15, 10); c.rect(11, 12, 13, 14, 6)
    c.rect(17, 11, 21, 15, 10); c.rect(18, 12, 20, 14, 6)
    c.rect(13, 18, 19, 19, 10)
    g = c.g
    for y in (6, 9, 17, 22, 25):
        sh = 2 if y % 2 else -2
        row = g[y][:]
        for x in range(N):
            g[y][x] = row[(x - sh) % N]
    for y in range(4, 30, 3):
        for x in range(N):
            if g[y][x] in (11, 12, 13):
                g[y][x] = 5 if x % 2 else g[y][x]
    for (x, y) in [(3, 6), (27, 10), (4, 24), (28, 26)]:
        c.rect(x, y, x + 2, y + 1, 14)
    return c


def chupa():
    c = C()
    c.sell(17, 21, 10, 6, B)
    for i in range(6):
        x = 9 + i * 3
        c.poly([(x, 16), (x + 1.5, 9 + (i % 2) * 2), (x + 3, 16)], 6)
    c.rect(9, 25, 12, 30, 2); c.rect(22, 25, 25, 30, 2)
    c.sell(7, 17, 6, 5, B)
    c.poly([(3, 13), (1, 6), (6, 12)], 3)
    c.poly([(26, 20), (31, 25), (27, 23)], 3)
    c.outline()
    c.ell(5, 16, 1.6, 1.8, 10); c.px(5, 16, 9); c.px(4, 15, 8)
    c.ell(9, 16, 1.4, 1.6, 10); c.px(9, 16, 9)
    c.line(3, 20, 9, 20, 10)
    c.px(4, 21, 8); c.px(8, 21, 8)
    return c


def wrapling():
    c = C()
    c.sell(15.5, 24, 6, 6, X)
    c.ell(9, 22, 2, 3.5, 12)
    c.sell(15.5, 12, 7, 7, X)
    c.mirror()
    c.poly([(20, 22), (27, 30), (24, 31), (19, 25)], 12)
    c.outline()
    for y in range(7, 30, 3):
        for x in range(N):
            if c.g[y][x] in (12, 13):
                c.g[y][x] = 11
    c.ell(12.5, 12, 2.2, 1.6, 10); c.px(12, 12, 9); c.px(13, 12, 14)
    c.line(17, 12, 21, 11, 11)
    return c


def candlewick():
    c = C()
    c.poly([(15.5, -1), (9, 9), (11, 14), (20, 14), (22, 9)], 14)
    c.poly([(15.5, 3), (12, 10), (15.5, 13), (19, 10)], 15)
    c.rect(15, 12, 17, 15, 10)
    c.sell(15.5, 22, 6.5, 9, X)
    c.rect(9, 15, 22, 18, 13)
    c.ell(10, 19, 1.2, 3, 13); c.ell(21, 21, 1.2, 4, 13)
    c.ell(15.5, 30, 9, 2, 5)
    c.outline()
    c.line(12, 21, 14, 22, 10); c.line(19, 21, 17, 22, 10)
    c.px(15, 25, 10); c.px(16, 25, 10)
    c.px(13, 23, 6); c.px(18, 23, 6)
    return c


def halofly():
    c = C()
    c.ell(9, 11, 6, 4, 12); c.ell(9, 18, 5, 3, 13)
    c.sell(15.5, 22, 4.5, 6, (14, 15, 15))
    c.sell(15.5, 13, 4, 4, B)
    c.line(14, 9, 11, 3, 2)
    c.mirror()
    c.outline()
    c.ell(15.5, 2.5, 5, 1.4, 14); c.ell(15.5, 2.5, 3.5, 0.5, 0)
    c.ell(13.5, 13, 1, 1.3, 10); c.ell(17.5, 13, 1, 1.3, 10)
    c.px(13, 12, 8); c.px(17, 12, 8)
    c.px(11, 3, 15); c.px(20, 3, 15)
    return c


def seraphly():
    c = C()
    for (cy, rx, ry, col) in [(6, 8, 4, 12), (13, 9, 3.5, 13), (20, 7, 3, 12)]:
        c.ell(15.5 - rx - 0.5, cy, rx, ry, col)
        c.ell(15.5 - rx, cy, rx * 0.4, ry * 0.4, 9)
    c.sell(15.5, 22, 4.5, 8, (14, 15, 15))
    c.sell(15.5, 12, 4.5, 4.5, B)
    c.mirror()
    c.outline()
    c.ell(15.5, 3, 6, 1.6, 14); c.ell(15.5, 3, 4, 0.6, 0)
    glow_eyes(c, 13, 12, 9, size=2)
    return c


def lochling():
    c = C()
    c.ell(15.5, 29, 15.5, 3, 5)
    c.sell(22, 26, 5, 4, B)
    c.sell(28, 27, 3, 2.5, B)
    c.line(13, 27, 11, 12, 3, 4)
    c.line(13, 27, 12, 18, 2, 3)
    c.sell(13, 9, 5.5, 4.5, B)
    c.ell(8.5, 11, 3, 2, 4)
    c.poly([(14, 5), (17, 1), (17, 6)], 6)
    c.outline()
    for x in range(0, 32, 4):
        c.px(x, 28, 7); c.px(x + 1, 28, 7)
    c.ell(13, 8, 1.4, 1.7, 8); c.px(13, 8, 10); c.px(12, 7, 8)
    c.px(7, 11, 10)
    c.px(16, 8, 6)
    return c


def wendigo():
    c = C()
    c.poly([(11, 13), (20, 13), (21, 31), (10, 31)], 2)
    c.line(11, 14, 5, 28, 3, 2); c.line(20, 14, 26, 28, 3, 2)
    for (x0, y0) in [(5, 28), (26, 28)]:
        for d in (-2, 0, 2):
            c.line(x0, y0, x0 + d, y0 + 3, 1)
    c.line(13, 6, 7, 0, 5, 2); c.line(9, 3, 4, 3, 5); c.line(8, 2, 8, -1, 5)
    c.line(18, 6, 24, 0, 5, 2); c.line(22, 3, 27, 3, 5); c.line(23, 2, 23, -1, 5)
    c.sell(15.5, 9, 4, 5, X)
    c.ell(15.5, 13, 2.5, 2.2, 12)
    c.outline()
    c.ell(13.5, 8.5, 1.3, 1.6, 10); c.ell(17.5, 8.5, 1.3, 1.6, 10)
    c.px(13.5, 8.5, 9); c.px(17.5, 8.5, 9)
    for x in (14, 15, 16, 17):
        c.px(x, 13, 10 if x % 2 else 8)
    for y in (17, 20, 23):
        c.line(12, y, 19, y, 3)
    return c


def hollowking():
    c = C()
    c.ell(15.5, 18, 15, 13, 5)
    c.ell(15.5, 18, 12, 11, 6)
    c.poly([(15.5, 9), (2, 31), (29, 31)], 2)
    c.poly([(15.5, 12), (7, 31), (24, 31)], 3)
    c.poly([(12, 16), (19, 16), (17, 31), (14, 31)], 4)
    c.sell(15.5, 10, 5, 5.5, X)
    c.poly([(9.5, 6), (10, 0), (12.5, 3), (15.5, -1), (18.5, 3), (21, 0), (21.5, 6)], 14)
    c.outline()
    c.px(15, 2, 9); c.px(16, 2, 9)
    c.ell(13.5, 10, 1.5, 1.5, 10); c.ell(17.5, 10, 1.5, 1.5, 10)
    c.px(13.5, 10, 14); c.px(17.5, 10, 14)
    c.rect(13, 13, 19, 14, 10)
    for x in (13, 15, 17):
        c.px(x, 14, 11)
    for y in (19, 22, 25):
        c.line(13, y, 18, y, 11)
    return c


def eclipsa():
    c = C()
    c.ell(15.5, 12, 14, 11.5, 14)
    c.ell(15.5, 12, 12, 10, 1)
    c.poly([(9, 16), (0, 8), (1, 18), (0, 27), (6, 23), (9, 26)], 5)
    c.poly([(9, 17), (3, 13), (3, 21), (8, 22)], 6)
    c.sell(15.5, 22, 7, 8, B)
    c.sell(15.5, 12, 5, 5, B)
    c.poly([(12, 9), (8, 1), (14, 7)], 11)
    c.mirror()
    c.outline()
    c.ell(15.5, 22, 3, 5, 12)
    c.poly([(11, 11), (14.5, 12), (11, 13)], 9)
    c.poly([(20, 11), (16.5, 12), (20, 13)], 9)
    c.rect(14, 15, 18, 16, 10)
    c.ell(15.5, 8, 1, 1, 15)
    return c


def lycanth():
    c = C()
    c.poly([(8, 13), (23, 13), (21, 26), (10, 26)], 3)
    c.sell(15.5, 18, 8, 7, B)
    c.ell(5.5, 20, 2.5, 6.5, 3)
    c.poly([(3, 25), (2, 30), (4, 28), (5, 31), (6, 28), (8, 30), (7, 25)], 11)
    c.rect(10, 25, 15, 31, 5); c.rect(10, 28, 14, 29, 6)
    c.poly([(10, 7), (8, 0), (13, 4)], 3)
    c.sell(15.5, 8, 6, 5.5, B)
    c.ell(15.5, 12, 3, 2.6, 11)
    c.mirror()
    c.outline()
    c.poly([(11, 7), (14.5, 8), (11, 9)], 9)
    c.poly([(20, 7), (16.5, 8), (20, 9)], 9)
    c.rect(14, 10, 18, 11, 10)
    c.rect(14, 13, 18, 14, 10)
    c.px(14, 13, 8); c.px(17, 13, 8)
    c.line(13, 17, 15, 21, 2); c.line(18, 17, 16, 21, 2)
    return c


# name, draw fn, palette (index 1..15)
def pal(outline, body, acc, white="#f4f0e8", eye="#ff3048", dark="#200810", extra=("#9a98a8", "#c8c6d2", "#eeeef4"),
        glow="#ffd040", hi="#fff8d0"):
    return ["#000000", outline] + list(body) + list(acc) + [white, eye, dark] + list(extra) + [glow, hi]


SPECIES = []


def sp(name, fn, p):
    SPECIES.append({"name": name, "fn": fn, "pal": p})


sp("Fangling", fangling, pal("#1a0e20", ("#4a3a5c", "#6c5a84", "#9a88b0"), ("#5a1020", "#9a2038", "#d04a5a"), eye="#ff2040"))
sp("Countess", countess, pal("#12060e", ("#1e1020", "#34203a", "#4c3456"), ("#4a0818", "#8a1028", "#d02a40"), eye="#ff2040",
                            extra=("#c8b8c8", "#e8dce4", "#fff4f8")))
sp("Dawnfang", dawnfang, pal("#2a1a10", ("#a08a60", "#d0b880", "#f4e4b0"), ("#8a5020", "#c88a40", "#f0c070"), eye="#ffd040",
                            extra=("#c8c8d8", "#e8e8f4", "#ffffff")))
sp("Pupwolf", pupwolf, pal("#10141c", ("#3a4a62", "#5c7090", "#8ca4c4"), ("#5a3040", "#c06a80", "#f0a0b0"),
                          extra=("#c8d0dc", "#e4eaf0", "#f8f8ff"), glow="#e8e8ff"))
sp("Direwolf", direwolf, pal("#100c0a", ("#3a3028", "#5e4c3c", "#8a7458"), ("#5a2010", "#b04020", "#e07040"), eye="#ffd020",
                            extra=("#b8a890", "#d8ccb4", "#f4ecd8")))
sp("Moonhowl", moonhowl, pal("#06080e", ("#141c30", "#24304e", "#3a4c74"), ("#203050", "#4060a0", "#80a8e0"), eye="#40f0ff",
                            extra=("#8898b8", "#a8b8d4", "#d0dcf0"), glow="#e8f4ff"))
sp("Wispette", wispette, pal("#1c2a3a", ("#3a4a5a", "#5a6a7a", "#8a9aaa"), ("#704080", "#ff80b0", "#ffb0d0"),
                            extra=("#a8d0e8", "#d0ecfa", "#f4fcff"), glow="#80e8ff", hi="#e8ffff"))
sp("Lanternis", lanternis, pal("#1c2030", ("#3a4a5a", "#5a6a7a", "#8a9aaa"), ("#3a2a50", "#5a4080", "#8060b0"),
                              extra=("#b8d8e8", "#d8f0fa", "#f8ffff"), glow="#ffc040", hi="#fff4c0"))
sp("Banshriek", banshriek, pal("#0c0c14", ("#1a1a26", "#3a3a4e", "#6a6a80"), ("#3a3a4e", "#ff4060", "#ff90a0"), eye="#80ffff",
                              extra=("#a8b0c0", "#c8d0dc", "#eef2f8")))
sp("Batling", batling, pal("#140c18", ("#3a2a40", "#5a4462", "#806a8a"), ("#40182a", "#702a48", "#a04a6a"), eye="#ffd020"))
sp("Chirovamp", chirovamp, pal("#0e060c", ("#2a1424", "#46223a", "#6a3858"), ("#3a0814", "#6a1428", "#a02840"), eye="#ff3040"))
sp("Ghoulet", ghoulet, pal("#0e1408", ("#4a6a3a", "#6a8e50", "#94b870"), ("#3a2a1a", "#5a4430", "#7a6248")))
sp("Ghoulord", ghoulord, pal("#0a0e06", ("#2a3a20", "#405a30", "#628048"), ("#3a2a1a", "#5a4430", "#7a6248"),
                            extra=("#6a6a70", "#9a9aa0", "#c0c0c8"), glow="#ff8020", hi="#ffe040"))
sp("Mothkin", mothkin, pal("#1a1008", ("#6a4a2a", "#8a6a40", "#b08e60"), ("#5a4a3a", "#8a7458", "#e0d0a0"), eye="#ff4020",
                          extra=("#d8c8a8", "#e8dcc0", "#f8f0e0")))
sp("Mothman", mothman, pal("#06040a", ("#141018", "#221c2a", "#3a3044"), ("#2a2028", "#40343c", "#5a4c52"), eye="#ff1020",
                          glow="#ff8080"))
sp("Impish", impish, pal("#1a0606", ("#7a1818", "#b02a20", "#e05a3a"), ("#3a0a0a", "#601818", "#902a2a"),
                        extra=("#a89880", "#d8c8a8", "#f4ecd0"), glow="#ffd020"))
sp("Hellion", hellion, pal("#120404", ("#4a0e10", "#7a1a1a", "#b03028"), ("#2a0808", "#4a1010", "#7a2020"),
                          extra=("#6a6058", "#a89880", "#d8c8a8"), glow="#ff9020", hi="#fff060"))
sp("Hexcat", hexcat, pal("#06060c", ("#1a1a26", "#2c2c3e", "#46465e"), ("#2a1640", "#4a2a70", "#7448a8"), eye="#60ff60",
                        extra=("#808090", "#a0a0b0", "#c0c0d0"), glow="#ffd040"))
sp("Grimalkin", grimalkin, pal("#08040c", ("#1e1028", "#341c46", "#523070"), ("#2a1640", "#4a2a70", "#7448a8"), eye="#c060ff",
                              glow="#d090ff", hi="#ffffff"))
sp("Bonehop", bonehop, pal("#1a1418", ("#4a4048", "#6a6068", "#8a8088"), ("#4a4048", "#6a6068", "#8a8088"), eye="#ff60c0",
                          extra=("#c8c0b0", "#e8e0d0", "#fffcf0")))
sp("Squatchy", squatchy, pal("#140c06", ("#4a2e18", "#6e4626", "#946238"), ("#4a2e18", "#6e4626", "#946238"),
                            extra=("#8a6a4a", "#c09a74", "#e0c09a")))
sp("Sasquatch", sasquatch, pal("#100806", ("#3a2010", "#5a341c", "#80502c"), ("#3a2010", "#5a341c", "#80502c"),
                              extra=("#7a5a3a", "#b08a64", "#d4b08a")))
sp("Gargoyl", gargoyl, pal("#0e0e12", ("#4a4a54", "#6a6a76", "#92929e"), ("#3a3a44", "#5a5a66", "#7a7a86"), eye="#40e0ff",
                          extra=("#3a5a3a", "#5a7a4a", "#7a9a5a"), hi="#c0ffff"))
sp("Cerberpup", cerberpup, pal("#120606", ("#3a1412", "#5a201c", "#7e322a"), ("#3a1412", "#5a201c", "#7e322a"),
                              extra=("#8a5040", "#b07060", "#d09080"), glow="#ffa020"))
sp("Cerberus", cerberus, pal("#0e0404", ("#2a0c0a", "#461612", "#6a2418"), ("#2a0c0a", "#461612", "#6a2418"),
                            extra=("#7a3a2a", "#a05a40", "#c07a5a"), glow="#ff7010"))
sp("Glitchost", glitchost, pal("#0a0618", ("#2a1a50", "#4a2a80", "#7040c0"), ("#ff3fa4", "#22e0ff", "#a8f4ff"),
                              extra=("#60c0e0", "#90e0f8", "#d0f8ff"), glow="#ff3fa4"))
sp("Chupa", chupa, pal("#0a100a", ("#3a4a3a", "#566a50", "#7a906c"), ("#5a1018", "#a02030", "#d04050"), eye="#ff3020",
                      extra=("#8a9a80", "#a8b89c", "#c8d8bc")))
sp("Wrapling", wrapling, pal("#1a140c", ("#4a3a2a", "#6a5a44", "#8a7a5e"), ("#3a2a1a", "#5a4a36", "#7a6a50"), eye="#40ff80",
                            extra=("#a89878", "#d0c4a0", "#ece4c8"), glow="#a0ffc0"))
sp("Candlewick", candlewick, pal("#1a1220", ("#3a2a4a", "#5a4470", "#8064a0"), ("#3a2a60", "#5a40a0", "#8060d0"),
                                extra=("#c8b8a0", "#e4d8c0", "#fcf4e0"), glow="#7060ff", hi="#d0c8ff"))
sp("Halofly", halofly, pal("#1a1820", ("#3a3a50", "#585878", "#8080a0"), ("#3a3a50", "#585878", "#8080a0"),
                          extra=("#c0e0f0", "#d8f0ff", "#f0faff"), glow="#fff080", hi="#ffffe0"))
sp("Seraphly", seraphly, pal("#201a10", ("#a08040", "#d0b060", "#f0d890"), ("#a08040", "#d0b060", "#f0d890"), eye="#40c0ff",
                            extra=("#d0d8f0", "#e8f0ff", "#ffffff"), glow="#fff080", hi="#ffffe0"))
sp("Lochling", lochling, pal("#061014", ("#1a4a4a", "#2a6a64", "#4a9488"), ("#123050", "#2c6090", "#a8d0f0"),
                            extra=("#80b0a0", "#a0d0c0", "#d0f0e0")))
sp("Wendigo", wendigo, pal("#060608", ("#14121a", "#24202c", "#3a3444"), ("#2a1c10", "#4a3420", "#6a4c30"), eye="#e0f8ff",
                          extra=("#a8a090", "#d0c8b4", "#f0ecdc")))
sp("Hollowking", hollowking, pal("#08040a", ("#2a1030", "#40184a", "#5c2468"), ("#6a2a08", "#b0480c", "#ff8020"),
                                eye="#ff2050", extra=("#a8a090", "#d0c8b4", "#f0ecdc"), glow="#ffd030"))
sp("Eclipsa", eclipsa, pal("#06040e", ("#141028", "#241c44", "#3a2e68"), ("#3a1a5a", "#6a30a0", "#a060e0"), eye="#ffe040",
                          extra=("#c08a20", "#e0b040", "#ffe080"), glow="#ffb020", hi="#fff8c0"))
sp("Lycanth", lycanth, pal("#0a0a0e", ("#4a4e5a", "#6e7484", "#9aa2b4"), ("#2a2a3a", "#40405a", "#5a5a7a"), eye="#ffc020",
                          extra=("#c8ccd8", "#e0e4ee", "#f8faff")))


def variant_palette(p):
    """'Eclipse' rare variant: hue-shift body/accent ramps."""
    out = list(p)
    for i in (2, 3, 4, 5, 6, 7, 11, 12, 13):
        h = p[i].lstrip("#")
        r, g, b = int(h[0:2], 16) / 255, int(h[2:4], 16) / 255, int(h[4:6], 16) / 255
        hh, l, s = colorsys.rgb_to_hls(r, g, b)
        hh = (hh + 0.42) % 1.0
        s = min(1.0, s * 1.3 + 0.08)
        r, g, b = colorsys.hls_to_rgb(hh, l, s)
        out[i] = "#%02x%02x%02x" % (int(r * 255), int(g * 255), int(b * 255))
    return out


def make_icon(g):
    ic = [[0] * 16 for _ in range(16)]
    for y in range(16):
        for x in range(16):
            px = [g[y * 2 + a][x * 2 + b] for a in (0, 1) for b in (0, 1)]
            nz = [p for p in px if p not in (0,)]
            if len(nz) < 2:
                ic[y][x] = 0
                continue
            pri = [p for p in nz if p in (8, 9, 10, 14)]
            if pri and px.count(1) < 3:
                ic[y][x] = pri[0]
                continue
            inner = [p for p in nz if p != 1]
            pool = inner if len(inner) >= 2 else nz
            ic[y][x] = max(set(pool), key=pool.count)
    # re-outline
    add = []
    for y in range(16):
        for x in range(16):
            if ic[y][x] == 0:
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    a, b = x + dx, y + dy
                    if 0 <= a < 16 and 0 <= b < 16 and ic[b][a] not in (0, 1):
                        add.append((x, y))
                        break
    for x, y in add:
        ic[y][x] = 1
    return ic


for s in SPECIES:
    s["sprite"] = s["fn"]().g
    s["vpal"] = variant_palette(s["pal"])
