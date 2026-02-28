# ═══════════════════════════════════════════════════════════════════════════
# Apostrophe — my355 (Miyoo Mini Flip) cross-compile Makefile
# ═══════════════════════════════════════════════════════════════════════════
# Called by root Makefile inside Docker container.
# Based on Undertune / scrapegoat-pak conventions.

CROSS_COMPILE ?= aarch64-nextui-linux-gnu-
CC      := $(CROSS_COMPILE)gcc
SYSROOT := /opt/aarch64-nextui-linux-gnu/aarch64-nextui-linux-gnu/libc/usr

PKG_CONFIG := pkg-config
SDL_CFLAGS  := $(shell $(PKG_CONFIG) --cflags sdl2 SDL2_ttf SDL2_image)
SDL_LDFLAGS := $(shell $(PKG_CONFIG) --libs sdl2 SDL2_ttf SDL2_image)

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
BUILD_DIR ?= /workspace/build/my355/$(EXAMPLE)
SRC       := /workspace/examples/$(EXAMPLE)/main.c

CFLAGS := -std=gnu11 -O2 -Wall -Wextra -Wno-unused-parameter \
	-mcpu=cortex-a55 -mtune=cortex-a55 \
	-DPLATFORM_MY355 \
	-I/workspace/include \
	$(SDL_CFLAGS) $(CURL_CFLAGS)

LDFLAGS := $(SDL_LDFLAGS) $(CURL_LDFLAGS) -lpthread -ldl -lm

.PHONY: all clean

all: $(BUILD_DIR)/$(EXAMPLE)

$(BUILD_DIR)/$(EXAMPLE): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

