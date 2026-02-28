# Getting Started with Apostrophe

This guide walks you through creating your first NextUI Pak using Apostrophe.

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

### Device Cross-Compilation

You need Docker installed and running. The build system automatically pulls the correct toolchain image for each platform.

## Project Structure

A typical Apostrophe project looks like this:

```
MyPak/
├── main.c              # Your application code
├── font.ttf            # Font file (required)
├── Makefile            # Build configuration
└── pak/
    └── launch.sh       # NextUI launch script
```

For a multi-platform project using the Apostrophe repo structure:

```
MyProject/
├── include/
│   ├── apostrophe.h
│   └── apostrophe_widgets.h
├── res/
│   └── font.ttf
├── examples/
│   └── myapp/
│       ├── main.c
│       └── pak/
│           └── launch.sh
├── ports/
│   ├── tg5040/Makefile
│   ├── tg5050/Makefile
│   └── my355/Makefile
└── Makefile
```

## Step 1: Include the Headers

Create a `main.c` file. Define the implementation macros in exactly **one** translation unit:

```c
#define AP_IMPLEMENTATION
#include "apostrophe.h"
#define AP_WIDGETS_IMPLEMENTATION
#include "apostrophe_widgets.h"
```

If your project has multiple `.c` files, only one should have the `#define` lines. All others just `#include` the headers normally.

## Step 2: Initialise

```c
int main(int argc, char *argv[]) {
    ap_config cfg = {
        .window_title = "My Pak",
        .font_path    = "font.ttf",
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

### macOS

```bash
make mac
make run-mac
```

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
DIR="$(dirname "$0")"
cd "$DIR"
./myapp
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
int font_height = AP_S(44);     // Medium font size
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

Font tiers (at 1024px reference): Extra Large (60), Large (50), Medium (44), Small (34), Tiny (24), Micro (18).

## Next Steps

- Read the [API Reference](API.md) for the complete function list
- Browse the [Widget Catalog](WIDGETS.md) for all available components
- Check the [Porting from Gabagool](PORTING_FROM_GABAGOOL.md) guide if migrating from Go
- Study the `examples/demo/main.c` for a comprehensive usage example
