// MonsterMons - entry point, frame loop, title screen and new-game intro
#include <string.h>
#include "core.h"

Save sv;
u16 key_cur, key_prev;
u32 frame_count;
bool clock_running;
static u32 rng_state = 0x6D2B79F5;
static int clock_sub;

u32 rnd(void) {
    // xorshift32
    u32 x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

int rnd_n(int n) { return n <= 1 ? 0 : (int)((rnd() >> 8) % (u32)n); }

int clock_hour(void) { return sv.clock / 60; }
bool is_night(void) {
    int h = clock_hour();
    return h >= 20 || h < 6;
}

void frame(void) {
    vblank_wait();
    dma_copy32(OAM, oam_buf, 128 * 2);
    key_prev = key_cur;
    key_cur = ~REG_KEYINPUT & 0x3FF;
    sound_update();
    frame_count++;
    if (key_cur) rng_state ^= frame_count * 2654435761u;
    rnd();
    if (clock_running) {
        sv.play_frames++;
        if (++clock_sub >= 30) {
            clock_sub = 0;
            sv.clock = (sv.clock + 1) % 1440;
            if (sv.clock % 60 == 0) pal_refresh();
        }
    }
}

static void load_scene_bg(int id) {
    const SceneDef *s = &scene_defs[id];
    dma_copy32(CHARBLOCK(CB_SCENE), s->tiles, s->ntiles * 8);
    vu16 *sb = SCREENBLOCK(SB_MAP);
    dma_fill32(CHARBLOCK(CB_SCENE) + 511 * 16, 0, 8);
    for (int i = 0; i < 32 * 32; i++) sb[i] = 511 | (BANK_SCENE << 12);
    for (int y = 0; y < s->h; y++)
        for (int x = 0; x < s->w; x++) sb[y * 32 + x] = s->map[y * s->w + x] | (BANK_SCENE << 12);
    for (int i = 0; i < 16; i++) PAL_BG[BANK_SCENE * 16 + i] = s->pal[i];
    REG_BGCNT(1) = BG_PRIO(3) | BG_CBB(CB_SCENE) | BG_SBB(SB_MAP);
    REG_BGHOFS(1) = 0;
    REG_BGVOFS(1) = 0;
}

static void intro_story(void) {
    txt_clear();
    oam_clear();
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_OBJ;
    PAL_BG[0] = 0;
    fade_from_black(20);
    static const char *const pages[] = {
        "Ten years ago, the moon went dark for a single night.",
        "We call it the NIGHTFALL. When dawn came, the world was no longer ours alone.",
        "Vampires. Werewolves. Ghosts. Demons. Cryptids from old campfire tales... all of them real.",
        "Some hunted us. Some hid from us. Most simply wanted to live.",
        "Then the Hunter Guild created the SOULBIND - a way to bond a creature's soul to a human's.",
        "Bound creatures became partners. Friends. We named them MONSTERMONS.",
        "Not everyone agrees. The ORDER OF THE PALE SUN believes every Monstermon must be destroyed.",
        "Tonight, in the town of Hollowmere, a rookie hunter's story begins...",
    };
    for (unsigned i = 0; i < sizeof(pages) / sizeof(pages[0]); i++) msg(pages[i]);
}

static void new_game(void) {
    memset(&sv, 0, sizeof(sv));
    sv.magic = 0x4D4F4E31;
    sv.coat = 0;
    sv.hair = 0;
    sv.money = 500;
    sv.clock = 19 * 60 + 30;
    sv.weapon = IT_KNIFE;
    sv.items[IT_KNIFE] = 1;
    sv.items[IT_TONIC] = 2;
    sv.coexist = 0;
    intro_story();
    say("WARDEN ASH", "Before you sign the Guild ledger... what name do you hunt under?");
    name_entry(sv.name, 7, "YOUR NAME?");
    customize_hunter();
    say("WARDEN ASH", "{P}. A good name for a hunter. Rest up at home, then come see me at the Guild Hall.");
    fade_to_black(20);
    sv.map = MAP_HOME;
    sv.px = 4;
    sv.py = 3;
    sv.facing = 0;
    sv.last_heal_map = MAP_HOME;
    sv.last_heal_x = 4;
    sv.last_heal_y = 3;
}

static int title_screen(void) {
    bool has_save = save_exists();
    txt_clear();
    oam_clear();
    load_scene_bg(SCENE_TITLE);
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ;
    bgm(BGM_TITLE);
    fade_from_black(30);
    txt_put(8, 6, "SUPERNATURAL HUNT", 1);
    int t = 0;
    while (1) {
        frame();
        t++;
        if ((t >> 5) & 1) txt_put(9, 15, "PRESS  START", 0);
        else win_clear(9, 15, 12, 1);
        txt_put(4, 18, "(C) 2026 MONSTERMONS DEV", 2);
        if (key_hit(KEY_START) || key_hit(KEY_A)) break;
    }
    sfx(SFX_OK);
    win_clear(9, 15, 12, 1);
    int choice = 0;
    if (has_save) {
        static const char *const opts[] = { "CONTINUE", "NEW GAME" };
        while (1) {
            int r = menu(9, 11, 12, opts, 2, 0, false);
            if (r == 0) { choice = 1; break; }
            if (yesno("Start a new hunt? Your old save will be overwritten when you save.")) { choice = 0; break; }
        }
    }
    fade_to_black(30);
    bgm_stop();
    txt_clear();
    return choice;
}

int main(void) {
    REG_WAITCNT = 0x4317;
    REG_DISPSTAT = 1 << 3;
    REG_IE = 1;
    REG_IME = 1;
    sound_init();
    memset(&sv, 0, sizeof(sv));
    gfx_init();
    set_brightness(16);
    if (title_screen() == 1 && save_read()) {
        load_player_palette();
    } else {
        gfx_init();
        set_brightness(16);
        new_game();
    }
    gfx_init();
    set_brightness(16);
    world_enter(sv.map, sv.px, sv.py, sv.facing);
    world_run();
    return 0;
}
