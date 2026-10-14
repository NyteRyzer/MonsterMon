// Headless MonsterMons test harness built on libmgba.
// usage: harness ROM SCRIPT OUTDIR
// script lines: "wait N" | "press KEYS [N]" (KEYS like A, B, UP, A+B; held N frames, default 6, then 6 released)
//               | "hold KEYS N" | "shot NAME" | "mash KEYS N" (tap every 12 frames for N frames)
#include <mgba/core/core.h>
#include <mgba/core/config.h>
#include <mgba/core/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct mCore *core;
static color_t *buf;
static unsigned W, H;
static const char *outdir;

static void quiet_log(struct mLogger *l, int cat, enum mLogLevel lvl, const char *fmt, va_list args) {
    (void)l; (void)cat; (void)lvl; (void)fmt; (void)args;
}
static struct mLogger logger = { .log = quiet_log };

static int keymask(const char *s) {
    static const char *names[] = { "A", "B", "SELECT", "START", "RIGHT", "LEFT", "UP", "DOWN", "R", "L" };
    int m = 0;
    char tmp[64];
    strncpy(tmp, s, 63);
    tmp[63] = 0;
    for (char *t = strtok(tmp, "+"); t; t = strtok(0, "+"))
        for (int i = 0; i < 10; i++)
            if (!strcmp(t, names[i])) m |= 1 << i;
    return m;
}

static void run(int keys, int frames) {
    core->setKeys(core, keys);
    for (int i = 0; i < frames; i++) core->runFrame(core);
}

static void shot(const char *name) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s.ppm", outdir, name);
    FILE *f = fopen(path, "wb");
    fprintf(f, "P6\n%u %u\n255\n", W, H);
    for (unsigned i = 0; i < W * H; i++) {
        uint32_t c = buf[i];
        unsigned char rgb[3] = { c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF };
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 4) return 1;
    mLogSetDefaultLogger(&logger);
    outdir = argv[3];
    core = mCoreFind(argv[1]);
    if (!core || !core->init(core)) { fprintf(stderr, "core init failed\n"); return 1; }
    mCoreInitConfig(core, NULL);
    core->desiredVideoDimensions(core, &W, &H);
    buf = calloc(W * H, sizeof(color_t));
    core->setVideoBuffer(core, buf, W);
    if (!mCoreLoadFile(core, argv[1])) { fprintf(stderr, "load failed\n"); return 1; }
    core->reset(core);
    FILE *sc = fopen(argv[2], "r");
    char line[256];
    long total = 0;
    while (fgets(line, sizeof line, sc)) {
        char cmd[32], arg[64];
        int n = 0;
        int k = sscanf(line, "%31s %63s %d", cmd, arg, &n);
        if (k < 1 || cmd[0] == '#') continue;
        if (!strcmp(cmd, "wait")) { n = atoi(arg); run(0, n); total += n; }
        else if (!strcmp(cmd, "press")) { if (k < 3) n = 6; run(keymask(arg), n); run(0, 6); total += n + 6; }
        else if (!strcmp(cmd, "hold")) { run(keymask(arg), n); total += n; }
        else if (!strcmp(cmd, "mash")) { for (int i = 0; i < n; i += 12) { run(keymask(arg), 4); run(0, 8); } total += n; }
        else if (!strcmp(cmd, "shot")) shot(arg);
        else if (!strcmp(cmd, "reset")) core->reset(core);
        else if (!strcmp(cmd, "peek")) { unsigned a = strtoul(arg, 0, 16); printf("peek %x = %u\n", a, core->rawRead8(core, a, -1)); }
        else if (!strcmp(cmd, "save") || !strcmp(cmd, "load")) {
            char path[512];
            snprintf(path, sizeof path, "%s/%s.state", outdir, arg);
            size_t sz = core->stateSize(core);
            void *st = malloc(sz);
            if (cmd[0] == 's') { core->saveState(core, st); FILE *f = fopen(path, "wb"); fwrite(st, 1, sz, f); fclose(f); }
            else { FILE *f = fopen(path, "rb"); if (f) { fread(st, 1, sz, f); fclose(f); core->loadState(core, st); } }
            free(st);
        }
        else if (!strcmp(cmd, "poke")) { /* poke ADDR VALUE (hex) */
            unsigned a = strtoul(arg, 0, 16); core->rawWrite8(core, a, -1, n);
        }
    }
    fprintf(stderr, "ran %ld frames\n", total);
    core->deinit(core);
    return 0;
}
