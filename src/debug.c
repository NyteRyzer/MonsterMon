// MonsterMons - debug tools, compiled only into the debug ROM (make debug)
#ifdef DEBUG
#include "core.h"

void debug_menu(void) {
    static const char *const opts[] = { "WARP", "POWER TEAM", "STORY: LICENSE", "STORY: SIGIL", "STORY: RIFT", "NIGHT/DAY", "WILD BATTLE", "STARTER TEAM", "CLOSE" };
    int c = menu(10, 2, 18, opts, 9, 0, true);
    if (c == 6) {
        int r = battle_wild(SP_GHOULET, 6, -1, 0, false);
        world_reload_gfx();
        if (r == BR_LOSE) whiteout(); else fade_from_black(12);
        return;
    }
    if (c == 7) {
        sv.party_n = 0;
        Mon m; mon_create(&m, SP_FANGLING, 8); give_mon(&m);
        mon_create(&m, SP_HEXCAT, 7); give_mon(&m);
        sv.items[IT_SOULBIND] = 5; sv.items[IT_TONIC] = 3;
        msg("Starter team ready.");
        return;
    }
    if (c == 0) {
        const char *names[MAP_COUNT];
        for (int i = 0; i < MAP_COUNT; i++) names[i] = map_defs[i].name;
        int m = menu(8, 0, 20, names, MAP_COUNT, 0, true);
        if (m >= 0) {
            static const u8 spawn[MAP_COUNT][2] = { { 11, 5 }, { 4, 5 }, { 4, 5 }, { 5, 6 }, { 12, 27 }, { 12, 17 },
                                                    { 2, 9 }, { 5, 9 }, { 9, 13 }, { 9, 17 }, { 9, 16 } };
            fade_to_black(8);
            world_enter(m, spawn[m][0], spawn[m][1], 1);
        }
        return;
    }
    if (c == 1) {
        static const u8 team[6] = { SP_COUNTESS, SP_CERBERUS, SP_SERAPHLY, SP_SASQUATCH, SP_GRIMALKIN, SP_LYCANTH };
        sv.party_n = 0;
        for (int i = 0; i < 6; i++) { Mon m; mon_create(&m, team[i], 45); give_mon(&m); }
        for (int i = 1; i < IT_COUNT; i++) sv.items[i] = 20;
        sv.money = 99999;
        msg("Debug team ready.");
    }
    if (c >= 2 && c <= 4) {
        set_flag(F_INTRO); set_flag(F_GOT_STARTER); set_flag(F_BEAT_VEX1); set_flag(F_WOODS_GRUNTS);
        set_flag(F_CLUE1); set_flag(F_CLUE2); set_flag(F_CLUE3); set_flag(F_CLUES_ALL); set_flag(F_MET_MIRA);
        set_flag(F_TRIAL1); set_flag(F_TRIAL2); set_flag(F_TRIAL3); set_flag(F_LICENSE); set_flag(F_VEX2);
        if (c >= 3) { set_flag(F_GOT_SIGIL); sv.items[IT_SIGIL] = 1; set_flag(F_GATE_OPEN); }
        if (c >= 4) { set_flag(F_CASTLE_OPEN); set_flag(F_BEAT_SOLENNE); set_flag(F_BEAT_HOLLOW); set_flag(F_RIFT_OPEN); }
        msg("Story flags set.");
    }
    if (c == 5) { sv.clock = is_night() ? 12 * 60 : 23 * 60; pal_refresh(); }
}
#endif
