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
 *               my355 (Miyoo Mini Flip), macOS (dev/testing)
 *
 * License: MIT
 * https://github.com/LoveRetro/Apostrophe
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
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>

#ifdef __linux__
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <signal.h>
#endif

#ifdef __APPLE__
#include <pthread.h>
#include <unistd.h>
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

/* Input timing defaults (milliseconds) */
#define AP_INPUT_REPEAT_DELAY  150
#define AP_INPUT_REPEAT_RATE    50
#define AP_INPUT_DEBOUNCE       20
#define AP_AXIS_DEADZONE     16000

/* Text scroll timing */
#define AP_TEXT_SCROLL_SPEED     1
#define AP_TEXT_SCROLL_PAUSE_MS 1000

/* Texture cache capacity */
#define AP_TEXTURE_CACHE_SIZE 8

/* Max combo registrations */
#define AP_MAX_COMBOS 16

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
    AP_FONT_EXTRA_LARGE = 0,  /* Base: 60px at 1024 width */
    AP_FONT_LARGE,             /* Base: 50px */
    AP_FONT_MEDIUM,            /* Base: 44px */
    AP_FONT_SMALL,             /* Base: 34px */
    AP_FONT_TINY,              /* Base: 24px */
    AP_FONT_MICRO,             /* Base: 18px */
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
    bool      pressed;  /* true = down, false = up */
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
} ap_footer_item;

/* Status bar options */
typedef struct {
    bool         show_clock;
    bool         use_24h;
    const char **icons;       /* Unicode icon strings (e.g., Material Design) */
    int          icon_count;
} ap_status_bar_opts;

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

/* Combo registration */
typedef struct {
    char       id[64];
    ap_button  buttons[8];
    int        button_count;
    uint32_t   window_ms;       /* Time window for chord/sequence */
    bool       is_sequence;     /* false = chord (simultaneous), true = sequence */
    bool       strict;          /* For sequences: must be exact order */
    bool       active;
} ap_combo;

/* Combo event */
typedef struct {
    const char *id;
    bool        triggered;
} ap_combo_event;

/* Configuration passed to ap_init() */
typedef struct {
    const char *window_title;     /* Window title (dev mode only) */
    const char *font_path;        /* Path to .ttf font file, NULL = auto */
    const char *bg_image_path;    /* Background image path, NULL = none */
    const char *log_path;         /* Log file path, NULL = stderr only */
    const char *primary_color_hex;/* Override accent color "#RRGGBB", NULL = theme default */
    bool        show_background;  /* Render background image behind UI */
    bool        is_nextui;        /* Load theme from NextUI's nextval.elf */
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

    /* Button held state for chords */
    bool          buttons_held[AP_BTN_COUNT];
    uint32_t      button_press_time[AP_BTN_COUNT];
    uint32_t      button_repeat_time[AP_BTN_COUNT];

    /* Texture cache */
    ap_texture_cache tex_cache;

    /* Logging */
    FILE         *log_file;

    /* Power button handling */
    bool          power_handler_enabled;
    #if AP_PLATFORM_IS_DEVICE
    pthread_t     power_thread;
    bool          power_thread_running;
    #endif

    /* Initialization flag */
    bool          initialized;
} ap__state;

/* ═══════════════════════════════════════════════════════════════════════════
 * Macros
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Scale a design-space value to screen space */
#define AP_S(base) ((int)((base) * ap__g.scale_factor))

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Lifecycle
 * ═══════════════════════════════════════════════════════════════════════════ */

int            ap_init(ap_config *cfg);
void           ap_quit(void);
SDL_Renderer  *ap_get_renderer(void);
SDL_Window    *ap_get_window(void);
int            ap_get_screen_width(void);
int            ap_get_screen_height(void);

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
ap_color       ap_hex_to_color(const char *hex);
void           ap_set_theme_color(const char *hex);

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API — Fonts
 * ═══════════════════════════════════════════════════════════════════════════ */

TTF_Font      *ap_get_font(ap_font_tier tier);

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
int            ap_draw_text(TTF_Font *font, const char *text, int x, int y, ap_color color);
int            ap_draw_text_clipped(TTF_Font *font, const char *text, int x, int y, ap_color color, int max_w);
void           ap_draw_text_wrapped(TTF_Font *font, const char *text, int x, int y, int max_w, ap_color color, ap_text_align align);
int            ap_measure_text(TTF_Font *font, const char *text);
void           ap_draw_image(SDL_Texture *tex, int x, int y, int w, int h);
SDL_Texture   *ap_load_image(const char *path);
void           ap_draw_scrollbar(int x, int y, int h, int visible, int total, int offset);
void           ap_draw_progress_bar(int x, int y, int w, int h, float progress, ap_color fg, ap_color bg);

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
 * Public API — Error Handling
 * ═══════════════════════════════════════════════════════════════════════════ */

const char    *ap_get_error(void);
bool           ap_is_cancelled(int result);

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

static int ap__max(int a, int b) { return a > b ? a : b; }

/* Base font sizes at 1024px reference width */
static const int ap__font_base_sizes[AP_FONT_TIER_COUNT] = {
    60, /* EXTRA_LARGE */
    50, /* LARGE */
    44, /* MEDIUM */
    34, /* SMALL */
    24, /* TINY */
    18, /* MICRO */
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

/* Font search paths by platform */
static const char *ap__font_search_paths[] = {
    "./font.ttf",
    "./res/font.ttf",
    "../res/font.ttf",
#if defined(PLATFORM_TG5040) || defined(PLATFORM_TG5050)
    "/mnt/SDCARD/.system/res/font.ttf",
    "/mnt/SDCARD/.system/tg5040/res/font.ttf",
#elif defined(PLATFORM_MY355)
    "/mnt/SDCARD/.system/res/font.ttf",
    "/mnt/SDCARD/.system/my355/res/font.ttf",
#elif defined(PLATFORM_MAC)
    "/System/Library/Fonts/Helvetica.ttc",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
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

/* my355 (Miyoo Mini Flip) keyboard scancode mapping.
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
    "None", "Up", "Down", "Left", "Right",
    "A", "B", "X", "Y",
    "L1", "L2", "R1", "R2",
    "Start", "Select", "Menu", "Power"
};

/* ─── Logging ────────────────────────────────────────────────────────────── */

static bool ap__same_output_file(FILE *a, FILE *b) {
    if (!a || !b) return false;
    int a_fd = fileno(a);
    int b_fd = fileno(b);
    if (a_fd < 0 || b_fd < 0) return false;

    struct stat a_st, b_st;
    if (fstat(a_fd, &a_st) != 0) return false;
    if (fstat(b_fd, &b_st) != 0) return false;
    return a_st.st_dev == b_st.st_dev && a_st.st_ino == b_st.st_ino;
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

int ap_theme_load_nextui(void) {
#if AP_PLATFORM_IS_DEVICE
    /* Look for nextval.elf in the system path */
    const char *nextval_paths[] = {
    #if defined(PLATFORM_TG5040)
        "/mnt/SDCARD/.system/tg5040/bin/nextval.elf",
    #elif defined(PLATFORM_TG5050)
        "/mnt/SDCARD/.system/tg5050/bin/nextval.elf",
    #elif defined(PLATFORM_MY355)
        "/mnt/SDCARD/.system/my355/bin/nextval.elf",
    #endif
        NULL,
    };

    const char *nextval_path = NULL;
    for (int i = 0; nextval_paths[i]; i++) {
        if (access(nextval_paths[i], X_OK) == 0) {
            nextval_path = nextval_paths[i];
            break;
        }
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
    ap__json_copy_string(json, "bgcolor", bg_buf, sizeof(bg_buf));

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
    fread(json, 1, sizeof(json) - 1, fp);
    fclose(fp);

    char c1_buf[32]={0}, c2_buf[32]={0}, c3_buf[32]={0}, c4_buf[32]={0};
    char c5_buf[32]={0}, c6_buf[32]={0}, bg_buf[32]={0};
    ap__json_copy_string(json, "color1", c1_buf, sizeof(c1_buf));
    ap__json_copy_string(json, "color2", c2_buf, sizeof(c2_buf));
    ap__json_copy_string(json, "color3", c3_buf, sizeof(c3_buf));
    ap__json_copy_string(json, "color4", c4_buf, sizeof(c4_buf));
    ap__json_copy_string(json, "color5", c5_buf, sizeof(c5_buf));
    ap__json_copy_string(json, "color6", c6_buf, sizeof(c6_buf));
    ap__json_copy_string(json, "bgcolor", bg_buf, sizeof(bg_buf));

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

/* ─── Scaling ────────────────────────────────────────────────────────────── */

float ap_get_scale_factor(void) {
    return ap__g.scale_factor;
}

int ap_scale(int base) {
    return (int)(base * ap__g.scale_factor);
}

int ap_font_size_for_resolution(int base_size) {
    return (int)(base_size * ap__g.scale_factor);
}

static void ap__compute_scale_factor(void) {
    float raw = (float)ap__g.screen_w / (float)AP_REFERENCE_WIDTH;
    if (raw > 1.0f)
        ap__g.scale_factor = 1.0f + (raw - 1.0f) * AP_SCALE_DAMPING;
    else
        ap__g.scale_factor = raw;
}

/* ─── Font Management ────────────────────────────────────────────────────── */

static TTF_Font *ap__open_font(const char *path, int size) {
    TTF_Font *f = TTF_OpenFont(path, size);
    return f;
}

static int ap__load_fonts(const char *user_font_path) {
    /* Determine font path */
    const char *font_path = NULL;

    /* 1. User-specified path */
    if (user_font_path && user_font_path[0]) {
        if (access(user_font_path, R_OK) == 0) {
            font_path = user_font_path;
        }
    }

    /* 2. Search fallback paths */
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

    /* Open at each tier size */
    for (int i = 0; i < AP_FONT_TIER_COUNT; i++) {
        int size = ap_font_size_for_resolution(ap__font_base_sizes[i]);
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

/* ─── Input System ───────────────────────────────────────────────────────── */

/* Map SDL joystick button to virtual button (raw joystick — used on TrimUI) */
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

/* Map SDL GameController button to virtual button (used on macOS / when SDL
 * recognises the device as a standard game controller) */
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

/* Map SDL keyboard to virtual button.
 * On my355 we match by scancode (the Flip sends buttons as keyboard HID scancodes).
 * On all other platforms we match by keycode for developer convenience. */
#if defined(PLATFORM_MY355)
static ap_button ap__map_key_event(SDL_KeyboardEvent *kev) {
    uint8_t sc = (uint8_t)kev->keysym.scancode;
    switch (sc) {
        case AP__MY355_CODE_UP:     return AP_BTN_UP;
        case AP__MY355_CODE_DOWN:   return AP_BTN_DOWN;
        case AP__MY355_CODE_LEFT:   return AP_BTN_LEFT;
        case AP__MY355_CODE_RIGHT:  return AP_BTN_RIGHT;
        case AP__MY355_CODE_A:      return AP_BTN_A;
        case AP__MY355_CODE_B:      return AP_BTN_B;
        case AP__MY355_CODE_X:      return AP_BTN_X;
        case AP__MY355_CODE_Y:      return AP_BTN_Y;
        case AP__MY355_CODE_L1:     return AP_BTN_L1;
        case AP__MY355_CODE_R1:     return AP_BTN_R1;
        case AP__MY355_CODE_L2:     return AP_BTN_L2;
        case AP__MY355_CODE_R2:     return AP_BTN_R2;
        case AP__MY355_CODE_START:  return AP_BTN_START;
        case AP__MY355_CODE_SELECT: return AP_BTN_SELECT;
        case AP__MY355_CODE_MENU:   return AP_BTN_MENU;
        case AP__MY355_CODE_POWER:  return AP_BTN_POWER;
        default:                    return AP_BTN_NONE;
    }
}
#else
static ap_button ap__map_key_event(SDL_KeyboardEvent *kev) {
    /* Match Gabagool DefaultInputMapping() — letter keys for face buttons */
    switch (kev->keysym.sym) {
        case SDLK_UP:        return AP_BTN_UP;
        case SDLK_DOWN:      return AP_BTN_DOWN;
        case SDLK_LEFT:      return AP_BTN_LEFT;
        case SDLK_RIGHT:     return AP_BTN_RIGHT;
        case SDLK_a:         return AP_BTN_A;
        case SDLK_b:         return AP_BTN_B;
        case SDLK_x:         return AP_BTN_X;
        case SDLK_y:         return AP_BTN_Y;
        case SDLK_l:         return AP_BTN_L1;
        case SDLK_SEMICOLON: return AP_BTN_L2;
        case SDLK_r:         return AP_BTN_R1;
        case SDLK_t:         return AP_BTN_R2;
        case SDLK_RETURN:    return AP_BTN_START;
        case SDLK_SPACE:     return AP_BTN_SELECT;
        case SDLK_h:         return AP_BTN_MENU;
        default:             return AP_BTN_NONE;
    }
}
#endif

/* Internal input event buffer */
static ap_input_event ap__input_queue[64];
static int ap__input_head = 0;
static int ap__input_tail = 0;

static void ap__input_push(ap_button btn, bool pressed) {
    if (btn == AP_BTN_NONE) return;
    int next = (ap__input_head + 1) % 64;
    if (next == ap__input_tail) return; /* queue full */
    ap__input_queue[ap__input_head].button = btn;
    ap__input_queue[ap__input_head].pressed = pressed;
    ap__input_head = next;
}

static void ap__process_sdl_events(void) {
    SDL_Event ev;
    uint32_t now = SDL_GetTicks();

    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                ap__input_push(AP_BTN_B, true);
                break;

            case SDL_KEYDOWN:
                if (!ev.key.repeat) {
                    ap_button b = ap__map_key_event(&ev.key);
                    ap__input_push(b, true);
                    if (b != AP_BTN_NONE) {
                        ap__g.buttons_held[b] = true;
                        ap__g.button_press_time[b] = now;
                        if (b == AP_BTN_UP || b == AP_BTN_DOWN || b == AP_BTN_LEFT || b == AP_BTN_RIGHT) {
                            ap__g.button_repeat_time[b] = now + ap__g.input_repeat_delay_ms;
                        }
                    }
                }
                break;

            case SDL_KEYUP: {
                ap_button b = ap__map_key_event(&ev.key);
                ap__input_push(b, false);
                if (b != AP_BTN_NONE) {
                    ap__g.buttons_held[b] = false;
                    ap__g.button_repeat_time[b] = 0;
                }
                break;
            }

            /* --- Raw Joystick events (TrimUI devices) --- */
            case SDL_JOYBUTTONDOWN: {
                ap_button b = ap__map_joy_button(ev.jbutton.button);
                ap__input_push(b, true);
                if (b != AP_BTN_NONE) {
                    ap__g.buttons_held[b] = true;
                    ap__g.button_press_time[b] = now;
                    if (b == AP_BTN_UP || b == AP_BTN_DOWN || b == AP_BTN_LEFT || b == AP_BTN_RIGHT) {
                        ap__g.button_repeat_time[b] = now + ap__g.input_repeat_delay_ms;
                    }
                }
                break;
            }

            case SDL_JOYBUTTONUP: {
                ap_button b = ap__map_joy_button(ev.jbutton.button);
                ap__input_push(b, false);
                if (b != AP_BTN_NONE) {
                    ap__g.buttons_held[b] = false;
                    ap__g.button_repeat_time[b] = 0;
                }
                break;
            }

            /* --- SDL GameController events (macOS / recognised controllers) --- */
            case SDL_CONTROLLERBUTTONDOWN: {
                ap_button b = ap__map_controller_button(ev.cbutton.button);
                ap__input_push(b, true);
                if (b != AP_BTN_NONE) {
                    ap__g.buttons_held[b] = true;
                    ap__g.button_press_time[b] = now;
                    if (b == AP_BTN_UP || b == AP_BTN_DOWN || b == AP_BTN_LEFT || b == AP_BTN_RIGHT) {
                        ap__g.button_repeat_time[b] = now + ap__g.input_repeat_delay_ms;
                    }
                }
                break;
            }

            case SDL_CONTROLLERBUTTONUP: {
                ap_button b = ap__map_controller_button(ev.cbutton.button);
                ap__input_push(b, false);
                if (b != AP_BTN_NONE) {
                    ap__g.buttons_held[b] = false;
                    ap__g.button_repeat_time[b] = 0;
                }
                break;
            }

            case SDL_CONTROLLERAXISMOTION: {
                /* Map left analog stick to d-pad via GameController axis */
                if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                    if (ev.caxis.value < -AP_AXIS_DEADZONE) {
                        if (ap__g.axis_held_dir_y != -1) {
                            ap__input_push(AP_BTN_UP, true);
                            ap__g.axis_repeat_time_y = now + ap__g.input_repeat_delay_ms;
                        }
                        ap__g.axis_held_dir_y = -1;
                    } else if (ev.caxis.value > AP_AXIS_DEADZONE) {
                        if (ap__g.axis_held_dir_y != 1) {
                            ap__input_push(AP_BTN_DOWN, true);
                            ap__g.axis_repeat_time_y = now + ap__g.input_repeat_delay_ms;
                        }
                        ap__g.axis_held_dir_y = 1;
                    } else {
                        if (ap__g.axis_held_dir_y == -1) ap__input_push(AP_BTN_UP, false);
                        if (ap__g.axis_held_dir_y ==  1) ap__input_push(AP_BTN_DOWN, false);
                        ap__g.axis_held_dir_y = 0;
                    }
                } else if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    if (ev.caxis.value < -AP_AXIS_DEADZONE) {
                        if (ap__g.axis_held_dir_x != -1) {
                            ap__input_push(AP_BTN_LEFT, true);
                            ap__g.axis_repeat_time_x = now + ap__g.input_repeat_delay_ms;
                        }
                        ap__g.axis_held_dir_x = -1;
                    } else if (ev.caxis.value > AP_AXIS_DEADZONE) {
                        if (ap__g.axis_held_dir_x != 1) {
                            ap__input_push(AP_BTN_RIGHT, true);
                            ap__g.axis_repeat_time_x = now + ap__g.input_repeat_delay_ms;
                        }
                        ap__g.axis_held_dir_x = 1;
                    } else {
                        if (ap__g.axis_held_dir_x == -1) ap__input_push(AP_BTN_LEFT, false);
                        if (ap__g.axis_held_dir_x ==  1) ap__input_push(AP_BTN_RIGHT, false);
                        ap__g.axis_held_dir_x = 0;
                    }
                } else if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                    if (ev.caxis.value > AP_AXIS_DEADZONE) {
                        if (!ap__g.buttons_held[AP_BTN_L2]) {
                            ap__input_push(AP_BTN_L2, true);
                            ap__g.buttons_held[AP_BTN_L2] = true;
                        }
                    } else if (ap__g.buttons_held[AP_BTN_L2]) {
                        ap__input_push(AP_BTN_L2, false);
                        ap__g.buttons_held[AP_BTN_L2] = false;
                    }
                } else if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                    if (ev.caxis.value > AP_AXIS_DEADZONE) {
                        if (!ap__g.buttons_held[AP_BTN_R2]) {
                            ap__input_push(AP_BTN_R2, true);
                            ap__g.buttons_held[AP_BTN_R2] = true;
                        }
                    } else if (ap__g.buttons_held[AP_BTN_R2]) {
                        ap__input_push(AP_BTN_R2, false);
                        ap__g.buttons_held[AP_BTN_R2] = false;
                    }
                }
                break;
            }

            case SDL_JOYHATMOTION: {
                uint8_t hat = ev.jhat.value;
                /* Clear previous hat state */
                if (!(hat & SDL_HAT_UP) && ap__g.hat_held & SDL_HAT_UP)
                    ap__input_push(AP_BTN_UP, false);
                if (!(hat & SDL_HAT_DOWN) && ap__g.hat_held & SDL_HAT_DOWN)
                    ap__input_push(AP_BTN_DOWN, false);
                if (!(hat & SDL_HAT_LEFT) && ap__g.hat_held & SDL_HAT_LEFT)
                    ap__input_push(AP_BTN_LEFT, false);
                if (!(hat & SDL_HAT_RIGHT) && ap__g.hat_held & SDL_HAT_RIGHT)
                    ap__input_push(AP_BTN_RIGHT, false);

                /* New hat presses */
                if ((hat & SDL_HAT_UP) && !(ap__g.hat_held & SDL_HAT_UP))
                    ap__input_push(AP_BTN_UP, true);
                if ((hat & SDL_HAT_DOWN) && !(ap__g.hat_held & SDL_HAT_DOWN))
                    ap__input_push(AP_BTN_DOWN, true);
                if ((hat & SDL_HAT_LEFT) && !(ap__g.hat_held & SDL_HAT_LEFT))
                    ap__input_push(AP_BTN_LEFT, true);
                if ((hat & SDL_HAT_RIGHT) && !(ap__g.hat_held & SDL_HAT_RIGHT))
                    ap__input_push(AP_BTN_RIGHT, true);

                ap__g.hat_held = hat;
                ap__g.hat_repeat_time = now + ap__g.input_repeat_delay_ms;
                break;
            }

            case SDL_JOYAXISMOTION: {
                if (ev.jaxis.axis == 1) { /* Y axis (up/down) */
                    if (ev.jaxis.value < -AP_AXIS_DEADZONE) {
                        if (ap__g.axis_held_dir_y != -1) {
                            ap__input_push(AP_BTN_UP, true);
                            ap__g.axis_repeat_time_y = now + ap__g.input_repeat_delay_ms;
                        }
                        ap__g.axis_held_dir_y = -1;
                    } else if (ev.jaxis.value > AP_AXIS_DEADZONE) {
                        if (ap__g.axis_held_dir_y != 1) {
                            ap__input_push(AP_BTN_DOWN, true);
                            ap__g.axis_repeat_time_y = now + ap__g.input_repeat_delay_ms;
                        }
                        ap__g.axis_held_dir_y = 1;
                    } else {
                        if (ap__g.axis_held_dir_y == -1) ap__input_push(AP_BTN_UP, false);
                        if (ap__g.axis_held_dir_y ==  1) ap__input_push(AP_BTN_DOWN, false);
                        ap__g.axis_held_dir_y = 0;
                    }
                } else if (ev.jaxis.axis == 0) { /* X axis (left/right) */
                    if (ev.jaxis.value < -AP_AXIS_DEADZONE) {
                        if (ap__g.axis_held_dir_x != -1) {
                            ap__input_push(AP_BTN_LEFT, true);
                            ap__g.axis_repeat_time_x = now + ap__g.input_repeat_delay_ms;
                        }
                        ap__g.axis_held_dir_x = -1;
                    } else if (ev.jaxis.value > AP_AXIS_DEADZONE) {
                        if (ap__g.axis_held_dir_x != 1) {
                            ap__input_push(AP_BTN_RIGHT, true);
                            ap__g.axis_repeat_time_x = now + ap__g.input_repeat_delay_ms;
                        }
                        ap__g.axis_held_dir_x = 1;
                    } else {
                        if (ap__g.axis_held_dir_x == -1) ap__input_push(AP_BTN_LEFT, false);
                        if (ap__g.axis_held_dir_x ==  1) ap__input_push(AP_BTN_RIGHT, false);
                        ap__g.axis_held_dir_x = 0;
                    }
                }
                break;
            }
        }
    }

    bool repeated_up = false;
    bool repeated_down = false;
    bool repeated_left = false;
    bool repeated_right = false;

    /* Directional hold repeat — digital buttons (keyboard/D-pad/button maps) */
    if (ap__g.buttons_held[AP_BTN_UP] && now >= ap__g.button_repeat_time[AP_BTN_UP]) {
        ap__input_push(AP_BTN_UP, true);
        ap__g.button_repeat_time[AP_BTN_UP] = now + ap__g.input_repeat_rate_ms;
        repeated_up = true;
    }
    if (ap__g.buttons_held[AP_BTN_DOWN] && now >= ap__g.button_repeat_time[AP_BTN_DOWN]) {
        ap__input_push(AP_BTN_DOWN, true);
        ap__g.button_repeat_time[AP_BTN_DOWN] = now + ap__g.input_repeat_rate_ms;
        repeated_down = true;
    }
    if (ap__g.buttons_held[AP_BTN_LEFT] && now >= ap__g.button_repeat_time[AP_BTN_LEFT]) {
        ap__input_push(AP_BTN_LEFT, true);
        ap__g.button_repeat_time[AP_BTN_LEFT] = now + ap__g.input_repeat_rate_ms;
        repeated_left = true;
    }
    if (ap__g.buttons_held[AP_BTN_RIGHT] && now >= ap__g.button_repeat_time[AP_BTN_RIGHT]) {
        ap__input_push(AP_BTN_RIGHT, true);
        ap__g.button_repeat_time[AP_BTN_RIGHT] = now + ap__g.input_repeat_rate_ms;
        repeated_right = true;
    }

    /* Directional hold repeat — hat */
    if (ap__g.hat_held && now >= ap__g.hat_repeat_time) {
        if ((ap__g.hat_held & SDL_HAT_UP) && !repeated_up) ap__input_push(AP_BTN_UP, true);
        if ((ap__g.hat_held & SDL_HAT_DOWN) && !repeated_down) ap__input_push(AP_BTN_DOWN, true);
        if ((ap__g.hat_held & SDL_HAT_LEFT) && !repeated_left) ap__input_push(AP_BTN_LEFT, true);
        if ((ap__g.hat_held & SDL_HAT_RIGHT) && !repeated_right) ap__input_push(AP_BTN_RIGHT, true);
        ap__g.hat_repeat_time = now + ap__g.input_repeat_rate_ms;
    }

    /* Directional hold repeat — analog Y */
    if (ap__g.axis_held_dir_y && now >= ap__g.axis_repeat_time_y) {
        if (ap__g.axis_held_dir_y < 0) {
            if (!repeated_up) ap__input_push(AP_BTN_UP, true);
            repeated_up = true;
        } else {
            if (!repeated_down) ap__input_push(AP_BTN_DOWN, true);
            repeated_down = true;
        }
        ap__g.axis_repeat_time_y = now + ap__g.input_repeat_rate_ms;
    }

    /* Directional hold repeat — analog X */
    if (ap__g.axis_held_dir_x && now >= ap__g.axis_repeat_time_x) {
        if (ap__g.axis_held_dir_x < 0) {
            if (!repeated_left) ap__input_push(AP_BTN_LEFT, true);
        } else {
            if (!repeated_right) ap__input_push(AP_BTN_RIGHT, true);
        }
        ap__g.axis_repeat_time_x = now + ap__g.input_repeat_rate_ms;
    }
}

bool ap_poll_input(ap_input_event *event) {
    /* Process SDL events into our queue */
    ap__process_sdl_events();

    /* Pop from internal queue */
    if (ap__input_head == ap__input_tail) return false;

    *event = ap__input_queue[ap__input_tail];
    ap__input_tail = (ap__input_tail + 1) % 64;

    /* Debounce: skip events too close together */
    uint32_t now = SDL_GetTicks();
    if (ap__g.input_delay_ms > 0 && (now - ap__g.last_input_time) < ap__g.input_delay_ms) {
        return ap_poll_input(event); /* skip and try next */
    }
    ap__g.last_input_time = now;

    return true;
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

/* ─── Combo System ───────────────────────────────────────────────────────── */

int ap_register_chord(const char *id, ap_button *buttons, int count, uint32_t window_ms) {
    if (ap__g.combo_count >= AP_MAX_COMBOS) return AP_ERROR;
    if (count > 8) return AP_ERROR;

    ap_combo *c = &ap__g.combos[ap__g.combo_count++];
    memset(c, 0, sizeof(*c));
    strncpy(c->id, id, sizeof(c->id) - 1);
    memcpy(c->buttons, buttons, count * sizeof(ap_button));
    c->button_count = count;
    c->window_ms = window_ms > 0 ? window_ms : 100;
    c->is_sequence = false;
    c->active = true;
    return AP_OK;
}

int ap_register_sequence(const char *id, ap_button *buttons, int count, uint32_t timeout_ms, bool strict) {
    if (ap__g.combo_count >= AP_MAX_COMBOS) return AP_ERROR;
    if (count > 8) return AP_ERROR;

    ap_combo *c = &ap__g.combos[ap__g.combo_count++];
    memset(c, 0, sizeof(*c));
    strncpy(c->id, id, sizeof(c->id) - 1);
    memcpy(c->buttons, buttons, count * sizeof(ap_button));
    c->button_count = count;
    c->window_ms = timeout_ms > 0 ? timeout_ms : 500;
    c->is_sequence = true;
    c->strict = strict;
    c->active = true;
    return AP_OK;
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
}

bool ap_poll_combo(ap_combo_event *event) {
    if (ap__g.combo_queue_head == ap__g.combo_queue_tail) return false;
    *event = ap__g.combo_queue[ap__g.combo_queue_tail];
    ap__g.combo_queue_tail = (ap__g.combo_queue_tail + 1) % 16;
    return true;
}


/* ─── Drawing Primitives ─────────────────────────────────────────────────── */

void ap_clear_screen(void) {
    ap_color bg = ap__g.theme.background;
    SDL_SetRenderDrawColor(ap__g.renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(ap__g.renderer);
}

void ap_present(void) {
    SDL_RenderPresent(ap__g.renderer);
}

void ap_draw_background(void) {
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

void ap_draw_text_wrapped(TTF_Font *font, const char *text, int x, int y, int max_w, ap_color color, ap_text_align align) {
    if (!font || !text || !text[0] || max_w <= 0) return;

    /* Word wrap: break text into lines that fit within max_w */
    char buf[4096];
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    int line_h = TTF_FontLineSkip(font);
    int cur_y = y;

    char *line_start = buf;
    while (*line_start) {
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
}

int ap_measure_text(TTF_Font *font, const char *text) {
    if (!font || !text || !text[0]) return 0;
    int w = 0;
    TTF_SizeUTF8(font, text, &w, NULL);
    return w;
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
    return AP_S(80);
}

void ap_draw_footer(ap_footer_item *items, int count) {
    if (!items || count <= 0) return;

    /* Match Gabagool v2 footer sizing: base 60px outer pill, SmallFont (34px),
     * 20px bottom padding between pill and screen edge. */
    TTF_Font *font = ap_get_font(AP_FONT_SMALL);
    if (!font) return;

    int margin     = AP_S(20);                         /* screen-edge margin */
    int outer_h    = AP_S(60);                         /* outer pill height */
    int inner_marg = AP_S(6);                          /* inset from outer → inner */
    int inner_h    = outer_h - inner_marg * 2;         /* inner button pill height */
    int pill_y     = ap__g.screen_h - AP_S(20) - outer_h; /* 20px gap from bottom */
    int pad_after_btn = AP_S(10);                      /* gap after inner pill before label */
    int item_gap   = AP_S(20);                         /* gap between items inside outer pill */
    int outer_pad  = AP_S(10);                         /* padding at start/end of outer pill */
    int font_h     = TTF_FontHeight(font);

    /* ── Helper: measure total inner width for a group of items ── */
    /* Each item: [inner_pill] + pad_after_btn + label_w, separated by item_gap */
    #define AP__FOOTER_ITEM_W(btn_name, label) \
        ({ int _bw = ap_measure_text(font, btn_name); \
           int _bpw = (_bw < inner_h - AP_S(4)) ? inner_h : _bw + AP_S(12); \
           int _lw = ap_measure_text(font, label); \
           _bpw + pad_after_btn + _lw; })

    /* ── Left group (non-confirm items) ── */
    int left_count = 0;
    int left_total_inner = 0;
    for (int i = 0; i < count; i++) {
        if (items[i].is_confirm) continue;
        if (left_count > 0) left_total_inner += item_gap;
        left_total_inner += AP__FOOTER_ITEM_W(ap_button_name(items[i].button), items[i].label);
        left_count++;
    }

    if (left_count > 0) {
        int outer_w = outer_pad * 2 + left_total_inner;
        ap_draw_pill(margin, pill_y, outer_w, outer_h, ap__g.theme.accent);

        int cx = margin + outer_pad;
        for (int i = 0; i < count; i++) {
            if (items[i].is_confirm) continue;

            const char *btn_name = ap_button_name(items[i].button);
            int btn_tw = ap_measure_text(font, btn_name);
            int btn_pill_w = (btn_tw < inner_h - AP_S(4)) ? inner_h : btn_tw + AP_S(12);
            int btn_pill_y = pill_y + inner_marg;

            /* Inner button pill */
            ap_draw_pill(cx, btn_pill_y, btn_pill_w, inner_h, ap__g.theme.highlight);
            ap_draw_text(font, btn_name,
                         cx + (btn_pill_w - btn_tw) / 2,
                         btn_pill_y + (inner_h - font_h) / 2,
                         ap__g.theme.button_label);

            /* Label text */
            cx += btn_pill_w + pad_after_btn;
            ap_draw_text(font, items[i].label,
                         cx,
                         pill_y + (outer_h - font_h) / 2,
                         ap__g.theme.hint);
            cx += ap_measure_text(font, items[i].label) + item_gap;
        }
    }

    /* ── Right group (confirm items) ── */
    int right_count = 0;
    int right_total_inner = 0;
    for (int i = 0; i < count; i++) {
        if (!items[i].is_confirm) continue;
        if (right_count > 0) right_total_inner += item_gap;
        right_total_inner += AP__FOOTER_ITEM_W(ap_button_name(items[i].button), items[i].label);
        right_count++;
    }

    if (right_count > 0) {
        int outer_w = outer_pad * 2 + right_total_inner;
        int rx = ap__g.screen_w - margin - outer_w;
        ap_draw_pill(rx, pill_y, outer_w, outer_h, ap__g.theme.accent);

        int cx = rx + outer_pad;
        for (int i = 0; i < count; i++) {
            if (!items[i].is_confirm) continue;

            const char *btn_name = ap_button_name(items[i].button);
            int btn_tw = ap_measure_text(font, btn_name);
            int btn_pill_w = (btn_tw < inner_h - AP_S(4)) ? inner_h : btn_tw + AP_S(12);
            int btn_pill_y = pill_y + inner_marg;

            /* Inner button pill */
            ap_draw_pill(cx, btn_pill_y, btn_pill_w, inner_h, ap__g.theme.highlight);
            ap_draw_text(font, btn_name,
                         cx + (btn_pill_w - btn_tw) / 2,
                         btn_pill_y + (inner_h - font_h) / 2,
                         ap__g.theme.button_label);

            /* Label text */
            cx += btn_pill_w + pad_after_btn;
            ap_draw_text(font, items[i].label,
                         cx,
                         pill_y + (outer_h - font_h) / 2,
                         ap__g.theme.hint);
            cx += ap_measure_text(font, items[i].label) + item_gap;
        }
    }

    #undef AP__FOOTER_ITEM_W
}

int ap_get_status_bar_height(void) {
    return AP_S(50);
}

/* Calculate the rendered pixel width of the status bar pill (for title width reduction) */
int ap_get_status_bar_width(ap_status_bar_opts *opts) {
    if (!opts) return 0;

    TTF_Font *font = ap_get_font(AP_FONT_SMALL);
    if (!font) return 0;

    int outer_pad = AP_S(20);   /* padding inside pill at sides */
    int icon_spacing = AP_S(8);
    int total_w = 0;

    /* Icons */
    for (int i = 0; i < opts->icon_count && opts->icons; i++) {
        if (i > 0) total_w += icon_spacing;
        total_w += ap_measure_text(font, opts->icons[i]);
    }

    /* Clock */
    if (opts->show_clock) {
        char clock_text[32];
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        if (opts->use_24h)
            strftime(clock_text, sizeof(clock_text), "%H:%M", t);
        else
            strftime(clock_text, sizeof(clock_text), "%I:%M %p", t);
        if (total_w > 0) total_w += icon_spacing;
        total_w += ap_measure_text(font, clock_text);
    }

    if (total_w <= 0) return 0;
    return total_w + outer_pad * 2;  /* pill width */
}

void ap_draw_status_bar(ap_status_bar_opts *opts) {
    if (!opts) return;

    /* Match Gabagool: SmallFont (34px base), accent pill, hint text,
     * positioned at Y=20 (aligning with title), right-aligned. */
    TTF_Font *font = ap_get_font(AP_FONT_SMALL);
    if (!font) return;

    int outer_pad = AP_S(20);      /* Gabagool outerPadding */
    int inner_pad_y = AP_S(6);     /* Gabagool innerPaddingY */
    int icon_spacing = AP_S(8);    /* Gabagool iconSpacing */
    int margin = AP_S(20);
    int font_h = TTF_FontHeight(font);

    /* Calculate pill width so we know its position */
    int pill_w = ap_get_status_bar_width(opts);
    if (pill_w <= 0) return;

    int pill_h = font_h + inner_pad_y * 2;
    int pill_y = AP_S(0);   /* Flush to top, matching title */
    int pill_x = ap__g.screen_w - margin - pill_w;
    int pill_r = pill_h / 2;

    ap_draw_rounded_rect(pill_x, pill_y, pill_w, pill_h, pill_r, ap__g.theme.accent);

    /* Render each element right-to-left inside the pill */
    int cx = pill_x + pill_w - outer_pad;  /* start from right */

    /* Clock (rightmost) */
    if (opts->show_clock) {
        char clock_text[32];
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        if (opts->use_24h)
            strftime(clock_text, sizeof(clock_text), "%H:%M", t);
        else
            strftime(clock_text, sizeof(clock_text), "%I:%M %p", t);
        int tw = ap_measure_text(font, clock_text);
        cx -= tw;
        ap_draw_text(font, clock_text, cx, pill_y + inner_pad_y, ap__g.theme.hint);
        cx -= icon_spacing;
    }

    /* Icons (right-to-left, from last to first) */
    for (int i = opts->icon_count - 1; i >= 0 && opts->icons; i--) {
        int tw = ap_measure_text(font, opts->icons[i]);
        cx -= tw;
        ap_draw_text(font, opts->icons[i], cx, pill_y + inner_pad_y, ap__g.theme.hint);
        if (i > 0) cx -= icon_spacing;
    }
}

/* ─── Power Button Handler ───────────────────────────────────────────────── */

#if AP_PLATFORM_IS_DEVICE
static void *ap__power_thread_func(void *arg) {
    (void)arg;

    /* Try to find the power button input device */
    const char *input_paths[] = {
        "/dev/input/event1",
        "/dev/input/event2",
        NULL,
    };

    int fd = -1;
    for (int i = 0; input_paths[i]; i++) {
        fd = open(input_paths[i], O_RDONLY);
        if (fd >= 0) break;
    }

    if (fd < 0) {
        ap_log("Power handler: could not open input device");
        return NULL;
    }

    while (ap__g.power_thread_running) {
        struct input_event ev;
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n != sizeof(ev)) continue;

        if (ev.type == EV_KEY && ev.code == KEY_POWER) {
            if (ev.value == 1) { /* Press */
                /* Track press time for short/long detection */
                uint32_t press_start = SDL_GetTicks();
                bool released = false;

                while (ap__g.power_thread_running) {
                    n = read(fd, &ev, sizeof(ev));
                    if (n == sizeof(ev) && ev.type == EV_KEY && ev.code == KEY_POWER && ev.value == 0) {
                        released = true;
                        break;
                    }
                    if (SDL_GetTicks() - press_start > 2000) break;
                }

                uint32_t held_ms = SDL_GetTicks() - press_start;
                if (held_ms >= 2000) {
                    /* Long press: shutdown */
                    ap_log("Power: long press → shutdown");
                    #if defined(PLATFORM_TG5040)
                    system("/mnt/SDCARD/.system/tg5040/bin/shutdown.sh");
                    #elif defined(PLATFORM_TG5050)
                    system("/mnt/SDCARD/.system/tg5050/bin/shutdown.sh");
                    #elif defined(PLATFORM_MY355)
                    system("poweroff");
                    #endif
                } else if (released) {
                    /* Short press: suspend */
                    ap_log("Power: short press → suspend");
                    #if defined(PLATFORM_TG5040)
                    system("/mnt/SDCARD/.system/tg5040/bin/suspend.sh");
                    #elif defined(PLATFORM_TG5050)
                    system("/mnt/SDCARD/.system/tg5050/bin/suspend.sh");
                    #elif defined(PLATFORM_MY355)
                    system("echo mem > /sys/power/state");
                    #endif
                }
            }
        }
    }

    close(fd);
    return NULL;
}
#endif

void ap_set_power_handler(bool enabled) {
    ap__g.power_handler_enabled = enabled;

#if AP_PLATFORM_IS_DEVICE
    if (enabled && !ap__g.power_thread_running) {
        ap__g.power_thread_running = true;
        pthread_create(&ap__g.power_thread, NULL, ap__power_thread_func, NULL);
    } else if (!enabled && ap__g.power_thread_running) {
        ap__g.power_thread_running = false;
        pthread_join(ap__g.power_thread, NULL);
    }
#endif
}

/* ─── Accessors ──────────────────────────────────────────────────────────── */

SDL_Renderer *ap_get_renderer(void)   { return ap__g.renderer; }
SDL_Window   *ap_get_window(void)     { return ap__g.window; }
int           ap_get_screen_width(void)  { return ap__g.screen_w; }
int           ap_get_screen_height(void) { return ap__g.screen_h; }

/* ─── Initialization ─────────────────────────────────────────────────────── */

int ap_init(ap_config *cfg) {
    if (ap__g.initialized) {
        ap__set_error("Already initialized");
        return AP_ERROR;
    }

    memset(&ap__g, 0, sizeof(ap__g));

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

    /* Init SDL2 — include GAMECONTROLLER for macOS / standard gamepads */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0) {
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

    /* Open input devices — prefer GameController API, fall back to raw joystick */
    int num_joy = SDL_NumJoysticks();
    for (int i = 0; i < num_joy; i++) {
        if (SDL_IsGameController(i)) {
            ap__g.controller = SDL_GameControllerOpen(i);
            if (ap__g.controller) {
                ap_log("Game controller opened: %s", SDL_GameControllerName(ap__g.controller));
                /* Also grab underlying joystick for hat/axis fallback */
                ap__g.joystick = SDL_GameControllerGetJoystick(ap__g.controller);
                break;
            }
        } else {
            ap__g.joystick = SDL_JoystickOpen(i);
            if (ap__g.joystick) {
                ap_log("Joystick opened: %s", SDL_JoystickName(ap__g.joystick));
                break;
            }
        }
    }

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

    /* Framebuffer sync workaround — render 3 black frames */
    for (int i = 0; i < 3; i++) {
        SDL_SetRenderDrawColor(ap__g.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ap__g.renderer);
        SDL_RenderPresent(ap__g.renderer);
    }

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

    /* Load background image if specified */
    if (cfg && cfg->show_background) {
        const char *bg_path = cfg->bg_image_path;
        if (!bg_path || !bg_path[0]) {
            /* Try NextUI default background */
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

    /* Start power handler on device if NextUI mode */
    if (cfg && cfg->is_nextui) {
        ap_set_power_handler(true);
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
