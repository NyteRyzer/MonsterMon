// MonsterMons - overworld: maps, camera, movement, NPCs, encounters, weather
#include <string.h>
#include "world.h"

u8 weather_now;
Npc npcs[MAX_NPCS];
int npc_n;
int ply_x, ply_y, ply_facing;

static u8 mapbuf[64 * 40];
static int cur_map, mw, mh;
static int cam_x, cam_y;
static bool moving;
static int move_off, step_parity, bump_timer;
static int banner_timer;
static int fog_scroll;
static u8 icon_species[3];
static int icon_n;

typedef struct { s16 x, y; } Drop;
static Drop drops[16];

extern u16 bld_normal;
void scene_palette_tinted(const u16 *pal, int bank);

static const s8 DX[4] = { 0, 0, -1, 1 };
static const s8 DY[4] = { 1, -1, 0, 0 };

bool map_is_outdoor(int map) {
    return map == MAP_HOLLOWMERE || map == MAP_WOODS || map == MAP_GRAVE || map == MAP_NEON;
}

u8 world_tile(int x, int y) {
    if (x < 0 || y < 0 || x >= mw || y >= mh) return MT_BLACK;
    return mapbuf[y * mw + x];
}

static void draw_metatile(int tx, int ty) {
    u8 mt = world_tile(tx, ty);
    const u16 *e = &mt_entries[mt * 4];
    vu16 *sb = SCREENBLOCK(SB_MAP);
    int sx = (tx * 2) & 31, sy = (ty * 2) & 31;
    sb[sy * 32 + sx] = e[0];
    sb[sy * 32 + sx + 1] = e[1];
    sb[(sy + 1) * 32 + sx] = e[2];
    sb[(sy + 1) * 32 + sx + 1] = e[3];
}

void world_set_tile(int x, int y, u8 mt) {
    if (x < 0 || y < 0 || x >= mw || y >= mh) return;
    mapbuf[y * mw + x] = mt;
    draw_metatile(x, y);
}

static int floordiv16(int v) { return v >= 0 ? v / 16 : -((15 - v) / 16); }

void world_redraw(void) {
    int bx = floordiv16(cam_x), by = floordiv16(cam_y);
    for (int y = 0; y < 11; y++)
        for (int x = 0; x < 16; x++) draw_metatile(bx + x, by + y);
    REG_BGHOFS(1) = cam_x & 0x1FF;
    REG_BGVOFS(1) = cam_y & 0x1FF;
}

static void update_camera(void) {
    int wx = ply_x * 16 + (moving ? DX[ply_facing] * move_off : 0);
    int wy = ply_y * 16 + (moving ? DY[ply_facing] * move_off : 0);
    int ncx, ncy;
    if (mw * 16 <= 240) ncx = (mw * 16 - 240) / 2;
    else {
        ncx = wx - 112;
        if (ncx < 0) ncx = 0;
        if (ncx > mw * 16 - 240) ncx = mw * 16 - 240;
    }
    if (mh * 16 <= 160) ncy = (mh * 16 - 160) / 2;
    else {
        ncy = wy - 72;
        if (ncy < 0) ncy = 0;
        if (ncy > mh * 16 - 160) ncy = mh * 16 - 160;
    }
    if (ncx != cam_x || ncy != cam_y) {
        cam_x = ncx;
        cam_y = ncy;
        world_redraw();
    }
}

// ------------------------------------------------------------------ NPCs
static bool npc_def_visible(const NpcDef *d) {
    if (d->show_flag >= 0 && !flag(d->show_flag)) return false;
    if (d->hide_flag >= 0 && flag(d->hide_flag)) return false;
    return true;
}

static int icon_slot(int sp) {
    for (int i = 0; i < icon_n; i++) if (icon_species[i] == sp) return i;
    if (icon_n >= 3) return 0;
    icon_species[icon_n] = sp;
    dma_copy32(OBJ_TILES + (432 + icon_n * 4) * 16, mon_icon_tiles + sp * 4 * 8, 4 * 8);
    for (int i = 0; i < 16; i++) PAL_OBJ[(12 + icon_n) * 16 + i] = mon_pals[sp * 16 + i];
    return icon_n++;
}

void world_refresh_npcs(void) {
    npc_n = 0;
    icon_n = 0;
    for (int i = 0; i < npc_def_count && npc_n < MAX_NPCS; i++) {
        const NpcDef *d = &npc_defs[i];
        if (d->map != cur_map || !npc_def_visible(d)) continue;
        Npc *n = &npcs[npc_n++];
        n->def = d;
        n->x = d->x;
        n->y = d->y;
        n->facing = d->facing;
        n->active = true;
        if (d->sprite >= 100) icon_slot(d->sprite - 100);
    }
}

static int npc_at(int x, int y) {
    for (int i = 0; i < npc_n; i++)
        if (npcs[i].active && npcs[i].x == x && npcs[i].y == y) return i;
    return -1;
}

void npc_face(int i, int dir) { npcs[i].facing = dir; }

static void person_sprite(int oam, int sx, int sy, int person, int facing, int anim) {
    int base = facing == DIR_DOWN ? 0 : facing == DIR_UP ? 3 : 6;
    int tile = person * 36 + (base + anim) * 4;
    spr(oam, sx, sy, 0, 1, tile, person, 2, facing == DIR_RIGHT);
}

void world_draw_sprites(void) {
    int sx = ply_x * 16 + (moving ? DX[ply_facing] * move_off : 0) - cam_x;
    int sy = ply_y * 16 + (moving ? DY[ply_facing] * move_off : 0) - cam_y - 2;
    int anim = moving ? (move_off < 8 ? 1 + step_parity : 0) : 0;
    person_sprite(0, sx, sy, PS_PLAYER, ply_facing, anim);
    for (int i = 0; i < MAX_NPCS; i++) {
        if (i >= npc_n || !npcs[i].active) { spr(1 + i, 240, 160, 0, 0, 0, 0, 0, false); continue; }
        Npc *n = &npcs[i];
        int nx = n->x * 16 - cam_x, ny = n->y * 16 - cam_y - 2;
        if (n->def->sprite >= 100) {
            int slot = icon_slot(n->def->sprite - 100);
            int bob = (frame_count >> 4) & 1;
            spr(1 + i, nx, ny + bob, 0, 1, 432 + slot * 4, 12 + slot, 2, false);
        } else {
            person_sprite(1 + i, nx, ny, n->def->sprite, n->facing, 0);
        }
    }
    // rain
    for (int i = 0; i < 16; i++) {
        if (weather_now == WX_RAIN) {
            Drop *d = &drops[i];
            d->x -= 2;
            d->y += 6;
            if (d->y > 160 || d->x < -8) { d->x = rnd_n(260); d->y = -8 - rnd_n(60); }
            spr(20 + i, d->x, d->y, 0, 0, 470, 15, 0, false);
        } else {
            spr(20 + i, 240, 160, 0, 0, 0, 0, 0, false);
        }
    }
}

// ------------------------------------------------------------------ weather & effects
static void setup_effect_gfx(void) {
    // rain drop tile (OBJ 470)
    static const u32 drop[8] = { 0x00000010, 0x00000100, 0x00000100, 0x00001000, 0x00001000, 0x00010000, 0, 0 };
    dma_copy32(OBJ_TILES + 470 * 16, drop, 8);
    PAL_OBJ[15 * 16 + 1] = RGB15(18, 22, 31);
    // fog tiles (BG tiles 500..503, palette bank 13)
    vu32 *t = (vu32 *)(CHARBLOCK(CB_MAP) + 500 * 16);
    for (int k = 0; k < 4; k++) {
        for (int y = 0; y < 8; y++) {
            u32 w = 0;
            for (int x = 0; x < 8; x++) {
                int gx = (k & 1) * 8 + x, gy = (k >> 1) * 8 + y;
                int v = (gx * 7 + gy * 13 + ((gx * gy) % 11) * 3) % 16;
                int c = v < 5 ? 0 : v < 9 ? 1 : v < 12 ? 2 : v < 15 ? 3 : 4;
                w |= (u32)c << (x * 4);
            }
            t[k * 8 + y] = w;
        }
    }
    vu16 *sb = SCREENBLOCK(SB_FOG);
    for (int y = 0; y < 32; y++)
        for (int x = 0; x < 32; x++) sb[y * 32 + x] = (500 + ((x & 1) | ((y & 1) << 1))) | (13 << 12);
}

static void apply_weather_display(void) {
    u16 disp = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ;
    if (weather_now == WX_FOG) {
        disp |= DCNT_BG2;
        bld_normal = 0x0004 | 0x0040 | 0x0200 | 0x1000 | 0x2000;
        REG_BLDALPHA = 7 | (11 << 8);
    } else {
        bld_normal = 0;
    }
    REG_DISPCNT = disp;
    REG_BLDCNT = bld_normal;
}

static void roll_weather(void) {
    int r = rnd_n(100);
    weather_now = WX_CLEAR;
    switch (cur_map) {
    case MAP_HOLLOWMERE: weather_now = r < 15 ? WX_RAIN : r < 22 ? WX_FOG : WX_CLEAR; break;
    case MAP_WOODS: weather_now = r < 25 ? WX_RAIN : r < 55 ? WX_FOG : WX_CLEAR; break;
    case MAP_GRAVE: weather_now = r < 15 ? WX_RAIN : r < 60 ? WX_FOG : WX_CLEAR; break;
    case MAP_NEON: weather_now = r < 55 ? WX_RAIN : WX_CLEAR; break;
    }
    if (map_is_outdoor(cur_map) && cur_map != MAP_HOLLOWMERE && is_night() && rnd_n(10) == 0) weather_now = WX_BLOODMOON;
    sv.weather = weather_now;
}

void world_banner(const char *s) {
    int n = strlen(s);
    win_clear(0, 0, 30, 3);
    win_draw(0, 0, n + 2, 3);
    txt_put(1, 1, s, 1);
    banner_timer = 100;
}

// ------------------------------------------------------------------ map entry
void world_reload_gfx(void) {
    REG_DISPCNT = DCNT_BLANK;
    dma_copy32(CHARBLOCK(CB_MAP), mt_tiles, MT_TILE_COUNT * 8);
    REG_BGCNT(0) = BG_PRIO(0) | BG_CBB(CB_UI) | BG_SBB(SB_TEXT);
    REG_BGCNT(1) = BG_PRIO(3) | BG_CBB(CB_MAP) | BG_SBB(SB_MAP);
    REG_BGCNT(2) = BG_PRIO(1) | BG_CBB(CB_MAP) | BG_SBB(SB_FOG);
    load_person_gfx();
    tint_enabled = map_is_outdoor(cur_map);
    pal_load_bg_banks();
    icon_n = 0;
    world_refresh_npcs();
    setup_effect_gfx();
    txt_clear();
    oam_clear();
    cam_x = -1000;
    update_camera();
    world_redraw();
    world_draw_sprites();
    apply_weather_display();
}

void world_set_player(int x, int y, int facing) {
    ply_x = x;
    ply_y = y;
    ply_facing = facing;
    moving = false;
}

void world_enter(int map, int x, int y, int facing) {
    cur_map = map;
    const MapDef *m = &map_defs[map];
    mw = m->w;
    mh = m->h;
    memcpy(mapbuf, m->data, mw * mh);
    sv.map = map;
    world_set_player(x, y, facing);
    clock_running = true;
    roll_weather();
    events_apply_map_patches();
    world_reload_gfx();
    static const u8 music[4] = { BGM_TOWN, BGM_FIELD, BGM_DUNGEON, BGM_RIFT };
    bgm(music[m->bgm]);
    fade_from_black(16);
    world_banner(m->name);
    events_map_enter();
}

// ------------------------------------------------------------------ encounters
typedef struct { u8 sp, lo, hi, cond, w; } Enc;
enum { C_ANY, C_DAY, C_NIGHT, C_RAIN, C_FOG };
#define E(s, l, h, c, w) { SP_##s, l, h, c, w }
static const Enc enc_woods[] = {
    E(MOTHKIN, 3, 6, C_ANY, 25), E(SQUATCHY, 4, 6, C_ANY, 15), E(HALOFLY, 3, 6, C_DAY, 20), E(BATLING, 3, 6, C_NIGHT, 25),
    E(HEXCAT, 5, 7, C_NIGHT, 10), E(CHUPA, 5, 7, C_ANY, 5), E(MOTHKIN, 5, 7, C_FOG, 10), { 0, 0, 0, 0, 0 } };
static const Enc enc_lake[] = {
    E(LOCHLING, 5, 7, C_ANY, 30), E(MOTHKIN, 3, 6, C_ANY, 30), E(HALOFLY, 3, 6, C_DAY, 20), E(BATLING, 3, 6, C_NIGHT, 20),
    { 0, 0, 0, 0, 0 } };
static const Enc enc_glade[] = {
    E(WENDIGO, 10, 12, C_NIGHT, 30), E(HALOFLY, 6, 8, C_DAY, 30), E(MOTHKIN, 6, 8, C_ANY, 30), E(HEXCAT, 7, 9, C_NIGHT, 20),
    E(LOCHLING, 7, 9, C_ANY, 10), { 0, 0, 0, 0, 0 } };
static const Enc enc_grave[] = {
    E(GHOULET, 8, 12, C_ANY, 25), E(BONEHOP, 9, 12, C_ANY, 20), E(WISPETTE, 9, 12, C_NIGHT, 20), E(BATLING, 9, 11, C_NIGHT, 15),
    E(WRAPLING, 10, 12, C_ANY, 15), E(BANSHRIEK, 13, 14, C_NIGHT, 5), E(MOTHKIN, 9, 11, C_FOG, 10), { 0, 0, 0, 0, 0 } };
static const Enc enc_neon[] = {
    E(IMPISH, 14, 17, C_ANY, 25), E(GLITCHOST, 15, 18, C_RAIN, 30), E(CHUPA, 14, 17, C_ANY, 15), E(HEXCAT, 15, 17, C_NIGHT, 15),
    E(CERBERPUP, 16, 18, C_ANY, 5), E(BATLING, 14, 16, C_NIGHT, 10), { 0, 0, 0, 0, 0 } };
static const Enc enc_manor[] = {
    E(GARGOYL, 18, 21, C_ANY, 20), E(CANDLEWICK, 18, 22, C_ANY, 20), E(WISPETTE, 18, 21, C_NIGHT, 15), E(CHIROVAMP, 19, 22, C_ANY, 15),
    E(HEXCAT, 18, 21, C_ANY, 15), E(WRAPLING, 19, 22, C_ANY, 10), E(BANSHRIEK, 20, 22, C_NIGHT, 5), { 0, 0, 0, 0, 0 } };
static const Enc enc_castle[] = {
    E(GHOULORD, 24, 28, C_ANY, 20), E(HELLION, 25, 28, C_ANY, 15), E(GARGOYL, 24, 27, C_ANY, 15), E(WRAPLING, 24, 26, C_ANY, 15),
    E(CERBERPUP, 25, 28, C_ANY, 15), E(CHIROVAMP, 25, 28, C_NIGHT, 10), E(BANSHRIEK, 25, 28, C_NIGHT, 10), { 0, 0, 0, 0, 0 } };
static const Enc enc_rift[] = {
    E(GRIMALKIN, 32, 38, C_ANY, 20), E(SERAPHLY, 32, 38, C_ANY, 20), E(SASQUATCH, 33, 38, C_ANY, 15), E(CERBERUS, 34, 40, C_ANY, 12),
    E(MOTHMAN, 33, 38, C_NIGHT, 15), E(COUNTESS, 34, 38, C_NIGHT, 5), E(MOONHOWL, 34, 38, C_NIGHT, 5), E(LANTERNIS, 34, 38, C_ANY, 5),
    E(DAWNFANG, 34, 38, C_DAY, 3), E(DIREWOLF, 34, 38, C_DAY, 5), { 0, 0, 0, 0, 0 } };

static bool cond_ok(int c) {
    switch (c) {
    case C_DAY: return !is_night();
    case C_NIGHT: return is_night();
    case C_RAIN: return weather_now == WX_RAIN;
    case C_FOG: return weather_now == WX_FOG;
    }
    return true;
}

static const Enc *pick(const Enc *t) {
    int total = 0;
    for (const Enc *e = t; e->w; e++) if (cond_ok(e->cond)) total += e->w;
    if (!total) return 0;
    int r = rnd_n(total);
    for (const Enc *e = t; e->w; e++) {
        if (!cond_ok(e->cond)) continue;
        if (r < e->w) return e;
        r -= e->w;
    }
    return 0;
}

static const Enc *table_for(int x, int y) {
    switch (cur_map) {
    case MAP_WOODS:
        if (x >= 19) return enc_glade;
        if (x <= 9 && y <= 11) return enc_lake;
        return enc_woods;
    case MAP_GRAVE: return enc_grave;
    case MAP_NEON: return enc_neon;
    case MAP_MANOR: return enc_manor;
    case MAP_CASTLE: return enc_castle;
    case MAP_RIFT: return enc_rift;
    }
    return 0;
}

static void battle_transition(void) {
    sfx(SFX_ENCOUNTER);
    for (int k = 0; k < 3; k++) {
        set_brightness(12); wait_frames(3);
        set_brightness(0); wait_frames(3);
    }
    REG_MOSAIC = 0;
    fade_to_black(12);
}

void whiteout(void) {
    msg("Your team is exhausted... You retreat to safety, dropping some coins in the dark.");
    sv.money /= 2;
    party_heal_all();
    fade_to_black(16);
    world_enter(sv.last_heal_map, sv.last_heal_x, sv.last_heal_y, DIR_DOWN);
}

static void try_encounter(int x, int y) {
    int rate = 10;
    if (sv.tool == IT_WARD) rate = 20;
    if (rnd_n(rate) != 0) return;
    const Enc *t = table_for(x, y);
    if (!t) return;
    const Enc *e = pick(t);
    if (!e) return;
    int bonus = (weather_now == WX_BLOODMOON ? 3 : 0) + (flag(F_GAME_CLEAR) && cur_map == MAP_RIFT ? 10 : 0);
    int lv = e->lo + rnd_n(e->hi - e->lo + 1) + bonus;
    int sp2 = -1, lv2 = 0;
    if (is_night() && rnd_n(5) == 0) {
        const Enc *e2 = pick(t);
        if (e2) { sp2 = e2->sp; lv2 = e2->lo + rnd_n(e2->hi - e2->lo + 1) + bonus; }
    }
    battle_transition();
    int r = battle_wild(e->sp, lv, sp2, lv2, false);
    world_reload_gfx();
    if (r == BR_LOSE) { whiteout(); return; }
    fade_from_black(12);
}

// ------------------------------------------------------------------ movement
static bool passable(int x, int y) {
    u8 a = mt_attr[world_tile(x, y)];
    if (x < 0 || y < 0 || x >= mw || y >= mh) return false;
    if (a == A_FOG) return sv.tool == IT_LANTERN;
    if (a != A_WALK && a != A_GRASS && a != A_DOOR) return false;
    if (npc_at(x, y) >= 0) return false;
    return true;
}

static bool check_warp(void) {
    for (int i = 0; i < warp_count; i++) {
        const Warp *w = &warps[i];
        if (w->map == cur_map && w->x == ply_x && w->y == ply_y) {
            sfx(SFX_DOOR);
            fade_to_black(12);
            world_enter(w->dest, w->dx, w->dy, w->facing);
            return true;
        }
    }
    return false;
}

static void step_done(void) {
    if (events_step(ply_x, ply_y)) return;
    if (check_warp()) return;
    if (mt_attr[world_tile(ply_x, ply_y)] == A_GRASS) try_encounter(ply_x, ply_y);
}

static void interact(void) {
    int fx = ply_x + DX[ply_facing], fy = ply_y + DY[ply_facing];
    int n = npc_at(fx, fy);
    if (n < 0 && mt_attr[world_tile(fx, fy)] == A_COUNTER) n = npc_at(fx + DX[ply_facing], fy + DY[ply_facing]);
    if (n >= 0) {
        static const u8 opposite[4] = { DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT };
        if (npcs[n].def->sprite < 100) npcs[n].facing = opposite[ply_facing];
        world_draw_sprites();
        frame();
        events_npc(npcs[n].def->script);
        return;
    }
    events_interact_tile(fx, fy, world_tile(fx, fy));
}

void world_run(void) {
    while (1) {
        frame();
        if (banner_timer && --banner_timer == 0) win_clear(0, 0, 30, 3);
        if (weather_now == WX_FOG) {
            fog_scroll++;
            REG_BGHOFS(2) = (cam_x + (fog_scroll >> 2)) & 0x1FF;
            REG_BGVOFS(2) = (cam_y + (fog_scroll >> 3)) & 0x1FF;
        }
        if (bump_timer) bump_timer--;
        if (!moving) {
            if (key_hit(KEY_START)) {
                win_clear(0, 0, 30, 3);
                banner_timer = 0;
                start_menu();
                continue;
            }
            if (key_hit(KEY_A)) {
                interact();
                continue;
            }
            int dir = -1;
            if (key_held(KEY_DOWN)) dir = DIR_DOWN;
            else if (key_held(KEY_UP)) dir = DIR_UP;
            else if (key_held(KEY_LEFT)) dir = DIR_LEFT;
            else if (key_held(KEY_RIGHT)) dir = DIR_RIGHT;
            if (dir >= 0) {
                ply_facing = dir;
                int nx = ply_x + DX[dir], ny = ply_y + DY[dir];
                if (passable(nx, ny)) {
                    moving = true;
                    move_off = 0;
                } else if (!bump_timer) {
                    sfx(SFX_BUMP);
                    bump_timer = 20;
                }
            }
        }
        if (moving) {
            move_off += key_held(KEY_B) ? 4 : 2;
            if (move_off >= 16) {
                moving = false;
                ply_x += DX[ply_facing];
                ply_y += DY[ply_facing];
                step_parity ^= 1;
                sv.px = ply_x;
                sv.py = ply_y;
                sv.facing = ply_facing;
                update_camera();
                world_draw_sprites();
                step_done();
                continue;
            }
        }
        update_camera();
        world_draw_sprites();
    }
}
