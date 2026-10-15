// MonsterMons - creature, move, item and type data
#include "core.h"

const char *const type_names[T_COUNT] = { "BEAST", "BLOOD", "SHADE", "INFERNAL", "CRYPTID", "ARCANE", "UNDEAD", "LUMIN" };
const char *const nature_names[NAT_COUNT] = { "BRAVE", "CALM", "SWIFT", "MYSTIC", "LOYAL" };
const char *const status_names[5] = { "", "BRN", "CRS", "SLP", "FER" };

// super effective / not very effective per attacking type
static const u8 se[T_COUNT][3] = {
    { T_SHADE, T_ARCANE, T_NONE },      // BEAST
    { T_BEAST, T_CRYPTID, T_NONE },     // BLOOD
    { T_BLOOD, T_SHADE, T_NONE },       // SHADE
    { T_CRYPTID, T_UNDEAD, T_NONE },    // INFERNAL
    { T_LUMIN, T_ARCANE, T_NONE },      // CRYPTID
    { T_INFERNAL, T_BLOOD, T_NONE },    // ARCANE
    { T_CRYPTID, T_BEAST, T_NONE },     // UNDEAD
    { T_SHADE, T_UNDEAD, T_NONE },      // LUMIN
};
static const u8 nve[T_COUNT][3] = {
    { T_UNDEAD, T_INFERNAL, T_NONE },
    { T_UNDEAD, T_LUMIN, T_BLOOD },
    { T_LUMIN, T_INFERNAL, T_NONE },
    { T_INFERNAL, T_ARCANE, T_NONE },
    { T_INFERNAL, T_BLOOD, T_NONE },
    { T_ARCANE, T_CRYPTID, T_NONE },
    { T_LUMIN, T_UNDEAD, T_NONE },
    { T_LUMIN, T_INFERNAL, T_NONE },
};

int type_eff(int atk, int def) {
    if (atk >= T_COUNT || def >= T_COUNT) return 1;
    for (int i = 0; i < 3; i++) {
        if (se[atk][i] == def) return 2;
        if (nve[atk][i] == def) return 0;
    }
    return 1;
}

int type_mult4(int mtype, int t1, int t2) {
    static const u8 f[3] = { 1, 2, 4 }; // half, normal, double (x2 scale)
    int m = f[type_eff(mtype, t1)];
    if (t2 != T_NONE && t2 != t1) m = m * f[type_eff(mtype, t2)];
    else m = m * 2;
    return m; // 4 == neutral
}

#define BE T_BEAST
#define BL T_BLOOD
#define SH T_SHADE
#define IN T_INFERNAL
#define CR T_CRYPTID
#define AR T_ARCANE
#define UN T_UNDEAD
#define LU T_LUMIN
#define NO T_NONE

const Move moves[MOVE_COUNT] = {
    { "---", NO, 0, 0, 0, EF_NONE, 0, TGT_FOE },
    { "SCRATCH", BE, 40, 100, 35, EF_NONE, 0, TGT_FOE },
    { "BITE", BE, 60, 100, 25, EF_FEAR, 10, TGT_FOE },
    { "HOWL", BE, 0, 100, 20, EF_ATKUP, 100, TGT_SELF },
    { "FERAL RUSH", BE, 90, 90, 10, EF_RECOIL, 100, TGT_FOE },
    { "MOON FANG", BE, 80, 100, 15, EF_NONE, 0, TGT_FOE },
    { "DRAIN", BL, 40, 100, 25, EF_DRAIN, 100, TGT_FOE },
    { "BLOODLET", BL, 70, 95, 15, EF_NONE, 0, TGT_FOE },
    { "CRIMSON TIDE", BL, 60, 100, 10, EF_NONE, 0, TGT_ALL_FOES },
    { "VAMP KISS", BL, 80, 100, 10, EF_DRAIN, 100, TGT_FOE },
    { "HEMOBURST", BL, 110, 85, 5, EF_NONE, 0, TGT_FOE },
    { "SPOOK", SH, 40, 100, 30, EF_FEAR, 20, TGT_FOE },
    { "SHADE CLAW", SH, 65, 100, 20, EF_NONE, 0, TGT_FOE },
    { "WAIL", SH, 55, 95, 15, EF_NONE, 0, TGT_ALL_FOES },
    { "NIGHTMARE", SH, 0, 75, 10, EF_SLEEP, 100, TGT_FOE },
    { "PHANTOM", SH, 95, 95, 10, EF_NONE, 0, TGT_FOE },
    { "EMBER", IN, 40, 100, 25, EF_BURN, 10, TGT_FOE },
    { "HELLFIRE", IN, 90, 90, 10, EF_BURN, 20, TGT_FOE },
    { "INFERNO", IN, 70, 90, 10, EF_BURN, 10, TGT_ALL_FOES },
    { "BRIMSTONE", IN, 110, 85, 5, EF_NONE, 0, TGT_FOE },
    { "TACKLE", CR, 40, 100, 35, EF_NONE, 0, TGT_FOE },
    { "STOMP", CR, 70, 100, 20, EF_FEAR, 20, TGT_FOE },
    { "PRIMAL ROAR", CR, 0, 100, 20, EF_ATKDOWN, 100, TGT_ALL_FOES },
    { "QUAKE", CR, 80, 100, 10, EF_NONE, 0, TGT_ALL_FOES },
    { "MOSS HIDE", CR, 0, 100, 20, EF_DEFUP, 100, TGT_SELF },
    { "HEX", AR, 40, 100, 30, EF_NONE, 0, TGT_FOE },
    { "CURSE", AR, 0, 90, 15, EF_CURSE, 100, TGT_FOE },
    { "ARCANE BOLT", AR, 75, 100, 15, EF_NONE, 0, TGT_FOE },
    { "FOCUS SIGIL", AR, 0, 100, 15, EF_SPCUP, 100, TGT_SELF },
    { "STARFALL", AR, 100, 90, 5, EF_NONE, 0, TGT_FOE },
    { "GRASP", UN, 45, 100, 30, EF_NONE, 0, TGT_FOE },
    { "ROT", UN, 0, 90, 15, EF_CURSE, 100, TGT_FOE },
    { "BONE RUSH", UN, 75, 95, 15, EF_NONE, 0, TGT_FOE },
    { "DEATH RATTLE", UN, 100, 90, 5, EF_NONE, 0, TGT_FOE },
    { "GLIMMER", LU, 40, 100, 30, EF_NONE, 0, TGT_FOE },
    { "MEND LIGHT", LU, 0, 100, 10, EF_HEAL, 100, TGT_ALLY },
    { "RADIANCE", LU, 80, 100, 15, EF_NONE, 0, TGT_FOE },
    { "DAWNBURST", LU, 110, 85, 5, EF_NONE, 0, TGT_FOE },
    { "QUICKSTEP", BE, 0, 100, 20, EF_SPDUP, 100, TGT_SELF },
    { "SCREECH", BL, 0, 100, 20, EF_ATKDOWN, 100, TGT_FOE },
    { "WISP MEND", SH, 0, 100, 10, EF_HEAL, 100, TGT_ALLY },
    { "SOUL COMBO", NO, 50, 100, 1, EF_NONE, 0, TGT_ALL_FOES },
};

enum { M_SCRATCH = 1, M_BITE, M_HOWL, M_FERAL, M_MOONFANG, M_DRAIN, M_BLOODLET, M_CRIMSON, M_VKISS, M_HEMO,
       M_SPOOK, M_SCLAW, M_WAIL, M_NIGHTMARE, M_PHANTOM, M_EMBER, M_HELLFIRE, M_INFERNO, M_BRIM, M_TACKLE,
       M_STOMP, M_ROAR, M_QUAKE, M_MOSS, M_HEX, M_CURSE, M_ABOLT, M_FOCUS, M_STARFALL, M_GRASP, M_ROT,
       M_BONERUSH, M_RATTLE, M_GLIMMER, M_MEND, M_RADIANCE, M_DAWN, M_QUICK, M_SCREECH, M_WISPMEND };

#define L(...) (const u8[]){ __VA_ARGS__, 0 }

const Species species[SPECIES_COUNT] = {
    { "FANGLING", BL, NO, 45, 55, 40, 60, 45, 45, 62, 0, EVO_FANG, 16, 2, 1,
      L(1, M_DRAIN, 1, M_SPOOK, 4, M_SCREECH, 7, M_BITE, 11, M_BLOODLET, 15, M_SCLAW, 20, M_VKISS, 26, M_CRIMSON, 34, M_HEMO),
      "A vampire whelp that naps upside down in belfries. Its bite is more ticklish than fearsome.", "Guild partner" },
    { "COUNTESS", BL, SH, 70, 90, 60, 85, 90, 20, 170, 0, EVO_NONE, 0, 0, 0,
      L(1, M_VKISS, 1, M_SCLAW, 1, M_NIGHTMARE, 1, M_BLOODLET, 24, M_CRIMSON, 30, M_PHANTOM, 36, M_HEMO),
      "A noble of the night. Its gaze can still a beating heart. It answers only to those it respects.", "Fangling under moonlight" },
    { "DAWNFANG", BL, LU, 75, 85, 70, 75, 85, 20, 170, 0, EVO_NONE, 0, 0, 0,
      L(1, M_VKISS, 1, M_RADIANCE, 1, M_MEND, 1, M_BLOODLET, 26, M_CRIMSON, 32, M_DAWN, 38, M_HEMO),
      "A vampire who learned to walk in sunlight through a hunter's devotion. Legends call it a miracle.", "Fangling with deep bond" },
    { "PUPWOLF", BE, NO, 50, 60, 45, 55, 35, 45, 62, 0, EVO_DAYNIGHT, 16, 4, 5,
      L(1, M_SCRATCH, 1, M_HOWL, 5, M_BITE, 9, M_QUICK, 14, M_MOONFANG, 21, M_STOMP, 28, M_FERAL),
      "A werewolf pup that howls at streetlights it mistakes for the moon. Fiercely loyal to its pack.", "Guild partner" },
    { "DIREWOLF", BE, NO, 80, 105, 70, 80, 45, 30, 170, 0, EVO_NONE, 0, 0, 0,
      L(1, M_MOONFANG, 1, M_BITE, 1, M_HOWL, 1, M_QUICK, 24, M_FERAL, 30, M_STOMP, 36, M_BONERUSH),
      "A sun-hardened alpha. It guards villages at dusk and will not yield ground to any predator.", "Pupwolf raised by day" },
    { "MOONHOWL", BE, SH, 75, 95, 65, 95, 70, 30, 170, 0, EVO_NONE, 0, 0, 0,
      L(1, M_MOONFANG, 1, M_SCLAW, 1, M_HOWL, 1, M_SPOOK, 24, M_WAIL, 30, M_FERAL, 36, M_PHANTOM),
      "Its howl bends shadows. Travelers who hear it at midnight swear the moon grows larger.", "Pupwolf raised by night" },
    { "WISPETTE", SH, NO, 45, 35, 45, 55, 65, 45, 62, CAP_NIGHT, EVO_BOND, 16, 7, 8,
      L(1, M_SPOOK, 1, M_GLIMMER, 5, M_WISPMEND, 9, M_HEX, 12, M_SCLAW, 18, M_WAIL, 24, M_NIGHTMARE, 30, M_PHANTOM),
      "A shy little spirit that follows lonely people home. It cries when left in the dark.", "Guild partner" },
    { "LANTERNIS", SH, LU, 75, 50, 80, 70, 100, 30, 170, CAP_NIGHT, EVO_NONE, 0, 0, 0,
      L(1, M_RADIANCE, 1, M_WAIL, 1, M_MEND, 1, M_SCLAW, 28, M_PHANTOM, 34, M_DAWN),
      "A spirit that found its way home. It carries a lantern to guide other lost souls.", "Wispette with deep bond" },
    { "BANSHRIEK", SH, NO, 70, 60, 60, 90, 105, 30, 170, CAP_NIGHT, EVO_NONE, 0, 0, 0,
      L(1, M_WAIL, 1, M_NIGHTMARE, 1, M_SCLAW, 1, M_SPOOK, 26, M_CURSE, 32, M_PHANTOM),
      "Its scream foretells a death in the house. Few know it is mourning, not threatening.", "Grave Hill, night" },
    { "BATLING", BL, NO, 35, 40, 30, 65, 30, 190, 40, 0, EVO_LEVEL, 14, 10, 0,
      L(1, M_DRAIN, 1, M_SCREECH, 6, M_SPOOK, 10, M_BITE, 16, M_BLOODLET, 22, M_VKISS),
      "Swarms of Batlings blot out the stars over Hollowmere. Each one sips only a drop.", "Woods, Grave Hill, night" },
    { "CHIROVAMP", BL, NO, 65, 75, 55, 95, 55, 90, 140, 0, EVO_NONE, 0, 0, 0,
      L(1, M_BLOODLET, 1, M_DRAIN, 1, M_BITE, 1, M_SCREECH, 22, M_VKISS, 28, M_CRIMSON, 36, M_HEMO),
      "A bat demon with a wingspan wider than a carriage. It hunts by listening for heartbeats.", "Manor, Castle" },
    { "GHOULET", UN, NO, 50, 55, 50, 30, 30, 190, 45, 0, EVO_LEVEL, 18, 12, 0,
      L(1, M_GRASP, 1, M_TACKLE, 6, M_ROT, 12, M_BITE, 18, M_BONERUSH),
      "A sleepy little ghoul. It mostly wants a warm grave and someone to share snacks with.", "Grave Hill" },
    { "GHOULORD", UN, IN, 90, 95, 85, 40, 70, 45, 160, 0, EVO_NONE, 0, 0, 0,
      L(1, M_BONERUSH, 1, M_EMBER, 1, M_ROT, 1, M_GRASP, 24, M_HELLFIRE, 32, M_RATTLE),
      "A hulking corpse-king wreathed in grave-fire. Chains rattle wherever it walks.", "Duskhold Castle" },
    { "MOTHKIN", CR, NO, 40, 35, 40, 60, 55, 170, 45, 0, EVO_NIGHT, 20, 14, 0,
      L(1, M_TACKLE, 1, M_GLIMMER, 6, M_SPOOK, 10, M_HEX, 15, M_STOMP, 20, M_MOSS),
      "A fluffy moth cryptid drawn to porch lights. Its eye-spots are a bluff - it is very gentle.", "Whisper Woods" },
    { "MOTHMAN", CR, SH, 75, 80, 65, 95, 85, 45, 165, 0, EVO_NONE, 0, 0, 0,
      L(1, M_STOMP, 1, M_SCLAW, 1, M_NIGHTMARE, 1, M_ROAR, 28, M_QUAKE, 34, M_PHANTOM),
      "Red eyes on a bridge at midnight. Sightings always come right before disasters.", "Mothkin evolved at night" },
    { "IMPISH", IN, NO, 40, 45, 35, 65, 60, 170, 50, 0, EVO_LEVEL, 18, 16, 0,
      L(1, M_EMBER, 1, M_SCRATCH, 6, M_HEX, 11, M_SPOOK, 16, M_CURSE, 22, M_HELLFIRE),
      "A prankster demon that steals left socks and hides car keys. Grins even when scolded.", "Neon Row" },
    { "HELLION", IN, NO, 80, 85, 70, 85, 100, 45, 170, 0, EVO_NONE, 0, 0, 0,
      L(1, M_HELLFIRE, 1, M_CURSE, 1, M_EMBER, 1, M_HEX, 26, M_INFERNO, 34, M_BRIM),
      "A greater demon whose horns glow like forge-iron. Its contracts are always in fine print.", "Duskhold Castle" },
    { "HEXCAT", AR, NO, 45, 50, 40, 75, 60, 120, 60, 0, EVO_LEVEL, 22, 18, 0,
      L(1, M_SCRATCH, 1, M_HEX, 7, M_QUICK, 12, M_CURSE, 17, M_ABOLT, 22, M_FOCUS),
      "A witch's familiar that outlived its witch. It still wears her hat and judges everyone.", "Woods at night, Neon Row" },
    { "GRIMALKIN", AR, SH, 75, 70, 65, 105, 105, 45, 175, 0, EVO_NONE, 0, 0, 0,
      L(1, M_ABOLT, 1, M_SCLAW, 1, M_CURSE, 1, M_FOCUS, 30, M_STARFALL, 36, M_PHANTOM),
      "A cat sorceress with nine lives and two tails. Runes orbit it like curious moths.", "The Rift" },
    { "BONEHOP", UN, NO, 55, 60, 50, 80, 40, 120, 70, 0, EVO_NONE, 0, 0, 0,
      L(1, M_GRASP, 1, M_QUICK, 8, M_BITE, 14, M_BONERUSH, 22, M_ROT, 30, M_RATTLE),
      "A skeletal rabbit that hops between graves. Its rattling is a lullaby to the dead.", "Grave Hill" },
    { "SQUATCHY", CR, NO, 60, 65, 60, 40, 35, 150, 60, 0, EVO_LEVEL, 24, 21, 0,
      L(1, M_TACKLE, 1, M_MOSS, 8, M_STOMP, 14, M_ROAR, 20, M_BITE, 24, M_QUAKE),
      "Blurry photos of Squatchy sell for fortunes. In person it is shy and smells of pine.", "Whisper Woods" },
    { "SASQUATCH", CR, BE, 105, 110, 90, 55, 50, 45, 180, 0, EVO_NONE, 0, 0, 0,
      L(1, M_QUAKE, 1, M_STOMP, 1, M_FERAL, 1, M_ROAR, 34, M_MOONFANG),
      "The king of the deep forest. Each footprint is the size of a doorway.", "The Rift" },
    { "GARGOYL", AR, NO, 70, 75, 105, 40, 60, 75, 120, 0, EVO_NONE, 0, 0, 0,
      L(1, M_HEX, 1, M_MOSS, 1, M_GRASP, 18, M_ABOLT, 26, M_STOMP, 34, M_STARFALL),
      "A stone guardian that wakes at dusk. It has watched over Ravencrest for three centuries.", "Ravencrest Manor, Castle" },
    { "CERBERPUP", IN, BE, 55, 65, 45, 65, 60, 75, 90, 0, EVO_LEVEL, 28, 24, 0,
      L(1, M_EMBER, 1, M_BITE, 8, M_HOWL, 14, M_HELLFIRE, 20, M_MOONFANG, 28, M_INFERNO),
      "Three heads, three opinions. The middle one is usually in charge of naps.", "Neon Row (rare)" },
    { "CERBERUS", IN, BE, 95, 105, 80, 90, 90, 30, 190, 0, EVO_NONE, 0, 0, 0,
      L(1, M_INFERNO, 1, M_MOONFANG, 1, M_HELLFIRE, 1, M_HOWL, 36, M_FERAL, 42, M_BRIM),
      "Gatekeeper of the underworld. It will let anyone in, but no one out.", "The Rift" },
    { "GLITCHOST", SH, AR, 55, 40, 50, 90, 90, 90, 110, 0, EVO_NONE, 0, 0, 0,
      L(1, M_HEX, 1, M_SPOOK, 10, M_SCLAW, 16, M_ABOLT, 22, M_CURSE, 30, M_PHANTOM, 36, M_STARFALL),
      "A spirit trapped in the neon grid. It appears in rain, flickering between billboards.", "Neon Row in rain" },
    { "CHUPA", CR, BL, 60, 80, 55, 75, 40, 100, 100, 0, EVO_NONE, 0, 0, 0,
      L(1, M_BITE, 1, M_DRAIN, 10, M_STOMP, 16, M_BLOODLET, 24, M_VKISS, 30, M_QUAKE),
      "The goat-sucker of legend. Farmers leave out milk to keep it away from the herd.", "Woods, Neon Row alleys" },
    { "WRAPLING", UN, NO, 60, 55, 75, 35, 55, 120, 80, 0, EVO_NONE, 0, 0, 0,
      L(1, M_GRASP, 1, M_ROT, 10, M_HEX, 16, M_BONERUSH, 24, M_CURSE, 32, M_RATTLE),
      "A mummy child that keeps unravelling. It is always looking for its missing bandages.", "Grave Hill, Castle" },
    { "CANDLEWICK", IN, SH, 50, 40, 55, 55, 95, 60, 110, CAP_RITUAL, EVO_NONE, 0, 0, 0,
      L(1, M_EMBER, 1, M_SPOOK, 12, M_HEX, 18, M_WAIL, 24, M_HELLFIRE, 32, M_INFERNO),
      "A haunted candle from a forgotten seance. Only a Ritual Candle's flame can bind it.", "Ravencrest Manor" },
    { "HALOFLY", LU, NO, 40, 35, 40, 70, 60, 170, 50, 0, EVO_LEVEL, 20, 30, 0,
      L(1, M_GLIMMER, 1, M_TACKLE, 6, M_MEND, 12, M_HEX, 18, M_RADIANCE),
      "A firefly sprite born from the last light of dusk. Its glow keeps nightmares away.", "Whisper Woods by day" },
    { "SERAPHLY", LU, AR, 75, 55, 70, 95, 110, 45, 175, 0, EVO_NONE, 0, 0, 0,
      L(1, M_RADIANCE, 1, M_MEND, 1, M_ABOLT, 1, M_FOCUS, 30, M_STARFALL, 36, M_DAWN),
      "Six wings of living light. Its eyes see every lie ever told beneath the sun.", "The Rift" },
    { "LOCHLING", CR, NO, 70, 55, 65, 45, 60, 60, 110, 0, EVO_NONE, 0, 0, 0,
      L(1, M_TACKLE, 1, M_MOSS, 8, M_STOMP, 14, M_GLIMMER, 20, M_QUAKE, 28, M_RADIANCE),
      "A lake cryptid that surfaces only for people it trusts. It loves skipping stones.", "Whisper Woods lakeshore" },
    { "WENDIGO", CR, SH, 85, 100, 65, 90, 70, 30, 170, CAP_TRUST | CAP_NIGHT, EVO_NONE, 0, 0, 0,
      L(1, M_BITE, 1, M_SPOOK, 1, M_STOMP, 1, M_SCLAW, 24, M_FERAL, 30, M_PHANTOM),
      "A starving spirit of the frozen wood. It will only follow a hunter who feeds it an Offering.", "Hidden glade, night" },
    { "HOLLOWKING", UN, IN, 120, 115, 100, 70, 110, 5, 255, CAP_BOSS, EVO_NONE, 0, 0, 0,
      L(1, M_RATTLE, 1, M_HELLFIRE, 1, M_CURSE, 1, M_INFERNO),
      "An ancient king bound by the Pale Sun to serve as a weapon. Its crown is a cage.", "Duskhold Castle" },
    { "ECLIPSA", SH, LU, 125, 110, 100, 105, 125, 3, 255, CAP_BOSS, EVO_NONE, 0, 0, 0,
      L(1, M_PHANTOM, 1, M_DAWN, 1, M_STARFALL, 1, M_NIGHTMARE),
      "The dragon between night and day. Its waking caused the Nightfall - or so the Pale Sun claims.", "The Rift" },
    { "LYCANTH", BE, BL, 85, 105, 70, 90, 55, 20, 170, 0, EVO_NONE, 0, 0, 0,
      L(1, M_MOONFANG, 1, M_BLOODLET, 1, M_HOWL, 1, M_BITE, 20, M_FERAL, 28, M_VKISS, 36, M_HEMO),
      "An alpha werewolf who walks between two worlds. It never forgets an act of mercy.", "Whisper Woods (event)" },
    { "CROWLING", SH, NO, 40, 50, 35, 70, 40, 190, 45, 0, EVO_LEVEL, 18, 37, 0,
      L(1, M_SCRATCH, 1, M_SPOOK, 5, M_QUICK, 9, M_BITE, 14, M_SCLAW, 20, M_PHANTOM),
      "A crow that drifted back from the other side. It collects shiny things from graves.", "Woods, Grave Hill" },
    { "MURDERCROW", SH, BE, 70, 85, 60, 100, 70, 60, 150, 0, EVO_NONE, 0, 0, 0,
      L(1, M_SCLAW, 1, M_BITE, 1, M_QUICK, 1, M_SPOOK, 26, M_FERAL, 32, M_PHANTOM),
      "Where one appears, a hundred follow. A flock of them can blot out the moon.", "Crowling evolved" },
    { "PUMPKIT", CR, IN, 45, 45, 55, 40, 55, 150, 55, 0, EVO_NIGHT, 24, 39, 0,
      L(1, M_TACKLE, 1, M_EMBER, 7, M_MOSS, 12, M_HEX, 18, M_STOMP, 24, M_HELLFIRE),
      "A carved pumpkin that woke up on the Nightfall. Its candle never goes out.", "Woods, Grave Hill at night" },
    { "JACKLORD", IN, SH, 80, 85, 75, 60, 95, 45, 165, 0, EVO_NONE, 0, 0, 0,
      L(1, M_HELLFIRE, 1, M_SCLAW, 1, M_HEX, 1, M_STOMP, 30, M_INFERNO, 36, M_PHANTOM),
      "The scarecrow king of the harvest. Fields it guards never fail - or are never harvested.", "Pumpkit evolved at night" },
    { "SPIDROT", UN, CR, 45, 60, 45, 60, 35, 150, 55, 0, EVO_LEVEL, 22, 41, 0,
      L(1, M_BITE, 1, M_GRASP, 6, M_ROT, 11, M_STOMP, 16, M_BONERUSH),
      "A spider stitched together from cobwebs and old bones. It weaves webs across doorways.", "Ravencrest Manor" },
    { "WIDOWEB", UN, AR, 75, 90, 70, 85, 75, 45, 160, 0, EVO_NONE, 0, 0, 0,
      L(1, M_BONERUSH, 1, M_CURSE, 1, M_BITE, 1, M_ROT, 28, M_ABOLT, 34, M_RATTLE),
      "Its web hums with old curses. Those caught in it forget their own names.", "Spidrot evolved" },
    { "SLIMOOZE", AR, NO, 90, 50, 70, 30, 70, 120, 90, 0, EVO_NONE, 0, 0, 0,
      L(1, M_TACKLE, 1, M_HEX, 8, M_MOSS, 14, M_CURSE, 20, M_ABOLT, 28, M_FOCUS),
      "Living ectoplasm from a botched seance. It absorbs anything it touches - including ghosts.", "Neon Row, Manor" },
    { "KAPPLET", CR, NO, 60, 60, 60, 55, 50, 120, 70, 0, EVO_NONE, 0, 0, 0,
      L(1, M_TACKLE, 1, M_GLIMMER, 8, M_STOMP, 13, M_MOSS, 19, M_QUAKE, 26, M_RADIANCE),
      "A polite river imp. Bow to it and it must bow back, spilling the water from its head.", "Whisper Woods lakeshore" },
    { "GOLEMITE", AR, CR, 85, 85, 115, 25, 55, 75, 140, 0, EVO_NONE, 0, 0, 0,
      L(1, M_TACKLE, 1, M_MOSS, 1, M_HEX, 20, M_STOMP, 28, M_QUAKE, 36, M_STARFALL),
      "A castle wall that decided it wanted to walk. The runes on its chest still glow.", "Duskhold Castle" },
    { "DREADMARE", SH, IN, 80, 100, 70, 110, 90, 25, 190, CAP_NIGHT, EVO_NONE, 0, 0, 0,
      L(1, M_SCLAW, 1, M_EMBER, 1, M_NIGHTMARE, 1, M_STOMP, 34, M_HELLFIRE, 40, M_PHANTOM),
      "A nightmare given hooves. Its mane is black fire; sleepers it passes never wake rested.", "Castle, Rift at night" },
    { "FOXFIRE", IN, AR, 50, 50, 45, 85, 75, 60, 90, 0, EVO_LEVEL, 32, 47, 0,
      L(1, M_EMBER, 1, M_HEX, 8, M_QUICK, 14, M_ABOLT, 20, M_HELLFIRE, 26, M_FOCUS),
      "A two-tailed fox spirit that lights blue lanterns to lead travelers astray - mostly for fun.", "Neon Row (rare), Rift" },
    { "KYUBI", AR, LU, 85, 70, 75, 115, 120, 30, 200, 0, EVO_NONE, 0, 0, 0,
      L(1, M_ABOLT, 1, M_RADIANCE, 1, M_HELLFIRE, 1, M_FOCUS, 38, M_STARFALL, 44, M_DAWN),
      "A nine-tailed sage a thousand years old. Each tail holds a century of memories.", "Foxfire evolved" },
    { "BOGGLE", CR, UN, 60, 55, 60, 35, 45, 150, 60, 0, EVO_LEVEL, 22, 49, 0,
      L(1, M_TACKLE, 1, M_GRASP, 6, M_MOSS, 11, M_ROT, 16, M_STOMP, 22, M_BONERUSH),
      "A bog frog that was buried in peat for a thousand years. It croaks in a dead language.", "Mistfen Marsh" },
    { "BOGLORD", CR, UN, 105, 90, 95, 40, 65, 45, 170, 0, EVO_NONE, 0, 0, 0,
      L(1, M_QUAKE, 1, M_BONERUSH, 1, M_ROT, 1, M_MOSS, 30, M_RATTLE, 36, M_STOMP),
      "The swamp's undead king. Will-o-wisps gather on its crown like flies to a lamp.", "Boggle evolved" },
    { "LUREFIN", CR, SH, 55, 70, 50, 65, 60, 120, 75, 0, EVO_NONE, 0, 0, 0,
      L(1, M_BITE, 1, M_GLIMMER, 8, M_SPOOK, 14, M_SCLAW, 20, M_VKISS, 28, M_PHANTOM),
      "Its glowing lure leads lost travelers to the water's edge. The rest of it waits below.", "Marsh reeds, Saltmarrow docks" },
    { "SIRENNE", AR, SH, 65, 45, 60, 85, 100, 45, 150, CAP_NIGHT, EVO_NONE, 0, 0, 0,
      L(1, M_HEX, 1, M_WAIL, 10, M_NIGHTMARE, 16, M_ABOLT, 24, M_CURSE, 32, M_STARFALL),
      "A drowned singer whose song still carries over the water. Sailors plug their ears with wax.", "Marsh and docks at night" },
    { "RATTLING", UN, BE, 40, 55, 35, 75, 30, 190, 45, 0, EVO_LEVEL, 26, 53, 0,
      L(1, M_SCRATCH, 1, M_GRASP, 5, M_QUICK, 10, M_BITE, 16, M_BONERUSH, 22, M_ROT),
      "A rat skeleton that never stopped scurrying. Where there is one, there are fifty.", "Ashen Rails" },
    { "RATKING", UN, BE, 85, 95, 70, 85, 60, 45, 170, 0, EVO_NONE, 0, 0, 0,
      L(1, M_BONERUSH, 1, M_BITE, 1, M_ROT, 1, M_FERAL, 32, M_RATTLE, 38, M_CRIMSON),
      "Five rat skulls bound by a single tail and a golden crown. They argue constantly.", "Rattling evolved" },
    { "RUSTWRAITH", SH, UN, 80, 85, 75, 70, 95, 25, 180, CAP_NIGHT, EVO_NONE, 0, 0, 0,
      L(1, M_SCLAW, 1, M_GRASP, 1, M_WAIL, 1, M_CURSE, 30, M_PHANTOM, 36, M_RATTLE),
      "The conductor of the last train to leave before the Nightfall. It still punches tickets.", "Ashen Rails at night" },
    { "COALIMP", IN, NO, 45, 50, 50, 55, 55, 170, 55, 0, EVO_LEVEL, 28, 56, 0,
      L(1, M_EMBER, 1, M_TACKLE, 6, M_HOWL, 12, M_HEX, 18, M_HELLFIRE, 24, M_MOSS),
      "A lump of living coal. It sneaks into stoves for warmth and gets very offended when lit.", "Ashen Rails" },
    { "STOKER", IN, AR, 100, 95, 100, 35, 80, 45, 175, 0, EVO_NONE, 0, 0, 0,
      L(1, M_HELLFIRE, 1, M_QUAKE, 1, M_FOCUS, 1, M_EMBER, 34, M_INFERNO, 40, M_BRIM),
      "A furnace spirit that powered the old trains. Its heart has burned for a hundred years.", "Coalimp evolved" },
    { "GRIMOIRE", AR, NO, 60, 40, 70, 60, 105, 75, 120, 0, EVO_NONE, 0, 0, 0,
      L(1, M_HEX, 1, M_CURSE, 10, M_ABOLT, 18, M_FOCUS, 26, M_NIGHTMARE, 34, M_STARFALL),
      "A spellbook that reads its own pages aloud. Some of the spells have never been tried.", "Gallowmoor graveyard at night" },
    { "STRAWMAN", CR, SH, 70, 75, 65, 60, 55, 90, 110, 0, EVO_NONE, 0, 0, 0,
      L(1, M_TACKLE, 1, M_SPOOK, 10, M_STOMP, 16, M_SCLAW, 24, M_ROAR, 32, M_PHANTOM),
      "A scarecrow that started scaring people instead of crows. It moves only when unwatched.", "Gallowmoor" },
    { "TIDEMOTHER", CR, LU, 115, 85, 95, 80, 115, 10, 230, CAP_TRUST | CAP_NIGHT, EVO_NONE, 0, 0, 0,
      L(1, M_QUAKE, 1, M_RADIANCE, 1, M_MEND, 1, M_MOSS, 36, M_DAWN, 42, M_STARFALL),
      "The marsh's guardian spirit. When she weeps, the tide rises. When she sings, it heals.", "Saltmarrow moon pool" },
};

const Item items[IT_COUNT] = {
    { "---", IK_KEY, 0, "" },
    { "SOULBIND", IK_BIND, 200, "Binds a weakened Monstermon's soul to yours." },
    { "GREATER BIND", IK_BIND, 600, "A stronger Soulbind. Better odds of success." },
    { "DUSK BIND", IK_BIND, 1000, "Works wonders at night. Weak during the day." },
    { "TONIC", IK_HEAL, 150, "Restores 30 HP to one Monstermon." },
    { "ELIXIR", IK_HEAL, 900, "Fully restores HP and cures status." },
    { "REVIVE ASH", IK_HEAL, 1200, "Revives a fainted Monstermon with half HP." },
    { "CLEANSE SALT", IK_HEAL, 200, "Cures burns, curses, sleep and fear." },
    { "RITUAL CANDLE", IK_BATTLE, 500, "Light it in battle so ritual-bound spirits can be Soulbound." },
    { "OFFERING", IK_BATTLE, 400, "Offer it to a wild Monstermon to earn its trust." },
    { "BOND TREAT", IK_FIELD, 800, "A favourite snack. Greatly raises a Monstermon's bond." },
    { "MOON SIGIL", IK_KEY, 0, "A silver seal from Ravencrest. Opens Duskhold's gate." },
    { "LANTERN", IK_TOOL, 0, "TOOL: pierces supernatural fog." },
    { "TRACKER LENS", IK_TOOL, 0, "TOOL: rare Eclipse variants appear 3x as often." },
    { "WARD CHARM", IK_TOOL, 3000, "TOOL: halves wild encounters." },
    { "IRON KNIFE", IK_WEAPON, 0, "WEAPON: HUNT strikes for 10% of a foe's HP." },
    { "SILVER BLADE", IK_WEAPON, 0, "WEAPON: 12% HP. Double vs BEAST and BLOOD." },
    { "HOLY CROSSBOW", IK_WEAPON, 0, "WEAPON: 12% HP. Double vs SHADE and UNDEAD." },
    { "RUNE STAFF", IK_WEAPON, 0, "WEAPON: 8% HP and marks the foe: +50% bind odds." },
};

// ------------------------------------------------------------------ abilities
const char *const ability_names[AB_COUNT] = { "NONE", "DREAD AURA", "BLOODLUST", "THICK HIDE", "NOCTURNAL",
                                              "SUNBORN", "LAST STAND", "PHANTOM BODY", "REGROWTH", "SOUL SURGE" };
const char *const ability_desc[AB_COUNT] = {
    "", "Lowers foes' ATTACK on entry.", "Drains twice as much; heals on a KO.", "Takes less from super-effective hits.",
    "Hits 20% harder at night.", "Hits 20% harder by day.", "Hits 50% harder when HP is low.",
    "Physical attacks may pass through it.", "Recovers a little HP each turn.", "Fills the Soul gauge faster.",
};
const u8 species_ability[SPECIES_COUNT] = {
    AB_BLOODLUST, AB_DREAD, AB_SUNBORN,          // Fangling, Countess, Dawnfang
    AB_NOCTURNAL, AB_SUNBORN, AB_NOCTURNAL,      // Pupwolf, Direwolf, Moonhowl
    AB_PHANTOM, AB_PHANTOM, AB_DREAD,            // Wispette, Lanternis, Banshriek
    AB_BLOODLUST, AB_BLOODLUST,                  // Batling, Chirovamp
    AB_REGROWTH, AB_THICKHIDE,                   // Ghoulet, Ghoulord
    AB_NOCTURNAL, AB_DREAD,                      // Mothkin, Mothman
    AB_LASTSTAND, AB_LASTSTAND,                  // Impish, Hellion
    AB_NOCTURNAL, AB_NOCTURNAL,                  // Hexcat, Grimalkin
    AB_REGROWTH,                                 // Bonehop
    AB_THICKHIDE, AB_THICKHIDE,                  // Squatchy, Sasquatch
    AB_THICKHIDE,                                // Gargoyl
    AB_LASTSTAND, AB_LASTSTAND,                  // Cerberpup, Cerberus
    AB_PHANTOM,                                  // Glitchost
    AB_BLOODLUST,                                // Chupa
    AB_REGROWTH,                                 // Wrapling
    AB_PHANTOM,                                  // Candlewick
    AB_SUNBORN, AB_SUNBORN,                      // Halofly, Seraphly
    AB_THICKHIDE,                                // Lochling
    AB_DREAD,                                    // Wendigo
    AB_DREAD,                                    // Hollowking
    AB_SOULSURGE,                                // Eclipsa
    AB_BLOODLUST,                                // Lycanth
    AB_NOCTURNAL, AB_DREAD,                      // Crowling, Murdercrow
    AB_REGROWTH, AB_LASTSTAND,                   // Pumpkit, Jacklord
    AB_NOCTURNAL, AB_DREAD,                      // Spidrot, Widoweb
    AB_REGROWTH, AB_THICKHIDE, AB_THICKHIDE,     // Slimooze, Kapplet, Golemite
    AB_DREAD, AB_PHANTOM, AB_SUNBORN,            // Dreadmare, Foxfire, Kyubi
    AB_REGROWTH, AB_THICKHIDE,                   // Boggle, Boglord
    AB_NOCTURNAL, AB_DREAD,                      // Lurefin, Sirenne
    AB_LASTSTAND, AB_DREAD,                      // Rattling, Ratking
    AB_PHANTOM,                                  // Rustwraith
    AB_LASTSTAND, AB_THICKHIDE,                  // Coalimp, Stoker
    AB_SOULSURGE, AB_NOCTURNAL,                  // Grimoire, Strawman
    AB_REGROWTH,                                 // Tidemother
};
