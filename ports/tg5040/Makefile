# ═══════════════════════════════════════════════════════════════════════════
# Apostrophe — tg5040 (TrimUI Brick / Smart Pro) cross-compile Makefile
# ═══════════════════════════════════════════════════════════════════════════
# Called by root Makefile inside Docker container.
# Based on Undertune / scrapegoat-pak conventions.

CROSS_COMPILE ?= aarch64-nextui-linux-gnu-
CC      := $(CROSS_COMPILE)gcc
SYSROOT := /opt/aarch64-nextui-linux-gnu/aarch64-nextui-linux-gnu/libc/usr
PLATFORM_ID := tg5040
THIRD_PARTY_SCRIPT := /workspace/scripts/build_third_party.sh

PKG_CONFIG := pkg-config
# NOTE: SDL2_ttf.pc and SDL2_image.pc are missing from the Docker toolchain
# sysroots, but the .so stubs exist.  Use sdl2.pc only and add -l flags manually.
SDL_CFLAGS  := $(shell $(PKG_CONFIG) --cflags sdl2)
SDL_LDFLAGS := $(shell $(PKG_CONFIG) --libs sdl2) -lSDL2_ttf -lSDL2_image

EXAMPLE   ?= hello
BUILD_DIR ?= /workspace/build/tg5040/$(EXAMPLE)
SRC       := /workspace/examples/$(EXAMPLE)/main.c
DEPS      := /workspace/include/apostrophe.h /workspace/include/apostrophe_widgets.h
USE_BUNDLED_CURL ?= $(if $(filter download,$(EXAMPLE)),1,0)

CURL_CFLAGS :=
CURL_LDFLAGS :=

ifeq ($(USE_BUNDLED_CURL),1)
CURL_PREFIX := /workspace/build/third_party/$(PLATFORM_ID)/curl/install
OPENSSL_PREFIX := /workspace/build/third_party/$(PLATFORM_ID)/openssl/install
OPENSSL_LIBSSL := $(OPENSSL_PREFIX)/lib/libssl.a
OPENSSL_LIBCRYPTO := $(OPENSSL_PREFIX)/lib/libcrypto.a
CURL_STAMP  := $(CURL_PREFIX)/.stamp-curl-8.7.1-static2
DEPS += $(CURL_STAMP)
CURL_CFLAGS += -I$(CURL_PREFIX)/include -DAP_ENABLE_CURL -ffunction-sections -fdata-sections
CURL_LDFLAGS += \
	-Wl,--whole-archive $(CURL_PREFIX)/lib/libcurl.a -Wl,--no-whole-archive \
	$(OPENSSL_LIBSSL) $(OPENSSL_LIBCRYPTO) \
	-lz -Wl,--gc-sections
endif

CFLAGS := -std=gnu11 -O2 -Wall -Wextra -Wno-unused-parameter \
	-mcpu=cortex-a53 -mtune=cortex-a53 \
	-DPLATFORM_TG5040 \
	-I/workspace/include \
	$(SDL_CFLAGS) $(CURL_CFLAGS)

LDFLAGS := $(SDL_LDFLAGS) $(CURL_LDFLAGS) -lpthread -ldl -lm

.PHONY: all clean

all: $(BUILD_DIR)/$(EXAMPLE)

ifeq ($(USE_BUNDLED_CURL),1)
$(CURL_STAMP): /workspace/scripts/build_third_party.sh
	$(THIRD_PARTY_SCRIPT) ensure-curl $(PLATFORM_ID)
endif

$(BUILD_DIR)/$(EXAMPLE): $(SRC) $(DEPS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@if [ "$(USE_BUNDLED_CURL)" = "1" ]; then \
		$(THIRD_PARTY_SCRIPT) stage-runtime-libs $(PLATFORM_ID) "$@" "$(BUILD_DIR)/lib"; \
	else \
		rm -rf "$(BUILD_DIR)/lib"; \
	fi

clean:
	rm -rf $(BUILD_DIR)
