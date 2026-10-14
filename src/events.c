// MonsterMons - story, NPCs, warps, trainers, puzzles and endings
#include <string.h>
#include "world.h"

enum {
    S_TOWN_MAN, S_TOWN_WOMAN, S_TOWN_KID, S_TOWN_GRUNT, S_TOWN_HUNTER, S_WREN, S_SHOP, S_ASH, S_VEX_GUILD,
    S_VEX_FINAL, S_WOODS_GRUNTS, S_WOLF, S_WOODS_HUNTER, S_MIRA, S_GRAVEKEEPER, S_GRAVE_HUNTER, S_VEX_GRAVE,
    S_GATE_GUARD, S_MARKET, S_NEON_WOMAN, S_NEON_KID, S_NEON_HUNTER, S_JUDGE, S_TRIAL1, S_TRIAL2, S_TRIAL3,
    S_VEX_ARENA, S_CASTLE_G1, S_CASTLE_G2, S_SOLENNE, S_HOLLOW, S_MIRA_CASTLE, S_ECLIPSA,
};

#define N(m, x, y, spr, f, show, hide, sc) { MAP_##m, x, y, spr, f, show, hide, sc }
const NpcDef npc_defs[] = {
    N(HOLLOWMERE, 8, 7, PS_MAN, DIR_DOWN, -1, -1, S_TOWN_MAN),
    N(HOLLOWMERE, 16, 10, PS_WOMAN, DIR_LEFT, -1, -1, S_TOWN_WOMAN),
    N(HOLLOWMERE, 3, 12, PS_KID, DIR_RIGHT, -1, -1, S_TOWN_KID),
    N(HOLLOWMERE, 20, 7, PS_GRUNT, DIR_DOWN, -1, F_BEAT_SOLENNE, S_TOWN_GRUNT),
    N(HOLLOWMERE, 14, 17, PS_HUNTER, DIR_UP, -1, -1, S_TOWN_HUNTER),
    N(SANCTUARY, 3, 1, PS_WREN, DIR_DOWN, -1, -1, S_WREN),
    N(SANCTUARY, 6, 1, PS_KEEPER, DIR_DOWN, -1, -1, S_SHOP),
    N(GUILD, 5, 2, PS_ASH, DIR_DOWN, -1, -1, S_ASH),
    N(GUILD, 7, 4, PS_VEX, DIR_LEFT, -1, F_BEAT_VEX1, S_VEX_GUILD),
    N(GUILD, 8, 5, PS_VEX, DIR_LEFT, F_GAME_CLEAR, F_VEX_FINAL, S_VEX_FINAL),
    N(WOODS, 11, 9, PS_GRUNT, DIR_DOWN, -1, F_WOODS_GRUNTS, S_WOODS_GRUNTS),
    N(WOODS, 12, 9, PS_GRUNT, DIR_DOWN, -1, F_WOODS_GRUNTS, S_WOODS_GRUNTS),
    N(WOODS, 4, 1, SPR_MON(SP_LYCANTH), DIR_DOWN, F_CLUES_ALL, F_WOLF_DONE, S_WOLF),
    N(WOODS, 10, 24, PS_HUNTER, DIR_LEFT, -1, -1, S_WOODS_HUNTER),
    N(GRAVE, 14, 9, PS_MIRA, DIR_LEFT, -1, F_MET_MIRA, S_MIRA),
    N(GRAVE, 4, 10, PS_MAN, DIR_RIGHT, -1, -1, S_GRAVEKEEPER),
    N(GRAVE, 19, 9, PS_HUNTER, DIR_LEFT, -1, -1, S_GRAVE_HUNTER),
    N(GRAVE, 12, 16, PS_VEX, DIR_DOWN, F_BEAT_VEX1, F_VEX2, S_VEX_GRAVE),
    N(NEON, 14, 1, PS_GRUNT, DIR_DOWN, -1, F_CASTLE_OPEN, S_GATE_GUARD),
    N(NEON, 15, 1, PS_GRUNT, DIR_DOWN, -1, F_CASTLE_OPEN, S_GATE_GUARD),
    N(NEON, 17, 7, PS_KEEPER, DIR_DOWN, -1, -1, S_MARKET),
    N(NEON, 8, 12, PS_WOMAN, DIR_UP, -1, -1, S_NEON_WOMAN),
    N(NEON, 25, 12, PS_KID, DIR_DOWN, -1, -1, S_NEON_KID),
    N(NEON, 5, 7, PS_HUNTER, DIR_RIGHT, -1, -1, S_NEON_HUNTER),
    N(ARENA, 7, 10, PS_KEEPER, DIR_LEFT, -1, -1, S_JUDGE),
    N(ARENA, 4, 8, PS_HUNTER, DIR_RIGHT, -1, -1, S_TRIAL1),
    N(ARENA, 7, 6, PS_WOMAN, DIR_LEFT, -1, -1, S_TRIAL2),
    N(ARENA, 4, 4, PS_MAN, DIR_RIGHT, -1, -1, S_TRIAL3),
    N(ARENA, 6, 2, PS_VEX, DIR_DOWN, -1, -1, S_VEX_ARENA),
    N(CASTLE, 8, 11, PS_GRUNT, DIR_RIGHT, -1, -1, S_CASTLE_G1),
    N(CASTLE, 11, 8, PS_GRUNT, DIR_LEFT, -1, -1, S_CASTLE_G2),
    N(CASTLE, 9, 5, PS_SOLENNE, DIR_DOWN, -1, F_BEAT_SOLENNE, S_SOLENNE),
    N(CASTLE, 10, 3, SPR_MON(SP_HOLLOWKING), DIR_DOWN, -1, F_BEAT_HOLLOW, S_HOLLOW),
    N(CASTLE, 12, 4, PS_MIRA, DIR_LEFT, F_BEAT_HOLLOW, F_BEAT_ECLIPSA, S_MIRA_CASTLE),
    N(RIFT, 9, 2, SPR_MON(SP_ECLIPSA), DIR_DOWN, -1, F_BEAT_ECLIPSA, S_ECLIPSA),
};
const int npc_def_count = sizeof(npc_defs) / sizeof(npc_defs[0]);

#define W(m, x, y, d, dx, dy, f) { MAP_##m, x, y, MAP_##d, dx, dy, f }
const Warp warps[] = {
    W(HOLLOWMERE, 5, 4, HOME, 4, 6, DIR_UP), W(HOLLOWMERE, 18, 4, SANCTUARY, 4, 6, DIR_UP),
    W(HOLLOWMERE, 11, 15, GUILD, 5, 7, DIR_UP), W(HOLLOWMERE, 12, 15, GUILD, 6, 7, DIR_UP),
    W(HOLLOWMERE, 11, 0, WOODS, 11, 28, DIR_UP), W(HOLLOWMERE, 12, 0, WOODS, 12, 28, DIR_UP),
    W(HOME, 4, 7, HOLLOWMERE, 5, 5, DIR_DOWN), W(HOME, 5, 7, HOLLOWMERE, 5, 5, DIR_DOWN),
    W(SANCTUARY, 4, 7, HOLLOWMERE, 18, 5, DIR_DOWN), W(SANCTUARY, 5, 7, HOLLOWMERE, 18, 5, DIR_DOWN),
    W(GUILD, 5, 8, HOLLOWMERE, 11, 16, DIR_DOWN), W(GUILD, 6, 8, HOLLOWMERE, 12, 16, DIR_DOWN),
    W(WOODS, 11, 29, HOLLOWMERE, 11, 1, DIR_DOWN), W(WOODS, 12, 29, HOLLOWMERE, 12, 1, DIR_DOWN),
    W(WOODS, 11, 0, GRAVE, 11, 18, DIR_UP), W(WOODS, 12, 0, GRAVE, 12, 18, DIR_UP),
    W(GRAVE, 11, 19, WOODS, 11, 1, DIR_DOWN), W(GRAVE, 12, 19, WOODS, 12, 1, DIR_DOWN),
    W(GRAVE, 23, 9, NEON, 1, 9, DIR_RIGHT), W(GRAVE, 23, 10, NEON, 1, 10, DIR_RIGHT),
    W(GRAVE, 11, 2, MANOR, 9, 14, DIR_UP), W(GRAVE, 12, 2, MANOR, 10, 14, DIR_UP),
    W(NEON, 0, 9, GRAVE, 22, 9, DIR_LEFT), W(NEON, 0, 10, GRAVE, 22, 10, DIR_LEFT),
    W(NEON, 14, 0, CASTLE, 9, 18, DIR_UP), W(NEON, 15, 0, CASTLE, 10, 18, DIR_UP),
    W(NEON, 22, 17, ARENA, 5, 10, DIR_UP), W(NEON, 23, 17, ARENA, 6, 10, DIR_UP),
    W(ARENA, 5, 11, NEON, 22, 18, DIR_DOWN), W(ARENA, 6, 11, NEON, 23, 18, DIR_DOWN),
    W(MANOR, 9, 15, GRAVE, 11, 3, DIR_DOWN), W(MANOR, 10, 15, GRAVE, 12, 3, DIR_DOWN),
    W(CASTLE, 9, 19, NEON, 14, 1, DIR_DOWN), W(CASTLE, 10, 19, NEON, 15, 1, DIR_DOWN),
    W(CASTLE, 14, 2, RIFT, 9, 18, DIR_UP),
    W(RIFT, 9, 19, CASTLE, 14, 3, DIR_DOWN),
};
const int warp_count = sizeof(warps) / sizeof(warps[0]);

// ------------------------------------------------------------------ helpers
static void give_item(int it, int n) {
    sv.items[it] = sv.items[it] + n > 99 ? 99 : sv.items[it] + n;
    sfx(SFX_ITEM);
    if (n == 1) msgf("%s received the %s!", sv.name, items[it].name);
    else msgf("%s received %d x %s!", sv.name, n, items[it].name);
}

static void heal_scene(void) {
    party_heal_all();
    sfx(SFX_HEAL);
    for (int k = 0; k < 4; k++) { set_brightness(6); wait_frames(4); set_brightness(0); wait_frames(4); }
}

static int vex_species(int evolved) {
    static const u8 counter[3][2] = { { SP_WISPETTE, SP_BANSHRIEK }, { SP_FANGLING, SP_COUNTESS }, { SP_PUPWOLF, SP_MOONHOWL } };
    int i = sv.starter == SP_FANGLING ? 0 : sv.starter == SP_PUPWOLF ? 1 : 2;
    return counter[i][evolved];
}

// Runs a trainer battle and restores the overworld. Returns true on victory.
static bool duel(const Trainer *t) {
    if (t->intro) say(t->name, t->intro);
    int r = battle_trainer(t);
    world_reload_gfx();
    if (r == BR_LOSE) { whiteout(); return false; }
    fade_from_black(12);
    return true;
}

static bool boss_fight(int sp, int lv) {
    int r = battle_wild(sp, lv, -1, 0, true);
    world_reload_gfx();
    if (r == BR_LOSE) { whiteout(); return false; }
    fade_from_black(12);
    return true;
}

static Trainer tr_buf;
static Trainer *make_trainer(const char *name, int spr, int max_active, u16 prize, const char *intro, const char *lose,
                             bool boss, int n, const u8 *sp, const u8 *lv) {
    memset(&tr_buf, 0, sizeof(tr_buf));
    tr_buf.name = name;
    tr_buf.sprite = spr;
    tr_buf.n = n;
    tr_buf.max_active = max_active;
    for (int i = 0; i < n; i++) { tr_buf.sp[i] = sp[i]; tr_buf.lv[i] = lv[i]; }
    tr_buf.prize = prize;
    tr_buf.intro = intro;
    tr_buf.lose = lose;
    tr_buf.boss = boss;
    return &tr_buf;
}
// ------------------------------------------------------------------ starter ceremony
static void starter_ceremony(void) {
    say("WARDEN ASH", "{P}! There you are. Ten years since the Nightfall, and the Guild still needs every hunter it can get.");
    say("WARDEN ASH", "Every hunter begins with a partner. These three were found orphaned near Hollowmere.");
    say("WARDEN ASH", "A vampire whelp, a werewolf pup, and a lost little spirit. Choose the one whose soul calls to yours.");
    static const u8 choices[3] = { SP_FANGLING, SP_PUPWOLF, SP_WISPETTE };
    while (1) {
        txt_clear();
        for (int i = 0; i < 3; i++) show_mon_preview(choices[i], 0, 20 + i * 76, 20, i, false);
        for (int i = 0; i < 3; i++) {
            win_draw(10 * i, 9, 10, 4);
            txt_put(10 * i + 1, 10, species[choices[i]].name, 0);
            txt_put(10 * i + 1, 11, type_names[species[choices[i]].t1], 1);
        }
        frame();
        static const char *const opts[3] = { "FANGLING", "PUPWOLF", "WISPETTE" };
        int r = menu(18, 1, 12, opts, 3, 0, false);
        int sp = choices[r];
        char q[120];
        msg(species[sp].lore);
        fmt(q, "Bond your soul with %s, the %s Monstermon?", species[sp].name, type_names[species[sp].t1]);
        if (yesno(q)) {
            oam_clear();
            txt_clear();
            Mon m;
            mon_create(&m, sp, 5);
            m.bond = 70;
            give_mon(&m);
            sv.starter = sp;
            sfx(SFX_BIND_OK);
            msgf("The Soulbind glows! %s's soul is now bonded with %s!", sv.name, species[sp].name);
            break;
        }
    }
    oam_clear();
    world_reload_gfx();
    set_flag(F_GOT_STARTER);
    say("WARDEN ASH", "Take these too. A hunter without Soulbinds is just a tourist with a flashlight.");
    give_item(IT_SOULBIND, 5);
    give_item(IT_TONIC, 3);
    say("VEX", "Hold it, Warden. Let me see if the rookie's even worth the ink in your ledger.");
    say("VEX", "I'll take this one. Hah - looks like it's got your number, {P}.");
    msgf("VEX bonded with %s!", species[vex_species(0)].name);
    u8 sp[1] = { (u8)vex_species(0) }, lv[1] = { 4 };
    Trainer *t = make_trainer("VEX", PS_VEX, 1, 300, "Let's see what that Soulbind can really do!",
                              "...Beginner's luck. Don't let it go to your head.", false, 1, sp, lv);
    int r = battle_trainer(t);
    world_reload_gfx();
    if (r == BR_LOSE) {
        party_heal_all();
        fade_from_black(12);
        say("VEX", "Ha! Told you. Go heal up, rookie. I'll be waiting out in the world.");
    } else {
        fade_from_black(12);
    }
    party_heal_all();
    set_flag(F_BEAT_VEX1);
    world_refresh_npcs();
    say("WARDEN ASH", "Easy now, both of you. Your team's rested. Listen, {P} - strange tracks were found in the Whisper Woods.");
    say("WARDEN ASH", "And the ORDER OF THE PALE SUN has been sending 'hunting parties' there. They don't Soulbind. They destroy.");
    say("WARDEN ASH", "Head north through the woods. Investigate. Earn your Hunter License at the Trial Arena in Neon Row.");
    say("WARDEN ASH", "Check the HUNT BOARD here for bounties, and the weapon rack for your gear. May the moon guide you.");
}

// ------------------------------------------------------------------ the ending
static void credits(void) {
    fade_to_black(30);
    txt_clear();
    oam_clear();
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_OBJ;
    PAL_BG[0] = 0;
    bgm(BGM_TITLE);
    set_brightness(0);
    static const char *const lines[] = {
        "MONSTERMONS", "", "A supernatural hunt", "", "Story, art, code, music", "made for the GBA", "",
        "Thank you for playing!", "", "Your hunt continues...", "The Rift remains open.",
    };
    for (unsigned i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
        int n = strlen(lines[i]);
        txt_put(15 - n / 2, 3 + i, lines[i], i == 0 ? 1 : 0);
        wait_frames(40);
    }
    while (!key_hit(KEY_A) && !key_hit(KEY_START)) frame();
    fade_to_black(30);
    txt_clear();
}

static void ending(void) {
    say("ECLIPSA", "...Hunter. You have bested me. Ten years ago the Pale Sun tore me from my slumber to steal my light.");
    say("ECLIPSA", "My pain became the Nightfall. Every creature that crossed into your world fled through my wound.");
    say("ECLIPSA", "Now I am yours to judge. Will you end me... chain me... or walk beside me?");
    static const char *const opts[3] = { "DESTROY", "CONTROL", "COEXIST" };
    int c;
    while (1) {
        msg_noclose("What will you do?");
        c = menu(19, 6, 11, opts, 3, 2, false);
        msg_close();
        static const char *const confirm[3] = {
            "Destroy Eclipsa? The Rift will close forever... and so might every bond between our worlds.",
            "Bind Eclipsa as a weapon? Its power will answer to you alone.",
            "Offer Eclipsa your hand, as an equal?",
        };
        if (yesno(confirm[c])) break;
    }
    sv.ending = c + 1;
    if (c == 0) {
        sv.coexist -= 5;
        say("", "You raise your weapon. Eclipsa closes its eyes and does not resist.");
        say("", "Light swallows the Rift. When it fades, the dragon is gone... and a cold silence falls over every Monstermon.");
    } else if (c == 1) {
        sv.coexist -= 2;
        say("", "The Soulbind roars to life. Eclipsa's will strains... then breaks. Its eyes dull to obedience.");
        Mon m;
        mon_create(&m, SP_ECLIPSA, 40);
        m.bond = 0;
        give_mon(&m);
        set_flag(F_ECLIPSA_JOINED);
        msg("ECLIPSA was bound to your soul. Its power is yours to command.");
    } else {
        sv.coexist += 5;
        say("", "You lower your weapon and extend your hand. For a long moment, nothing moves.");
        say("ECLIPSA", "...Ten years I waited for a human to choose kindness. I will walk beside you, {P}.");
        Mon m;
        mon_create(&m, SP_ECLIPSA, 40);
        m.bond = 255;
        give_mon(&m);
        set_flag(F_ECLIPSA_JOINED);
        msg("ECLIPSA joined your team of its own free will! The moon blazes bright for the first time in ten years.");
    }
    // epilogue shaped by the hunter's choices
    if (sv.coexist >= 5) {
        say("MIRA", "Monstermons and people are rebuilding Hollowmere side by side. They're calling you the Bridge, {P}.");
    } else if (sv.coexist >= 0) {
        say("MIRA", "The world is uneasy, but it's healing. Some fear Monstermons. Others are learning not to. That's a start.");
    } else {
        say("MIRA", "Hunters are feared now, even by the creatures that once trusted us. I hope the cost was worth it, {P}.");
    }
    if (flag(F_WOLF_SPARED)) say("LYCANTH", "*The alpha you saved howls a long, grateful note from the Whisper Woods.*");
    if (flag(F_HOLLOW_FREED)) say("", "Somewhere in Duskhold, an ancient king sleeps peacefully at last, free of his crown-cage.");
    say("WARDEN ASH", "You did it, {P}. Whatever comes next, the Guild is proud to have you. Now... the Rift still calls. Rest, then hunt.");
    set_flag(F_GAME_CLEAR);
    set_flag(F_BEAT_ECLIPSA);
    credits();
    party_heal_all();
    sv.clock = 7 * 60;
    gfx_init();
    world_enter(MAP_HOME, 4, 3, DIR_DOWN);
    msg("A new day dawns. Postgame unlocked: the Rift teems with powerful Monstermons, and VEX awaits a rematch at the Guild.");
}

// ------------------------------------------------------------------ NPC scripts
void events_npc(int script) {
    char buf[160];
    switch (script) {
    case S_TOWN_MAN:
        if (!flag(F_GOT_STARTER)) say("TOWNSMAN", "Warden Ash is waiting at the Guild Hall - the big building south of the plaza.");
        else say("TOWNSMAN", "The memorial in the plaza honors those lost on the Nightfall. My brother's name is on it.");
        break;
    case S_TOWN_WOMAN:
        say("TOWNSWOMAN", is_night()
            ? "At night the spirits come out. Ghost-types can only be Soulbound after dark, you know."
            : "By day you'll find gentler Monstermons. Halofly only appear while the sun is up.");
        break;
    case S_TOWN_KID:
        say("KID", "Hold B to run! And Monstermons evolve differently depending on day, night... even how much they love you!");
        break;
    case S_TOWN_GRUNT:
        say("PALE SUN RECRUITER", "The Order of the Pale Sun will burn every monster from this world. Join us, hunter. Purity is coming.");
        break;
    case S_TOWN_HUNTER:
        say("HUNTER", "Weather changes what you'll meet. Glitchost only shows in Neon Row's rain. And a BLOOD MOON brings rare colors...");
        break;
    case S_WREN:
        say("SISTER WREN", "Welcome to the Sanctuary, child. Let me tend to your Monstermons.");
        heal_scene();
        sv.last_heal_map = MAP_SANCTUARY;
        sv.last_heal_x = 4;
        sv.last_heal_y = 3;
        say("SISTER WREN", "Your team is fully restored. The altar to the left is the Soul Vault; the right one keeps your journey safe.");
        break;
    case S_SHOP: {
        say("KEEPER", "Supplies for the hunt. Coin up front, no refunds from beyond the grave.");
        static const u8 stock[] = { IT_SOULBIND, IT_GREATBIND, IT_TONIC, IT_SALT, IT_REVIVE, IT_OFFERING, 0 };
        shop_screen(stock);
        world_reload_gfx();
        break;
    }
    case S_ASH:
        if (!flag(F_GOT_STARTER)) { starter_ceremony(); break; }
        if (flag(F_GAME_CLEAR)) say("WARDEN ASH", "The Apex bounties are posted on the Hunt Board. Only the greatest hunters need apply.");
        else if (!flag(F_CLUES_ALL)) say("WARDEN ASH", "Examine anything strange in the Whisper Woods. Claw marks, fur, tracks... they'll lead you to the truth.");
        else if (!flag(F_LICENSE)) say("WARDEN ASH", "Beyond Grave Hill lies Neon Row. Pass the Trials at the Arena there to earn your Hunter License.");
        else if (!flag(F_GOT_SIGIL)) say("WARDEN ASH", "Licensed! Ravencrest Manor on Grave Hill will open for you now. Find out what the Pale Sun is hiding.");
        else say("WARDEN ASH", "Duskhold Castle... so that's where the Pale Sun nests. Be careful, {P}. Bring a full team.");
        break;
    case S_VEX_GUILD:
        say("VEX", "What, you want to talk to the Warden? He's right there. Go on.");
        break;
    case S_VEX_FINAL: {
        say("VEX", "So the world-saver finally has time for a real rematch. No holding back this time, {P}.");
        u8 sp[6] = { (u8)vex_species(1), SP_CERBERUS, SP_GRIMALKIN, SP_SERAPHLY, SP_SASQUATCH, SP_GHOULORD };
        u8 lv[6] = { 52, 48, 48, 48, 48, 47 };
        Trainer *t = make_trainer("VEX", PS_VEX, 3, 10000, 0,
                                  "...Okay. You're the real deal. Maybe I'll stop chasing you and start hunting WITH you.", true, 6, sp, lv);
        if (duel(t)) set_flag(F_VEX_FINAL);
        break;
    }
    case S_WOODS_GRUNTS: {
        say("PALE SUN GRUNT", "Halt! This forest is under purification. Every creature in here gets burned out by dawn.");
        say("PALE SUN GRUNT", "A Guild hunter? Your kind PETS the abominations. We'll purify you too!");
        u8 sp[2] = { SP_BATLING, SP_GHOULET }, lv[2] = { 6, 7 };
        Trainer *t = make_trainer("PALE SUN DUO", PS_GRUNT, 2, 400, 0,
                                  "Tch! Retreat! Commander Solenne will hear about this...", false, 2, sp, lv);
        if (duel(t)) {
            set_flag(F_WOODS_GRUNTS);
            world_refresh_npcs();
            msg("The Pale Sun grunts fled north. They dropped something...");
            give_item(IT_SOULBIND, 3);
        }
        break;
    }
    case S_WOLF:
        say("", "A huge wolf-man lies in the flowers, breathing hard. A silver Pale Sun bolt is buried in its side.");
        say("LYCANTH", "*Its golden eyes meet yours. It snarls weakly... then simply waits.*");
        say("", "The Pale Sun pays a bounty for werewolves. A hunter could make a fortune turning this one in.");
        {
            static const char *const opts[2] = { "HELP IT", "REPORT IT" };
            msg_noclose("What will you do?");
            int c = menu(19, 8, 11, opts, 2, 0, false);
            msg_close();
            if (c == 0) {
                say("", "You pull the silver bolt free and bind the wound. The wolf's growl softens into a low rumble.");
                say("LYCANTH", "*It presses its forehead to your hand. It wants to come with you.*");
                Mon m;
                mon_create(&m, SP_LYCANTH, 12);
                m.bond = 160;
                give_mon(&m);
                sfx(SFX_BIND_OK);
                msg("LYCANTH joined your team! (Sent to the Soul Vault if your team is full.)");
                sv.coexist += 2;
                set_flag(F_WOLF_SPARED);
            } else {
                say("", "You light a flare. Pale Sun hunters arrive within minutes and drag the wolf away in chains.");
                say("PALE SUN HUNTER", "Good work, Guild pup. Here's your bounty. Maybe there's hope for you yet.");
                sv.money += 1500;
                msg("You received $1500.");
                sv.items[IT_SILVER] = 1;
                sfx(SFX_ITEM);
                msg("You received the SILVER BLADE! (Equip it at the Guild weapon rack.)");
                sv.coexist -= 2;
            }
            set_flag(F_WOLF_DONE);
            world_refresh_npcs();
        }
        break;
    case S_WOODS_HUNTER: {
        if (flag(F_WOODS_HUNTER)) { say("HUNTER BRAM", "Squatchy prints everywhere out here. Never got a clear photo though."); break; }
        u8 sp[2] = { SP_MOTHKIN, SP_SQUATCHY }, lv[2] = { 6, 7 };
        Trainer *t = make_trainer("HUNTER BRAM", PS_HUNTER, 1, 300, "Fresh Guild blood! Let's see your bond in action!",
                                  "Good instincts. Keep them sharp.", false, 2, sp, lv);
        if (duel(t)) set_flag(F_WOODS_HUNTER);
        break;
    }
    case S_MIRA:
        say("???", "You're the hunter who's been poking around the woods. Don't worry - I'm not with the Pale Sun.");
        say("MIRA", "Name's Mira. I'm with the VEIL - we protect Monstermons that just want to live in peace.");
        say("MIRA", "The Pale Sun claims they're cleansing the world. But they knew the Nightfall was coming. They KNEW.");
        say("MIRA", "Ravencrest Manor up the hill was their old headquarters. The Guild only lets licensed hunters inside.");
        say("MIRA", "Take this. The fog in the Whisper Woods hides a glade the Pale Sun doesn't know about. A LANTERN cuts through it.");
        give_item(IT_LANTERN, 1);
        msg("Equip the LANTERN as your TOOL from the BAG to walk through supernatural fog.");
        set_flag(F_MET_MIRA);
        world_refresh_npcs();
        break;
    case S_GRAVEKEEPER:
        say("GRAVEKEEPER", "Spirits rise between the graves after dark. You'll need patience - and nightfall - to bind one.");
        say("GRAVEKEEPER", "They say Ravencrest's lord kept a silver sigil to open Duskhold Castle. Never saw it myself.");
        break;
    case S_GRAVE_HUNTER: {
        if (flag(F_GRAVE_HUNTER)) { say("MEDIUM OSRIC", "The dead are restless tonight. Mind your step."); break; }
        u8 sp[3] = { SP_WISPETTE, SP_BONEHOP, SP_GHOULET }, lv[3] = { 12, 12, 13 };
        Trainer *t = make_trainer("MEDIUM OSRIC", PS_HUNTER, 2, 800, "The spirits told me you'd come. They also said you'd lose!",
                                  "The spirits... lied?!", false, 3, sp, lv);
        if (duel(t)) set_flag(F_GRAVE_HUNTER);
        break;
    }
    case S_VEX_GRAVE: {
        say("VEX", "{P}! Took you long enough. I've been training out here in the fog. Ready for round two?");
        u8 sp[2] = { (u8)vex_species(0), SP_HEXCAT }, lv[2] = { 15, 13 };
        Trainer *t = make_trainer("VEX", PS_VEX, 2, 1200, 0,
                                  "Tch. Fine. I'll see you at the Trial Arena in Neon Row. I'm not losing THERE.", false, 2, sp, lv);
        if (duel(t)) { set_flag(F_VEX2); world_refresh_npcs(); }
        break;
    }
    case S_GATE_GUARD:
        if (flag(F_GOT_SIGIL)) {
            say("PALE SUN GUARD", "That's... the Moon Sigil of Ravencrest! Only the Commander's agents carry those. Y-you may pass!");
            set_flag(F_CASTLE_OPEN);
            world_refresh_npcs();
        } else {
            say("PALE SUN GUARD", "Duskhold Castle is under Pale Sun quarantine. No one passes without a sigil of authority.");
        }
        break;
    case S_MARKET: {
        say("NIGHT MARKET", "Psst. Rare goods for rare hunters. Ritual candles, dusk binds, treats your Monstermons will adore...");
        if (!flag(F_MARKET_GIFT)) {
            set_flag(F_MARKET_GIFT);
            say("NIGHT MARKET", "First visit? Have a candle on the house. Some spirits only bond inside a ritual flame.");
            give_item(IT_CANDLE, 1);
        }
        static const u8 stock[] = { IT_GREATBIND, IT_DUSKBIND, IT_ELIXIR, IT_REVIVE, IT_CANDLE, IT_OFFERING, IT_TREAT, IT_WARD, 0 };
        shop_screen(stock);
        world_reload_gfx();
        break;
    }
    case S_NEON_WOMAN:
        say("CLUBGOER", "When it rains, the billboards flicker with faces. Glitchost! Catch one and it'll never stop humming.");
        break;
    case S_NEON_KID:
        say("STREET KID", "My Impish stole my homework and set it on fire. Best day ever.");
        break;
    case S_NEON_HUNTER: {
        if (flag(F_NEON_HUNTER)) { say("RUNNER KAI", "Neon Row never sleeps. Neither do I!"); break; }
        u8 sp[2] = { SP_IMPISH, SP_GLITCHOST }, lv[2] = { 16, 16 };
        Trainer *t = make_trainer("RUNNER KAI", PS_HUNTER, 2, 1000, "Race you to the bottom of my Monstermons' HP bars!",
                                  "Whoa, you're fast!", false, 2, sp, lv);
        if (duel(t)) set_flag(F_NEON_HUNTER);
        break;
    }
    case S_JUDGE:
        if (flag(F_LICENSE)) { say("ARENA JUDGE", "Congratulations again, licensed hunter. The Trials are always open for practice."); break; }
        say("ARENA JUDGE", "Welcome to the Hunter Trials! Defeat the three Trialists, then face this season's champion.");
        say("ARENA JUDGE", "Trialists fight with full teams of three. Bring your best - and heal at the Sanctuary first!");
        break;
    case S_TRIAL1: case S_TRIAL2: case S_TRIAL3: {
        int idx = script - S_TRIAL1;
        int f = F_TRIAL1 + idx;
        static const char *const names[3] = { "TRIALIST FENN", "TRIALIST MORWEN", "TRIALIST GORDO" };
        if (flag(f)) { say(names[idx], "You've passed my trial. Go on!"); break; }
        static const u8 sps[3][3] = { { SP_SQUATCHY, SP_HEXCAT, SP_CHUPA }, { SP_WRAPLING, SP_BANSHRIEK, SP_CANDLEWICK },
                                      { SP_GARGOYL, SP_CERBERPUP, SP_LOCHLING } };
        static const u8 lvs[3][3] = { { 17, 17, 18 }, { 18, 18, 19 }, { 19, 19, 20 } };
        static const char *const intro[3] = { "First trial: raw power! Show me your team can take a hit!",
                                              "Second trial: the dead. Can your bonds outlast the grave?",
                                              "Final trial: stone and flame. No one gets past me!" };
        Trainer *t = make_trainer(names[idx], PS_HUNTER, 3, 1200, intro[idx], "Trial passed! Well fought.", false, 3,
                                  sps[idx], lvs[idx]);
        if (duel(t)) set_flag(f);
        break;
    }
    case S_VEX_ARENA: {
        if (flag(F_LICENSE)) { say("VEX", "Champion, huh? Don't get comfortable. I'll catch up."); break; }
        if (!flag(F_TRIAL1) || !flag(F_TRIAL2) || !flag(F_TRIAL3)) {
            say("VEX", "This season's champion is me, {P}. Beat all three Trialists first - then we'll settle this.");
            break;
        }
        u8 sp[3] = { (u8)vex_species(1), SP_CHIROVAMP, SP_HEXCAT }, lv[3] = { 23, 21, 21 };
        Trainer *t = make_trainer("CHAMPION VEX", PS_VEX, 3, 3000,
                                  "Full team, three on three. No excuses this time. Let's GO!",
                                  "...I lost. To you. Again. Okay. You earned this, {P}. For real.", true, 3, sp, lv);
        if (duel(t)) {
            set_flag(F_LICENSE);
            set_flag(F_TRIAL_VEX);
            say("ARENA JUDGE", "The champion falls! By the authority of the Hunter Guild, I grant you a HUNTER LICENSE!");
            give_item(IT_GREATBIND, 5);
            say("VEX", "Hey. That Mira person was asking about you. Said Ravencrest Manor holds the Pale Sun's secrets.");
        }
        break;
    }
    case S_CASTLE_G1: case S_CASTLE_G2: {
        int f = script == S_CASTLE_G1 ? F_CASTLE_G1 : F_CASTLE_G2;
        if (flag(f)) { say("PALE SUN ZEALOT", "The Commander will purify you herself..."); break; }
        Trainer *t;
        if (script == S_CASTLE_G1) {
            u8 sp[2] = { SP_GHOULORD, SP_HELLION }, lv[2] = { 25, 25 };
            t = make_trainer("PALE SUN ZEALOT", PS_GRUNT, 2, 1500, "An intruder in Duskhold! The Pale Sun will not fall!",
                             "Impossible...", false, 2, sp, lv);
        } else {
            u8 sp[3] = { SP_CERBERPUP, SP_GARGOYL, SP_WRAPLING }, lv[3] = { 26, 26, 26 };
            t = make_trainer("PALE SUN ZEALOT", PS_GRUNT, 3, 1500, "We bind monsters with chains, not friendship! Witness true control!",
                             "Chains... broken...", false, 3, sp, lv);
        }
        if (duel(t)) set_flag(f);
        break;
    }
    case S_SOLENNE: {
        say("COMMANDER SOLENNE", "So. The Guild's little stray found her way into my castle. Do you know what you're standing in, hunter?");
        say("COMMANDER SOLENNE", "Ten years ago, the Pale Sun opened the Rift beneath Duskhold. PROJECT DAWNBREAK. We sought the light of Eclipsa.");
        say("COMMANDER SOLENNE", "The ritual failed. The Nightfall was the result. A mistake... that we will now correct, by erasing every monster it let in.");
        u8 sp[4] = { SP_CERBERUS, SP_GHOULORD, SP_HELLION, SP_CHIROVAMP }, lv[4] = { 30, 29, 30, 29 };
        Trainer *t = make_trainer("COMMANDER SOLENNE", PS_SOLENNE, 3, 5000, "Your bonds are a sickness. I am the cure!",
                                  "No... NO! Then let the Hollow King finish you!", true, 4, sp, lv);
        if (!duel(t)) break;
        set_flag(F_BEAT_SOLENNE);
        say("COMMANDER SOLENNE", "Rise, HOLLOW KING! Your chains command you - DESTROY THEM!");
        say("HOLLOWKING", "*The undead king's crown-cage flares with Pale Sun runes. It lurches forward in agony.*");
        if (!boss_fight(SP_HOLLOWKING, 33)) break;
        set_flag(F_BEAT_HOLLOW);
        say("", "The Hollow King falls to one knee. The cage of runes around its crown begins to crack.");
        {
            static const char *const opts[2] = { "BIND IT", "FREE IT" };
            msg_noclose("The Hollow King is helpless. What will you do?");
            int c = menu(19, 8, 11, opts, 2, 1, false);
            msg_close();
            if (c == 0) {
                Mon m;
                mon_create(&m, SP_HOLLOWKING, 33);
                m.bond = 10;
                give_mon(&m);
                sfx(SFX_BIND_OK);
                msg("You Soulbind the Hollow King. It obeys... but its hollow eyes never leave you.");
                sv.coexist -= 1;
            } else {
                say("", "You shatter the rune-cage. The ancient king exhales a breath held for centuries... and crumbles peacefully to dust.");
                sv.coexist += 2;
                set_flag(F_HOLLOW_FREED);
                give_item(IT_ELIXIR, 3);
            }
        }
        say("", "The castle shudders. Behind the throne, the air tears open into swirling violet light - THE RIFT.");
        say("COMMANDER SOLENNE", "The Hollow King's chains were holding it shut... You fool! Eclipsa will wake!");
        say("", "Solenne flees into the dark.");
        set_flag(F_RIFT_OPEN);
        world_set_tile(14, 2, MT_PORTAL);
        world_refresh_npcs();
        break;
    }
    case S_HOLLOW:
        say("HOLLOWKING", "*It stands motionless behind the Commander, bound by chains of light.*");
        break;
    case S_MIRA_CASTLE:
        say("MIRA", "{P}! The Veil felt the Rift open from across the city. Eclipsa - the dragon of dusk and dawn - sleeps inside.");
        say("MIRA", "Whatever you decide in there... decide it with your heart, not your weapon. I'll heal your team.");
        heal_scene();
        set_flag(F_MIRA_CASTLE);
        break;
    case S_ECLIPSA:
        say("ECLIPSA", "*A vast shape uncoils in the dark. Half of its body burns gold, half drinks in the light.*");
        say("ECLIPSA", "Another human comes to take my light. Very well. Show me what your bonds are worth!");
        if (!boss_fight(SP_ECLIPSA, 40)) break;
        ending();
        break;
    }
    (void)buf;
}

// ------------------------------------------------------------------ tiles
static void clue(int f, const char *text, int x, int y) {
    if (flag(f)) return;
    say("", text);
    set_flag(f);
    world_set_tile(x, y, MT_TALLGRASS);
    int n = flag(F_CLUE1) + flag(F_CLUE2) + flag(F_CLUE3);
    if (n == 3) {
        say("", "Claw marks, silver-grey fur, heavy tracks... and blood. Everything leads to the flowered clearing in the north-west.");
        set_flag(F_CLUES_ALL);
        world_refresh_npcs();
    } else {
        msgf("Clues found: %d of 3.", n);
    }
}

static void brazier(int f, int order, int x, int y) {
    if (flag(f)) { msg("The brazier burns with a steady silver flame."); return; }
    if (!yesno("A cold iron brazier. Light it?")) return;
    if (sv.brazier_step != order) {
        msg("The flame flares... then a freezing wind howls through the hall, snuffing every brazier!");
        clr_flag(F_BRAZ_E); clr_flag(F_BRAZ_S); clr_flag(F_BRAZ_W); clr_flag(F_BRAZ_N);
        sv.brazier_step = 0;
        world_set_tile(15, 8, MT_BRAZIER); world_set_tile(9, 12, MT_BRAZIER);
        world_set_tile(3, 8, MT_BRAZIER); world_set_tile(9, 5, MT_BRAZIER);
        if (rnd_n(2)) {
            msg("Something stirs in the smoke!");
            int r = battle_wild(SP_GARGOYL, 20, -1, 0, false);
            world_reload_gfx();
            if (r == BR_LOSE) whiteout();
            else fade_from_black(12);
        }
        return;
    }
    sfx(SFX_BIND);
    set_flag(f);
    sv.brazier_step++;
    world_set_tile(x, y, MT_BRAZIERLIT);
    msg("The brazier blazes to life with a silver flame.");
    if (sv.brazier_step == 4) {
        sfx(SFX_DOOR);
        msg("A deep grinding echoes through the manor. The iron gate to the study swings open!");
        set_flag(F_GATE_OPEN);
        world_set_tile(9, 3, MT_WOODFLOOR);
        world_set_tile(10, 3, MT_WOODFLOOR);
    }
}

bool events_interact_tile(int x, int y, u8 mt) {
    int map = sv.map;
    switch (mt) {
    case MT_SIGN:
        if (map == MAP_HOLLOWMERE) msg("HOLLOWMERE - Last light before the Whisper Woods. Hunter Guild: south. Sanctuary: east.");
        else if (map == MAP_WOODS) msg("WHISPER WOODS. Warning: Pale Sun 'purification' zone. Stay on the path after dark.");
        else if (map == MAP_GRAVE) msg("GRAVE HILL. Ravencrest Manor (north) - Licensed hunters only. Neon Row: east.");
        else if (map == MAP_NEON) msg("NEON ROW - the city that never sleeps since the Nightfall. Trial Arena: south-east.");
        return true;
    case MT_CLUE:
        if (map == MAP_WOODS) {
            if (x == 5 && y == 25) clue(F_CLUE1, "Deep claw marks rake this tree trunk - too large for any normal wolf.", x, y);
            else if (x == 20 && y == 24) clue(F_CLUE2, "Tufts of silver-grey fur caught in the brambles. They smell of iron and smoke.", x, y);
            else clue(F_CLUE3, "Huge tracks, with a trail of dark blood... and a spent silver crossbow bolt marked with a sun.", x, y);
        }
        return true;
    case MT_BED:
        if (yesno("Rest in your bed? (Heals your team and lets time pass.)")) {
            static const char *const opts[3] = { "UNTIL MORNING", "UNTIL NIGHT", "CANCEL" };
            int c = menu(14, 6, 16, opts, 3, 0, true);
            if (c == 0 || c == 1) {
                fade_to_black(30);
                sv.clock = c == 0 ? 7 * 60 : 21 * 60;
                party_heal_all();
                sfx(SFX_HEAL);
                pal_refresh();
                fade_from_black(30);
                msg(c == 0 ? "You wake as dawn breaks over Hollowmere. Your team is refreshed." :
                             "You wake as the moon rises. Your team is refreshed. The night creatures stir...");
            }
        }
        return true;
    case MT_BOOKSHELF:
        if (map == MAP_HOME) msg("Your father's hunting journal: 'A Soulbind is a promise, not a leash. Never forget that.'");
        else if (map == MAP_GUILD) msg("SOULBIND THEORY: Weaken a Monstermon, then bind it. Spirits bond only at night; some need a ritual flame or an offering.");
        else if (map == MAP_SANCTUARY) msg("A hymnal. Someone has scribbled monster doodles in the margins.");
        else if (map == MAP_MANOR && x == 17) {
            msg("A torn note: 'Light the flames as the moon travels - it rises in the EAST, crosses the SOUTH sky, sets in the WEST, and sleeps in the NORTH.'");
            set_flag(F_MANOR_NOTE);
        } else if (map == MAP_MANOR) {
            msg("PALE SUN JOURNAL: 'Project Dawnbreak proceeds. Beneath Duskhold, the Rift widens. Eclipsa's light will be ours...'");
            msg("'...Subject has awakened. The sky is going dark. God help us, what have we done?' The rest is burned.");
        }
        return true;
    case MT_TABLE:
        if (map == MAP_GUILD && x == 3) { hunt_board(); world_reload_gfx(); }
        else if (map == MAP_GUILD) { equip_screen(); world_reload_gfx(); }
        else if (map == MAP_MANOR) {
            if (!flag(F_GOT_SIGIL)) {
                msg("On the desk lies a silver seal shaped like a crescent moon, beside a stack of Pale Sun orders.");
                give_item(IT_SIGIL, 1);
                set_flag(F_GOT_SIGIL);
                msg("The orders mention Duskhold Castle, north of Neon Row. The sigil should get you past the guards.");
            } else msg("Stacks of Pale Sun orders. 'Purify the Woods.' 'Silence the Veil.' 'Prepare the Hollow King.'");
        } else msg("A cold cup of tea sits forgotten on the table.");
        return true;
    case MT_ALTAR:
        if (map == MAP_SANCTUARY && x == 1) { vault_screen(); world_reload_gfx(); }
        else if (map == MAP_SANCTUARY) {
            if (yesno("Pray at the shrine to record your journey? (Save)")) {
                save_write();
                sfx(SFX_HEAL);
                msg("Your journey has been recorded.");
            }
        }
        return true;
    case MT_GRAVE:
        if (map == MAP_HOLLOWMERE) msg("THE NIGHTFALL MEMORIAL. 'For the 412 souls lost when the moon went dark. We remember.'");
        else {
            static const char *const ep[4] = { "'Here lies Old Tom. He did not believe in ghosts. They believed in him.'",
                                               "'Beloved mother. Please stay buried this time.'",
                                               "The name has been scratched away by claws.",
                                               "'Gone hunting. Back never.'" };
            msg(ep[(x + y) & 3]);
        }
        return true;
    case MT_MOONPOOL:
        if (!flag(F_GLADE_ITEM)) {
            msg("The moon pool glows softly. Something glints at the bottom...");
            give_item(IT_ELIXIR, 2);
            give_item(IT_TREAT, 1);
            set_flag(F_GLADE_ITEM);
        } else {
            msgf("Your reflection ripples in the moonlit water. It is %s.", is_night() ? "night" : "day");
        }
        return true;
    case MT_BRAZIER: case MT_BRAZIERLIT:
        if (map != MAP_MANOR) return false;
        if (x == 15 && y == 8) brazier(F_BRAZ_E, 0, x, y);
        else if (x == 9 && y == 12) brazier(F_BRAZ_S, 1, x, y);
        else if (x == 3 && y == 8) brazier(F_BRAZ_W, 2, x, y);
        else brazier(F_BRAZ_N, 3, x, y);
        return true;
    case MT_GATE:
        msg("A heavy iron gate. Four cold braziers surround the great hall...");
        return true;
    case MT_CRATE:
        if (map == MAP_HOME && !flag(F_HOME_ITEM)) {
            msg("You rummage through your old hunting crate...");
            give_item(IT_SOULBIND, 2);
            set_flag(F_HOME_ITEM);
        } else msg("Dusty crates full of old hunting gear.");
        return true;
    case MT_THRONE:
        msg("A throne of black stone, carved with suns being devoured by the moon.");
        return true;
    case MT_STATUE:
        msg("A gargoyle statue. Its stone eyes seem to follow you.");
        return true;
    case MT_WATER:
        msg("Dark water. Something ripples beneath the surface...");
        return true;
    case MT_WINDOW:
        msg("Warm candlelight spills from inside.");
        return true;
    case MT_NEONPINK: case MT_NEONCYAN: case MT_NEONVIOLET:
        msg("The neon sign buzzes and flickers: OPEN ALL NIGHT.");
        return true;
    }
    return false;
}

// ------------------------------------------------------------------ steps & map entry
bool events_step(int x, int y) {
    int map = sv.map;
    if (map == MAP_HOLLOWMERE && y <= 1 && !flag(F_GOT_STARTER)) {
        say("WARDEN ASH", "{P}! Don't wander into the woods without a partner! Come to the Guild Hall first.");
        world_set_player(x, 2, DIR_DOWN);
        return true;
    }
    if (map == MAP_GRAVE && y == 2 && (x == 11 || x == 12) && !flag(F_LICENSE)) {
        msg("The manor gate bears the Hunter Guild's seal: LICENSED HUNTERS ONLY.");
        world_set_player(x, 3, DIR_DOWN);
        return true;
    }
    if (map == MAP_CASTLE && x == 14 && y == 2 && !flag(F_RIFT_OPEN)) return true;
    return false;
}

void events_apply_map_patches(void) {
    switch (sv.map) {
    case MAP_WOODS:
        if (flag(F_CLUE1)) world_set_tile(5, 25, MT_TALLGRASS);
        if (flag(F_CLUE2)) world_set_tile(20, 24, MT_TALLGRASS);
        if (flag(F_CLUE3)) world_set_tile(6, 16, MT_TALLGRASS);
        break;
    case MAP_MANOR:
        if (flag(F_BRAZ_E)) world_set_tile(15, 8, MT_BRAZIERLIT);
        if (flag(F_BRAZ_S)) world_set_tile(9, 12, MT_BRAZIERLIT);
        if (flag(F_BRAZ_W)) world_set_tile(3, 8, MT_BRAZIERLIT);
        if (flag(F_BRAZ_N)) world_set_tile(9, 5, MT_BRAZIERLIT);
        if (flag(F_GATE_OPEN)) { world_set_tile(9, 3, MT_WOODFLOOR); world_set_tile(10, 3, MT_WOODFLOOR); }
        break;
    case MAP_CASTLE:
        if (flag(F_RIFT_OPEN)) world_set_tile(14, 2, MT_PORTAL);
        break;
    }
}

void events_map_enter(void) {
    switch (sv.map) {
    case MAP_HOME:
        if (!flag(F_INTRO)) {
            set_flag(F_INTRO);
            msg("Church bells ring across Hollowmere. Warden Ash asked you to come to the Guild Hall - south of the plaza.");
            msg("(Tip: press START for the menu. A talks and examines. Hold B to run.)");
        }
        break;
    case MAP_GUILD:
        if (!flag(F_GOT_STARTER)) {
            say("WARDEN ASH", "Ah, {P}! Come, come - over here.");
        }
        break;
    case MAP_WOODS:
        if (!flag(F_SEEN_WOODS)) {
            set_flag(F_SEEN_WOODS);
            msg("The trees creak and whisper. Tall grass hides wild Monstermons - and somewhere here, something is wounded.");
        }
        break;
    case MAP_GRAVE:
        if (!flag(F_SEEN_GRAVE)) { set_flag(F_SEEN_GRAVE); msg("Mist coils between the headstones. Ravencrest Manor looms on the hill."); }
        break;
    case MAP_NEON:
        if (!flag(F_SEEN_NEON)) { set_flag(F_SEEN_NEON); msg("Neon hums over wet asphalt. The city never sleeps - and neither do its monsters."); }
        break;
    case MAP_MANOR:
        if (!flag(F_SEEN_MANOR)) {
            set_flag(F_SEEN_MANOR);
            msg("Dust and candle-smoke. Four braziers stand cold around the great hall. An iron gate guards the study.");
        }
        break;
    case MAP_CASTLE:
        if (!flag(F_SEEN_CASTLE)) { set_flag(F_SEEN_CASTLE); msg("Duskhold Castle. The air hums with Pale Sun runes... and something ancient groans in chains."); }
        break;
    case MAP_RIFT:
        if (!flag(F_SEEN_RIFT)) { set_flag(F_SEEN_RIFT); msg("You step into the Rift. Stars swim beneath your feet. Gravity is a suggestion here."); }
        break;
    }
}
