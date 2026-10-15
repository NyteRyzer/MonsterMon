// MonsterMons - overworld data shared between world.c and events.c
#ifndef WORLD_H
#define WORLD_H
#include "core.h"

typedef struct {
    u8 map, x, y, sprite, facing;
    s8 show_flag, hide_flag;   // -1 = none
    u8 script;
    u8 move;                   // 0 static, 1 wanders, >=2 trainer with that sight range
    s8 tflag;                  // trainer: flag set once beaten (stops spotting you)
} NpcDef;

typedef struct { u8 map, x, y, dest, dx, dy, facing; } Warp;

extern const NpcDef npc_defs[];
extern const int npc_def_count;
extern const Warp warps[];
extern const int warp_count;

// runtime NPC state for the current map
typedef struct { const NpcDef *def; u8 x, y, facing; bool active; s8 off; u8 timer, hx, hy; } Npc;
#define MAX_NPCS 14
extern Npc npcs[MAX_NPCS];
extern int npc_n;

extern int ply_x, ply_y, ply_facing;
enum { DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT };
void world_refresh_npcs(void);
void world_draw_sprites(void);
void npc_face(int i, int dir);
bool map_is_outdoor(int map);
void world_banner(const char *s);
void world_set_player(int x, int y, int facing);

#define SPR_MON(sp) (100 + (sp))
#define SPR_RELIC 99
#endif
