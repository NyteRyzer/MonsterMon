// MonsterMons - shared engine declarations
#ifndef CORE_H
#define CORE_H

#include "gba.h"
#include "assets.h"

// ------------------------------------------------------------- types
enum { T_BEAST, T_BLOOD, T_SHADE, T_INFERNAL, T_CRYPTID, T_ARCANE, T_UNDEAD, T_LUMIN, T_COUNT, T_NONE = 0xFF };

enum { ST_NONE, ST_BURN, ST_CURSE, ST_SLEEP, ST_FEAR };
enum { NAT_BRAVE, NAT_CALM, NAT_SWIFT, NAT_MYSTIC, NAT_LOYAL, NAT_COUNT };

enum { TGT_FOE, TGT_ALL_FOES, TGT_SELF, TGT_ALLY };
enum { EF_NONE, EF_BURN, EF_CURSE, EF_SLEEP, EF_FEAR, EF_DRAIN, EF_ATKUP, EF_DEFUP, EF_SPCUP, EF_SPDUP,
       EF_ATKDOWN, EF_HEAL, EF_RECOIL };

enum { EVO_NONE, EVO_LEVEL, EVO_DAYNIGHT, EVO_BOND, EVO_NIGHT, EVO_FANG };
#define CAP_NIGHT  1
#define CAP_RITUAL 2
#define CAP_TRUST  4
#define CAP_BOSS   8

typedef struct {
    const char *name;
    u8 t1, t2;
    u8 hp, atk, def, spd, spc;
    u8 catch_rate, xp_yield, capflags;
    u8 evo_kind, evo_lv, evo_a, evo_b;
    const u8 *learn;           // pairs (level, move), 0-terminated
    const char *lore;
    const char *habitat;
} Species;

typedef struct {
    const char *name;
    u8 type, power, acc, pp, effect, chance, target;
} Move;

enum {
    IT_NONE, IT_SOULBIND, IT_GREATBIND, IT_DUSKBIND, IT_TONIC, IT_ELIXIR, IT_REVIVE, IT_SALT, IT_CANDLE,
    IT_OFFERING, IT_TREAT, IT_SIGIL, IT_LANTERN, IT_LENS, IT_WARD, IT_KNIFE, IT_SILVER, IT_CROSSBOW, IT_STAFF,
    IT_COUNT
};
enum { IK_BIND, IK_HEAL, IK_BATTLE, IK_KEY, IK_TOOL, IK_WEAPON, IK_FIELD };
typedef struct { const char *name; u8 kind; u16 price; const char *desc; } Item;

typedef struct {
    u8 species, level, variant, bond;
    u8 moves[4];
    u8 pp[4];
    u16 hp;
    u8 status, nature;
    u32 xp;
} Mon;

#define FLAG_BYTES 32
#define VAULT_MAX 40
typedef struct {
    u32 magic;
    char name[8];
    u8 coat, hair;
    u8 map, px, py, facing;
    u32 money;
    u32 play_frames;
    u16 clock;              // minutes since midnight
    u8 weather, weapon, tool;
    s8 coexist;
    u8 party_n, vault_n;
    Mon party[6];
    Mon vault[VAULT_MAX];
    u8 items[IT_COUNT];
    u8 flags[FLAG_BYTES];
    u8 seen[8], caught[8];
    u16 wild_defeated;
    u8 ending;
    u8 last_heal_map, last_heal_x, last_heal_y;
    u8 brazier_step;
    u8 starter;
    u32 checksum;
} Save;

extern Save sv;

// story / world flags
enum {
    F_INTRO, F_GOT_STARTER, F_BEAT_VEX1, F_CLUE1, F_CLUE2, F_CLUE3, F_WOLF_DONE, F_WOLF_SPARED,
    F_WOODS_GRUNTS, F_MET_MIRA, F_TRIAL1, F_TRIAL2, F_TRIAL3, F_LICENSE, F_BRAZ_E, F_BRAZ_S,
    F_BRAZ_W, F_BRAZ_N, F_GATE_OPEN, F_GOT_SIGIL, F_CASTLE_OPEN, F_BEAT_SOLENNE, F_BEAT_HOLLOW, F_HOLLOW_FREED,
    F_RIFT_OPEN, F_BEAT_ECLIPSA, F_GAME_CLEAR, F_GLADE_ITEM, F_BOUNTY1, F_BOUNTY2, F_BOUNTY3, F_BOUNTY4,
    F_ROOK, F_CASTLE_G1, F_CASTLE_G2, F_VEX_FINAL, F_MANOR_NOTE, F_MARKET_GIFT, F_GRAVE_HUNTER, F_NEON_HUNTER,
    F_SEEN_WOODS, F_SEEN_GRAVE, F_SEEN_NEON, F_SEEN_MANOR, F_SEEN_CASTLE, F_SEEN_RIFT, F_CLUES_ALL, F_VEX2,
    F_WOODS_HUNTER, F_HOME_ITEM, F_MIRA_CASTLE, F_TRIAL_VEX, F_ECLIPSA_JOINED,
    F_T_SIA, F_T_OTTO, F_T_ELSPETH, F_T_HOLT, F_T_RAVEN, F_T_VEY,
    F_SEAL_WILD, F_SEAL_DEAD, F_SEAL_NEON, F_SEAL_DAWN, F_COUNCIL,
    F_ITEM0, F_ITEM_LAST = F_ITEM0 + 15,
    F_COUNT
};
static inline bool flag(int f) { return (sv.flags[f >> 3] >> (f & 7)) & 1; }
static inline void set_flag(int f) { sv.flags[f >> 3] |= 1 << (f & 7); }
static inline void clr_flag(int f) { sv.flags[f >> 3] &= ~(1 << (f & 7)); }

// ------------------------------------------------------------- data (data.c)
extern const Species species[SPECIES_COUNT];
extern const Move moves[];
extern const Item items[IT_COUNT];
extern const char *const type_names[T_COUNT];
extern const char *const nature_names[NAT_COUNT];
extern const char *const status_names[5];
int type_eff(int atk, int def);   // 0 = half, 1 = normal, 2 = double (x2 per type, combined *4 scale)
int type_mult4(int mtype, int t1, int t2); // returns multiplier * 4
enum { AB_NONE, AB_DREAD, AB_BLOODLUST, AB_THICKHIDE, AB_NOCTURNAL, AB_SUNBORN, AB_LASTSTAND, AB_PHANTOM,
       AB_REGROWTH, AB_SOULSURGE, AB_COUNT };
extern const char *const ability_names[AB_COUNT];
extern const char *const ability_desc[AB_COUNT];
extern const u8 species_ability[SPECIES_COUNT];
#define MV_SOULCOMBO 41
#define MOVE_COUNT 42

// ------------------------------------------------------------- mon.c
void mon_create(Mon *m, int sp, int level);
int mon_maxhp(const Mon *m);
int mon_stat(const Mon *m, int which);  // 0 atk 1 def 2 spd 3 spc
u32 xp_for_level(int lv);
int mon_gain_xp(Mon *m, u32 xp, bool show);  // returns levels gained (shows messages)
int mon_evolve_target(const Mon *m);
void mon_heal(Mon *m);
void party_heal_all(void);
int party_alive_count(void);
bool give_mon(const Mon *m);    // party, else vault; false if full
void mark_seen(int sp);
void mark_caught(int sp);
bool is_seen(int sp);
bool is_caught(int sp);
int caught_count(void);
const char *mon_name(const Mon *m);
void mon_learn_message(Mon *m, int mv);
void mon_try_evolve(Mon *m);

// ------------------------------------------------------------- system (main.c)
extern u16 key_cur, key_prev;
extern u32 frame_count;
extern bool clock_running;
void frame(void);
static inline bool key_hit(u16 k) { return (key_cur & k) && !(key_prev & k); }
static inline bool key_held(u16 k) { return key_cur & k; }
u16 key_repeat(void);
u32 rnd(void);
int rnd_n(int n);
bool is_night(void);
int clock_hour(void);

// ------------------------------------------------------------- gfx / ui (ui.c)
#define SB_TEXT 31
#define SB_MAP 28
#define SB_FOG 29
#define CB_MAP 0
#define CB_UI 1
#define CB_SCENE 2
#define BANK_SCENE 14
typedef struct { u16 a0, a1, a2, a3; } OamEntry;
extern OamEntry oam_buf[128];
void gfx_init(void);
void oam_clear(void);
void spr(int i, int x, int y, int shape, int size, int tile, int pal, int prio, bool hflip);
void spr_affine(int i, int x, int y, int shape, int size, int tile, int pal, int prio, int aff, bool dbl);
void aff_set(int n, int pa, int pb, int pc, int pd);
void txt_fill(u16 entry);
void txt_clear(void);
void txt_put(int x, int y, const char *s, int pal);
void txt_putc(int x, int y, char c, int pal);
void txt_num(int x, int y, int n, int width, int pal);
void txt_rect(int x, int y, int w, int h, u16 entry);
void win_draw(int x, int y, int w, int h);
void win_clear(int x, int y, int w, int h);
void hp_bar(int x, int y, int tiles, int cur, int max);
void exp_bar(int x, int y, int tiles, u32 cur, u32 max);
void fade_to_black(int frames);
void fade_from_black(int frames);
void set_brightness(int level);    // 0 normal .. 16 black
void fmt(char *out, const char *f, ...);
void msg(const char *s);                   // bottom dialog box, waits
void msgf(const char *f, ...);
void say(const char *who, const char *s); // speaker-labelled dialog
void msg_noclose(const char *s);
void msg_close(void);
int menu(int x, int y, int w, const char *const *opts, int n, int start, bool cancel);
bool yesno(const char *q);
void wait_frames(int n);
void pal_load_bg_banks(void);
void pal_refresh(void);   // applies time of day / weather tint
extern bool tint_enabled;
extern bool world_obj_pals;
extern int msg_auto;
void load_person_gfx(void);
void load_player_palette(void);
void show_mon_preview(int sp, int variant, int x, int y, int slot, bool big);

// ------------------------------------------------------------- sound.c
enum { SFX_CURSOR, SFX_OK, SFX_BACK, SFX_HIT, SFX_SUPER, SFX_WEAK, SFX_FAINT, SFX_BIND, SFX_BIND_OK, SFX_LEVEL,
       SFX_HEAL, SFX_DOOR, SFX_BUMP, SFX_ENCOUNTER, SFX_ITEM, SFX_EVOLVE };
enum { BGM_TOWN, BGM_FIELD, BGM_DUNGEON, BGM_RIFT, BGM_BATTLE, BGM_BOSS, BGM_TITLE, BGM_VICTORY, BGM_NONE = 0xFF };
void sound_init(void);
void sound_update(void);
void sfx(int id);
void bgm(int id);
void bgm_stop(void);

// ------------------------------------------------------------- battle.c
typedef struct {
    const char *name;     // trainer name (NULL for wild)
    u8 sprite;
    u8 n, max_active;
    u8 sp[6], lv[6];
    u16 prize;
    const char *intro, *lose;
    bool boss;
} Trainer;
enum { BR_WIN, BR_LOSE, BR_FLED, BR_CAUGHT };
int battle_wild(int sp1, int lv1, int sp2, int lv2, bool boss);
int battle_trainer(const Trainer *t);
extern u8 last_caught_slot;

// ------------------------------------------------------------- world.c / events.c
extern u8 weather_now;
enum { WX_CLEAR, WX_RAIN, WX_FOG, WX_BLOODMOON };
void world_run(void);
void world_enter(int map, int x, int y, int facing);
void world_redraw(void);
void world_reload_gfx(void);
u8 world_tile(int x, int y);
void world_set_tile(int x, int y, u8 mt);
void events_apply_map_patches(void);
bool events_step(int x, int y);
bool events_interact_tile(int x, int y, u8 mt);
void events_npc(int script);
void events_map_enter(void);
void whiteout(void);

// ------------------------------------------------------------- menus.c
void start_menu(void);
int party_pick(const char *prompt, bool allow_cancel);
void party_screen(void);
void bag_screen(bool in_battle, int *out_item);
void bestiary_screen(void);
void hunter_card(void);
void vault_screen(void);
void shop_screen(const u8 *stock);
void hunt_board(void);
void equip_screen(void);
void name_entry(char *out, int maxlen, const char *prompt);
void customize_hunter(void);
void mon_summary(Mon *m);

// ------------------------------------------------------------- save.c
bool save_exists(void);
void save_write(void);
bool save_read(void);

#endif
