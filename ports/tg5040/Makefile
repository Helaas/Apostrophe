# ═══════════════════════════════════════════════════════════════════════════
# Apostrophe — tg5040 (TrimUI Brick / Smart Pro) cross-compile Makefile
# ═══════════════════════════════════════════════════════════════════════════
# Called by root Makefile inside Docker container.
# Based on Undertune / scrapegoat-pak conventions.

CROSS_COMPILE ?= aarch64-nextui-linux-gnu-
CC      := $(CROSS_COMPILE)gcc
SYSROOT := /opt/aarch64-nextui-linux-gnu/aarch64-nextui-linux-gnu/libc/usr

PKG_CONFIG := pkg-config
# NOTE: SDL2_ttf.pc and SDL2_image.pc are missing from the Docker toolchain
# sysroots, but the .so stubs exist.  Use sdl2.pc only and add -l flags manually.
SDL_CFLAGS  := $(shell $(PKG_CONFIG) --cflags sdl2)
SDL_LDFLAGS := $(shell $(PKG_CONFIG) --libs sdl2) -lSDL2_ttf -lSDL2_image

# libcurl (optional — enable with CURL=1 or auto-detect)
CURL ?= $(shell $(PKG_CONFIG) --exists libcurl 2>/dev/null && echo 1 || echo 0)
ifeq ($(CURL),1)
CURL_CFLAGS := $(shell $(PKG_CONFIG) --cflags libcurl) -DAP_ENABLE_CURL
CURL_LDFLAGS := $(shell $(PKG_CONFIG) --libs libcurl)
else
CURL_CFLAGS :=
CURL_LDFLAGS :=
endif

EXAMPLE   ?= hello
BUILD_DIR ?= /workspace/build/tg5040/$(EXAMPLE)
SRC       := /workspace/examples/$(EXAMPLE)/main.c
DEPS      := /workspace/include/apostrophe.h /workspace/include/apostrophe_widgets.h

CFLAGS := -std=gnu11 -O2 -Wall -Wextra -Wno-unused-parameter \
	-mcpu=cortex-a53 -mtune=cortex-a53 \
	-DPLATFORM_TG5040 \
	-I/workspace/include \
	$(SDL_CFLAGS) $(CURL_CFLAGS)

LDFLAGS := $(SDL_LDFLAGS) $(CURL_LDFLAGS) -lpthread -ldl -lm

.PHONY: all clean

all: $(BUILD_DIR)/$(EXAMPLE)

$(BUILD_DIR)/$(EXAMPLE): $(SRC) $(DEPS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
