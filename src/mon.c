// MonsterMons - creature creation, stats, experience, bonds and transformations
#include <string.h>
#include "core.h"

const char *mon_name(const Mon *m) { return species[m->species].name; }

u32 xp_for_level(int lv) { return (u32)lv * lv * lv * 4 / 5; }

static const Species *SP(const Mon *m) { return &species[m->species]; }

int mon_maxhp(const Mon *m) {
    const Species *s = SP(m);
    int v = s->hp * 2 * m->level / 100 + m->level + 10;
    if (m->variant) v += v / 10;
    return v;
}

int mon_stat(const Mon *m, int which) {
    const Species *s = SP(m);
    static const u8 nat_stat[NAT_COUNT] = { 0, 1, 2, 3, 0xFF };
    int base = which == 0 ? s->atk : which == 1 ? s->def : which == 2 ? s->spd : s->spc;
    int v = base * 2 * m->level / 100 + 5;
    if (nat_stat[m->nature] == which) v += v / 10;
    if (m->variant) v += v / 10;
    return v;
}

static void learn_upto(Mon *m, int level) {
    const u8 *l = SP(m)->learn;
    int n = 0;
    u8 mv[4] = { 0 };
    for (; l[0]; l += 2) {
        if (l[0] > level) break;
        bool dup = false;
        for (int i = 0; i < 4; i++) if (mv[i] == l[1]) dup = true;
        if (dup) continue;
        if (n < 4) mv[n++] = l[1];
        else { mv[0] = mv[1]; mv[1] = mv[2]; mv[2] = mv[3]; mv[3] = l[1]; }
    }
    for (int i = 0; i < 4; i++) {
        m->moves[i] = mv[i];
        m->pp[i] = moves[mv[i]].pp;
    }
}

void mon_create(Mon *m, int sp, int level) {
    memset(m, 0, sizeof(*m));
    m->species = sp;
    m->level = level;
    m->nature = rnd_n(NAT_COUNT);
    m->bond = 20;
    m->xp = xp_for_level(level);
    learn_upto(m, level);
    m->hp = mon_maxhp(m);
}

void mon_heal(Mon *m) {
    m->hp = mon_maxhp(m);
    m->status = ST_NONE;
    for (int i = 0; i < 4; i++) m->pp[i] = moves[m->moves[i]].pp;
}

void party_heal_all(void) {
    for (int i = 0; i < sv.party_n; i++) mon_heal(&sv.party[i]);
}

int party_alive_count(void) {
    int n = 0;
    for (int i = 0; i < sv.party_n; i++) if (sv.party[i].hp) n++;
    return n;
}

void mark_seen(int sp) { sv.seen[sp >> 3] |= 1 << (sp & 7); }
void mark_caught(int sp) { mark_seen(sp); sv.caught[sp >> 3] |= 1 << (sp & 7); }
bool is_seen(int sp) { return (sv.seen[sp >> 3] >> (sp & 7)) & 1; }
bool is_caught(int sp) { return (sv.caught[sp >> 3] >> (sp & 7)) & 1; }
int caught_count(void) {
    int n = 0;
    for (int i = 0; i < SPECIES_COUNT; i++) n += is_caught(i);
    return n;
}

bool give_mon(const Mon *m) {
    mark_caught(m->species);
    if (sv.party_n < 6) { sv.party[sv.party_n++] = *m; return true; }
    if (sv.vault_n < VAULT_MAX) { sv.vault[sv.vault_n++] = *m; return true; }
    return false;
}

void mon_learn_message(Mon *m, int mv) {
    for (int i = 0; i < 4; i++) if (m->moves[i] == mv) return;
    for (int i = 0; i < 4; i++) {
        if (!m->moves[i]) {
            m->moves[i] = mv;
            m->pp[i] = moves[mv].pp;
            sfx(SFX_ITEM);
            msgf("%s learned %s!", mon_name(m), moves[mv].name);
            return;
        }
    }
    char q[96];
    fmt(q, "%s wants to learn %s. Forget a move for it?", mon_name(m), moves[mv].name);
    if (!yesno(q)) {
        msgf("%s did not learn %s.", mon_name(m), moves[mv].name);
        return;
    }
    const char *opts[5];
    for (int i = 0; i < 4; i++) opts[i] = moves[m->moves[i]].name;
    opts[4] = "KEEP ALL";
    msg_noclose("Which move should be forgotten?");
    int r = menu(14, 2, 16, opts, 5, 0, true);
    msg_close();
    if (r < 0 || r == 4) {
        msgf("%s did not learn %s.", mon_name(m), moves[mv].name);
        return;
    }
    msgf("1, 2, and... Poof! %s forgot %s and learned %s!", mon_name(m), moves[m->moves[r]].name, moves[mv].name);
    m->moves[r] = mv;
    m->pp[r] = moves[mv].pp;
}

int mon_gain_xp(Mon *m, u32 xp, bool show) {
    int gained = 0;
    m->xp += xp;
    while (m->level < 100 && m->xp >= xp_for_level(m->level + 1)) {
        int oldmax = mon_maxhp(m);
        m->level++;
        gained++;
        int newmax = mon_maxhp(m);
        if (m->hp) m->hp += newmax - oldmax;
        int bonus = m->nature == NAT_LOYAL ? 6 : 3;
        m->bond = m->bond + bonus > 255 ? 255 : m->bond + bonus;
        if (show) {
            sfx(SFX_LEVEL);
            msgf("%s grew to level %d!", mon_name(m), m->level);
        }
        for (const u8 *l = SP(m)->learn; l[0]; l += 2)
            if (l[0] == m->level && show) mon_learn_message(m, l[1]);
    }
    return gained;
}

int mon_evolve_target(const Mon *m) {
    const Species *s = SP(m);
    if (s->evo_kind == EVO_NONE || m->level < s->evo_lv) return -1;
    switch (s->evo_kind) {
    case EVO_LEVEL: return s->evo_a;
    case EVO_NIGHT: return is_night() ? s->evo_a : -1;
    case EVO_DAYNIGHT: return is_night() ? s->evo_b : s->evo_a;
    case EVO_BOND: return m->bond >= 150 ? s->evo_a : s->evo_b;
    case EVO_FANG:
        if (m->bond >= 150) return s->evo_a;
        return is_night() ? s->evo_b : -1;
    }
    return -1;
}

static void evo_flash(int from, int to, int variant) {
    // alternate the old and new forms with growing speed
    for (int k = 0; k < 14; k++) {
        int sp = (k & 1) ? to : from;
        show_mon_preview(sp, variant, 88, 16, 0, true);
        sfx(SFX_CURSOR);
        wait_frames(18 - k);
    }
    show_mon_preview(to, variant, 88, 16, 0, true);
}

void mon_try_evolve(Mon *m) {
    int to = mon_evolve_target(m);
    if (to < 0) return;
    txt_clear();
    oam_clear();
    int from = m->species;
    show_mon_preview(from, m->variant, 88, 16, 0, true);
    frame();
    char buf[96];
    fmt(buf, "Something is happening to %s! Its soul is transforming...", mon_name(m));
    msg(buf);
    if (!yesno("Allow the transformation?")) {
        msgf("%s stayed as it is.", mon_name(m));
        oam_clear();
        return;
    }
    sfx(SFX_EVOLVE);
    evo_flash(from, to, m->variant);
    int oldmax = mon_maxhp(m);
    m->species = to;
    int newmax = mon_maxhp(m);
    if (m->hp) m->hp += newmax - oldmax;
    mark_caught(to);
    sfx(SFX_BIND_OK);
    msgf("%s transformed into %s!", species[from].name, species[to].name);
    for (const u8 *l = species[to].learn; l[0]; l += 2)
        if (l[0] <= 1) mon_learn_message(m, l[1]);
    oam_clear();
}
