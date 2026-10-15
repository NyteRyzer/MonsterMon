// MonsterMons - graphics helpers, text, dialog boxes and menus
#include <stdarg.h>
#include <string.h>
#include "core.h"

OamEntry oam_buf[128];
bool tint_enabled;
bool world_obj_pals = true;   // OBJ banks hold overworld people (false in battle/menus)
static u16 bg_base[16 * 16];
static u16 obj_base[16 * 16];
static int cur_brightness;
u16 bld_normal;

#define TXT SCREENBLOCK(SB_TEXT)
#define TE(tile, pal) ((u16)((tile) | ((pal) << 12)))
#define CLEAR_ENTRY TE(UI_CLEAR, 0)
#define FILL_ENTRY TE(UI_WIN_FILL, 0)

void oam_clear(void) {
    for (int i = 0; i < 128; i++) {
        oam_buf[i].a0 = 0x0200;
        oam_buf[i].a1 = 0;
        oam_buf[i].a2 = 0;
    }
}

void spr(int i, int x, int y, int shape, int size, int tile, int pal, int prio, bool hflip) {
    if (x <= -64 || x >= 240 || y <= -64 || y >= 160) {
        oam_buf[i].a0 = 0x0200;
        return;
    }
    oam_buf[i].a0 = (y & 0xFF) | (shape << 14);
    oam_buf[i].a1 = (x & 0x1FF) | (size << 14) | (hflip ? 0x1000 : 0);
    oam_buf[i].a2 = tile | (prio << 10) | (pal << 12);
}

void spr_affine(int i, int x, int y, int shape, int size, int tile, int pal, int prio, int aff, bool dbl) {
    oam_buf[i].a0 = (y & 0xFF) | 0x100 | (dbl ? 0x200 : 0) | (shape << 14);
    oam_buf[i].a1 = (x & 0x1FF) | (size << 14) | (aff << 9);
    oam_buf[i].a2 = tile | (prio << 10) | (pal << 12);
}

void aff_set(int n, int pa, int pb, int pc, int pd) {
    oam_buf[n * 4 + 0].a3 = pa;
    oam_buf[n * 4 + 1].a3 = pb;
    oam_buf[n * 4 + 2].a3 = pc;
    oam_buf[n * 4 + 3].a3 = pd;
}

void load_person_gfx(void) {
    dma_copy32(OBJ_TILES, person_tiles, PERSON_COUNT * 36 * 8);
    for (int i = 0; i < PERSON_COUNT * 16; i++) obj_base[i] = person_pals[i];
    load_player_palette();
}

void load_player_palette(void) {
    obj_base[6] = player_coat_pals[sv.coat * 2];
    obj_base[7] = player_coat_pals[sv.coat * 2 + 1];
    obj_base[4] = player_hair_pals[sv.hair * 2];
    obj_base[5] = player_hair_pals[sv.hair * 2 + 1];
    for (int i = 0; i < 16; i++) PAL_OBJ[i] = obj_base[i];
    pal_refresh();
}

void pal_load_bg_banks(void) {
    for (int i = 0; i < 64; i++) bg_base[i] = ui_pals[i];
    for (int i = 0; i < 16; i++) bg_base[240 + i] = ui_pals[64 + i];   // bank 15: blue EXP bar
    for (int i = 0; i < 160; i++) bg_base[64 + i] = bg_pals[i];
    pal_refresh();
}

static u16 tint(u16 c, int fr, int fg, int fb) {
    int r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
    r = r * fr >> 8; g = g * fg >> 8; b = b * fb >> 8;
    if (r > 31) r = 31;
    if (g > 31) g = 31;
    if (b > 31) b = 31;
    return r | (g << 5) | (b << 10);
}

void tint_factors(int *fr, int *fg, int *fb) {
    *fr = *fg = *fb = 256;
    if (!tint_enabled) return;
    int h = clock_hour();
    if (h >= 20 || h < 5) { *fr = 135; *fg = 145; *fb = 210; }
    else if (h == 5) { *fr = 170; *fg = 160; *fb = 210; }
    else if (h == 6) { *fr = 230; *fg = 200; *fb = 200; }
    else if (h == 18) { *fr = 256; *fg = 200; *fb = 170; }
    else if (h == 19) { *fr = 200; *fg = 150; *fb = 170; }
    if (weather_now == WX_BLOODMOON) { *fr = 220; *fg = 90; *fb = 110; }
    if (weather_now == WX_RAIN) { *fr = *fr * 220 >> 8; *fg = *fg * 220 >> 8; *fb = *fb * 235 >> 8; }
}

void pal_refresh(void) {
    int fr, fg, fb;
    tint_factors(&fr, &fg, &fb);
    for (int i = 0; i < 64; i++) PAL_BG[i] = bg_base[i];
    for (int i = 240; i < 256; i++) PAL_BG[i] = bg_base[i];
    for (int bank = 4; bank < 14; bank++) {
        u16 em = bg_emissive[bank - 4];
        for (int i = 0; i < 16; i++) {
            u16 c = bg_base[bank * 16 + i];
            PAL_BG[bank * 16 + i] = (em >> i) & 1 ? c : tint(c, fr, fg, fb);
        }
    }
    for (int i = 0; world_obj_pals && i < PERSON_COUNT * 16; i++) {
        if ((i & 15) == 12) PAL_OBJ[i] = obj_base[i];  // glowing visors stay lit
        else PAL_OBJ[i] = tint(obj_base[i], fr, fg, fb);
    }
}

void scene_palette_tinted(const u16 *pal, int bank) {
    int fr, fg, fb;
    tint_factors(&fr, &fg, &fb);
    for (int i = 0; i < 16; i++) PAL_BG[bank * 16 + i] = tint(pal[i], fr, fg, fb);
}

void gfx_init(void) {
    REG_DISPCNT = DCNT_BLANK;
    dma_copy32(CHARBLOCK(CB_UI), ui_tiles, UI_TILE_COUNT * 8);
    dma_copy32(CHARBLOCK(CB_MAP), mt_tiles, MT_TILE_COUNT * 8);
    REG_BGCNT(0) = BG_PRIO(0) | BG_CBB(CB_UI) | BG_SBB(SB_TEXT);
    REG_BGCNT(1) = BG_PRIO(3) | BG_CBB(CB_MAP) | BG_SBB(SB_MAP);
    REG_BGCNT(2) = BG_PRIO(1) | BG_CBB(CB_MAP) | BG_SBB(SB_FOG);
    for (int i = 0; i < 4; i++) { REG_BGHOFS(i) = 0; REG_BGVOFS(i) = 0; }
    pal_load_bg_banks();
    load_person_gfx();
    oam_clear();
    txt_clear();
    bld_normal = 0;
    REG_BLDCNT = 0;
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ;
}

// ------------------------------------------------------------------ text layer
void txt_fill(u16 entry) { dma_fill32(TXT, entry | (entry << 16), 32 * 32 / 2); }
void txt_clear(void) { txt_fill(CLEAR_ENTRY); }

void txt_putc(int x, int y, char c, int pal) {
    if (x < 0 || x >= 32 || y < 0 || y >= 32) return;
    if (c < 32 || c > 126) c = '?';
    TXT[y * 32 + x] = TE(c - 32, pal);
}

void txt_put(int x, int y, const char *s, int pal) {
    while (*s) {
        if (*s == '\n') { y++; x = 0; s++; continue; }
        txt_putc(x++, y, *s++, pal);
    }
}

void txt_num(int x, int y, int n, int width, int pal) {
    char buf[12];
    int len = 0;
    bool neg = n < 0;
    unsigned u = neg ? -n : n;
    do { buf[len++] = '0' + u % 10; u /= 10; } while (u);
    if (neg) buf[len++] = '-';
    for (int i = len; i < width; i++) txt_putc(x++, y, ' ', pal);
    while (len) txt_putc(x++, y, buf[--len], pal);
}

void txt_rect(int x, int y, int w, int h, u16 entry) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            if (x + i >= 0 && x + i < 32 && y + j >= 0 && y + j < 32) TXT[(y + j) * 32 + x + i] = entry;
}

void win_draw(int x, int y, int w, int h) {
    txt_rect(x + 1, y + 1, w - 2, h - 2, FILL_ENTRY);
    for (int i = 1; i < w - 1; i++) {
        TXT[y * 32 + x + i] = TE(UI_WIN_T, 0);
        TXT[(y + h - 1) * 32 + x + i] = TE(UI_WIN_T, 0) | 0x800;
    }
    for (int j = 1; j < h - 1; j++) {
        TXT[(y + j) * 32 + x] = TE(UI_WIN_L, 0);
        TXT[(y + j) * 32 + x + w - 1] = TE(UI_WIN_L, 0) | 0x400;
    }
    TXT[y * 32 + x] = TE(UI_WIN_TL, 0);
    TXT[y * 32 + x + w - 1] = TE(UI_WIN_TL, 0) | 0x400;
    TXT[(y + h - 1) * 32 + x] = TE(UI_WIN_TL, 0) | 0x800;
    TXT[(y + h - 1) * 32 + x + w - 1] = TE(UI_WIN_TL, 0) | 0xC00;
}

void exp_bar(int x, int y, int tiles, u32 cur, u32 max) {
    if (!max) max = 1;
    int px = cur * tiles * 8 / max;
    if (px > tiles * 8) px = tiles * 8;
    TXT[y * 32 + x] = TE(UI_BAR_L, 0);
    for (int i = 0; i < tiles; i++) {
        int f = px - i * 8;
        if (f < 0) f = 0;
        if (f > 8) f = 8;
        TXT[y * 32 + x + 1 + i] = TE(UI_BAR0 + f, 15);
    }
    TXT[y * 32 + x + 1 + tiles] = TE(UI_BAR_R, 0);
}

void win_clear(int x, int y, int w, int h) { txt_rect(x, y, w, h, CLEAR_ENTRY); }

void hp_bar(int x, int y, int tiles, int cur, int max) {
    if (max <= 0) max = 1;
    if (cur < 0) cur = 0;
    int px = (cur * tiles * 8 + max - 1) / max;
    if (cur > 0 && px == 0) px = 1;
    int pal = cur * 5 > max * 2 ? 0 : (cur * 5 > max ? 1 : 2);
    TXT[y * 32 + x] = TE(UI_BAR_L, 0);
    for (int i = 0; i < tiles; i++) {
        int f = px - i * 8;
        if (f < 0) f = 0;
        if (f > 8) f = 8;
        TXT[y * 32 + x + 1 + i] = TE(UI_BAR0 + f, pal);
    }
    TXT[y * 32 + x + 1 + tiles] = TE(UI_BAR_R, 0);
}

// ------------------------------------------------------------------ fades
void set_brightness(int level) {
    cur_brightness = level;
    if (level <= 0) {
        REG_BLDCNT = bld_normal;
        REG_BLDY = 0;
    } else {
        REG_BLDCNT = 0x00FF;
        REG_BLDY = level > 16 ? 16 : level;
    }
}

void fade_to_black(int frames) {
    for (int i = 0; i <= frames; i++) {
        set_brightness(i * 16 / frames);
        frame();
    }
}

void fade_from_black(int frames) {
    for (int i = frames; i >= 0; i--) {
        set_brightness(i * 16 / frames);
        frame();
    }
}

void wait_frames(int n) {
    while (n--) frame();
}

// ------------------------------------------------------------------ formatting
static char *put_str(char *o, const char *s) {
    while (*s) *o++ = *s++;
    return o;
}

static void vfmt(char *out, const char *f, va_list ap) {
    char *o = out;
    while (*f) {
        if (*f != '%') { *o++ = *f++; continue; }
        f++;
        int width = 0;
        while (*f >= '0' && *f <= '9') width = width * 10 + (*f++ - '0');
        char c = *f++;
        if (c == 's') {
            o = put_str(o, va_arg(ap, const char *));
        } else if (c == 'd' || c == 'u') {
            int n = va_arg(ap, int);
            char buf[12];
            int len = 0;
            bool neg = c == 'd' && n < 0;
            unsigned u = neg ? -n : (unsigned)n;
            do { buf[len++] = '0' + u % 10; u /= 10; } while (u);
            if (neg) buf[len++] = '-';
            for (int i = len; i < width; i++) *o++ = ' ';
            while (len) *o++ = buf[--len];
        } else if (c == 'c') {
            *o++ = (char)va_arg(ap, int);
        } else if (c == '%') {
            *o++ = '%';
        }
    }
    *o = 0;
}

void fmt(char *out, const char *f, ...) {
    va_list ap;
    va_start(ap, f);
    vfmt(out, f, ap);
    va_end(ap);
}

// ------------------------------------------------------------------ dialog
#define BOX_Y 14
#define LINE_W 28

static bool box_open;

static void box_draw(void) {
    win_draw(0, BOX_Y, 30, 6);
    box_open = true;
}

void msg_close(void) {
    win_clear(0, BOX_Y - 1, 30, 7);
    box_open = false;
}

// typewriter one page (up to 2 lines); returns pointer to the rest of the text
static const char *show_page(const char *s, bool *more) {
    txt_rect(1, BOX_Y + 1, 28, 4, FILL_ENTRY);
    for (int line = 0; line < 2 && *s; line++) {
        // measure a line with word wrap
        int len = 0, last_space = -1;
        while (s[len] && s[len] != '\n' && s[len] != '\f' && len < LINE_W) {
            if (s[len] == ' ') last_space = len;
            len++;
        }
        int cut = len;
        if (s[len] && s[len] != '\n' && s[len] != '\f' && len == LINE_W && last_space > 0) cut = last_space;
        for (int i = 0; i < cut; i++) {
            txt_putc(1 + i, BOX_Y + 1 + line * 2, s[i], 0);
            if (s[i] != ' ' && !key_held(KEY_A | KEY_B)) frame();
            else if (s[i] != ' ' && (i & 3) == 0) frame();
        }
        s += cut;
        if (*s == ' ' || *s == '\n') s++;
        if (*s == '\f') { s++; break; }
    }
    *more = *s != 0;
    return s;
}

int msg_auto;   // >0: dialog pages auto-advance after this many frames (battle pacing)
static void wait_ack(bool more) {
    int t = 0;
    while (1) {
        frame();
        t++;
        txt_putc(28, BOX_Y + 4, ' ', 0);
        if ((t >> 4) & 1) TXT[(BOX_Y + 4) * 32 + 28] = TE(UI_MORE, 0);
        if (key_hit(KEY_A) || key_hit(KEY_B)) break;
        if (msg_auto && t >= msg_auto) break;
    }
    (void)more;
    sfx(SFX_CURSOR);
    TXT[(BOX_Y + 4) * 32 + 28] = FILL_ENTRY;
}

static const char *speaker;

void msg_noclose(const char *s) {
    if (!box_open) box_draw();
    if (speaker && *speaker) {
        int n = strlen(speaker);
        win_draw(0, BOX_Y - 2, n + 2, 3);
        txt_put(1, BOX_Y - 1, speaker, 1);
        TXT[BOX_Y * 32 + 0] = TE(UI_WIN_L, 0);
        for (int i = 1; i <= n; i++) TXT[BOX_Y * 32 + i] = FILL_ENTRY;
        TXT[BOX_Y * 32 + n + 1] = TE(UI_WIN_T, 0);
    }
    bool more = true;
    while (more) {
        s = show_page(s, &more);
        wait_ack(more);
    }
}

void msg(const char *s) {
    msg_noclose(s);
    msg_close();
}

void msgf(const char *f, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, f);
    vfmt(buf, f, ap);
    va_end(ap);
    msg(buf);
}

void say(const char *who, const char *s) {
    char buf[320];
    // expand {P} to player name
    char *o = buf;
    while (*s && o < buf + 300) {
        if (s[0] == '{' && s[1] == 'P' && s[2] == '}') { o = put_str(o, sv.name); s += 3; }
        else *o++ = *s++;
    }
    *o = 0;
    win_clear(0, BOX_Y - 2, 30, 2);
    speaker = who;
    msg_noclose(buf);
    speaker = 0;
    msg_close();
    win_clear(0, BOX_Y - 2, 30, 2);
}

// ------------------------------------------------------------------ menus
u16 key_repeat(void) {
    static int held_frames;
    u16 dirs = key_cur & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT);
    if (!dirs) { held_frames = 0; return 0; }
    held_frames++;
    if (held_frames == 1) return dirs;
    if (held_frames > 14 && (held_frames & 3) == 0) return dirs;
    return 0;
}

int menu(int x, int y, int w, const char *const *opts, int n, int start, bool cancel) {
    win_draw(x, y, w, n + 2);
    for (int i = 0; i < n; i++) txt_put(x + 2, y + 1 + i, opts[i], 0);
    int sel = start < n ? start : 0;
    while (1) {
        for (int i = 0; i < n; i++) TXT[(y + 1 + i) * 32 + x + 1] = i == sel ? TE(UI_CURSOR, 0) : FILL_ENTRY;
        frame();
        u16 r = key_repeat();
        if (r & KEY_UP) { sel = (sel + n - 1) % n; sfx(SFX_CURSOR); }
        if (r & KEY_DOWN) { sel = (sel + 1) % n; sfx(SFX_CURSOR); }
        if (key_hit(KEY_A)) { sfx(SFX_OK); win_clear(x, y, w, n + 2); return sel; }
        if (cancel && key_hit(KEY_B)) { sfx(SFX_BACK); win_clear(x, y, w, n + 2); return -1; }
    }
}

bool yesno(const char *q) {
    static const char *const yn[2] = { "YES", "NO" };
    msg_noclose(q);
    int r = menu(23, 9, 7, yn, 2, 0, true);
    msg_close();
    return r == 0;
}

// ------------------------------------------------------------------ monster previews
void show_mon_preview(int sp, int variant, int x, int y, int slot, bool big) {
    dma_copy32(OBJ_TILES + (512 + slot * 16) * 16, mon_tiles + sp * 16 * 8, 16 * 8);
    const u16 *p = (variant ? mon_vpals : mon_pals) + sp * 16;
    for (int i = 0; i < 16; i++) PAL_OBJ[(10 + slot) * 16 + i] = p[i];
    if (big) {
        aff_set(slot, 0x80, 0, 0, 0x80);
        spr_affine(100 + slot, x, y, 0, 2, 512 + slot * 16, 10 + slot, 0, slot, true);
    } else {
        spr(100 + slot, x, y, 0, 2, 512 + slot * 16, 10 + slot, 0, false);
    }
}
