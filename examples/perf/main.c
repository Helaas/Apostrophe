/*
 * Apostrophe Performance Demo
 *
 * Demonstrates CPU speed and fan control:
 *
 *  Main screen — live readout of CPU MHz, temperature, fan mode, and fan %.
 *  "CPU Speed"  — pick a preset; the change takes effect immediately.
 *  "Fan Speed"  — pick a NextUI-style fan mode or fixed percentage.
 *
 * All values display "N/A" on desktop builds (non-device).
 * MENU quits from any screen.
 */

#define AP_IMPLEMENTATION
#include "apostrophe.h"
#define AP_WIDGETS_IMPLEMENTATION
#include "apostrophe_widgets.h"

/* ──────────────────────────────────────────────────────────────────────────
 * Shared render state
 * ────────────────────────────────────────────────────────────────────────── */

static TTF_Font *g_body_font;
static TTF_Font *g_hint_font;
static ap_color  g_fg;
static ap_color  g_accent;
static int       g_sw, g_sh, g_pad;

static void init_render_state(void) {
    g_body_font  = ap_get_font(AP_FONT_LARGE);
    g_hint_font  = ap_get_font(AP_FONT_SMALL);
    g_fg         = ap_get_theme()->text;
    g_accent     = ap_get_theme()->accent;
    g_sw         = ap_get_screen_width();
    g_sh         = ap_get_screen_height();
    g_pad        = AP_DS(12);
}

/* Draw a labelled key/value row */
static int draw_kv(int x, int y, const char *key, const char *value) {
    char line[128];
    snprintf(line, sizeof(line), "%-18s %s", key, value);
    ap_draw_text(g_body_font, line, x, y, g_fg);
    return y + AP_DS(22);
}

/* Format an integer sensor value, showing "N/A" when -1 */
static const char *fmt_sensor(char *buf, int val, const char *unit) {
    if (val < 0) { snprintf(buf, 32, "N/A"); }
    else         { snprintf(buf, 32, "%d %s", val, unit); }
    return buf;
}

/* ──────────────────────────────────────────────────────────────────────────
 * Sub-screen: CPU Speed preset picker
 * ────────────────────────────────────────────────────────────────────────── */

static const struct { const char *label; ap_cpu_speed preset; } g_cpu_presets[] = {
    { "Menu       (~600–672 MHz)",   AP_CPU_SPEED_MENU        },
    { "Powersave  (~1200 MHz)",      AP_CPU_SPEED_POWERSAVE   },
    { "Normal     (~1608–1680 MHz)", AP_CPU_SPEED_NORMAL      },
    { "Performance (~2000–2160 MHz)",AP_CPU_SPEED_PERFORMANCE },
};
#define CPU_PRESET_COUNT 4

#if defined(PLATFORM_TG5050)
static const char *fan_mode_label(ap_fan_mode mode) {
    switch (mode) {
        case AP_FAN_MODE_MANUAL:           return "Manual";
        case AP_FAN_MODE_AUTO_QUIET:       return "Auto Quiet";
        case AP_FAN_MODE_AUTO_NORMAL:      return "Auto Normal";
        case AP_FAN_MODE_AUTO_PERFORMANCE: return "Auto Performance";
        default:                           return "N/A";
    }
}
#endif

static void run_cpu_screen(void) {
    int sel     = 2; /* default cursor on Normal */
    bool running = true;

    ap_footer_item footer[] = {
        { .button = AP_BTN_MENU, .label = "BACK" },
        { .button = AP_BTN_A,    .label = "APPLY", .is_confirm = true },
    };

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_UP:   sel = (sel - 1 + CPU_PRESET_COUNT) % CPU_PRESET_COUNT; break;
                case AP_BTN_DOWN: sel = (sel + 1) % CPU_PRESET_COUNT;                    break;
                case AP_BTN_A:
                    ap_set_cpu_speed(g_cpu_presets[sel].preset);
                    ap_log("perf: CPU speed set to preset %d", sel);
                    running = false;
                    break;
                case AP_BTN_MENU: running = false; break;
                default: break;
            }
        }

        ap_clear_screen();
        ap_draw_screen_title("CPU Speed", NULL);
        SDL_Rect content_rect = ap_get_content_rect(true, true, false);
        int y = content_rect.y;

        ap_draw_text(g_hint_font, "Select a preset and press A to apply:", g_pad, y, g_fg);
        y += AP_DS(20);

        for (int i = 0; i < CPU_PRESET_COUNT; i++) {
            ap_color col = (i == sel) ? g_accent : g_fg;
            char line[128];
            snprintf(line, sizeof(line), "%s %s",
                     (i == sel) ? ">" : " ", g_cpu_presets[i].label);
            ap_draw_text(g_body_font, line, g_pad, y, col);
            y += AP_DS(22);
        }

        /* Live current speed below the list */
        y += AP_DS(10);
        char buf[32];
        char cur[64];
        snprintf(cur, sizeof(cur), "Current: %s", fmt_sensor(buf, ap_get_cpu_speed_mhz(), "MHz"));
        ap_draw_text(g_hint_font, cur, g_pad, y, g_fg);

        ap_draw_footer(footer, 2);
        ap_present();
        SDL_Delay(16);
    }
}

/* ──────────────────────────────────────────────────────────────────────────
 * Sub-screen: Fan mode / speed picker (TG5050 only)
 * ────────────────────────────────────────────────────────────────────────── */

#if defined(PLATFORM_TG5050)
static const struct {
    const char *label;
    bool        auto_mode;
    ap_fan_mode mode;
    int         percent;
} g_fan_levels[] = {
    { "Performance (auto)", true,  AP_FAN_MODE_AUTO_PERFORMANCE, -1  },
    { "Normal (auto)",      true,  AP_FAN_MODE_AUTO_NORMAL,      -1  },
    { "Quiet (auto)",       true,  AP_FAN_MODE_AUTO_QUIET,       -1  },
    { "0%",                 false, AP_FAN_MODE_MANUAL,            0  },
    { "10%",                false, AP_FAN_MODE_MANUAL,           10  },
    { "20%",                false, AP_FAN_MODE_MANUAL,           20  },
    { "30%",                false, AP_FAN_MODE_MANUAL,           30  },
    { "40%",                false, AP_FAN_MODE_MANUAL,           40  },
    { "50%",                false, AP_FAN_MODE_MANUAL,           50  },
    { "60%",                false, AP_FAN_MODE_MANUAL,           60  },
    { "70%",                false, AP_FAN_MODE_MANUAL,           70  },
    { "80%",                false, AP_FAN_MODE_MANUAL,           80  },
    { "90%",                false, AP_FAN_MODE_MANUAL,           90  },
    { "100%",               false, AP_FAN_MODE_MANUAL,          100  },
};
#define FAN_LEVEL_COUNT ((int)(sizeof(g_fan_levels) / sizeof(g_fan_levels[0])))
#endif

static void run_fan_screen(void) {
    ap_footer_item footer[] = {
        { .button = AP_BTN_MENU, .label = "BACK" },
        { .button = AP_BTN_A,    .label = "APPLY", .is_confirm = true },
    };
#if defined(PLATFORM_TG5050)
    ap_list_item items[FAN_LEVEL_COUNT];
    for (int i = 0; i < FAN_LEVEL_COUNT; i++) {
        items[i] = (ap_list_item){ .label = g_fan_levels[i].label };
    }

    ap_list_opts opts = ap_list_default_opts("Fan Speed", items, FAN_LEVEL_COUNT);
    opts.footer       = footer;
    opts.footer_count = 2;

    ap_list_result result;
    if (ap_list(&opts, &result) == AP_OK && result.selected_index >= 0) {
        int sel = result.selected_index;
        if (g_fan_levels[sel].auto_mode) {
            ap_set_fan_mode(g_fan_levels[sel].mode);
            ap_log("perf: fan mode set to %s", fan_mode_label(g_fan_levels[sel].mode));
        } else {
            ap_set_fan_speed(g_fan_levels[sel].percent);
            ap_log("perf: fan set to %d%%", g_fan_levels[sel].percent);
        }
    }
#else
    bool running = true;
    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            if (ev.button == AP_BTN_MENU || ev.button == AP_BTN_A) running = false;
        }

        ap_clear_screen();
        ap_draw_screen_title("Fan Speed", NULL);
        SDL_Rect content_rect = ap_get_content_rect(true, true, false);
        int y = content_rect.y;
        ap_draw_text(g_hint_font, "Fan control is only available on TG5050.", g_pad, y, g_fg);
        y += AP_DS(18);
        ap_draw_text(g_hint_font, "This device has no fan hardware.", g_pad, y, g_fg);
        ap_draw_footer(footer, 2);
        ap_present();
        SDL_Delay(16);
    }
#endif
}

/* ──────────────────────────────────────────────────────────────────────────
 * Main screen — live readout + navigation
 * ────────────────────────────────────────────────────────────────────────── */

static const struct {
    const char *label;
    void (*fn)(void);
} g_actions[] = {
    { "Set CPU Speed", run_cpu_screen },
    { "Set Fan Speed", run_fan_screen },
};
#define ACTION_COUNT 2

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    ap_config cfg = {
        .window_title = "Performance Demo",
        .log_path     = ap_resolve_log_path("perf"),
        .is_nextui    = AP_PLATFORM_IS_DEVICE,
        /* Don't set cpu_speed here — the demo lets the user choose interactively */
    };
    if (ap_init(&cfg) != AP_OK) {
        fprintf(stderr, "Failed to initialise Apostrophe\n");
        return 1;
    }
    ap_log("perf demo: startup");

    init_render_state();

    int  sel     = 0;
    bool running = true;

    ap_footer_item footer[] = {
        { .button = AP_BTN_MENU, .label = "QUIT" },
        { .button = AP_BTN_A,    .label = "OPEN", .is_confirm = true },
    };

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_UP:   sel = (sel - 1 + ACTION_COUNT) % ACTION_COUNT; break;
                case AP_BTN_DOWN: sel = (sel + 1) % ACTION_COUNT;                break;
                case AP_BTN_A:    g_actions[sel].fn();                            break;
                case AP_BTN_MENU: running = false;                                break;
                default: break;
            }
        }

        /* Read live sensor values each frame */
        int   cpu_mhz  = ap_get_cpu_speed_mhz();
        int   cpu_temp = ap_get_cpu_temp_celsius();
        int   fan_pct  = ap_get_fan_speed();
        ap_fan_mode fan_mode = ap_get_fan_mode();

        ap_clear_screen();
        ap_draw_screen_title("Performance", NULL);
        SDL_Rect content_rect = ap_get_content_rect(true, true, false);
        int y = content_rect.y;

        /* ── Live sensor readout ── */
        char b1[32], b2[32], b3[32];
        y = draw_kv(g_pad, y, "Platform:",   AP_PLATFORM_NAME);
        y = draw_kv(g_pad, y, "CPU speed:",  fmt_sensor(b1, cpu_mhz,  "MHz"));
        y = draw_kv(g_pad, y, "CPU temp:",   fmt_sensor(b2, cpu_temp, "C"));
#if defined(PLATFORM_TG5050)
        y = draw_kv(g_pad, y, "Fan mode:",   fan_mode_label(fan_mode));
        y = draw_kv(g_pad, y, "Fan speed:",  fmt_sensor(b3, fan_pct,  "%"));
#else
        (void)b3; (void)fan_pct; (void)fan_mode;
        y = draw_kv(g_pad, y, "Fan speed:",  "N/A (no fan)");
#endif

        y += AP_DS(16);

        /* ── Action menu ── */
        ap_draw_text(g_hint_font, "Actions:", g_pad, y, g_fg);
        y += AP_DS(20);
        for (int i = 0; i < ACTION_COUNT; i++) {
            ap_color col = (i == sel) ? g_accent : g_fg;
            char line[64];
            snprintf(line, sizeof(line), "%s %s", (i == sel) ? ">" : " ", g_actions[i].label);
            ap_draw_text(g_body_font, line, g_pad, y, col);
            y += AP_DS(22);
        }

        ap_draw_footer(footer, 2);
        ap_present();
        SDL_Delay(250); /* slower refresh — sensor reads are slow sysfs calls */
    }

    ap_quit();
    return 0;
}
