// MonsterMons - PSG music sequencer and sound effects
#include "core.h"

typedef struct { const char *mel, *bass; u8 step; } Song;

static const Song songs[] = {
    // BGM_TOWN
    { "E5:2 C5:2 A4:2 C5:2 D5:2 B4:2 G4:2 B4:2 C5:2 A4:2 F4:2 A4:2 B4:4 G#4:4 "
      "E5:2 C5:2 A4:2 E5:2 F5:2 D5:2 B4:2 D5:2 E5:2 C5:2 A4:2 C5:2 B4:4 A4:4",
      "A2:8 G2:8 F2:8 E2:8 A2:8 D2:8 F2:4 E2:4 A2:8", 10 },
    // BGM_FIELD
    { "A4:3 A4:1 C5:2 E5:2 D5:3 C5:1 B4:4 G4:3 G4:1 B4:2 D5:2 C5:3 B4:1 A4:4 "
      "A4:3 A4:1 C5:2 E5:2 G5:3 F5:1 E5:4 D5:2 C5:2 B4:2 G#4:2 A4:8",
      "A2:4 E3:4 G2:4 D3:4 F2:4 C3:4 E2:4 B2:4 A2:4 E3:4 C3:4 G2:4 D3:4 A2:4 E2:8", 8 },
    // BGM_DUNGEON
    { "E4:6 F4:2 E4:4 R:4 C5:6 B4:2 A#4:4 R:4 E4:6 F4:2 G4:4 F4:4 E4:16",
      "E2:16 F2:16 E2:16 D#2:8 E2:8", 11 },
    // BGM_RIFT
    { "B4:2 F#5:2 E5:2 B4:2 D5:4 C#5:4 B4:2 F#5:2 G5:2 F#5:2 E5:8 "
      "D5:2 E5:2 F#5:2 A5:2 G5:4 F#5:4 E5:2 D5:2 C#5:2 A4:2 B4:8",
      "B2:8 G2:8 E2:8 F#2:8 B2:8 G2:8 A2:8 F#2:8", 9 },
    // BGM_BATTLE
    { "A4:2 A4:1 A4:1 C5:2 A4:2 D#5:2 E5:2 C5:2 A4:2 G4:2 G4:1 G4:1 B4:2 G4:2 D5:2 C5:2 B4:2 G4:2 "
      "F4:2 F4:1 F4:1 A4:2 F4:2 C5:2 B4:2 A4:2 F4:2 E4:2 G#4:2 B4:2 E5:2 D5:2 C5:2 B4:2 G#4:2",
      "A2:2 A3:2 A2:2 A3:2 A2:2 A3:2 A2:2 A3:2 G2:2 G3:2 G2:2 G3:2 G2:2 G3:2 G2:2 G3:2 "
      "F2:2 F3:2 F2:2 F3:2 F2:2 F3:2 F2:2 F3:2 E2:2 E3:2 E2:2 E3:2 E2:2 E3:2 E2:2 E3:2", 6 },
    // BGM_BOSS
    { "E5:1 D#5:1 E5:1 B4:1 C5:2 A4:2 E5:1 D#5:1 E5:1 B4:1 D5:2 B4:2 C5:2 E5:2 A5:4 G#5:4",
      "A2:2 A3:2 A2:2 A3:2 G#2:2 G#3:2 G#2:2 G#3:2 F2:2 F3:2 F2:2 F3:2 E2:2 E3:2 E2:2 E3:2", 6 },
    // BGM_TITLE
    { "A4:4 E5:4 D5:2 C5:2 B4:4 C5:2 B4:2 A4:2 G#4:2 A4:8 R:4 E4:4 A4:2 B4:2 C5:4 D5:2 E5:2 F5:4 E5:4 D5:2 C5:2 B4:8",
      "A2:8 F2:8 G2:8 E2:8 A2:8 F2:8 D2:8 E2:8 E2:8", 12 },
    // BGM_VICTORY
    { "C5:2 E5:2 G5:2 C6:6 B5:2 G5:2 A5:2 B5:2 C6:8 R:4", "C3:8 G2:8 A2:4 G2:4 C3:8", 7 },
};

#define MAXN 96
typedef struct { u8 note[MAXN], len[MAXN]; u8 n; } Track;
static Track mel_t, bass_t;
static u8 mel_i, bass_i, mel_left, bass_left, step_frames, step_ctr;
static int cur_song = BGM_NONE;
static u16 sq_rate[128];
static u16 wv_rate[128];

static void parse(const char *s, Track *t) {
    static const s8 base[7] = { 9, 11, 0, 2, 4, 5, 7 }; // A B C D E F G
    t->n = 0;
    while (*s && t->n < MAXN) {
        while (*s == ' ') s++;
        if (!*s) break;
        int note = 0;
        if (*s == 'R') { s++; }
        else {
            int b = base[*s - 'A'];
            s++;
            if (*s == '#') { b++; s++; }
            int oct = *s++ - '0';
            note = (oct + 1) * 12 + b;
        }
        int len = 0;
        if (*s == ':') {
            s++;
            while (*s >= '0' && *s <= '9') len = len * 10 + (*s++ - '0');
        }
        t->note[t->n] = note;
        t->len[t->n] = len ? len : 1;
        t->n++;
    }
}

void sound_init(void) {
    REG_SOUNDCNT_X = 0x80;
    REG_SOUNDCNT_L = 0xFF77;
    REG_SOUNDCNT_H = 0x0002;
    // frequency tables: f = 440 * 2^((n-69)/12)
    static const u32 c4x100[12] = { 26163, 27718, 29366, 31113, 32963, 34923, 36999, 39200, 41530, 44000, 46616, 49388 };
    for (int n = 24; n < 108; n++) {
        int oct = n / 12 - 1, k = n % 12;
        u32 f100 = c4x100[k];
        if (oct >= 4) f100 <<= (oct - 4);
        else f100 >>= (4 - oct);
        u32 r = 2048 - (131072u * 100u) / f100;
        u32 w = 2048 - (65536u * 100u) / f100;
        sq_rate[n] = r > 2047 ? 2047 : r;
        wv_rate[n] = w > 2047 ? 0 : w;
    }
    // wave RAM: soft triangle bass
    REG_SOUND3CNT_L = 0x40;
    static const u32 wave[4] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
    for (int i = 0; i < 4; i++) WAVE_RAM[i] = wave[i];
    REG_SOUND3CNT_L = 0x80;
    REG_SOUND3CNT_H = 0x4000;
}

void bgm(int id) {
    if (id == cur_song) return;
    cur_song = id;
    if (id == BGM_NONE) { bgm_stop(); return; }
    parse(songs[id].mel, &mel_t);
    parse(songs[id].bass, &bass_t);
    step_frames = songs[id].step;
    mel_i = bass_i = 0;
    mel_left = bass_left = 0;
    step_ctr = 0;
}

void bgm_stop(void) {
    cur_song = BGM_NONE;
    mel_t.n = bass_t.n = 0;
    REG_SOUND2CNT_L = 0;
    REG_SOUND2CNT_H = 0x8000;
    REG_SOUND3CNT_H = 0;
}

// sfx: short arpeggio on channel 1 or a noise burst on channel 4
typedef struct { u8 notes[6]; u8 frames; u8 noise; u16 env; } Sfx;
static const Sfx sfx_tab[] = {
    { { 84 }, 2, 0, 0x6180 },                      // CURSOR
    { { 79, 86 }, 3, 0, 0x8180 },                  // OK
    { { 72, 67 }, 3, 0, 0x8180 },                  // BACK
    { { 0 }, 0, 0x34, 0xC200 },                    // HIT
    { { 0 }, 0, 0x26, 0xF300 },                    // SUPER
    { { 0 }, 0, 0x56, 0x8100 },                    // WEAK
    { { 72, 67, 62, 55, 48 }, 5, 0, 0xA2C0 },      // FAINT
    { { 60, 67, 72 }, 4, 0, 0xA180 },              // BIND
    { { 72, 76, 79, 84, 88, 91 }, 5, 0, 0xC280 },  // BIND_OK
    { { 72, 76, 79, 84 }, 6, 0, 0xC280 },          // LEVEL
    { { 76, 79, 84, 88 }, 4, 0, 0xA180 },          // HEAL
    { { 0 }, 0, 0x71, 0x9100 },                    // DOOR
    { { 43 }, 4, 0, 0x7180 },                      // BUMP
    { { 64, 70, 64, 70, 76, 82 }, 3, 0, 0xC280 },  // ENCOUNTER
    { { 79, 84, 91 }, 4, 0, 0xB180 },              // ITEM
    { { 60, 64, 67, 72, 76, 79 }, 7, 0, 0xC380 },  // EVOLVE
};
static const Sfx *cur_sfx;
static u8 sfx_i, sfx_t;

void sfx(int id) {
    const Sfx *s = &sfx_tab[id];
    if (s->noise) {
        REG_SOUND4CNT_L = s->env;
        REG_SOUND4CNT_H = 0x8000 | s->noise;
        return;
    }
    cur_sfx = s;
    sfx_i = 0;
    sfx_t = 0;
}

void sound_update(void) {
    if (cur_sfx) {
        if (sfx_t == 0) {
            u8 n = sfx_i < 6 ? cur_sfx->notes[sfx_i] : 0;
            if (!n) cur_sfx = 0;
            else {
                REG_SOUND1CNT_L = 0;
                REG_SOUND1CNT_H = cur_sfx->env;
                REG_SOUND1CNT_X = sq_rate[n] | 0x8000;
            }
            sfx_i++;
        }
        if (cur_sfx && ++sfx_t >= cur_sfx->frames) sfx_t = 0;
    }
    if (!mel_t.n) return;
    if (++step_ctr < step_frames) return;
    step_ctr = 0;
    if (mel_left == 0) {
        u8 n = mel_t.note[mel_i];
        mel_left = mel_t.len[mel_i];
        if (n) {
            REG_SOUND2CNT_L = 0x6280 | (mel_left > 2 ? 0x100 : 0);
            REG_SOUND2CNT_H = sq_rate[n] | 0x8000;
        }
        mel_i = (mel_i + 1) % mel_t.n;
    }
    mel_left--;
    if (bass_left == 0) {
        u8 n = bass_t.note[bass_i];
        bass_left = bass_t.len[bass_i];
        if (n && wv_rate[n]) {
            REG_SOUND3CNT_H = 0x4000;
            REG_SOUND3CNT_X = wv_rate[n] | 0x8000;
        } else {
            REG_SOUND3CNT_H = 0;
        }
        bass_i = (bass_i + 1) % bass_t.n;
    }
    bass_left--;
}
