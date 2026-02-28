# ═══════════════════════════════════════════════════════════════════════════
# Apostrophe — tg5040 (TrimUI Brick / Smart Pro) cross-compile Makefile
# ═══════════════════════════════════════════════════════════════════════════
# Called by root Makefile inside Docker container.

CC      := aarch64-nextui-linux-gnu-gcc
SYSROOT := $(shell $(CC) -print-sysroot)

EXAMPLE   ?= hello
BUILD_DIR ?= build/tg5040/$(EXAMPLE)
SRC       := examples/$(EXAMPLE)/main.c

CFLAGS := -std=gnu11 -O2 \
	-mcpu=cortex-a53 -mtune=cortex-a53 \
	-DPLATFORM_TG5040 \
	-I include \
	-I$(SYSROOT)/usr/include/SDL2

LDFLAGS := -lSDL2 -lSDL2_ttf -lSDL2_image -lpthread -ldl -lm

.PHONY: all clean

all: $(BUILD_DIR)/$(EXAMPLE)

$(BUILD_DIR)/$(EXAMPLE): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
