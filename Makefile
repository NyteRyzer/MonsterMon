# MonsterMons - GBA build
CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
PYTHON  := python3

TARGET  := MonsterMons
BUILD   := build
SRC     := src
GEN     := $(BUILD)/gen

ARCH    := -mthumb -mthumb-interwork -mcpu=arm7tdmi -mtune=arm7tdmi
CFLAGS  := $(ARCH) -O2 -Wall -Wextra -Wno-unused-parameter -std=gnu11 \
           -ffunction-sections -fdata-sections -fno-strict-aliasing -I$(SRC) -I$(GEN) $(EXTRA)
LDFLAGS := $(ARCH) -nostartfiles -T $(SRC)/gba.ld -Wl,--gc-sections \
           --specs=nano.specs --specs=nosys.specs

CSRC    := $(wildcard $(SRC)/*.c)
OBJS    := $(BUILD)/crt0.o $(CSRC:$(SRC)/%.c=$(BUILD)/%.o) $(BUILD)/assets.o

all: $(TARGET).gba

$(GEN)/assets.c $(GEN)/assets.h: $(wildcard tools/*.py)
	@mkdir -p $(GEN)
	$(PYTHON) tools/gen_assets.py $(GEN)

$(BUILD)/assets.o: $(GEN)/assets.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/crt0.o: $(SRC)/crt0.s
	@mkdir -p $(BUILD)
	$(CC) $(ARCH) -c $< -o $@

$(BUILD)/%.o: $(SRC)/%.c $(wildcard $(SRC)/*.h) $(GEN)/assets.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(TARGET).gba: $(BUILD)/$(TARGET).elf tools/gbafix.py
	$(OBJCOPY) -O binary $< $@
	$(PYTHON) tools/gbafix.py $@

# debug ROM: SELECT opens a warp/power/story-flag menu (for testing only)
debug:
	$(MAKE) BUILD=build/debug TARGET=MonsterMons_debug EXTRA=-DDEBUG

clean:
	rm -rf build $(TARGET).gba MonsterMons_debug.gba

.PHONY: all clean debug
