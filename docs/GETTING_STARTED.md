# Getting Started with Apostrophe

This guide walks you through creating your first NextUI Pak using Apostrophe.
It reflects the same overall setup used by the current real projects such as `nextui-shortcuts-pak` and `nextui-scrapegoat-pak`.

This guide targets Apostrophe **v1.1.0** (2026-03-30).

## Prerequisites

### macOS (Development)

```bash
# Install Homebrew dependencies
brew install sdl2 sdl2_ttf sdl2_image

# Optional: libcurl for the Download Manager widget
brew install curl

# Optional: Docker for cross-compiling to device
brew install --cask docker
```

### Linux (Development)

```bash
# Debian/Ubuntu
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev

# Optional: libcurl for the Download Manager widget
sudo apt install libcurl4-openssl-dev

# Fedora
sudo dnf install SDL2-devel SDL2_ttf-devel SDL2_image-devel

# Arch
sudo pacman -S sdl2 sdl2_ttf sdl2_image

# Optional: Docker for cross-compiling to device
# Follow Docker's official install instructions for your distro
```

### Windows (Development)

Requires [MSYS2](https://www.msys2.org/). Run these commands in an MSYS2 MinGW64 shell:

```bash
# x86_64
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-pkg-config make

# ARM64 (use CLANGARM64 shell instead)
pacman -S mingw-w64-clang-aarch64-gcc mingw-w64-clang-aarch64-SDL2 mingw-w64-clang-aarch64-SDL2_ttf mingw-w64-clang-aarch64-SDL2_image mingw-w64-clang-aarch64-pkg-config make

# Optional: libcurl for the Download Manager widget
pacman -S mingw-w64-x86_64-curl  # or mingw-w64-clang-aarch64-curl

# Optional: Docker for cross-compiling to device
# Install Docker Desktop for Windows
```

### Device Cross-Compilation

You need Docker installed and running. The build system automatically pulls the correct toolchain image for each platform.

### Device Download Manager (Bundled curl)

`ap_download_manager` on device builds uses a bundled libcurl toolchain flow.

- Default behavior: `EXAMPLE=download` enables `USE_BUNDLED_CURL=1` automatically.
- Sources are downloaded once and cached in `build/third_party/sources`.
- Built dependencies are stored per-platform under `build/third_party/<platform>/...`.
- Device targets automatically build OpenSSL first, then curl (TLS-enabled).
- curl and OpenSSL are linked statically; the `lib` directory in the pak contains only `cacert.pem` for SSL certificate verification.

You can force-enable it explicitly:

```bash
make tg5040-download USE_BUNDLED_CURL=1
make tg5050-download USE_BUNDLED_CURL=1
make my355-download USE_BUNDLED_CURL=1
```

## Project Structure

A small experiment can be very simple:

```
MyPak/
├── main.c              # Your application code
├── font.ttf            # Optional desktop test font
├── Makefile            # Build configuration
├── launch.sh           # NextUI launch script
└── pak.json            # Pak metadata
```

Real multi-platform Apostrophe paks usually look more like this:

```
MyProject/
├── Makefile
├── launch.sh
├── pak.json
├── src/
│   └── main.c
├── ports/
│   ├── tg5040/Makefile
│   ├── tg5050/Makefile
│   └── my355/Makefile
└── third_party/
    └── apostrophe/
        ├── include/
        │   ├── apostrophe.h
        │   └── apostrophe_widgets.h
        └── res/
            └── font.ttf
```

That second layout is closer to how real projects are currently structured: application code under `src/`, platform packaging under `ports/`, and Apostrophe vendored under `third_party/apostrophe`.

## Step 1: Include the Headers

Create a `main.c` file. Define the implementation macros in exactly **one** translation unit:

```c
#define AP_IMPLEMENTATION
#include "apostrophe.h"
#define AP_WIDGETS_IMPLEMENTATION
#include "apostrophe_widgets.h"
```

If your project has multiple `.c` files, only one should have the `#define` lines. All others just `#include` the headers normally.

If you vendor Apostrophe under `third_party/apostrophe`, add `-Ithird_party/apostrophe/include` to your compiler flags and keep the `#include "apostrophe.h"` form shown above.

## Step 2: Initialise

```c
int main(int argc, char *argv[]) {
    ap_config cfg = {
        .window_title = "My Pak",
        .font_path    = AP_PLATFORM_IS_DEVICE ? NULL
                        : "third_party/apostrophe/res/font.ttf",
        .log_path     = ap_resolve_log_path("myapp"), // optional helper
        .is_nextui    = AP_PLATFORM_IS_DEVICE,
    };

    if (ap_init(&cfg) != AP_OK) {
        return 1;
    }

    // ... your UI code ...

    ap_quit();
    return 0;
}
```

`ap_init()` handles everything: SDL initialisation, window/renderer creation, font loading, theme loading (on device), input setup, and screen size detection.
`ap_resolve_log_path()` maps logs to `LOGS_PATH`, then `SHARED_USERDATA_PATH/logs`, then `HOME/.userdata/logs`.

`font_path` is mainly a desktop concern. On device, leave it as `NULL` and Apostrophe will load the font selected in NextUI (`font1.ttf` / `font2.ttf`) automatically. For desktop testing, pointing `font_path` at a bundled `.ttf` gives you predictable rendering across machines.

## Step 3: Show a Widget

All widgets are **blocking** — they run their own event loop and return when the user takes an action.

```c
// Create list items
ap_list_item items[] = {
    { .label = "Play Game" },
    { .label = "Settings"  },
    { .label = "About"     },
};

// Create footer hints
ap_footer_item footer[] = {
    { .button = AP_BTN_B, .label = "Quit" },
    { .button = AP_BTN_A, .label = "Select", .is_confirm = true },
};

// Configure the list
ap_list_opts opts = ap_list_default_opts("Main Menu", items, 3);
opts.footer       = footer;
opts.footer_count = 2;
// Footer hints are visual only; behavior comes from action bindings.
opts.action_button = AP_BTN_A;  // optional custom trigger

// Show it (blocks until user acts)
ap_list_result result;
int rc = ap_list(&opts, &result);

if (rc == AP_OK) {
    printf("Selected: %d\n", result.selected_index);
} else if (rc == AP_CANCELLED) {
    printf("User pressed back\n");
}
```

## Step 4: Build & Run

### Any platform (auto-detect)

```bash
make native
make run-native

# Optional: fetch the pinned NextUI preview cache used by the macOS demo aliases
# and reusable on Linux/Windows via the AP_* overrides
make setup-nextui-preview-cache

# Override the desktop preview resolution to match device targets
# Substitute other width/height values as needed.
AP_WINDOW_WIDTH=1024 AP_WINDOW_HEIGHT=768 make run-native-demo   # Brick
AP_WINDOW_WIDTH=1280 AP_WINDOW_HEIGHT=720 make run-native-demo   # Smart Pro / Smart Pro S
AP_WINDOW_WIDTH=640 AP_WINDOW_HEIGHT=480 make run-native-demo    # Miyoo Flip
```

`run-native-demo` resolves to the host-specific demo target. On macOS, the `run-mac-demo` and `run-mac-download` aliases automatically point at `.cache/nextui-preview`. On Linux/Windows, export `AP_STATUS_ASSETS_DIR`, `AP_NEXTVAL_PATH`, and `AP_MINUI_SETTINGS_PATH` yourself if you want the same NextUI preview assets.

### macOS

```bash
make mac
make run-mac
make run-mac-demo      # auto-uses .cache/nextui-preview when available
make run-mac-download  # same desktop NextUI preview path for the status bar demo

# Same preview override when running the macOS target directly
# Substitute other width/height values as needed.
AP_WINDOW_WIDTH=1024 AP_WINDOW_HEIGHT=768 make run-mac-demo
```

`make setup-nextui-preview-cache` sparse-checks out the pinned NextUI sprite sheets into
`.cache/nextui-preview/` and generates local `nextval.json` and `minuisettings.txt` fixtures.
`run-mac-demo` and `run-mac-download` automatically point `AP_STATUS_ASSETS_DIR`,
`AP_NEXTVAL_PATH`, and `AP_MINUI_SETTINGS_PATH` at that cache unless you already exported your
own override values.

### Linux

```bash
make linux
make run-linux
```

### Windows (MSYS2 MinGW shell)

```bash
make windows
make run-windows
```

### Keyboard Controls

When running on desktop (macOS, Linux, or Windows), keyboard keys are mapped to controller buttons matching [Gabagool's `DefaultInputMapping()`](https://github.com/BrandonKowalski/gabagool):

| Key | Button | | Key | Button |
|-----|--------|-|-----|--------|
| `A` | A (confirm) | | `L` | L1 |
| `B` | B (back) | | `;` | L2 |
| `X` | X | | `R` | R1 |
| `Y` | Y | | `T` | R2 |
| Arrow keys | D-pad | | `Return` | Start |
| `H` | Menu | | `Space` | Select |

> **Note:** This matches the physical layout on the device — press `a` on your keyboard to confirm/select, `b` to go back. `Return` maps to Start (not confirm) and `Space` maps to Select.

### Device

```bash
make tg5040        # or tg5050 or my355
make package       # Creates .pakz archive
make deploy        # Push to connected device via adb
```

## Step 5: Create the Launch Script

Every Pak needs a `launch.sh`:

```bash
#!/bin/sh
set -eu

APP_BIN="myapp"
PAK_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PAK_NAME=$(basename "$PAK_DIR")
PAK_NAME=${PAK_NAME%.pak}

cd "$PAK_DIR"

# Point OpenSSL/curl to bundled CA certificates for SSL verification
if [ -f "$PAK_DIR/lib/cacert.pem" ]; then
    export SSL_CERT_FILE="$PAK_DIR/lib/cacert.pem"
fi

SHARED_USERDATA_ROOT=${SHARED_USERDATA_PATH:-"${HOME:-/tmp}/.userdata/shared"}
LOG_ROOT=${LOGS_PATH:-"$SHARED_USERDATA_ROOT/logs"}
mkdir -p "$LOG_ROOT"
LOG_FILE="$LOG_ROOT/$APP_BIN.txt"
: >"$LOG_FILE"

exec >>"$LOG_FILE"
exec 2>&1

echo "=== Launching $PAK_NAME ($APP_BIN) at $(date) ==="
echo "platform=${PLATFORM:-unknown} device=${DEVICE:-unknown}"
echo "args: $*"

exec "./$APP_BIN" "$@"
```

Make it executable: `chmod +x launch.sh`

## Key Concepts

### Return Codes

All widget functions return:
- `AP_OK` (0) — Action completed successfully
- `AP_ERROR` (-1) — Something went wrong
- `AP_CANCELLED` (-2) — User pressed back

### Scaling with `AP_S()`

Never hard-code pixel sizes. Use the scaling macro:

```c
int margin = AP_S(20);           // 20px at 1024-width reference
int padding = AP_S(10);          // 10px at 1024-width reference
```

This automatically adapts to the target screen resolution.

### The Theme

Access current theme colors via `ap_get_theme()`:

```c
ap_theme *t = ap_get_theme();
ap_draw_rounded_rect(x, y, w, h, AP_S(8), t->highlight);
```

### Fonts

Get sized fonts with `ap_get_font()`:

```c
TTF_Font *font = ap_get_font(AP_FONT_MEDIUM);
ap_draw_text(font, "Hello!", x, y, ap_get_theme()->text);
```

Font tiers (base × device_scale, matching NextUI): Extra Large (24), Large (16), Medium (14), Small (12), Tiny (10), Micro (7). At device_scale=2 (MY355, TG5050): 48, 32, 28, 24, 20, 14px.

## Next Steps

- Read the [API Reference](API.md) for the complete function list
- Browse the [Widget Catalog](WIDGETS.md) for all available components
- Check the [Porting from Gabagool](PORTING_FROM_GABAGOOL.md) guide if migrating from Go
- Study the `examples/demo/main.c` for a comprehensive usage example
