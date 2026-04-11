# Apostrophe API Reference

Complete reference for all public functions, types, and macros in `apostrophe.h` and `apostrophe_widgets.h`.

This reference documents Apostrophe **v1.1.0** (2026-03-30).

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
  - [Screen Fade](#screen-fade)
  - [Footer & Status Bar](#footer--status-bar)
  - [Text Scrolling](#text-scrolling)
  - [Texture Cache](#texture-cache)
  - [Combos](#combos)
  - [Logging](#logging)
  - [Accessors](#accessors)
  - [Power](#power)
  - [CPU & Fan](#cpu--fan)
  - [Error Handling](#error-handling)
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
  - [File Picker](#file-picker)

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
| `AP_DS(base)` | — | Scale a pixel value by integer `device_scale` (2 or 3) |
| `AP_S(base)` | — | Scale a pixel value from reference to actual screen |
| `AP_PLATFORM_NAME` | `"tg5040"` etc. | Compile-time platform identifier |
| `AP_PLATFORM_IS_DEVICE` | `0` or `1` | Whether building for a real device |
| `AP_INPUT_DEBOUNCE` | `20` | Input debounce delay (ms) |
| `AP_INPUT_REPEAT_DELAY` | `300` | Initial hold delay (ms) |
| `AP_INPUT_REPEAT_RATE` | `100` | Repeat rate (ms) |
| `AP_AXIS_DEADZONE` | `16000` / `20000` on `my355` | Joystick axis dead zone |
| `AP_TEXT_SCROLL_SPEED` | `1` | Text scroll speed (pixels per tick) |
| `AP_TEXT_SCROLL_PAUSE_MS` | `1000` | Pause at scroll endpoints (ms) |
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
AP_FONT_EXTRA_LARGE  // 24 × device_scale (title/header)
AP_FONT_LARGE        // 16 × device_scale (list items — NextUI FONT_LARGE)
AP_FONT_MEDIUM       // 14 × device_scale (single-char button label — NextUI FONT_MEDIUM)
AP_FONT_SMALL        // 12 × device_scale (hint text — NextUI FONT_SMALL)
AP_FONT_TINY         // 10 × device_scale (multi-char button label — NextUI FONT_TINY)
AP_FONT_MICRO        //  7 × device_scale (overlay text — NextUI FONT_MICRO)
```

Font sizes use integer `device_scale` (2 for MY355/TG5050/TG5040 handheld, 3 for TG5040 brick), matching NextUI's `SCALE1(FONT_*)` exactly. On screens with more logical pixels than the 320×240 baseline (MY355), an automatic font bump of 0–5 is added to each base size before scaling. See `ap_get_font_bump()`.

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
    ap_cpu_speed cpu_speed;        // Set CPU at init; 0 = AP_CPU_SPEED_DEFAULT (no-op)
    bool        disable_font_bump; // Set true to disable automatic font bumping
} ap_config;
```

#### `ap_input_event`

```c
typedef struct {
    ap_button button;
    bool      pressed;   // true = down, false = up
    bool      repeated;  // true = generated by hold-repeat, false = fresh press
} ap_input_event;
```

#### `ap_footer_item`

```c
typedef struct {
    ap_button    button;
    const char  *label;
    bool         is_confirm;  // true = right-aligned group
    const char  *button_text; // optional display-only override for the button pill text
} ap_footer_item;
```

`button_text` only affects what is drawn in the footer. It does not change input handling, which still comes from your widget logic and button bindings.

#### `ap_footer_overflow_opts`

```c
typedef struct {
    bool      enabled;  // true = collapse overflowing footer hints into +N
    ap_button chord_a;  // First button in the hidden-actions chord
    ap_button chord_b;  // Second button in the hidden-actions chord
} ap_footer_overflow_opts;
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
    bool show_battery;   // Show battery icon from device or desktop preview state
    bool show_wifi;      // Show wifi icon when connected / previewed
} ap_status_bar_opts;
```

**Clock behaviour:** By default (`show_clock` left at 0 / `AP_CLOCK_AUTO`), the clock
visibility and format are read from the NextUI device settings (`showclock` and `clock24h`
in `minuisettings.txt`). On desktop builds, auto mode follows the preview settings file
from `AP_MINUI_SETTINGS_PATH` when provided; otherwise auto mode hides the clock. Use
`AP_CLOCK_SHOW` to force the clock visible regardless of settings, or `AP_CLOCK_HIDE` to
always suppress it. When using `AP_CLOCK_SHOW`, the `use_24h` field controls the time
format; in auto mode, `clock24h` from the settings file is used instead.

**Wifi behaviour:** When `show_wifi` is true, the wifi icon is only shown when the current
device or desktop preview signal strength is greater than 0. When disconnected, the icon is
hidden and the pill shrinks accordingly. This matches NextUI's behaviour of hiding the wifi
icon when not connected.

Battery and wifi can be enabled independently. When status sprites are active, the clock is
hidden, and exactly one of those icons is visible, Apostrophe renders a centered square pill
for that single icon so width calculation and drawing stay aligned.

On device builds, battery and wifi icons are rendered from the NextUI asset spritesheet
(`$SDCARD_PATH/.system/res/assets@Nx.png`, defaulting to `/mnt/SDCARD/.system/res/assets@Nx.png`).
On desktop builds, the same sprite path is used when `AP_STATUS_ASSETS_DIR` points at a folder
containing `assets@1x.png` through `assets@4x.png`. Pills (status bar, footer) then use the
same pre-rendered sprites for smooth anti-aliased edges on both device and desktop. Without
status assets, desktop falls back to procedural pill drawing and text placeholders. Apostrophe
does not bundle the upstream NextUI sprite sheets; these files must come from your own local
preview cache or device/source checkout.

Desktop preview environment variables:

- `AP_STATUS_ASSETS_DIR`: directory containing `assets@Nx.png`
- `AP_NEXTVAL_PATH`: JSON file used for NextUI theme colors on desktop
- `AP_MINUI_SETTINGS_PATH`: mock `minuisettings.txt` used by `AP_CLOCK_AUTO` and `batteryperc`
- `AP_PREVIEW_WIFI_STRENGTH`: preview wifi strength (`0`..`3`)
- `AP_PREVIEW_BATTERY_PERCENT`: preview battery level (`0`..`100`)
- `AP_PREVIEW_CHARGING`: preview charging state (`0` or `1`)

### Lifecycle

#### `int ap_init(ap_config *cfg)`

Initialise Apostrophe. Creates SDL window/renderer, loads fonts, detects screen size, loads theme (if `is_nextui`), sets up input, starts power button handler (on device).

On my355 device builds, raw power handling listens for `KEY_POWER` from Linux input devices. A short press triggers suspend, and a long press (>= 1000ms) triggers shutdown orchestration (`/tmp/poweroff`). Suspend first attempts `echo mem > /sys/power/state` and falls back to `echo freeze > /sys/power/state` if needed. After resume, power-key events are ignored for 1000ms to avoid immediate re-suspend from wake events (matching NextUI behavior).

Returns `AP_OK` on success, `AP_ERROR` on failure.

#### `void ap_quit(void)`

Shut down completely. Frees all resources, destroys SDL context, stops background threads.

### Screen & Scaling

#### `int ap_get_screen_width(void)` / `int ap_get_screen_height(void)`

Get the current screen dimensions in pixels.

#### `float ap_get_scale_factor(void)`

Get the current scaling factor (screen_width / reference_width, with damping).

#### `int ap_scale(int base)`

Scale a pixel value from 1024-reference space to actual screen space. Equivalent to the `AP_S()` macro as a function call.

#### `int ap_font_size_for_resolution(int base_size)`

Scale a base font size by `device_scale` (integer multiplier: 2 for MY355/TG5050/TG5040 handheld, 3 for TG5040 brick). Returns `base_size * device_scale`.

#### `AP_S(base)`

Macro. Scales an integer pixel value from reference (1024px) to actual screen:

```c
int margin = AP_S(20);  // 20px * scale factor
```

### Theming

#### `ap_theme *ap_get_theme(void)`

Get a pointer to the current theme. Modifiable.

#### `int ap_theme_load_nextui(void)`

Load theme colors from the NextUI configuration, including the fallback background color. Accepts both the current `color7` background key and the legacy `bgcolor` key for backward compatibility. Returns `AP_OK` on success, `AP_ERROR` on failure. Called automatically during `ap_init()` when `ap_config.is_nextui` is true.

#### `ap_color ap_hex_to_color(const char *hex)`

Parse a `#RRGGBB` hex string and return the corresponding `ap_color` (with alpha 255). Returns black `{0,0,0,255}` on invalid input.

#### `void ap_set_theme_color(const char *hex)`

Parse a `#RRGGBB` string and apply it as the theme accent color: `ap_set_theme_color("#FF6600");`

#### `int ap_reload_background(const char *bg_path)`

Reload the background image at runtime. Destroys the current background texture and loads a new one from `bg_path`. If `bg_path` is NULL or empty, falls back to `/mnt/SDCARD/bg.png` on device or the `AP_BACKGROUND_PATH` environment variable on desktop. Returns `AP_OK` on success (or if no fallback path is available), `AP_ERROR` if the image cannot be loaded.

### Fonts

#### `TTF_Font *ap_get_font(ap_font_tier tier)`

Get a pre-loaded, pre-scaled font for the given tier. Returns NULL if not loaded.

#### `int ap_get_font_bump(void)`

Get the automatic font bump computed at init (0–5). The bump is added to each base font size before `device_scale` multiplication. Computed from the logical resolution (`screen / device_scale`) relative to the 320×240 baseline (MY355). Returns 0 on MY355 and TG5040 brick, typically 2 on TG5050. Set `ap_config.disable_font_bump = true` to force 0.

### Input

#### `bool ap_poll_input(ap_input_event *event)`

Poll for the next input event. Returns `true` if an event was available.

Handles SDL event processing internally: keyboard events, raw joystick buttons/axes/hats (TrimUI), SDL GameController buttons/axes (macOS + recognised gamepads), platform-specific scancodes (my355), and quit events.

#### `void ap_flip_face_buttons(bool flip)`

Swap A/B and X/Y button mappings. When `flip` is true, hardware A reports as `AP_BTN_B` and vice versa (likewise X/Y). Useful for platforms where the firmware already swaps face buttons.

#### `const char *ap_button_name(ap_button btn)`

Get the display name string for a virtual button (e.g. `"A"`, `"UP"`, `"START"`). Returns `"Unknown"` for out-of-range values.

#### `void ap_set_input_delay(uint32_t ms)`

Set input debounce delay in milliseconds.

#### `void ap_set_input_repeat(uint32_t delay_ms, uint32_t rate_ms)`

Configure directional hold repeat timing for D-pad/arrow/button-mapped directions.

### Drawing Primitives

#### `void ap_clear_screen(void)`

Clear the screen to the theme background color (or render bg image if configured).

#### `void ap_present(void)`

Present the rendered frame. Call after all drawing for the frame. When the renderer does not have vsync (e.g. software fallback), this function automatically throttles to ~60 fps via `SDL_Delay` to prevent CPU hot-spinning.

#### `void ap_request_frame(void)`

Request another frame immediately. Use this for active animations or callbacks that need the next frame to render without waiting for input.

#### `void ap_request_frame_in(uint32_t ms)`

Schedule a redraw in the future while the UI is otherwise idle. This is useful for widgets that need periodic refresh without adding their own polling loop. For example, an `ap_list()` footer callback can call `ap_request_frame_in(100)` to keep a live footer label in sync with external state.

#### `void ap_draw_background(void)`

Draw the background image/color (called automatically by `ap_clear_screen`).

#### `int ap_draw_text(TTF_Font *font, const char *text, int x, int y, ap_color color)`

Render text. Returns the rendered width in pixels.

#### `int ap_draw_text_clipped(TTF_Font *font, const char *text, int x, int y, ap_color color, int max_w)`

Render text clipped to a maximum width. Performs a hard pixel clip with no truncation indicator.

#### `int ap_draw_text_ellipsized(TTF_Font *font, const char *text, int x, int y, ap_color color, int max_w)`

Render text truncated with "..." if it exceeds `max_w`. If the text fits, it is rendered normally. Uses a binary search to find the longest prefix that fits alongside the ellipsis, respecting UTF-8 character boundaries. Returns the rendered width in pixels.

#### `void ap_draw_text_wrapped(TTF_Font *font, const char *text, int x, int y, int max_w, ap_color color, ap_text_align align)`

Render multi-line word-wrapped text.

#### `int ap_measure_text(TTF_Font *font, const char *text)`

Measure text width without rendering.

#### `int ap_measure_text_ellipsized(TTF_Font *font, const char *text, int max_w)`

Measure the width the text would occupy if ellipsized to fit `max_w`, without rendering. Returns the full text width when the text already fits. Returns `0` when `max_w <= 0`. If the ellipsis itself is as wide as or wider than `max_w`, returns `max_w` to match the clipped fallback behavior used by `ap_draw_text_ellipsized()`.

#### `int ap_measure_wrapped_text_height(TTF_Font *font, const char *text, int max_w)`

Measure the total height in pixels that word-wrapped text would occupy at the given `max_w` constraint, without rendering. Useful for pre-calculating layout sizes.

#### `void ap_draw_rect(int x, int y, int w, int h, ap_color color)`

Draw a filled rectangle.

#### `void ap_draw_pill(int x, int y, int w, int h, ap_color color)`

Draw a pill shape (fully rounded rectangle where corner radius = h/2). Uses the pre-rendered NextUI pill sprite when status assets are loaded; otherwise falls back to procedural drawing.

#### `void ap_draw_rounded_rect(int x, int y, int w, int h, int radius, ap_color color)`

Draw a filled rounded rectangle with arbitrary corner radius using scanline quarter-circle fill with sub-pixel anti-aliasing (no SDL2_gfx dependency).

#### `void ap_draw_circle(int cx, int cy, int r, ap_color c)`

Draw a filled circle at center (cx, cy) with radius r.

#### `void ap_draw_image(SDL_Texture *tex, int x, int y, int w, int h)`

Draw a loaded SDL texture at the given position/size.

#### `SDL_Texture *ap_load_image(const char *path)`

Load an image from disk (PNG, JPG) and return an SDL_Texture. Returns NULL on failure.

#### `void ap_draw_scrollbar(int x, int y, int h, int visible, int total, int offset)`

Draw a vertical scrollbar track and thumb. The thumb size and position are computed from visible/total/offset. Does nothing if total <= visible.

#### `void ap_draw_progress_bar(int x, int y, int w, int h, float progress, ap_color fg, ap_color bg)`

Draw a rounded progress bar. `progress` is clamped to 0.0–1.0.

#### `SDL_Rect ap_get_content_rect(bool has_title, bool has_footer, bool has_status_bar)`

Calculate the usable content area of the screen, accounting for title bar, footer, and status bar. Returns an `SDL_Rect` with `x`, `y`, `w`, `h` in pixels. Use this to position widget content within the available space.

#### `void ap_draw_screen_title(const char *title, ap_status_bar_opts *status_bar)`

Draw a title at the top-left of the screen. If `status_bar` is non-NULL, the status bar is also drawn at the top-right and the title is clipped to avoid overlapping it.

#### `void ap_draw_screen_title_centered(const char *title, ap_status_bar_opts *status_bar)`

Draw a title centered horizontally in the available space. Behaves like `ap_draw_screen_title` but centers the text instead of left-aligning it. Uses the same progressive font-tier fallback (extra-large → large → medium) and clips to avoid overlapping the status bar.

### Screen Fade

Fade-in/fade-out overlay for scene transitions.

#### `ap_fade`

```c
typedef struct {
    uint32_t start_ms;     // SDL_GetTicks() value when fade began
    int      duration_ms;  // Total duration of the fade
    bool     fade_in;      // true = black->transparent, false = transparent->black
    bool     active;       // false = not animating
} ap_fade;
```

#### `void ap_fade_begin_in(ap_fade *f, int duration_ms)`

Start a fade-in: the screen transitions from fully black to transparent over `duration_ms` milliseconds.

#### `void ap_fade_begin_out(ap_fade *f, int duration_ms)`

Start a fade-out: the screen transitions from transparent to fully black over `duration_ms` milliseconds.

#### `bool ap_fade_draw(ap_fade *f)`

Draw the fade overlay. Call after drawing your scene and before `ap_present()`. Returns `true` while the fade is still animating, `false` when complete.

### Footer & Status Bar

#### `void ap_draw_footer(ap_footer_item *items, int count)`

Draw the footer bar at the bottom of the screen with button hints.

Non-confirm items render in one continuous outer pill on the left; confirm items render in one continuous outer pill on the right. Inside each outer pill, every item shows an inner button pill (letter/symbol) followed by a text label. Sizing matches NextUI: `PILL_SIZE` (30) outer height, `BUTTON_SIZE` (20) inner circles, all scaled by `device_scale`. Font tiers: `AP_FONT_MEDIUM` (14 base) for single-char button labels, `AP_FONT_TINY` (10 base) for multi-char labels, `AP_FONT_SMALL` (12 base) for hint text.

When footer overflow handling is enabled and the hints do not fit on one row, Apostrophe keeps the footer on a single line, preserves the right-aligned confirm group, shows a compact `+N` marker for hidden hints, and exposes a hidden-actions overlay that can be opened by calling `ap_show_footer_overflow()` (commonly bound to the Menu button) or by configuring a chord. Hidden footer items remain normal actions; only their on-screen hints are collapsed.

#### `int ap_get_footer_height(void)`

Get the footer height in pixels (scaled).

#### `void ap_set_footer_overflow_opts(const ap_footer_overflow_opts *opts)`

Set the global footer overflow behaviour. Pass `NULL` to restore the default configuration (`enabled = true`, `chord_a = AP_BTN_NONE`, `chord_b = AP_BTN_NONE`). To enable a button chord shortcut, set `chord_a` and `chord_b` to the desired buttons (e.g. `AP_BTN_L1` and `AP_BTN_R1`).

#### `void ap_get_footer_overflow_opts(ap_footer_overflow_opts *out)`

Read the current global footer overflow configuration.

#### `void ap_show_footer_overflow(void)`

Programmatically open the hidden-actions overlay when hidden footer items exist. This is useful in screens with custom input loops (i.e. not using `ap_list`) that need to support the Menu button for footer overflow. The call is a no-op when there are no hidden items. Note: hidden item state is computed by `ap_draw_footer()`, so this function must be called after at least one footer draw pass in the current frame.

#### `void ap_draw_status_bar(ap_status_bar_opts *opts)`

Draw a status bar pill at the top-right of the screen. Shows clock, battery, and wifi status. Battery and wifi icons come from the NextUI asset spritesheet whenever status assets are loaded. Position matches NextUI's `PADDING` offset (10px unscaled). When sprites are active and only one battery/wifi icon is visible with the clock hidden, the pill collapses to a centered square icon pill.

#### `int ap_get_status_bar_width(ap_status_bar_opts *opts)`

Calculate the pixel width of the status bar pill, including padding. Use this to clip long title text to avoid overlap. The result matches `ap_draw_status_bar()` for all clock/battery/wifi combinations, including square single-icon sprite pills.

#### `int ap_get_status_bar_height(void)`

Get the height of the status bar in pixels (scaled).

### Text Scrolling

#### `void ap_text_scroll_init(ap_text_scroll *s)`

Initialise a text scroll state. Resets offset, direction, and pause timer.

#### `void ap_text_scroll_reset(ap_text_scroll *scroll)`

Reset a text scroll state to the beginning.

#### `void ap_text_scroll_update(ap_text_scroll *scroll, int text_w, int visible_w, uint32_t dt_ms)`

Advance the ping-pong scroll animation. Call once per frame, passing the frame delta time in milliseconds.

### Texture Cache

#### `SDL_Texture *ap_cache_get(const char *key, int *w, int *h)`

Look up a texture in the LRU cache by key string. Returns NULL on miss.

#### `void ap_cache_put(const char *key, SDL_Texture *tex, int w, int h)`

Store a texture in the LRU cache. Evicts least-recently-used entries when full.

#### `void ap_cache_clear(void)`

Flush the entire texture cache and free all textures.

### Combos

The combo system detects two kinds of multi-button input:

- **Chords** — multiple buttons pressed simultaneously (e.g. L1+R1 together)
- **Sequences** — buttons pressed in a specific order (e.g. Up, Up, Down, Down)

Combos do **not** suppress individual button events — `ap_poll_input()` still returns every press and release as normal.

#### Types

##### `ap_combo_type`

```c
typedef enum { AP_COMBO_CHORD, AP_COMBO_SEQUENCE } ap_combo_type;
```

Distinguishes the kind of combo in a `ap_combo_event`. Set to `AP_COMBO_CHORD` for chords and `AP_COMBO_SEQUENCE` for sequences.

##### `ap_combo_callback`

```c
typedef void (*ap_combo_callback)(const char *id, ap_combo_type type, void *userdata);
```

Callback signature used by the `_ex` registration variants. Called synchronously at the moment the combo triggers or a chord releases.

##### `ap_combo_event`

```c
typedef struct {
    const char    *id;        /* identifier passed to ap_register_chord/sequence */
    bool           triggered; /* true = fired, false = chord released */
    ap_combo_type  type;      /* AP_COMBO_CHORD or AP_COMBO_SEQUENCE */
} ap_combo_event;
```

#### `int ap_register_chord(const char *id, ap_button *buttons, int count, uint32_t window_ms)`

Register a simultaneous button chord. All `count` buttons must be held at the same time, with
the time between the earliest and latest press no greater than `window_ms` (default 100ms if 0).
Returns `AP_ERROR` when `id` is NULL/empty, `buttons` is NULL, or `count` is outside `1..8`.

When the chord triggers, a combo event is queued with `triggered = true`. When any button in the
chord is released, a second event is queued with `triggered = false`. A chord will not re-trigger
until all buttons have been released and pressed again.

#### `int ap_register_chord_ex(const char *id, ap_button *buttons, int count, uint32_t window_ms, ap_combo_callback on_trigger, ap_combo_callback on_release, void *userdata)`

Like `ap_register_chord`, but also registers optional callbacks. `on_trigger` fires when the chord fires; `on_release` fires when any button is released. Either may be `NULL`. Callbacks fire synchronously before the event is enqueued, so polling still works alongside them — both are additive.

#### `int ap_register_sequence(const char *id, ap_button *buttons, int count, uint32_t timeout_ms, bool strict)`

Register an ordered button sequence. Each button must be pressed within `timeout_ms` of the
previous one (default 500ms if 0). When `strict` is true, any extraneous button press during
the sequence window causes the match to fail.
Returns `AP_ERROR` when `id` is NULL/empty, `buttons` is NULL, or `count` is outside `1..8`.

Sequences only fire `triggered = true` events (no release event).

#### `int ap_register_sequence_ex(const char *id, ap_button *buttons, int count, uint32_t timeout_ms, bool strict, ap_combo_callback on_trigger, void *userdata)`

Like `ap_register_sequence`, but also registers an optional `on_trigger` callback. There is no `on_release` parameter for sequences — they are one-shot events with no release phase.

#### `void ap_unregister_combo(const char *id)`

Deactivate a previously registered combo by its `id`. The combo slot is marked inactive but not removed.

#### `void ap_clear_combos(void)`

Remove all registered combos and reset internal detection state.

#### `bool ap_poll_combo(ap_combo_event *event)`

Poll the combo event queue. Returns `true` if a combo event is available, filling `event` with:
- `id` — the string identifier passed to `ap_register_chord()` or `ap_register_sequence()`
- `triggered` — `true` when the combo fires, `false` when a chord is released
- `type` — `AP_COMBO_CHORD` or `AP_COMBO_SEQUENCE`

#### Limits

| Limit | Value |
|-------|-------|
| Max registered combos | 16 (`AP_MAX_COMBOS`) |
| Max buttons per combo | 8 |
| Combo event queue size | 16 |
| Sequence detection buffer | 20 recent presses |

#### Example: Polling

```c
/* Register combos */
ap_button shoulders[] = { AP_BTN_L1, AP_BTN_R1 };
ap_register_chord("shoulders", shoulders, 2, 150);

ap_button uudd[] = { AP_BTN_UP, AP_BTN_UP, AP_BTN_DOWN, AP_BTN_DOWN };
ap_register_sequence("uudd", uudd, 4, 500, false);

/* In your main loop: */
ap_combo_event combo;
while (ap_poll_combo(&combo)) {
    const char *kind = (combo.type == AP_COMBO_CHORD) ? "chord" : "seq";
    if (combo.triggered)
        printf("Triggered [%s]: %s\n", kind, combo.id);
    else
        printf("Released  [%s]: %s\n", kind, combo.id);
}
```

#### Example: Callbacks (_ex variants)

```c
void on_trigger(const char *id, ap_combo_type type, void *userdata) {
    printf("Triggered: %s\n", id);
}
void on_release(const char *id, ap_combo_type type, void *userdata) {
    printf("Released: %s\n", id);
}

/* Chord with trigger + release callbacks */
ap_button shoulders[] = { AP_BTN_L1, AP_BTN_R1 };
ap_register_chord_ex("shoulders", shoulders, 2, 150, on_trigger, on_release, NULL);

/* Sequence with trigger callback only (no release phase) */
ap_button uudd[] = { AP_BTN_UP, AP_BTN_UP, AP_BTN_DOWN, AP_BTN_DOWN };
ap_register_sequence_ex("uudd", uudd, 4, 500, false, on_trigger, NULL);

/* ap_poll_combo() still works — callbacks and polling are both active */
```

See `examples/combo/main.c` for a complete working example with both modes.

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

### Accessors

#### `SDL_Renderer *ap_get_renderer(void)`

Get the underlying SDL renderer.

#### `SDL_Window *ap_get_window(void)`

Get the underlying SDL window.

#### `void ap_show_window(void)`

Show the SDL window. Primarily useful on desktop builds where the window may be hidden at startup.

#### `void ap_hide_window(void)`

Hide the SDL window. No-op if the window has not been created.

### Power

#### `void ap_set_power_handler(bool enabled)`

Enable or disable the background power button handler. On device, this listens for `KEY_POWER` from Linux input devices — a short press triggers suspend, a long press (>= 1s) triggers shutdown. Enabled automatically by `ap_init()` on device builds.

### CPU & Fan

Control CPU frequency and fan speed on NextUI handheld hardware. All functions are no-ops (or
return -1) on desktop builds. Fan functions have no effect on MY355 and TG5040, which have no
fan hardware.

#### `ap_cpu_speed` (enum)

```c
typedef enum {
    AP_CPU_SPEED_DEFAULT     = 0, /* do not change (zero-init default) */
    AP_CPU_SPEED_MENU,            /* light UI work  */
    AP_CPU_SPEED_POWERSAVE,       /* battery saving  */
    AP_CPU_SPEED_NORMAL,          /* standard pak speed — recommended default */
    AP_CPU_SPEED_PERFORMANCE,     /* maximum speed  */
} ap_cpu_speed;
```

Approximate frequencies per preset and platform:

| Preset | MY355 | TG5040 | TG5050 (big core) |
|--------|-------|--------|-------------------|
| `AP_CPU_SPEED_MENU` | 600 MHz | 600 MHz | 672 MHz |
| `AP_CPU_SPEED_POWERSAVE` | 1200 MHz | 1200 MHz | 1200 MHz |
| `AP_CPU_SPEED_NORMAL` | 1608 MHz | 1608 MHz | 1680 MHz |
| `AP_CPU_SPEED_PERFORMANCE` | 2000 MHz | 2000 MHz | 2160 MHz |

`AP_CPU_SPEED_NORMAL` is the recommended default for paks — it matches what NextUI's `launch.sh`
sets before handing control to a pak binary.

**`ap_config.cpu_speed`** — set this field in `ap_config` to apply a speed preset during
`ap_init()`. Zero (default struct value) leaves the CPU unchanged.

```c
ap_config cfg = {
    .window_title = "My Pak",
    .is_nextui    = AP_PLATFORM_IS_DEVICE,
    .cpu_speed    = AP_CPU_SPEED_NORMAL,  /* set at init */
};
ap_init(&cfg);
```

#### `int ap_set_cpu_speed(ap_cpu_speed speed)`

Set the CPU to a named preset. Internally writes `userspace` to the cpufreq governor and then
writes the platform-specific frequency to `scaling_setspeed`. Returns `AP_OK` on success,
`AP_ERROR` if the sysfs write fails. No-op (returns `AP_OK`) on desktop builds.

#### `int ap_get_cpu_speed_mhz(void)`

Read the current CPU frequency in MHz. Returns `-1` on error or desktop builds.

#### `int ap_get_cpu_temp_celsius(void)`

Read the CPU temperature in °C from `/sys/devices/virtual/thermal/thermal_zone0/temp`.
Returns `-1` on error or desktop builds.

`my355` is a special case in NextUI: the shipped `MinUI.pak/launch.sh` boots at `1992 MHz`, but
NextUI's `workspace/my355/platform/platform.c` uses `2000 MHz` for the named performance preset.
Apostrophe follows `platform.c` for the preset API.

#### `ap_fan_mode` (enum)

```c
typedef enum {
    AP_FAN_MODE_UNSUPPORTED = -1,
    AP_FAN_MODE_MANUAL = 0,
    AP_FAN_MODE_AUTO_QUIET,
    AP_FAN_MODE_AUTO_NORMAL,
    AP_FAN_MODE_AUTO_PERFORMANCE,
} ap_fan_mode;
```

`AP_FAN_MODE_AUTO_*` mirrors NextUI's TG5050 `fancontrol` helper modes. On platforms without fan
hardware, `ap_get_fan_mode()` returns `AP_FAN_MODE_UNSUPPORTED`.

#### `int ap_set_fan_mode(ap_fan_mode mode)`

Set the TG5050 fan mode. `AP_FAN_MODE_MANUAL` stops any active `fancontrol` daemon without
changing the current raw fan state. The auto modes launch NextUI's
`/mnt/SDCARD/.system/tg5050/bin/fancontrol` helper with `quiet`, `normal`, or `performance`.
Returns `AP_ERROR` if an auto mode is requested and the helper is unavailable. No-op (returns
`AP_OK`) on non-TG5050 platforms and desktop builds.

#### `ap_fan_mode ap_get_fan_mode(void)`

Read the current TG5050 fan mode. If a `fancontrol` daemon is running, this reports the matching
auto mode. Otherwise, it reports `AP_FAN_MODE_MANUAL` when the fan sysfs node is readable, or
`AP_FAN_MODE_UNSUPPORTED` if the mode cannot be determined.

#### `int ap_set_fan_speed(int percent)`

Set the fan to a fixed 0–100 percentage. On TG5050, Apostrophe first stops any active
`fancontrol` daemon, then prefers invoking NextUI's helper with that percentage; if the helper is
missing it falls back to a direct write to `cooling_device0/cur_state`. The direct sysfs fallback
uses NextUI-compatible rounding: `(31 * percent + 50) / 100`. Pass `-1` to leave the current
speed unchanged. Only has effect on TG5050; no-op (returns `AP_OK`) on all other platforms and
desktop builds.

#### `int ap_get_fan_speed(void)`

Read the current fan speed as a 0–100 percentage. Returns `0` on non-TG5050 platforms,
`-1` if the sysfs read fails.

#### Platform notes

| Platform | CPU sysfs | Fan |
|----------|-----------|-----|
| MY355 (Miyoo Flip) | `cpufreq/policy0/scaling_setspeed` | None |
| TG5040 (Trimui Brick) | `cpu0/cpufreq/scaling_setspeed` | None |
| TG5050 (Trimui Smart Pro S) | `cpu4/cpufreq/scaling_setspeed` (big core) | `cooling_device0/cur_state` (0–31), plus `fancontrol` helper auto curves |
| Desktop | No-op | No-op |

#### Example

```c
/* Set CPU at init */
ap_config cfg = { .is_nextui = AP_PLATFORM_IS_DEVICE,
                  .cpu_speed = AP_CPU_SPEED_NORMAL };
ap_init(&cfg);

/* Change speed at runtime */
ap_set_cpu_speed(AP_CPU_SPEED_PERFORMANCE);

int mhz  = ap_get_cpu_speed_mhz();   /* e.g. 1680 on TG5050 */
int temp = ap_get_cpu_temp_celsius(); /* e.g. 42   */

/* Fan (TG5050 only) */
ap_set_fan_mode(AP_FAN_MODE_AUTO_NORMAL);
ap_set_fan_speed(50);          /* 50% */
ap_fan_mode mode = ap_get_fan_mode();
int fan = ap_get_fan_speed();  /* 0–100 */
```

See `examples/perf/main.c` for a live-readout demo with preset picker.

### Error Handling

#### `const char *ap_get_error(void)`

Get the last error message set by Apostrophe. Returns an empty string if no error.

#### `bool ap_is_cancelled(int result)`

Convenience check: returns `true` if `result == AP_CANCELLED`.

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
- Optional hidden scrollbar (`hide_scrollbar`)
- Optional live footer updates (`footer_update`)
- Text overflow scrolling
- Help overlay (Menu)
- Explicit action bindings (`action_button`, `secondary_action_button`, `confirm_button`, `tertiary_action_button`)

Footer hints are visual only. Behavior is driven by the action button fields in `ap_list_opts`.

**`ap_list_item`**:
```c
typedef struct {
    const char  *label;
    const char  *metadata;         // Hidden item data (e.g. path), not rendered
    SDL_Texture *image;            // Optional thumbnail (shown when show_images = true)
    bool         selected;         // For multi-select
    SDL_Texture *background_image; // Optional fullscreen preview for the focused item
    const char  *trailing_text;    // Optional right-aligned visible hint text
} ap_list_item;
```

Use `metadata` for hidden payloads such as paths or IDs. Use `trailing_text` for right-aligned UI text shown in the row. The `AP_LIST_ITEM` / `AP_LIST_ITEM_BG` helper macros still initialize only `label` and `metadata`; use designated initializers when you want a visible trailing hint.

**`ap_list_opts`** (action-related fields):
```c
typedef struct ap_list_opts ap_list_opts;
typedef void (*ap_list_footer_update_fn)(ap_list_opts *opts, int cursor, void *userdata);

struct ap_list_opts {
    ...
    ap_button reorder_button;
    ap_button action_button;
    ap_button secondary_action_button;
    ap_button confirm_button;
    ap_button tertiary_action_button;
    bool      hide_scrollbar;      // Hide scrollbar while keeping scrolling behavior unchanged
    int       initial_index;
    int       visible_start_index;
    TTF_Font *item_font;           // Override list item text (default: AP_FONT_LARGE)
    ap_list_footer_update_fn footer_update; // Optional live footer updater
    void     *footer_update_userdata;
};
```

`item_font` overrides the font used to render list item labels and trailing hints. When `NULL` (zero-init default), the widget uses `ap_get_font(AP_FONT_LARGE)`. Pass a font obtained from `ap_get_font()` or a custom-loaded `TTF_Font` to override.

`hide_scrollbar` suppresses the scrollbar gutter and thumb without changing list navigation, cursor behavior, or visible-item paging.

`footer_update` runs once per list loop after cursor and scroll position are finalized, but before the footer is drawn. It may inspect `opts->items[cursor]` and update existing footer text such as `label` or `button_text`. Keep the callback cheap and avoid mutating layout-driving state such as `item_count`, `footer_count`, or replacing the footer/items arrays while the list is open. If the footer needs to refresh without input, call `ap_request_frame_in(ms)` or `ap_request_frame()` from inside the callback.

When `trailing_text` is set, `ap_list()` renders it right-aligned using `theme->hint`. The hint is skipped for that row if reserving space for it would leave less than `AP_S(96)` for the main label.

D-Pad Left/Right skip forward/backward by one page (`max_visible` items) in `ap_list`. L1/R1 jump between alphabetical letter groups (items should be pre-sorted for best results). Both require no configuration but are disabled while reorder mode is active.

The help overlay is triggered by the Menu button. When `help_text` is set, Menu shows the help overlay first; if hidden footer items also exist, the footer overflow overlay follows. When no `help_text` is set, Menu opens the footer overflow directly.

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
| `AP_OPT_STANDARD` | Left/Right cycles through predefined values; if `confirm_button == AP_BTN_A`, A confirms instead of cycling |
| `AP_OPT_KEYBOARD` | A opens keyboard for text input; if `confirm_button == AP_BTN_A`, confirming text also confirms the list |
| `AP_OPT_CLICKABLE` | A triggers a navigation/action callback |
| `AP_OPT_COLOR_PICKER` | A opens the color picker; if `confirm_button == AP_BTN_A`, picking a color also confirms the list |

Action buttons are explicit in `ap_options_list_opts` (`action_button`, `secondary_action_button`, `confirm_button`), and footer hints remain visual-only.
When `confirm_button` is set to `AP_BTN_A`, A takes on a "confirm and exit" role across all item types:
- **Standard items**: A confirms immediately (use Left/Right to change values).
- **Keyboard/Color picker items**: A opens the sub-editor; confirming inside it also exits the options list with `AP_ACTION_CONFIRMED`. Cancelling the sub-editor returns to the list.
- **Clickable items**: Unchanged — A exits with `AP_ACTION_SELECTED`.
When option storage is malformed (`options == NULL` or out-of-range `selected_option`), Apostrophe safely clamps/ignores the invalid value instead of dereferencing invalid memory.
Long labels and option values are ellipsized as needed to keep the left label and right value area from overlapping on narrow screens. `AP_OPT_CLICKABLE` rows render their trailing `>` in both focused and unfocused states.

**`ap_options_list_opts`** (action/scroll fields):
```c
typedef struct {
    ...
    int       initial_selected_index;
    int       visible_start_index;
    ap_button action_button;
    ap_button secondary_action_button;
    ap_button confirm_button;
    TTF_Font *label_font;          // Override option label text (default: AP_FONT_LARGE)
    TTF_Font *value_font;          // Override option value text (default: AP_FONT_TINY)
} ap_options_list_opts;
```

`label_font` overrides the font used for option labels; `value_font` overrides the font used for option values. When `NULL` (zero-init default), the widget uses `ap_get_font(AP_FONT_LARGE)` and `ap_get_font(AP_FONT_TINY)` respectively. Pass a font obtained from `ap_get_font()` or a custom-loaded `TTF_Font` to override.

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

**Parameters**:
- `initial_text`: Pre-filled text shown in the input field on open (may be `NULL` for empty).
- `help_text`: Text shown verbatim in the **Menu help overlay**. Pass `NULL` to use the built-in keyboard instructions. Do not pass a prompt string here — it is not displayed as an on-screen label.

**Button mapping** (Gabagool-compatible):
- **B**: Backspace
- **X**: Space (general) / Toggle symbol alternates (URL)
- **Y**: Exit without saving
- **Select**: Toggle shift
- **Start**: Confirm
- **L1/R1**: Move text cursor left/right
- **Menu**: Help overlay (shows `help_text` or built-in instructions when `help_text` is `NULL`)

**URL Keyboard** adds configurable shortcut rows above the QWERTY keys:
- Default shortcuts: `https://`, `www.`, `.com`, `.org`, `.net`, `.io`, `.dev`, `.app`, `.edu`, `.gov`
- X toggles to symbol alternates: `http://`, `ftp://`, `.co`, `.tv`, `.me`, `.gg`, `.uk`, `.de`, `.ca`, `.au`
- URL special chars row: `/ : @ - _ . ~ ? # &`
- Bottom-row toggle switches between `123` and `abc`, replacing the URL rows with digits and symbol sets when enabled
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

**`ap_detail_action`** enum:
```c
typedef enum {
    AP_DETAIL_BACK = 0,   // User pressed back
    AP_DETAIL_ACTION      // User pressed the action button
} ap_detail_action;
```

**`ap_detail_opts`** (styling fields):
```c
typedef struct {
    ...
    bool        center_title;
    bool        show_section_separator;
    const ap_color *key_color;
    TTF_Font   *body_font;           // Override body/value text (default: AP_FONT_TINY)
    TTF_Font   *section_title_font;  // Override section headers (default: AP_FONT_SMALL)
    TTF_Font   *key_font;            // Override info-pair key text (default: AP_FONT_TINY)
} ap_detail_opts;
```

`body_font` overrides the font used for description text and info-pair values. `section_title_font` overrides section header text. `key_font` overrides info-pair key (left-hand) text. When `NULL` (zero-init default), the widget uses `ap_get_font(AP_FONT_TINY)` for body and key text, and `ap_get_font(AP_FONT_SMALL)` for section titles. Pass a font obtained from `ap_get_font()` or a custom-loaded `TTF_Font` to override.

**`ap_detail_result`**:
```c
typedef struct {
    ap_detail_action action;
} ap_detail_result;
```

### Queue Viewer

```c
int ap_queue_viewer(const ap_queue_opts *opts);
```

Live-updating, filterable display for background job queues. The widget polls a caller-supplied snapshot callback each frame — all threading stays in the caller.

**Features**:
- Animated pill selection (same as list widget)
- Horizontal text scroll on long titles when selected
- Per-item inline progress bars on the subtitle row
- Filter cycling: ALL / IN PROGRESS / DONE / FAILED by default, overrideable via `filter_labels[4]` (Y button)
- Summary bar: `"X/Y COMPLETE, Z FAILED"` above footer
- A: Detail callback for terminal items (DONE/FAILED/SKIPPED)
- X: Cancel callback while active, clear-done callback when idle
- Menu / desktop `H`: open hidden footer actions when the footer shows `+N`
- Idle-aware: calls `ap_request_frame()` only while jobs are active

Navigation matches `ap_list()`: `D-Pad Left/Right` skip by one visible page, while `L1/R1` jump to the previous/next first-letter group within the active filter. For best results, keep queue titles pre-sorted.

**`ap_queue_status`**:
```c
typedef enum {
    AP_QUEUE_PENDING = 0,   // Waiting to start          (hint color)
    AP_QUEUE_RUNNING = 1,   // Actively being processed  (accent color)
    AP_QUEUE_DONE    = 2,   // Completed successfully     (soft green, RGBA 100, 200, 100, 255)
    AP_QUEUE_FAILED  = 3,   // Ended in error             (red)
    AP_QUEUE_SKIPPED = 4,   // Intentionally skipped or cancelled (hint color)
} ap_queue_status;
```

**`ap_queue_item`** (filled by snapshot callback each frame):
```c
typedef struct {
    char            title[256];      // Large primary label
    char            subtitle[128];   // Small secondary label
    char            status_text[64]; // Right-aligned status string
    ap_queue_status status;          // Color-coding and filter
    float           progress;        // 0.0–1.0 = inline progress bar; < 0 = no bar
    void           *userdata;        // Caller-defined context
} ap_queue_item;
```

**`ap_queue_opts`**:
```c
typedef struct {
    const char           *title;       // Screen title, e.g. "DOWNLOADS"
    ap_queue_snapshot_fn  snapshot;    // Required: fills items each frame
    int                   max_items;   // Buffer capacity; 0 → 256
    void                 *userdata;    // Passed to all callbacks
    ap_queue_detail_fn    on_detail;   // Optional: A button on terminal items
    ap_queue_cancel_fn    on_cancel;   // Optional: X button while queue active
    ap_queue_clear_fn     on_clear;    // Optional: X button when queue idle
    ap_status_bar_opts   *status_bar;  // Optional: top-right status pill
    bool                  hide_filter; // Set true to hide Y=FILTER cycling
    const char           *filter_labels[4]; // Optional filter labels: [0]=ALL, [1]=IN PROGRESS (PENDING+RUNNING), [2]=DONE, [3]=FAILED; NULL or "" entries use defaults
} ap_queue_opts;
```

**Callbacks**:
```c
// Fill buf[0..max] with current state. Must be thread-safe. Return count.
typedef int  (*ap_queue_snapshot_fn)(ap_queue_item *buf, int max, void *userdata);

// Called when A is pressed on a terminal item. May push another widget.
typedef void (*ap_queue_detail_fn)(const ap_queue_item *item, void *userdata);

// Called when X is pressed while active items remain.
// Update your queue state so subsequent snapshots reflect cancellation.
typedef void (*ap_queue_cancel_fn)(void *userdata);

// Called when X is pressed to clear completed items.
typedef void (*ap_queue_clear_fn)(void *userdata);
```

**Example** (minimal):
```c
static int my_snapshot(ap_queue_item *buf, int max, void *ud) {
    pthread_mutex_lock(&queue_mutex);
    int n = copy_queue_to_buf(buf, max);  // your thread-safe copy
    pthread_mutex_unlock(&queue_mutex);
    return n;
}

static void my_cancel(void *ud) {
    /* confirm cancellation, then mark unfinished items as CANCELLED */
}

ap_queue_opts opts = {
    .title         = "DOWNLOADS",
    .snapshot      = my_snapshot,
    .on_cancel     = my_cancel,
    .filter_labels = { "ALL", "ACTIVE", "COMPLETE", "ERRORS" },
};
ap_queue_viewer(&opts);
```

To represent cancelled downloads, keep queue state in your own data model and have future snapshots return `AP_QUEUE_SKIPPED` with `status_text = "CANCELLED"` for unfinished items after `on_cancel` runs.

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

Full-screen scrollable text overlay. Typically triggered by Menu in widgets that have `help_text` configured.

### File Picker

#### Types

```c
typedef enum {
    AP_FILE_PICKER_FILES = 0,   /* Only files are selectable            */
    AP_FILE_PICKER_DIRS  = 1,   /* Only directories are selectable      */
    AP_FILE_PICKER_BOTH  = 2,   /* Files and directories are selectable */
} ap_file_picker_mode;
```

```c
typedef struct {
    const char           *title;           /* Screen title (NULL = show relative path from root) */
    ap_file_picker_mode   mode;            /* What can be selected */
    const char           *initial_path;    /* Starting directory (NULL = root_path) */
    const char           *root_path;       /* Navigation boundary (NULL = auto-detect per platform) */
    const char          **extensions;      /* File extension filter array, e.g. {"zip","7z"} */
    int                   extension_count; /* Number of entries in extensions array */
    bool                  allow_create;    /* Show NEW DIR action (X button) in DIRS/BOTH modes */
    bool                  show_hidden;     /* Show dotfiles/dotdirs (e.g. .env, .gitignore, .config) */
    ap_status_bar_opts   *status_bar;      /* Optional status bar */
} ap_file_picker_opts;
```

```c
typedef struct {
    char path[1024];      /* Full absolute path of the selected item */
    bool is_directory;    /* True when the selected item is a directory */
} ap_file_picker_result;
```

#### Functions

| Function | Description |
|----------|-------------|
| `ap_file_picker_default_opts(title)` | Create default options (mode = FILES, no filter, no create) |
| `ap_file_picker(opts, result)` | Show blocking file picker. Returns `AP_OK` on selection, `AP_CANCELLED` on back from root |

#### Controls

| Button | Action |
|--------|--------|
| D-Pad Up/Down | Navigate items |
| D-Pad Left/Right | Page up/down |
| L1/R1 | Jump between letter groups |
| A | Enter folder / select file |
| B | Go up one directory (cancel at root) |
| X | Create new folder (DIRS / BOTH modes with `allow_create`) |
| START | Select current directory (DIRS / BOTH modes) |

#### Root Path Resolution

| Platform | Default root |
|----------|-------------|
| Device (`AP_PLATFORM_IS_DEVICE`) | `SDCARD_PATH` env var, or `/mnt/SDCARD` |
| Windows | `USERPROFILE` env var, or `.` |
| macOS / Linux | `HOME` env var, or `.` |

Notes:
- On device, `SDCARD_PATH` is always the hard ceiling even if `root_path` is provided.
- On desktop, `root_path` overrides the default home root when provided.
- `initial_path` is only used when it resolves to a directory inside the effective root.
- `allow_create` is ignored in `AP_FILE_PICKER_FILES`.
- In `AP_FILE_PICKER_BOTH`, the `A` footer hint updates live: `ENTER` on directories, `OPEN` on files.
- Set `show_hidden = true` to list and select dotfiles like `.env` and dot-directories like `.config`.
- New-folder creation only accepts a single path component; separators and `.` / `..` are rejected.
- Dotfiles still use the normal extension-filter logic when `extensions` are configured.

#### Usage Example

```c
ap_file_picker_opts opts = ap_file_picker_default_opts("Select ROM");
opts.mode = AP_FILE_PICKER_FILES;
opts.extensions = (const char *[]){"zip", "7z"};
opts.extension_count = 2;

ap_file_picker_result result;
int rc = ap_file_picker(&opts, &result);
if (rc == AP_OK) {
    printf("Selected: %s (dir=%d)\n", result.path, result.is_directory);
}
```
