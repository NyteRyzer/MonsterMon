// MonsterMons - turn-based team battles (up to 3 vs 3)
#include <string.h>
#include "core.h"

#define TXT SCREENBLOCK(SB_TEXT)
void scene_palette_tinted(const u16 *pal, int bank);
bool map_is_outdoor(int map);

typedef struct {
    Mon *m;
    u8 side, idx;          // idx = party index (player) / enemy array index
    s8 stage[4];
    u8 sleep, fear;
    bool active;
    bool trust, marked;
    s8 lunge;
    bool hidden;
    u8 scale;              // 0 = normal, else affine shrink amount
} Bat;

static Bat B[2][3];
static Mon foes[6];
static int foe_n, foe_next;
static int slots[2];
static bool wild, boss;
static const Trainer *trainer;
static int soul_gauge;
static bool ritual;
static bool participated[6];
static bool leveled[6];
u8 last_caught_slot;

enum { ACT_NONE, ACT_MOVE, ACT_COMBO };
typedef struct { u8 kind, side, slot, move, tside, tslot; } Act;

// ------------------------------------------------------------------ presentation
static void sprite_pos(int side, int slot, int *x, int *y, bool *big) {
    int n = slots[side];
    *big = n == 1;
    if (side == 1) {
        if (n == 1) { *x = 155; *y = -10; }
        else if (n == 2) { *x = 150 + slot * 44; *y = 14; }
        else { *x = 138 + slot * 34; *y = 14; }
    } else {
        if (n == 1) { *x = 28; *y = 42; }
        else if (n == 2) { *x = 20 + slot * 44; *y = 70; }
        else { *x = 6 + slot * 38; *y = 70; }
    }
}

static int gslot(int side, int slot) { return side * 3 + slot; }

static void load_slot(int side, int slot) {
    Bat *b = &B[side][slot];
    int g = gslot(side, slot);
    dma_copy32(OBJ_TILES + (512 + g * 16) * 16, mon_tiles + b->m->species * 16 * 8, 16 * 8);
    const u16 *p = (b->m->variant ? mon_vpals : mon_pals) + b->m->species * 16;
    for (int i = 0; i < 16; i++) PAL_OBJ[(10 + g) * 16 + i] = p[i];
}

static void draw_sprites(void) {
    for (int side = 0; side < 2; side++) {
        for (int s = 0; s < 3; s++) {
            Bat *b = &B[side][s];
            int g = gslot(side, s);
            if (s >= slots[side] || !b->active || b->hidden) { spr(100 + g, 240, 160, 0, 0, 0, 0, 0, false); continue; }
            int x, y;
            bool big;
            sprite_pos(side, s, &x, &y, &big);
            x += side == 0 ? b->lunge : -b->lunge;
            if (big || b->scale) {
                int sc = big ? 0x80 : 0x100;
                sc = sc + b->scale * 24;
                if (side == 0) aff_set(g, -sc, 0, 0, sc);
                else aff_set(g, sc, 0, 0, sc);
                if (!big) { x -= 16; y -= 16; }
                spr_affine(100 + g, x, y, 0, 2, 512 + g * 16, 10 + g, 1, g, true);
            } else {
                spr(100 + g, x, y, 0, 2, 512 + g * 16, 10 + g, 1, side == 0);
            }
        }
    }
}

static void bframe(void) {
    draw_sprites();
    frame();
}

static void draw_panels(void) {
    for (int side = 0; side < 2; side++) {
        int n = slots[side];
        int h = n * 2 + 2;
        int x = side == 1 ? 0 : 15;
        int y = side == 1 ? 0 : 14 - h;
        if (side == 1) win_clear(0, 0, 15, 8);
        else win_clear(15, 6, 15, 8);
        win_draw(x, y, 15, h);
        for (int s = 0; s < n; s++) {
            Bat *b = &B[side][s];
            int ry = y + 1 + s * 2;
            if (!b->active) { txt_put(x + 1, ry, "  ---", 2); continue; }
            Mon *m = b->m;
            txt_put(x + 1, ry, mon_name(m), m->hp ? 0 : 2);
            txt_putc(x + 10, ry, m->variant ? '*' : ' ', 1);
            txt_putc(x + 11, ry, 'L', 1);
            txt_num(x + 12, ry, m->level, 0, 1);
            hp_bar(x + 1, ry + 1, 6, m->hp, mon_maxhp(m));
            if (side == 0) txt_num(x + 9, ry + 1, m->hp, 3, 0);
            if (m->status) txt_put(x + 10 + (side == 0 ? 2 : -1), ry + 1, status_names[m->status], 3);
        }
    }
    // soul gauge
    if (slots[0] >= 2) {
        int y = 14 - (slots[0] * 2 + 2);
        txt_put(15, y - 1, "SOUL", 1);
        hp_bar(19, y - 1, 4, soul_gauge, 100);
        txt_putc(25, y - 1, soul_gauge >= 100 ? '!' : ' ', 1);
    }
}

static void hit_flash(int side, int slot) {
    for (int k = 0; k < 8; k++) {
        B[side][slot].hidden = k & 1;
        bframe();
        bframe();
    }
    B[side][slot].hidden = false;
}

static void lunge(int side, int slot) {
    static const s8 seq[] = { 3, 6, 9, 10, 8, 4, 0 };
    for (unsigned k = 0; k < sizeof(seq); k++) {
        B[side][slot].lunge = seq[k];
        bframe();
    }
}

static void animate_hp(int side, int slot, int from, int to) {
    Mon *m = B[side][slot].m;
    int step = (from > to ? from - to : to - from) / 24 + 1;
    int v = from;
    while (v != to) {
        if (v > to) { v -= step; if (v < to) v = to; }
        else { v += step; if (v > to) v = to; }
        m->hp = v;
        draw_panels();
        bframe();
    }
}

// ------------------------------------------------------------------ helpers
static Bat *bat(int side, int slot) { return &B[side][slot]; }
static bool alive(Bat *b) { return b->active && b->m->hp > 0; }

static int count_alive(int side) {
    int n = 0;
    for (int s = 0; s < slots[side]; s++) n += alive(&B[side][s]);
    return n;
}

static int stage_apply(int v, int st) {
    if (st >= 0) return v * (2 + st) / 2;
    return v * 2 / (2 - st);
}

static bool physical(int type) { return type == T_BEAST || type == T_BLOOD || type == T_CRYPTID || type == T_UNDEAD; }

static void bmsg(const char *f, const char *a, const char *b2) {
    char buf[160];
    fmt(buf, f, a, b2);
    msg(buf);
    draw_panels();
}

static const char *pname(Bat *b) {
    static char buf[2][24];
    static int k;
    k ^= 1;
    if (b->side == 1 && wild) fmt(buf[k], "Wild %s", mon_name(b->m));
    else if (b->side == 1) fmt(buf[k], "Foe %s", mon_name(b->m));
    else fmt(buf[k], "%s", mon_name(b->m));
    return buf[k];
}

static bool party_active(int idx) {
    for (int s = 0; s < slots[0]; s++)
        if (B[0][s].active && B[0][s].idx == idx) return true;
    return false;
}

static void place(int side, int slot, Mon *m, int idx) {
    Bat *b = &B[side][slot];
    memset(b, 0, sizeof(*b));
    b->m = m;
    b->side = side;
    b->idx = idx;
    b->active = true;
    load_slot(side, slot);
    if (side == 0) participated[idx] = true;
    else mark_seen(m->species);
}

// ------------------------------------------------------------------ damage
static int damage_calc(Bat *a, Bat *d, int mv, int power, bool spread, int *eff_out, bool *crit_out) {
    const Move *M = &moves[mv];
    int type = M->type;
    bool phys = type == T_NONE ? true : physical(type);
    int A = stage_apply(mon_stat(a->m, phys ? 0 : 3), a->stage[phys ? 0 : 3]);
    int D = stage_apply(mon_stat(d->m, phys ? 1 : 3), d->stage[phys ? 1 : 3]);
    if (phys && a->m->status == ST_BURN) A /= 2;
    if (D < 1) D = 1;
    int lv = a->m->level;
    int dmg = ((2 * lv / 5 + 2) * power * A / D) / 50 + 2;
    const Species *as = &species[a->m->species], *ds = &species[d->m->species];
    if (type != T_NONE && (as->t1 == type || as->t2 == type)) dmg = dmg * 3 / 2;
    int e4 = type == T_NONE ? 4 : type_mult4(type, ds->t1, ds->t2);
    dmg = dmg * e4 / 4;
    if (is_night() && (type == T_SHADE || type == T_BLOOD)) dmg = dmg * 6 / 5;
    if (!is_night() && type == T_LUMIN) dmg = dmg * 6 / 5;
    if (weather_now == WX_RAIN && type == T_INFERNAL) dmg /= 2;
    if (weather_now == WX_BLOODMOON && type == T_BLOOD) dmg = dmg * 3 / 2;
    if (a->side == 0 && sv.weapon == IT_SILVER) dmg = dmg * 21 / 20;
    bool crit = rnd_n(a->side == 0 && a->m->bond >= 200 ? 8 : 16) == 0;
    if (crit) dmg = dmg * 3 / 2;
    if (spread) dmg = dmg * 3 / 4;
    dmg = dmg * (85 + rnd_n(16)) / 100;
    if (dmg < 1) dmg = 1;
    *eff_out = e4;
    *crit_out = crit;
    return dmg;
}

static void apply_damage(Bat *d, int dmg) {
    int from = d->m->hp;
    int to = from - dmg;
    if (to < 0) to = 0;
    if (to == 0 && d->side == 0 && d->m->bond >= 100 && from > 1 && rnd_n(10) == 0) {
        to = 1;
        animate_hp(d->side, d - B[d->side], from, to);
        bmsg("%s held on for %s!", mon_name(d->m), sv.name);
        return;
    }
    animate_hp(d->side, d - B[d->side], from, to);
}

static void give_status(Bat *t, int st) {
    if (!alive(t) || t->m->status) return;
    t->m->status = st;
    if (st == ST_SLEEP) { t->sleep = 1 + rnd_n(3); bmsg("%s fell into a deep sleep!", pname(t), ""); }
    else if (st == ST_FEAR) { t->fear = 3; bmsg("%s is gripped by fear!", pname(t), ""); }
    else if (st == ST_BURN) bmsg("%s was scorched by hellfire!", pname(t), "");
    else if (st == ST_CURSE) bmsg("%s was cursed!", pname(t), "");
}

static void stat_change(Bat *t, int which, int delta) {
    static const char *const nm[4] = { "ATTACK", "DEFENSE", "SPEED", "SPIRIT" };
    int v = t->stage[which] + delta;
    if (v > 3 || v < -3) { bmsg("%s's %s won't go any further!", pname(t), nm[which]); return; }
    t->stage[which] = v;
    bmsg(delta > 0 ? "%s's %s rose!" : "%s's %s fell!", pname(t), nm[which]);
}

static void faint_check(Bat *t);

static void hit_target(Bat *a, Bat *t, int mv, int power, bool spread) {
    const Move *M = &moves[mv];
    int e4;
    bool crit;
    int dmg = damage_calc(a, t, mv, power, spread, &e4, &crit);
    sfx(e4 > 4 ? SFX_SUPER : e4 < 4 ? SFX_WEAK : SFX_HIT);
    hit_flash(t->side, t - B[t->side]);
    apply_damage(t, dmg);
    if (crit) bmsg("A critical strike!", "", "");
    if (e4 > 4) bmsg("It's super effective!", "", "");
    else if (e4 < 4) bmsg("It's not very effective...", "", "");
    if (a->side == 0) soul_gauge += 8;
    else soul_gauge += 5;
    if (soul_gauge > 100) soul_gauge = 100;
    if (M->effect == EF_DRAIN && alive(a)) {
        int heal = dmg / 2 + 1;
        int mx = mon_maxhp(a->m);
        int to = a->m->hp + heal > mx ? mx : a->m->hp + heal;
        animate_hp(a->side, a - B[a->side], a->m->hp, to);
        bmsg("%s drained life force!", pname(a), "");
    }
    if (M->effect == EF_RECOIL && alive(a)) {
        int r = dmg / 4 + 1;
        int to = a->m->hp > r ? a->m->hp - r : 0;
        animate_hp(a->side, a - B[a->side], a->m->hp, to);
        bmsg("%s is hurt by recoil!", pname(a), "");
    }
    if (alive(t) && M->chance && rnd_n(100) < M->chance) {
        if (M->effect == EF_BURN) give_status(t, ST_BURN);
        else if (M->effect == EF_FEAR) give_status(t, ST_FEAR);
    }
    faint_check(t);
    if (a->active) faint_check(a);
}

static void do_move(Bat *a, int mv, int tside, int tslot) {
    const Move *M = &moves[mv];
    if (!alive(a)) return;
    // status gates
    if (a->m->status == ST_SLEEP) {
        if (a->sleep) a->sleep--;
        if (a->sleep) { bmsg("%s is fast asleep.", pname(a), ""); return; }
        a->m->status = ST_NONE;
        bmsg("%s woke up!", pname(a), "");
    }
    if (a->m->status == ST_FEAR) {
        if (a->fear) a->fear--;
        if (!a->fear) { a->m->status = ST_NONE; bmsg("%s shook off its fear!", pname(a), ""); }
        else if (rnd_n(100) < 30) { bmsg("%s is frozen with fear!", pname(a), ""); return; }
    }
    for (int i = 0; i < 4; i++)
        if (a->m->moves[i] == mv && a->m->pp[i]) a->m->pp[i]--;
    char line[64];
    fmt(line, "%s used %s!", pname(a), M->name);
    msg(line);
    draw_panels();
    lunge(a->side, a - B[a->side]);
    int foe_side = a->side ^ 1;
    if (M->target == TGT_SELF) {
        if (M->effect == EF_ATKUP) stat_change(a, 0, 1);
        if (M->effect == EF_DEFUP) stat_change(a, 1, 1);
        if (M->effect == EF_SPDUP) stat_change(a, 2, 1);
        if (M->effect == EF_SPCUP) stat_change(a, 3, 1);
        return;
    }
    if (M->target == TGT_ALLY) {
        Bat *t = bat(a->side, tslot);
        if (!alive(t)) t = a;
        int mx = mon_maxhp(t->m);
        int to = t->m->hp + mx / 2 > mx ? mx : t->m->hp + mx / 2;
        sfx(SFX_HEAL);
        animate_hp(t->side, t - B[t->side], t->m->hp, to);
        bmsg("%s's wounds closed!", pname(t), "");
        return;
    }
    if (M->target == TGT_ALL_FOES) {
        bool any = false;
        for (int s = 0; s < slots[foe_side]; s++) {
            Bat *t = bat(foe_side, s);
            if (!alive(t)) continue;
            any = true;
            if (rnd_n(100) >= M->acc) { bmsg("%s avoided the attack!", pname(t), ""); continue; }
            if (M->power) hit_target(a, t, mv, M->power, count_alive(foe_side) > 1);
            else if (M->effect == EF_ATKDOWN) stat_change(t, 0, -1);
            if (!alive(a)) return;
        }
        if (!any) bmsg("But there was no target...", "", "");
        return;
    }
    Bat *t = bat(tside, tslot);
    if (!alive(t)) {
        // retarget to any living foe
        t = 0;
        for (int s = 0; s < slots[foe_side]; s++) if (alive(bat(foe_side, s))) { t = bat(foe_side, s); break; }
        if (!t) { bmsg("But there was no target...", "", ""); return; }
    }
    int acc = M->acc;
    if (weather_now == WX_FOG) acc = acc * 9 / 10;
    if (rnd_n(100) >= acc) { bmsg("%s's attack missed!", pname(a), ""); return; }
    if (M->power) { hit_target(a, t, mv, M->power, false); return; }
    switch (M->effect) {
    case EF_SLEEP: if (t->m->status) bmsg("But it failed!", "", ""); else give_status(t, ST_SLEEP); break;
    case EF_CURSE: if (t->m->status) bmsg("But it failed!", "", ""); else give_status(t, ST_CURSE); break;
    case EF_ATKDOWN: stat_change(t, 0, -1); break;
    }
}

static void do_combo(void) {
    int n = 0, power = 0, bond = 0;
    Bat *lead = 0;
    for (int s = 0; s < slots[0]; s++) {
        Bat *b = bat(0, s);
        if (!alive(b)) continue;
        if (!lead) lead = b;
        n++;
        bond += b->m->bond;
    }
    if (!lead || n < 2) return;
    soul_gauge = 0;
    char buf[80];
    fmt(buf, "%s and the team unleash a SOUL COMBO!", sv.name);
    msg(buf);
    sfx(SFX_EVOLVE);
    for (int s = 0; s < slots[0]; s++) if (alive(bat(0, s))) lunge(0, s);
    for (int k = 0; k < 6; k++) { set_brightness(8); bframe(); set_brightness(0); bframe(); }
    power = 35 * n + bond / n / 6;
    for (int s = 0; s < slots[1]; s++) {
        Bat *t = bat(1, s);
        if (alive(t)) hit_target(lead, t, MV_SOULCOMBO, power, false);
    }
}

// ------------------------------------------------------------------ fainting / switching / rewards
static void award_xp(Bat *dead) {
    const Species *s = &species[dead->m->species];
    u32 xp = (u32)s->xp_yield * dead->m->level / 4;
    if (!wild) xp = xp * 3 / 2;
    for (int sl = 0; sl < slots[0]; sl++) {
        Bat *b = bat(0, sl);
        if (!alive(b)) continue;
        char buf[64];
        fmt(buf, "%s gained %d EXP!", mon_name(b->m), (int)xp);
        msg(buf);
        int lv = mon_gain_xp(b->m, xp, true);
        if (lv) leveled[b->idx] = true;
        draw_panels();
    }
}

static void faint_check(Bat *t) {
    if (!t->active || t->m->hp > 0) return;
    sfx(SFX_FAINT);
    for (int k = 0; k < 12; k++) { t->hidden = k & 1; bframe(); }
    t->hidden = true;
    bframe();
    t->active = false;
    t->hidden = false;
    t->m->status = ST_NONE;
    bmsg("%s fainted!", pname(t), "");
    if (t->side == 0) {
        t->m->bond = t->m->bond > 5 ? t->m->bond - 5 : 0;
    } else {
        if (wild) sv.wild_defeated++;
        award_xp(t);
    }
    draw_panels();
}

static void refill_foes(void) {
    for (int s = 0; s < slots[1]; s++) {
        if (B[1][s].active || foe_next >= foe_n) continue;
        place(1, s, &foes[foe_next], foe_next);
        foe_next++;
        draw_panels();
        char buf[64];
        fmt(buf, "%s sends out %s!", trainer->name, mon_name(B[1][s].m));
        msg(buf);
        draw_panels();
    }
}

extern int party_pick_start;
static int pick_bench(const char *prompt, bool cancel) {
    while (1) {
        for (int k = 0; k < sv.party_n; k++)
            if (sv.party[k].hp && !party_active(k)) { party_pick_start = k; break; }
        int i = party_pick(prompt, cancel);
        if (i < 0) return -1;
        if (sv.party[i].hp == 0) { msg("It has no strength left to fight!"); continue; }
        if (party_active(i)) { msg("It's already fighting!"); continue; }
        return i;
    }
}

static void restore_battle_screen(void);

static void refill_player(void) {
    for (int s = 0; s < slots[0]; s++) {
        if (B[0][s].active) continue;
        bool any = false;
        for (int i = 0; i < sv.party_n; i++) if (sv.party[i].hp && !party_active(i)) any = true;
        if (!any) return;
        int i = pick_bench("Send in which Monstermon?", false);
        restore_battle_screen();
        place(0, s, &sv.party[i], i);
        draw_panels();
        bmsg("Go, %s!", mon_name(B[0][s].m), "");
    }
}

static void end_of_turn(void) {
    for (int side = 0; side < 2; side++) {
        for (int s = 0; s < slots[side]; s++) {
            Bat *b = bat(side, s);
            if (!alive(b)) continue;
            int st = b->m->status;
            if (st == ST_BURN || st == ST_CURSE) {
                int mx = mon_maxhp(b->m);
                int d = st == ST_BURN ? mx / 16 + 1 : mx / 8 + 1;
                int to = b->m->hp > d ? b->m->hp - d : 0;
                animate_hp(side, s, b->m->hp, to);
                bmsg(st == ST_BURN ? "%s is hurt by its burn!" : "%s is wracked by the curse!", pname(b), "");
                faint_check(b);
            }
        }
    }
}

// ------------------------------------------------------------------ capture & hunter actions
static int choose_foe(const char *prompt) {
    const char *opts[3];
    int map[3], n = 0;
    char names[3][24];
    for (int s = 0; s < slots[1]; s++) {
        if (!alive(bat(1, s))) continue;
        fmt(names[n], "%s L%d", mon_name(B[1][s].m), B[1][s].m->level);
        opts[n] = names[n];
        map[n++] = s;
    }
    if (n == 1) return map[0];
    if (n == 0) return -1;
    msg_noclose(prompt);
    int r = menu(14, 6, 16, opts, n, 0, true);
    msg_close();
    draw_panels();
    return r < 0 ? -1 : map[r];
}

static int choose_ally(const char *prompt) {
    const char *opts[3];
    int map[3], n = 0;
    for (int s = 0; s < slots[0]; s++) {
        if (!alive(bat(0, s))) continue;
        opts[n] = mon_name(B[0][s].m);
        map[n++] = s;
    }
    if (n == 1) return map[0];
    msg_noclose(prompt);
    int r = menu(14, 6, 16, opts, n, 0, true);
    msg_close();
    draw_panels();
    return r < 0 ? -1 : map[r];
}

static bool try_bind(int item, int slot) {
    Bat *t = bat(1, slot);
    const Species *s = &species[t->m->species];
    sv.items[item]--;
    char buf[96];
    fmt(buf, "%s raises the %s!", sv.name, items[item].name);
    msg(buf);
    sfx(SFX_BIND);
    // shrink into the binding light
    for (int k = 1; k <= 8; k++) { t->scale = k; bframe(); bframe(); }
    const char *fail = 0;
    if (s->capflags & CAP_BOSS) fail = "Its soul is far too vast! The Soulbind shatters!";
    else if ((s->capflags & CAP_NIGHT) && !is_night()) fail = "The spirit slips through the Soulbind like mist! Spirits can only be bound at night.";
    else if ((s->capflags & CAP_RITUAL) && !ritual) fail = "The binding flame gutters out! A RITUAL CANDLE must be lit first.";
    else if ((s->capflags & CAP_TRUST) && !t->trust) fail = "It recoils from the Soulbind. It doesn't trust you yet...";
    if (!fail) {
        int mx = mon_maxhp(t->m);
        int mod = item == IT_GREATBIND ? 15 : item == IT_DUSKBIND ? (is_night() ? 30 : 10) : 10;
        u32 c = (u32)(3 * mx - 2 * t->m->hp) * s->catch_rate * mod / (3 * mx) / 10;
        if (t->m->status) c = c * 3 / 2;
        if (t->marked) c = c * 3 / 2;
        if (c > 255) c = 255;
        int shakes = 0;
        for (int k = 0; k < 3; k++) if ((int)rnd_n(256) < (int)c + 20) shakes++;
        bool ok = rnd_n(256) < (int)c;
        for (int k = 0; k < (ok ? 3 : shakes); k++) {
            for (int j = 0; j < 4; j++) { t->lunge = (j & 1) ? 3 : -3; bframe(); bframe(); bframe(); }
            t->lunge = 0;
            sfx(SFX_CURSOR);
            wait_frames(20);
        }
        if (ok) {
            sfx(SFX_BIND_OK);
            t->hidden = true;
            bframe();
            fmt(buf, "Soulbound! %s is now bonded to %s!", mon_name(t->m), sv.name);
            msg(buf);
            Mon m = *t->m;
            m.bond = 40;
            m.status = ST_NONE;
            bool first = !is_caught(m.species);
            if (!give_mon(&m)) msg("...but there's no room in your team or the Soul Vault!");
            else if (sv.party_n == 6 && sv.party[5].species != m.species) msg("It was sent to the Soul Vault at the Sanctuary.");
            if (first) { fmt(buf, "%s's data was added to the Bestiary.", mon_name(&m)); msg(buf); }
            t->active = false;
            t->hidden = false;
            t->m->hp = 0;
            draw_panels();
            return true;
        }
        fail = shakes >= 2 ? "Argh! So close! It broke free!" : "It broke free of the Soulbind!";
    }
    t->scale = 0;
    bframe();
    msg(fail);
    draw_panels();
    return false;
}

static void hunt_strike(int slot) {
    Bat *t = bat(1, slot);
    const Species *s = &species[t->m->species];
    int pct = 10;
    bool dbl = false;
    switch (sv.weapon) {
    case IT_SILVER: pct = 12; dbl = s->t1 == T_BEAST || s->t1 == T_BLOOD || s->t2 == T_BEAST || s->t2 == T_BLOOD; break;
    case IT_CROSSBOW: pct = 12; dbl = s->t1 == T_SHADE || s->t1 == T_UNDEAD || s->t2 == T_SHADE || s->t2 == T_UNDEAD; break;
    case IT_STAFF: pct = 8; break;
    }
    if (dbl) pct *= 2;
    char buf[80];
    fmt(buf, "%s strikes with the %s!", sv.name, items[sv.weapon ? sv.weapon : IT_KNIFE].name);
    msg(buf);
    int d = mon_maxhp(t->m) * pct / 100 + 1;
    int to = t->m->hp - d < 1 ? 1 : t->m->hp - d;
    sfx(dbl ? SFX_SUPER : SFX_HIT);
    hit_flash(1, slot);
    animate_hp(1, slot, t->m->hp, to);
    if (dbl) msg("The weapon's blessing bites deep!");
    if (sv.weapon == IT_STAFF && !t->marked) {
        t->marked = true;
        bmsg("A binding rune marks %s!", pname(t), "");
    }
    soul_gauge += 5;
    if (soul_gauge > 100) soul_gauge = 100;
}

// ------------------------------------------------------------------ command input
static void show_move_info(Bat *b, int mv, int pp) {
    txt_rect(20, 15, 9, 4, UI_WIN_FILL);
    if (!mv) return;
    const Move *M = &moves[mv];
    if (mv == MV_SOULCOMBO) {
        txt_put(20, 15, "ALL ALLY", 1);
        txt_put(20, 16, "ALL FOES", 0);
        return;
    }
    txt_put(20, 15, type_names[M->type], 1);
    txt_put(20, 16, "PP", 0);
    txt_num(23, 16, pp, 2, 0);
    txt_putc(25, 16, '/', 0);
    txt_num(26, 16, M->pp, 2, 0);
    if (M->power) { txt_put(20, 17, "PWR", 0); txt_num(24, 17, M->power, 3, 0); }
    else txt_put(20, 17, "STATUS", 0);
    // matchup hint vs first living foe
    for (int s = 0; s < slots[1]; s++) {
        Bat *t = bat(1, s);
        if (!alive(t) || !M->power || M->target == TGT_SELF) continue;
        const Species *ds = &species[t->m->species];
        int e = type_mult4(M->type, ds->t1, ds->t2);
        if (is_seen(t->m->species)) txt_put(20, 18, e > 4 ? "STRONG!" : e < 4 ? "WEAK" : "", e > 4 ? 1 : 2);
        break;
    }
    (void)b;
}

static int command_menu(bool combo_ok) {
    static const char *const opt[6] = { "FIGHT", "BAG", "HUNT", "TEAM", "RUN", "COMBO" };
    static const u8 col[3] = { 13, 19, 24 };
    int n = combo_ok ? 6 : 5;
    static int sel;
    if (sel >= n) sel = 0;
    while (1) {
        for (int i = 0; i < 6; i++) {
            int cx = col[i % 3], cy = 15 + (i / 3) * 2;
            if (i >= n) { txt_rect(cx, cy, 6, 1, UI_WIN_FILL); continue; }
            TXT[cy * 32 + cx] = i == sel ? UI_CURSOR : UI_WIN_FILL;
            txt_put(cx + 1, cy, opt[i], i == 5 ? 1 : 0);
        }
        bframe();
        u16 r = key_repeat();
        int ns = sel;
        if (r & KEY_LEFT) ns = sel % 3 ? sel - 1 : sel;
        if (r & KEY_RIGHT) ns = sel % 3 < 2 ? sel + 1 : sel;
        if (r & KEY_UP) ns = sel >= 3 ? sel - 3 : sel;
        if (r & KEY_DOWN) ns = sel < 3 ? sel + 3 : sel;
        if (ns < n && ns != sel) { sel = ns; sfx(SFX_CURSOR); }
        if (key_hit(KEY_A)) { sfx(SFX_OK); int c = sel; if (c == 5) sel = 0; return c; }
    }
}

// returns: 0 = action chosen, -1 = back
static int choose_move(int slot, Act *act, bool *combo) {
    Bat *b = bat(0, slot);
    bool can_combo = false;
    (void)combo;
    if (!b->m->pp[0] && !b->m->pp[1] && !b->m->pp[2] && !b->m->pp[3]) {
        // out of PP: lash out with a basic strike
        bmsg("%s has no energy left for its techniques!", mon_name(b->m), "");
        act->kind = ACT_MOVE; act->side = 0; act->slot = slot; act->move = 20;
        act->tside = 1;
        int t = choose_foe("Target which foe?");
        act->tslot = t < 0 ? 0 : t;
        return 0;
    }
    const char *opts[5];
    char names[5][16];
    int n = 4;
    for (int i = 0; i < 4; i++) {
        int mv = b->m->moves[i];
        fmt(names[i], "%s", mv ? moves[mv].name : "-");
        opts[i] = names[i];
    }
    if (can_combo) { opts[4] = "SOUL COMBO"; n = 5; }
    win_draw(0, 14, 30, 6);
    win_draw(19, 14, 11, 6);
    int sel = 0;
    while (1) {
        // custom loop so the info panel tracks the cursor
        for (int i = 0; i < n; i++) {
            int cx = 1 + (i & 1) * 9, cy = 15 + (i >> 1) * 2;
            TXT[cy * 32 + cx] = (i == sel ? UI_CURSOR : UI_WIN_FILL);
            txt_put(cx + 1, cy, opts[i], i == 4 ? 1 : (b->m->moves[i] && b->m->pp[i] ? 0 : 2));
        }
        int mv = sel == 4 ? MV_SOULCOMBO : b->m->moves[sel];
        show_move_info(b, mv, sel < 4 ? b->m->pp[sel] : 1);
        bframe();
        u16 r = key_repeat();
        int ns = sel;
        if (r & KEY_LEFT) ns = sel & ~1;
        if (r & KEY_RIGHT) ns = sel | 1;
        if (r & KEY_UP) ns = sel - 2;
        if (r & KEY_DOWN) ns = sel + 2;
        if (ns >= 0 && ns < n && ns != sel) { sel = ns; sfx(SFX_CURSOR); }
        if (key_hit(KEY_B)) { sfx(SFX_BACK); win_clear(0, 14, 30, 6); return -1; }
        if (key_hit(KEY_A)) {
            if (sel == 4) { sfx(SFX_OK); *combo = true; win_clear(0, 14, 30, 6); return 0; }
            if (!mv) continue;
            if (!b->m->pp[sel]) { sfx(SFX_BUMP); continue; }
            sfx(SFX_OK);
            break;
        }
    }
    win_clear(0, 14, 30, 6);
    int mv = b->m->moves[sel];
    act->kind = ACT_MOVE;
    act->side = 0;
    act->slot = slot;
    act->move = mv;
    act->tside = 1;
    act->tslot = 0;
    const Move *M = &moves[mv];
    if (M->target == TGT_FOE) {
        int t = choose_foe("Target which foe?");
        if (t < 0) return -1;
        act->tslot = t;
    } else if (M->target == TGT_ALLY) {
        int t = choose_ally("Heal which ally?");
        if (t < 0) return -1;
        act->tside = 0;
        act->tslot = t;
    }
    return 0;
}

static void enemy_choose(int slot, Act *act) {
    Bat *b = bat(1, slot);
    int best = -1, best_score = -1, best_t = 0;
    for (int i = 0; i < 4; i++) {
        int mv = b->m->moves[i];
        if (!mv || !b->m->pp[i]) continue;
        const Move *M = &moves[mv];
        int t = 0, score;
        // pick a random living target
        int tries = 0;
        do { t = rnd_n(slots[0]); } while (!alive(bat(0, t)) && ++tries < 8);
        if (!alive(bat(0, t))) for (int s = 0; s < slots[0]; s++) if (alive(bat(0, s))) t = s;
        if (M->power) {
            const Species *ds = &species[B[0][t].m->species];
            score = M->power * type_mult4(M->type, ds->t1, ds->t2) / 4;
            const Species *as = &species[b->m->species];
            if (as->t1 == M->type || as->t2 == M->type) score = score * 3 / 2;
        } else if (M->target == TGT_ALLY) {
            score = b->m->hp * 2 < mon_maxhp(b->m) ? 90 : 5;
        } else if (M->target == TGT_SELF) {
            score = 35;
        } else {
            score = B[0][t].m->status ? 5 : 45;
        }
        score += rnd_n(40);
        if (score > best_score) { best_score = score; best = mv; best_t = t; }
    }
    act->kind = ACT_MOVE;
    act->side = 1;
    act->slot = slot;
    act->move = best > 0 ? best : 20;
    act->tside = moves[act->move].target == TGT_ALLY ? 1 : 0;
    act->tslot = moves[act->move].target == TGT_ALLY ? slot : best_t;
}

static int speed_of(const Act *a) {
    Bat *b = bat(a->side, a->slot);
    if (!b->active) return -1;
    int s = stage_apply(mon_stat(b->m, 2), b->stage[2]);
    if (b->m->status == ST_FEAR) s /= 2;
    return s * 4 + rnd_n(4);
}

// ------------------------------------------------------------------ battle screen setup
static int scene_for_map(void) { return 1 + map_defs[sv.map].area; }

static void restore_battle_screen(void) {
    REG_DISPCNT = DCNT_BLANK;
    const SceneDef *s = &scene_defs[scene_for_map()];
    dma_copy32(CHARBLOCK(CB_SCENE), s->tiles, s->ntiles * 8);
    vu16 *sb = SCREENBLOCK(SB_MAP);
    dma_fill32(CHARBLOCK(CB_SCENE) + 511 * 16, 0, 8);
    for (int i = 0; i < 32 * 32; i++) sb[i] = 511 | (BANK_SCENE << 12);
    for (int y = 0; y < s->h; y++)
        for (int x = 0; x < s->w; x++) sb[y * 32 + x] = s->map[y * s->w + x] | (BANK_SCENE << 12);
    REG_BGCNT(1) = BG_PRIO(3) | BG_CBB(CB_SCENE) | BG_SBB(SB_MAP);
    REG_BGHOFS(1) = 0;
    REG_BGVOFS(1) = 0;
    tint_enabled = map_is_outdoor(sv.map);
    world_obj_pals = false;
    pal_refresh();
    if (tint_enabled) scene_palette_tinted(s->pal, BANK_SCENE);
    else for (int i = 0; i < 16; i++) PAL_BG[BANK_SCENE * 16 + i] = s->pal[i];
    PAL_BG[0] = RGB15(1, 0, 3);
    txt_clear();
    oam_clear();
    for (int side = 0; side < 2; side++)
        for (int sl = 0; sl < slots[side]; sl++)
            if (B[side][sl].active) load_slot(side, sl);
    set_brightness(0);
    REG_BLDCNT = 0;
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ;
    draw_panels();
    draw_sprites();
}

static int run(void) {
    msg_auto = 50;
    memset(B, 0, sizeof(B));
    memset(participated, 0, sizeof(participated));
    memset(leveled, 0, sizeof(leveled));
    soul_gauge = 0;
    ritual = false;
    // player slots: first up to 3 healthy team members (trainer max_active also caps)
    int maxp = trainer ? trainer->max_active : (slots[1] == 1 ? 1 : 2);
    if (maxp < 1) maxp = 1;
    if (!trainer && slots[1] == 1) maxp = 1;
    if (wild && party_alive_count() >= 2 && slots[1] >= 2) maxp = 2;
    slots[0] = 0;
    for (int i = 0; i < sv.party_n && slots[0] < maxp; i++) {
        if (sv.party[i].hp) { slots[0]++; }
    }
    if (slots[0] == 0) return BR_LOSE;
    restore_battle_screen();
    int k = 0;
    for (int i = 0; i < sv.party_n && k < slots[0]; i++)
        if (sv.party[i].hp) place(0, k++, &sv.party[i], i);
    for (int s = 0; s < slots[1]; s++) {
        place(1, s, &foes[s], s);
    }
    foe_next = slots[1];
    // intro
    for (int s = 0; s < 2; s++) for (int sl = 0; sl < slots[s]; sl++) B[s][sl].hidden = true;
    draw_sprites();
    fade_from_black(12);
    bgm(boss ? BGM_BOSS : BGM_BATTLE);
    for (int sl = 0; sl < slots[1]; sl++) {
        B[1][sl].hidden = false;
        for (int k2 = 8; k2 >= 0; k2--) { B[1][sl].scale = k2; bframe(); }
    }
    draw_panels();
    char buf[96];
    if (wild) {
        if (slots[1] == 2) fmt(buf, "A pair of wild %s and %s emerged from the dark!", mon_name(&foes[0]), mon_name(&foes[1]));
        else if (boss) fmt(buf, "%s blocks the way!", mon_name(&foes[0]));
        else fmt(buf, "A wild %s emerged from the dark!", mon_name(&foes[0]));
        msg(buf);
        for (int sl = 0; sl < slots[1]; sl++)
            if (foes[sl].variant) { sfx(SFX_ITEM); fmt(buf, "Whoa! %s glimmers with ECLIPSE colors!", mon_name(&foes[sl])); msg(buf); }
    } else {
        fmt(buf, "%s wants to battle!", trainer->name);
        msg(buf);
    }
    for (int sl = 0; sl < slots[0]; sl++) {
        B[0][sl].hidden = false;
        for (int k2 = 8; k2 >= 0; k2--) { B[0][sl].scale = k2; bframe(); }
    }
    if (slots[0] == 1) fmt(buf, "Go, %s!", mon_name(B[0][0].m));
    else if (slots[0] == 2) fmt(buf, "Go, %s and %s!", mon_name(B[0][0].m), mon_name(B[0][1].m));
    else fmt(buf, "Go, %s, %s and %s!", mon_name(B[0][0].m), mon_name(B[0][1].m), mon_name(B[0][2].m));
    msg(buf);
    draw_panels();

    Act acts[6];
    while (1) {
        // --- command phase
        int nact = 0;
        bool turn_used = false;
        while (1) {
            draw_panels();
            win_draw(0, 14, 30, 6);
            Bat *lead = 0;
            for (int s = 0; s < slots[0]; s++) if (alive(bat(0, s))) { lead = bat(0, s); break; }
            txt_put(1, 15, "What will", 0);
            fmt(buf, "%s do?", sv.name);
            txt_put(1, 16, buf, 0);
            if (lead && soul_gauge >= 100 && count_alive(0) >= 2) txt_put(1, 18, "COMBO READY", 1);
            bool combo_ok = lead && soul_gauge >= 100 && count_alive(0) >= 2;
            win_draw(12, 14, 18, 6);
            int c = command_menu(combo_ok);
            win_clear(0, 14, 30, 6);
            if (c == 5) {
                acts[0].kind = ACT_COMBO;
                acts[0].side = 0;
                acts[0].slot = lead - B[0];
                nact = 1;
                break;
            }
            if (c == 0) {
                // FIGHT: a move per living ally
                nact = 0;
                bool combo = false;
                int s = 0;
                while (s < slots[0]) {
                    if (!alive(bat(0, s))) { s++; continue; }
                    combo = false;
                    int r = choose_move(s, &acts[nact], &combo);
                    if (r < 0) {
                        // step back to the previous ally, or the main menu
                        if (nact == 0) break;
                        nact--;
                        s = acts[nact].slot;
                        continue;
                    }
                    if (combo) { acts[0].kind = ACT_COMBO; acts[0].side = 0; acts[0].slot = s; nact = 1; break; }
                    nact++;
                    s++;
                }
                if (nact > 0) break;
                continue;
            }
            if (c == 1) {
                int item = 0;
                bag_screen(true, &item);
                restore_battle_screen();
                if (!item) continue;
                const Item *it = &items[item];
                if (it->kind == IK_BIND) {
                    if (!wild) { msg("You can't Soulbind another hunter's partner!"); continue; }
                    int t = choose_foe("Bind which Monstermon?");
                    if (t < 0) continue;
                    if (try_bind(item, t)) {
                        if (count_alive(1) == 0) {
                            last_caught_slot = sv.party_n - 1;
                            return BR_CAUGHT;
                        }
                    }
                    turn_used = true;
                    break;
                }
                if (item == IT_CANDLE) {
                    sv.items[item]--;
                    ritual = true;
                    msg("The Ritual Candle flares with violet flame. Ritual-bound spirits can now be Soulbound!");
                    turn_used = true;
                    break;
                }
                if (item == IT_OFFERING) {
                    int t = choose_foe("Offer it to which Monstermon?");
                    if (t < 0) continue;
                    sv.items[item]--;
                    B[1][t].trust = true;
                    bmsg("%s accepts the Offering... It seems to trust you a little.", pname(bat(1, t)), "");
                    turn_used = true;
                    break;
                }
                if (it->kind == IK_HEAL) {
                    int i = party_pick("Use on which Monstermon?", true);
                    restore_battle_screen();
                    if (i < 0) continue;
                    Mon *m = &sv.party[i];
                    int mx = mon_maxhp(m);
                    if (item == IT_REVIVE) {
                        if (m->hp) { msg("It isn't fainted."); continue; }
                        m->hp = mx / 2;
                    } else if (item == IT_SALT) {
                        if (!m->status) { msg("It has no ailment."); continue; }
                        m->status = ST_NONE;
                        for (int s = 0; s < slots[0]; s++) if (B[0][s].m == m) B[0][s].fear = B[0][s].sleep = 0;
                    } else {
                        if (!m->hp) { msg("It has fainted - use REVIVE ASH."); continue; }
                        if (m->hp >= mx && !(item == IT_ELIXIR && m->status)) { msg("It's already at full health."); continue; }
                        m->hp = item == IT_ELIXIR ? mx : (m->hp + 30 > mx ? mx : m->hp + 30);
                        if (item == IT_ELIXIR) m->status = ST_NONE;
                    }
                    sv.items[item]--;
                    sfx(SFX_HEAL);
                    draw_panels();
                    msgf("%s used a %s on %s.", sv.name, it->name, mon_name(m));
                    turn_used = true;
                    break;
                }
                msg("That can't be used here.");
                continue;
            }
            if (c == 2) {
                if (!wild) { msg("A hunter never strikes another hunter's partner."); continue; }
                int t = choose_foe("Strike which Monstermon?");
                if (t < 0) continue;
                hunt_strike(t);
                turn_used = true;
                break;
            }
            if (c == 3) {
                int i = pick_bench("Swap in which Monstermon?", true);
                restore_battle_screen();
                if (i < 0) continue;
                int target = 0;
                if (slots[0] > 1) {
                    const char *opts[3];
                    for (int s = 0; s < slots[0]; s++) opts[s] = B[0][s].active ? mon_name(B[0][s].m) : "(empty)";
                    msg_noclose("Swap out which ally?");
                    target = menu(14, 6, 16, opts, slots[0], 0, true);
                    msg_close();
                    if (target < 0) continue;
                }
                if (B[0][target].active) bmsg("Come back, %s!", mon_name(B[0][target].m), "");
                place(0, target, &sv.party[i], i);
                draw_panels();
                bmsg("Go, %s!", mon_name(B[0][target].m), "");
                turn_used = true;
                break;
            }
            if (c == 4) {
                if (!wild) { msg("There's no running from a hunter's duel!"); continue; }
                if (boss) { msg("Its presence holds you in place! You can't escape!"); continue; }
                int my = 0, their = 0;
                for (int s = 0; s < slots[0]; s++) if (alive(bat(0, s))) my = mon_stat(B[0][s].m, 2) > my ? mon_stat(B[0][s].m, 2) : my;
                for (int s = 0; s < slots[1]; s++) if (alive(bat(1, s))) their = mon_stat(B[1][s].m, 2) > their ? mon_stat(B[1][s].m, 2) : their;
                if (rnd_n(100) < 60 + (my - their) * 2) {
                    sfx(SFX_DOOR);
                    msg("You slipped away into the night!");
                    return BR_FLED;
                }
                msg("Couldn't get away!");
                turn_used = true;
                break;
            }
        }
        // --- enemy actions
        int n_total = turn_used ? 0 : nact;
        for (int s = 0; s < slots[1]; s++)
            if (alive(bat(1, s))) enemy_choose(s, &acts[n_total++]);
        // --- order by speed (combo always first)
        int spd[6];
        for (int i = 0; i < n_total; i++) spd[i] = acts[i].kind == ACT_COMBO ? 100000 : speed_of(&acts[i]);
        for (int i = 0; i < n_total; i++)
            for (int j = i + 1; j < n_total; j++)
                if (spd[j] > spd[i]) {
                    int ts = spd[i]; spd[i] = spd[j]; spd[j] = ts;
                    Act ta = acts[i]; acts[i] = acts[j]; acts[j] = ta;
                }
        for (int i = 0; i < n_total; i++) {
            Act *a = &acts[i];
            if (a->kind == ACT_COMBO) do_combo();
            else if (a->kind == ACT_MOVE) do_move(bat(a->side, a->slot), a->move, a->tside, a->tslot);
            if (count_alive(1) == 0 || party_alive_count() == 0) break;
        }
        if (count_alive(1) > 0 && party_alive_count() > 0) end_of_turn();
        // --- reinforcements / outcome
        if (!wild) refill_foes();
        if (count_alive(1) == 0) return BR_WIN;
        if (party_alive_count() == 0) return BR_LOSE;
        refill_player();
    }
}

static void finish(int result) {
    msg_auto = 0;
    oam_clear();
    for (int i = 0; i < sv.party_n; i++)
        if (participated[i]) {
            Mon *m = &sv.party[i];
            int bonus = m->nature == NAT_LOYAL ? 4 : 2;
            m->bond = m->bond + bonus > 255 ? 255 : m->bond + bonus;
        }
    if (result == BR_WIN && trainer) {
        bgm(BGM_VICTORY);
        char buf[96];
        fmt(buf, "You defeated %s!", trainer->name);
        msg(buf);
        if (trainer->lose) say(trainer->name, trainer->lose);
        if (trainer->prize) {
            sv.money += trainer->prize;
            fmt(buf, "%s earned $%d for winning!", sv.name, trainer->prize);
            msg(buf);
        }
    } else if (result == BR_WIN && wild && !boss) {
        bgm(BGM_VICTORY);
    }
    // transformations after battle
    if (result != BR_LOSE) {
        for (int i = 0; i < sv.party_n; i++)
            if (leveled[i] && sv.party[i].hp) mon_try_evolve(&sv.party[i]);
    }
    for (int i = 0; i < sv.party_n; i++)
        if (sv.party[i].status == ST_SLEEP || sv.party[i].status == ST_FEAR) sv.party[i].status = ST_NONE;
    fade_to_black(16);
    txt_clear();
    oam_clear();
    bgm_stop();
}

static void make_wild(Mon *m, int sp, int lv) {
    mon_create(m, sp, lv);
    int odds = 64;
    if (sv.tool == IT_LENS) odds /= 3;
    if (weather_now == WX_BLOODMOON) odds /= 4;
    if (rnd_n(odds) == 0) { m->variant = 1; m->hp = mon_maxhp(m); }
}

int battle_wild(int sp1, int lv1, int sp2, int lv2, bool is_boss) {
    trainer = 0;
    wild = true;
    boss = is_boss;
    make_wild(&foes[0], sp1, lv1);
    foe_n = 1;
    if (sp2 >= 0 && !is_boss) { make_wild(&foes[1], sp2, lv2); foe_n = 2; }
    if (is_boss) foes[0].variant = 0, foes[0].hp = mon_maxhp(&foes[0]);
    slots[1] = foe_n;
    int r = run();
    finish(r);
    return r;
}

int battle_trainer(const Trainer *t) {
    trainer = t;
    wild = false;
    boss = t->boss;
    foe_n = t->n;
    for (int i = 0; i < t->n; i++) {
        mon_create(&foes[i], t->sp[i], t->lv[i]);
        foes[i].nature = NAT_BRAVE;
    }
    slots[1] = t->n < t->max_active ? t->n : t->max_active;
    int r = run();
    finish(r);
    return r;
}
