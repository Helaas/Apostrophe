/*
 * Apostrophe — A C UI toolkit for NextUI Paks on retro gaming handhelds
 *
 * Header-only library. Define AP_IMPLEMENTATION in exactly ONE .c file
 * before including this header to generate the implementation.
 *
 *   #define AP_IMPLEMENTATION
 *   #include "apostrophe.h"
 *
 * Dependencies: SDL2, SDL2_ttf, SDL2_image, C standard library, pthreads
 * Platforms:    tg5040 (TrimUI Brick/Smart Pro), tg5050 (TrimUI Smart Pro S),
 *               my355 (Miyoo Flip), macOS, Linux, Windows (dev/testing)
 *
 * License: MIT
 * https://github.com/Helaas/apostrophe
 */

#ifndef APOSTROPHE_H
#define APOSTROPHE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>

#ifdef __linux__
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#if defined(PLATFORM_TG5040) || defined(PLATFORM_TG5050) || defined(PLATFORM_MY355)
#include <linux/input.h>
#endif
#endif

#ifdef __APPLE__
#include <pthread.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <io.h>
#ifndef R_OK
#define R_OK 4
#endif
#ifndef X_OK
#define X_OK 0  /* X_OK not meaningful on Windows; fall back to existence check */
#endif
#ifndef access
#define access _access
#endif
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * Platform Detection
 * ═══════════════════════════════════════════════════════════════════════════ */

#if defined(PLATFORM_TG5040)
    #define AP_PLATFORM_NAME "tg5040"
    #define AP_PLATFORM_IS_DEVICE 1
#elif defined(PLATFORM_TG5050)
    #define AP_PLATFORM_NAME "tg5050"
    #define AP_PLATFORM_IS_DEVICE 1
#elif defined(PLATFORM_MY355)
    #define AP_PLATFORM_NAME "my355"
    #define AP_PLATFORM_IS_DEVICE 1
#elif defined(PLATFORM_MAC) || defined(__APPLE__)
    #define AP_PLATFORM_NAME "mac"
    #define AP_PLATFORM_IS_DEVICE 0
    #ifndef PLATFORM_MAC
        #define PLATFORM_MAC
    #endif
#elif defined(PLATFORM_WINDOWS) || defined(_WIN32)
    #define AP_PLATFORM_NAME "windows"
    #define AP_PLATFORM_IS_DEVICE 0
    #ifndef PLATFORM_WINDOWS
        #define PLATFORM_WINDOWS
    #endif
#elif defined(PLATFORM_LINUX) || defined(__linux__)
    #define AP_PLATFORM_NAME "linux"
    #define AP_PLATFORM_IS_DEVICE 0
    #ifndef PLATFORM_LINUX
        #define PLATFORM_LINUX
    #endif
#else
    #define AP_PLATFORM_NAME "unknown"
    #define AP_PLATFORM_IS_DEVICE 0
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * Constants & Return Codes
 * ═══════════════════════════════════════════════════════════════════════════ */

#define AP_OK        0
#define AP_ERROR    (-1)
#define AP_CANCELLED (-2)

/* Design reference width for scaling calculations */
#define AP_REFERENCE_WIDTH 1024

/* Scaling damping factor for screens wider than reference */
#define AP_SCALE_DAMPING 0.75f

/* Font bump: additive increase to base font sizes on larger logical screens.
   Reference is MY355 logical resolution (640/2 × 480/2 = 320×240). */
#define AP_FONT_BUMP_MAX           5
#define AP_FONT_BUMP_REF_LOGICAL_W 320
#define AP_FONT_BUMP_REF_LOGICAL_H 240

/* Input timing defaults (milliseconds) */
#define AP_INPUT_REPEAT_DELAY  300
#define AP_INPUT_REPEAT_RATE   100
#define AP_INPUT_DEBOUNCE       20
#ifdef PLATFORM_MY355
#define AP_AXIS_DEADZONE     20000  /* MY355 joystick needs higher deadzone to avoid crosstalk */
#else
#define AP_AXIS_DEADZONE     16000
#endif

/* Text scroll timing */
#define AP_TEXT_SCROLL_SPEED     1
#define AP_TEXT_SCROLL_PAUSE_MS 1000

/* Texture cache capacity */
#define AP_TEXTURE_CACHE_SIZE 8

/* Max combo registrations */
#define AP_MAX_COMBOS 16

/* Footer layout bookkeeping */
#define AP__MAX_FOOTER_ITEMS 64

/* Max log message length */
#define AP_MAX_LOG_LEN 2048

/* ═══════════════════════════════════════════════════════════════════════════
 * Enums
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Virtual button abstraction — unifies keyboard, joystick, gamepad input */
typedef enum {
    AP_BTN_NONE = 0,
    AP_BTN_UP,
    AP_BTN_DOWN,
    AP_BTN_LEFT,
    AP_BTN_RIGHT,
    AP_BTN_A,
    AP_BTN_B,
    AP_BTN_X,
    AP_BTN_Y,
    AP_BTN_L1,
    AP_BTN_L2,
    AP_BTN_R1,
    AP_BTN_R2,
    AP_BTN_START,
    AP_BTN_SELECT,
    AP_BTN_MENU,
    AP_BTN_POWER,
    AP_BTN_COUNT
} ap_button;

/* Font size tiers — scaled to screen resolution at init */
typedef enum {
    AP_FONT_EXTRA_LARGE = 0,  /* Base: 24 × device_scale */
    AP_FONT_LARGE,             /* Base: 16 × device_scale */
    AP_FONT_MEDIUM,            /* Base: 14 × device_scale */
    AP_FONT_SMALL,             /* Base: 12 × device_scale */
    AP_FONT_TINY,              /* Base: 10 × device_scale */
    AP_FONT_MICRO,             /* Base:  7 × device_scale */
    AP_FONT_TIER_COUNT
} ap_font_tier;

/* Text alignment */
typedef enum {
    AP_ALIGN_LEFT = 0,
    AP_ALIGN_CENTER,
    AP_ALIGN_RIGHT
} ap_text_align;

/* List actions returned by widgets */
typedef enum {
    AP_ACTION_SELECTED = 0,
    AP_ACTION_BACK,
    AP_ACTION_TRIGGERED,
    AP_ACTION_SECONDARY_TRIGGERED,
    AP_ACTION_CONFIRMED,
    AP_ACTION_TERTIARY_TRIGGERED,
    AP_ACTION_CUSTOM = AP_ACTION_TRIGGERED
} ap_list_action;

/* CPU speed presets — platform-transparent, see ap_set_cpu_speed().
 * Approximate frequencies per platform:
 *
 *   Preset           MY355        TG5040       TG5050 (big core)
 *   AP_CPU_SPEED_MENU       600 MHz      600 MHz      672 MHz
 *   AP_CPU_SPEED_POWERSAVE 1200 MHz     1200 MHz     1200 MHz
 *   AP_CPU_SPEED_NORMAL    1608 MHz     1608 MHz     1680 MHz  ← standard pak speed
 *   AP_CPU_SPEED_PERFORMANCE 2000 MHz  2000 MHz     2160 MHz
 *
 * AP_CPU_SPEED_DEFAULT (0) means "do not change" and is the zero-init default. */
typedef enum {
    AP_CPU_SPEED_DEFAULT     = 0, /* do not change at init */
    AP_CPU_SPEED_MENU,            /* ~600–672 MHz  — light UI / menus    */
    AP_CPU_SPEED_POWERSAVE,       /* ~1200 MHz     — battery saving       */
    AP_CPU_SPEED_NORMAL,          /* ~1608–1680 MHz — standard pak speed  */
    AP_CPU_SPEED_PERFORMANCE,     /* ~2000–2160 MHz — max speed           */
} ap_cpu_speed;

typedef enum {
    AP_FAN_MODE_UNSUPPORTED = -1,
    AP_FAN_MODE_MANUAL = 0,
    AP_FAN_MODE_AUTO_QUIET,
    AP_FAN_MODE_AUTO_NORMAL,
    AP_FAN_MODE_AUTO_PERFORMANCE,
} ap_fan_mode;

/* ═══════════════════════════════════════════════════════════════════════════
 * Structs
 * ═══════════════════════════════════════════════════════════════════════════ */

/* RGBA color — wraps SDL_Color for convenience */
typedef SDL_Color ap_color;

/* Theme — all colors used by the UI, loaded from NextUI or set manually */
typedef struct {
    ap_color highlight;         /* Selected item pill background */
    ap_color accent;            /* Footer outer pill, status bar bg */
    ap_color button_label;      /* Text inside footer button pills */
    ap_color text;              /* Default text color */
    ap_color highlighted_text;  /* Text on highlighted/selected items */
    ap_color hint;              /* Help text, dim text */
    ap_color background;        /* Screen background color */
    char     font_path[512];    /* Primary font file path */
    char     bg_image_path[512];/* Background image path (PNG) */
} ap_theme;

/* Input event — unified from all input sources */
typedef struct {
    ap_button button;
    bool      pressed;   /* true = down, false = up */
    bool      repeated;  /* true = generated by hold-repeat, false = fresh press */
} ap_input_event;

/* Text scroll state — horizontal ping-pong scrolling for overflow text */
typedef struct {
    int   offset;
    int   direction;     /* 1 = right-to-left, -1 = left-to-right */
    int   pause_timer;   /* ms remaining in pause at each end */
    bool  active;
} ap_text_scroll;

/* Footer help item — button hint displayed at screen bottom */
typedef struct {
    ap_button    button;
    const char  *label;
    bool         is_confirm;  /* true = right-aligned confirm group */
    const char  *button_text; /* Optional footer pill text override (display-only) */
} ap_footer_item;

/* Global footer overflow behaviour */
typedef struct {
    bool      enabled;  /* When false, footer hints render without overflow handling */
    ap_button chord_a;  /* First button in the hidden-actions chord */
    ap_button chord_b;  /* Second button in the hidden-actions chord */
} ap_footer_overflow_opts;

/* Clock display mode for ap_status_bar_opts.show_clock */
#define AP_CLOCK_AUTO  0  /* Follow NextUI showclock setting (default) */
#define AP_CLOCK_SHOW  1  /* Always show, regardless of NextUI setting  */
#define AP_CLOCK_HIDE  2  /* Always hide, regardless of NextUI setting  */

/* Status bar options */
typedef struct {
    int          show_clock;    /* AP_CLOCK_AUTO/SHOW/HIDE (default: AP_CLOCK_AUTO) */
    bool         use_24h;       /* Ignored in AUTO mode; NextUI clock24h used instead */
    bool         show_battery;  /* Show battery icon from device or desktop preview state */
    bool         show_wifi;     /* Show wifi icon from device or desktop preview state */
} ap_status_bar_opts;

/* Screen fade overlay — per-frame draw state for fade-in / fade-out transitions */
typedef struct {
    uint32_t start_ms;     /* SDL_GetTicks() value when fade began */
    int      duration_ms;  /* Total duration of the fade */
    bool     fade_in;      /* true = black→transparent, false = transparent→black */
    bool     active;       /* false = not animating */
} ap_fade;

/* Texture cache entry */
typedef struct {
    char          key[256];
    SDL_Texture  *texture;
    int           w, h;
    uint32_t      last_used;
} ap_cache_entry;

/* Texture cache (LRU) */
typedef struct {
    ap_cache_entry entries[AP_TEXTURE_CACHE_SIZE];
    int            count;
} ap_texture_cache;

/* Sequence buffer entry (internal) */
typedef struct {
    ap_button  button;
    uint32_t   time_ms;
} ap__seq_entry;

/* Combo types and callbacks */
typedef enum { AP_COMBO_CHORD, AP_COMBO_SEQUENCE } ap_combo_type;
typedef void (*ap_combo_callback)(const char *id, ap_combo_type type, void *userdata);

/* Combo registration */
typedef struct {
    char       id[64];
    ap_button  buttons[8];
    int        button_count;
    uint32_t   window_ms;       /* Time window for chord/sequence */
    bool       is_sequence;     /* false = chord (simultaneous), true = sequence */
    bool       strict;          /* For sequences: must be exact order */
    bool       active;
    ap_combo_callback on_trigger;
    ap_combo_callback on_release;   /* Chords only */
    void             *userdata;
} ap_combo;

/* Combo event */
typedef struct {
    const char    *id;
    bool           triggered;
    ap_combo_type  type;
} ap_combo_event;

/* Configuration passed to ap_init() */
typedef struct {
    const char *window_title;     /* Window title (dev mode only) */
    const char *font_path;        /* Path to .ttf font file, NULL = auto */
    const char *bg_image_path;    /* Background image path, NULL = none */
    const char *log_path;         /* Log file path, NULL = stderr only */
    const char *primary_color_hex;/* Override accent color "#RRGGBB", NULL = theme default */
    bool        disable_background;  /* Set true to skip bg.png rendering */
    bool        is_nextui;        /* Load theme from NextUI's nextval.elf */
    ap_cpu_speed cpu_speed;       /* Set CPU at init; 0 = AP_CPU_SPEED_DEFAULT (no-op) */
    bool        disable_font_bump;   /* Set true to disable automatic font bumping */
} ap_config;

/* ═══════════════════════════════════════════════════════════════════════════
 * Internal State (opaque to user, but defined here for header-only use)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    /* SDL */
    SDL_Window         *window;
    SDL_Renderer       *renderer;
    SDL_Joystick       *joystick;
    SDL_GameController *controller;
    SDL_Texture        *bg_texture;
    int                 screen_w;
    int                 screen_h;
    bool                renderer_has_vsync;
    uint32_t            last_present_ms;
    bool                needs_frame;       /* true = render next frame at 60fps */
    uint32_t            next_redraw_ms;    /* absolute time of next scheduled redraw (0 = none) */
    bool                has_wake_event;    /* stored event from SDL_WaitEventTimeout */
    SDL_Event           wake_event;

    /* Scaling */
    float         scale_factor;

    /* Theme */
    ap_theme      theme;

    /* Fonts */
    TTF_Font     *fonts[AP_FONT_TIER_COUNT];

    /* Input state */
    bool          face_buttons_flipped;
    uint32_t      input_delay_ms;
    uint32_t      input_repeat_delay_ms;
    uint32_t      input_repeat_rate_ms;
    uint32_t      last_input_time;

    /* Directional repeat */
    uint8_t       hat_held;
    uint32_t      hat_repeat_time;
    int           axis_held_dir_y;   /* -1=up, +1=down, 0=none */
    int           axis_held_dir_x;   /* -1=left, +1=right, 0=none */
    uint32_t      axis_repeat_time_y;
    uint32_t      axis_repeat_time_x;

    /* Combos */
    ap_combo      combos[AP_MAX_COMBOS];
    int           combo_count;

    /* Combo event queue */
    ap_combo_event combo_queue[16];
    int            combo_queue_head;
    int            combo_queue_tail;

    /* Sequence detection buffer */
    ap__seq_entry seq_buffer[20];
    int           seq_buffer_count;

    /* Chord held tracking (separate from ap_combo.active which means "registered") */
    bool          combo_held[AP_MAX_COMBOS];

    /* Button held state for chords */
    bool          buttons_held[AP_BTN_COUNT];
    uint32_t      button_press_time[AP_BTN_COUNT];
    uint32_t      button_repeat_time[AP_BTN_COUNT];

    /* Footer overflow */
    ap_footer_overflow_opts footer_overflow_opts;
    bool          footer_overflow_active;
    bool          footer_overflow_chord_held;
    bool          footer_overflow_open_requested;
    bool          footer_overflow_overlay_open;
    bool          footer_overflow_swallow[AP_BTN_COUNT];
    ap_footer_item footer_hidden_items[AP__MAX_FOOTER_ITEMS];
    int            footer_hidden_count;

    /* Texture cache */
    ap_texture_cache tex_cache;

    /* Logging */
    FILE         *log_file;

    /* Status bar assets (NextUI spritesheet) */
    SDL_Texture  *status_assets;
    int           status_asset_scale;  /* 1–4, matches loaded spritesheet */

    /* Integer device scale matching NextUI's FIXED_SCALE (for pixel-perfect bars) */
    int           device_scale;   /* typically 2 (handheld) or 3 (brick) */
    int           device_padding; /* NextUI's PADDING constant (unscaled) */
    int           font_bump;     /* additive bump applied to base font sizes (0–5) */

    /* WiFi signal strength cache (device only, matches NextUI 5s poll interval) */
    #if AP_PLATFORM_IS_DEVICE
    int           cached_wifi_strength;     /* 0=off, 1=low, 2=med, 3=high */
    uint32_t      wifi_cache_time_ms;       /* SDL_GetTicks() when last polled */
    #endif

    /* Power button handling */
    bool          power_handler_enabled;
    #if AP_PLATFORM_IS_DEVICE
    pthread_t     power_thread;
    bool          power_thread_running;
    int           power_fd;
    #endif

    /* Initialization flag */
    bool          initialized;
} ap__state;

/* ═══════════════════════════════════════════════════════════════════════════
 * Macros
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Scale a design-space value to screen space */
#define AP_S(base) ((int)((base) * ap__g.scale_factor))

/* Scale using integer device scale (matches NextUI's SCALE1 macro for bar sizing) */
#define AP_DS(base) ((base) * ap__g.device_scale)

/* NextUI bar layout constants (all unscaled, multiply by device_scale) */
#define AP__PILL_SIZE       30  /* Height of status/footer pill */
#define AP__BUTTON_SIZE     20  /* Button circle size inside footer */
#define AP__BUTTON_MARGIN    5  /* Margin between pill edge and button / inter-element gap */
#define AP__BUTTON_PADDING  12  /* Padding between pill edge and content */

/* Float lerp/clamp — used by animation helpers */
static inline float ap__lerpf(float a, float b, float t) { return a + (b - a) * t; }
static inline float ap__clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Lifecycle
 * ═══════════════════════════════════════════════════════════════════════════ */

int            ap_init(ap_config *cfg);
void           ap_quit(void);
SDL_Renderer  *ap_get_renderer(void);
SDL_Window    *ap_get_window(void);
int            ap_get_screen_width(void);
int            ap_get_screen_height(void);
void           ap_show_window(void);
void           ap_hide_window(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Scaling
 * ═══════════════════════════════════════════════════════════════════════════ */

float          ap_get_scale_factor(void);
int            ap_scale(int base);
int            ap_font_size_for_resolution(int base_size);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Theming
 * ═══════════════════════════════════════════════════════════════════════════ */

ap_theme      *ap_get_theme(void);
int            ap_theme_load_nextui(void);
int            ap_reload_background(const char *bg_path);
ap_color       ap_hex_to_color(const char *hex);
void           ap_set_theme_color(const char *hex);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Fonts
 * ═══════════════════════════════════════════════════════════════════════════ */

TTF_Font      *ap_get_font(ap_font_tier tier);
int            ap_get_font_bump(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Input
 * ═══════════════════════════════════════════════════════════════════════════ */

bool           ap_poll_input(ap_input_event *event);
void           ap_set_input_delay(uint32_t ms);
void           ap_set_input_repeat(uint32_t delay_ms, uint32_t rate_ms);
void           ap_flip_face_buttons(bool flip);
const char    *ap_button_name(ap_button btn);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Combos
 * ═══════════════════════════════════════════════════════════════════════════ */

int            ap_register_chord(const char *id, ap_button *buttons, int count, uint32_t window_ms);
int            ap_register_sequence(const char *id, ap_button *buttons, int count, uint32_t timeout_ms, bool strict);
int            ap_register_chord_ex(const char *id, ap_button *buttons, int count,
                                    uint32_t window_ms,
                                    ap_combo_callback on_trigger,
                                    ap_combo_callback on_release,
                                    void *userdata);
int            ap_register_sequence_ex(const char *id, ap_button *buttons, int count,
                                      uint32_t timeout_ms, bool strict,
                                      ap_combo_callback on_trigger,
                                      void *userdata);
void           ap_unregister_combo(const char *id);
void           ap_clear_combos(void);
bool           ap_poll_combo(ap_combo_event *event);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Drawing Primitives
 * ═══════════════════════════════════════════════════════════════════════════ */

void           ap_clear_screen(void);
void           ap_present(void);
void           ap_draw_background(void);
void           ap_draw_rounded_rect(int x, int y, int w, int h, int r, ap_color c);
void           ap_draw_pill(int x, int y, int w, int h, ap_color c);
void           ap_draw_rect(int x, int y, int w, int h, ap_color c);
void           ap_draw_circle(int cx, int cy, int r, ap_color c);
int            ap_draw_text(TTF_Font *font, const char *text, int x, int y, ap_color color);          /* returns rendered width in pixels */
int            ap_draw_text_clipped(TTF_Font *font, const char *text, int x, int y, ap_color color, int max_w); /* returns rendered width */
int            ap_draw_text_ellipsized(TTF_Font *font, const char *text, int x, int y, ap_color color, int max_w); /* truncate with "..." if too wide */
int            ap_draw_text_wrapped(TTF_Font *font, const char *text, int x, int y, int max_w, ap_color color, ap_text_align align); /* returns rendered height */
int            ap_measure_text(TTF_Font *font, const char *text);
int            ap_measure_text_ellipsized(TTF_Font *font, const char *text, int max_w); /* measure width text would occupy when ellipsized to fit max_w */
void           ap_draw_image(SDL_Texture *tex, int x, int y, int w, int h);
SDL_Texture   *ap_load_image(const char *path);
void           ap_draw_scrollbar(int x, int y, int h, int visible, int total, int offset);
void           ap_draw_progress_bar(int x, int y, int w, int h, float progress, ap_color fg, ap_color bg);
SDL_Rect       ap_get_content_rect(bool has_title, bool has_footer, bool has_status_bar);
void           ap_draw_screen_title(const char *title, ap_status_bar_opts *status_bar);
void           ap_draw_screen_title_centered(const char *title, ap_status_bar_opts *status_bar);
int            ap_measure_wrapped_text_height(TTF_Font *font, const char *text, int max_w);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Text Scrolling
 * ═══════════════════════════════════════════════════════════════════════════ */

void           ap_text_scroll_init(ap_text_scroll *s);
void           ap_text_scroll_update(ap_text_scroll *s, int text_w, int visible_w, uint32_t dt_ms);
void           ap_text_scroll_reset(ap_text_scroll *s);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Texture Cache
 * ═══════════════════════════════════════════════════════════════════════════ */

SDL_Texture   *ap_cache_get(const char *key, int *w, int *h);
void           ap_cache_put(const char *key, SDL_Texture *tex, int w, int h);
void           ap_cache_clear(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Footer & Status Bar
 * ═══════════════════════════════════════════════════════════════════════════ */

void           ap_draw_footer(ap_footer_item *items, int count);
int            ap_get_footer_height(void);
void           ap_set_footer_overflow_opts(const ap_footer_overflow_opts *opts);
void           ap_get_footer_overflow_opts(ap_footer_overflow_opts *out);
void           ap_show_footer_overflow(void);
void           ap_draw_status_bar(ap_status_bar_opts *opts);
int            ap_get_status_bar_height(void);
int            ap_get_status_bar_width(ap_status_bar_opts *opts);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Logging
 * ═══════════════════════════════════════════════════════════════════════════ */

void           ap_log(const char *fmt, ...);
void           ap_set_log_path(const char *path);
const char    *ap_resolve_log_path(const char *app_name);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Power Button
 * ═══════════════════════════════════════════════════════════════════════════ */

void           ap_set_power_handler(bool enabled);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — CPU & Fan
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Set CPU to a named speed preset. Sets the governor to "userspace" first,
 * then writes the platform-specific frequency. Returns AP_OK on success.
 * No-op (returns AP_OK) on desktop builds. */
int            ap_set_cpu_speed(ap_cpu_speed speed);

/* Read the current CPU frequency in MHz. Returns -1 on error or desktop. */
int            ap_get_cpu_speed_mhz(void);

/* Read the CPU temperature in °C. Returns -1 on error or desktop. */
int            ap_get_cpu_temp_celsius(void);

/* Set TG5050 fan mode. Manual stops any active auto daemon; auto modes mirror
 * NextUI's quiet/normal/performance fancontrol helper.
 * No-op (returns AP_OK) on platforms without fan hardware. */
int            ap_set_fan_mode(ap_fan_mode mode);

/* Read the current TG5050 fan mode. Returns AP_FAN_MODE_UNSUPPORTED on
 * platforms without fan hardware or if the current mode cannot be determined. */
ap_fan_mode    ap_get_fan_mode(void);

/* Set fan speed as a 0–100 percentage. Internally maps to 0–31 raw levels.
 * On TG5050 this also stops any active auto fan daemon before applying a
 * fixed speed. Pass -1 to leave the current speed unchanged.
 * Only has effect on TG5050; no-op (returns AP_OK) on all other platforms. */
int            ap_set_fan_speed(int percent);

/* Read current fan speed as a 0–100 percentage.
 * Returns 0 on non-TG5050 platforms, -1 if the value cannot be read. */
int            ap_get_fan_speed(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Screen Fade
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Begin a fade-in: overlay starts fully black and becomes transparent */
static inline void ap_fade_begin_in(ap_fade *f, int duration_ms) {
    if (!f) return;
    f->start_ms    = SDL_GetTicks();
    f->duration_ms = duration_ms > 0 ? duration_ms : 1;
    f->fade_in     = true;
    f->active      = true;
}

/* Begin a fade-out: overlay starts transparent and becomes fully black */
static inline void ap_fade_begin_out(ap_fade *f, int duration_ms) {
    if (!f) return;
    f->start_ms    = SDL_GetTicks();
    f->duration_ms = duration_ms > 0 ? duration_ms : 1;
    f->fade_in     = false;
    f->active      = true;
}

void ap_request_frame(void);  /* forward declaration for ap_fade_draw */

/* Draw the fade overlay. Call AFTER drawing your scene, BEFORE ap_present().
 * Returns true while the fade is still active, false when complete. */
static inline bool ap_fade_draw(ap_fade *f) {
    if (!f || !f->active) return false;
    uint32_t elapsed = SDL_GetTicks() - f->start_ms;
    float t = ap__clampf((float)elapsed / (float)f->duration_ms, 0.0f, 1.0f);
    float alpha_f = f->fade_in ? ap__lerpf(255.0f, 0.0f, t)
                               : ap__lerpf(0.0f, 255.0f, t);
    SDL_Renderer *rend = ap_get_renderer();
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, (Uint8)(int)alpha_f);
    SDL_Rect full = { 0, 0, ap_get_screen_width(), ap_get_screen_height() };
    SDL_RenderFillRect(rend, &full);
    if (t >= 1.0f) { f->active = false; return false; }
    ap_request_frame();  /* keep rendering at 60fps while fade is active */
    return true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Error Handling
 * ═══════════════════════════════════════════════════════════════════════════ */

const char    *ap_get_error(void);
bool           ap_is_cancelled(int result);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Rendering
 * ═══════════════════════════════════════════════════════════════════════════ */

void  ap_request_frame(void);
void  ap_request_frame_in(uint32_t ms);

/* ═══════════════════════════════════════════════════════════════════════════
 * IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifdef AP_IMPLEMENTATION

/* Global state singleton */
static ap__state ap__g = {0};

/* Last error message buffer */
static char ap__error_buf[512] = {0};

/* ─── Internal Helpers ───────────────────────────────────────────────────── */

static void ap__set_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(ap__error_buf, sizeof(ap__error_buf), fmt, args);
    va_end(args);
}

static int ap__clamp(int val, int lo, int hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

#if !AP_PLATFORM_IS_DEVICE
static bool ap__env_parse_int(const char *name, int *out) {
    if (!name || !out) return false;

    const char *value = getenv(name);
    if (!value || !value[0]) return false;

    errno = 0;
    char *end = NULL;
    long parsed = strtol(value, &end, 10);
    if (errno != 0 || end == value || (end && *end != '\0')) return false;
    if (parsed < INT_MIN || parsed > INT_MAX) return false;

    *out = (int)parsed;
    return true;
}
#endif

static const char *ap__status_assets_dir(char *buf, size_t buf_size) {
    const char *dir = getenv("AP_STATUS_ASSETS_DIR");
    if (dir && dir[0]) return dir;

#if AP_PLATFORM_IS_DEVICE
    const char *sdcard = getenv("SDCARD_PATH");
    if (!sdcard || !sdcard[0]) sdcard = "/mnt/SDCARD";
    snprintf(buf, buf_size, "%s/.system/res", sdcard);
    return buf;
#else
    (void)buf;
    (void)buf_size;
    return NULL;
#endif
}

static int ap__max(int a, int b) { return a > b ? a : b; }

/* Base font sizes — multiplied by device_scale, matching NextUI defines.h */
static const int ap__font_base_sizes[AP_FONT_TIER_COUNT] = {
    24, /* EXTRA_LARGE — title/header (no NextUI equivalent) */
    16, /* LARGE       — menu/list items (NextUI FONT_LARGE) */
    14, /* MEDIUM      — single-char button label (NextUI FONT_MEDIUM) */
    12, /* SMALL       — hint text, status clock (NextUI FONT_SMALL) */
    10, /* TINY        — multi-char button label (NextUI FONT_TINY) */
     7, /* MICRO       — overlay text (NextUI FONT_MICRO) */
};

/* Default theme (matches Gabagool's NextUI defaults) */
static const ap_theme ap__default_theme = {
    .highlight        = {255, 255, 255, 255},  /* white */
    .accent           = {155,  34,  87, 255},  /* #9B2257 */
    .button_label     = { 30,  35,  41, 255},  /* #1E2329 */
    .text             = {255, 255, 255, 255},  /* white */
    .highlighted_text = {  0,   0,   0, 255},  /* black */
    .hint             = {255, 255, 255, 255},  /* white */
    .background       = {  0,   0,   0, 255},  /* black */
    .font_path        = "",
    .bg_image_path    = "",
};

/* Font search paths by platform — NextUI font1.ttf preferred over legacy font.ttf */
static const char *ap__font_search_paths[] = {
    "./font.ttf",
    "./res/font.ttf",
    "../res/font.ttf",
#if defined(PLATFORM_TG5040) || defined(PLATFORM_TG5050)
    "/mnt/SDCARD/.system/res/font1.ttf",
    "/mnt/SDCARD/.system/res/font2.ttf",
    "/mnt/SDCARD/.system/res/font.ttf",
    "/mnt/SDCARD/.system/tg5040/res/font.ttf",
#elif defined(PLATFORM_MY355)
    "/mnt/SDCARD/.system/res/font1.ttf",
    "/mnt/SDCARD/.system/res/font2.ttf",
    "/mnt/SDCARD/.system/res/font.ttf",
    "/mnt/SDCARD/.system/my355/res/font.ttf",
#elif defined(PLATFORM_MAC)
    "/System/Library/Fonts/Helvetica.ttc",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
#elif defined(PLATFORM_LINUX)
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
#elif defined(PLATFORM_WINDOWS)
    "C:\\Windows\\Fonts\\segoeui.ttf",
    "C:\\Windows\\Fonts\\arial.ttf",
#endif
    NULL,
};

/* Joystick button mapping — TrimUI raw values (firmware swaps A/B, X/Y) */
#define AP__JOY_BTN_A       1
#define AP__JOY_BTN_B       0
#define AP__JOY_BTN_X       3
#define AP__JOY_BTN_Y       2
#define AP__JOY_BTN_L1      4
#define AP__JOY_BTN_R1      5
#define AP__JOY_BTN_L2      10
#define AP__JOY_BTN_R2      11
#define AP__JOY_BTN_SELECT  6
#define AP__JOY_BTN_START   7
#define AP__JOY_BTN_MENU    8

/* TrimUI analog trigger axes (L2/R2 are axes, not buttons, on TG5040/TG5050) */
#define AP__JOY_AXIS_L2     2   /* ABS_Z   */
#define AP__JOY_AXIS_R2     5   /* ABS_RZ  */

/* my355 (Miyoo Flip) keyboard scancode mapping.
 * On the Flip, ALL buttons arrive as SDL keyboard scancodes, not joystick. */
#define AP__MY355_CODE_A       44   /* SDL_SCANCODE_SPACE */
#define AP__MY355_CODE_B       224  /* SDL_SCANCODE_LCTRL */
#define AP__MY355_CODE_X       225  /* SDL_SCANCODE_LSHIFT */
#define AP__MY355_CODE_Y       226  /* SDL_SCANCODE_LALT */
#define AP__MY355_CODE_UP      82   /* SDL_SCANCODE_UP */
#define AP__MY355_CODE_DOWN    81   /* SDL_SCANCODE_DOWN */
#define AP__MY355_CODE_LEFT    80   /* SDL_SCANCODE_LEFT */
#define AP__MY355_CODE_RIGHT   79   /* SDL_SCANCODE_RIGHT */
#define AP__MY355_CODE_START   40   /* SDL_SCANCODE_RETURN */
#define AP__MY355_CODE_SELECT  228  /* SDL_SCANCODE_RCTRL */
#define AP__MY355_CODE_L1      43   /* SDL_SCANCODE_TAB */
#define AP__MY355_CODE_R1      42   /* SDL_SCANCODE_BACKSLASH */
#define AP__MY355_CODE_L2      75   /* SDL_SCANCODE_PAGEUP */
#define AP__MY355_CODE_R2      78   /* SDL_SCANCODE_PAGEDOWN */
#define AP__MY355_CODE_MENU    41   /* SDL_SCANCODE_ESCAPE */
#define AP__MY355_CODE_POWER   102  /* SDL_SCANCODE_POWER */

/* Virtual button names */
static const char *ap__button_names[AP_BTN_COUNT] = {
    "NONE", "UP", "DOWN", "LEFT", "RIGHT",
    "A", "B", "X", "Y",
    "L1", "L2", "R1", "R2",
    "START", "SELECT", "MENU", "POWER"
};

/* ─── Logging ────────────────────────────────────────────────────────────── */

static bool ap__same_output_file(FILE *a, FILE *b) {
    if (!a || !b) return false;
#ifdef _WIN32
    int a_fd = _fileno(a);
    int b_fd = _fileno(b);
    if (a_fd < 0 || b_fd < 0) return false;
    return a_fd == b_fd;
#else
    int a_fd = fileno(a);
    int b_fd = fileno(b);
    if (a_fd < 0 || b_fd < 0) return false;

    struct stat a_st, b_st;
    if (fstat(a_fd, &a_st) != 0) return false;
    if (fstat(b_fd, &b_st) != 0) return false;
    return a_st.st_dev == b_st.st_dev && a_st.st_ino == b_st.st_ino;
#endif
}

void ap_log(const char *fmt, ...) {
    char buf[AP_MAX_LOG_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    /* Timestamp */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "%H:%M:%S", t);

    fprintf(stderr, "[%s] %s\n", ts, buf);
    if (ap__g.log_file && !ap__same_output_file(ap__g.log_file, stderr)) {
        fprintf(ap__g.log_file, "[%s] %s\n", ts, buf);
        fflush(ap__g.log_file);
    }
}

void ap_set_log_path(const char *path) {
    if (ap__g.log_file && ap__g.log_file != stderr) {
        fclose(ap__g.log_file);
        ap__g.log_file = NULL;
    }
    if (path) {
        ap__g.log_file = fopen(path, "a");
        if (!ap__g.log_file) {
            fprintf(stderr, "Warning: could not open log file: %s\n", path);
        }
    }
}

const char *ap_resolve_log_path(const char *app_name) {
    static char path[1024];
    if (!app_name || !app_name[0]) return NULL;

    const char *logs = getenv("LOGS_PATH");
    if (logs && logs[0]) {
        snprintf(path, sizeof(path), "%s/%s.txt", logs, app_name);
        return path;
    }

    const char *shared = getenv("SHARED_USERDATA_PATH");
    if (shared && shared[0]) {
        snprintf(path, sizeof(path), "%s/logs/%s.txt", shared, app_name);
        return path;
    }

    const char *home = getenv("HOME");
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.userdata/logs/%s.txt", home, app_name);
        return path;
    }

    return NULL;
}

/* ─── Error Handling ─────────────────────────────────────────────────────── */

const char *ap_get_error(void) {
    return ap__error_buf;
}

bool ap_is_cancelled(int result) {
    return result == AP_CANCELLED;
}

/* ─── Color Utilities ────────────────────────────────────────────────────── */

ap_color ap_hex_to_color(const char *hex) {
    ap_color c = {0, 0, 0, 255};
    if (!hex) return c;

    /* Skip "0x" or "#" prefix */
    if (hex[0] == '#') hex++;
    else if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) hex += 2;

    unsigned long val = strtoul(hex, NULL, 16);
    c.r = (uint8_t)((val >> 16) & 0xFF);
    c.g = (uint8_t)((val >>  8) & 0xFF);
    c.b = (uint8_t)( val        & 0xFF);
    c.a = 255;
    return c;
}

void ap_set_theme_color(const char *hex) {
    if (hex) {
        ap__g.theme.accent = ap_hex_to_color(hex);
    }
}

/* ─── Theme Loading ──────────────────────────────────────────────────────── */

ap_theme *ap_get_theme(void) {
    return &ap__g.theme;
}

/* Simple JSON string value extractor — finds "key": "value" */
static const char *ap__json_find_string(const char *json, const char *key) {
    static char value_buf[256];

    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *pos = strstr(json, search);
    if (!pos) return NULL;

    pos += strlen(search);
    /* Skip whitespace and colon */
    while (*pos && (*pos == ' ' || *pos == '\t' || *pos == ':')) pos++;

    if (*pos != '"') return NULL;
    pos++; /* skip opening quote */

    int i = 0;
    while (*pos && *pos != '"' && i < (int)sizeof(value_buf) - 1) {
        value_buf[i++] = *pos++;
    }
    value_buf[i] = '\0';
    return value_buf;
}

static bool ap__json_copy_string(const char *json, const char *key, char *out, size_t out_size) {
    if (!out || out_size == 0) return false;
    out[0] = '\0';
    const char *v = ap__json_find_string(json, key);
    if (!v) return false;
    size_t n = strlen(v);
    if (n >= out_size) n = out_size - 1;
    memcpy(out, v, n);
    out[n] = '\0';
    return true;
}

static bool ap__json_copy_background_string(const char *json, char *out, size_t out_size) {
    if (!out || out_size == 0) return false;
    out[0] = '\0';

    /* Prefer NextUI's current color7 key but accept legacy bgcolor for compatibility. */
    if (ap__json_copy_string(json, "color7", out, out_size)) return true;
    return ap__json_copy_string(json, "bgcolor", out, out_size);
}

int ap_theme_load_nextui(void) {
#if AP_PLATFORM_IS_DEVICE
    /* Look for nextval.elf — prefer SYSTEM_PATH env var, fall back to hardcoded path */
    const char *nextval_path = NULL;
    char nextval_env_buf[256] = {0};
    const char *system_path_env = getenv("SYSTEM_PATH");
    if (system_path_env && system_path_env[0]) {
        snprintf(nextval_env_buf, sizeof(nextval_env_buf), "%s/bin/nextval.elf", system_path_env);
        if (access(nextval_env_buf, X_OK) == 0) nextval_path = nextval_env_buf;
    }
    if (!nextval_path) {
    #if defined(PLATFORM_TG5040)
        if (access("/mnt/SDCARD/.system/tg5040/bin/nextval.elf", X_OK) == 0)
            nextval_path = "/mnt/SDCARD/.system/tg5040/bin/nextval.elf";
    #elif defined(PLATFORM_TG5050)
        if (access("/mnt/SDCARD/.system/tg5050/bin/nextval.elf", X_OK) == 0)
            nextval_path = "/mnt/SDCARD/.system/tg5050/bin/nextval.elf";
    #elif defined(PLATFORM_MY355)
        if (access("/mnt/SDCARD/.system/my355/bin/nextval.elf", X_OK) == 0)
            nextval_path = "/mnt/SDCARD/.system/my355/bin/nextval.elf";
    #endif
    }

    if (!nextval_path) {
        ap_log("nextval.elf not found, using default theme");
        return AP_ERROR;
    }

    FILE *fp = popen(nextval_path, "r");
    if (!fp) {
        ap_log("Failed to run nextval.elf");
        return AP_ERROR;
    }

    char json[4096] = {0};
    size_t total = 0;
    while (total < sizeof(json) - 1) {
        size_t n = fread(json + total, 1, sizeof(json) - 1 - total, fp);
        if (n == 0) break;
        total += n;
    }
    json[total] = '\0';
    pclose(fp);

    char c1_buf[32]={0}, c2_buf[32]={0}, c3_buf[32]={0}, c4_buf[32]={0};
    char c5_buf[32]={0}, c6_buf[32]={0}, bg_buf[32]={0};
    ap__json_copy_string(json, "color1", c1_buf, sizeof(c1_buf));
    ap__json_copy_string(json, "color2", c2_buf, sizeof(c2_buf));
    ap__json_copy_string(json, "color3", c3_buf, sizeof(c3_buf));
    ap__json_copy_string(json, "color4", c4_buf, sizeof(c4_buf));
    ap__json_copy_string(json, "color5", c5_buf, sizeof(c5_buf));
    ap__json_copy_string(json, "color6", c6_buf, sizeof(c6_buf));
    ap__json_copy_background_string(json, bg_buf, sizeof(bg_buf));

    if (c1_buf[0]) ap__g.theme.highlight        = ap_hex_to_color(c1_buf);
    if (c2_buf[0]) ap__g.theme.accent           = ap_hex_to_color(c2_buf);
    if (c3_buf[0]) ap__g.theme.button_label     = ap_hex_to_color(c3_buf);
    if (c4_buf[0]) ap__g.theme.text             = ap_hex_to_color(c4_buf);
    if (c5_buf[0]) ap__g.theme.highlighted_text = ap_hex_to_color(c5_buf);
    if (c6_buf[0]) ap__g.theme.hint             = ap_hex_to_color(c6_buf);
    if (bg_buf[0]) ap__g.theme.background       = ap_hex_to_color(bg_buf);

    ap_log("Loaded NextUI theme (accent: #%02X%02X%02X)",
           ap__g.theme.accent.r, ap__g.theme.accent.g, ap__g.theme.accent.b);
    return AP_OK;

#else
    /* Dev mode: check for NEXTVAL_PATH env var pointing to a JSON file */
    const char *path = getenv("AP_NEXTVAL_PATH");
    if (!path) {
        ap_log("No AP_NEXTVAL_PATH set, using default theme");
        return AP_ERROR;
    }

    FILE *fp = fopen(path, "r");
    if (!fp) {
        ap_log("Could not open nextval file: %s", path);
        return AP_ERROR;
    }

    char json[4096] = {0};
    size_t nread = fread(json, 1, sizeof(json) - 1, fp);
    fclose(fp);
    if (nread == 0) {
        ap_log("Theme file empty or unreadable: %s", path);
        return AP_ERROR;
    }
    json[nread] = '\0';

    char c1_buf[32]={0}, c2_buf[32]={0}, c3_buf[32]={0}, c4_buf[32]={0};
    char c5_buf[32]={0}, c6_buf[32]={0}, bg_buf[32]={0};
    ap__json_copy_string(json, "color1", c1_buf, sizeof(c1_buf));
    ap__json_copy_string(json, "color2", c2_buf, sizeof(c2_buf));
    ap__json_copy_string(json, "color3", c3_buf, sizeof(c3_buf));
    ap__json_copy_string(json, "color4", c4_buf, sizeof(c4_buf));
    ap__json_copy_string(json, "color5", c5_buf, sizeof(c5_buf));
    ap__json_copy_string(json, "color6", c6_buf, sizeof(c6_buf));
    ap__json_copy_background_string(json, bg_buf, sizeof(bg_buf));

    if (c1_buf[0]) ap__g.theme.highlight        = ap_hex_to_color(c1_buf);
    if (c2_buf[0]) ap__g.theme.accent           = ap_hex_to_color(c2_buf);
    if (c3_buf[0]) ap__g.theme.button_label     = ap_hex_to_color(c3_buf);
    if (c4_buf[0]) ap__g.theme.text             = ap_hex_to_color(c4_buf);
    if (c5_buf[0]) ap__g.theme.highlighted_text = ap_hex_to_color(c5_buf);
    if (c6_buf[0]) ap__g.theme.hint             = ap_hex_to_color(c6_buf);
    if (bg_buf[0]) ap__g.theme.background       = ap_hex_to_color(bg_buf);

    ap_log("Loaded theme from: %s", path);
    return AP_OK;
#endif
}

int ap_reload_background(const char *bg_path) {
    const char *resolved = bg_path;

    if (!resolved || !resolved[0]) {
    #if AP_PLATFORM_IS_DEVICE
        resolved = "/mnt/SDCARD/bg.png";
    #else
        resolved = getenv("AP_BACKGROUND_PATH");
    #endif
    }

    if (!resolved || !resolved[0]) {
        return AP_OK;
    }

    SDL_Texture *new_tex = ap_load_image(resolved);
    if (!new_tex) {
        ap_log("Warning: could not reload background: %s", resolved);
        return AP_ERROR;
    }

    if (ap__g.bg_texture) {
        SDL_DestroyTexture(ap__g.bg_texture);
    }
    ap__g.bg_texture = new_tex;

    strncpy(ap__g.theme.bg_image_path, resolved, sizeof(ap__g.theme.bg_image_path) - 1);
    ap__g.theme.bg_image_path[sizeof(ap__g.theme.bg_image_path) - 1] = '\0';
    ap_log("Reloaded background: %s", resolved);
    return AP_OK;
}

/* ─── Scaling ────────────────────────────────────────────────────────────── */

float ap_get_scale_factor(void) {
    return ap__g.scale_factor;
}

int ap_scale(int base) {
    return (int)(base * ap__g.scale_factor);
}

int ap_font_size_for_resolution(int base_size) {
    int scale = ap__g.device_scale ? ap__g.device_scale : 2;
    return base_size * scale;
}

static void ap__compute_scale_factor(void) {
    float raw = (float)ap__g.screen_w / (float)AP_REFERENCE_WIDTH;
    if (raw > 1.0f)
        ap__g.scale_factor = 1.0f + (raw - 1.0f) * AP_SCALE_DAMPING;
    else
        ap__g.scale_factor = raw;
}

static void ap__resolve_device_metrics(void) {
#if defined(PLATFORM_TG5040) || !AP_PLATFORM_IS_DEVICE
    /* Match TG5040 preview profiles by resolution: Brick 1024×768 uses 3/5,
       handheld-style layouts use 2/10. Desktop previews follow the same rule. */
    if (ap__g.screen_w <= 1024 && ap__g.screen_h >= 768) {
        ap__g.device_scale = 3;
        ap__g.device_padding = 5;
    } else {
        ap__g.device_scale = 2;
        ap__g.device_padding = 10;
    }
#elif defined(PLATFORM_TG5050)
    ap__g.device_scale = 2;
    ap__g.device_padding = 10;
#elif defined(PLATFORM_MY355)
    ap__g.device_scale = 2;
    ap__g.device_padding = 10;
#else
    ap__g.device_scale = 2;
    ap__g.device_padding = 10;
#endif
}

/* ─── NextUI settings reader ─────────────────────────────────────────────── */

static const char *ap__nextui_settings_path(char *buf, size_t buf_size) {
#if AP_PLATFORM_IS_DEVICE
    const char *shared = getenv("SHARED_USERDATA_PATH");
    if (!shared || !shared[0]) shared = "/mnt/SDCARD/.userdata/shared";
    snprintf(buf, buf_size, "%s/minuisettings.txt", shared);
    return buf;
#else
    (void)buf;
    (void)buf_size;
    {
        const char *path = getenv("AP_MINUI_SETTINGS_PATH");
        return (path && path[0]) ? path : NULL;
    }
#endif
}

/* Read an integer setting from NextUI's minuisettings.txt or a desktop preview
   settings file. Returns the value, or default_val if key is not found. */
static int ap__read_nextui_setting_int(const char *key, int default_val) {
    char settings_path[256];
    const char *path = ap__nextui_settings_path(settings_path, sizeof(settings_path));
    if (!path) return default_val;

    FILE *f = fopen(path, "r");
    if (!f) return default_val;
    char line[256];
    size_t klen = strlen(key);
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, key, klen) == 0 && line[klen] == '=') {
            int val = 0;
            if (sscanf(line + klen + 1, "%d", &val) == 1) {
                fclose(f);
                return val;
            }
        }
    }
    fclose(f);
    return default_val;
}

/* ─── Font Management ────────────────────────────────────────────────────── */

static int ap__compute_font_bump(void) {
    int ds = ap__g.device_scale ? ap__g.device_scale : 2;
    int logical_w = ap__g.screen_w / ds;
    int logical_h = ap__g.screen_h / ds;
    int bump_w = ((logical_w - AP_FONT_BUMP_REF_LOGICAL_W) * AP_FONT_BUMP_MAX)
                 / AP_FONT_BUMP_REF_LOGICAL_W;
    int bump_h = ((logical_h - AP_FONT_BUMP_REF_LOGICAL_H) * AP_FONT_BUMP_MAX)
                 / AP_FONT_BUMP_REF_LOGICAL_H;
    int bump = (bump_w < bump_h) ? bump_w : bump_h;
    return ap__clamp(bump, 0, AP_FONT_BUMP_MAX);
}

static TTF_Font *ap__open_font(const char *path, int size) {
    TTF_Font *f = TTF_OpenFont(path, size);
    if (f) TTF_SetFontStyle(f, TTF_STYLE_BOLD); /* match NextUI: all fonts loaded bold */
    return f;
}

static int ap__load_fonts(const char *user_font_path) {
    /* Determine font path */
    const char *font_path = NULL;

    /* 1. User-specified path (allows font override via ap_config.font_path) */
    if (user_font_path && user_font_path[0]) {
        if (access(user_font_path, R_OK) == 0) {
            font_path = user_font_path;
        }
    }

    /* 2. On device, respect NextUI font setting (font=1 → font1.ttf, font=2 → font2.ttf) */
    #if AP_PLATFORM_IS_DEVICE
    if (!font_path) {
        static char nextui_font_buf[256];
        int font_id = ap__read_nextui_setting_int("font", 1);
        {
            const char *sdcard = getenv("SDCARD_PATH");
            if (!sdcard || !sdcard[0]) sdcard = "/mnt/SDCARD";
            snprintf(nextui_font_buf, sizeof(nextui_font_buf),
                     "%s/.system/res/font%d.ttf", sdcard, font_id);
        }
        if (access(nextui_font_buf, R_OK) == 0) {
            font_path = nextui_font_buf;
        }
    }
    #endif

    /* 3. Search fallback paths */
    if (!font_path) {
        for (int i = 0; ap__font_search_paths[i]; i++) {
            if (access(ap__font_search_paths[i], R_OK) == 0) {
                font_path = ap__font_search_paths[i];
                break;
            }
        }
    }

    if (!font_path) {
        ap__set_error("No font file found");
        ap_log("ERROR: No font file found in any search path");
        return AP_ERROR;
    }

    /* Store in theme */
    strncpy(ap__g.theme.font_path, font_path, sizeof(ap__g.theme.font_path) - 1);
    ap_log("Loading font: %s", font_path);

    /* Open at each tier size (base + font_bump, then × device_scale) */
    for (int i = 0; i < AP_FONT_TIER_COUNT; i++) {
        int size = ap_font_size_for_resolution(ap__font_base_sizes[i] + ap__g.font_bump);
        if (size < 8) size = 8;
        ap__g.fonts[i] = ap__open_font(font_path, size);
        if (!ap__g.fonts[i]) {
            ap__set_error("Failed to open font at size %d: %s", size, TTF_GetError());
            ap_log("ERROR: Failed to open font tier %d (size %d): %s", i, size, TTF_GetError());
            return AP_ERROR;
        }
    }

    return AP_OK;
}

TTF_Font *ap_get_font(ap_font_tier tier) {
    if (tier < 0 || tier >= AP_FONT_TIER_COUNT) return ap__g.fonts[AP_FONT_SMALL];
    return ap__g.fonts[tier];
}

int ap_get_font_bump(void) {
    return ap__g.font_bump;
}

/* ─── Input System ───────────────────────────────────────────────────────── */

/* Map SDL joystick button to virtual button (raw joystick — used on TrimUI) */
#if !defined(PLATFORM_MY355)
static ap_button ap__map_joy_button(uint8_t btn) {
    if (ap__g.face_buttons_flipped) {
        if (btn == AP__JOY_BTN_A) return AP_BTN_B;
        if (btn == AP__JOY_BTN_B) return AP_BTN_A;
    }
    switch (btn) {
        case AP__JOY_BTN_A:      return AP_BTN_A;
        case AP__JOY_BTN_B:      return AP_BTN_B;
        case AP__JOY_BTN_X:      return AP_BTN_X;
        case AP__JOY_BTN_Y:      return AP_BTN_Y;
        case AP__JOY_BTN_L1:     return AP_BTN_L1;
        case AP__JOY_BTN_R1:     return AP_BTN_R1;
        case AP__JOY_BTN_L2:     return AP_BTN_L2;
        case AP__JOY_BTN_R2:     return AP_BTN_R2;
        case AP__JOY_BTN_SELECT: return AP_BTN_SELECT;
        case AP__JOY_BTN_START:  return AP_BTN_START;
        case AP__JOY_BTN_MENU:   return AP_BTN_MENU;
        default:                 return AP_BTN_NONE;
    }
}
#endif

/* Map SDL GameController button to virtual button (used on macOS / when SDL
 * recognises the device as a standard game controller) */
#if !AP_PLATFORM_IS_DEVICE
static ap_button ap__map_controller_button(uint8_t btn) {
    ap_button mapped = AP_BTN_NONE;
    switch (btn) {
        case SDL_CONTROLLER_BUTTON_A:             mapped = AP_BTN_A;      break;
        case SDL_CONTROLLER_BUTTON_B:             mapped = AP_BTN_B;      break;
        case SDL_CONTROLLER_BUTTON_X:             mapped = AP_BTN_X;      break;
        case SDL_CONTROLLER_BUTTON_Y:             mapped = AP_BTN_Y;      break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  mapped = AP_BTN_L1;     break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: mapped = AP_BTN_R1;     break;
        case SDL_CONTROLLER_BUTTON_BACK:          mapped = AP_BTN_SELECT; break;
        case SDL_CONTROLLER_BUTTON_START:         mapped = AP_BTN_START;  break;
        case SDL_CONTROLLER_BUTTON_GUIDE:         mapped = AP_BTN_MENU;   break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:       mapped = AP_BTN_UP;     break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:     mapped = AP_BTN_DOWN;   break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:     mapped = AP_BTN_LEFT;   break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:    mapped = AP_BTN_RIGHT;  break;
        default: break;
    }
    /* Apply face-button flip (Nintendo-style A↔B, X↔Y) */
    if (ap__g.face_buttons_flipped) {
        if (mapped == AP_BTN_A) return AP_BTN_B;
        if (mapped == AP_BTN_B) return AP_BTN_A;
        if (mapped == AP_BTN_X) return AP_BTN_Y;
        if (mapped == AP_BTN_Y) return AP_BTN_X;
    }
    return mapped;
}
#endif

/* Map SDL keyboard to virtual button.
 * On my355 we match by scancode (the Flip sends buttons as keyboard HID scancodes).
 * On all other platforms we match by keycode for developer convenience. */
#if defined(PLATFORM_MY355)
static ap_button ap__map_key_event(SDL_KeyboardEvent *kev) {
    uint8_t sc = (uint8_t)kev->keysym.scancode;
    ap_button mapped = AP_BTN_NONE;
    switch (sc) {
        case AP__MY355_CODE_UP:     mapped = AP_BTN_UP;     break;
        case AP__MY355_CODE_DOWN:   mapped = AP_BTN_DOWN;   break;
        case AP__MY355_CODE_LEFT:   mapped = AP_BTN_LEFT;   break;
        case AP__MY355_CODE_RIGHT:  mapped = AP_BTN_RIGHT;  break;
        case AP__MY355_CODE_A:      mapped = AP_BTN_A;      break;
        case AP__MY355_CODE_B:      mapped = AP_BTN_B;      break;
        case AP__MY355_CODE_X:      mapped = AP_BTN_X;      break;
        case AP__MY355_CODE_Y:      mapped = AP_BTN_Y;      break;
        case AP__MY355_CODE_L1:     mapped = AP_BTN_L1;     break;
        case AP__MY355_CODE_R1:     mapped = AP_BTN_R1;     break;
        case AP__MY355_CODE_L2:     mapped = AP_BTN_L2;     break;
        case AP__MY355_CODE_R2:     mapped = AP_BTN_R2;     break;
        case AP__MY355_CODE_START:  mapped = AP_BTN_START;  break;
        case AP__MY355_CODE_SELECT: mapped = AP_BTN_SELECT; break;
        case AP__MY355_CODE_MENU:   mapped = AP_BTN_MENU;   break;
        case AP__MY355_CODE_POWER:  mapped = AP_BTN_POWER;  break;
        default: break;
    }
    if (ap__g.face_buttons_flipped) {
        if (mapped == AP_BTN_A) return AP_BTN_B;
        if (mapped == AP_BTN_B) return AP_BTN_A;
        if (mapped == AP_BTN_X) return AP_BTN_Y;
        if (mapped == AP_BTN_Y) return AP_BTN_X;
    }
    return mapped;
}
#else
static ap_button ap__map_key_event(SDL_KeyboardEvent *kev) {
    /* Match Gabagool DefaultInputMapping() — letter keys for face buttons */
    ap_button mapped = AP_BTN_NONE;
    switch (kev->keysym.sym) {
        case SDLK_UP:        mapped = AP_BTN_UP;     break;
        case SDLK_DOWN:      mapped = AP_BTN_DOWN;   break;
        case SDLK_LEFT:      mapped = AP_BTN_LEFT;   break;
        case SDLK_RIGHT:     mapped = AP_BTN_RIGHT;  break;
        case SDLK_a:         mapped = AP_BTN_A;      break;
        case SDLK_b:         mapped = AP_BTN_B;      break;
        case SDLK_x:         mapped = AP_BTN_X;      break;
        case SDLK_y:         mapped = AP_BTN_Y;      break;
        case SDLK_l:         mapped = AP_BTN_L1;     break;
        case SDLK_SEMICOLON: mapped = AP_BTN_L2;     break;
        case SDLK_r:         mapped = AP_BTN_R1;     break;
        case SDLK_t:         mapped = AP_BTN_R2;     break;
        case SDLK_RETURN:    mapped = AP_BTN_START;  break;
        case SDLK_SPACE:     mapped = AP_BTN_SELECT; break;
        case SDLK_h:         mapped = AP_BTN_MENU;   break;
        default: break;
    }
    if (ap__g.face_buttons_flipped) {
        if (mapped == AP_BTN_A) return AP_BTN_B;
        if (mapped == AP_BTN_B) return AP_BTN_A;
        if (mapped == AP_BTN_X) return AP_BTN_Y;
        if (mapped == AP_BTN_Y) return AP_BTN_X;
    }
    return mapped;
}
#endif

/* Internal input event buffer */
static ap_input_event ap__input_queue[64];
static int ap__input_head = 0;
static int ap__input_tail = 0;

static void ap__footer_overflow_show_hidden_actions(void);
static bool ap__footer_overflow_consume_open_request(void);

static void ap__footer_overflow_clear_visible_state(void) {
    ap__g.footer_overflow_active = false;
    ap__g.footer_hidden_count = 0;
}

static void ap__footer_overflow_begin_frame(void) {
    if (ap__g.footer_overflow_overlay_open) return;
    ap__footer_overflow_clear_visible_state();
}

static bool ap__footer_overflow_enabled(void) {
    if (!ap__g.footer_overflow_opts.enabled) return false;
    if (ap__g.footer_overflow_overlay_open) return false;
    if (!ap__g.footer_overflow_active || ap__g.footer_hidden_count <= 0) return false;
    if (ap__g.footer_overflow_opts.chord_a == AP_BTN_NONE ||
        ap__g.footer_overflow_opts.chord_b == AP_BTN_NONE) {
        return false;
    }
    if (ap__g.footer_overflow_opts.chord_a == ap__g.footer_overflow_opts.chord_b) return false;
    return true;
}

static void ap__input_remove_buttons(ap_button a, ap_button b) {
    ap_input_event filtered[64];
    int filtered_count = 0;
    int idx = ap__input_tail;

    while (idx != ap__input_head && filtered_count < 64) {
        ap_input_event ev = ap__input_queue[idx];
        idx = (idx + 1) % 64;
        if (!ev.repeated && (ev.button == a || ev.button == b)) continue;
        filtered[filtered_count++] = ev;
    }

    ap__input_tail = 0;
    ap__input_head = filtered_count % 64;
    for (int i = 0; i < filtered_count; i++) {
        ap__input_queue[i] = filtered[i];
    }
}

static bool ap__footer_overflow_note_button(ap_button btn, bool pressed) {
    if (btn == AP_BTN_NONE) return false;

    if (!pressed && ap__g.footer_overflow_swallow[btn]) {
        ap__g.footer_overflow_swallow[btn] = false;
        if (btn == ap__g.footer_overflow_opts.chord_a || btn == ap__g.footer_overflow_opts.chord_b) {
            if (!ap__g.buttons_held[ap__g.footer_overflow_opts.chord_a] &&
                !ap__g.buttons_held[ap__g.footer_overflow_opts.chord_b]) {
                ap__g.footer_overflow_chord_held = false;
            }
        }
        return true;
    }

    if (!pressed) {
        if ((btn == ap__g.footer_overflow_opts.chord_a || btn == ap__g.footer_overflow_opts.chord_b) &&
            !ap__g.buttons_held[ap__g.footer_overflow_opts.chord_a] &&
            !ap__g.buttons_held[ap__g.footer_overflow_opts.chord_b]) {
            ap__g.footer_overflow_chord_held = false;
        }
        return false;
    }

    if (!ap__footer_overflow_enabled() || ap__g.footer_overflow_chord_held) return false;
    if (btn != ap__g.footer_overflow_opts.chord_a && btn != ap__g.footer_overflow_opts.chord_b) return false;

    if (ap__g.buttons_held[ap__g.footer_overflow_opts.chord_a] &&
        ap__g.buttons_held[ap__g.footer_overflow_opts.chord_b]) {
        ap__g.footer_overflow_chord_held = true;
        ap__g.footer_overflow_open_requested = true;
        ap__g.footer_overflow_swallow[ap__g.footer_overflow_opts.chord_a] = true;
        ap__g.footer_overflow_swallow[ap__g.footer_overflow_opts.chord_b] = true;
        ap__input_remove_buttons(ap__g.footer_overflow_opts.chord_a,
                                 ap__g.footer_overflow_opts.chord_b);
        return true;
    }

    return false;
}

/* ─── Combo Detection Helpers ────────────────────────────────────────────── */

static void ap__combo_push_event(ap_combo *c, bool triggered, ap_combo_type type) {
    /* Fire callback if registered */
    if (triggered && c->on_trigger)
        c->on_trigger(c->id, type, c->userdata);
    else if (!triggered && c->on_release)
        c->on_release(c->id, type, c->userdata);

    /* Enqueue for polling */
    int next = (ap__g.combo_queue_head + 1) % 16;
    if (next == ap__g.combo_queue_tail) return; /* queue full */
    ap__g.combo_queue[ap__g.combo_queue_head].id = c->id;
    ap__g.combo_queue[ap__g.combo_queue_head].triggered = triggered;
    ap__g.combo_queue[ap__g.combo_queue_head].type = type;
    ap__g.combo_queue_head = next;
}

static void ap__combo_add_seq_entry(ap_button btn, uint32_t now) {
    if (ap__g.seq_buffer_count >= 20) {
        memmove(&ap__g.seq_buffer[0], &ap__g.seq_buffer[1],
                19 * sizeof(ap__seq_entry));
        ap__g.seq_buffer_count = 19;
    }
    ap__g.seq_buffer[ap__g.seq_buffer_count].button = btn;
    ap__g.seq_buffer[ap__g.seq_buffer_count].time_ms = now;
    ap__g.seq_buffer_count++;
}

static void ap__combo_check_chords(uint32_t now) {
    (void)now;
    for (int i = 0; i < ap__g.combo_count; i++) {
        ap_combo *c = &ap__g.combos[i];
        if (!c->active || c->is_sequence || ap__g.combo_held[i]) continue;

        bool all_pressed = true;
        uint32_t earliest = UINT32_MAX;
        uint32_t latest = 0;

        for (int j = 0; j < c->button_count; j++) {
            ap_button btn = c->buttons[j];
            if (!ap__g.buttons_held[btn]) {
                all_pressed = false;
                break;
            }
            uint32_t t = ap__g.button_press_time[btn];
            if (t < earliest) earliest = t;
            if (t > latest) latest = t;
        }

        if (all_pressed && (latest - earliest) <= c->window_ms) {
            ap__g.combo_held[i] = true;
            ap__combo_push_event(c, true, AP_COMBO_CHORD);
        }
    }
}

static void ap__combo_check_chord_releases(void) {
    for (int i = 0; i < ap__g.combo_count; i++) {
        ap_combo *c = &ap__g.combos[i];
        if (!c->active || c->is_sequence || !ap__g.combo_held[i]) continue;

        for (int j = 0; j < c->button_count; j++) {
            if (!ap__g.buttons_held[c->buttons[j]]) {
                ap__g.combo_held[i] = false;
                ap__combo_push_event(c, false, AP_COMBO_CHORD);
                break;
            }
        }
    }
}

static void ap__combo_check_sequences(void) {
    for (int i = 0; i < ap__g.combo_count; i++) {
        ap_combo *c = &ap__g.combos[i];
        if (!c->active || !c->is_sequence) continue;
        if (ap__g.seq_buffer_count < c->button_count) continue;

        int start = ap__g.seq_buffer_count - c->button_count;
        bool match = true;

        for (int j = 0; j < c->button_count; j++) {
            ap__seq_entry *entry = &ap__g.seq_buffer[start + j];
            if (entry->button != c->buttons[j]) {
                match = false;
                break;
            }
            if (j > 0) {
                ap__seq_entry *prev = &ap__g.seq_buffer[start + j - 1];
                if (entry->time_ms - prev->time_ms > c->window_ms) {
                    match = false;
                    break;
                }
            }
        }

        /* Strict mode: reject if an extraneous button was pressed just before
         * the matched range but within the same time window.  Buffer entries
         * are chronological, so checking only the entry immediately before the
         * matched range is sufficient — any earlier entry has an equal or
         * earlier timestamp. */
        if (match && c->strict && start > 0) {
            uint32_t seq_start_time = ap__g.seq_buffer[start].time_ms;
            if (ap__g.seq_buffer[start - 1].time_ms >= seq_start_time) {
                match = false;
            }
        }

        if (match) {
            ap__combo_push_event(c, true, AP_COMBO_SEQUENCE);
            ap__g.seq_buffer_count = 0;
            return;
        }
    }
}

/* ─── Input Queue ────────────────────────────────────────────────────────── */

static void ap__input_push(ap_button btn, bool pressed) {
    if (btn == AP_BTN_NONE) return;

    uint32_t now = SDL_GetTicks();
    bool suppress_event = false;

    /* Combo detection — only on real presses/releases, not auto-repeats */
    if (pressed && !ap__g.buttons_held[btn]) {
        ap__g.buttons_held[btn] = true;
        ap__g.button_press_time[btn] = now;
        ap__combo_add_seq_entry(btn, now);
        ap__combo_check_chords(now);
        ap__combo_check_sequences();
    } else if (!pressed && ap__g.buttons_held[btn]) {
        ap__g.buttons_held[btn] = false;
        ap__combo_check_chord_releases();
    }

    suppress_event = ap__footer_overflow_note_button(btn, pressed);
    if (suppress_event) return;

    /* Push to input queue */
    int next = (ap__input_head + 1) % 64;
    if (next == ap__input_tail) return; /* queue full */
    ap__input_queue[ap__input_head] = (ap_input_event){ btn, pressed, false };
    ap__input_head = next;
    ap__g.needs_frame = true;
}

static bool ap__is_direction_button(ap_button btn) {
    return btn == AP_BTN_UP || btn == AP_BTN_DOWN ||
           btn == AP_BTN_LEFT || btn == AP_BTN_RIGHT;
}

static void ap__arm_direction_repeat(ap_button btn, uint32_t now) {
    if (!ap__is_direction_button(btn)) return;
    ap__g.button_repeat_time[btn] = now + ap__g.input_repeat_delay_ms;
}

static void ap__advance_repeat_deadline(uint32_t *deadline, uint32_t now) {
    if (!deadline) return;
    if (*deadline == 0) {
        *deadline = now + ap__g.input_repeat_rate_ms;
    } else {
        *deadline += ap__g.input_repeat_rate_ms;
    }
}

static void ap__advance_direction_repeat(ap_button btn, uint32_t now) {
    if (!ap__is_direction_button(btn)) return;
    ap__advance_repeat_deadline(&ap__g.button_repeat_time[btn], now);
}

static void ap__clear_direction_repeat(ap_button btn) {
    if (!ap__is_direction_button(btn)) return;
    ap__g.button_repeat_time[btn] = 0;
}

static void ap__push_button_press(ap_button btn, uint32_t now) {
    ap__arm_direction_repeat(btn, now);
    ap__input_push(btn, true);
}

static void ap__push_button_release(ap_button btn) {
    ap__clear_direction_repeat(btn);
    ap__input_push(btn, false);
}

static bool ap__repeat_direction(ap_button btn, uint32_t now) {
    if (!ap__is_direction_button(btn)) return false;
    int next = (ap__input_head + 1) % 64;
    if (next != ap__input_tail) {
        ap__input_queue[ap__input_head] = (ap_input_event){ btn, true, true };
        ap__input_head = next;
        ap__g.needs_frame = true;
    }
    ap__advance_direction_repeat(btn, now);
    return true;
}

#if !defined(PLATFORM_MY355)
static void ap__set_hat_state(uint8_t hat, uint32_t now) {
    uint8_t prev = ap__g.hat_held;

    if (!(hat & SDL_HAT_UP) && (prev & SDL_HAT_UP))
        ap__push_button_release(AP_BTN_UP);
    if (!(hat & SDL_HAT_DOWN) && (prev & SDL_HAT_DOWN))
        ap__push_button_release(AP_BTN_DOWN);
    if (!(hat & SDL_HAT_LEFT) && (prev & SDL_HAT_LEFT))
        ap__push_button_release(AP_BTN_LEFT);
    if (!(hat & SDL_HAT_RIGHT) && (prev & SDL_HAT_RIGHT))
        ap__push_button_release(AP_BTN_RIGHT);

    if ((hat & SDL_HAT_UP) && !(prev & SDL_HAT_UP))
        ap__push_button_press(AP_BTN_UP, now);
    if ((hat & SDL_HAT_DOWN) && !(prev & SDL_HAT_DOWN))
        ap__push_button_press(AP_BTN_DOWN, now);
    if ((hat & SDL_HAT_LEFT) && !(prev & SDL_HAT_LEFT))
        ap__push_button_press(AP_BTN_LEFT, now);
    if ((hat & SDL_HAT_RIGHT) && !(prev & SDL_HAT_RIGHT))
        ap__push_button_press(AP_BTN_RIGHT, now);

    ap__g.hat_held = hat;
    ap__g.hat_repeat_time = hat ? (now + ap__g.input_repeat_delay_ms) : 0;
}
#endif

static void ap__set_axis_direction_y(int dir, uint32_t now) {
    if (ap__g.axis_held_dir_y == dir) return;

    if (ap__g.axis_held_dir_y == -1) {
        ap__push_button_release(AP_BTN_UP);
    } else if (ap__g.axis_held_dir_y == 1) {
        ap__push_button_release(AP_BTN_DOWN);
    }

    ap__g.axis_held_dir_y = dir;
    if (dir < 0) {
        ap__push_button_press(AP_BTN_UP, now);
        ap__g.axis_repeat_time_y = now + ap__g.input_repeat_delay_ms;
    } else if (dir > 0) {
        ap__push_button_press(AP_BTN_DOWN, now);
        ap__g.axis_repeat_time_y = now + ap__g.input_repeat_delay_ms;
    } else {
        ap__g.axis_repeat_time_y = 0;
    }
}

static void ap__set_axis_direction_x(int dir, uint32_t now) {
    if (ap__g.axis_held_dir_x == dir) return;

    if (ap__g.axis_held_dir_x == -1) {
        ap__push_button_release(AP_BTN_LEFT);
    } else if (ap__g.axis_held_dir_x == 1) {
        ap__push_button_release(AP_BTN_RIGHT);
    }

    ap__g.axis_held_dir_x = dir;
    if (dir < 0) {
        ap__push_button_press(AP_BTN_LEFT, now);
        ap__g.axis_repeat_time_x = now + ap__g.input_repeat_delay_ms;
    } else if (dir > 0) {
        ap__push_button_press(AP_BTN_RIGHT, now);
        ap__g.axis_repeat_time_x = now + ap__g.input_repeat_delay_ms;
    } else {
        ap__g.axis_repeat_time_x = 0;
    }
}

static void ap__handle_sdl_event(SDL_Event *ev, uint32_t now) {
    switch (ev->type) {
        case SDL_QUIT:
            ap__input_push(AP_BTN_B, true);
            break;

        case SDL_USEREVENT:
            break; /* wake-up only — no action needed */

        case SDL_KEYDOWN:
            if (!ev->key.repeat) {
                ap_button b = ap__map_key_event(&ev->key);
                ap__push_button_press(b, now);
            }
            break;

        case SDL_KEYUP: {
            ap_button b = ap__map_key_event(&ev->key);
            ap__push_button_release(b);
            break;
        }

        /* --- Raw Joystick button/hat events (TrimUI devices) ---
           MY355 sends buttons and d-pad as keyboard scancodes; processing
           joystick button/hat events too would cause double-input.
           When a GameController is active (macOS), skip raw joystick
           button/hat events — the GameController API already maps them
           correctly, and the raw mappings differ, causing phantom inputs.
           Axis events (thumbstick) are allowed through on all platforms. */
        #if !defined(PLATFORM_MY355)
        case SDL_JOYBUTTONDOWN: {
            if (ap__g.controller) break; /* GameController handles this */
            ap_button b = ap__map_joy_button(ev->jbutton.button);
            ap__push_button_press(b, now);
            break;
        }

        case SDL_JOYBUTTONUP: {
            if (ap__g.controller) break; /* GameController handles this */
            ap_button b = ap__map_joy_button(ev->jbutton.button);
            ap__push_button_release(b);
            break;
        }

        /* --- SDL GameController events (macOS / recognised controllers) --- */
        #if !AP_PLATFORM_IS_DEVICE
        case SDL_CONTROLLERBUTTONDOWN: {
            ap_button b = ap__map_controller_button(ev->cbutton.button);
            ap__push_button_press(b, now);
            break;
        }

        case SDL_CONTROLLERBUTTONUP: {
            ap_button b = ap__map_controller_button(ev->cbutton.button);
            ap__push_button_release(b);
            break;
        }

        case SDL_CONTROLLERAXISMOTION: {
            /* Map left analog stick to d-pad via GameController axis */
            if (ev->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                if (ev->caxis.value < -AP_AXIS_DEADZONE) {
                    ap__set_axis_direction_y(-1, now);
                } else if (ev->caxis.value > AP_AXIS_DEADZONE) {
                    ap__set_axis_direction_y(1, now);
                } else {
                    ap__set_axis_direction_y(0, now);
                }
            } else if (ev->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                if (ev->caxis.value < -AP_AXIS_DEADZONE) {
                    ap__set_axis_direction_x(-1, now);
                } else if (ev->caxis.value > AP_AXIS_DEADZONE) {
                    ap__set_axis_direction_x(1, now);
                } else {
                    ap__set_axis_direction_x(0, now);
                }
            } else if (ev->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                if (ev->caxis.value > AP_AXIS_DEADZONE) {
                    if (!ap__g.buttons_held[AP_BTN_L2]) {
                        ap__input_push(AP_BTN_L2, true);
                    }
                } else if (ap__g.buttons_held[AP_BTN_L2]) {
                    ap__input_push(AP_BTN_L2, false);
                }
            } else if (ev->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                if (ev->caxis.value > AP_AXIS_DEADZONE) {
                    if (!ap__g.buttons_held[AP_BTN_R2]) {
                        ap__input_push(AP_BTN_R2, true);
                    }
                } else if (ap__g.buttons_held[AP_BTN_R2]) {
                    ap__input_push(AP_BTN_R2, false);
                }
            }
            break;
        }
        #endif

        case SDL_JOYHATMOTION: {
            if (ap__g.controller) break; /* GameController handles d-pad */
            ap__set_hat_state(ev->jhat.value, now);
            break;
        }
        #endif /* !PLATFORM_MY355 */

        /* --- Analog stick axis events (all device platforms) ---
           Thumbstick generates SDL_JOYAXISMOTION on all devices including
           MY355, so this must remain outside the MY355 exclusion guard. */
        case SDL_JOYAXISMOTION: {
            if (ev->jaxis.axis == 1) { /* Y axis (up/down) */
                if (ev->jaxis.value < -AP_AXIS_DEADZONE) {
                    ap__set_axis_direction_y(-1, now);
                } else if (ev->jaxis.value > AP_AXIS_DEADZONE) {
                    ap__set_axis_direction_y(1, now);
                } else {
                    ap__set_axis_direction_y(0, now);
                }
            } else if (ev->jaxis.axis == 0) { /* X axis (left/right) */
                if (ev->jaxis.value < -AP_AXIS_DEADZONE) {
                    ap__set_axis_direction_x(-1, now);
                } else if (ev->jaxis.value > AP_AXIS_DEADZONE) {
                    ap__set_axis_direction_x(1, now);
                } else {
                    ap__set_axis_direction_x(0, now);
                }
            }
            #if AP_PLATFORM_IS_DEVICE
            /* L2/R2 analog triggers (TrimUI TG5040/TG5050 send triggers as
             * axes 2/5 rather than joystick buttons). Use val > 0 threshold
             * to match NextUI's trigger handling.  The buttons_held guard in
             * ap__input_push prevents duplicate events if the platform also
             * sends joystick button events for these triggers. */
            else if (ev->jaxis.axis == AP__JOY_AXIS_L2) {
                if (ev->jaxis.value > 0) {
                    if (!ap__g.buttons_held[AP_BTN_L2])
                        ap__input_push(AP_BTN_L2, true);
                } else if (ap__g.buttons_held[AP_BTN_L2]) {
                    ap__input_push(AP_BTN_L2, false);
                }
            } else if (ev->jaxis.axis == AP__JOY_AXIS_R2) {
                if (ev->jaxis.value > 0) {
                    if (!ap__g.buttons_held[AP_BTN_R2])
                        ap__input_push(AP_BTN_R2, true);
                } else if (ap__g.buttons_held[AP_BTN_R2]) {
                    ap__input_push(AP_BTN_R2, false);
                }
            }
            #endif
            break;
        }
    }
}

static void ap__process_sdl_events(void) {
    uint32_t now = SDL_GetTicks();

    /* Process stored wake event first (from idle sleep in ap_present) */
    if (ap__g.has_wake_event) {
        ap__g.has_wake_event = false;
        ap__handle_sdl_event(&ap__g.wake_event, now);
    }

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        ap__handle_sdl_event(&ev, now);
    }

    bool repeated_up = false;
    bool repeated_down = false;
    bool repeated_left = false;
    bool repeated_right = false;

    /* Directional hold repeat — digital buttons (keyboard/D-pad/button maps) */
    if (ap__g.buttons_held[AP_BTN_UP] && now >= ap__g.button_repeat_time[AP_BTN_UP]) {
        repeated_up = ap__repeat_direction(AP_BTN_UP, now);
    }
    if (ap__g.buttons_held[AP_BTN_DOWN] && now >= ap__g.button_repeat_time[AP_BTN_DOWN]) {
        repeated_down = ap__repeat_direction(AP_BTN_DOWN, now);
    }
    if (ap__g.buttons_held[AP_BTN_LEFT] && now >= ap__g.button_repeat_time[AP_BTN_LEFT]) {
        repeated_left = ap__repeat_direction(AP_BTN_LEFT, now);
    }
    if (ap__g.buttons_held[AP_BTN_RIGHT] && now >= ap__g.button_repeat_time[AP_BTN_RIGHT]) {
        repeated_right = ap__repeat_direction(AP_BTN_RIGHT, now);
    }

    /* Directional hold repeat — hat */
    if (ap__g.hat_held && now >= ap__g.hat_repeat_time) {
        if ((ap__g.hat_held & SDL_HAT_UP) && !repeated_up) repeated_up = ap__repeat_direction(AP_BTN_UP, now);
        if ((ap__g.hat_held & SDL_HAT_DOWN) && !repeated_down) repeated_down = ap__repeat_direction(AP_BTN_DOWN, now);
        if ((ap__g.hat_held & SDL_HAT_LEFT) && !repeated_left) repeated_left = ap__repeat_direction(AP_BTN_LEFT, now);
        if ((ap__g.hat_held & SDL_HAT_RIGHT) && !repeated_right) repeated_right = ap__repeat_direction(AP_BTN_RIGHT, now);
        ap__advance_repeat_deadline(&ap__g.hat_repeat_time, now);
    }

    /* Directional hold repeat — analog Y */
    if (ap__g.axis_held_dir_y && now >= ap__g.axis_repeat_time_y) {
        if (ap__g.axis_held_dir_y < 0) {
            if (!repeated_up) repeated_up = ap__repeat_direction(AP_BTN_UP, now);
        } else {
            if (!repeated_down) repeated_down = ap__repeat_direction(AP_BTN_DOWN, now);
        }
        ap__advance_repeat_deadline(&ap__g.axis_repeat_time_y, now);
    }

    /* Directional hold repeat — analog X */
    if (ap__g.axis_held_dir_x && now >= ap__g.axis_repeat_time_x) {
        if (ap__g.axis_held_dir_x < 0) {
            if (!repeated_left) repeated_left = ap__repeat_direction(AP_BTN_LEFT, now);
        } else {
            if (!repeated_right) repeated_right = ap__repeat_direction(AP_BTN_RIGHT, now);
        }
        ap__advance_repeat_deadline(&ap__g.axis_repeat_time_x, now);
    }
}

bool ap_poll_input(ap_input_event *event) {
    while (1) {
        /* Process SDL events into our queue */
        ap__process_sdl_events();

        if (ap__footer_overflow_consume_open_request()) continue;

        /* Pop from internal queue */
        if (ap__input_head == ap__input_tail) return false;

        *event = ap__input_queue[ap__input_tail];
        ap__input_tail = (ap__input_tail + 1) % 64;

        /* Debounce: skip events too close together */
        uint32_t now = SDL_GetTicks();
        if (ap__g.input_delay_ms > 0 && (now - ap__g.last_input_time) < ap__g.input_delay_ms) {
            continue;
        }
        ap__g.last_input_time = now;

        return true;
    }
}

void ap_set_input_delay(uint32_t ms) {
    ap__g.input_delay_ms = ms;
}

void ap_set_input_repeat(uint32_t delay_ms, uint32_t rate_ms) {
    ap__g.input_repeat_delay_ms = delay_ms;
    ap__g.input_repeat_rate_ms = rate_ms;
}

void ap_flip_face_buttons(bool flip) {
    ap__g.face_buttons_flipped = flip;
}

const char *ap_button_name(ap_button btn) {
    if (btn < 0 || btn >= AP_BTN_COUNT) return "Unknown";
    return ap__button_names[btn];
}

static size_t ap__utf8_codepoint_count(const char *text) {
    size_t count = 0;
    if (!text) return 0;
    while (*text) {
        if ((((unsigned char)*text) & 0xC0u) != 0x80u) count++;
        text++;
    }
    return count;
}

/* ─── Combo System ───────────────────────────────────────────────────────── */

static int ap__register_combo(const char *id, ap_button *buttons, int count,
                              uint32_t timing_ms, bool is_sequence, bool strict,
                              ap_combo_callback on_trigger,
                              ap_combo_callback on_release,
                              void *userdata) {
    if (!id || !id[0] || !buttons) return AP_ERROR;
    if (count < 1 || count > 8) return AP_ERROR;
    if (ap__g.combo_count >= AP_MAX_COMBOS) return AP_ERROR;

    ap_combo *c = &ap__g.combos[ap__g.combo_count++];
    size_t button_bytes = (size_t)count * sizeof(ap_button);
    memset(c, 0, sizeof(*c));
    strncpy(c->id, id, sizeof(c->id) - 1);
    memcpy(c->buttons, buttons, button_bytes);
    c->button_count = count;
    c->window_ms = timing_ms > 0 ? timing_ms : (is_sequence ? 500 : 100);
    c->is_sequence = is_sequence;
    c->strict = strict;
    c->on_trigger = on_trigger;
    c->on_release = on_release;
    c->userdata = userdata;
    c->active = true;
    return AP_OK;
}

int ap_register_chord(const char *id, ap_button *buttons, int count, uint32_t window_ms) {
    return ap__register_combo(id, buttons, count, window_ms, false, false, NULL, NULL, NULL);
}

int ap_register_sequence(const char *id, ap_button *buttons, int count, uint32_t timeout_ms, bool strict) {
    return ap__register_combo(id, buttons, count, timeout_ms, true, strict, NULL, NULL, NULL);
}

int ap_register_chord_ex(const char *id, ap_button *buttons, int count,
                         uint32_t window_ms,
                         ap_combo_callback on_trigger,
                         ap_combo_callback on_release,
                         void *userdata) {
    return ap__register_combo(id, buttons, count, window_ms, false, false,
                              on_trigger, on_release, userdata);
}

int ap_register_sequence_ex(const char *id, ap_button *buttons, int count,
                            uint32_t timeout_ms, bool strict,
                            ap_combo_callback on_trigger,
                            void *userdata) {
    return ap__register_combo(id, buttons, count, timeout_ms, true, strict,
                              on_trigger, NULL, userdata);
}

void ap_unregister_combo(const char *id) {
    for (int i = 0; i < ap__g.combo_count; i++) {
        if (strcmp(ap__g.combos[i].id, id) == 0) {
            ap__g.combos[i].active = false;
            break;
        }
    }
}

void ap_clear_combos(void) {
    ap__g.combo_count = 0;
    ap__g.seq_buffer_count = 0;
    memset(ap__g.combo_held, 0, sizeof(ap__g.combo_held));
    ap__g.combo_queue_head = 0;
    ap__g.combo_queue_tail = 0;
}

bool ap_poll_combo(ap_combo_event *event) {
    if (ap__g.combo_queue_head == ap__g.combo_queue_tail) return false;
    *event = ap__g.combo_queue[ap__g.combo_queue_tail];
    ap__g.combo_queue_tail = (ap__g.combo_queue_tail + 1) % 16;
    return true;
}


/* ─── Drawing Primitives ─────────────────────────────────────────────────── */

void ap_clear_screen(void) {
    ap__footer_overflow_begin_frame();
    ap_color bg = ap__g.theme.background;
    SDL_SetRenderDrawColor(ap__g.renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(ap__g.renderer);
}

void ap_request_frame(void) {
    ap__g.needs_frame = true;
}

void ap_request_frame_in(uint32_t ms) {
    uint32_t target = SDL_GetTicks() + ms;
    if (ap__g.next_redraw_ms == 0 || target < ap__g.next_redraw_ms) {
        ap__g.next_redraw_ms = target;
    }
}

static uint32_t ap__next_wake_time(void) {
    uint32_t now = SDL_GetTicks();
    uint32_t wake = now + 1000; /* max 1s sleep — ensures clock updates */

    if (ap__g.next_redraw_ms != 0 && ap__g.next_redraw_ms < wake)
        wake = ap__g.next_redraw_ms;

    for (int i = 0; i < AP_BTN_COUNT; i++) {
        if (ap__g.buttons_held[i] && ap__g.button_repeat_time[i] != 0
            && ap__g.button_repeat_time[i] < wake)
            wake = ap__g.button_repeat_time[i];
    }
    if (ap__g.hat_held && ap__g.hat_repeat_time != 0
        && ap__g.hat_repeat_time < wake)
        wake = ap__g.hat_repeat_time;
    if (ap__g.axis_held_dir_y && ap__g.axis_repeat_time_y != 0
        && ap__g.axis_repeat_time_y < wake)
        wake = ap__g.axis_repeat_time_y;
    if (ap__g.axis_held_dir_x && ap__g.axis_repeat_time_x != 0
        && ap__g.axis_repeat_time_x < wake)
        wake = ap__g.axis_repeat_time_x;

    return wake;
}

void ap_present(void) {
    SDL_RenderPresent(ap__g.renderer);

    if (ap__g.needs_frame) {
        /* Active rendering: normal 60fps pacing */
        ap__g.needs_frame = false;
        uint32_t now = SDL_GetTicks();
        uint32_t elapsed = now - ap__g.last_present_ms;
        if (elapsed < 16) {
            SDL_Delay(16 - elapsed);
        }
    } else {
        /* Idle: sleep until next wake event or scheduled redraw */
        uint32_t wake = ap__next_wake_time();
        uint32_t now = SDL_GetTicks();
        int timeout = (wake > now) ? (int)(wake - now) : 0;
        if (timeout > 0) {
            if (SDL_WaitEventTimeout(&ap__g.wake_event, timeout) == 1) {
                ap__g.has_wake_event = true;
            }
        }
        if (ap__g.next_redraw_ms != 0 && SDL_GetTicks() >= ap__g.next_redraw_ms) {
            ap__g.next_redraw_ms = 0;
        }
    }
    ap__g.last_present_ms = SDL_GetTicks();
}

void ap_draw_background(void) {
    ap__footer_overflow_begin_frame();
    if (ap__g.bg_texture) {
        SDL_RenderCopy(ap__g.renderer, ap__g.bg_texture, NULL, NULL);
    } else {
        ap_clear_screen();
    }
}

/* Fill a quarter-circle arc with anti-aliased edges */
static void ap__fill_circle_quadrant(int cx, int cy, int r, int quadrant) {
    SDL_Renderer *rend = ap__g.renderer;
    Uint8 base_r, base_g, base_b, base_a;
    SDL_GetRenderDrawColor(rend, &base_r, &base_g, &base_b, &base_a);

    for (int dy = 0; dy <= r; dy++) {
        float fx = sqrtf((float)(r * r - dy * dy));
        int ix = (int)fx;            /* integer part = fully filled pixels */
        float frac = fx - (float)ix; /* fractional part = edge pixel alpha */
        Uint8 edge_a = (Uint8)(frac * base_a);
        int y0;

        switch (quadrant) {
            case 0: /* top-left */
                y0 = cy - dy;
                SDL_RenderDrawLine(rend, cx - ix, y0, cx, y0);
                if (edge_a > 0) {
                    SDL_SetRenderDrawColor(rend, base_r, base_g, base_b, edge_a);
                    SDL_RenderDrawPoint(rend, cx - ix - 1, y0);
                    SDL_SetRenderDrawColor(rend, base_r, base_g, base_b, base_a);
                }
                break;
            case 1: /* top-right */
                y0 = cy - dy;
                SDL_RenderDrawLine(rend, cx, y0, cx + ix, y0);
                if (edge_a > 0) {
                    SDL_SetRenderDrawColor(rend, base_r, base_g, base_b, edge_a);
                    SDL_RenderDrawPoint(rend, cx + ix + 1, y0);
                    SDL_SetRenderDrawColor(rend, base_r, base_g, base_b, base_a);
                }
                break;
            case 2: /* bottom-left */
                y0 = cy + dy;
                SDL_RenderDrawLine(rend, cx - ix, y0, cx, y0);
                if (edge_a > 0) {
                    SDL_SetRenderDrawColor(rend, base_r, base_g, base_b, edge_a);
                    SDL_RenderDrawPoint(rend, cx - ix - 1, y0);
                    SDL_SetRenderDrawColor(rend, base_r, base_g, base_b, base_a);
                }
                break;
            case 3: /* bottom-right */
                y0 = cy + dy;
                SDL_RenderDrawLine(rend, cx, y0, cx + ix, y0);
                if (edge_a > 0) {
                    SDL_SetRenderDrawColor(rend, base_r, base_g, base_b, edge_a);
                    SDL_RenderDrawPoint(rend, cx + ix + 1, y0);
                    SDL_SetRenderDrawColor(rend, base_r, base_g, base_b, base_a);
                }
                break;
        }
    }
}

void ap_draw_rounded_rect(int x, int y, int w, int h, int r, ap_color c) {
    SDL_Renderer *rend = ap__g.renderer;
    if (r > h / 2) r = h / 2;
    if (r > w / 2) r = w / 2;
    if (r < 0) r = 0;

    /* Use quarter-circle sprites from the pill asset for smooth AA whenever
       the NextUI assets sheet is available.
       The 30×30 white pill at (1,1) in assets@Nx.png is a perfect circle — each
       quadrant scaled to the corner radius via bilinear-filtered SDL_RenderCopy. */
    if (ap__g.status_assets && r > 0) {
        int s = ap__g.status_asset_scale;
        int sx = 1 * s, sy = 1 * s; /* white pill sprite origin */
        int half = 15 * s;           /* half of the 30×30 sprite */

        SDL_SetTextureColorMod(ap__g.status_assets, c.r, c.g, c.b);
        SDL_SetTextureAlphaMod(ap__g.status_assets, c.a);

        /* Four corner sprites (quarter-circles) */
        SDL_Rect tl_src = { sx,        sy,        half, half };
        SDL_Rect tr_src = { sx + half, sy,        half, half };
        SDL_Rect bl_src = { sx,        sy + half, half, half };
        SDL_Rect br_src = { sx + half, sy + half, half, half };

        SDL_Rect tl_dst = { x,         y,         r, r };
        SDL_Rect tr_dst = { x + w - r, y,         r, r };
        SDL_Rect bl_dst = { x,         y + h - r, r, r };
        SDL_Rect br_dst = { x + w - r, y + h - r, r, r };

        SDL_RenderCopy(rend, ap__g.status_assets, &tl_src, &tl_dst);
        SDL_RenderCopy(rend, ap__g.status_assets, &tr_src, &tr_dst);
        SDL_RenderCopy(rend, ap__g.status_assets, &bl_src, &bl_dst);
        SDL_RenderCopy(rend, ap__g.status_assets, &br_src, &br_dst);

        /* Fill body rectangles (cross pattern between corners) */
        SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, c.a);
        SDL_Rect top_bar = { x + r, y,         w - 2 * r, r };
        SDL_Rect mid_bar = { x,     y + r,     w,         h - 2 * r };
        SDL_Rect bot_bar = { x + r, y + h - r, w - 2 * r, r };
        SDL_RenderFillRect(rend, &top_bar);
        SDL_RenderFillRect(rend, &mid_bar);
        SDL_RenderFillRect(rend, &bot_bar);
        return;
    }

    /* Desktop / fallback: procedural anti-aliased corners */
    SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, c.a);

    /* Center rectangle (between top/bottom arcs) */
    SDL_Rect center = {x, y + r, w, h - 2 * r};
    SDL_RenderFillRect(rend, &center);

    /* Top bar between corners */
    SDL_Rect top = {x + r, y, w - 2 * r, r};
    SDL_RenderFillRect(rend, &top);

    /* Bottom bar between corners */
    SDL_Rect bot = {x + r, y + h - r, w - 2 * r, r};
    SDL_RenderFillRect(rend, &bot);

    /* Four corner arcs */
    ap__fill_circle_quadrant(x + r - 1,     y + r - 1,     r, 0);
    ap__fill_circle_quadrant(x + w - r,     y + r - 1,     r, 1);
    ap__fill_circle_quadrant(x + r - 1,     y + h - r,     r, 2);
    ap__fill_circle_quadrant(x + w - r,     y + h - r,     r, 3);
}

void ap_draw_pill(int x, int y, int w, int h, ap_color c) {
    /* Use the pre-rendered NextUI pill sprite for proper AA whenever status
       assets are available.
       White pill sprite at (1,1, 30,30) in 1x coords — a 30×30 circle with
       anti-aliased alpha edges. Blit left cap + center fill + right cap.
       Color tinting via SDL_SetTextureColorMod on the white sprite. */
    if (ap__g.status_assets) {
        int s = ap__g.status_asset_scale;
        int sprite_x = 1 * s;  /* white pill top-left in spritesheet */
        int sprite_y = 1 * s;
        int sprite_sz = 30 * s; /* full sprite width/height */
        int cap_src_w = sprite_sz / 2; /* half-sprite = one cap */
        int cap_dst_w = h / 2;         /* destination cap width = half pill height */

        SDL_SetTextureColorMod(ap__g.status_assets, c.r, c.g, c.b);
        SDL_SetTextureAlphaMod(ap__g.status_assets, c.a);

        /* Left cap */
        SDL_Rect lsrc = { sprite_x, sprite_y, cap_src_w, sprite_sz };
        SDL_Rect ldst = { x, y, cap_dst_w, h };
        SDL_RenderCopy(ap__g.renderer, ap__g.status_assets, &lsrc, &ldst);

        /* Center fill — solid rect between caps.
           Use 2*cap_dst_w (not h) to avoid 1px gap when h is odd. */
        int center_w = w - 2 * cap_dst_w;
        if (center_w > 0) {
            SDL_SetRenderDrawColor(ap__g.renderer, c.r, c.g, c.b, c.a);
            SDL_Rect center = { x + cap_dst_w, y, center_w, h };
            SDL_RenderFillRect(ap__g.renderer, &center);
        }

        /* Right cap — position from left cap + center to avoid rounding gap */
        SDL_Rect rsrc = { sprite_x + cap_src_w, sprite_y, cap_src_w, sprite_sz };
        SDL_Rect rdst = { x + cap_dst_w + (center_w > 0 ? center_w : 0), y, cap_dst_w, h };
        SDL_RenderCopy(ap__g.renderer, ap__g.status_assets, &rsrc, &rdst);

        return;
    }
    /* Desktop / fallback: procedural drawing */
    ap_draw_rounded_rect(x, y, w, h, h / 2, c);
}

void ap_draw_rect(int x, int y, int w, int h, ap_color c) {
    SDL_SetRenderDrawColor(ap__g.renderer, c.r, c.g, c.b, c.a);
    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(ap__g.renderer, &r);
}

void ap_draw_circle(int cx, int cy, int r, ap_color c) {
    SDL_SetRenderDrawColor(ap__g.renderer, c.r, c.g, c.b, c.a);
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)(sqrtf((float)(r * r - dy * dy)) + 0.5f);
        SDL_RenderDrawLine(ap__g.renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

int ap_draw_text(TTF_Font *font, const char *text, int x, int y, ap_color color) {
    if (!font || !text || !text[0]) return 0;

    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return 0;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ap__g.renderer, surf);
    if (!tex) { SDL_FreeSurface(surf); return 0; }

    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(ap__g.renderer, tex, NULL, &dst);

    int w = surf->w;
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
    return w;
}

int ap_draw_text_clipped(TTF_Font *font, const char *text, int x, int y, ap_color color, int max_w) {
    if (!font || !text || !text[0]) return 0;
    if (max_w <= 0) return ap_draw_text(font, text, x, y, color);

    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return 0;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ap__g.renderer, surf);
    if (!tex) { SDL_FreeSurface(surf); return 0; }

    int draw_w = surf->w;
    if (draw_w > max_w) draw_w = max_w;

    SDL_Rect src = {0, 0, draw_w, surf->h};
    SDL_Rect dst = {x, y, draw_w, surf->h};
    SDL_RenderCopy(ap__g.renderer, tex, &src, &dst);

    int orig_w = surf->w;
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
    return orig_w;
}

int ap_draw_text_ellipsized(TTF_Font *font, const char *text, int x, int y, ap_color color, int max_w) {
    if (!font || !text || !text[0]) return 0;
    if (max_w <= 0) return ap_draw_text(font, text, x, y, color);

    int full_w = 0;
    TTF_SizeUTF8(font, text, &full_w, NULL);
    if (full_w <= max_w) return ap_draw_text(font, text, x, y, color);

    int ellipsis_w = 0;
    TTF_SizeUTF8(font, "...", &ellipsis_w, NULL);
    if (ellipsis_w >= max_w) return ap_draw_text_clipped(font, text, x, y, color, max_w);

    int target_w = max_w - ellipsis_w;
    int len = (int)strlen(text);
    int lo = 0, hi = len, best = 0;

    /* Use stack buffer for typical labels, heap for unusually long strings */
    char stack_buf[1024];
    int buf_size = len + 4;  /* +4 for "..." and NUL */
    char *buf = buf_size <= (int)sizeof(stack_buf) ? stack_buf : (char *)malloc(buf_size);
    if (!buf) return ap_draw_text_clipped(font, text, x, y, color, max_w);

    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        /* Walk back to a valid UTF-8 start byte */
        int safe = mid;
        while (safe > 0 && (text[safe] & 0xC0) == 0x80) safe--;

        memcpy(buf, text, safe);
        buf[safe] = '\0';

        int tw = 0;
        TTF_SizeUTF8(font, buf, &tw, NULL);

        if (tw <= target_w) {
            best = safe;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    /* Strip trailing spaces before ellipsis */
    while (best > 0 && text[best - 1] == ' ') best--;

    memcpy(buf, text, best);
    buf[best]     = '.';
    buf[best + 1] = '.';
    buf[best + 2] = '.';
    buf[best + 3] = '\0';

    int result = ap_draw_text(font, buf, x, y, color);
    if (buf != stack_buf) free(buf);
    return result;
}

int ap_measure_text_ellipsized(TTF_Font *font, const char *text, int max_w) {
    if (!font || !text || !text[0]) return 0;
    if (max_w <= 0) return 0;

    int full_w = 0;
    TTF_SizeUTF8(font, text, &full_w, NULL);
    if (full_w <= max_w) return full_w;

    int ellipsis_w = 0;
    TTF_SizeUTF8(font, "...", &ellipsis_w, NULL);
    if (ellipsis_w >= max_w) return max_w;

    int target_w = max_w - ellipsis_w;
    int len = (int)strlen(text);
    int lo = 0, hi = len, best = 0;

    char stack_buf[1024];
    int buf_size = len + 4;
    char *buf = buf_size <= (int)sizeof(stack_buf) ? stack_buf : (char *)malloc(buf_size);
    int result_w;

    if (!buf) return max_w;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int safe = mid;
        while (safe > 0 && (text[safe] & 0xC0) == 0x80) safe--;

        memcpy(buf, text, safe);
        buf[safe] = '\0';

        int tw = 0;
        TTF_SizeUTF8(font, buf, &tw, NULL);

        if (tw <= target_w) {
            best = safe;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    while (best > 0 && text[best - 1] == ' ') best--;

    memcpy(buf, text, best);
    buf[best]     = '.';
    buf[best + 1] = '.';
    buf[best + 2] = '.';
    buf[best + 3] = '\0';

    TTF_SizeUTF8(font, buf, &result_w, NULL);
    if (buf != stack_buf) free(buf);
    return result_w;
}

int ap_draw_text_wrapped(TTF_Font *font, const char *text, int x, int y, int max_w, ap_color color, ap_text_align align) {
    if (!font || !text || !text[0] || max_w <= 0) return 0;

    /* Word wrap: break text into lines that fit within max_w */
    char buf[4096];
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    int line_h = TTF_FontLineSkip(font);
    int cur_y = y;

    char *line_start = buf;
    while (*line_start) {
        /* Preserve explicit blank lines ("\n\n") as vertical spacing. */
        if (*line_start == '\n') {
            cur_y += line_h;
            line_start++;
            continue;
        }

        /* Find how much text fits on this line */
        char *best_break = NULL;
        char *p = line_start;

        while (*p) {
            /* Find next word boundary */
            char *word_end = p;
            while (*word_end && *word_end != ' ' && *word_end != '\n') word_end++;

            /* Check if text up to word_end fits */
            char saved = *word_end;
            *word_end = '\0';

            int tw = 0;
            TTF_SizeUTF8(font, line_start, &tw, NULL);

            *word_end = saved;

            if (tw > max_w && best_break) {
                /* Doesn't fit — break at last good position */
                break;
            }

            best_break = word_end;

            if (*word_end == '\n') {
                best_break = word_end;
                break;
            }

            if (*word_end == '\0') break;
            p = word_end + 1;
        }

        if (!best_break || best_break == line_start) {
            /* Single word wider than max_w, or end of text */
            best_break = line_start + strlen(line_start);
        }

        /* Render this line */
        char saved = *best_break;
        *best_break = '\0';

        if (line_start[0]) {
            int tw = 0;
            TTF_SizeUTF8(font, line_start, &tw, NULL);

            int draw_x = x;
            if (align == AP_ALIGN_CENTER) draw_x = x + (max_w - tw) / 2;
            else if (align == AP_ALIGN_RIGHT) draw_x = x + max_w - tw;

            ap_draw_text(font, line_start, draw_x, cur_y, color);
        }

        cur_y += line_h;
        *best_break = saved;

        /* Advance past the break character */
        if (*best_break == ' ' || *best_break == '\n')
            line_start = best_break + 1;
        else
            line_start = best_break;
    }
    return cur_y - y;
}

static int ap__wrapped_line_count(TTF_Font *font, const char *text, int max_w) {
    if (!font || !text || !text[0] || max_w <= 0) return 0;

    char buf[4096];
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    int lines = 0;
    char *line_start = buf;

    while (*line_start) {
        if (*line_start == '\n') {
            lines++;
            line_start++;
            continue;
        }

        char *best_break = NULL;
        char *p = line_start;

        while (*p) {
            char *word_end = p;
            while (*word_end && *word_end != ' ' && *word_end != '\n') word_end++;

            char saved = *word_end;
            *word_end = '\0';

            int tw = 0;
            TTF_SizeUTF8(font, line_start, &tw, NULL);

            *word_end = saved;

            if (tw > max_w && best_break) break;

            best_break = word_end;

            if (*word_end == '\n') {
                best_break = word_end;
                break;
            }

            if (*word_end == '\0') break;
            p = word_end + 1;
        }

        if (!best_break || best_break == line_start) {
            best_break = line_start + strlen(line_start);
        }

        lines++;

        if (*best_break == ' ' || *best_break == '\n')
            line_start = best_break + 1;
        else
            line_start = best_break;
    }

    return lines;
}

int ap_measure_text(TTF_Font *font, const char *text) {
    if (!font || !text || !text[0]) return 0;
    int w = 0;
    TTF_SizeUTF8(font, text, &w, NULL);
    return w;
}

int ap_measure_wrapped_text_height(TTF_Font *font, const char *text, int max_w) {
    if (!font || !text || !text[0] || max_w <= 0) return 0;
    return ap__wrapped_line_count(font, text, max_w) * TTF_FontLineSkip(font);
}

SDL_Rect ap_get_content_rect(bool has_title, bool has_footer, bool has_status_bar) {
    SDL_Rect rect = { 0, 0, ap_get_screen_width(), ap_get_screen_height() };

    int top = 0;
    if (has_title || has_status_bar) {
        top = ap__max(AP_DS(40), ap_get_status_bar_height());
    }

    int bottom = has_footer ? ap_get_footer_height() : 0;
    rect.y = top;
    rect.h = ap_get_screen_height() - top - bottom;
    if (rect.h < 0) rect.h = 0;

    return rect;
}

static void ap__draw_screen_title_impl(const char *title, ap_status_bar_opts *status_bar, bool center) {
    if (!title || !title[0]) return;

    int margin = AP_DS(ap__g.device_padding + 5);
    int max_w = ap_get_screen_width() - margin * 2;
    if (status_bar) {
        int status_bar_w = ap_get_status_bar_width(status_bar);
        if (status_bar_w > 0) max_w -= status_bar_w + AP_S(10);
    }
    if (max_w < 1) max_w = 1;

    static const ap_font_tier title_tiers[] = {
        AP_FONT_EXTRA_LARGE,
        AP_FONT_LARGE,
        AP_FONT_MEDIUM,
    };

    TTF_Font *font = NULL;
    for (size_t i = 0; i < sizeof(title_tiers) / sizeof(title_tiers[0]); i++) {
        TTF_Font *candidate = ap_get_font(title_tiers[i]);
        if (!candidate) continue;
        font = candidate;
        if (ap_measure_text(candidate, title) <= max_w) break;
    }

    if (!font) return;

    int x = margin;
    if (center) {
        int text_w = ap_measure_text(font, title);
        if (text_w > max_w) text_w = max_w;
        x = margin + (max_w - text_w) / 2;
    }
    ap_draw_text_clipped(font, title, x, 0, ap_get_theme()->text, max_w);
}

void ap_draw_screen_title(const char *title, ap_status_bar_opts *status_bar) {
    ap__draw_screen_title_impl(title, status_bar, false);
}

void ap_draw_screen_title_centered(const char *title, ap_status_bar_opts *status_bar) {
    ap__draw_screen_title_impl(title, status_bar, true);
}

void ap_draw_image(SDL_Texture *tex, int x, int y, int w, int h) {
    if (!tex) return;
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(ap__g.renderer, tex, NULL, &dst);
}

SDL_Texture *ap_load_image(const char *path) {
    if (!path) return NULL;
    return IMG_LoadTexture(ap__g.renderer, path);
}

void ap_draw_scrollbar(int x, int y, int h, int visible, int total, int offset) {
    if (total <= visible || total <= 0) return;

    SDL_Renderer *rend = ap__g.renderer;
    SDL_BlendMode prev_blend = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(rend, &prev_blend);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);

    int bar_w = AP_S(4);
    int track_h = h;
    int thumb_h = ap__max((track_h * visible) / total, AP_S(20));
    int thumb_y = y + (offset * (track_h - thumb_h)) / (total - visible);

    /* Track */
    ap_color track_color = ap__g.theme.hint;
    track_color.a = 40;
    ap_draw_rounded_rect(x, y, bar_w, track_h, bar_w / 2, track_color);

    /* Thumb */
    ap_color thumb_color = ap__g.theme.hint;
    thumb_color.a = 120;
    ap_draw_rounded_rect(x, thumb_y, bar_w, thumb_h, bar_w / 2, thumb_color);

    SDL_SetRenderDrawBlendMode(rend, prev_blend);
}

void ap_draw_progress_bar(int x, int y, int w, int h, float progress, ap_color fg, ap_color bg) {
    /* Background track */
    ap_draw_rounded_rect(x, y, w, h, h / 2, bg);

    /* Fill */
    int fill_w = (int)(w * ap__clamp((int)(progress * 100), 0, 100) / 100.0f);
    if (fill_w > 0) {
        ap_draw_rounded_rect(x, y, fill_w, h, h / 2, fg);
    }
}

/* ─── Text Scrolling ─────────────────────────────────────────────────────── */

void ap_text_scroll_init(ap_text_scroll *s) {
    if (!s) return;
    s->offset = 0;
    s->direction = 1;
    s->pause_timer = AP_TEXT_SCROLL_PAUSE_MS;
    s->active = false;
}

void ap_text_scroll_update(ap_text_scroll *s, int text_w, int visible_w, uint32_t dt_ms) {
    if (!s) return;

    if (text_w <= visible_w) {
        s->offset = 0;
        s->active = false;
        return;
    }

    s->active = true;
    int max_offset = text_w - visible_w;

    if (s->pause_timer > 0) {
        s->pause_timer -= (int)dt_ms;
        return;
    }

    s->offset += s->direction * AP_TEXT_SCROLL_SPEED;

    if (s->offset >= max_offset) {
        s->offset = max_offset;
        s->direction = -1;
        s->pause_timer = AP_TEXT_SCROLL_PAUSE_MS;
    } else if (s->offset <= 0) {
        s->offset = 0;
        s->direction = 1;
        s->pause_timer = AP_TEXT_SCROLL_PAUSE_MS;
    }
}

void ap_text_scroll_reset(ap_text_scroll *s) {
    ap_text_scroll_init(s);
}

/* ─── Texture Cache ──────────────────────────────────────────────────────── */

SDL_Texture *ap_cache_get(const char *key, int *w, int *h) {
    uint32_t now = SDL_GetTicks();
    for (int i = 0; i < ap__g.tex_cache.count; i++) {
        if (strcmp(ap__g.tex_cache.entries[i].key, key) == 0) {
            ap__g.tex_cache.entries[i].last_used = now;
            if (w) *w = ap__g.tex_cache.entries[i].w;
            if (h) *h = ap__g.tex_cache.entries[i].h;
            return ap__g.tex_cache.entries[i].texture;
        }
    }
    return NULL;
}

void ap_cache_put(const char *key, SDL_Texture *tex, int w, int h) {
    uint32_t now = SDL_GetTicks();

    /* Check if already cached */
    for (int i = 0; i < ap__g.tex_cache.count; i++) {
        if (strcmp(ap__g.tex_cache.entries[i].key, key) == 0) {
            if (ap__g.tex_cache.entries[i].texture != tex) {
                SDL_DestroyTexture(ap__g.tex_cache.entries[i].texture);
            }
            ap__g.tex_cache.entries[i].texture = tex;
            ap__g.tex_cache.entries[i].w = w;
            ap__g.tex_cache.entries[i].h = h;
            ap__g.tex_cache.entries[i].last_used = now;
            return;
        }
    }

    /* If cache is full, evict least recently used */
    if (ap__g.tex_cache.count >= AP_TEXTURE_CACHE_SIZE) {
        int lru = 0;
        for (int i = 1; i < ap__g.tex_cache.count; i++) {
            if (ap__g.tex_cache.entries[i].last_used < ap__g.tex_cache.entries[lru].last_used)
                lru = i;
        }
        SDL_DestroyTexture(ap__g.tex_cache.entries[lru].texture);
        ap__g.tex_cache.entries[lru] = (ap_cache_entry){0};
        /* Move last entry to fill gap */
        if (lru < ap__g.tex_cache.count - 1) {
            ap__g.tex_cache.entries[lru] = ap__g.tex_cache.entries[ap__g.tex_cache.count - 1];
        }
        ap__g.tex_cache.count--;
    }

    /* Add new entry */
    ap_cache_entry *e = &ap__g.tex_cache.entries[ap__g.tex_cache.count++];
    strncpy(e->key, key, sizeof(e->key) - 1);
    e->texture = tex;
    e->w = w;
    e->h = h;
    e->last_used = now;
}

void ap_cache_clear(void) {
    for (int i = 0; i < ap__g.tex_cache.count; i++) {
        if (ap__g.tex_cache.entries[i].texture) {
            SDL_DestroyTexture(ap__g.tex_cache.entries[i].texture);
        }
    }
    memset(&ap__g.tex_cache, 0, sizeof(ap__g.tex_cache));
}

/* ─── Footer & Status Bar ────────────────────────────────────────────────── */

int ap_get_footer_height(void) {
    /* padding + pill_size, matching NextUI: SCALE1(PADDING + PILL_SIZE) */
    return AP_DS(ap__g.device_padding + AP__PILL_SIZE);
}

void ap_set_footer_overflow_opts(const ap_footer_overflow_opts *opts) {
    if (opts) {
        ap__g.footer_overflow_opts = *opts;
    } else {
        ap__g.footer_overflow_opts.enabled = true;
        ap__g.footer_overflow_opts.chord_a = AP_BTN_NONE;
        ap__g.footer_overflow_opts.chord_b = AP_BTN_NONE;
    }

    memset(ap__g.footer_overflow_swallow, 0, sizeof(ap__g.footer_overflow_swallow));
    ap__g.footer_overflow_chord_held = false;
    ap__g.footer_overflow_open_requested = false;

    if (!ap__g.footer_overflow_opts.enabled ||
        ap__g.footer_overflow_opts.chord_a == AP_BTN_NONE ||
        ap__g.footer_overflow_opts.chord_b == AP_BTN_NONE ||
        ap__g.footer_overflow_opts.chord_a == ap__g.footer_overflow_opts.chord_b) {
        ap__g.footer_overflow_active = false;
        ap__g.footer_hidden_count = 0;
    }
}

void ap_show_footer_overflow(void) {
    if (ap__g.footer_hidden_count > 0)
        ap__footer_overflow_show_hidden_actions();
}

void ap_get_footer_overflow_opts(ap_footer_overflow_opts *out) {
    if (!out) return;
    *out = ap__g.footer_overflow_opts;
}

static const char *ap__footer_button_text(ap_footer_item *item) {
    if (!item) return "";
    if (item->button_text && item->button_text[0]) return item->button_text;
    return ap_button_name(item->button);
}

static bool ap__footer_button_text_is_single_codepoint(const char *text) {
    return ap__utf8_codepoint_count(text) == 1;
}

static TTF_Font *ap__footer_button_font(const char *btn_name) {
    if (!btn_name || !btn_name[0]) return ap_get_font(AP_FONT_SMALL);
    if (ap__footer_button_text_is_single_codepoint(btn_name)) {
        return ap_get_font(AP_FONT_MEDIUM); /* NextUI: single-codepoint button label */
    }
    return ap_get_font(AP_FONT_TINY); /* NextUI: multi-codepoint button label */
}

static int ap__footer_item_width(ap_footer_item *item, TTF_Font *hint_font, int btn_margin) {
    const char *btn_name = ap__footer_button_text(item);
    const char *label = item->label ? item->label : "";
    TTF_Font *btn_font = ap__footer_button_font(btn_name);
    if (!btn_font) btn_font = hint_font;

    int btn_tw = ap_measure_text(btn_font, btn_name);
    int btn_w = ap__footer_button_text_is_single_codepoint(btn_name)
        ? AP_DS(AP__BUTTON_SIZE)
        : (AP_DS(AP__BUTTON_SIZE) / 2 + btn_tw);
    int label_w = ap_measure_text(hint_font, label);
    return btn_w + btn_margin + label_w + btn_margin;
}

static int ap__footer_marker_width(int hidden_count, TTF_Font *hint_font, int btn_margin) {
    if (hidden_count <= 0) return 0;
    char marker[16];
    snprintf(marker, sizeof(marker), "+%d", hidden_count);
    return btn_margin + ap_measure_text(hint_font, marker) + btn_margin;
}

static int ap__footer_group_outer_width(const int *widths, int visible_count,
                                        int item_gap, int outer_pad, int marker_w) {
    if (visible_count <= 0 && marker_w <= 0) return 0;

    int inner = 0;
    for (int i = 0; i < visible_count; i++) {
        if (i > 0) inner += item_gap;
        inner += widths[i];
    }
    if (marker_w > 0) {
        if (visible_count > 0) inner += item_gap;
        inner += marker_w;
    }
    return outer_pad + inner + outer_pad;
}

static void ap__footer_draw_item(int *cx, int btn_y, int inner_h, int btn_margin,
                                 int hint_font_h, TTF_Font *hint_font, ap_footer_item *item) {
    const char *btn_name = ap__footer_button_text(item);
    const char *label = item->label ? item->label : "";
    TTF_Font *btn_font = ap__footer_button_font(btn_name);
    if (!btn_font) btn_font = hint_font;

    int btn_font_h = TTF_FontHeight(btn_font);
    int btn_tw = ap_measure_text(btn_font, btn_name);
    int btn_pill_w = ap__footer_button_text_is_single_codepoint(btn_name)
        ? AP_DS(AP__BUTTON_SIZE)
        : (AP_DS(AP__BUTTON_SIZE) / 2 + btn_tw);

    ap_draw_pill(*cx, btn_y, btn_pill_w, inner_h, ap__g.theme.highlight);
    ap_draw_text(btn_font, btn_name,
                 *cx + (btn_pill_w - btn_tw) / 2,
                 btn_y + (inner_h - btn_font_h) / 2,
                 ap__g.theme.button_label);

    *cx += btn_pill_w + btn_margin;
    ap_draw_text(hint_font, label,
                 *cx,
                 btn_y + (inner_h - hint_font_h) / 2,
                 ap__g.theme.hint);
    *cx += ap_measure_text(hint_font, label) + btn_margin;
}

static void ap__footer_store_hidden_item(ap_footer_item *item) {
    if (!item) return;
    ap__g.footer_overflow_active = true;
    if (ap__g.footer_hidden_count >= AP__MAX_FOOTER_ITEMS) return;
    ap__g.footer_hidden_items[ap__g.footer_hidden_count++] = *item;
}

void ap_draw_footer(ap_footer_item *items, int count) {
    ap__footer_overflow_clear_visible_state();
    if (!items || count <= 0) return;
    if (count > AP__MAX_FOOTER_ITEMS) count = AP__MAX_FOOTER_ITEMS;

    /* Match NextUI GFX_blitButtonGroup layout:
     * outer pill = PILL_SIZE high, inner buttons = BUTTON_SIZE high,
     * inset by BUTTON_MARGIN, positioned PADDING from screen edges. */
    TTF_Font *hint_font = ap_get_font(AP_FONT_SMALL); /* NextUI: hint text font.small */
    if (!hint_font) return;

    int padding    = AP_DS(ap__g.device_padding);      /* screen-edge padding (NextUI PADDING) */
    int outer_h    = AP_DS(AP__PILL_SIZE);              /* outer pill height */
    int btn_margin = AP_DS(AP__BUTTON_MARGIN);          /* inset from outer → inner + inter-element */
    int inner_h    = AP_DS(AP__BUTTON_SIZE);             /* inner button pill height */
    int pill_y     = ap__g.screen_h - AP_DS(ap__g.device_padding + AP__PILL_SIZE); /* NextUI: screen_h - SCALE1(PADDING + PILL_SIZE) */
    int item_gap   = btn_margin;                        /* gap between items */
    int outer_pad  = btn_margin;                        /* padding at start/end of outer pill */
    int hint_font_h = TTF_FontHeight(hint_font);
    int max_total_w = ap__g.screen_w - padding * 2;
    if (max_total_w < 0) max_total_w = 0;

    int left_indices[AP__MAX_FOOTER_ITEMS];
    int left_widths[AP__MAX_FOOTER_ITEMS];
    int right_indices[AP__MAX_FOOTER_ITEMS];
    int right_widths[AP__MAX_FOOTER_ITEMS];

    int left_count = 0;
    int right_count = 0;
    for (int i = 0; i < count; i++) {
        int width = ap__footer_item_width(&items[i], hint_font, btn_margin);
        if (items[i].is_confirm) {
            right_indices[right_count] = i;
            right_widths[right_count] = width;
            right_count++;
        } else {
            left_indices[left_count] = i;
            left_widths[left_count] = width;
            left_count++;
        }
    }

    int right_visible_start = 0;
    int right_visible_count = right_count;
    int right_outer_w = ap__footer_group_outer_width(right_widths, right_count, item_gap, outer_pad, 0);

    if (ap__g.footer_overflow_opts.enabled && right_count > 0 && right_outer_w > max_total_w) {
        int inner = 0;
        right_visible_start = right_count;
        right_visible_count = 0;

        for (int i = right_count - 1; i >= 0; i--) {
            int candidate = right_widths[i];
            if (right_visible_count > 0) candidate += item_gap;
            if (outer_pad + inner + candidate + outer_pad <= max_total_w || right_visible_count == 0) {
                inner += candidate;
                right_visible_start = i;
                right_visible_count++;
            }
        }

        right_outer_w = (right_visible_count > 0) ? (outer_pad + inner + outer_pad) : 0;
        for (int i = 0; i < right_visible_start; i++) {
            ap__footer_store_hidden_item(&items[right_indices[i]]);
        }
    }

    int available_left_w = max_total_w - right_outer_w;
    if (available_left_w < 0) available_left_w = 0;

    int left_visible_count = left_count;

    if (ap__g.footer_overflow_opts.enabled) {
        int best_visible = -1;

        for (int visible = 0; visible <= left_count; visible++) {
            int hidden_total = ap__g.footer_hidden_count + (left_count - visible);
            int candidate_marker_w = ap__footer_marker_width(hidden_total, hint_font, btn_margin);
            int outer_w = ap__footer_group_outer_width(left_widths, visible, item_gap, outer_pad,
                                                       hidden_total > 0 ? candidate_marker_w : 0);
            if (outer_w <= available_left_w) {
                best_visible = visible;
            }
        }

        if (best_visible < 0) {
            left_visible_count = 0;
        } else {
            left_visible_count = best_visible;
        }

        for (int i = left_visible_count; i < left_count; i++) {
            ap__footer_store_hidden_item(&items[left_indices[i]]);
        }
        if (ap__g.footer_hidden_count <= 0) {
            ap__g.footer_overflow_active = false;
        }
    }

    int left_outer_w = ap__footer_group_outer_width(left_widths, left_visible_count, item_gap, outer_pad,
                                                    (ap__g.footer_hidden_count > 0 && ap__g.footer_overflow_opts.enabled)
                                                        ? ap__footer_marker_width(ap__g.footer_hidden_count, hint_font, btn_margin)
                                                        : 0);

    if (!ap__g.footer_overflow_opts.enabled) {
        left_outer_w = ap__footer_group_outer_width(left_widths, left_count, item_gap, outer_pad, 0);
        right_outer_w = ap__footer_group_outer_width(right_widths, right_count, item_gap, outer_pad, 0);
        left_visible_count = left_count;
        right_visible_start = 0;
        right_visible_count = right_count;
    }

    if (left_outer_w > 0) {
        ap_draw_pill(padding, pill_y, left_outer_w, outer_h, ap__g.theme.accent);
        int cx = padding + outer_pad;
        int btn_y = pill_y + btn_margin; /* buttons inset by BUTTON_MARGIN from pill top */
        for (int i = 0; i < left_visible_count; i++) {
            if (i > 0) cx += item_gap;
            ap__footer_draw_item(&cx, btn_y, inner_h, btn_margin, hint_font_h, hint_font,
                                 &items[left_indices[i]]);
        }

        if (ap__g.footer_hidden_count > 0 && ap__g.footer_overflow_opts.enabled) {
            char marker[16];
            snprintf(marker, sizeof(marker), "+%d", ap__g.footer_hidden_count);
            if (left_visible_count > 0) cx += item_gap;
            ap_draw_text(hint_font, marker,
                         cx + btn_margin,
                         btn_y + (inner_h - hint_font_h) / 2,
                         ap__g.theme.hint);
        }
    }

    if (right_visible_count > 0 && right_outer_w > 0) {
        int rx = ap__g.screen_w - padding - right_outer_w;
        ap_draw_pill(rx, pill_y, right_outer_w, outer_h, ap__g.theme.accent);
        int cx = rx + outer_pad;
        int btn_y = pill_y + btn_margin;
        for (int i = right_visible_start; i < right_count; i++) {
            if (i > right_visible_start) cx += item_gap;
            ap__footer_draw_item(&cx, btn_y, inner_h, btn_margin, hint_font_h, hint_font,
                                 &items[right_indices[i]]);
        }
    }
}

static void ap__footer_overflow_show_hidden_actions(void) {
    if (ap__g.footer_hidden_count <= 0) return;

    TTF_Font *title_font = ap_get_font(AP_FONT_SMALL);
    TTF_Font *body_font = ap_get_font(AP_FONT_TINY);
    TTF_Font *hint_font = ap_get_font(AP_FONT_MICRO);
    if (!body_font) return;

    char text[2048];
    int off = 0;
    for (int i = 0; i < ap__g.footer_hidden_count && off < (int)sizeof(text) - 1; i++) {
        const char *btn_name = ap__footer_button_text(&ap__g.footer_hidden_items[i]);
        const char *label = ap__g.footer_hidden_items[i].label ? ap__g.footer_hidden_items[i].label : "";
        off += snprintf(text + off, sizeof(text) - (size_t)off,
                        "%s  %.120s\n", btn_name, label);
        if (off < 0 || off >= (int)sizeof(text)) {
            text[sizeof(text) - 1] = '\0';
            break;
        }
    }

    ap__g.footer_overflow_overlay_open = true;

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();
    int margin = AP_S(40);
    int title_h = title_font ? TTF_FontLineSkip(title_font) : 0;
    int line_h = TTF_FontLineSkip(body_font);
    int body_y = margin + title_h + AP_S(12);
    int max_w = screen_w - margin * 2;
    int body_h = screen_h - body_y - margin;
    if (body_h < line_h) body_h = line_h;

    int chars_per_line = max_w / AP_S(10);
    if (chars_per_line < 1) chars_per_line = 1;
    int est_lines = ((int)strlen(text) / chars_per_line) + 2;
    int content_h = est_lines * line_h;
    int scroll = 0;
    int max_scroll = content_h - body_h;
    if (max_scroll < 0) max_scroll = 0;

    bool running = true;
    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_UP:
                    scroll -= AP_S(40);
                    if (scroll < 0) scroll = 0;
                    break;
                case AP_BTN_DOWN:
                    scroll += AP_S(40);
                    if (scroll > max_scroll) scroll = max_scroll;
                    break;
                default:
                    running = false;
                    break;
            }
        }

        ap_color overlay_bg = {0, 0, 0, 220};
        ap_draw_rect(0, 0, screen_w, screen_h, overlay_bg);

        if (title_font) {
            ap_draw_text(title_font, "Hidden Actions", margin, margin, theme->text);
        }

        SDL_Rect clip = { margin, body_y, max_w, body_h };
        SDL_RenderSetClipRect(ap_get_renderer(), &clip);
        ap_draw_text_wrapped(body_font, text, margin, body_y - scroll, max_w, theme->text, AP_ALIGN_LEFT);
        SDL_RenderSetClipRect(ap_get_renderer(), NULL);

        if (max_scroll > 0) {
            ap_draw_scrollbar(screen_w - margin + AP_S(8), body_y, body_h, body_h, content_h, scroll);
        }

        if (hint_font) {
            const char *hint = "Press any button to close";
            int hint_w = ap_measure_text(hint_font, hint);
            ap_draw_text(hint_font, hint, (screen_w - hint_w) / 2,
                         screen_h - margin + AP_S(8), theme->hint);
        }

        ap_present();
    }

    ap__g.footer_overflow_overlay_open = false;
}

static bool ap__footer_overflow_consume_open_request(void) {
    if (!ap__g.footer_overflow_open_requested) return false;

    ap__g.footer_overflow_open_requested = false;
    if (!ap__footer_overflow_enabled()) return true;

    ap__footer_overflow_show_hidden_actions();
    ap__g.last_input_time = SDL_GetTicks();
    return true;
}

int ap_get_status_bar_height(void) {
    /* padding + pill_size, matching NextUI */
    return AP_DS(ap__g.device_padding + AP__PILL_SIZE);
}

/* ─── Device / Preview status helpers ────────────────────────────────────── */

#if AP_PLATFORM_IS_DEVICE
static int ap__read_sysfs_int(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    int val = -1;
    if (fscanf(f, "%d", &val) != 1) val = -1;
    fclose(f);
    return val;
}
#endif

static int ap__get_battery_percent(void) {
#if AP_PLATFORM_IS_DEVICE
    int cap;
    #if defined(PLATFORM_MY355)
    cap = ap__read_sysfs_int("/sys/class/power_supply/battery/capacity");
    #else /* tg5040, tg5050 */
    cap = ap__read_sysfs_int("/sys/class/power_supply/axp2202-battery/capacity");
    #endif
    return (cap >= 0 && cap <= 100) ? cap : -1;
#else
    int cap = -1;
    if (!ap__env_parse_int("AP_PREVIEW_BATTERY_PERCENT", &cap)) return -1;
    return ap__clamp(cap, 0, 100);
#endif
}

static bool ap__is_charging(void) {
#if AP_PLATFORM_IS_DEVICE
    int charger, ttf;
    #if defined(PLATFORM_MY355)
    charger = ap__read_sysfs_int("/sys/class/power_supply/ac/online");
    ttf     = ap__read_sysfs_int("/sys/class/power_supply/battery/time_to_full_now");
    #else
    charger = ap__read_sysfs_int("/sys/class/power_supply/axp2202-usb/online");
    ttf     = ap__read_sysfs_int("/sys/class/power_supply/axp2202-battery/time_to_full_now");
    #endif
    return (charger == 1) && (ttf > 0);
#else
    int charging = 0;
    return ap__env_parse_int("AP_PREVIEW_CHARGING", &charging) && charging != 0;
#endif
}

/* ─── CPU & Fan sysfs paths and frequency presets ───────────────────────── */

#if AP_PLATFORM_IS_DEVICE
#if defined(PLATFORM_MY355)
#  define AP__CPU_GOVERNOR_PATH  "/sys/devices/system/cpu/cpufreq/policy0/scaling_governor"
#  define AP__CPU_SPEED_PATH     "/sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed"
#  define AP__CPU_CUR_FREQ_PATH  "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"
#  define AP__CPU_FREQ_MENU         600000
#  define AP__CPU_FREQ_POWERSAVE   1200000
#  define AP__CPU_FREQ_NORMAL      1608000
#  define AP__CPU_FREQ_PERFORMANCE 2000000
#elif defined(PLATFORM_TG5040)
#  define AP__CPU_GOVERNOR_PATH  "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#  define AP__CPU_SPEED_PATH     "/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed"
#  define AP__CPU_CUR_FREQ_PATH  "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"
#  define AP__CPU_FREQ_MENU         600000
#  define AP__CPU_FREQ_POWERSAVE   1200000
#  define AP__CPU_FREQ_NORMAL      1608000
#  define AP__CPU_FREQ_PERFORMANCE 2000000
#elif defined(PLATFORM_TG5050)
#  define AP__CPU_GOVERNOR_PATH  "/sys/devices/system/cpu/cpu4/cpufreq/scaling_governor"
#  define AP__CPU_SPEED_PATH     "/sys/devices/system/cpu/cpu4/cpufreq/scaling_setspeed"
#  define AP__CPU_CUR_FREQ_PATH  "/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq"
#  define AP__CPU_FREQ_MENU         672000
#  define AP__CPU_FREQ_POWERSAVE   1200000
#  define AP__CPU_FREQ_NORMAL      1680000
#  define AP__CPU_FREQ_PERFORMANCE 2160000
#  define AP__FAN_STATE_PATH     "/sys/class/thermal/cooling_device0/cur_state"
#  define AP__FAN_HELPER_PATH    "/mnt/SDCARD/.system/tg5050/bin/fancontrol"
#  define AP__FAN_LOCK_PATH      "/var/run/fan-control.lock"
#endif

#define AP__CPU_TEMP_PATH "/sys/devices/virtual/thermal/thermal_zone0/temp"

static int ap__write_sysfs_int(const char *path, int value) {
    FILE *f = fopen(path, "w");
    if (!f) return AP_ERROR;
    fprintf(f, "%d\n", value);
    fclose(f);
    return AP_OK;
}

static int ap__write_sysfs_str(const char *path, const char *value) {
    FILE *f = fopen(path, "w");
    if (!f) return AP_ERROR;
    fputs(value, f);
    fclose(f);
    return AP_OK;
}

#if defined(PLATFORM_TG5050) && defined(AP__FAN_STATE_PATH)
static bool ap__is_all_digits(const char *s) {
    if (!s || !s[0]) return false;
    for (const char *p = s; *p; ++p) {
        if (*p < '0' || *p > '9') return false;
    }
    return true;
}

static const char *ap__fan_mode_arg(ap_fan_mode mode) {
    switch (mode) {
        case AP_FAN_MODE_AUTO_QUIET: return "quiet";
        case AP_FAN_MODE_AUTO_NORMAL: return "normal";
        case AP_FAN_MODE_AUTO_PERFORMANCE: return "performance";
        default: return NULL;
    }
}

static int ap__fan_stop_helper(void) {
    (void)system("killall fancontrol 2>/dev/null");
    usleep(100000);
    unlink(AP__FAN_LOCK_PATH);
    return AP_OK;
}

static bool ap__fan_helper_available(void) {
    return access(AP__FAN_HELPER_PATH, X_OK) == 0;
}

static int ap__fan_launch_helper(const char *arg) {
    char command[256];
    if (!arg || !arg[0]) return AP_ERROR;
    snprintf(command, sizeof(command), "%s %s >/dev/null 2>&1 &", AP__FAN_HELPER_PATH, arg);
    return system(command) == 0 ? AP_OK : AP_ERROR;
}

static ap_fan_mode ap__fan_detect_helper_mode(void) {
    DIR *dir = opendir("/proc");
    if (!dir) return AP_FAN_MODE_UNSUPPORTED;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (!ap__is_all_digits(ent->d_name)) continue;

        char path[sizeof("/proc//cmdline") + sizeof(ent->d_name)];
        snprintf(path, sizeof(path), "/proc/%s/cmdline", ent->d_name);

        FILE *f = fopen(path, "r");
        if (!f) continue;

        char cmdline[256];
        size_t n = fread(cmdline, 1, sizeof(cmdline) - 1, f);
        fclose(f);
        if (n == 0) continue;
        cmdline[n] = '\0';

        const char *arg = cmdline;
        const char *found = NULL;
        const char *next = NULL;
        while (arg < cmdline + n && *arg) {
            const char *base = strrchr(arg, '/');
            base = base ? base + 1 : arg;
            if (strcmp(base, "fancontrol") == 0) {
                found = arg;
                next = arg + strlen(arg) + 1;
                break;
            }
            arg += strlen(arg) + 1;
        }
        if (!found) continue;

        if (next >= cmdline + n || !*next) {
            closedir(dir);
            return AP_FAN_MODE_MANUAL;
        }

        if (strcmp(next, "quiet") == 0) {
            closedir(dir);
            return AP_FAN_MODE_AUTO_QUIET;
        }
        if (strcmp(next, "normal") == 0) {
            closedir(dir);
            return AP_FAN_MODE_AUTO_NORMAL;
        }
        if (strcmp(next, "performance") == 0) {
            closedir(dir);
            return AP_FAN_MODE_AUTO_PERFORMANCE;
        }

        closedir(dir);
        return AP_FAN_MODE_MANUAL;
    }

    closedir(dir);
    return AP_FAN_MODE_UNSUPPORTED;
}
#endif

#endif /* AP_PLATFORM_IS_DEVICE */

/* ─── CPU & Fan implementations ──────────────────────────────────────────── */

int ap_set_cpu_speed(ap_cpu_speed speed) {
#if AP_PLATFORM_IS_DEVICE && defined(AP__CPU_SPEED_PATH)
    int freq = 0;
    switch (speed) {
        case AP_CPU_SPEED_MENU:        freq = AP__CPU_FREQ_MENU;        break;
        case AP_CPU_SPEED_POWERSAVE:   freq = AP__CPU_FREQ_POWERSAVE;   break;
        case AP_CPU_SPEED_NORMAL:      freq = AP__CPU_FREQ_NORMAL;      break;
        case AP_CPU_SPEED_PERFORMANCE: freq = AP__CPU_FREQ_PERFORMANCE; break;
        default: return AP_ERROR;
    }
    ap__write_sysfs_str(AP__CPU_GOVERNOR_PATH, "userspace\n");
    return ap__write_sysfs_int(AP__CPU_SPEED_PATH, freq);
#else
    (void)speed;
    return AP_OK;
#endif
}

int ap_get_cpu_speed_mhz(void) {
#if AP_PLATFORM_IS_DEVICE && defined(AP__CPU_CUR_FREQ_PATH)
    int khz = ap__read_sysfs_int(AP__CPU_CUR_FREQ_PATH);
    return (khz > 0) ? khz / 1000 : -1;
#else
    return -1;
#endif
}

int ap_get_cpu_temp_celsius(void) {
#if AP_PLATFORM_IS_DEVICE
    int mk = ap__read_sysfs_int(AP__CPU_TEMP_PATH);
    return (mk > 0) ? mk / 1000 : -1;
#else
    return -1;
#endif
}

int ap_set_fan_mode(ap_fan_mode mode) {
#if defined(PLATFORM_TG5050) && defined(AP__FAN_STATE_PATH)
    const char *arg;
    if (mode == AP_FAN_MODE_MANUAL) return ap__fan_stop_helper();

    arg = ap__fan_mode_arg(mode);
    if (!arg) return AP_ERROR;

    ap__fan_stop_helper();
    if (!ap__fan_helper_available()) return AP_ERROR;
    return ap__fan_launch_helper(arg);
#else
    (void)mode;
    return AP_OK;
#endif
}

ap_fan_mode ap_get_fan_mode(void) {
#if defined(PLATFORM_TG5050) && defined(AP__FAN_STATE_PATH)
    ap_fan_mode mode = ap__fan_detect_helper_mode();
    if (mode != AP_FAN_MODE_UNSUPPORTED) return mode;
    return ap__read_sysfs_int(AP__FAN_STATE_PATH) >= 0 ? AP_FAN_MODE_MANUAL : AP_FAN_MODE_UNSUPPORTED;
#else
    return AP_FAN_MODE_UNSUPPORTED;
#endif
}

int ap_set_fan_speed(int percent) {
#if defined(PLATFORM_TG5050) && defined(AP__FAN_STATE_PATH)
    if (percent < 0) return AP_OK; /* -1 = keep current */
    if (percent > 100) percent = 100;
    ap__fan_stop_helper();
    if (ap__fan_helper_available()) {
        char arg[16];
        snprintf(arg, sizeof(arg), "%d", percent);
        if (ap__fan_launch_helper(arg) == AP_OK) return AP_OK;
    }
    return ap__write_sysfs_int(AP__FAN_STATE_PATH, (31 * percent + 50) / 100);
#else
    (void)percent;
    return AP_OK;
#endif
}

int ap_get_fan_speed(void) {
#if defined(PLATFORM_TG5050) && defined(AP__FAN_STATE_PATH)
    int raw = ap__read_sysfs_int(AP__FAN_STATE_PATH);
    return (raw >= 0) ? raw * 100 / 31 : -1;
#else
    return 0;
#endif
}

#if AP_PLATFORM_IS_DEVICE
    #define AP__WIFI_CACHE_TTL_MS 5000  /* Match NextUI's 5-second polling interval */

    static int ap__map_rssi_to_wifi_strength(int rssi) {
        /* Match NextUI thresholds:
           0   = disconnected
           -60 = high
           -70 = med
           else low */
        if (rssi == 0)    return 0;
        if (rssi >= -60)  return 3;
        if (rssi >= -70)  return 2;
        return 1;
    }

    static int ap__read_wifi_rssi_iw(void) {
        const int unavailable = -10000;
        FILE *f = popen("iw dev wlan0 link 2>/dev/null", "r");
        if (!f) return unavailable;

        char line[256];
        int rssi = unavailable;
        bool disconnected = false;

        while (fgets(line, sizeof(line), f)) {
            int val;
            if (sscanf(line, " signal: %d dBm", &val) == 1 ||
                sscanf(line, "signal: %d dBm", &val) == 1) {
                rssi = val;
                break;
            }
            if (strstr(line, "Not connected") != NULL) {
                disconnected = true;
            }
        }

        int rc = pclose(f);
        if (rssi != unavailable) return rssi;
        if (disconnected) return 0;
        if (rc == -1) return unavailable;
        if (WIFEXITED(rc) && WEXITSTATUS(rc) != 0) return unavailable;
        return unavailable;
    }

    static int ap__read_wifi_rssi_wpa_cli(void) {
        const int unavailable = -10000;
        static const char *cmds[] = {
            "wpa_cli -p /var/run/wpa_supplicant -i wlan0 signal_poll 2>/dev/null",
            "/usr/sbin/wpa_cli -p /var/run/wpa_supplicant -i wlan0 signal_poll 2>/dev/null",
            "wpa_cli -p /etc/wifi/sockets -i wlan0 signal_poll 2>/dev/null",
            "/usr/sbin/wpa_cli -p /etc/wifi/sockets -i wlan0 signal_poll 2>/dev/null",
            "wpa_cli -i wlan0 signal_poll 2>/dev/null",
            "/usr/sbin/wpa_cli -i wlan0 signal_poll 2>/dev/null",
            NULL
        };

        for (int i = 0; cmds[i]; i++) {
            FILE *f = popen(cmds[i], "r");
            if (!f) continue;

            int rssi = unavailable;
            char line[256];
            while (fgets(line, sizeof(line), f)) {
                int val;
                if (sscanf(line, "RSSI=%d", &val) == 1) {
                    rssi = val;
                    break;
                }
            }

            int rc = pclose(f);
            if (rssi != unavailable) return rssi;
            if (rc == -1) continue;
            if (WIFEXITED(rc) && WEXITSTATUS(rc) == 0) {
                /* Command worked but did not report RSSI.
                   Treat as disconnected rather than unavailable. */
                return 0;
            }
        }

        return unavailable;
    }
#endif

/* Returns wifi signal strength: 0=off, 1=low, 2=med, 3=high
   Uses iw first; falls back to wpa_cli signal_poll on my355 where iw may be unavailable.
   Results are cached for AP__WIFI_CACHE_TTL_MS to match NextUI's polling cadence. */
static int ap__get_wifi_strength(void) {
#if AP_PLATFORM_IS_DEVICE
    uint32_t now = SDL_GetTicks();

    /* Return cached value if within TTL window */
    if (ap__g.wifi_cache_time_ms != 0 &&
        (now - ap__g.wifi_cache_time_ms) < AP__WIFI_CACHE_TTL_MS) {
        return ap__g.cached_wifi_strength;
    }

    const int unavailable = -10000;
    int result;

    /* Check if interface is up */
    FILE *f = fopen("/sys/class/net/wlan0/operstate", "r");
    if (!f) { result = 0; goto cache; }
    char state[16] = {0};
    if (fgets(state, sizeof(state), f)) {
        char *nl = strchr(state, '\n');
        if (nl) *nl = '\0';
    }
    fclose(f);
    if (strcmp(state, "up") != 0) { result = 0; goto cache; }

    /* Read signal via iw first, then wpa_cli fallback */
    int rssi = ap__read_wifi_rssi_iw();
    if (rssi == unavailable) {
        rssi = ap__read_wifi_rssi_wpa_cli();
    }

    /* Keep icon visible when interface is up but RSSI source is unavailable */
    result = (rssi == unavailable) ? 1 : ap__map_rssi_to_wifi_strength(rssi);

cache:
    ap__g.cached_wifi_strength = result;
    ap__g.wifi_cache_time_ms = now;
    return result;
#else
    int strength = 0;
    if (!ap__env_parse_int("AP_PREVIEW_WIFI_STRENGTH", &strength)) return 0;
    return ap__clamp(strength, 0, 3);
#endif
}

/* ─── Status bar icon blitting ───────────────────────────────────────────── */

/* Blit a colored icon from the NextUI asset spritesheet.
   src_x/y/w/h are at 1x scale; they are multiplied by status_asset_scale. */
static void ap__blit_status_icon(int src_x, int src_y, int src_w, int src_h,
                                  int dst_x, int dst_y, int dst_w, int dst_h,
                                  ap_color tint) {
    if (!ap__g.status_assets) return;
    int s = ap__g.status_asset_scale;
    SDL_Rect src = { src_x * s, src_y * s, src_w * s, src_h * s };
    SDL_Rect dst = { dst_x, dst_y, dst_w, dst_h };
    SDL_SetTextureColorMod(ap__g.status_assets, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(ap__g.status_assets, tint.a);
    SDL_RenderCopy(ap__g.renderer, ap__g.status_assets, &src, &dst);
}

/* ─── Status bar ─────────────────────────────────────────────────────────── */

/* Icon sizes at 1x logical (from NextUI spritesheet).
   Actual pixel size = these × status_asset_scale (matched to loaded assets@Nx.png). */
#define AP__BATTERY_W  17
#define AP__BATTERY_H  10
#define AP__WIFI_SIZE  12

/* Helper: icon pixel size using the loaded spritesheet scale (1:1, no GPU upscaling) */
#define AP__ICON_PX(logical) ((logical) * ap__g.status_asset_scale)

typedef struct {
    bool use_sprite_layout;
    bool wifi_visible;
    bool battery_visible;
    bool clock_visible;
    bool clock_24h;
    int  wifi_strength;
    int  visible_icon_count;
    bool single_icon_sprite_mode;
} ap__status_bar_layout;

/* Resolve status-bar visibility once so width and draw logic stay in sync. */
static inline ap__status_bar_layout ap__resolve_status_bar_layout(const ap_status_bar_opts *opts) {
    ap__status_bar_layout layout = {0};
    if (!opts) return layout;

    layout.use_sprite_layout = (ap__g.status_assets != NULL);
    layout.battery_visible   = opts->show_battery;
    layout.clock_24h         = opts->use_24h;

    if (opts->show_wifi) {
        layout.wifi_strength = ap__get_wifi_strength();
        layout.wifi_visible  = (layout.wifi_strength > 0);
    }

    if (opts->show_clock == AP_CLOCK_SHOW) {
        layout.clock_visible = true;
    } else if (opts->show_clock == AP_CLOCK_AUTO) {
        layout.clock_visible = ap__read_nextui_setting_int("showclock", 0) != 0;
        if (layout.clock_visible)
            layout.clock_24h = ap__read_nextui_setting_int("clock24h", 1) != 0;
    }

    if (layout.wifi_visible)    layout.visible_icon_count++;
    if (layout.battery_visible) layout.visible_icon_count++;

    layout.single_icon_sprite_mode =
        layout.use_sprite_layout && !layout.clock_visible && layout.visible_icon_count == 1;

    return layout;
}

static int ap__measure_status_bar_width(const ap_status_bar_opts *opts, TTF_Font *font,
                                        const ap__status_bar_layout *layout) {
    if (!opts || !layout) return 0;

    int s = ap__g.device_scale ? ap__g.device_scale : 2;
    int margin = AP_DS(AP__BUTTON_MARGIN);
    int total_w = margin;
    bool has_any = false;

    if (layout->wifi_visible) {
        int wifi_w = layout->use_sprite_layout ? (AP__WIFI_SIZE * s)
                                               : (font ? ap_measure_text(font, "WiFi") : AP__WIFI_SIZE * s);
        total_w += wifi_w + margin;
        has_any = true;
    }

    if (layout->battery_visible) {
        int battery_w = layout->use_sprite_layout ? (AP__BATTERY_W * s)
                                                  : (font ? ap_measure_text(font, "BAT") : AP__BATTERY_W * s);
        total_w += battery_w + margin;
        has_any = true;
    }

    if (layout->clock_visible && font) {
        char clock_text[32];
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        if (layout->clock_24h)
            strftime(clock_text, sizeof(clock_text), "%H:%M", t);
        else
            strftime(clock_text, sizeof(clock_text), "%I:%M %p", t);
        total_w += ap_measure_text(font, clock_text) + margin;
        has_any = true;
    }

    if (!has_any) return 0;
    if (layout->single_icon_sprite_mode) return AP_DS(AP__PILL_SIZE);
    return total_w;
}

static void ap__draw_status_bar_wifi_sprite(int x, int y, int wifi_strength) {
    int s = ap__g.device_scale ? ap__g.device_scale : 2;
    int iw = AP__WIFI_SIZE * s;
    int ih = AP__WIFI_SIZE * s;
    int sx;

    switch (wifi_strength) {
        case 3:  sx = 1;  break;  /* high */
        case 2:  sx = 14; break;  /* med */
        case 1:  sx = 27; break;  /* low */
        default: sx = 40; break;  /* off/disconnected */
    }

    ap__blit_status_icon(sx, 104, AP__WIFI_SIZE, AP__WIFI_SIZE,
                         x, y, iw, ih, ap__g.theme.hint);
}

static void ap__draw_status_bar_battery_sprite(int x, int y, TTF_Font *font) {
    int s = ap__g.device_scale ? ap__g.device_scale : 2;
    int iw = AP__BATTERY_W * s;
    int ih = AP__BATTERY_H * s;
    int bat = ap__get_battery_percent();
    bool charging = ap__is_charging();
    bool show_pct = ap__read_nextui_setting_int("batteryperc", 0) && !charging;

    if (charging) {
        ap__blit_status_icon(47, 51, AP__BATTERY_W, AP__BATTERY_H,
                             x, y, iw, ih, ap__g.theme.hint);
        ap__blit_status_icon(81, 41, 12, 6,
                             x + 3 * s, y + 2 * s,
                             12 * s, 6 * s, ap__g.theme.hint);
    } else {
        bool low = (bat >= 0 && bat <= 10);
        ap__blit_status_icon(low ? 66 : 47, 51, AP__BATTERY_W, AP__BATTERY_H,
                             x, y, iw, ih, ap__g.theme.hint);
        if (!show_pct && bat >= 0) {
            int fill_src_x = (bat <= 20) ? 1 : 81;
            int fill_src_y = (bat <= 20) ? 55 : 33;
            int fill_full_w = 12 * s;
            int fill_h_px = 6 * s;
            int fill_w = fill_full_w * bat / 100;
            int clip_off = fill_full_w - fill_w;
            if (fill_w > 0) {
                SDL_Rect fsrc = { fill_src_x * s + clip_off, fill_src_y * s, fill_w, fill_h_px };
                SDL_Rect fdst = { x + 3 * s + clip_off, y + 2 * s, fill_w, fill_h_px };
                SDL_SetTextureColorMod(ap__g.status_assets, ap__g.theme.hint.r, ap__g.theme.hint.g, ap__g.theme.hint.b);
                SDL_SetTextureAlphaMod(ap__g.status_assets, ap__g.theme.hint.a);
                SDL_RenderCopy(ap__g.renderer, ap__g.status_assets, &fsrc, &fdst);
            }
        }
    }

    if (show_pct && bat >= 0) {
        char pct_text[8];
        TTF_Font *micro = ap_get_font(AP_FONT_MICRO);
        if (!micro) micro = font;
        snprintf(pct_text, sizeof(pct_text), "%d", bat);
        if (micro) {
            int tw = ap_measure_text(micro, pct_text);
            int th = TTF_FontHeight(micro);
            int tx = x + (iw - tw) / 2 + 1;
            int ty = y + (ih - th) / 2 - 1;
            ap_draw_text(micro, pct_text, tx, ty, ap__g.theme.hint);
        }
    }
}

/* Calculate the rendered pixel width of the status bar pill.
   Layout matches NextUI's GFX_blitHardwareGroup: BUTTON_MARGIN between each element. */
int ap_get_status_bar_width(ap_status_bar_opts *opts) {
    if (!opts) return 0;

    TTF_Font *font = ap_get_font(AP_FONT_SMALL);
    ap__status_bar_layout layout = ap__resolve_status_bar_layout(opts);
    return ap__measure_status_bar_width(opts, font, &layout);
}

void ap_draw_status_bar(ap_status_bar_opts *opts) {
    if (!opts) return;

    TTF_Font *font = ap_get_font(AP_FONT_SMALL);
    if (!font) return;

    int padding = AP_DS(ap__g.device_padding); /* outer UI padding */
    int margin = AP_DS(AP__BUTTON_MARGIN);     /* inter-element gap inside pill */
    int s = ap__g.device_scale ? ap__g.device_scale : 2;
    ap__status_bar_layout layout = ap__resolve_status_bar_layout(opts);

    int pill_w = ap__measure_status_bar_width(opts, font, &layout);
    if (pill_w <= 0) return;

    int pill_h = AP_DS(AP__PILL_SIZE);
    int pill_y = padding;
    int pill_x = ap__g.screen_w - padding - pill_w;

    ap_draw_pill(pill_x, pill_y, pill_w, pill_h, ap__g.theme.accent);

    if (layout.single_icon_sprite_mode) {
        if (layout.battery_visible) {
            int iw = AP__BATTERY_W * s;
            int ih = AP__BATTERY_H * s;
            int bx = pill_x + (pill_h - (iw + s)) / 2; /* NextUI centers with +FIXED_SCALE fudge */
            int by = pill_y + (pill_h - ih) / 2;
            ap__draw_status_bar_battery_sprite(bx, by, font);
        } else if (layout.wifi_visible) {
            int iw = AP__WIFI_SIZE * s;
            int ih = AP__WIFI_SIZE * s;
            int wx = pill_x + (pill_h - iw) / 2;
            int wy = pill_y + (pill_h - ih) / 2;
            ap__draw_status_bar_wifi_sprite(wx, wy, layout.wifi_strength);
        }
        return;
    }

    /* Multi-element mode: render left-to-right matching NextUI order */
    int cx = pill_x + margin;
    int cy = pill_y;

    /* Wifi icon */
    if (layout.wifi_visible) {
        int iw = AP__WIFI_SIZE * s;
        int ih = AP__WIFI_SIZE * s;
        int iy = cy + (pill_h - ih) / 2;

        if (ap__g.status_assets) {
            ap__draw_status_bar_wifi_sprite(cx, iy, layout.wifi_strength);
            cx += iw + margin;
        } else {
            int text_w = ap_measure_text(font, "WiFi");
            ap_draw_text(font, "WiFi", cx, cy + (pill_h - TTF_FontHeight(font)) / 2, ap__g.theme.hint);
            cx += text_w + margin;
        }
    }

    /* Battery icon */
    if (opts->show_battery) {
        int iw = AP__BATTERY_W * s;
        int ih = AP__BATTERY_H * s;
        int iy = cy + (pill_h - ih) / 2;

        if (ap__g.status_assets) {
            ap__draw_status_bar_battery_sprite(cx, iy, font);
        } else {
            int text_w = ap_measure_text(font, "BAT");
            ap_draw_text(font, "BAT", cx, cy + (pill_h - TTF_FontHeight(font)) / 2, ap__g.theme.hint);
            cx += text_w + margin;
        }
        if (ap__g.status_assets) {
            cx += iw + margin;
        }
    }

    /* Clock (rightmost) */
    {
        if (layout.clock_visible) {
            char clock_text[32];
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            if (layout.clock_24h)
                strftime(clock_text, sizeof(clock_text), "%H:%M", t);
            else
                strftime(clock_text, sizeof(clock_text), "%I:%M %p", t);
            int th = TTF_FontHeight(font);
            int ty = cy + (pill_h - th) / 2;
            ap_draw_text(font, clock_text, cx, ty, ap__g.theme.hint);
        }
    }
}

/* ─── Power Button Handler ───────────────────────────────────────────────── */

#if AP_PLATFORM_IS_DEVICE
#if !defined(PLATFORM_MY355)
static const char **ap__power_input_paths(void) {
    #if defined(PLATFORM_TG5040)
    static const char *paths[] = { "/dev/input/event1", NULL };
    #elif defined(PLATFORM_TG5050)
    static const char *paths[] = { "/dev/input/event2", NULL };
    #else
    static const char *paths[] = { "/dev/input/event1", NULL };
    #endif
    return paths;
}
#endif

/* MY355: scan all /dev/input/event* devices for the one that has KEY_POWER.
   Returns an open fd on success, -1 if no matching device found.
   Uses EVIOCGBIT to query key capabilities, matching the Linux input API. */
#if defined(PLATFORM_MY355)
static int ap__open_power_device_by_capability(void) {
    unsigned char key_bits[(KEY_MAX + 1) / 8];
    for (int i = 0; i < 16; i++) {
        char path[32];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;
        memset(key_bits, 0, sizeof(key_bits));
        if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) >= 0) {
            /* KEY_POWER capability bit */
            if (key_bits[KEY_POWER / 8] & (1 << (KEY_POWER % 8))) {
                ap_log("Power: selected input device %s (KEY_POWER capability)", path);
                return fd;
            }
        }
        close(fd);
    }
    ap_log("Power: no /dev/input/event* device has KEY_POWER capability");
    return -1;
}
#endif

#if defined(PLATFORM_MY355)
static bool ap__is_power_key_code(int code) {
    /* Compatibility: some stacks may still expose power as 102. */
    return code == KEY_POWER || code == 102;
}
#else
static bool ap__is_power_key_code(int code) {
    return code == KEY_POWER;
}
#endif

static void ap__drain_power_events(int fd) {
    if (fd < 0) return;
    struct input_event ev;
    while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_KEY && ap__is_power_key_code((int)ev.code)) {
            ap_log("Power: draining queued key event code=%d value=%d", (int)ev.code, (int)ev.value);
        }
    }
}

static int ap__run_power_command(const char *action, const char *command) {
    errno = 0;
    int rc = system(command);
    int saved_errno = errno;

    if (rc == -1) {
        ap_log("Power: %s command launch failed: cmd='%s' errno=%d (%s)",
               action, command, saved_errno, strerror(saved_errno));
        return rc;
    }

    if (WIFEXITED(rc)) {
        int exit_code = WEXITSTATUS(rc);
        ap_log("Power: %s command exited: cmd='%s' exit=%d", action, command, exit_code);
        return exit_code;
    }

    if (WIFSIGNALED(rc)) {
        ap_log("Power: %s command signaled: cmd='%s' signal=%d", action, command, WTERMSIG(rc));
        return rc;
    }

    ap_log("Power: %s command returned status=%d: cmd='%s'", action, rc, command);
    return rc;
}

static void *ap__power_thread_func(void *arg) {
    (void)arg;

    int fd = -1;

#if defined(PLATFORM_MY355)
    /* Use EVIOCGBIT capability scan to find the device that actually has KEY_POWER */
    fd = ap__open_power_device_by_capability();
#else
    const char **input_paths = ap__power_input_paths();
    const char *opened_path = NULL;
    for (int i = 0; input_paths[i]; i++) {
        fd = open(input_paths[i], O_RDONLY | O_NONBLOCK);
        if (fd >= 0) {
            opened_path = input_paths[i];
            break;
        }
    }
    if (fd >= 0)
        ap_log("Power handler: listening on %s", opened_path ? opened_path : "unknown");
#endif

    if (fd < 0) {
        ap_log("Power handler: could not open input device");
        return NULL;
    }
    ap_log("Power handler: ready (fd=%d)", fd);
    ap__g.power_fd = fd;
    uint32_t ignore_power_until = 0;

    while (ap__g.power_thread_running) {
        struct input_event ev;
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n != sizeof(ev)) {
            SDL_Delay(10);
            continue;
        }

        if (ev.type == EV_KEY && ap__is_power_key_code((int)ev.code)) {
            uint32_t now = SDL_GetTicks();
            if (ignore_power_until && now < ignore_power_until) {
                ap_log("Power: ignoring key event during post-resume guard code=%d value=%d ms_left=%u",
                       (int)ev.code, (int)ev.value, (unsigned)(ignore_power_until - now));
                continue;
            }
            ap_log("Power: key event code=%d value=%d", (int)ev.code, (int)ev.value);
            if (ev.value == 1) { /* Press */
                /* Track press time for short/long detection */
                uint32_t press_start = SDL_GetTicks();
                bool released = false;

                while (ap__g.power_thread_running) {
                    n = read(fd, &ev, sizeof(ev));
                    if (n == sizeof(ev) && ev.type == EV_KEY &&
                        ap__is_power_key_code((int)ev.code) && ev.value == 0) {
                        ap_log("Power: key release code=%d", (int)ev.code);
                        released = true;
                        break;
                    }
                    if (SDL_GetTicks() - press_start > 1000) break;
                    SDL_Delay(10);
                }

                uint32_t held_ms = SDL_GetTicks() - press_start;
                if (held_ms >= 1000) {
                    /* Long press: shutdown (NextUI-style signal) */
                    ap_log("Power: long press → shutdown");
                    system("rm -f /tmp/nextui_exec && sync");
                    system("touch /tmp/poweroff");
                    sync();
                    exit(0);
                } else if (released) {
                    /* Short press: suspend */
                    ap_log("Power: short press → suspend");
                    #if defined(PLATFORM_MY355)
                    int rc = ap__run_power_command("suspend", "echo mem > /sys/power/state");
                    if (rc != 0) {
                        ap_log("Power: suspend mem failed, trying freeze fallback");
                        ap__run_power_command("suspend-fallback", "echo freeze > /sys/power/state");
                    }
                    #elif defined(PLATFORM_TG5040) || defined(PLATFORM_TG5050)
                    {
                        const char *sp = getenv("SYSTEM_PATH");
                        char suspend_cmd[256];
                        #if defined(PLATFORM_TG5040)
                        snprintf(suspend_cmd, sizeof(suspend_cmd), "%s/bin/suspend",
                                 (sp && sp[0]) ? sp : "/mnt/SDCARD/.system/tg5040");
                        #else
                        snprintf(suspend_cmd, sizeof(suspend_cmd), "%s/bin/suspend",
                                 (sp && sp[0]) ? sp : "/mnt/SDCARD/.system/tg5050");
                        #endif
                        ap__run_power_command("suspend", suspend_cmd);
                    }
                    #endif

                    /* Match NextUI behavior: ignore power key for a short time after resume
                       to avoid immediate re-suspend from wake button events. */
                    ap__drain_power_events(fd);
                    ignore_power_until = SDL_GetTicks() + 1000;
                    ap_log("Power: resume guard active for 1000ms");
                }
            }
        }
    }

    int fd_to_close = ap__g.power_fd;
    ap__g.power_fd = -1;
    if (fd_to_close >= 0) close(fd_to_close);
    return NULL;
}
#endif

void ap_set_power_handler(bool enabled) {
    ap__g.power_handler_enabled = enabled;

#if AP_PLATFORM_IS_DEVICE
    if (enabled && !ap__g.power_thread_running) {
        ap_log("Power handler: starting thread");
        ap__g.power_fd = -1;
        ap__g.power_thread_running = true;
        if (pthread_create(&ap__g.power_thread, NULL, ap__power_thread_func, NULL) != 0) {
            ap_log("Power handler: failed to create thread");
            ap__g.power_thread_running = false;
        }
    } else if (!enabled && ap__g.power_thread_running) {
        ap_log("Power handler: stopping thread");
        ap__g.power_thread_running = false;
        /* Thread uses non-blocking reads with a 10ms poll loop, so it will
           notice the flag change quickly.  Let the thread close its own fd
           to avoid racing with a concurrent read(). */
        pthread_join(ap__g.power_thread, NULL);
        ap_log("Power handler: thread stopped");
    }
#endif
}

/* ─── Accessors ──────────────────────────────────────────────────────────── */

SDL_Renderer *ap_get_renderer(void)   { return ap__g.renderer; }
SDL_Window   *ap_get_window(void)     { return ap__g.window; }
void ap_show_window(void) { if (ap__g.window) SDL_ShowWindow(ap__g.window); }
void ap_hide_window(void) { if (ap__g.window) SDL_HideWindow(ap__g.window); }
int           ap_get_screen_width(void)  { return ap__g.screen_w; }
int           ap_get_screen_height(void) { return ap__g.screen_h; }

/* ─── Initialization ─────────────────────────────────────────────────────── */

int ap_init(ap_config *cfg) {
    if (ap__g.initialized) {
        ap__set_error("Already initialized");
        return AP_ERROR;
    }

    memset(&ap__g, 0, sizeof(ap__g));
    #if AP_PLATFORM_IS_DEVICE
    ap__g.power_fd = -1;
    #endif

    /* Logging */
    if (cfg && cfg->log_path) {
        ap_set_log_path(cfg->log_path);
    }

    ap_log("Apostrophe initializing (platform: %s)", AP_PLATFORM_NAME);

    /* Set default theme */
    ap__g.theme = ap__default_theme;

    /* Input defaults */
    ap__g.input_delay_ms = AP_INPUT_DEBOUNCE;
    ap__g.input_repeat_delay_ms = AP_INPUT_REPEAT_DELAY;
    ap__g.input_repeat_rate_ms = AP_INPUT_REPEAT_RATE;
    ap__g.footer_overflow_opts.enabled = true;
    ap__g.footer_overflow_opts.chord_a = AP_BTN_NONE;
    ap__g.footer_overflow_opts.chord_b = AP_BTN_NONE;

    uint32_t sdl_flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS;
    #if !AP_PLATFORM_IS_DEVICE
    sdl_flags |= SDL_INIT_GAMECONTROLLER;
    #endif
    if (SDL_Init(sdl_flags) < 0) {
        ap__set_error("SDL_Init failed: %s", SDL_GetError());
        return AP_ERROR;
    }

    if (TTF_Init() < 0) {
        ap__set_error("TTF_Init failed: %s", TTF_GetError());
        SDL_Quit();
        return AP_ERROR;
    }

    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        ap_log("Warning: SDL_image init incomplete: %s", IMG_GetError());
        /* Non-fatal — some platforms may not support all formats */
    }

    /* Open input devices.
     * On device we prefer raw joystick mapping because SDL GameController
     * DB mappings can remap face buttons in ways that differ from NextUI's
     * expected A/B layout on TrimUI hardware.
     * On device, ALL joysticks are opened (like NextUI's PLAT_initInput) so that
     * SDL receives keyboard/power events from every registered input device. */
    int num_joy = SDL_NumJoysticks();
    #if AP_PLATFORM_IS_DEVICE
    for (int i = 0; i < num_joy; i++) {
        SDL_Joystick *joy = SDL_JoystickOpen(i);
        if (joy) {
            ap_log("Joystick %d opened: %s", i, SDL_JoystickName(joy));
            if (!ap__g.joystick) ap__g.joystick = joy; /* keep first for backward compat */
        }
    }
    #else
    for (int i = 0; i < num_joy; i++) {
        if (SDL_IsGameController(i)) {
            ap__g.controller = SDL_GameControllerOpen(i);
            if (ap__g.controller) {
                ap_log("Game controller opened: %s", SDL_GameControllerName(ap__g.controller));
                /* Also grab underlying joystick for hat/axis fallback */
                ap__g.joystick = SDL_GameControllerGetJoystick(ap__g.controller);
                break;
            }
        }
        ap__g.joystick = SDL_JoystickOpen(i);
        if (ap__g.joystick) {
            ap_log("Joystick opened: %s", SDL_JoystickName(ap__g.joystick));
            break;
        }
    }
    #endif
    ap_log("Input backend: %s",
           ap__g.controller ? "gamecontroller" :
           (ap__g.joystick ? "joystick" : "none"));

    /* Default face-button flip on TrimUI devices (firmware swaps A/B at hardware level) */
#if defined(PLATFORM_TG5040) || defined(PLATFORM_TG5050)
    ap__g.face_buttons_flipped = false;  /* Raw joystick map already accounts for TrimUI swap */
#endif

    /* Determine screen size */
    bool dev_mode = false;
    const char *env_val = getenv("AP_ENV");
    if (!env_val) env_val = getenv("ENVIRONMENT");
    if (env_val && strcmp(env_val, "DEV") == 0) dev_mode = true;

    #if !AP_PLATFORM_IS_DEVICE
    dev_mode = true;
    #endif

    if (dev_mode) {
        /* Windowed mode */
        const char *ww = getenv("AP_WINDOW_WIDTH");
        const char *wh = getenv("AP_WINDOW_HEIGHT");
        ap__g.screen_w = ww ? atoi(ww) : 1024;
        ap__g.screen_h = wh ? atoi(wh) :  768;
    } else {
        /* Fullscreen — get native display resolution */
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
            ap__g.screen_w = dm.w;
            ap__g.screen_h = dm.h;
        } else {
            /* Fallback defaults per platform */
            #if defined(PLATFORM_MY355)
            ap__g.screen_w = 640;
            ap__g.screen_h = 480;
            #else
            ap__g.screen_w = 1280;
            ap__g.screen_h = 720;
            #endif
        }
    }

    ap_log("Screen size: %dx%d (dev_mode=%d)", ap__g.screen_w, ap__g.screen_h, dev_mode);

    /* Compute scale factor */
    ap__compute_scale_factor();
    ap_log("Scale factor: %.3f", ap__g.scale_factor);

    /* Compute device scale & padding before loading fonts.
       Desktop/dev previews now use the same resolution-based profile
       selection as device builds, based on the effective screen size
       (from AP_WINDOW_WIDTH/AP_WINDOW_HEIGHT in dev mode or native
       resolution). */
    ap__resolve_device_metrics();
    ap_log("Device scale: %d, padding: %d", ap__g.device_scale, ap__g.device_padding);

    /* Compute font bump (must be after device_scale, before font loading) */
    if (cfg && cfg->disable_font_bump)
        ap__g.font_bump = 0;
    else
        ap__g.font_bump = ap__compute_font_bump();
    ap_log("Font bump: %d", ap__g.font_bump);

    /* Create window */
    uint32_t win_flags = SDL_WINDOW_SHOWN;
    if (!dev_mode) {
        win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    const char *title = (cfg && cfg->window_title) ? cfg->window_title : "Apostrophe";

    ap__g.window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        ap__g.screen_w, ap__g.screen_h,
        win_flags
    );

    if (!ap__g.window) {
        ap__set_error("SDL_CreateWindow failed: %s", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return AP_ERROR;
    }

    /* Set render quality hint before creating renderer/textures (matches NextUI) */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); /* bilinear filtering */

    /* Create renderer — try HW accelerated first, fall back to software */
    ap__g.renderer = SDL_CreateRenderer(ap__g.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    if (!ap__g.renderer) {
        ap_log("HW renderer failed, trying software: %s", SDL_GetError());
        ap__g.renderer = SDL_CreateRenderer(ap__g.window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!ap__g.renderer) {
        ap__set_error("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(ap__g.window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return AP_ERROR;
    }

    SDL_SetRenderDrawBlendMode(ap__g.renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(ap__g.renderer, ap__g.screen_w, ap__g.screen_h);
    SDL_ShowCursor(SDL_DISABLE);

    SDL_RendererInfo renderer_info;
    if (SDL_GetRendererInfo(ap__g.renderer, &renderer_info) == 0) {
        ap__g.renderer_has_vsync =
            (renderer_info.flags & SDL_RENDERER_PRESENTVSYNC) != 0;
    } else {
        ap__g.renderer_has_vsync = false;
    }
    ap_log("Renderer vsync: %s", ap__g.renderer_has_vsync ? "yes" : "no");

    /* Framebuffer sync workaround — render 3 black frames */
    for (int i = 0; i < 3; i++) {
        SDL_SetRenderDrawColor(ap__g.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ap__g.renderer);
        SDL_RenderPresent(ap__g.renderer);
    }
    ap__g.last_present_ms = SDL_GetTicks();

    /* Load theme from NextUI if requested */
    if (cfg && cfg->is_nextui) {
        ap_theme_load_nextui();
    }

    /* Override accent color if specified */
    if (cfg && cfg->primary_color_hex) {
        ap_set_theme_color(cfg->primary_color_hex);
    }

    /* Load fonts */
    const char *font_path = (cfg && cfg->font_path) ? cfg->font_path : NULL;
    if (ap__load_fonts(font_path) != AP_OK) {
        ap__set_error("Failed to load fonts");
        SDL_DestroyRenderer(ap__g.renderer);
        SDL_DestroyWindow(ap__g.window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return AP_ERROR;
    }

    /* Load background image (on by default unless disabled) */
    if (!cfg || !cfg->disable_background) {
        const char *bg_path = (cfg && cfg->bg_image_path) ? cfg->bg_image_path : NULL;
        if (!bg_path || !bg_path[0]) {
            #if AP_PLATFORM_IS_DEVICE
            bg_path = "/mnt/SDCARD/bg.png";
            #else
            bg_path = getenv("AP_BACKGROUND_PATH");
            #endif
        }

        if (bg_path && bg_path[0]) {
            ap__g.bg_texture = ap_load_image(bg_path);
            if (ap__g.bg_texture) {
                strncpy(ap__g.theme.bg_image_path, bg_path, sizeof(ap__g.theme.bg_image_path) - 1);
                ap_log("Loaded background: %s", bg_path);
            } else {
                ap_log("Warning: could not load background: %s", bg_path);
            }
        }
    }

    /* Load NextUI asset spritesheet for status bar icons / pill caps */
    {
        char assets_dir_buf[256];
        int scale = ap__g.device_scale;
        char asset_path[512];
        const char *assets_dir = ap__status_assets_dir(assets_dir_buf, sizeof(assets_dir_buf));
        if (assets_dir && assets_dir[0]) {
            snprintf(asset_path, sizeof(asset_path), "%s/assets@%dx.png", assets_dir, scale);
            SDL_Surface *surf = IMG_Load(asset_path);
            if (surf) {
                ap__g.status_assets = SDL_CreateTextureFromSurface(ap__g.renderer, surf);
                ap__g.status_asset_scale = scale;
                SDL_SetTextureBlendMode(ap__g.status_assets, SDL_BLENDMODE_BLEND);
                SDL_FreeSurface(surf);
                ap_log("Loaded status assets: %s", asset_path);
            } else {
                ap_log("Warning: could not load status assets: %s", asset_path);
            }
        }
    }

    /* Start power handler on device if NextUI mode */
    if (cfg && cfg->is_nextui) {
        ap_set_power_handler(true);
    }

    /* Apply CPU speed preset if specified */
    if (cfg && cfg->cpu_speed != AP_CPU_SPEED_DEFAULT) {
        if (ap_set_cpu_speed(cfg->cpu_speed) != AP_OK) {
            ap_log("Warning: ap_set_cpu_speed failed for preset %d", (int)cfg->cpu_speed);
        } else {
            ap_log("CPU speed set to preset %d", (int)cfg->cpu_speed);
        }
    }

    ap__g.initialized = true;
    ap_log("Apostrophe initialized successfully");

    return AP_OK;
}

void ap_quit(void) {
    if (!ap__g.initialized) return;

    ap_log("Apostrophe shutting down");

    /* Stop power handler */
    ap_set_power_handler(false);

    /* Clear texture cache */
    ap_cache_clear();

    /* Destroy background texture */
    if (ap__g.bg_texture) {
        SDL_DestroyTexture(ap__g.bg_texture);
        ap__g.bg_texture = NULL;
    }

    /* Destroy status assets */
    if (ap__g.status_assets) {
        SDL_DestroyTexture(ap__g.status_assets);
        ap__g.status_assets = NULL;
    }

    /* Close fonts */
    for (int i = 0; i < AP_FONT_TIER_COUNT; i++) {
        if (ap__g.fonts[i]) {
            TTF_CloseFont(ap__g.fonts[i]);
            ap__g.fonts[i] = NULL;
        }
    }

    /* Close controller / joystick */
    if (ap__g.controller) {
        SDL_GameControllerClose(ap__g.controller);
        ap__g.controller = NULL;
        ap__g.joystick = NULL; /* owned by controller */
    } else if (ap__g.joystick) {
        SDL_JoystickClose(ap__g.joystick);
        ap__g.joystick = NULL;
    }

    /* Destroy renderer and window */
    if (ap__g.renderer) {
        SDL_DestroyRenderer(ap__g.renderer);
        ap__g.renderer = NULL;
    }
    if (ap__g.window) {
        SDL_DestroyWindow(ap__g.window);
        ap__g.window = NULL;
    }

    /* Close log file */
    if (ap__g.log_file && ap__g.log_file != stderr) {
        fclose(ap__g.log_file);
        ap__g.log_file = NULL;
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    ap__g.initialized = false;
}

#endif /* AP_IMPLEMENTATION */
#endif /* APOSTROPHE_H */
