/*
 * Apostrophe Combo Demo
 *
 * Demonstrates the combo system with two modes selectable from a menu:
 *
 *  "Polling (classic)"  — register combos and poll ap_poll_combo() each frame.
 *  "Callbacks (_ex)"    — register combos with _ex variants so callbacks fire
 *                         automatically on trigger/release, without polling.
 *
 * Navigate the menu with D-Pad Up/Down, press A to enter, B to quit/go back.
 */

#define AP_IMPLEMENTATION
#include "apostrophe.h"

/* ──────────────────────────────────────────────────────────────────────────
 * Shared render state (set up once after ap_init)
 * ────────────────────────────────────────────────────────────────────────── */

static TTF_Font *g_title_font;
static TTF_Font *g_body_font;
static TTF_Font *g_hint_font;
static ap_color  g_fg;
static ap_color  g_accent;
static int       g_sw, g_sh, g_pad;

static void init_render_state(void) {
    g_title_font = ap_get_font(AP_FONT_EXTRA_LARGE);
    g_body_font  = ap_get_font(AP_FONT_LARGE);
    g_hint_font  = ap_get_font(AP_FONT_SMALL);
    g_fg         = ap_get_theme()->text;
    g_accent     = ap_get_theme()->accent;
    g_sw         = ap_get_screen_width();
    g_sh         = ap_get_screen_height();
    g_pad        = AP_DS(12);
}

/* Draw a centred status / last-event line */
static void draw_status(const char *status) {
    int tw = 0;
    TTF_SizeUTF8(g_body_font, status, &tw, NULL);
    ap_draw_text(g_body_font, status, (g_sw - tw) / 2, g_sh / 2 + AP_DS(10), g_accent);
}

/* ──────────────────────────────────────────────────────────────────────────
 * Demo A: Polling (classic)
 *
 * Standard usage: register combos with ap_register_chord / ap_register_sequence,
 * then drain ap_poll_combo() every frame just like ap_poll_input().
 * ────────────────────────────────────────────────────────────────────────── */

static void run_polling_demo(void) {
    /* Chord: L1 + R1 together within 150ms */
    ap_button shoulders[] = { AP_BTN_L1, AP_BTN_R1 };
    ap_register_chord("shoulders", shoulders, 2, 150);

    /* Chord: L2 + R2 together within 150ms */
    ap_button triggers[] = { AP_BTN_L2, AP_BTN_R2 };
    ap_register_chord("triggers", triggers, 2, 150);

    /* Sequence: Up Up Down Down — each press within 500ms of the previous */
    ap_button uudd[] = { AP_BTN_UP, AP_BTN_UP, AP_BTN_DOWN, AP_BTN_DOWN };
    ap_register_sequence("uudd", uudd, 4, 500, false);

    /* Sequence: A B A — strict mode (no extra presses allowed) */
    ap_button aba[] = { AP_BTN_A, AP_BTN_B, AP_BTN_A };
    ap_register_sequence("aba_strict", aba, 3, 400, true);

    char status[256] = "Waiting for combos...";
    bool running = true;

    ap_footer_item footer[] = {
        { .button = AP_BTN_MENU, .label = "BACK" },
    };

    while (running) {
        /* Normal input — MENU to go back */
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.button == AP_BTN_MENU && ev.pressed)
                running = false;
        }

        /* Poll combo events — ap_combo_event.type distinguishes chord vs sequence */
        ap_combo_event combo;
        while (ap_poll_combo(&combo)) {
            const char *kind = (combo.type == AP_COMBO_CHORD) ? "chord" : "seq";
            if (combo.triggered)
                snprintf(status, sizeof(status), "TRIGGERED [%s]: %s", kind, combo.id);
            else
                snprintf(status, sizeof(status), "Released  [%s]: %s", kind, combo.id);
            ap_log("poll combo: %s %s", combo.triggered ? "triggered" : "released", combo.id);
        }

        ap_clear_screen();
        int y = g_pad;
        ap_draw_text(g_title_font, "Polling (classic)", g_pad, y, g_fg);
        y += AP_DS(30);

        ap_draw_text(g_hint_font, "Chords — press simultaneously:", g_pad, y, g_fg);
        y += AP_DS(16);
        ap_draw_text(g_hint_font, "  L1 + R1              \"shoulders\"", g_pad, y, g_fg);
        y += AP_DS(14);
        ap_draw_text(g_hint_font, "  L2 + R2              \"triggers\"", g_pad, y, g_fg);
        y += AP_DS(20);

        ap_draw_text(g_hint_font, "Sequences — press in order:", g_pad, y, g_fg);
        y += AP_DS(16);
        ap_draw_text(g_hint_font, "  UP UP DOWN DOWN      \"uudd\"", g_pad, y, g_fg);
        y += AP_DS(14);
        ap_draw_text(g_hint_font, "  A B A (strict)       \"aba_strict\"", g_pad, y, g_fg);
        y += AP_DS(20);

        ap_draw_text(g_hint_font, "Events read by calling ap_poll_combo() each frame.", g_pad, y, g_fg);

        draw_status(status);
        ap_draw_footer(footer, 1);
        ap_present();
        SDL_Delay(16);
    }

    ap_clear_combos();
}

/* ──────────────────────────────────────────────────────────────────────────
 * Demo B: Callbacks (_ex variants)
 *
 * Register combos with ap_register_chord_ex / ap_register_sequence_ex.
 * Callbacks fire automatically at trigger/release time — no poll loop needed.
 * ap_poll_combo() still works alongside callbacks (events are enqueued either way).
 * ────────────────────────────────────────────────────────────────────────── */

static char g_cb_status[256] = "Waiting for combos...";

static void on_combo_trigger(const char *id, ap_combo_type type, void *userdata) {
    (void)userdata;
    const char *kind = (type == AP_COMBO_CHORD) ? "chord" : "seq";
    snprintf(g_cb_status, sizeof(g_cb_status), "CB TRIGGERED [%s]: %s", kind, id);
    ap_log("callback triggered: %s (%s)", id, kind);
}

static void on_combo_release(const char *id, ap_combo_type type, void *userdata) {
    (void)userdata;
    const char *kind = (type == AP_COMBO_CHORD) ? "chord" : "seq";
    snprintf(g_cb_status, sizeof(g_cb_status), "CB released  [%s]: %s", kind, id);
    ap_log("callback released: %s (%s)", id, kind);
}

static void run_callback_demo(void) {
    /* Chords: provide both on_trigger and on_release callbacks */
    ap_button shoulders[] = { AP_BTN_L1, AP_BTN_R1 };
    ap_register_chord_ex("shoulders_cb", shoulders, 2, 150,
                         on_combo_trigger, on_combo_release, NULL);

    ap_button triggers[] = { AP_BTN_L2, AP_BTN_R2 };
    ap_register_chord_ex("triggers_cb", triggers, 2, 150,
                         on_combo_trigger, on_combo_release, NULL);

    /* Sequences: only on_trigger (no release event for sequences) */
    ap_button uudd[] = { AP_BTN_UP, AP_BTN_UP, AP_BTN_DOWN, AP_BTN_DOWN };
    ap_register_sequence_ex("uudd_cb", uudd, 4, 500, false, on_combo_trigger, NULL);

    ap_button aba[] = { AP_BTN_A, AP_BTN_B, AP_BTN_A };
    ap_register_sequence_ex("aba_cb", aba, 3, 400, true, on_combo_trigger, NULL);

    snprintf(g_cb_status, sizeof(g_cb_status), "Waiting for combos...");
    bool running = true;

    ap_footer_item footer[] = {
        { .button = AP_BTN_MENU, .label = "BACK" },
    };

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.button == AP_BTN_MENU && ev.pressed)
                running = false;
        }

        /* Callbacks have already fired — drain the queue to keep it clean. */
        ap_combo_event combo;
        while (ap_poll_combo(&combo)) { /* events still enqueued alongside callbacks */ }

        ap_clear_screen();
        int y = g_pad;
        ap_draw_text(g_title_font, "Callbacks (_ex)", g_pad, y, g_fg);
        y += AP_DS(30);

        ap_draw_text(g_hint_font, "Chords — press simultaneously:", g_pad, y, g_fg);
        y += AP_DS(16);
        ap_draw_text(g_hint_font, "  L1 + R1              \"shoulders_cb\"", g_pad, y, g_fg);
        y += AP_DS(14);
        ap_draw_text(g_hint_font, "  L2 + R2              \"triggers_cb\"", g_pad, y, g_fg);
        y += AP_DS(20);

        ap_draw_text(g_hint_font, "Sequences — press in order:", g_pad, y, g_fg);
        y += AP_DS(16);
        ap_draw_text(g_hint_font, "  UP UP DOWN DOWN      \"uudd_cb\"", g_pad, y, g_fg);
        y += AP_DS(14);
        ap_draw_text(g_hint_font, "  A B A (strict)       \"aba_cb\"", g_pad, y, g_fg);
        y += AP_DS(20);

        ap_draw_text(g_hint_font, "on_trigger/on_release fire automatically — no poll needed.", g_pad, y, g_fg);

        draw_status(g_cb_status);
        ap_draw_footer(footer, 1);
        ap_present();
        SDL_Delay(16);
    }

    ap_clear_combos();
}

/* ──────────────────────────────────────────────────────────────────────────
 * Main menu
 * ────────────────────────────────────────────────────────────────────────── */

static const struct {
    const char *label;
    void (*fn)(void);
} g_modes[] = {
    { "Polling (classic)",  run_polling_demo  },
    { "Callbacks (_ex)",    run_callback_demo },
};
#define MODE_COUNT 2

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    ap_config cfg = {
        .window_title = "Combo Demo",
        .log_path     = ap_resolve_log_path("combo"),
        .is_nextui    = AP_PLATFORM_IS_DEVICE,
    };
    if (ap_init(&cfg) != AP_OK) {
        fprintf(stderr, "Failed to initialise Apostrophe\n");
        return 1;
    }
    ap_log("combo demo: startup");

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
                case AP_BTN_UP:   sel = (sel - 1 + MODE_COUNT) % MODE_COUNT; break;
                case AP_BTN_DOWN: sel = (sel + 1) % MODE_COUNT;              break;
                case AP_BTN_A:    g_modes[sel].fn();                          break;
                case AP_BTN_MENU: running = false;                            break;
                default: break;
            }
        }

        ap_clear_screen();
        int y = g_pad;
        ap_draw_text(g_title_font, "Combo Demo", g_pad, y, g_fg);
        y += AP_DS(30);
        ap_draw_text(g_hint_font, "Choose a demo mode:", g_pad, y, g_fg);
        y += AP_DS(24);

        for (int i = 0; i < MODE_COUNT; i++) {
            ap_color col = (i == sel) ? g_accent : g_fg;
            char line[128];
            snprintf(line, sizeof(line), "%s %s", (i == sel) ? ">" : " ", g_modes[i].label);
            ap_draw_text(g_body_font, line, g_pad, y, col);
            y += AP_DS(22);
        }

        ap_draw_footer(footer, 2);
        ap_present();
        SDL_Delay(16);
    }

    ap_quit();
    return 0;
}
