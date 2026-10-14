// Minimal GBA hardware definitions for MonsterMons
#ifndef GBA_H
#define GBA_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  u8;  typedef int8_t  s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
typedef volatile u16 vu16; typedef volatile u32 vu32; typedef volatile u8 vu8;

#define EWRAM_BSS  __attribute__((section(".sbss")))
#define IWRAM_CODE __attribute__((section(".iwram"), long_call, target("arm")))

#define REG_DISPCNT   (*(vu16*)0x04000000)
#define REG_DISPSTAT  (*(vu16*)0x04000004)
#define REG_VCOUNT    (*(vu16*)0x04000006)
#define REG_BGCNT(n)  (*(vu16*)(0x04000008 + 2 * (n)))
#define REG_BGHOFS(n) (*(vu16*)(0x04000010 + 4 * (n)))
#define REG_BGVOFS(n) (*(vu16*)(0x04000012 + 4 * (n)))
#define REG_WIN0H     (*(vu16*)0x04000040)
#define REG_WIN0V     (*(vu16*)0x04000044)
#define REG_WININ     (*(vu16*)0x04000048)
#define REG_WINOUT    (*(vu16*)0x0400004A)
#define REG_MOSAIC    (*(vu16*)0x0400004C)
#define REG_BLDCNT    (*(vu16*)0x04000050)
#define REG_BLDALPHA  (*(vu16*)0x04000052)
#define REG_BLDY      (*(vu16*)0x04000054)

#define REG_SOUNDCNT_L (*(vu16*)0x04000080)
#define REG_SOUNDCNT_H (*(vu16*)0x04000082)
#define REG_SOUNDCNT_X (*(vu16*)0x04000084)
#define REG_SOUND1CNT_L (*(vu16*)0x04000060)
#define REG_SOUND1CNT_H (*(vu16*)0x04000062)
#define REG_SOUND1CNT_X (*(vu16*)0x04000064)
#define REG_SOUND2CNT_L (*(vu16*)0x04000068)
#define REG_SOUND2CNT_H (*(vu16*)0x0400006C)
#define REG_SOUND3CNT_L (*(vu16*)0x04000070)
#define REG_SOUND3CNT_H (*(vu16*)0x04000072)
#define REG_SOUND3CNT_X (*(vu16*)0x04000074)
#define REG_SOUND4CNT_L (*(vu16*)0x04000078)
#define REG_SOUND4CNT_H (*(vu16*)0x0400007C)
#define WAVE_RAM       ((vu32*)0x04000090)

#define REG_KEYINPUT  (*(vu16*)0x04000130)
#define REG_IE        (*(vu16*)0x04000200)
#define REG_IF        (*(vu16*)0x04000202)
#define REG_WAITCNT   (*(vu16*)0x04000204)
#define REG_IME       (*(vu16*)0x04000208)

#define DCNT_MODE0  0x0000
#define DCNT_OBJ_1D 0x0040
#define DCNT_BLANK  0x0080
#define DCNT_BG0    0x0100
#define DCNT_BG1    0x0200
#define DCNT_BG2    0x0400
#define DCNT_BG3    0x0800
#define DCNT_OBJ    0x1000
#define DCNT_WIN0   0x2000

#define BG_PRIO(n)  (n)
#define BG_CBB(n)   ((n) << 2)
#define BG_SBB(n)   ((n) << 8)
#define BG_4BPP     0
#define BG_SIZE0    0
#define BG_SIZE1    0x4000  // 64x32

#define KEY_A      0x0001
#define KEY_B      0x0002
#define KEY_SELECT 0x0004
#define KEY_START  0x0008
#define KEY_RIGHT  0x0010
#define KEY_LEFT   0x0020
#define KEY_UP     0x0040
#define KEY_DOWN   0x0080
#define KEY_R      0x0100
#define KEY_L      0x0200

#define PAL_BG   ((vu16*)0x05000000)
#define PAL_OBJ  ((vu16*)0x05000200)
#define VRAM     ((vu16*)0x06000000)
#define CHARBLOCK(n) ((vu16*)(0x06000000 + (n) * 0x4000))
#define SCREENBLOCK(n) ((vu16*)(0x06000000 + (n) * 0x800))
#define OBJ_TILES ((vu16*)0x06010000)
#define OAM      ((vu16*)0x07000000)
#define SRAM     ((vu8*)0x0E000000)

#define RGB15(r,g,b) ((r) | ((g) << 5) | ((b) << 10))

static inline void vblank_wait(void) {
    __asm__ volatile("swi 0x05" ::: "r0", "r1", "r2", "r3", "memory");
}

#endif
