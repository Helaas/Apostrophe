/*
 * Apostrophe Combo Demo
 *
 * Demonstrates the combo system: register chords (simultaneous button presses)
 * and sequences (ordered button presses), then poll for combo events alongside
 * normal input events.
 */

#define AP_IMPLEMENTATION
#include "apostrophe.h"

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

    /* ─── Register chords (simultaneous presses) ─────────────────────── */

    /* L1 + R1 pressed together within 150ms */
    ap_button shoulder_chord[] = { AP_BTN_L1, AP_BTN_R1 };
    ap_register_chord("shoulders", shoulder_chord, 2, 150);

    /* L2 + R2 pressed together within 150ms */
    ap_button trigger_chord[] = { AP_BTN_L2, AP_BTN_R2 };
    ap_register_chord("triggers", trigger_chord, 2, 150);

    /* ─── Register sequences (ordered presses) ───────────────────────── */

    /* Up, Up, Down, Down — each press within 500ms of the previous */
    ap_button uudd[] = { AP_BTN_UP, AP_BTN_UP, AP_BTN_DOWN, AP_BTN_DOWN };
    ap_register_sequence("uudd", uudd, 4, 500, false);

    /* A, B, A — strict mode: no extraneous buttons allowed */
    ap_button aba[] = { AP_BTN_A, AP_BTN_B, AP_BTN_A };
    ap_register_sequence("aba_strict", aba, 3, 400, true);

    /* ─── Main loop ──────────────────────────────────────────────────── */

    TTF_Font *title_font  = ap_get_font(AP_FONT_EXTRA_LARGE);
    TTF_Font *body_font   = ap_get_font(AP_FONT_LARGE);
    TTF_Font *hint_font   = ap_get_font(AP_FONT_SMALL);
    ap_color  fg          = ap_get_theme()->text;
    ap_color  accent      = ap_get_theme()->accent;
    int       sw          = ap_get_screen_width();
    int       sh          = ap_get_screen_height();
    int       pad         = AP_DS(12);

    char status_line[256] = "Waiting for combos...";
    bool running = true;

    ap_footer_item footer[] = {
        { .button = AP_BTN_MENU, .label = "QUIT" },
    };

    while (running) {
        /* Poll normal input events — MENU to quit */
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.button == AP_BTN_MENU && ev.pressed) {
                running = false;
            }
        }

        /* Poll combo events */
        ap_combo_event combo;
        while (ap_poll_combo(&combo)) {
            if (combo.triggered) {
                snprintf(status_line, sizeof(status_line),
                         "TRIGGERED: %s", combo.id);
                ap_log("combo triggered: %s", combo.id);
            } else {
                snprintf(status_line, sizeof(status_line),
                         "Released: %s", combo.id);
                ap_log("combo released: %s", combo.id);
            }
        }

        /* Render */
        ap_clear_screen();

        int y = pad;
        ap_draw_text(title_font, "Combo Demo", pad, y, fg);
        y += AP_DS(30);

        ap_draw_text(hint_font, "Chords (press together):", pad, y, fg);
        y += AP_DS(16);
        ap_draw_text(hint_font, "  L1 + R1    \"shoulders\"", pad, y, fg);
        y += AP_DS(14);
        ap_draw_text(hint_font, "  L2 + R2    \"triggers\"", pad, y, fg);
        y += AP_DS(20);

        ap_draw_text(hint_font, "Sequences (press in order):", pad, y, fg);
        y += AP_DS(16);
        ap_draw_text(hint_font, "  UP UP DOWN DOWN    \"uudd\"", pad, y, fg);
        y += AP_DS(14);
        ap_draw_text(hint_font, "  A B A (strict)     \"aba_strict\"", pad, y, fg);
        y += AP_DS(30);

        /* Status line — centered vertically in remaining space */
        int status_y = sh / 2 + AP_DS(10);
        int tw = 0;
        TTF_SizeUTF8(body_font, status_line, &tw, NULL);
        ap_draw_text(body_font, status_line, (sw - tw) / 2, status_y, accent);

        ap_draw_footer(footer, 1);
        ap_present();
        SDL_Delay(16);
    }

    ap_clear_combos();
    ap_quit();
    return 0;
}
