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
           -ffunction-sections -fdata-sections -fno-strict-aliasing -I$(SRC) -I$(GEN)
LDFLAGS := $(ARCH) -nostartfiles -T $(SRC)/gba.ld -Wl,--gc-sections \
           --specs=nano.specs --specs=nosys.specs

CSRC    := $(wildcard $(SRC)/*.c)
OBJS    := $(BUILD)/crt0.o $(CSRC:$(SRC)/%.c=$(BUILD)/%.o) $(BUILD)/assets.o

all: $(TARGET).gba

$(GEN)/assets.c $(GEN)/assets.h: tools/gen_assets.py
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

clean:
	rm -rf $(BUILD) $(TARGET).gba

.PHONY: all clean
