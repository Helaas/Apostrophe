# ═══════════════════════════════════════════════════════════════════════════
# Apostrophe — Build System
# ═══════════════════════════════════════════════════════════════════════════
#
# Targets:
#   make mac          — Build examples natively for macOS
#   make tg5040       — Cross-compile for TrimUI Brick/Smart Pro
#   make tg5050       — Cross-compile for TrimUI Smart Pro S
#   make my355        — Cross-compile for Miyoo Mini Flip
#   make all          — Build for all device platforms
#   make package      — Build all + create .pakz archives
#   make deploy       — Deploy to connected device via adb
#   make clean        — Remove build artifacts
#   make help         — Show this help
#
# ═══════════════════════════════════════════════════════════════════════════

SHELL := /bin/bash

# Docker toolchain images
TG5040_TOOLCHAIN := ghcr.io/loveretro/tg5040-toolchain:latest
TG5050_TOOLCHAIN := ghcr.io/loveretro/tg5050-toolchain:latest
MY355_TOOLCHAIN  := ghcr.io/loveretro/my355-toolchain:latest

# Directories
BUILD_DIR    := build
DIST_DIR     := $(BUILD_DIR)/dist
STAGING_DIR  := $(BUILD_DIR)/staging
EXAMPLES_DIR := examples
INCLUDE_DIR  := include
RES_DIR      := res

# Example names
EXAMPLES := hello demo download

# ─── Phony targets ───────────────────────────────────────────────────────

.PHONY: all mac tg5040 tg5050 my355 package deploy clean help

all: tg5040 tg5050 my355

# ─── macOS (native) ─────────────────────────────────────────────────────

mac: $(EXAMPLES:%=mac-%)

# libcurl support (optional — enable with CURL=1 or auto-detect)
CURL ?= $(shell pkg-config --exists libcurl 2>/dev/null && echo 1 || echo 0)
ifeq ($(CURL),1)
CURL_CFLAGS := $(shell pkg-config --cflags libcurl) -DAP_ENABLE_CURL
CURL_LDFLAGS := $(shell pkg-config --libs libcurl)
else
CURL_CFLAGS :=
CURL_LDFLAGS :=
endif

mac-%:
	@echo "════════ Building $* for macOS ════════"
	@mkdir -p $(BUILD_DIR)/mac/$*
	cc -std=gnu11 -O0 -g \
		-DPLATFORM_MAC \
		-I$(INCLUDE_DIR) \
		$(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image) \
		$(CURL_CFLAGS) \
		-o $(BUILD_DIR)/mac/$*/$* \
		$(EXAMPLES_DIR)/$*/main.c \
		$(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image) \
		$(CURL_LDFLAGS) \
		-lm -lpthread
	@cp -f $(RES_DIR)/font.ttf $(BUILD_DIR)/mac/$*/font.ttf
	@echo "→ $(BUILD_DIR)/mac/$*/$*"

run-mac-%: mac-%
	@cd $(BUILD_DIR)/mac/$* && ./$*

run-mac: run-mac-hello

# ─── TG5040 (TrimUI Brick / Smart Pro) via Docker ──────────────────────

tg5040: $(EXAMPLES:%=tg5040-%)

tg5040-%:
	@echo "════════ Building $* for tg5040 ════════"
	@mkdir -p $(BUILD_DIR)/tg5040/$*
	docker run --rm \
		-v "$(CURDIR)":/workspace \
		$(TG5040_TOOLCHAIN) \
		make -C /workspace -f ports/tg5040/Makefile \
			EXAMPLE=$* \
			BUILD_DIR=/workspace/$(BUILD_DIR)/tg5040/$*
	@echo "→ $(BUILD_DIR)/tg5040/$*/$*"

# ─── TG5050 (TrimUI Smart Pro S) via Docker ────────────────────────────

tg5050: $(EXAMPLES:%=tg5050-%)

tg5050-%:
	@echo "════════ Building $* for tg5050 ════════"
	@mkdir -p $(BUILD_DIR)/tg5050/$*
	docker run --rm \
		-v "$(CURDIR)":/workspace \
		$(TG5050_TOOLCHAIN) \
		make -C /workspace -f ports/tg5050/Makefile \
			EXAMPLE=$* \
			BUILD_DIR=/workspace/$(BUILD_DIR)/tg5050/$*
	@echo "→ $(BUILD_DIR)/tg5050/$*/$*"

# ─── MY355 (Miyoo Mini Flip) via Docker ────────────────────────────────

my355: $(EXAMPLES:%=my355-%)

my355-%:
	@echo "════════ Building $* for my355 ════════"
	@mkdir -p $(BUILD_DIR)/my355/$*
	docker run --rm \
		-v "$(CURDIR)":/workspace \
		$(MY355_TOOLCHAIN) \
		make -C /workspace -f ports/my355/Makefile \
			EXAMPLE=$* \
			BUILD_DIR=/workspace/$(BUILD_DIR)/my355/$*
	@echo "→ $(BUILD_DIR)/my355/$*/$*"

# ─── Packaging ──────────────────────────────────────────────────────────

package: all
	@echo "════════ Packaging ════════"
	@rm -rf $(STAGING_DIR) $(DIST_DIR)
	@mkdir -p $(DIST_DIR)
	@for example in $(EXAMPLES); do \
		echo "Packaging $$example..."; \
		for platform in tg5040 tg5050 my355; do \
			pak_name=$$(echo "$$example" | sed 's/.*/\u&/'); \
			pak_dir="$(STAGING_DIR)/Tools/$$platform/$${pak_name}.pak"; \
			mkdir -p "$$pak_dir"; \
			cp -f "$(BUILD_DIR)/$$platform/$$example/$$example" "$$pak_dir/$$example" 2>/dev/null || true; \
			cp -f "$(RES_DIR)/font.ttf" "$$pak_dir/font.ttf"; \
			if [ -f "$(EXAMPLES_DIR)/$$example/pak/launch.sh" ]; then \
				cp -f "$(EXAMPLES_DIR)/$$example/pak/launch.sh" "$$pak_dir/launch.sh"; \
			fi; \
		done; \
		cd $(STAGING_DIR) && zip -r "$(CURDIR)/$(DIST_DIR)/$${example}.pakz" . && cd $(CURDIR); \
		rm -rf $(STAGING_DIR); \
	done
	@echo "→ Packages in $(DIST_DIR)/"

# ─── Deploy via adb ─────────────────────────────────────────────────────

deploy:
	@echo "Detecting platform..."
	@PLATFORM=$$(adb shell cat /tmp/minui_platform 2>/dev/null | tr -d '\r\n'); \
	if [ -z "$$PLATFORM" ]; then \
		echo "Error: Could not detect platform. Is device connected via adb?"; \
		exit 1; \
	fi; \
	echo "Detected platform: $$PLATFORM"; \
	for example in $(EXAMPLES); do \
		pak_name=$$(echo "$$example" | sed 's/.*/\u&/'); \
		echo "Deploying $$pak_name to $$PLATFORM..."; \
		adb push "$(BUILD_DIR)/$$PLATFORM/$$example/$$example" \
			"/mnt/SDCARD/Tools/$$PLATFORM/$${pak_name}.pak/$$example"; \
		adb push "$(RES_DIR)/font.ttf" \
			"/mnt/SDCARD/Tools/$$PLATFORM/$${pak_name}.pak/font.ttf"; \
		if [ -f "$(EXAMPLES_DIR)/$$example/pak/launch.sh" ]; then \
			adb push "$(EXAMPLES_DIR)/$$example/pak/launch.sh" \
				"/mnt/SDCARD/Tools/$$PLATFORM/$${pak_name}.pak/launch.sh"; \
		fi; \
	done
	@echo "Deploy complete."

# ─── Clean ──────────────────────────────────────────────────────────────

clean:
	rm -rf $(BUILD_DIR)

# ─── Help ───────────────────────────────────────────────────────────────

help:
	@echo ""
	@echo "Apostrophe Build System"
	@echo "══════════════════════════════════════"
	@echo ""
	@echo "  make mac            Build examples for macOS (native)"
	@echo "  make run-mac        Build + run hello example on macOS"
	@echo "  make run-mac-demo   Build + run demo example on macOS"
	@echo "  make tg5040         Cross-compile for TrimUI Brick/Smart Pro"
	@echo "  make tg5050         Cross-compile for TrimUI Smart Pro S"
	@echo "  make my355          Cross-compile for Miyoo Mini Flip"
	@echo "  make all            Build for all device platforms"
	@echo "  make package        Build all + create .pakz packages"
	@echo "  make deploy         Deploy to connected device via adb"
	@echo "  make clean          Remove build artifacts"
	@echo ""
	@echo "Examples: $(EXAMPLES)"
	@echo "Platforms: tg5040, tg5050, my355, mac"
	@echo ""
