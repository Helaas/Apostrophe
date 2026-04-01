# Apostrophe (')

A header-only C UI toolkit for building graphical tools (Paks) on retro gaming handhelds running [NextUI](https://github.com/LoveRetro/NextUI).

Current release: **v1.1.0** (2026-03-30).

Inspired by [Gabagool](https://github.com/BrandonKowalski/gabagool) (Go). Its framework design directly informed the structure of this project, and this C port would not have been feasible without that foundation.

Thanks to Brandon T. Kowalski (https://github.com/BrandonKowalski) for creating Gabagool and publishing such a well-designed and practical reference implementation.

---

## Supported Platforms

> **Note:** The TrimUI Smart Brick is internally referred to as `tg3040`, but under NextUI it shares the `tg5040` platform with the Smart Pro.

| Platform | Device | Resolution | CPU |
|-----------|--------|------------|-----|
| `tg5040` | TrimUI Smart Pro | 1280×720 | Allwinner A133 Plus – Quad-core Cortex-A53 |
| `tg5040` | TrimUI Smart Brick (`tg3040` hardware) | 1024×768 | Allwinner A133 Plus – Quad-core Cortex-A53 |
| `tg5050` | TrimUI Smart Pro S | 1280×720 | Allwinner A523 – Octa-core Cortex-A55 |
| `my355`  | Miyoo Flip | 640×480 | Rockchip RK3566 – Quad-core Cortex-A55 |
| `mac`    | macOS (dev/testing) | Windowed preview (default 1024×768) | native host CPU |
| `linux`  | Linux (dev/testing) | Windowed preview (default 1024×768) | native host CPU |
| `windows` | Windows (MSYS2/MinGW dev/testing) | Windowed preview (default 1024×768) | native host CPU |

Desktop development/testing is supported on macOS, Linux, and Windows. `make native` auto-selects the current host target; `make windows` expects an MSYS2/MinGW shell.

## Quick Start

### 1. Clone

```bash
git clone https://github.com/Helaas/apostrophe.git
cd apostrophe
```

### 2. Build for Desktop

Desktop development/testing is supported on macOS, Linux, and Windows (MSYS2/MinGW). Install SDL2, SDL2_ttf, and SDL2_image for your host platform, then use the matching target:

```bash
make native
make run-native        # Runs the hello world example

# Preview other device resolutions on desktop
# Substitute other width/height values as needed.
AP_WINDOW_WIDTH=1024 AP_WINDOW_HEIGHT=768 make run-mac-demo   # Brick
AP_WINDOW_WIDTH=1280 AP_WINDOW_HEIGHT=720 make run-mac-demo   # Smart Pro / Smart Pro S
AP_WINDOW_WIDTH=640 AP_WINDOW_HEIGHT=480 make run-mac-demo    # Miyoo Flip

# `run-mac-demo` and `run-mac-download` automatically use `.cache/nextui-preview`
# unless you already exported AP_STATUS_ASSETS_DIR / AP_NEXTVAL_PATH / AP_MINUI_SETTINGS_PATH.

# Override with your own local asset/settings cache
AP_STATUS_ASSETS_DIR=/path/to/nextui-preview/assets \
AP_NEXTVAL_PATH=/path/to/nextui-preview/nextval.json \
AP_MINUI_SETTINGS_PATH=/path/to/nextui-preview/minuisettings.txt \
AP_PREVIEW_WIFI_STRENGTH=3 \
AP_PREVIEW_BATTERY_PERCENT=100 \
AP_PREVIEW_CHARGING=0 \
make run-mac-demo
```

For platform-specific dependency install commands, see [Getting Started](docs/GETTING_STARTED.md#prerequisites).
The `run-mac-demo` / `run-mac-download` aliases automatically point at `.cache/nextui-preview`. On Linux/Windows, the matching `run-linux-*` / `run-windows-*` targets work too, but you need to export `AP_STATUS_ASSETS_DIR`, `AP_NEXTVAL_PATH`, and `AP_MINUI_SETTINGS_PATH` yourself if you want the same NextUI preview assets.

### 3. Build for Device

Requires Docker. Each platform has its own toolchain image:

```bash
make tg5040           # Cross-compile for TrimUI Brick/Smart Pro
make tg5050           # Cross-compile for TrimUI Smart Pro S
make my355            # Cross-compile for Miyoo Flip
make all              # All device platforms
```

### 4. Package & Deploy

```bash
make package          # Create .pakz archives (zipped Pak bundles)
make deploy           # Push to connected device via adb
```

The preview cache target sparse-checks out only `skeleton/SYSTEM/res/assets@{1,2,3,4}x.png`
from `https://github.com/LoveRetro/NextUI.git` pinned to commit
`7d201cf293f3a253e09749b8bb002e0b9f66d652`, then generates local `nextval.json` and
`minuisettings.txt` files under `.cache/nextui-preview/`.

## Usage

Apostrophe is **header-only** (stb-style). In exactly **one** `.c` file:

```c
#define AP_IMPLEMENTATION
#include "apostrophe.h"
#define AP_WIDGETS_IMPLEMENTATION
#include "apostrophe_widgets.h"
```

All other files just include the headers normally (without the `#define`s).

### Minimal Example

```c
#define AP_IMPLEMENTATION
#include "apostrophe.h"
#define AP_WIDGETS_IMPLEMENTATION
#include "apostrophe_widgets.h"

int main(int argc, char *argv[]) {
    ap_config cfg = {
        .window_title = "My Pak",
        .font_path    = AP_PLATFORM_IS_DEVICE ? NULL : "font.ttf",
        .is_nextui    = AP_PLATFORM_IS_DEVICE,
    };
    ap_init(&cfg);

    ap_list_item items[] = {
        { .label = "Option A" },
        { .label = "Option B" },
        { .label = "Option C" },
    };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "Quit" },
        { .button = AP_BTN_A, .label = "Select", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Menu", items, 3);
    opts.footer       = footer;
    opts.footer_count = 2;

    ap_list_result result;
    ap_list(&opts, &result);

    ap_quit();
    return 0;
}
```

On device, leave `font_path` unset / `NULL` to use the font currently configured in NextUI. A bundled `.ttf` is mainly useful for consistent desktop preview/testing.

## Architecture

```
apostrophe.h          — Core: init, lifecycle, input, drawing, theming, fonts, scaling
apostrophe_widgets.h  — Widgets: list, options list, keyboard, confirmation,
                        selection, process message, download manager, queue manager,
                        detail screen, color picker, help overlay, file picker
```

All widgets use a **blocking model**: they run their own event loop and return a result struct when the user completes an action or presses back (`AP_CANCELLED`).

### Scaling

All pixel values are specified at a **1024px reference width** and automatically scaled to the target screen via the `AP_S(x)` macro. Screens wider than 1024px use 75% damping to prevent oversized UI.

### Theming

On device, colors are loaded from NextUI's theme system (`nextval.elf`). Apostrophe accepts both the current `color7` background key and the legacy `bgcolor` key for backward compatibility. On desktop, you can load the same theme and status-bar sprites by setting `AP_NEXTVAL_PATH`, `AP_STATUS_ASSETS_DIR`, and `AP_MINUI_SETTINGS_PATH`; otherwise sensible defaults are used. Apostrophe does not bundle the upstream NextUI sprite assets; `make setup-nextui-preview-cache` fetches them into `.cache/nextui-preview/` for local demo use only. You can still override the accent color via `ap_config.primary_color_hex`.

### Input

Apostrophe abstracts all input sources into a unified virtual button system (`AP_BTN_*`). On desktop (macOS, Linux, or Windows) and recognised gamepads it uses the SDL GameController API; on TrimUI devices it reads raw joystick events; and on the Miyoo Flip (my355) it maps hardware-specific keyboard scancodes. Directional buttons auto-repeat with configurable delay/rate. Miyoo Flip builds also use a higher analog deadzone than other targets to reduce accidental horizontal movement from the thumbstick.

The **combo system** adds support for chords (simultaneous button presses like L1+R1) and sequences (ordered presses like Up, Up, Down, Down). Register combos with `ap_register_chord()` / `ap_register_sequence()` and poll for events with `ap_poll_combo()`. See `examples/combo/` and the [API reference](docs/API.md#combos) for details.

## Widgets

| Widget | Function | Description |
|--------|----------|-------------|
| List | `ap_list()` | Scrollable item list with selection, multi-select, reorder, trailing hints, optional hidden scrollbar |
| Options List | `ap_options_list()` | Settings-style list with cycle/keyboard/click/color options |
| Keyboard | `ap_keyboard()` | 5-row QWERTY keyboard (numbers, qwerty, asdf+enter, shift+zxcv+symbol, space) |
| URL Keyboard | `ap_url_keyboard()` | Keyboard with configurable URL shortcuts, symbol alternates, and a `123` / `abc` number-symbol toggle |
| Download Manager | `ap_download_manager()` | Multi-threaded file downloader with per-file progress bars (requires libcurl) |
| Confirmation | `ap_confirmation()` | Modal message dialog |
| Selection | `ap_selection()` | Horizontal pill-style option chooser |
| Process Message | `ap_process_message()` | Async worker with progress bar |
| Queue Viewer | `ap_queue_viewer()` | Live-updating background job queue with filter, detail, cancel/clear, and inline progress |
| Detail Screen | `ap_detail_screen()` | Scrollable multi-section info view |
| Color Picker | `ap_color_picker()` | 5×5 color grid selector |
| File Picker | `ap_file_picker()` | Filesystem browser for selecting files or directories with optional folder creation in dir-capable modes |
| Help Overlay | `ap_show_help_overlay()` | Scrollable text overlay (Menu trigger) |

## Docs

- [Getting Started](docs/GETTING_STARTED.md) — Step-by-step setup guide
- [API Reference](docs/API.md) — Complete function/struct reference
- [Demo Coverage](docs/DEMO_COVERAGE.md) — Demo-to-API coverage matrix
- [Widget Catalog](docs/WIDGETS.md) — Visual guide to every widget
- [Porting from Gabagool](docs/PORTING_FROM_GABAGOOL.md) — Migration guide from Go to C
- [Gabagool Parity v2.9.6](docs/GABAGOOL_PARITY_v2.9.6.md) — Feature parity matrix and backlog

## License

Apostrophe source code is MIT-licensed — see [LICENSE](LICENSE).

The bundled [res/font.ttf](res/font.ttf) is distributed separately under the SIL Open Font License 1.1. See [res/font.LICENSE.txt](res/font.LICENSE.txt).
