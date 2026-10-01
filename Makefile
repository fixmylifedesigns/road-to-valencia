# Road to Valencia: builds road-to-valencia.gba
CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
PYTHON  ?= python3
HOSTCC  ?= cc

TARGET  := road-to-valencia
VERSION := $(shell cat VERSION)
TITLE   := ROADTOVLNCIA
GAMECODE:= RTVE
MAKER   := FM

ARCH    := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS  := $(ARCH) -O2 -Wall -Wextra -Wno-unused-parameter -ffreestanding -fno-builtin \
           -fno-tree-loop-distribute-patterns -fno-strict-aliasing -Ibuild -Isrc \
           -DGAME_VERSION=\"v$(VERSION)\"
LDFLAGS := $(ARCH) -nostartfiles -nostdlib -T src/gba.ld -Wl,--gc-sections

SRC     := $(wildcard src/*.c) build/assets.c
OBJS    := build/crt0.o $(patsubst %.c,build/%.o,$(notdir $(SRC)))

$(shell mkdir -p build)

all: $(TARGET).gba

build/assets.c build/assets.h: $(wildcard tools/*.py) tools/font8x8_basic.h
	$(PYTHON) tools/gen_assets.py build

# Public-domain 8x8 font by Daniel Hepper, fetched on first build.
tools/font8x8_basic.h:
	curl -sfL https://raw.githubusercontent.com/dhepper/font8x8/master/font8x8_basic.h -o $@

build/crt0.o: src/crt0.s
	$(CC) $(ARCH) -c $< -o $@

build/%.o: src/%.c build/assets.h $(wildcard src/*.h) VERSION
	$(CC) $(CFLAGS) -c $< -o $@

build/assets.o: build/assets.c
	$(CC) $(CFLAGS) -c $< -o $@

build/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -lgcc -o $@

build/gbafix:
	curl -sfL https://raw.githubusercontent.com/devkitPro/gba-tools/master/src/gbafix.c -o build/gbafix.c
	$(HOSTCC) -O2 build/gbafix.c -o $@

$(TARGET).gba: build/$(TARGET).elf build/gbafix
	$(OBJCOPY) -O binary $< $@
	build/gbafix $@ -t$(TITLE) -c$(GAMECODE) -m$(MAKER) -r0

clean:
	rm -rf build $(TARGET).gba

.PHONY: all clean
