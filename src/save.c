// MonsterMons - battery save (32KB SRAM, byte access only)
#include <string.h>
#include <stddef.h>
#include "core.h"

#define SAVE_MAGIC 0x4D4F4E31

// emulators detect the save type from this string in ROM
__attribute__((used)) static const char sram_tag[] = "SRAM_V113";

static u32 checksum(const Save *s) {
    const u8 *p = (const u8 *)s;
    u32 c = 0x1F2E3D4C;
    for (unsigned i = 0; i < offsetof(Save, checksum); i++) c = (c << 5) + (c >> 27) + p[i];
    return c;
}

static void sram_read(void *dst, int off, int len) {
    u8 *d = dst;
    for (int i = 0; i < len; i++) d[i] = SRAM[off + i];
}

static void sram_write(const void *src, int off, int len) {
    const u8 *s = src;
    for (int i = 0; i < len; i++) SRAM[off + i] = s[i];
}

bool save_exists(void) {
    u32 magic;
    (void)*(volatile const char *)sram_tag;
    sram_read(&magic, 0, 4);
    return magic == SAVE_MAGIC;
}

void save_write(void) {
    sv.magic = SAVE_MAGIC;
    sv.checksum = checksum(&sv);
    sram_write(&sv, 0, sizeof(Save));
}

bool save_read(void) {
    static Save tmp;
    sram_read(&tmp, 0, sizeof(Save));
    if (tmp.magic != SAVE_MAGIC || tmp.checksum != checksum(&tmp)) return false;
    memcpy(&sv, &tmp, sizeof(Save));
    return true;
}
