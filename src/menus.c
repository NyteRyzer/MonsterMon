// MonsterMons - menus: team, summary, bag, bestiary, hunter card, vault, shop, bounties
#include <string.h>
#include "core.h"

#define TXT SCREENBLOCK(SB_TEXT)
#define FILL ((u16)UI_WIN_FILL)

static void screen_begin(const char *title) {
    world_obj_pals = false;
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_OBJ;
    REG_BLDCNT = 0;
    REG_BLDY = 0;
    oam_clear();
    txt_fill(FILL);
    PAL_BG[0] = RGB15(2, 1, 4);
    if (title) {
        win_draw(0, 0, 30, 3);
        txt_put(2, 1, title, 1);
    }
}

static void icon(int oam, int x, int y, const Mon *m, int slot) {
    dma_copy32(OBJ_TILES + (448 + slot * 4) * 16, mon_icon_tiles + m->species * 4 * 8, 4 * 8);
    const u16 *p = (m->variant ? mon_vpals : mon_pals) + m->species * 16;
    for (int i = 0; i < 16; i++) PAL_OBJ[(10 + slot) * 16 + i] = p[i];
    spr(oam, x, y, 0, 1, 448 + slot * 4, 10 + slot, 0, false);
}

// ------------------------------------------------------------------ party
static void draw_party(const char *prompt, int sel) {
    txt_fill(FILL);
    win_draw(0, 0, 30, 3);
    txt_put(2, 1, prompt, 1);
    for (int i = 0; i < 6; i++) {
        int y = 3 + i * 3;
        if (i >= sv.party_n) { spr(10 + i, 240, 160, 0, 0, 0, 0, 0, false); continue; }
        Mon *m = &sv.party[i];
        TXT[(y + 1) * 32 + 0] = i == sel ? UI_CURSOR : FILL;
        icon(10 + i, 12, y * 8 + 2 + ((i == sel && (frame_count >> 3) & 1) ? -2 : 0), m, i);
        txt_put(5, y, mon_name(m), m->hp ? 0 : 2);
        if (m->variant) txt_putc(15, y, '*', 1);
        txt_putc(17, y, 'L', 1);
        txt_num(18, y, m->level, 0, 1);
        if (m->status) txt_put(22, y, status_names[m->status], 3);
        if (!m->hp) txt_put(22, y, "KO", 3);
        hp_bar(5, y + 1, 7, m->hp, mon_maxhp(m));
        txt_num(15, y + 1, m->hp, 3, 0);
        txt_putc(18, y + 1, '/', 0);
        txt_num(19, y + 1, mon_maxhp(m), 0, 0);
        for (int h = 0; h < 5; h++) TXT[(y + 1) * 32 + 23 + h] = (m->bond > h * 51) ? (UI_IC_HEART | (0 << 12)) : FILL;
    }
}

int party_pick_start;
int party_pick(const char *prompt, bool allow_cancel) {
    screen_begin(0);
    int sel = party_pick_start < sv.party_n ? party_pick_start : 0;
    party_pick_start = 0;
    while (1) {
        draw_party(prompt, sel);
        frame();
        u16 r = key_repeat();
        if (r & KEY_UP) { sel = (sel + sv.party_n - 1) % sv.party_n; sfx(SFX_CURSOR); }
        if (r & KEY_DOWN) { sel = (sel + 1) % sv.party_n; sfx(SFX_CURSOR); }
        if (key_hit(KEY_A)) { sfx(SFX_OK); oam_clear(); return sel; }
        if (allow_cancel && key_hit(KEY_B)) { sfx(SFX_BACK); oam_clear(); return -1; }
    }
}

void party_screen(void) {
    int swap_from = -1;
    screen_begin(0);
    int sel = 0;
    while (1) {
        draw_party(swap_from >= 0 ? "Swap with which?" : "TEAM - A: options  B: back", sel);
        frame();
        u16 r = key_repeat();
        if (r & KEY_UP) { sel = (sel + sv.party_n - 1) % sv.party_n; sfx(SFX_CURSOR); }
        if (r & KEY_DOWN) { sel = (sel + 1) % sv.party_n; sfx(SFX_CURSOR); }
        if (key_hit(KEY_B)) {
            sfx(SFX_BACK);
            if (swap_from >= 0) { swap_from = -1; continue; }
            oam_clear();
            return;
        }
        if (key_hit(KEY_A)) {
            sfx(SFX_OK);
            if (swap_from >= 0) {
                Mon t = sv.party[sel];
                sv.party[sel] = sv.party[swap_from];
                sv.party[swap_from] = t;
                swap_from = -1;
                continue;
            }
            static const char *const opts[3] = { "SUMMARY", "SWAP", "CANCEL" };
            int c = menu(19, 3 + sel * 3 > 12 ? 12 : 3 + sel * 3, 11, opts, 3, 0, true);
            if (c == 0) { mon_summary(&sv.party[sel]); screen_begin(0); }
            else if (c == 1) swap_from = sel;
        }
    }
}

// ------------------------------------------------------------------ summary
void mon_summary(Mon *m) {
    int page = 0;
    while (1) {
        screen_begin(0);
        show_mon_preview(m->species, m->variant, 4, 8, 0, true);
        const Species *s = &species[m->species];
        win_draw(9, 0, 21, 3);
        txt_put(10, 1, s->name, 0);
        txt_putc(21, 1, 'L', 1);
        txt_num(22, 1, m->level, 0, 1);
        if (m->variant) txt_put(25, 1, "ECL", 1);
        txt_put(10, 4, type_names[s->t1], 1);
        if (s->t2 != T_NONE) { txt_putc(18, 4, '/', 0); txt_put(19, 4, type_names[s->t2], 1); }
        if (page == 0) {
            txt_put(10, 6, "NATURE", 2);
            txt_put(17, 6, nature_names[m->nature], 0);
            txt_put(10, 7, "BOND", 2);
            for (int h = 0; h < 5; h++) TXT[7 * 32 + 17 + h] = m->bond > h * 51 ? UI_IC_HEART : (u16)('.' - 32);
            static const char *const st[4] = { "ATTACK", "DEFENSE", "SPEED", "SPIRIT" };
            txt_put(1, 10, "HP", 2);
            txt_num(8, 10, m->hp, 3, 0);
            txt_putc(11, 10, '/', 0);
            txt_num(12, 10, mon_maxhp(m), 0, 0);
            for (int i = 0; i < 4; i++) {
                txt_put(1, 11 + i, st[i], 2);
                txt_num(9, 11 + i, mon_stat(m, i), 3, 0);
            }
            txt_put(1, 16, "NEXT LV", 2);
            txt_num(9, 16, m->level < 100 ? (int)(xp_for_level(m->level + 1) - m->xp) : 0, 5, 0);
            win_draw(15, 9, 15, 10);
            txt_put(16, 10, "MOVES", 1);
            for (int i = 0; i < 4; i++) {
                int mv = m->moves[i];
                if (!mv) continue;
                txt_put(16, 11 + i * 2, moves[mv].name, 0);
                txt_put(17, 12 + i * 2, type_names[moves[mv].type], 2);
                txt_num(25, 12 + i * 2, m->pp[i], 2, 0);
                txt_putc(27, 12 + i * 2, '/', 0);
                txt_num(28, 12 + i * 2, moves[mv].pp, 0, 0);
            }
            if (m->status) txt_put(10, 8, status_names[m->status], 3);
        } else {
            win_draw(0, 10, 30, 10);
            char buf[200];
            fmt(buf, "%s", s->lore);
            // simple wrap at 28 columns
            int x = 1, y = 11;
            const char *p = buf;
            while (*p) {
                const char *w = p;
                int len = 0;
                while (w[len] && w[len] != ' ') len++;
                if (x + len > 29) { x = 1; y++; }
                for (int i = 0; i < len; i++) txt_putc(x++, y, w[i], 0);
                p += len;
                if (*p == ' ') { p++; x++; }
            }
            txt_put(1, 17, "HABITAT:", 2);
            txt_put(1, 18, s->habitat, 1);
        }
        txt_put(1, 19, page == 0 ? "A/RIGHT: LORE   B: BACK" : "LEFT: STATS   B: BACK", 2);
        while (1) {
            frame();
            if (key_hit(KEY_B)) { sfx(SFX_BACK); oam_clear(); return; }
            if ((key_hit(KEY_A) || key_hit(KEY_RIGHT)) && page == 0) { page = 1; sfx(SFX_CURSOR); break; }
            if (key_hit(KEY_LEFT) && page == 1) { page = 0; sfx(SFX_CURSOR); break; }
        }
    }
}

// ------------------------------------------------------------------ bag
static bool in_battle_ok(int it) {
    int k = items[it].kind;
    return k == IK_BIND || k == IK_HEAL || k == IK_BATTLE;
}

static void use_field_item(int it) {
    const Item *I = &items[it];
    if (I->kind == IK_TOOL) {
        if (sv.tool == it) { sv.tool = 0; msgf("Unequipped the %s.", I->name); }
        else { sv.tool = it; msgf("Equipped the %s as your tool.", I->name); }
        return;
    }
    if (I->kind == IK_WEAPON) { sv.weapon = it; msgf("You ready the %s.", I->name); return; }
    if (I->kind == IK_BIND || I->kind == IK_BATTLE) { msg("That can only be used in battle."); return; }
    if (I->kind == IK_KEY) { msg(I->desc); return; }
    int i = party_pick("Use on which Monstermon?", true);
    screen_begin(0);
    if (i < 0) return;
    Mon *m = &sv.party[i];
    int mx = mon_maxhp(m);
    if (it == IT_TREAT) {
        if (m->bond >= 255) { msg("It's already completely devoted to you!"); return; }
        m->bond = m->bond + 40 > 255 ? 255 : m->bond + 40;
        sv.items[it]--;
        sfx(SFX_HEAL);
        msgf("%s happily gobbled the treat! Its bond deepened.", mon_name(m));
        if (mon_evolve_target(m) >= 0) mon_try_evolve(m);
        return;
    }
    if (it == IT_REVIVE) {
        if (m->hp) { msg("It isn't fainted."); return; }
        m->hp = mx / 2;
    } else if (it == IT_SALT) {
        if (!m->status) { msg("It has no ailment."); return; }
        m->status = ST_NONE;
    } else {
        if (!m->hp) { msg("It has fainted. Use REVIVE ASH."); return; }
        if (m->hp >= mx && !(it == IT_ELIXIR && m->status)) { msg("It's already at full health."); return; }
        m->hp = it == IT_ELIXIR ? mx : (m->hp + 30 > mx ? mx : m->hp + 30);
        if (it == IT_ELIXIR) m->status = ST_NONE;
    }
    sv.items[it]--;
    sfx(SFX_HEAL);
    msgf("Used the %s on %s.", I->name, mon_name(m));
}

void bag_screen(bool in_battle, int *out_item) {
    int sel = 0, top = 0;
    if (out_item) *out_item = 0;
    while (1) {
        u8 list[IT_COUNT];
        int n = 0;
        for (int i = 1; i < IT_COUNT; i++)
            if (sv.items[i] && (!in_battle || in_battle_ok(i))) list[n++] = i;
        screen_begin("BAG");
        txt_put(20, 1, "$", 1);
        txt_num(21, 1, sv.money, 7, 0);
        if (!n) {
            msg(in_battle ? "You have nothing useful for battle." : "Your bag is empty.");
            return;
        }
        if (sel >= n) sel = n - 1;
        while (1) {
            if (sel < top) top = sel;
            if (sel >= top + 11) top = sel - 10;
            txt_rect(0, 3, 30, 11, FILL);
            for (int r = 0; r < 11 && top + r < n; r++) {
                int it = list[top + r];
                TXT[(3 + r) * 32 + 1] = top + r == sel ? UI_CURSOR : FILL;
                txt_put(2, 3 + r, items[it].name, it == sv.tool || it == sv.weapon ? 1 : 0);
                if (it == sv.tool || it == sv.weapon) txt_put(17, 3 + r, "EQ", 1);
                txt_putc(23, 3 + r, 'x', 2);
                txt_num(24, 3 + r, sv.items[it], 2, 0);
            }
            win_draw(0, 14, 30, 6);
            const char *d = items[list[sel]].desc;
            int x = 1, y = 15;
            while (*d) {
                int len = 0;
                while (d[len] && d[len] != ' ') len++;
                if (x + len > 29) { x = 1; y++; }
                for (int i = 0; i < len; i++) txt_putc(x++, y, d[i], 0);
                d += len;
                if (*d == ' ') { d++; x++; }
            }
            frame();
            u16 r = key_repeat();
            if (r & KEY_UP) { sel = (sel + n - 1) % n; sfx(SFX_CURSOR); }
            if (r & KEY_DOWN) { sel = (sel + 1) % n; sfx(SFX_CURSOR); }
            if (key_hit(KEY_B)) { sfx(SFX_BACK); return; }
            if (key_hit(KEY_A)) {
                sfx(SFX_OK);
                if (in_battle) { *out_item = list[sel]; return; }
                use_field_item(list[sel]);
                break;
            }
        }
    }
}

// ------------------------------------------------------------------ bestiary
void bestiary_screen(void) {
    int sel = 0, top = 0;
    screen_begin("BESTIARY");
    int seen = 0;
    for (int i = 0; i < SPECIES_COUNT; i++) seen += is_seen(i);
    txt_put(13, 1, "SEEN", 2);
    txt_num(18, 1, seen, 2, 0);
    txt_put(21, 1, "BOUND", 2);
    txt_num(27, 1, caught_count(), 2, 0);
    while (1) {
        if (sel < top) top = sel;
        if (sel >= top + 16) top = sel - 15;
        txt_rect(0, 3, 17, 17, FILL);
        for (int r = 0; r < 16 && top + r < SPECIES_COUNT; r++) {
            int i = top + r;
            TXT[(3 + r) * 32 + 0] = i == sel ? UI_CURSOR : FILL;
            txt_num(1, 3 + r, i + 1, 2, 2);
            txt_put(4, 3 + r, is_seen(i) ? species[i].name : "??????????", is_seen(i) ? 0 : 2);
            if (is_caught(i)) TXT[(3 + r) * 32 + 15] = UI_IC_BALL;
        }
        win_draw(17, 3, 13, 17);
        txt_rect(18, 4, 11, 15, FILL);
        if (is_seen(sel)) {
            show_mon_preview(sel, 0, 168, 36, 0, true);
            txt_put(18, 13, type_names[species[sel].t1], 1);
            if (species[sel].t2 != T_NONE) txt_put(18, 14, type_names[species[sel].t2], 1);
            txt_put(18, 16, is_caught(sel) ? "BOUND" : "SEEN", is_caught(sel) ? 1 : 2);
            if (is_caught(sel)) txt_put(18, 17, "A: LORE", 2);
        } else {
            spr(100, 240, 160, 0, 0, 0, 0, 0, false);
            txt_put(18, 9, "UNKNOWN", 2);
        }
        frame();
        u16 r = key_repeat();
        if (r & KEY_UP) { sel = (sel + SPECIES_COUNT - 1) % SPECIES_COUNT; sfx(SFX_CURSOR); }
        if (r & KEY_DOWN) { sel = (sel + 1) % SPECIES_COUNT; sfx(SFX_CURSOR); }
        if (r & KEY_LEFT) { sel = sel >= 8 ? sel - 8 : 0; sfx(SFX_CURSOR); }
        if (r & KEY_RIGHT) { sel = sel + 8 < SPECIES_COUNT ? sel + 8 : SPECIES_COUNT - 1; sfx(SFX_CURSOR); }
        if (key_hit(KEY_B)) { sfx(SFX_BACK); oam_clear(); return; }
        if (key_hit(KEY_A) && is_caught(sel)) {
            msg(species[sel].lore);
            msgf("Habitat: %s", species[sel].habitat);
            screen_begin("BESTIARY");
            txt_put(13, 1, "SEEN", 2);
            txt_num(18, 1, seen, 2, 0);
            txt_put(21, 1, "BOUND", 2);
            txt_num(27, 1, caught_count(), 2, 0);
        }
    }
}

// ------------------------------------------------------------------ hunter card
void hunter_card(void) {
    screen_begin("HUNTER CARD");
    load_player_palette();
    win_draw(0, 3, 30, 17);
    aff_set(0, 0x80, 0, 0, 0x80);
    spr_affine(0, 8, 32, 0, 1, PS_PLAYER * 36, PS_PLAYER, 0, 0, true);
    txt_put(7, 5, "NAME", 2);
    txt_put(13, 5, sv.name, 0);
    txt_put(7, 6, "FUNDS", 2);
    txt_putc(13, 6, '$', 1);
    txt_num(14, 6, sv.money, 0, 0);
    txt_put(7, 7, "RANK", 2);
    txt_put(13, 7, flag(F_GAME_CLEAR) ? "LEGEND" : flag(F_LICENSE) ? "LICENSED" : "ROOKIE", 1);
    u32 secs = sv.play_frames / 60;
    txt_put(2, 10, "PLAY TIME", 2);
    txt_num(14, 10, secs / 3600, 2, 0);
    txt_putc(16, 10, ':', 0);
    txt_num(17, 10, (secs / 60) % 60 / 10, 0, 0);
    txt_num(18, 10, (secs / 60) % 10, 0, 0);
    txt_put(2, 11, "CLOCK", 2);
    txt_num(13, 11, sv.clock / 60, 2, 0);
    txt_putc(15, 11, ':', 0);
    txt_num(16, 11, sv.clock % 60 / 10, 0, 0);
    txt_num(17, 11, sv.clock % 10, 0, 0);
    txt_put(19, 11, is_night() ? "NIGHT" : "DAY", 1);
    txt_put(2, 12, "BESTIARY", 2);
    txt_num(13, 12, caught_count(), 0, 0);
    txt_put(16, 12, "BOUND", 0);
    txt_put(2, 13, "WEAPON", 2);
    txt_put(13, 13, sv.weapon ? items[sv.weapon].name : "NONE", 0);
    txt_put(2, 14, "TOOL", 2);
    txt_put(13, 14, sv.tool ? items[sv.tool].name : "NONE", 0);
    txt_put(2, 15, "WILD WON", 2);
    txt_num(13, 15, sv.wild_defeated, 0, 0);
    txt_put(2, 17, "PATH", 2);
    txt_put(13, 17, sv.coexist >= 3 ? "MONSTER ALLY" : sv.coexist <= -3 ? "IRON HUNTER" : "UNDECIDED", sv.coexist >= 3 ? 1 : sv.coexist <= -3 ? 3 : 0);
    do frame(); while (!key_hit(KEY_B) && !key_hit(KEY_A));
    sfx(SFX_BACK);
    oam_clear();
}

// ------------------------------------------------------------------ vault
void vault_screen(void) {
    while (1) {
        static const char *const opts[3] = { "DEPOSIT", "WITHDRAW", "CLOSE" };
        msg_noclose("The Soul Vault hums softly. Bound souls rest here safely.");
        int c = menu(19, 6, 11, opts, 3, 0, true);
        msg_close();
        if (c == 0) {
            if (sv.party_n <= 1) { msg("You can't leave yourself without a partner!"); continue; }
            if (sv.vault_n >= VAULT_MAX) { msg("The Soul Vault is full."); continue; }
            int i = party_pick("Deposit which Monstermon?", true);
            screen_begin(0);
            if (i < 0) return;
            int alive_others = 0;
            for (int k = 0; k < sv.party_n; k++) if (k != i && sv.party[k].hp) alive_others++;
            if (!alive_others) { msg("You need at least one healthy partner with you."); return; }
            sv.vault[sv.vault_n++] = sv.party[i];
            for (int k = i; k < sv.party_n - 1; k++) sv.party[k] = sv.party[k + 1];
            sv.party_n--;
            msgf("%s rests in the Soul Vault.", mon_name(&sv.vault[sv.vault_n - 1]));
            return;
        } else if (c == 1) {
            if (!sv.vault_n) { msg("The Soul Vault is empty."); continue; }
            if (sv.party_n >= 6) { msg("Your team is full."); continue; }
            int sel = 0, top = 0;
            screen_begin("SOUL VAULT - WITHDRAW");
            while (1) {
                if (sel < top) top = sel;
                if (sel >= top + 15) top = sel - 14;
                txt_rect(0, 3, 30, 16, FILL);
                for (int r = 0; r < 15 && top + r < sv.vault_n; r++) {
                    Mon *m = &sv.vault[top + r];
                    TXT[(4 + r) * 32 + 1] = top + r == sel ? UI_CURSOR : FILL;
                    txt_put(3, 4 + r, mon_name(m), 0);
                    txt_putc(15, 4 + r, 'L', 1);
                    txt_num(16, 4 + r, m->level, 0, 1);
                    txt_put(20, 4 + r, type_names[species[m->species].t1], 2);
                }
                frame();
                u16 r = key_repeat();
                if (r & KEY_UP) { sel = (sel + sv.vault_n - 1) % sv.vault_n; sfx(SFX_CURSOR); }
                if (r & KEY_DOWN) { sel = (sel + 1) % sv.vault_n; sfx(SFX_CURSOR); }
                if (key_hit(KEY_B)) { sfx(SFX_BACK); return; }
                if (key_hit(KEY_A)) {
                    sfx(SFX_OK);
                    Mon m = sv.vault[sel];
                    mon_heal(&m);
                    sv.party[sv.party_n++] = m;
                    for (int k = sel; k < sv.vault_n - 1; k++) sv.vault[k] = sv.vault[k + 1];
                    sv.vault_n--;
                    msgf("%s rejoined your team!", mon_name(&m));
                    return;
                }
            }
        } else return;
    }
}

// ------------------------------------------------------------------ shop
void shop_screen(const u8 *stock) {
    int n = 0;
    while (stock[n]) n++;
    int sel = 0;
    screen_begin("SHOP");
    while (1) {
        txt_put(18, 1, "$", 1);
        txt_num(19, 1, sv.money, 8, 0);
        txt_rect(0, 3, 30, 11, FILL);
        for (int i = 0; i < n; i++) {
            int it = stock[i];
            TXT[(4 + i) * 32 + 1] = i == sel ? UI_CURSOR : FILL;
            txt_put(2, 4 + i, items[it].name, 0);
            txt_putc(18, 4 + i, '$', 1);
            txt_num(19, 4 + i, items[it].price, 0, 0);
            txt_putc(25, 4 + i, 'x', 2);
            txt_num(26, 4 + i, sv.items[it], 2, 2);
        }
        win_draw(0, 14, 30, 6);
        txt_put(1, 15, items[stock[sel]].desc, 0);
        frame();
        u16 r = key_repeat();
        if (r & KEY_UP) { sel = (sel + n - 1) % n; sfx(SFX_CURSOR); }
        if (r & KEY_DOWN) { sel = (sel + 1) % n; sfx(SFX_CURSOR); }
        if (key_hit(KEY_B)) { sfx(SFX_BACK); txt_clear(); return; }
        if (key_hit(KEY_A)) {
            int it = stock[sel];
            int price = items[it].price;
            int maxq = items[it].kind == IK_TOOL ? (sv.items[it] ? 0 : 1) : 99 - sv.items[it];
            if (maxq <= 0) { sfx(SFX_BUMP); msg("You can't carry any more of those."); screen_begin("SHOP"); continue; }
            if ((int)sv.money < price) { sfx(SFX_BUMP); msg("You don't have enough money."); screen_begin("SHOP"); continue; }
            int q = 1;
            win_draw(16, 9, 14, 5);
            while (1) {
                txt_put(17, 10, "QTY", 2);
                txt_num(21, 10, q, 2, 0);
                txt_putc(17, 11, '$', 1);
                txt_num(18, 11, q * price, 7, 0);
                frame();
                u16 rr = key_repeat();
                if ((rr & KEY_UP) && q < maxq && (q + 1) * price <= (int)sv.money) { q++; sfx(SFX_CURSOR); }
                if ((rr & KEY_DOWN) && q > 1) { q--; sfx(SFX_CURSOR); }
                if ((rr & KEY_RIGHT) && q + 10 <= maxq && (q + 10) * price <= (int)sv.money) { q += 10; sfx(SFX_CURSOR); }
                if ((rr & KEY_LEFT) && q > 10) { q -= 10; sfx(SFX_CURSOR); }
                if (key_hit(KEY_B)) { q = 0; break; }
                if (key_hit(KEY_A)) break;
            }
            if (q) {
                sv.money -= q * price;
                sv.items[it] += q;
                sfx(SFX_ITEM);
                if (it == IT_SOULBIND && q >= 10) {
                    msg("Bought in bulk? Here's a GREATER BIND on the house!");
                    sv.items[IT_GREATBIND]++;
                }
            }
            screen_begin("SHOP");
        }
    }
}

// ------------------------------------------------------------------ hunt board
void hunt_board(void) {
    while (1) {
        screen_begin("HUNT BOARD - BOUNTIES");
        struct { const char *name; int flagid; bool done; bool avail; } b[4] = {
            { "BIND 5 KINDS", F_BOUNTY1, caught_count() >= 5, true },
            { "BIND A BANSHRIEK", F_BOUNTY2, is_caught(SP_BANSHRIEK), true },
            { "WIN 30 WILD FIGHTS", F_BOUNTY3, sv.wild_defeated >= 30, true },
            { "APEX: BIND 25 KINDS", F_BOUNTY4, caught_count() >= 25, flag(F_GAME_CLEAR) },
        };
        const char *opts[5];
        char lines[4][32];
        int n = 0;
        for (int i = 0; i < 4; i++) {
            if (!b[i].avail) continue;
            fmt(lines[n], "%s %s", flag(b[i].flagid) ? "[X]" : b[i].done ? "[!]" : "[ ]", b[i].name);
            opts[n] = lines[n];
            n++;
        }
        opts[n] = "CLOSE";
        txt_put(1, 18, "[!] = reward ready to claim", 2);
        int c = menu(1, 4, 28, opts, n + 1, 0, true);
        if (c < 0 || c == n) return;
        int idx = c;
        for (int i = 0, k = 0; i < 4; i++) {
            if (!b[i].avail) continue;
            if (k++ == c) { idx = i; break; }
        }
        if (flag(b[idx].flagid)) { msg("You already claimed this bounty."); continue; }
        if (!b[idx].done) { msg("Not yet complete. Keep hunting!"); continue; }
        set_flag(b[idx].flagid);
        sfx(SFX_ITEM);
        switch (idx) {
        case 0: sv.items[IT_GREATBIND] += 5; sv.money += 1000; msg("Bounty claimed! 5 GREATER BINDS and $1000."); break;
        case 1: sv.items[IT_LENS] = 1; msg("Bounty claimed! TRACKER LENS. (Equip it as a tool to find rare Eclipse variants.)"); break;
        case 2: sv.items[IT_CROSSBOW] = 1; msg("Bounty claimed! HOLY CROSSBOW. Equip it at the weapon rack."); break;
        case 3: sv.items[IT_STAFF] = 1; sv.money += 20000; msg("APEX bounty claimed! RUNE STAFF and $20000. You are a legend, hunter."); break;
        }
    }
}

// ------------------------------------------------------------------ equipment rack
void equip_screen(void) {
    const char *opts[6];
    u8 ids[6];
    int n = 0;
    for (int i = IT_KNIFE; i <= IT_STAFF; i++)
        if (sv.items[i]) { ids[n] = i; opts[n++] = items[i].name; }
    if (!n) { msg("The weapon rack is empty."); return; }
    msg_noclose("Choose a weapon for HUNT strikes:");
    int c = menu(14, 3, 16, opts, n, 0, true);
    msg_close();
    if (c >= 0) {
        sv.weapon = ids[c];
        sfx(SFX_ITEM);
        msgf("You equip the %s. %s", items[ids[c]].name, items[ids[c]].desc);
    }
}

// ------------------------------------------------------------------ naming & customization
void name_entry(char *out, int maxlen, const char *prompt) {
    static const char grid[6][11] = { "ABCDEFGHIJ", "KLMNOPQRST", "UVWXYZ-.' ", "abcdefghij", "klmnopqrst", "uvwxyz!?  " };
    char name[9] = { 0 };
    int len = 0, cx = 0, cy = 0;
    screen_begin(prompt);
    while (1) {
        txt_rect(8, 4, 14, 1, FILL);
        txt_put(10, 4, name, 0);
        for (int i = len; i < maxlen; i++) txt_putc(10 + i, 4, '_', 2);
        for (int y = 0; y < 6; y++)
            for (int x = 0; x < 10; x++) {
                TXT[(7 + y * 2) * 32 + 4 + x * 2] = (cx == x && cy == y) ? UI_CURSOR : FILL;
                txt_putc(5 + x * 2, 7 + y * 2, grid[y][x], 0);
            }
        TXT[19 * 32 + 6] = cy == 6 && cx < 5 ? UI_CURSOR : FILL;
        TXT[19 * 32 + 16] = cy == 6 && cx >= 5 ? UI_CURSOR : FILL;
        txt_put(7, 19, "DELETE", 0);
        txt_put(17, 19, "DONE", 1);
        frame();
        u16 r = key_repeat();
        if (r & KEY_UP) { cy = (cy + 6) % 7; sfx(SFX_CURSOR); }
        if (r & KEY_DOWN) { cy = (cy + 1) % 7; sfx(SFX_CURSOR); }
        if (r & KEY_LEFT) { cx = (cx + 9) % 10; sfx(SFX_CURSOR); }
        if (r & KEY_RIGHT) { cx = (cx + 1) % 10; sfx(SFX_CURSOR); }
        bool done = key_hit(KEY_START);
        if (key_hit(KEY_B) && len) { name[--len] = 0; sfx(SFX_BACK); }
        if (key_hit(KEY_A)) {
            sfx(SFX_OK);
            if (cy == 6) {
                if (cx < 5) { if (len) name[--len] = 0; }
                else done = true;
            } else if (len < maxlen) {
                char ch = grid[cy][cx];
                if (ch != ' ' || len > 0) { name[len++] = ch; name[len] = 0; }
            }
        }
        if (done) {
            while (len && name[len - 1] == ' ') name[--len] = 0;
            if (!len) strcpy(name, "ROOK");
            strcpy(out, name);
            txt_clear();
            return;
        }
    }
}

void customize_hunter(void) {
    static const char *const coats[4] = { "CRIMSON", "TEAL", "VIOLET", "ASH" };
    static const char *const hairs[3] = { "CHESTNUT", "SILVER", "RAVEN" };
    int row = 0;
    screen_begin("CUSTOMIZE YOUR HUNTER");
    tint_enabled = false;
    world_obj_pals = true;
    load_person_gfx();
    while (1) {
        load_player_palette();
        aff_set(0, 0x80, 0, 0, 0x80);
        int anim = (frame_count >> 5) & 3;
        static const u8 dirs[4] = { 0, 6, 3, 6 };
        spr_affine(0, 104, 40, 0, 1, PS_PLAYER * 36 + dirs[anim] * 4, PS_PLAYER, 0, 0, true);
        if (anim == 3) spr_affine(0, 104, 40, 0, 1, PS_PLAYER * 36 + 6 * 4, PS_PLAYER, 0, 0, true);
        txt_rect(0, 12, 30, 6, FILL);
        TXT[13 * 32 + 3] = row == 0 ? UI_CURSOR : FILL;
        TXT[15 * 32 + 3] = row == 1 ? UI_CURSOR : FILL;
        TXT[17 * 32 + 3] = row == 2 ? UI_CURSOR : FILL;
        txt_put(5, 13, "COAT", 2);
        txt_put(12, 13, "<", 1); txt_put(14, 13, coats[sv.coat], 0); txt_put(23, 13, ">", 1);
        txt_put(5, 15, "HAIR", 2);
        txt_put(12, 15, "<", 1); txt_put(14, 15, hairs[sv.hair], 0); txt_put(23, 15, ">", 1);
        txt_put(5, 17, "THAT'S ME!", 1);
        frame();
        u16 r = key_repeat();
        if (r & KEY_UP) { row = (row + 2) % 3; sfx(SFX_CURSOR); }
        if (r & KEY_DOWN) { row = (row + 1) % 3; sfx(SFX_CURSOR); }
        if (row == 0 && (r & KEY_LEFT)) { sv.coat = (sv.coat + 3) % 4; sfx(SFX_CURSOR); }
        if (row == 0 && (r & KEY_RIGHT)) { sv.coat = (sv.coat + 1) % 4; sfx(SFX_CURSOR); }
        if (row == 1 && (r & KEY_LEFT)) { sv.hair = (sv.hair + 2) % 3; sfx(SFX_CURSOR); }
        if (row == 1 && (r & KEY_RIGHT)) { sv.hair = (sv.hair + 1) % 3; sfx(SFX_CURSOR); }
        if (key_hit(KEY_A) && row == 2) { sfx(SFX_OK); break; }
        if (key_hit(KEY_START)) { sfx(SFX_OK); break; }
    }
    oam_clear();
    txt_clear();
}

// ------------------------------------------------------------------ start menu
void start_menu(void) {
    static int last;
    static const char *const opts[6] = { "TEAM", "BESTIARY", "BAG", "HUNTER", "SAVE", "CLOSE" };
    sfx(SFX_OK);
    while (1) {
        win_draw(0, 0, 13, 4);
        txt_num(1, 1, sv.clock / 60, 2, 0);
        txt_putc(3, 1, ':', 0);
        txt_num(4, 1, sv.clock % 60 / 10, 0, 0);
        txt_num(5, 1, sv.clock % 10, 0, 0);
        txt_put(7, 1, is_night() ? "NIGHT" : "DAY", 1);
        static const char *const wx[4] = { "CLEAR", "RAIN", "FOG", "BLOOD MOON" };
        txt_put(1, 2, wx[weather_now], weather_now == WX_BLOODMOON ? 3 : 2);
        int c = menu(20, 0, 10, opts, 6, last, true);
        win_clear(0, 0, 13, 4);
        if (c < 0 || c == 5) break;
        last = c;
        switch (c) {
        case 0: if (sv.party_n) party_screen(); else msg("You have no Monstermons yet."); break;
        case 1: bestiary_screen(); break;
        case 2: bag_screen(false, 0); break;
        case 3: hunter_card(); break;
        case 4:
            if (yesno("Save your hunt?")) {
                save_write();
                sfx(SFX_HEAL);
                msg("Your journey has been recorded.");
            }
            continue;
        }
        world_reload_gfx();
    }
    world_reload_gfx();
}
