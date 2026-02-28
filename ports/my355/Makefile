# ═══════════════════════════════════════════════════════════════════════════
# Apostrophe — my355 (Miyoo Mini Flip) cross-compile Makefile
# ═══════════════════════════════════════════════════════════════════════════
# Called by root Makefile inside Docker container.

CC      := aarch64-nextui-linux-gnu-gcc
SYSROOT := $(shell $(CC) -print-sysroot)

EXAMPLE   ?= hello
BUILD_DIR ?= build/my355/$(EXAMPLE)
SRC       := examples/$(EXAMPLE)/main.c

CFLAGS := -std=gnu11 -O2 \
	-mcpu=cortex-a55 -mtune=cortex-a55 \
	-DPLATFORM_MY355 \
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
