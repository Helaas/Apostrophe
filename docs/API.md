# Apostrophe API Reference

Complete reference for all public functions, types, and macros in `apostrophe.h` and `apostrophe_widgets.h`.

---

## Table of Contents

- [Core (apostrophe.h)](#core)
  - [Macros & Constants](#macros--constants)
  - [Types & Enums](#types--enums)
  - [Lifecycle](#lifecycle)
  - [Screen & Scaling](#screen--scaling)
  - [Theming](#theming)
  - [Fonts](#fonts)
  - [Input](#input)
  - [Drawing Primitives](#drawing-primitives)
  - [Footer & Status Bar](#footer--status-bar)
  - [Text Scrolling](#text-scrolling)
  - [Texture Cache](#texture-cache)
  - [Combos](#combos)
  - [Logging](#logging)
- [Widgets (apostrophe_widgets.h)](#widgets)
  - [List](#list)
  - [Options List](#options-list)
  - [Keyboard](#keyboard)
  - [Confirmation](#confirmation)
  - [Selection](#selection)
  - [Process Message](#process-message)
  - [Download Manager](#download-manager)
  - [Detail Screen](#detail-screen)
  - [Color Picker](#color-picker)
  - [Help Overlay](#help-overlay)

---

## Core

### Macros & Constants

| Macro | Value | Description |
|-------|-------|-------------|
| `AP_OK` | `0` | Success return code |
| `AP_ERROR` | `-1` | Error return code |
| `AP_CANCELLED` | `-2` | User cancelled (pressed back) |
| `AP_REFERENCE_WIDTH` | `1024` | Reference width for scaling |
| `AP_SCALE_DAMPING` | `0.75f` | Damping for screens wider than reference |
| `AP_S(base)` | — | Scale a pixel value from reference to actual screen |
| `AP_PLATFORM_NAME` | `"tg5040"` etc. | Compile-time platform identifier |
| `AP_PLATFORM_IS_DEVICE` | `0` or `1` | Whether building for a real device |
| `AP_INPUT_REPEAT_DELAY` | `150` | Initial hold delay (ms) |
| `AP_INPUT_REPEAT_RATE` | `50` | Repeat rate (ms) |
| `AP_AXIS_DEADZONE` | `16000` | Joystick axis dead zone |
| `AP_TEXTURE_CACHE_SIZE` | `8` | LRU texture cache capacity |
| `AP_MAX_COMBOS` | `16` | Max registered button combos |
| `AP_MAX_LOG_LEN` | `2048` | Max log message length |

### Types & Enums

#### `ap_button`

Virtual button identifiers, unified from all input sources.

```c
AP_BTN_NONE, AP_BTN_UP, AP_BTN_DOWN, AP_BTN_LEFT, AP_BTN_RIGHT,
AP_BTN_A, AP_BTN_B, AP_BTN_X, AP_BTN_Y,
AP_BTN_L1, AP_BTN_L2, AP_BTN_R1, AP_BTN_R2,
AP_BTN_START, AP_BTN_SELECT, AP_BTN_MENU, AP_BTN_POWER
```

#### `ap_font_tier`

```c
AP_FONT_EXTRA_LARGE  // 60px at 1024 ref
AP_FONT_LARGE        // 50px
AP_FONT_MEDIUM       // 44px
AP_FONT_SMALL        // 34px
AP_FONT_TINY         // 24px
AP_FONT_MICRO        // 18px
```

#### `ap_text_align`

```c
AP_ALIGN_LEFT, AP_ALIGN_CENTER, AP_ALIGN_RIGHT
```

#### `ap_list_action`

```c
AP_ACTION_SELECTED  // User pressed confirm
AP_ACTION_BACK      // User pressed back
AP_ACTION_TRIGGERED
AP_ACTION_SECONDARY_TRIGGERED
AP_ACTION_CONFIRMED
AP_ACTION_TERTIARY_TRIGGERED
AP_ACTION_CUSTOM    // Alias of AP_ACTION_TRIGGERED (backward compatibility)
```

#### `ap_color`

Alias for `SDL_Color` (r, g, b, a).

#### `ap_theme`

```c
typedef struct {
    ap_color highlight;        // Selected item pill background
    ap_color accent;           // Footer outer pill, status bar bg
    ap_color button_label;     // Text inside footer button pills
    ap_color text;             // Default text color
    ap_color highlighted_text; // Text on selected items
    ap_color hint;             // Dim/help text
    ap_color background;       // Screen background
    char     font_path[512];
    char     bg_image_path[512];
} ap_theme;
```

#### `ap_config`

```c
typedef struct {
    const char *window_title;      // Window title (macOS only)
    const char *font_path;         // Path to .ttf, NULL = auto
    const char *bg_image_path;     // Background image, NULL = none
    const char *log_path;          // Log file, NULL = stderr only
    const char *primary_color_hex; // Override accent "#RRGGBB"
    bool        disable_background; // Set true to skip bg.png
    bool        is_nextui;         // Load theme from nextval.elf
} ap_config;
```

#### `ap_input_event`

```c
typedef struct {
    ap_button button;
    bool      pressed;   // true = down, false = up
} ap_input_event;
```

#### `ap_footer_item`

```c
typedef struct {
    ap_button    button;
    const char  *label;
    bool         is_confirm;  // true = right-aligned group
} ap_footer_item;
```

#### `ap_status_bar_opts`

```c
// Clock display mode constants
#define AP_CLOCK_AUTO  0  // Follow NextUI showclock setting (default)
#define AP_CLOCK_SHOW  1  // Always show, regardless of device settings
#define AP_CLOCK_HIDE  2  // Always hide, regardless of device settings

typedef struct {
    int  show_clock;     // AP_CLOCK_AUTO (default), AP_CLOCK_SHOW, or AP_CLOCK_HIDE
    bool use_24h;        // Only used when show_clock == AP_CLOCK_SHOW
    bool show_battery;   // Show battery icon (device only)
    bool show_wifi;      // Show wifi icon (device only)
} ap_status_bar_opts;
```

**Clock behaviour:** By default (`show_clock` left at 0 / `AP_CLOCK_AUTO`), the clock
visibility and format are read from the NextUI device settings (`showclock` and `clock24h`
in `minuisettings.txt`). On desktop builds, auto mode means the clock is never shown (no
settings file). Use `AP_CLOCK_SHOW` to force the clock visible regardless of device
settings, or `AP_CLOCK_HIDE` to always suppress it. When using `AP_CLOCK_SHOW`, the
`use_24h` field controls the time format; in auto mode, `clock24h` from device settings
is used instead.

On device builds, battery and wifi icons are rendered from the NextUI asset spritesheet (`/mnt/SDCARD/.system/res/assets@Nx.png`). On macOS dev builds, these fields are silently ignored when the spritesheet is not available.

### Lifecycle

#### `int ap_init(ap_config *cfg)`

Initialise Apostrophe. Creates SDL window/renderer, loads fonts, detects screen size, loads theme (if `is_nextui`), sets up input, starts power button handler (on device).

Returns `AP_OK` on success, `AP_ERROR` on failure.

#### `void ap_quit(void)`

Shut down completely. Frees all resources, destroys SDL context, stops background threads.

### Screen & Scaling

#### `int ap_get_screen_width(void)` / `int ap_get_screen_height(void)`

Get the current screen dimensions in pixels.

#### `float ap_get_scale_factor(void)`

Get the current scaling factor (screen_width / reference_width, with damping).

#### `AP_S(base)`

Macro. Scales an integer pixel value from reference (1024px) to actual screen:

```c
int margin = AP_S(20);  // 20px * scale factor
```

### Theming

#### `ap_theme *ap_get_theme(void)`

Get a pointer to the current theme. Modifiable.

#### `void ap_set_theme_color(const char *hex)`

Parse a `#RRGGBB` string into a color: `ap_theme_set_color(&theme->accent, "#FF6600");`

### Fonts

#### `TTF_Font *ap_get_font(ap_font_tier tier)`

Get a pre-loaded, pre-scaled font for the given tier. Returns NULL if not loaded.

### Input

#### `bool ap_poll_input(ap_input_event *event)`

Poll for the next input event. Returns `true` if an event was available.

Handles SDL event processing internally: keyboard events, raw joystick buttons/axes/hats (TrimUI), SDL GameController buttons/axes (macOS + recognised gamepads), platform-specific scancodes (my355), and quit events.

#### `void ap_set_input_delay(uint32_t ms)`

Set input debounce delay in milliseconds.

#### `void ap_set_input_repeat(uint32_t delay_ms, uint32_t rate_ms)`

Configure directional hold repeat timing for D-pad/arrow/button-mapped directions.

### Drawing Primitives

#### `void ap_clear_screen(void)`

Clear the screen to the theme background color (or render bg image if configured).

#### `void ap_present(void)`

Present the rendered frame. Call after all drawing for the frame.

#### `void ap_draw_background(void)`

Draw the background image/color (called automatically by `ap_clear_screen`).

#### `int ap_draw_text(TTF_Font *font, const char *text, int x, int y, ap_color color)`

Render text. Returns the rendered width in pixels.

#### `int ap_draw_text_clipped(TTF_Font *font, const char *text, int x, int y, ap_color color, int max_w)`

Render text clipped to a maximum width.

#### `void ap_draw_text_wrapped(TTF_Font *font, const char *text, int x, int y, int max_w, ap_color color, ap_text_align align)`

Render multi-line word-wrapped text.

#### `int ap_measure_text(TTF_Font *font, const char *text)`

Measure text width without rendering.

#### `void ap_draw_rect(int x, int y, int w, int h, ap_color color)`

Draw a filled rectangle.

#### `void ap_draw_rounded_rect(int x, int y, int w, int h, int radius, ap_color color)`

Draw a filled rounded rectangle using scanline quarter-circle fill (no SDL2_gfx dependency).

#### `void ap_draw_image(SDL_Texture *tex, int x, int y, int w, int h)`

Draw a loaded SDL texture at the given position/size.

### Footer & Status Bar

#### `void ap_draw_footer(ap_footer_item *items, int count)`

Draw the footer bar at the bottom of the screen with button hints.

Non-confirm items render in one continuous outer pill on the left; confirm items render in one continuous outer pill on the right. Inside each outer pill, every item shows an inner button pill (letter/symbol) followed by a text label. Sizing matches Gabagool: base 60px outer pill height, SmallFont (34px), with circular inner pills for single-character buttons.

#### `int ap_get_footer_height(void)`

Get the footer height in pixels (scaled).

#### `void ap_draw_status_bar(ap_status_bar_opts *opts)`

Draw a status bar pill at the top-right of the screen. Shows clock, battery, and wifi status. On device, battery and wifi icons come from the NextUI asset spritesheet. Position matches NextUI's `PADDING` offset (10px unscaled).

#### `int ap_get_status_bar_width(ap_status_bar_opts *opts)`

Calculate the pixel width of the status bar pill, including padding. Use this to clip long title text to avoid overlap.

### Text Scrolling

#### `void ap_text_scroll_reset(ap_text_scroll *scroll)`

Reset a text scroll state to the beginning.

#### `void ap_text_scroll_update(ap_text_scroll *scroll, int text_w, int visible_w)`

Advance the ping-pong scroll animation. Call once per frame.

### Texture Cache

#### `SDL_Texture *ap_cache_get(const char *key, int *w, int *h)`

Look up a texture in the LRU cache by key string. Returns NULL on miss.

#### `void ap_cache_put(const char *key, SDL_Texture *tex, int w, int h)`

Store a texture in the LRU cache. Evicts least-recently-used entries when full.

#### `void ap_cache_clear(void)`

Flush the entire texture cache and free all textures.

### Combos

#### `int ap_register_chord(const char *id, ap_button *buttons, int count, uint32_t window_ms)`

Register a simultaneous button chord combo.
Returns `AP_ERROR` when `id` is NULL/empty, `buttons` is NULL, or `count` is outside `1..8`.

#### `int ap_register_sequence(const char *id, ap_button *buttons, int count, uint32_t timeout_ms, bool strict)`

Register an ordered button sequence combo.
Returns `AP_ERROR` when `id` is NULL/empty, `buttons` is NULL, or `count` is outside `1..8`.

#### `bool ap_poll_combo(ap_combo_event *event)`

Poll the combo event queue.

### Logging

#### `void ap_log(const char *fmt, ...)`

Printf-style logging. Writes to stderr and optionally to the configured log file.

#### `void ap_set_log_path(const char *path)`

Set the active log file path. Passing `NULL` disables file logging and keeps stderr logging only.

#### `const char *ap_resolve_log_path(const char *app_name)`

Resolve a standard NextUI-style log path for an app binary name:
- `LOGS_PATH/<app_name>.txt`
- `SHARED_USERDATA_PATH/logs/<app_name>.txt`
- `HOME/.userdata/logs/<app_name>.txt`

Returns `NULL` if no suitable base path is available.

---

## Widgets

All widget functions return `AP_OK` on successful interaction, `AP_CANCELLED` when the user presses back, or `AP_ERROR` on failure.

### List

```c
ap_list_opts ap_list_default_opts(const char *title, ap_list_item *items, int count);
int          ap_list(ap_list_opts *opts, ap_list_result *result);
```

Scrollable list with:
- Single selection (A button)
- Multi-select mode (checkboxes)
- Reorder mode (toggle reorder button + D-Pad)
- Image thumbnails
- Text overflow scrolling
- Help overlay (L1)
- Explicit action bindings (`action_button`, `secondary_action_button`, `confirm_button`, `tertiary_action_button`)

Footer hints are visual only. Behavior is driven by the action button fields in `ap_list_opts`.

**`ap_list_item`**:
```c
typedef struct {
    const char  *label;
    const char  *metadata;
    SDL_Texture *image;
    bool         selected;  // For multi-select
} ap_list_item;
```

**`ap_list_opts`** (action-related fields):
```c
typedef struct {
    ...
    ap_button reorder_button;
    ap_button action_button;
    ap_button secondary_action_button;
    ap_button confirm_button;
    ap_button tertiary_action_button;
    int       initial_index;
    int       visible_start_index;
} ap_list_opts;
```

**`ap_list_result`**:
```c
typedef struct {
    int             selected_index;
    ap_list_action  action;
    ap_list_item   *items;
    int             item_count;
    int             visible_start_index;
} ap_list_result;
```

### Options List

```c
int ap_options_list(ap_options_list_opts *opts, ap_options_list_result *result);
```

Settings-style list where each row has a label and a configurable value area:

| Type | Behavior |
|------|----------|
| `AP_OPT_STANDARD` | Left/Right cycles through predefined values |
| `AP_OPT_KEYBOARD` | A opens keyboard for text input |
| `AP_OPT_CLICKABLE` | A triggers a navigation/action callback |
| `AP_OPT_COLOR_PICKER` | A opens the color picker |

Action buttons are explicit in `ap_options_list_opts` (`action_button`, `secondary_action_button`, `confirm_button`), and footer hints remain visual-only.
When option storage is malformed (`options == NULL` or out-of-range `selected_option`), Apostrophe safely clamps/ignores the invalid value instead of dereferencing invalid memory.

**`ap_options_list_opts`** (action/scroll fields):
```c
typedef struct {
    ...
    int       initial_selected_index;
    int       visible_start_index;
    ap_button action_button;
    ap_button secondary_action_button;
    ap_button confirm_button;
} ap_options_list_opts;
```

**`ap_options_list_result`**:
```c
typedef struct {
    int            focused_index;
    ap_list_action action;
    ...
    int            visible_start_index;
} ap_options_list_result;
```

### Keyboard

```c
int ap_keyboard(const char *initial_text, const char *help_text,
                ap_keyboard_layout layout, ap_keyboard_result *result);

int ap_url_keyboard(const char *initial_text, const char *help_text,
                    ap_url_keyboard_config *cfg, ap_keyboard_result *result);
```

5-row on-screen keyboard matching Gabagool's layout:
- **Row 0**: Numbers 1-0 + backspace (⌫, 2× width)
- **Row 1**: QWERTY row (10 keys, centered)
- **Row 2**: ASDF row (9 keys) + enter (↵, 1.5× width)
- **Row 3**: Shift (⇧, 2× width) + ZXCV row (7 keys) + symbol toggle (#+=, 2× width)
- **Row 4**: Space bar (8× width, centered)

**Button mapping** (Gabagool-compatible):
- **B**: Backspace
- **X**: Space (general) / Toggle symbol alternates (URL)
- **Y**: Exit without saving
- **Select**: Toggle shift
- **Start**: Confirm
- **L1/R1**: Move text cursor left/right
- **Menu**: Help overlay

**URL Keyboard** adds configurable shortcut rows above the QWERTY keys:
- Default shortcuts: `https://`, `www.`, `.com`, `.org`, `.net`, `.io`, `.dev`, `.app`, `.edu`, `.gov`
- X toggles to symbol alternates: `http://`, `ftp://`, `.co`, `.tv`, `.me`, `.gg`, `.uk`, `.de`, `.ca`, `.au`
- URL special chars row: `/ : @ - _ . ~ ? # &`
- No space bar in URL mode

**Layouts**: `AP_KB_GENERAL`, `AP_KB_URL`, `AP_KB_NUMERIC`

**Result**: `ap_keyboard_result.text` (char[1024])

### Confirmation

```c
int ap_confirmation(ap_message_opts *opts, ap_confirm_result *result);
```

Modal dialog with a message (optionally with an image above it). Waits for user to press A (confirm) or B (cancel).

**Result**: `ap_confirm_result.confirmed` (bool)

### Selection

```c
int ap_selection(const char *message, ap_selection_option *options, int count,
                 ap_footer_item *footer, int footer_count,
                 ap_selection_result *result);
```

Horizontal pill-style chooser. User presses Left/Right to cycle options, A to confirm.

**Result**: `ap_selection_result.selected_index`

### Process Message

```c
int ap_process_message(ap_process_opts *opts, ap_process_fn fn, void *userdata);
```

Runs a worker function in a background thread while displaying a message and optional progress bar.

```c
typedef int (*ap_process_fn)(void *userdata);
```

**`ap_process_opts`**:
```c
typedef struct {
    const char     *message;
    bool            show_progress;
    float          *progress;          // Worker updates this [0.0–1.0]
    int            *interrupt_signal;  // UI sets to 1 on cancel
    ap_button       interrupt_button;  // Cancel button (AP_BTN_NONE = none)
    char          **dynamic_message;   // Worker can update displayed text
    int             message_lines;
} ap_process_opts;
```

### Detail Screen

```c
int ap_detail_screen(ap_detail_opts *opts, ap_detail_result *result);
```

Scrollable multi-section view for displaying information. Supports:

| Section Type | Content |
|-------------|---------|
| `AP_SECTION_INFO` | Key-value pairs |
| `AP_SECTION_DESCRIPTION` | Wrapped text block |
| `AP_SECTION_IMAGE` | Single image |
| `AP_SECTION_TABLE` | Tabular data |

`AP_SECTION_IMAGE` textures are loaded once when the detail screen opens and reused for each frame until the screen exits.
### Download Manager

```c
int ap_download_manager(ap_download *downloads, int count,
                        ap_download_opts *opts, ap_download_result *result);
```

Multi-threaded file downloader with per-file progress bars. Requires libcurl (compile with `-DAP_ENABLE_CURL` and link with `-lcurl`).

For Apostrophe device example builds, bundled curl is enabled by default for `EXAMPLE=download` via `USE_BUNDLED_CURL=1`.
This builds dependencies into `build/third_party/<platform>/...`, stages runtime libs in `build/<platform>/download/lib`,
and expects pak launchers to include that directory in `LD_LIBRARY_PATH`.

**Features**:
- Thread pool with configurable concurrency (default 3)
- Per-file progress bars (3/4 screen width, max 900px)
- Live download speed display (toggleable with X)
- Auto-scroll to active downloads
- Cancel all with Y
- Custom HTTP headers and SSL options

**`ap_download`** (per-job):
```c
typedef struct {
    const char          *url;         // Source URL
    const char          *dest_path;   // Destination file path
    const char          *label;       // Display label
    ap_download_status   status;      // AP_DL_PENDING/DOWNLOADING/COMPLETE/FAILED
    float                progress;    // 0.0–1.0
    double               speed_bps;   // Bytes per second
    int                  http_code;   // HTTP response code
    char                 error[256];  // Error message
} ap_download;
```

**`ap_download_opts`**:
```c
typedef struct {
    int   max_concurrent;     // Max simultaneous downloads (default 3)
    bool  skip_ssl_verify;    // Disable SSL cert verification
    const char **headers;     // "Header: Value" strings
    int   header_count;
} ap_download_opts;
```

**`ap_download_result`**:
```c
typedef struct {
    int  total;
    int  completed;
    int  failed;
    bool cancelled;
} ap_download_result;
```
### Color Picker

```c
int ap_color_picker(ap_color initial, ap_color *result);
```

5×5 grid of predefined colors. Navigate with D-Pad, confirm with A.

### Help Overlay

```c
void ap_show_help_overlay(const char *text);
```

Full-screen scrollable text overlay. Typically triggered by L1 in widgets that have `help_text` configured.
