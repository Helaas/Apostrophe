# ═══════════════════════════════════════════════════════════════════════════
# Apostrophe — Build System
# ═══════════════════════════════════════════════════════════════════════════
#
# Targets:
#   make native       — Build examples for the current host OS (auto-detect)
#   make mac          — Build examples natively for macOS
#   make linux        — Build examples natively for Linux
#   make windows      — Build examples natively for Windows (MSYS2/MinGW)
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

# Host platform auto-detection
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)
ifeq ($(UNAME_S),Darwin)
    NATIVE_PLATFORM := mac
else ifeq ($(UNAME_S),Linux)
    NATIVE_PLATFORM := linux
else
    NATIVE_PLATFORM := windows
endif

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
EXAMPLES := hello demo download combo perf

# ─── Phony targets ───────────────────────────────────────────────────────

.PHONY: all native mac linux windows tg5040 tg5050 my355 package deploy clean help

# ─── Native (auto-detect host OS) ─────────────────────────────────────

native: $(NATIVE_PLATFORM)
run-native: run-$(NATIVE_PLATFORM)

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
	@cp -f $(RES_DIR)/*.png $(BUILD_DIR)/mac/$*/ 2>/dev/null || true
	@echo "→ $(BUILD_DIR)/mac/$*/$*"

run-mac-%: mac-%
	@cd $(BUILD_DIR)/mac/$* && ./$*

run-mac: run-mac-hello

# ─── Linux (native) ───────────────────────────────────────────────────

linux: $(EXAMPLES:%=linux-%)

linux-%:
	@echo "════════ Building $* for Linux ════════"
	@mkdir -p $(BUILD_DIR)/linux/$*
	cc -std=gnu11 -O0 -g \
		-DPLATFORM_LINUX \
		-I$(INCLUDE_DIR) \
		$(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image) \
		$(CURL_CFLAGS) \
		-o $(BUILD_DIR)/linux/$*/$* \
		$(EXAMPLES_DIR)/$*/main.c \
		$(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image) \
		$(CURL_LDFLAGS) \
		-lm -lpthread
	@cp -f $(RES_DIR)/font.ttf $(BUILD_DIR)/linux/$*/font.ttf
	@cp -f $(RES_DIR)/*.png $(BUILD_DIR)/linux/$*/ 2>/dev/null || true
	@echo "→ $(BUILD_DIR)/linux/$*/$*"

run-linux-%: linux-%
	@cd $(BUILD_DIR)/linux/$* && ./$*

run-linux: run-linux-hello

# ─── Windows (MSYS2/MinGW) ────────────────────────────────────────────

windows: $(EXAMPLES:%=windows-%)

windows-%:
	@echo "════════ Building $* for Windows ════════"
	@mkdir -p $(BUILD_DIR)/windows/$*
	gcc -std=gnu11 -O0 -g \
		-DPLATFORM_WINDOWS \
		-I$(INCLUDE_DIR) \
		$(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image) \
		$(CURL_CFLAGS) \
		-o $(BUILD_DIR)/windows/$*/$*.exe \
		$(EXAMPLES_DIR)/$*/main.c \
		$(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image) \
		$(CURL_LDFLAGS) \
		-lm
	@cp -f $(RES_DIR)/font.ttf $(BUILD_DIR)/windows/$*/font.ttf
	@cp -f $(RES_DIR)/*.png $(BUILD_DIR)/windows/$*/ 2>/dev/null || true
	@echo "→ $(BUILD_DIR)/windows/$*/$*.exe"

run-windows-%: windows-%
	@cd $(BUILD_DIR)/windows/$* && ./$*.exe

run-windows: run-windows-hello

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
			pak_name=$$(printf '%s' "$$example" | awk '{print toupper(substr($$0,1,1)) substr($$0,2)}'); \
			pak_dir="$(STAGING_DIR)/Tools/$$platform/$${pak_name}.pak"; \
			mkdir -p "$$pak_dir"; \
			cp -f "$(BUILD_DIR)/$$platform/$$example/$$example" "$$pak_dir/$$example" 2>/dev/null || true; \
			if [ -f "$(EXAMPLES_DIR)/$$example/pak/launch.sh" ]; then \
				cp -f "$(EXAMPLES_DIR)/$$example/pak/launch.sh" "$$pak_dir/launch.sh"; \
			fi; \
			if [ -d "$(BUILD_DIR)/$$platform/$$example/lib" ]; then \
				mkdir -p "$$pak_dir/lib"; \
				cp -a "$(BUILD_DIR)/$$platform/$$example/lib/." "$$pak_dir/lib/"; \
			fi; \
		done; \
		cd $(STAGING_DIR) && zip -r "$(CURDIR)/$(DIST_DIR)/$${example}.pakz" . && cd $(CURDIR); \
		rm -rf $(STAGING_DIR); \
	done
	@echo "→ Packages in $(DIST_DIR)/"

# ─── Deploy via adb ─────────────────────────────────────────────────────

deploy:
	@echo "Detecting platform..."
	@SERIAL=$$(adb devices | awk 'NR>1 && $$2=="device" {print $$1; exit}'); \
	if [ -z "$$SERIAL" ]; then \
		echo "Error: No online adb device found."; \
		exit 1; \
	fi; \
	ADB="adb -s $$SERIAL"; \
	FINGERPRINT=$$($$ADB shell ' \
		cat /proc/device-tree/compatible 2>/dev/null; \
		echo; \
		cat /proc/device-tree/model 2>/dev/null; \
		echo; \
		uname -a 2>/dev/null' 2>/dev/null | tr '\000' '\n' | tr -d '\r'); \
	case "$$FINGERPRINT" in \
		*rk3566*|*miyoo-355*) PLATFORM=my355 ;; \
		*allwinner,a523*|*sun55iw3*) PLATFORM=tg5050 ;; \
		*allwinner,a133*|*sun50iw*) PLATFORM=tg5040 ;; \
		*allwinner*) \
			if printf '%s' "$$FINGERPRINT" | grep -qi 'a523'; then \
				PLATFORM=tg5050; \
			else \
				PLATFORM=tg5040; \
			fi \
			;; \
		*) \
			echo "Error: Could not detect a supported platform from adb fingerprint."; \
			echo "  Serial: $$SERIAL"; \
			echo "  Fingerprint snippet: $$(printf '%s' "$$FINGERPRINT" | head -c 240)"; \
			echo "Please report this fingerprint and add a mapping in Makefile deploy detection."; \
			exit 1; \
			;; \
	esac; \
	if [ -z "$$PLATFORM" ]; then \
		echo "Error: Platform detection failed for adb serial $$SERIAL."; \
		exit 1; \
	fi; \
	echo "Detected adb serial: $$SERIAL"; \
	echo "Detected platform: $$PLATFORM"; \
	for example in $(EXAMPLES); do \
		upper_name=$$(printf '%s' "$$example" | awk '{print toupper(substr($$0,1,1)) substr($$0,2)}'); \
		pak_dir="/mnt/SDCARD/Tools/$$PLATFORM/$${upper_name}.pak"; \
		echo "Deploying $$example to $$pak_dir..."; \
		$$ADB shell "mkdir -p '$$pak_dir'"; \
		$$ADB push "$(BUILD_DIR)/$$PLATFORM/$$example/$$example" "$$pak_dir/$$example"; \
		if [ -f "$(EXAMPLES_DIR)/$$example/pak/launch.sh" ]; then \
			$$ADB push "$(EXAMPLES_DIR)/$$example/pak/launch.sh" "$$pak_dir/launch.sh"; \
		fi; \
		if [ -d "$(BUILD_DIR)/$$PLATFORM/$$example/lib" ]; then \
			$$ADB shell "mkdir -p '$$pak_dir/lib'"; \
			$$ADB push "$(BUILD_DIR)/$$PLATFORM/$$example/lib/." "$$pak_dir/lib/"; \
		else \
			$$ADB shell "rm -rf '$$pak_dir/lib'"; \
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
	@echo "  Development (native):"
	@echo "  make native         Build for current host OS (detected: $(NATIVE_PLATFORM))"
	@echo "  make run-native     Build + run hello on current host OS"
	@echo "  make mac            Build examples for macOS"
	@echo "  make linux          Build examples for Linux"
	@echo "  make windows        Build examples for Windows (MSYS2/MinGW)"
	@echo ""
	@echo "  Device (cross-compile via Docker):"
	@echo "  make tg5040         Cross-compile for TrimUI Brick/Smart Pro"
	@echo "  make tg5050         Cross-compile for TrimUI Smart Pro S"
	@echo "  make my355          Cross-compile for Miyoo Mini Flip"
	@echo "  make all            Build for all device platforms"
	@echo "  make package        Build all + create .pakz packages"
	@echo "  make deploy         Deploy to connected device via adb"
	@echo ""
	@echo "  Other:"
	@echo "  make clean          Remove build artifacts"
	@echo ""
	@echo "Examples: $(EXAMPLES)"
	@echo "Dev platforms: mac, linux, windows"
	@echo "Device platforms: tg5040, tg5050, my355"
	@echo ""
