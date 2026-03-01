# Apostrophe (')

A header-only C UI toolkit for building graphical tools (Paks) on retro gaming handhelds running [NextUI](https://github.com/nickvon/NextUI).

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
| `mac`    | macOS (dev/testing) | 1024×768 windowed | native host CPU |

## Quick Start

### 1. Clone

```bash
git clone https://github.com/LoveRetro/Apostrophe.git
cd Apostrophe
```

### 2. Build for macOS

Requires SDL2, SDL2_ttf, and SDL2_image installed via Homebrew:

```bash
brew install sdl2 sdl2_ttf sdl2_image

# Optional: libcurl for the Download Manager widget
brew install curl
make mac
make run-mac          # Runs the hello world example
make run-mac-demo     # Runs the widget demo
```

### 3. Build for Device

Requires Docker. Each platform has its own toolchain image:

```bash
make tg5040           # Cross-compile for TrimUI Brick/Smart Pro
make tg5050           # Cross-compile for TrimUI Smart Pro S
make my355            # Cross-compile for Miyoo Mini Flip
make all              # All device platforms
```

### 4. Package & Deploy

```bash
make package          # Create .pakz archives (zipped Pak bundles)
make deploy           # Push to connected device via adb
```

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
        .font_path    = "font.ttf",
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

## Architecture

```
apostrophe.h          — Core: init, lifecycle, input, drawing, theming, fonts, scaling
apostrophe_widgets.h  — Widgets: list, options list, keyboard, confirmation,
                        selection, process message, download manager,
                        detail screen, color picker, help overlay
```

All widgets use a **blocking model**: they run their own event loop and return a result struct when the user completes an action or presses back (`AP_CANCELLED`).

### Scaling

All pixel values are specified at a **1024px reference width** and automatically scaled to the target screen via the `AP_S(x)` macro. Screens wider than 1024px use 75% damping to prevent oversized UI.

### Theming

On device, colors are loaded from NextUI's theme system (`nextval.elf`). On macOS, sensible defaults are used. You can override the accent color via `ap_config.primary_color_hex`.

### Input

Apostrophe abstracts all input sources into a unified virtual button system (`AP_BTN_*`). On macOS and recognised gamepads it uses the SDL GameController API; on TrimUI devices it reads raw joystick events; and on the Miyoo Flip (my355) it maps hardware-specific keyboard scancodes. Directional buttons auto-repeat with configurable delay/rate.

## Widgets

| Widget | Function | Description |
|--------|----------|-------------|
| List | `ap_list()` | Scrollable item list with selection, multi-select, reorder |
| Options List | `ap_options_list()` | Settings-style list with cycle/keyboard/click/color options |
| Keyboard | `ap_keyboard()` | 5-row QWERTY keyboard (numbers, qwerty, asdf+enter, shift+zxcv+symbol, space) |
| URL Keyboard | `ap_url_keyboard()` | Keyboard with configurable URL shortcuts and symbol alternates |
| Download Manager | `ap_download_manager()` | Multi-threaded file downloader with per-file progress bars (requires libcurl) |
| Confirmation | `ap_confirmation()` | Modal message dialog |
| Selection | `ap_selection()` | Horizontal pill-style option chooser |
| Process Message | `ap_process_message()` | Async worker with progress bar |
| Detail Screen | `ap_detail_screen()` | Scrollable multi-section info view |
| Color Picker | `ap_color_picker()` | 5×5 color grid selector |
| Help Overlay | `ap_show_help_overlay()` | Scrollable text overlay (L1 trigger) |

## Docs

- [Getting Started](docs/GETTING_STARTED.md) — Step-by-step setup guide
- [API Reference](docs/API.md) — Complete function/struct reference
- [Widget Catalog](docs/WIDGETS.md) — Visual guide to every widget
- [Porting from Gabagool](docs/PORTING_FROM_GABAGOOL.md) — Migration guide from Go to C
- [Gabagool Parity v2.9.6](docs/GABAGOOL_PARITY_v2.9.6.md) — Feature parity matrix and backlog

## License

MIT — see [LICENSE](LICENSE).
