/*
 * Apostrophe Widgets — UI component library for NextUI Paks
 *
 * Define AP_WIDGETS_IMPLEMENTATION in exactly ONE .c file before including.
 * Requires apostrophe.h to be included first (with AP_IMPLEMENTATION).
 *
 *   #define AP_IMPLEMENTATION
 *   #include "apostrophe.h"
 *   #define AP_WIDGETS_IMPLEMENTATION
 *   #include "apostrophe_widgets.h"
 *
 * Widgets use a blocking model: each runs its own event loop and returns
 * a result when the user makes a choice or presses back (AP_CANCELLED).
 */

#ifndef APOSTROPHE_WIDGETS_H
#define APOSTROPHE_WIDGETS_H

#ifndef APOSTROPHE_H
#error "apostrophe.h must be included before apostrophe_widgets.h"
#endif

#include <pthread.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * List Widget
 * ═══════════════════════════════════════════════════════════════════════════ */

/* A single item in a list */
typedef struct {
    const char  *label;
    const char  *metadata;    /* Arbitrary string stored with item (e.g. path) */
    SDL_Texture *image;       /* Optional preview image, NULL = none */
    bool         selected;    /* For multi-select: is this item checked? */
} ap_list_item;

/* Options controlling list behavior */
typedef struct {
    const char      *title;
    ap_list_item    *items;
    int              item_count;
    ap_footer_item  *footer;
    int              footer_count;
    ap_status_bar_opts *status_bar;
    bool             multi_select;     /* Enable checkbox multi-selection */
    ap_button        reorder_button;   /* AP_BTN_NONE = no reorder. Set e.g. AP_BTN_X */
    ap_button        action_button;    /* Custom action button (AP_BTN_START, etc.) */
    bool             show_images;      /* Show image column */
    const char      *help_text;        /* Help overlay text (L1 to show) */
    uint32_t         input_delay;      /* Override input debounce (0 = default) */
    int              initial_index;    /* Starting cursor position */
} ap_list_opts;

/* Result from closing a list */
typedef struct {
    int             selected_index;    /* Index of selected/confirmed item, or -1 */
    ap_list_action  action;            /* What caused the list to close */
    ap_list_item   *items;             /* Items array (potentially reordered) */
    int             item_count;
} ap_list_result;

/* Create default list options with sensible values */
ap_list_opts    ap_list_default_opts(const char *title, ap_list_item *items, int count);

/* Show a blocking list. Returns AP_OK on selection, AP_CANCELLED on back. */
int             ap_list(ap_list_opts *opts, ap_list_result *result);

/* ═══════════════════════════════════════════════════════════════════════════
 * Options List Widget (settings-style list with per-item options)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    AP_OPT_STANDARD = 0,   /* Left/Right to cycle through values */
    AP_OPT_KEYBOARD,       /* A opens keyboard input */
    AP_OPT_CLICKABLE,      /* A triggers navigation/action */
    AP_OPT_COLOR_PICKER    /* A opens color picker */
} ap_option_type;

typedef struct {
    const char *label;
    const char *value;     /* Display value, also returned as result */
} ap_option;

typedef struct {
    const char    *label;
    ap_option_type type;
    ap_option     *options;        /* Array of available options */
    int            option_count;
    int            selected_option;/* Currently selected option index */
} ap_options_item;

typedef struct {
    const char        *title;
    ap_options_item   *items;
    int                item_count;
    ap_footer_item    *footer;
    int                footer_count;
    ap_status_bar_opts *status_bar;
    ap_button          confirm_button;  /* Button that confirms/exits (e.g. START) */
    const char        *help_text;
    uint32_t           input_delay;
} ap_options_list_opts;

typedef struct {
    int              focused_index;
    ap_list_action   action;
    ap_options_item *items;
    int              item_count;
} ap_options_list_result;

int ap_options_list(ap_options_list_opts *opts, ap_options_list_result *result);

/* ═══════════════════════════════════════════════════════════════════════════
 * Keyboard Widget
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    AP_KB_GENERAL = 0,
    AP_KB_URL,
    AP_KB_NUMERIC
} ap_keyboard_layout;

typedef struct {
    char text[1024];
} ap_keyboard_result;

int ap_keyboard(const char *initial_text, const char *help_text,
                ap_keyboard_layout layout, ap_keyboard_result *result);

/* URL keyboard with customizable shortcut keys */
typedef struct {
    const char **shortcut_keys;   /* e.g. {".com", "https://", ".org"} */
    int          shortcut_count;
} ap_url_keyboard_config;

int ap_url_keyboard(const char *initial_text, const char *help_text,
                    ap_url_keyboard_config *cfg, ap_keyboard_result *result);

/* ═══════════════════════════════════════════════════════════════════════════
 * Confirmation Message
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char     *message;
    const char     *image_path;    /* Optional image above message */
    ap_footer_item *footer;
    int             footer_count;
} ap_message_opts;

typedef struct {
    bool confirmed;
} ap_confirm_result;

int ap_confirmation(ap_message_opts *opts, ap_confirm_result *result);

/* ═══════════════════════════════════════════════════════════════════════════
 * Selection Message (horizontal option selector)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char *label;
    const char *value;
} ap_selection_option;

typedef struct {
    int selected_index;
} ap_selection_result;

int ap_selection(const char *message, ap_selection_option *options, int count,
                 ap_footer_item *footer, int footer_count,
                 ap_selection_result *result);

/* ═══════════════════════════════════════════════════════════════════════════
 * Process Message (async task with progress bar)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef int (*ap_process_fn)(void *userdata);

typedef struct {
    const char     *message;
    bool            show_progress;
    float          *progress;           /* Pointer to float [0.0–1.0], updated by worker */
    int            *interrupt_signal;   /* Worker checks this; UI sets to 1 on cancel */
    ap_button       interrupt_button;   /* Button to cancel (AP_BTN_NONE = no cancel) */
    char          **dynamic_message;    /* Pointer to string pointer, updated by worker */
    int             message_lines;      /* Number of text lines to show (default 1) */
} ap_process_opts;

int ap_process_message(ap_process_opts *opts, ap_process_fn fn, void *userdata);

/* ═══════════════════════════════════════════════════════════════════════════
 * Detail Screen (scrollable multi-section view)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    AP_SECTION_INFO = 0,       /* Key-value pairs */
    AP_SECTION_DESCRIPTION,    /* Wrapped text block */
    AP_SECTION_IMAGE,          /* Single image */
    AP_SECTION_TABLE,          /* Table with rows/columns */
} ap_detail_section_type;

/* Key-value pair for info sections */
typedef struct {
    const char *key;
    const char *value;
} ap_detail_info_pair;

typedef struct {
    ap_detail_section_type type;
    const char *title;         /* Section header, NULL = no header */

    /* Type-specific data (use the appropriate one): */
    /* AP_SECTION_INFO */
    ap_detail_info_pair *info_pairs;
    int                  info_count;

    /* AP_SECTION_DESCRIPTION */
    const char          *description;

    /* AP_SECTION_IMAGE */
    const char          *image_path;
    int                  image_w;
    int                  image_h;

    /* AP_SECTION_TABLE */
    const char         **table_headers;
    const char        ***table_rows;   /* Array of row arrays */
    int                  table_cols;
    int                  table_rows_count;
} ap_detail_section;

typedef enum {
    AP_DETAIL_BACK = 0,
    AP_DETAIL_ACTION
} ap_detail_action;

typedef struct {
    const char        *title;
    ap_detail_section *sections;
    int                section_count;
    ap_footer_item    *footer;
    int                footer_count;
    ap_status_bar_opts *status_bar;
} ap_detail_opts;

typedef struct {
    ap_detail_action action;
} ap_detail_result;

int ap_detail_screen(ap_detail_opts *opts, ap_detail_result *result);

/* ═══════════════════════════════════════════════════════════════════════════
 * Color Picker
 * ═══════════════════════════════════════════════════════════════════════════ */

int ap_color_picker(ap_color initial, ap_color *result);

/* ═══════════════════════════════════════════════════════════════════════════
 * Help Overlay
 * ═══════════════════════════════════════════════════════════════════════════ */

void ap_show_help_overlay(const char *text);

/* ═══════════════════════════════════════════════════════════════════════════
 * IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifdef AP_WIDGETS_IMPLEMENTATION

/* ─── Internal widget helpers ────────────────────────────────────────────── */

/* Standard widget loop timing */
#define AP__FRAME_DELAY 16  /* ~60 fps */

/* Draw title bar text at top of screen */
static void ap__draw_title(const char *title) {
    if (!title || !title[0]) return;
    TTF_Font *font = ap_get_font(AP_FONT_SMALL);
    if (!font) return;

    int margin = AP_S(20);
    int title_y = AP_S(12);
    ap_draw_text(font, title, margin, title_y, ap_get_theme()->text);
}

/* Calculate the usable content area (below title, above footer) */
static void ap__content_area(int *y, int *h, bool has_title, bool has_footer) {
    int top = 0;
    if (has_title) top = AP_S(52);
    int bottom = 0;
    if (has_footer) bottom = ap_get_footer_height();
    *y = top;
    *h = ap_get_screen_height() - top - bottom;
}

/* Wait for a single press event (blocks within event loop). Returns AP_BTN_NONE on timeout. */
static ap_button ap__wait_for_press(uint32_t timeout_ms) {
    uint32_t start = SDL_GetTicks();
    while (1) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.pressed) return ev.button;
        }
        if (timeout_ms > 0 && (SDL_GetTicks() - start) > timeout_ms) break;
        SDL_Delay(AP__FRAME_DELAY);
    }
    return AP_BTN_NONE;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * LIST WIDGET Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

ap_list_opts ap_list_default_opts(const char *title, ap_list_item *items, int count) {
    ap_list_opts opts = {0};
    opts.title = title;
    opts.items = items;
    opts.item_count = count;
    opts.footer = NULL;
    opts.footer_count = 0;
    opts.status_bar = NULL;
    opts.multi_select = false;
    opts.reorder_button = AP_BTN_NONE;
    opts.action_button = AP_BTN_NONE;
    opts.show_images = false;
    opts.help_text = NULL;
    opts.input_delay = 0;
    opts.initial_index = 0;
    return opts;
}

int ap_list(ap_list_opts *opts, ap_list_result *result) {
    if (!opts || !result) return AP_ERROR;
    if (!opts->items || opts->item_count <= 0) return AP_ERROR;

    memset(result, 0, sizeof(*result));
    result->items = opts->items;
    result->item_count = opts->item_count;
    result->selected_index = -1;

    /* Save/restore input delay */
    uint32_t saved_delay = 0;
    if (opts->input_delay > 0) {
        saved_delay = opts->input_delay;
        ap_set_input_delay(opts->input_delay);
    }

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    TTF_Font *title_font = ap_get_font(AP_FONT_SMALL);
    TTF_Font *item_font  = ap_get_font(AP_FONT_SMALL);
    if (!title_font || !item_font) return AP_ERROR;

    /* Layout constants */
    int margin     = AP_S(20);
    int pill_h     = AP_S(56);
    int pill_pad   = AP_S(20);
    int item_gap   = AP_S(4);
    int pill_r     = pill_h / 2;  /* Capsule shape */
    int image_size = opts->show_images ? AP_S(48) : 0;
    int image_pad  = opts->show_images ? AP_S(12) : 0;

    /* Content area */
    int content_y, content_h;
    ap__content_area(&content_y, &content_h, opts->title != NULL, opts->footer_count > 0);

    /* Calculate visible items */
    int max_visible = content_h / (pill_h + item_gap);
    if (max_visible < 1) max_visible = 1;

    /* State */
    int cursor = opts->initial_index;
    if (cursor < 0) cursor = 0;
    if (cursor >= opts->item_count) cursor = opts->item_count - 1;
    int scroll_top = 0;
    bool reorder_mode = false;
    bool running = true;
    bool show_help = false;

    /* Text scroll state for selected item */
    ap_text_scroll sel_scroll;
    ap_text_scroll_init(&sel_scroll);
    int last_cursor = cursor;

    uint32_t last_frame = SDL_GetTicks();

    while (running) {
        uint32_t now = SDL_GetTicks();
        uint32_t dt = now - last_frame;
        last_frame = now;

        /* Input */
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            if (show_help) {
                show_help = false;
                continue;
            }

            switch (ev.button) {
                case AP_BTN_UP:
                    if (reorder_mode && cursor > 0) {
                        /* Swap items */
                        ap_list_item tmp = opts->items[cursor];
                        opts->items[cursor] = opts->items[cursor - 1];
                        opts->items[cursor - 1] = tmp;
                    }
                    if (cursor > 0) cursor--;
                    break;

                case AP_BTN_DOWN:
                    if (reorder_mode && cursor < opts->item_count - 1) {
                        ap_list_item tmp = opts->items[cursor];
                        opts->items[cursor] = opts->items[cursor + 1];
                        opts->items[cursor + 1] = tmp;
                    }
                    if (cursor < opts->item_count - 1) cursor++;
                    break;

                case AP_BTN_A:
                    if (opts->multi_select) {
                        opts->items[cursor].selected = !opts->items[cursor].selected;
                    } else {
                        result->selected_index = cursor;
                        result->action = AP_ACTION_SELECTED;
                        running = false;
                    }
                    break;

                case AP_BTN_B:
                    if (reorder_mode) {
                        reorder_mode = false;
                    } else {
                        result->action = AP_ACTION_BACK;
                        running = false;
                    }
                    break;

                case AP_BTN_L1:
                    if (opts->help_text) show_help = true;
                    break;

                default:
                    /* Check reorder button */
                    if (opts->reorder_button != AP_BTN_NONE && ev.button == opts->reorder_button) {
                        reorder_mode = !reorder_mode;
                    }
                    /* Check action button */
                    if (opts->action_button != AP_BTN_NONE && ev.button == opts->action_button) {
                        if (opts->multi_select) {
                            result->selected_index = cursor;
                            result->action = AP_ACTION_SELECTED;
                            running = false;
                        } else {
                            result->selected_index = cursor;
                            result->action = AP_ACTION_CUSTOM;
                            running = false;
                        }
                    }
                    break;
            }
        }

        /* Reset text scroll on cursor change */
        if (cursor != last_cursor) {
            ap_text_scroll_reset(&sel_scroll);
            last_cursor = cursor;
        }

        /* Scroll adjustment */
        if (cursor < scroll_top) scroll_top = cursor;
        if (cursor >= scroll_top + max_visible)
            scroll_top = cursor - max_visible + 1;
        if (scroll_top < 0) scroll_top = 0;

        /* Render */
        ap_draw_background();

        /* Title */
        if (opts->title) ap__draw_title(opts->title);

        /* Status bar */
        if (opts->status_bar) ap_draw_status_bar(opts->status_bar);

        /* List items */
        int available_w = screen_w - margin * 2;
        if (opts->item_count > max_visible) {
            available_w -= AP_S(12); /* Space for scrollbar */
        }

        for (int i = 0; i < max_visible && (scroll_top + i) < opts->item_count; i++) {
            int idx = scroll_top + i;
            int item_y = content_y + i * (pill_h + item_gap);
            bool is_selected = (idx == cursor);

            const char *label = opts->items[idx].label ? opts->items[idx].label : "";
            int text_h = TTF_FontHeight(item_font);

            /* Text area calculation */
            int text_x = margin + pill_pad;
            int text_max_w = available_w - pill_pad * 2;

            if (opts->show_images) {
                text_x += image_size + image_pad;
                text_max_w -= image_size + image_pad;
            }

            if (opts->multi_select) {
                /* Checkbox space */
                text_x += AP_S(32);
                text_max_w -= AP_S(32);
            }

            if (is_selected) {
                /* Measure text to determine pill width */
                int text_w = ap_measure_text(item_font, label);
                int pill_w = text_w + pill_pad * 2;

                /* Account for checkboxes and images */
                if (opts->multi_select) pill_w += AP_S(32);
                if (opts->show_images) pill_w += image_size + image_pad;

                if (pill_w > available_w) pill_w = available_w;

                /* Draw pill */
                ap_draw_pill(margin, item_y, pill_w, pill_h, theme->highlight);

                /* Draw image if present */
                if (opts->show_images && opts->items[idx].image) {
                    int img_y = item_y + (pill_h - image_size) / 2;
                    ap_draw_image(opts->items[idx].image, margin + pill_pad, img_y, image_size, image_size);
                }

                /* Draw checkbox */
                if (opts->multi_select) {
                    int cb_x = margin + pill_pad + (opts->show_images ? image_size + image_pad : 0);
                    int cb_y = item_y + (pill_h - AP_S(20)) / 2;
                    int cb_size = AP_S(20);
                    ap_color cb_color = theme->highlighted_text;
                    if (opts->items[idx].selected) {
                        ap_draw_rect(cb_x, cb_y, cb_size, cb_size, cb_color);
                        /* Check mark ─ simple "X" shape */
                        ap_color check = theme->highlight;
                        ap_draw_text(item_font, "✓", cb_x + AP_S(2), cb_y - AP_S(2), check);
                    } else {
                        /* Empty box */
                        SDL_SetRenderDrawColor(ap_get_renderer(), cb_color.r, cb_color.g, cb_color.b, cb_color.a);
                        SDL_Rect border = {cb_x, cb_y, cb_size, cb_size};
                        SDL_RenderDrawRect(ap_get_renderer(), &border);
                    }
                }

                /* Draw text with scroll for overflow */
                int draw_label_w = ap_measure_text(item_font, label);
                ap_text_scroll_update(&sel_scroll, draw_label_w, text_max_w, dt);

                if (draw_label_w > text_max_w) {
                    /* Scrolling clipped text */
                    SDL_Rect clip = {text_x, item_y, text_max_w, pill_h};
                    SDL_RenderSetClipRect(ap_get_renderer(), &clip);
                    ap_draw_text(item_font, label,
                                 text_x - sel_scroll.offset,
                                 item_y + (pill_h - text_h) / 2,
                                 theme->highlighted_text);
                    SDL_RenderSetClipRect(ap_get_renderer(), NULL);
                } else {
                    ap_draw_text(item_font, label,
                                 text_x,
                                 item_y + (pill_h - text_h) / 2,
                                 theme->highlighted_text);
                }

                /* Reorder mode indicator */
                if (reorder_mode) {
                    ap_color reorder_color = theme->accent;
                    int indicator_w = AP_S(4);
                    ap_draw_rect(margin - indicator_w - AP_S(4), item_y, indicator_w, pill_h, reorder_color);
                }
            } else {
                /* Non-selected item */
                if (opts->show_images && opts->items[idx].image) {
                    int img_y = item_y + (pill_h - image_size) / 2;
                    ap_draw_image(opts->items[idx].image, margin + pill_pad, img_y, image_size, image_size);
                }

                if (opts->multi_select) {
                    int cb_x = margin + pill_pad + (opts->show_images ? image_size + image_pad : 0);
                    int cb_y = item_y + (pill_h - AP_S(20)) / 2;
                    int cb_size = AP_S(20);
                    ap_color cb_color = theme->text;
                    cb_color.a = 180;
                    if (opts->items[idx].selected) {
                        ap_draw_rect(cb_x, cb_y, cb_size, cb_size, cb_color);
                        ap_draw_text(item_font, "✓", cb_x + AP_S(2), cb_y - AP_S(2), theme->text);
                    } else {
                        SDL_SetRenderDrawColor(ap_get_renderer(), cb_color.r, cb_color.g, cb_color.b, cb_color.a);
                        SDL_Rect border = {cb_x, cb_y, cb_size, cb_size};
                        SDL_RenderDrawRect(ap_get_renderer(), &border);
                    }
                }

                ap_draw_text_clipped(item_font, label,
                                     text_x,
                                     item_y + (pill_h - text_h) / 2,
                                     theme->text, text_max_w);
            }
        }

        /* Scrollbar */
        if (opts->item_count > max_visible) {
            int sb_x = screen_w - margin - AP_S(6);
            ap_draw_scrollbar(sb_x, content_y, content_h, max_visible, opts->item_count, scroll_top);
        }

        /* Footer */
        if (opts->footer && opts->footer_count > 0) {
            ap_draw_footer(opts->footer, opts->footer_count);
        }

        /* Help overlay (on top of everything) */
        if (show_help && opts->help_text) {
            ap_show_help_overlay(opts->help_text);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    /* Restore input delay */
    if (saved_delay > 0) {
        ap_set_input_delay(AP_INPUT_DEBOUNCE);
    }

    return result->action == AP_ACTION_BACK ? AP_CANCELLED : AP_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * OPTIONS LIST Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

int ap_options_list(ap_options_list_opts *opts, ap_options_list_result *result) {
    if (!opts || !result) return AP_ERROR;
    if (!opts->items || opts->item_count <= 0) return AP_ERROR;

    memset(result, 0, sizeof(*result));
    result->items = opts->items;
    result->item_count = opts->item_count;
    result->focused_index = 0;

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();

    TTF_Font *label_font = ap_get_font(AP_FONT_SMALL);
    TTF_Font *value_font = ap_get_font(AP_FONT_TINY);
    if (!label_font || !value_font) return AP_ERROR;

    int margin   = AP_S(20);
    int pill_h   = AP_S(56);
    int pill_pad = AP_S(20);
    int item_gap = AP_S(4);
    int arrow_w  = AP_S(24);

    int content_y, content_h;
    ap__content_area(&content_y, &content_h, opts->title != NULL, opts->footer_count > 0);
    int max_visible = content_h / (pill_h + item_gap);
    if (max_visible < 1) max_visible = 1;

    int cursor = 0;
    int scroll_top = 0;
    bool running = true;

    while (running) {
        /* Input */
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;

            switch (ev.button) {
                case AP_BTN_UP:
                    if (cursor > 0) cursor--;
                    break;

                case AP_BTN_DOWN:
                    if (cursor < opts->item_count - 1) cursor++;
                    break;

                case AP_BTN_LEFT: {
                    ap_options_item *item = &opts->items[cursor];
                    if (item->type == AP_OPT_STANDARD && item->option_count > 0) {
                        item->selected_option--;
                        if (item->selected_option < 0)
                            item->selected_option = item->option_count - 1;
                    }
                    break;
                }

                case AP_BTN_RIGHT: {
                    ap_options_item *item = &opts->items[cursor];
                    if (item->type == AP_OPT_STANDARD && item->option_count > 0) {
                        item->selected_option++;
                        if (item->selected_option >= item->option_count)
                            item->selected_option = 0;
                    }
                    break;
                }

                case AP_BTN_A: {
                    ap_options_item *item = &opts->items[cursor];
                    if (item->type == AP_OPT_CLICKABLE) {
                        result->focused_index = cursor;
                        result->action = AP_ACTION_SELECTED;
                        running = false;
                    } else if (item->type == AP_OPT_KEYBOARD) {
                        /* Open keyboard for this item */
                        ap_keyboard_result kb_result;
                        const char *initial = "";
                        if (item->option_count > 0 && item->options[item->selected_option].value) {
                            initial = item->options[item->selected_option].value;
                        }
                        int kb_ret = ap_keyboard(initial, "B: Cancel", AP_KB_GENERAL, &kb_result);
                        if (kb_ret == AP_OK) {
                            /* Update the option value — caller must manage memory */
                            if (item->option_count > 0) {
                                item->options[item->selected_option].value = strdup(kb_result.text);
                                item->options[item->selected_option].label = strdup(kb_result.text);
                            }
                        }
                    } else if (item->type == AP_OPT_COLOR_PICKER) {
                        ap_color initial_color = {255, 255, 255, 255};
                        ap_color picked;
                        if (ap_color_picker(initial_color, &picked) == AP_OK) {
                            /* Store color as hex string */
                            /* Caller handles this via result */
                        }
                    } else if (item->type == AP_OPT_STANDARD && item->option_count > 0) {
                        /* Cycle forward on A for standard options */
                        item->selected_option++;
                        if (item->selected_option >= item->option_count)
                            item->selected_option = 0;
                    }
                    break;
                }

                case AP_BTN_B:
                    result->action = AP_ACTION_BACK;
                    running = false;
                    break;

                default:
                    if (opts->confirm_button != AP_BTN_NONE && ev.button == opts->confirm_button) {
                        result->focused_index = cursor;
                        result->action = AP_ACTION_SELECTED;
                        running = false;
                    }
                    break;
            }
        }

        /* Scroll */
        if (cursor < scroll_top) scroll_top = cursor;
        if (cursor >= scroll_top + max_visible)
            scroll_top = cursor - max_visible + 1;

        /* Render */
        ap_draw_background();
        if (opts->title) ap__draw_title(opts->title);
        if (opts->status_bar) ap_draw_status_bar(opts->status_bar);

        int available_w = screen_w - margin * 2;

        for (int i = 0; i < max_visible && (scroll_top + i) < opts->item_count; i++) {
            int idx = scroll_top + i;
            int item_y = content_y + i * (pill_h + item_gap);
            bool is_selected = (idx == cursor);
            ap_options_item *item = &opts->items[idx];

            const char *label = item->label ? item->label : "";
            const char *value = "";
            if (item->option_count > 0 && item->selected_option >= 0 && item->selected_option < item->option_count) {
                value = item->options[item->selected_option].label;
                if (!value) value = item->options[item->selected_option].value;
                if (!value) value = "";
            }

            int label_w = ap_measure_text(label_font, label);
            int value_w = ap_measure_text(value_font, value);

            if (is_selected) {
                /* Full-width pill */
                ap_draw_pill(margin, item_y, available_w, pill_h, theme->highlight);

                /* Label on left */
                ap_draw_text_clipped(label_font, label,
                    margin + pill_pad,
                    item_y + (pill_h - TTF_FontHeight(label_font)) / 2,
                    theme->highlighted_text,
                    available_w / 2);

                /* Value on right with arrows for standard options */
                if (item->type == AP_OPT_STANDARD && item->option_count > 1) {
                    int right_x = margin + available_w - pill_pad - value_w - arrow_w * 2;
                    ap_draw_text(value_font, "<",
                        right_x,
                        item_y + (pill_h - TTF_FontHeight(value_font)) / 2,
                        theme->highlighted_text);
                    ap_draw_text(value_font, value,
                        right_x + arrow_w,
                        item_y + (pill_h - TTF_FontHeight(value_font)) / 2,
                        theme->highlighted_text);
                    ap_draw_text(value_font, ">",
                        right_x + arrow_w + value_w + AP_S(4),
                        item_y + (pill_h - TTF_FontHeight(value_font)) / 2,
                        theme->highlighted_text);
                } else if (item->type == AP_OPT_CLICKABLE) {
                    /* Show ">" indicator */
                    ap_draw_text(value_font, ">",
                        margin + available_w - pill_pad - AP_S(16),
                        item_y + (pill_h - TTF_FontHeight(value_font)) / 2,
                        theme->highlighted_text);
                } else {
                    int right_x = margin + available_w - pill_pad - value_w;
                    ap_draw_text(value_font, value,
                        right_x,
                        item_y + (pill_h - TTF_FontHeight(value_font)) / 2,
                        theme->highlighted_text);
                }
            } else {
                /* Unselected */
                ap_draw_text_clipped(label_font, label,
                    margin + pill_pad,
                    item_y + (pill_h - TTF_FontHeight(label_font)) / 2,
                    theme->text,
                    available_w / 2);

                int right_x = margin + available_w - pill_pad - value_w;
                ap_draw_text(value_font, value,
                    right_x,
                    item_y + (pill_h - TTF_FontHeight(value_font)) / 2,
                    theme->hint);
            }
        }

        /* Scrollbar */
        if (opts->item_count > max_visible) {
            int sb_x = screen_w - margin - AP_S(6);
            ap_draw_scrollbar(sb_x, content_y, content_h, max_visible, opts->item_count, scroll_top);
        }

        if (opts->footer && opts->footer_count > 0) {
            ap_draw_footer(opts->footer, opts->footer_count);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    return result->action == AP_ACTION_BACK ? AP_CANCELLED : AP_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * KEYBOARD Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Keyboard layouts */
static const char *ap__kb_general_lower[] = {
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", NULL,
    "z", "x", "c", "v", "b", "n", "m", NULL, NULL, NULL,
};

static const char *ap__kb_general_upper[] = {
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", NULL,
    "Z", "X", "C", "V", "B", "N", "M", NULL, NULL, NULL,
};

static const char *ap__kb_symbols[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
    "-", "_", "=", "+", "[", "]", "{", "}", ";", ":",
};

static const char *ap__kb_numeric[] = {
    "1", "2", "3",
    "4", "5", "6",
    "7", "8", "9",
    NULL, "0", NULL,
};

#define AP__KB_COLS_GENERAL 10
#define AP__KB_ROWS_GENERAL 3
#define AP__KB_COLS_NUMERIC  3
#define AP__KB_ROWS_NUMERIC  4

int ap_keyboard(const char *initial_text, const char *help_text,
                ap_keyboard_layout layout, ap_keyboard_result *result) {
    if (!result) return AP_ERROR;

    memset(result, 0, sizeof(*result));
    if (initial_text) {
        strncpy(result->text, initial_text, sizeof(result->text) - 1);
    }

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    TTF_Font *text_font = ap_get_font(AP_FONT_MEDIUM);
    TTF_Font *key_font  = ap_get_font(AP_FONT_SMALL);
    if (!text_font || !key_font) return AP_ERROR;

    /* Determine layout params */
    bool is_numeric = (layout == AP_KB_NUMERIC);
    int kb_cols = is_numeric ? AP__KB_COLS_NUMERIC : AP__KB_COLS_GENERAL;
    int kb_rows = is_numeric ? AP__KB_ROWS_NUMERIC : AP__KB_ROWS_GENERAL;

    const char **keys_lower = is_numeric ? ap__kb_numeric : ap__kb_general_lower;
    const char **keys_upper = is_numeric ? ap__kb_numeric : ap__kb_general_upper;
    const char **keys_sym   = ap__kb_symbols;

    int key_w   = AP_S(is_numeric ? 80 : 64);
    int key_h   = AP_S(52);
    int key_gap = AP_S(6);
    int key_r   = AP_S(8);

    /* Keyboard grid position (centered horizontally, lower portion of screen) */
    int grid_w = kb_cols * (key_w + key_gap) - key_gap;
    int grid_x = (screen_w - grid_w) / 2;
    int grid_y = screen_h - (kb_rows * (key_h + key_gap)) - AP_S(80);

    /* Text input area (above keyboard) */
    int input_y = AP_S(40);
    int input_h = AP_S(60);
    int input_x = AP_S(40);
    int input_w = screen_w - AP_S(80);

    /* State */
    int cursor_x = 0, cursor_y = 0;
    int text_cursor = (int)strlen(result->text);
    bool shift = false;
    bool symbols = false;
    bool running = true;

    /* Caret blink */
    uint32_t caret_blink = SDL_GetTicks();
    bool caret_visible = true;

    /* Footer for help */
    ap_footer_item footer[] = {
        {AP_BTN_B, help_text ? help_text : "Cancel", false},
        {AP_BTN_Y, shift ? "abc" : "ABC", false},
        {AP_BTN_X, symbols ? "abc" : "!@#", false},
        {AP_BTN_A, "OK", true},
    };

    while (running) {
        uint32_t now = SDL_GetTicks();

        /* Caret blink toggle every 500ms */
        if (now - caret_blink > 500) {
            caret_visible = !caret_visible;
            caret_blink = now;
        }

        /* Input */
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;

            switch (ev.button) {
                case AP_BTN_UP:
                    cursor_y--;
                    if (cursor_y < 0) cursor_y = kb_rows - 1;
                    if (cursor_x >= kb_cols) cursor_x = kb_cols - 1;
                    break;

                case AP_BTN_DOWN:
                    cursor_y++;
                    if (cursor_y >= kb_rows) cursor_y = 0;
                    if (cursor_x >= kb_cols) cursor_x = kb_cols - 1;
                    break;

                case AP_BTN_LEFT:
                    cursor_x--;
                    if (cursor_x < 0) cursor_x = kb_cols - 1;
                    break;

                case AP_BTN_RIGHT:
                    cursor_x++;
                    if (cursor_x >= kb_cols) cursor_x = 0;
                    break;

                case AP_BTN_A: {
                    /* Type the selected key */
                    int ki = cursor_y * kb_cols + cursor_x;
                    const char **active_keys = symbols ? keys_sym : (shift ? keys_upper : keys_lower);
                    int total_keys = kb_rows * kb_cols;

                    if (ki < total_keys && active_keys[ki] != NULL) {
                        int len = (int)strlen(result->text);
                        int klen = (int)strlen(active_keys[ki]);
                        if (len + klen < (int)sizeof(result->text) - 1) {
                            /* Insert at cursor position */
                            memmove(result->text + text_cursor + klen,
                                    result->text + text_cursor,
                                    len - text_cursor + 1);
                            memcpy(result->text + text_cursor, active_keys[ki], klen);
                            text_cursor += klen;
                        }
                    }
                    break;
                }

                case AP_BTN_B:
                    /* Cancel */
                    return AP_CANCELLED;

                case AP_BTN_Y:
                    /* Toggle shift */
                    shift = !shift;
                    symbols = false;
                    break;

                case AP_BTN_X:
                    /* Toggle symbols */
                    symbols = !symbols;
                    shift = false;
                    break;

                case AP_BTN_L1:
                    /* Backspace */
                    if (text_cursor > 0) {
                        int len = (int)strlen(result->text);
                        memmove(result->text + text_cursor - 1,
                                result->text + text_cursor,
                                len - text_cursor + 1);
                        text_cursor--;
                    }
                    break;

                case AP_BTN_R1:
                    /* Space */
                    {
                        int len = (int)strlen(result->text);
                        if (len < (int)sizeof(result->text) - 1) {
                            memmove(result->text + text_cursor + 1,
                                    result->text + text_cursor,
                                    len - text_cursor + 1);
                            result->text[text_cursor] = ' ';
                            text_cursor++;
                        }
                    }
                    break;

                case AP_BTN_START:
                    /* Confirm and return */
                    return AP_OK;

                case AP_BTN_L2:
                    /* Move text cursor left */
                    if (text_cursor > 0) text_cursor--;
                    break;

                case AP_BTN_R2:
                    /* Move text cursor right */
                    if (text_cursor < (int)strlen(result->text)) text_cursor++;
                    break;

                default:
                    break;
            }
        }

        /* Render */
        ap_draw_background();

        /* Input field */
        ap_draw_pill(input_x, input_y, input_w, input_h, theme->highlight);
        {
            int text_w_px = ap_measure_text(text_font, result->text);
            int text_y_center = input_y + (input_h - TTF_FontHeight(text_font)) / 2;
            int text_draw_x = input_x + AP_S(16);

            if (result->text[0]) {
                ap_draw_text_clipped(text_font, result->text,
                    text_draw_x, text_y_center,
                    theme->highlighted_text, input_w - AP_S(32));
            }

            /* Blinking caret */
            if (caret_visible) {
                /* Measure text up to cursor position */
                char saved = result->text[text_cursor];
                result->text[text_cursor] = '\0';
                int caret_x = text_draw_x + ap_measure_text(text_font, result->text);
                result->text[text_cursor] = saved;

                int caret_h = TTF_FontHeight(text_font);
                ap_draw_rect(caret_x, text_y_center, AP_S(2), caret_h, theme->highlighted_text);
            }
        }

        /* Key grid */
        const char **active_keys = symbols ? keys_sym : (shift ? keys_upper : keys_lower);
        for (int row = 0; row < kb_rows; row++) {
            /* Center each row */
            int row_cols = kb_cols;
            /* For general layout, rows have different widths */
            if (!is_numeric && row == 1) row_cols = 9;
            if (!is_numeric && row == 2) row_cols = 7;

            int row_w = row_cols * (key_w + key_gap) - key_gap;
            int row_x = (screen_w - row_w) / 2;

            for (int col = 0; col < row_cols; col++) {
                int ki = row * kb_cols + col;
                if (ki >= kb_rows * kb_cols) break;

                const char *key = active_keys[ki];
                if (key == NULL) continue;

                int kx = row_x + col * (key_w + key_gap);
                int ky = grid_y + row * (key_h + key_gap);

                bool is_cursor = (row == cursor_y && col == cursor_x);
                ap_color key_bg = is_cursor ? theme->highlight : theme->accent;
                ap_color key_fg = is_cursor ? theme->highlighted_text : theme->hint;

                ap_draw_rounded_rect(kx, ky, key_w, key_h, key_r, key_bg);

                int tw = ap_measure_text(key_font, key);
                int th = TTF_FontHeight(key_font);
                ap_draw_text(key_font, key,
                    kx + (key_w - tw) / 2,
                    ky + (key_h - th) / 2,
                    key_fg);
            }
        }

        /* Footer hints */
        footer[1].label = shift ? "abc" : "ABC";
        footer[2].label = symbols ? "abc" : "!@#";
        ap_draw_footer(footer, 4);

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    return AP_CANCELLED;
}

int ap_url_keyboard(const char *initial_text, const char *help_text,
                    ap_url_keyboard_config *cfg, ap_keyboard_result *result) {
    /* URL keyboard is General keyboard with URL shortcuts displayed */
    /* For now, delegate to general keyboard — shortcuts are a future enhancement */
    (void)cfg;
    return ap_keyboard(initial_text, help_text, AP_KB_URL, result);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CONFIRMATION MESSAGE Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

int ap_confirmation(ap_message_opts *opts, ap_confirm_result *result) {
    if (!opts || !result) return AP_ERROR;

    memset(result, 0, sizeof(*result));
    result->confirmed = false;

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    TTF_Font *msg_font = ap_get_font(AP_FONT_MEDIUM);
    if (!msg_font) return AP_ERROR;

    /* Load image if specified */
    SDL_Texture *img_tex = NULL;
    int img_w = 0, img_h = 0;
    if (opts->image_path) {
        img_tex = ap_load_image(opts->image_path);
        if (img_tex) {
            SDL_QueryTexture(img_tex, NULL, NULL, &img_w, &img_h);
            /* Scale image to fit */
            int max_img_w = screen_w / 2;
            int max_img_h = screen_h / 3;
            if (img_w > max_img_w || img_h > max_img_h) {
                float scale = fminf((float)max_img_w / img_w, (float)max_img_h / img_h);
                img_w = (int)(img_w * scale);
                img_h = (int)(img_h * scale);
            }
        }
    }

    bool running = true;
    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;

            switch (ev.button) {
                case AP_BTN_A:
                    result->confirmed = true;
                    running = false;
                    break;
                case AP_BTN_B:
                    result->confirmed = false;
                    running = false;
                    break;
                default:
                    break;
            }
        }

        /* Render */
        ap_draw_background();

        /* Center the content vertically */
        int total_h = 0;
        if (img_tex) total_h += img_h + AP_S(20);
        int msg_h = TTF_FontHeight(msg_font);
        total_h += msg_h;

        int base_y = (screen_h - total_h - ap_get_footer_height()) / 2;

        /* Image */
        if (img_tex) {
            ap_draw_image(img_tex,
                (screen_w - img_w) / 2, base_y, img_w, img_h);
            base_y += img_h + AP_S(20);
        }

        /* Message (centered, wrapped) */
        if (opts->message) {
            ap_draw_text_wrapped(msg_font, opts->message,
                AP_S(40), base_y,
                screen_w - AP_S(80),
                theme->text, AP_ALIGN_CENTER);
        }

        /* Footer */
        if (opts->footer && opts->footer_count > 0) {
            ap_draw_footer(opts->footer, opts->footer_count);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    /* Cleanup */
    if (img_tex) SDL_DestroyTexture(img_tex);

    return result->confirmed ? AP_OK : AP_CANCELLED;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SELECTION MESSAGE Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

int ap_selection(const char *message, ap_selection_option *options, int count,
                 ap_footer_item *footer, int footer_count,
                 ap_selection_result *result) {
    if (!result || !options || count <= 0) return AP_ERROR;

    memset(result, 0, sizeof(*result));

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    TTF_Font *msg_font  = ap_get_font(AP_FONT_MEDIUM);
    TTF_Font *opt_font  = ap_get_font(AP_FONT_SMALL);
    if (!msg_font || !opt_font) return AP_ERROR;

    int selected = 0;
    int pill_h = AP_S(44);
    int pill_pad = AP_S(16);
    int pill_gap = AP_S(10);
    bool running = true;

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;

            switch (ev.button) {
                case AP_BTN_LEFT:
                    if (selected > 0) selected--;
                    break;
                case AP_BTN_RIGHT:
                    if (selected < count - 1) selected++;
                    break;
                case AP_BTN_A:
                    result->selected_index = selected;
                    running = false;
                    break;
                case AP_BTN_B:
                    return AP_CANCELLED;
                default:
                    break;
            }
        }

        /* Render */
        ap_draw_background();

        /* Message */
        int msg_y = screen_h / 3;
        if (message) {
            ap_draw_text_wrapped(msg_font, message,
                AP_S(40), msg_y,
                screen_w - AP_S(80),
                theme->text, AP_ALIGN_CENTER);
            msg_y += TTF_FontHeight(msg_font) + AP_S(30);
        }

        /* Option pills (horizontal, centered) */
        int total_w = 0;
        for (int i = 0; i < count; i++) {
            total_w += ap_measure_text(opt_font, options[i].label) + pill_pad * 2;
            if (i < count - 1) total_w += pill_gap;
        }

        int opt_x = (screen_w - total_w) / 2;
        int opt_y = msg_y + AP_S(20);

        for (int i = 0; i < count; i++) {
            int tw = ap_measure_text(opt_font, options[i].label);
            int pw = tw + pill_pad * 2;
            bool is_sel = (i == selected);

            ap_color pill_bg = is_sel ? theme->highlight : theme->accent;
            ap_color pill_fg = is_sel ? theme->highlighted_text : theme->hint;

            ap_draw_pill(opt_x, opt_y, pw, pill_h, pill_bg);
            ap_draw_text(opt_font, options[i].label,
                opt_x + pill_pad,
                opt_y + (pill_h - TTF_FontHeight(opt_font)) / 2,
                pill_fg);

            opt_x += pw + pill_gap;
        }

        if (footer && footer_count > 0) {
            ap_draw_footer(footer, footer_count);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    return AP_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PROCESS MESSAGE Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Worker thread wrapper */
typedef struct {
    ap_process_fn  fn;
    void          *userdata;
    int            result;
    bool           done;
} ap__process_thread_data;

static void *ap__process_worker(void *arg) {
    ap__process_thread_data *d = (ap__process_thread_data *)arg;
    d->result = d->fn(d->userdata);
    d->done = true;
    return NULL;
}

int ap_process_message(ap_process_opts *opts, ap_process_fn fn, void *userdata) {
    if (!opts || !fn) return AP_ERROR;

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    TTF_Font *msg_font = ap_get_font(AP_FONT_MEDIUM);
    TTF_Font *dyn_font = ap_get_font(AP_FONT_SMALL);
    if (!msg_font) return AP_ERROR;

    /* Start worker thread */
    ap__process_thread_data thread_data = {
        .fn = fn,
        .userdata = userdata,
        .result = 0,
        .done = false,
    };

    pthread_t worker;
    if (pthread_create(&worker, NULL, ap__process_worker, &thread_data) != 0) {
        return AP_ERROR;
    }

    int bar_w = AP_S(400);
    int bar_h = AP_S(16);
    int bar_x = (screen_w - bar_w) / 2;

    bool running = true;
    while (running) {
        /* Check if worker is done */
        if (thread_data.done) {
            running = false;
            break;
        }

        /* Input: check for interrupt */
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            if (opts->interrupt_button != AP_BTN_NONE && ev.button == opts->interrupt_button) {
                if (opts->interrupt_signal) {
                    *(opts->interrupt_signal) = 1;
                }
            }
        }

        /* Render */
        ap_draw_background();

        int center_y = screen_h / 2;

        /* Static message */
        if (opts->message) {
            ap_draw_text_wrapped(msg_font, opts->message,
                AP_S(40), center_y - AP_S(60),
                screen_w - AP_S(80),
                theme->text, AP_ALIGN_CENTER);
        }

        /* Dynamic message */
        if (opts->dynamic_message && *opts->dynamic_message) {
            int dyn_y = center_y - AP_S(20);
            int lines = opts->message_lines > 0 ? opts->message_lines : 1;
            /* Show last N lines of dynamic message */
            ap_draw_text_wrapped(dyn_font, *opts->dynamic_message,
                AP_S(40), dyn_y,
                screen_w - AP_S(80),
                theme->hint, AP_ALIGN_CENTER);
            (void)lines; /* TODO: implement multi-line truncation */
        }

        /* Progress bar */
        if (opts->show_progress && opts->progress) {
            float prog = *opts->progress;
            int bar_y = center_y + AP_S(30);

            ap_color bar_bg = theme->accent;
            bar_bg.a = 80;
            ap_draw_progress_bar(bar_x, bar_y, bar_w, bar_h, prog, theme->accent, bar_bg);

            /* Progress percentage */
            char pct[16];
            snprintf(pct, sizeof(pct), "%.0f%%", prog * 100.0f);
            int pct_w = ap_measure_text(dyn_font, pct);
            ap_draw_text(dyn_font, pct,
                (screen_w - pct_w) / 2,
                bar_y + bar_h + AP_S(8),
                theme->hint);
        }

        /* Spinner (simple dots animation when no progress bar) */
        if (!opts->show_progress || !opts->progress) {
            uint32_t ticks = SDL_GetTicks() / 300;
            int dots = (ticks % 4);
            char spinner[8] = "";
            for (int i = 0; i < dots; i++) strcat(spinner, ".");
            int sw = ap_measure_text(msg_font, spinner);
            ap_draw_text(msg_font, spinner,
                (screen_w - sw) / 2,
                center_y + AP_S(20),
                theme->hint);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    /* Wait for thread to finish */
    pthread_join(worker, NULL);

    return thread_data.result;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DETAIL SCREEN Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

int ap_detail_screen(ap_detail_opts *opts, ap_detail_result *result) {
    if (!opts || !result) return AP_ERROR;

    memset(result, 0, sizeof(*result));

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    TTF_Font *title_font   = ap_get_font(AP_FONT_SMALL);
    TTF_Font *section_font = ap_get_font(AP_FONT_SMALL);
    TTF_Font *body_font    = ap_get_font(AP_FONT_TINY);
    TTF_Font *key_font     = ap_get_font(AP_FONT_TINY);
    if (!title_font || !body_font) return AP_ERROR;

    int margin = AP_S(20);
    int section_gap = AP_S(24);
    int line_h = TTF_FontLineSkip(body_font);

    /* Calculate total content height for scrolling */
    int total_content_h = 0;
    for (int s = 0; s < opts->section_count; s++) {
        ap_detail_section *sec = &opts->sections[s];
        if (sec->title) total_content_h += AP_S(32);

        switch (sec->type) {
            case AP_SECTION_INFO:
                total_content_h += sec->info_count * (line_h + AP_S(4));
                break;
            case AP_SECTION_DESCRIPTION:
                /* Rough estimate — actual wrapped height is complex */
                if (sec->description) {
                    int chars_per_line = (screen_w - margin * 2) / (AP_S(10));
                    int est_lines = ((int)strlen(sec->description) / (chars_per_line > 0 ? chars_per_line : 1)) + 1;
                    total_content_h += est_lines * line_h;
                }
                break;
            case AP_SECTION_IMAGE:
                total_content_h += sec->image_h > 0 ? sec->image_h : AP_S(200);
                break;
            case AP_SECTION_TABLE:
                total_content_h += (sec->table_rows_count + 1) * (line_h + AP_S(4));
                break;
        }
        total_content_h += section_gap;
    }

    int content_y, content_h;
    ap__content_area(&content_y, &content_h, opts->title != NULL, opts->footer_count > 0);

    int scroll_offset = 0;
    int max_scroll = total_content_h - content_h;
    if (max_scroll < 0) max_scroll = 0;

    bool running = true;
    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;

            switch (ev.button) {
                case AP_BTN_UP:
                    scroll_offset -= AP_S(40);
                    if (scroll_offset < 0) scroll_offset = 0;
                    break;
                case AP_BTN_DOWN:
                    scroll_offset += AP_S(40);
                    if (scroll_offset > max_scroll) scroll_offset = max_scroll;
                    break;
                case AP_BTN_B:
                    result->action = AP_DETAIL_BACK;
                    running = false;
                    break;
                case AP_BTN_A:
                    result->action = AP_DETAIL_ACTION;
                    running = false;
                    break;
                default:
                    break;
            }
        }

        /* Render */
        ap_draw_background();
        if (opts->title) ap__draw_title(opts->title);
        if (opts->status_bar) ap_draw_status_bar(opts->status_bar);

        /* Clip to content area */
        SDL_Rect clip = {0, content_y, screen_w, content_h};
        SDL_RenderSetClipRect(ap_get_renderer(), &clip);

        int draw_y = content_y - scroll_offset;

        for (int s = 0; s < opts->section_count; s++) {
            ap_detail_section *sec = &opts->sections[s];

            /* Section title */
            if (sec->title) {
                ap_draw_text(section_font, sec->title, margin, draw_y, theme->accent);
                draw_y += AP_S(32);
            }

            switch (sec->type) {
                case AP_SECTION_INFO:
                    for (int p = 0; p < sec->info_count; p++) {
                        if (sec->info_pairs[p].key) {
                            ap_draw_text(key_font, sec->info_pairs[p].key,
                                margin, draw_y, theme->hint);
                        }
                        if (sec->info_pairs[p].value) {
                            int val_x = margin + AP_S(200);
                            ap_draw_text_clipped(body_font, sec->info_pairs[p].value,
                                val_x, draw_y, theme->text,
                                screen_w - val_x - margin);
                        }
                        draw_y += line_h + AP_S(4);
                    }
                    break;

                case AP_SECTION_DESCRIPTION:
                    if (sec->description) {
                        ap_draw_text_wrapped(body_font, sec->description,
                            margin, draw_y,
                            screen_w - margin * 2,
                            theme->text, AP_ALIGN_LEFT);
                        /* Advance draw_y by estimated wrapped text height */
                        int est_lines = 1;
                        int cpl = (screen_w - margin * 2) / (AP_S(10));
                        if (cpl > 0) est_lines = ((int)strlen(sec->description) / cpl) + 1;
                        draw_y += est_lines * line_h;
                    }
                    break;

                case AP_SECTION_IMAGE: {
                    SDL_Texture *img = ap_load_image(sec->image_path);
                    if (img) {
                        int iw = sec->image_w > 0 ? sec->image_w : AP_S(300);
                        int ih = sec->image_h > 0 ? sec->image_h : AP_S(200);
                        ap_draw_image(img, (screen_w - iw) / 2, draw_y, iw, ih);
                        SDL_DestroyTexture(img);
                        draw_y += ih;
                    }
                    break;
                }

                case AP_SECTION_TABLE:
                    if (sec->table_headers && sec->table_cols > 0) {
                        int col_w = (screen_w - margin * 2) / sec->table_cols;
                        /* Headers */
                        for (int c = 0; c < sec->table_cols; c++) {
                            if (sec->table_headers[c]) {
                                ap_draw_text(key_font, sec->table_headers[c],
                                    margin + c * col_w, draw_y, theme->accent);
                            }
                        }
                        draw_y += line_h + AP_S(4);
                        /* Rows */
                        for (int r = 0; r < sec->table_rows_count; r++) {
                            if (sec->table_rows && sec->table_rows[r]) {
                                for (int c = 0; c < sec->table_cols; c++) {
                                    if (sec->table_rows[r][c]) {
                                        ap_draw_text_clipped(body_font, sec->table_rows[r][c],
                                            margin + c * col_w, draw_y,
                                            theme->text, col_w - AP_S(8));
                                    }
                                }
                            }
                            draw_y += line_h + AP_S(4);
                        }
                    }
                    break;
            }

            draw_y += section_gap;
        }

        SDL_RenderSetClipRect(ap_get_renderer(), NULL);

        /* Scrollbar */
        if (max_scroll > 0) {
            int sb_x = screen_w - margin - AP_S(6);
            ap_draw_scrollbar(sb_x, content_y, content_h,
                content_h, total_content_h, scroll_offset);
        }

        if (opts->footer && opts->footer_count > 0) {
            ap_draw_footer(opts->footer, opts->footer_count);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    return result->action == AP_DETAIL_BACK ? AP_CANCELLED : AP_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COLOR PICKER Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

/* 25 predefined colors in a 5x5 grid */
static const ap_color ap__picker_colors[25] = {
    {255,  59,  48, 255}, {255, 149,   0, 255}, {255, 204,   0, 255}, { 52, 199,  89, 255}, {  0, 199, 190, 255},
    { 48, 176, 199, 255}, { 50, 173, 230, 255}, {  0, 122, 255, 255}, { 88,  86, 214, 255}, {175,  82, 222, 255},
    {255,  45,  85, 255}, {162, 132,  94, 255}, {142, 142, 147, 255}, {174, 174, 178, 255}, {199, 199, 204, 255},
    {155,  34,  87, 255}, {128,   0,   0, 255}, {  0, 128,   0, 255}, {  0,   0, 128, 255}, {128, 128,   0, 255},
    {  0, 128, 128, 255}, {128,   0, 128, 255}, {255, 255, 255, 255}, {192, 192, 192, 255}, {  0,   0,   0, 255},
};

int ap_color_picker(ap_color initial, ap_color *result) {
    if (!result) return AP_ERROR;
    *result = initial;

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    int cell_size = AP_S(48);
    int cell_gap  = AP_S(8);
    int grid_size = 5;
    int grid_w = grid_size * (cell_size + cell_gap) - cell_gap;
    int grid_x = (screen_w - grid_w) / 2;
    int grid_y = (screen_h - grid_w) / 2;

    int cx = 0, cy = 0;

    /* Find initial selection */
    for (int i = 0; i < 25; i++) {
        if (ap__picker_colors[i].r == initial.r &&
            ap__picker_colors[i].g == initial.g &&
            ap__picker_colors[i].b == initial.b) {
            cx = i % 5;
            cy = i / 5;
            break;
        }
    }

    bool running = true;
    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_UP:    cy = (cy - 1 + grid_size) % grid_size; break;
                case AP_BTN_DOWN:  cy = (cy + 1) % grid_size; break;
                case AP_BTN_LEFT:  cx = (cx - 1 + grid_size) % grid_size; break;
                case AP_BTN_RIGHT: cx = (cx + 1) % grid_size; break;
                case AP_BTN_A:
                    *result = ap__picker_colors[cy * 5 + cx];
                    return AP_OK;
                case AP_BTN_B:
                    return AP_CANCELLED;
                default: break;
            }
        }

        ap_draw_background();

        /* Title */
        TTF_Font *font = ap_get_font(AP_FONT_SMALL);
        if (font) {
            const char *title = "Select Color";
            int tw = ap_measure_text(font, title);
            ap_draw_text(font, title, (screen_w - tw) / 2, grid_y - AP_S(50), theme->text);
        }

        /* Grid */
        for (int row = 0; row < grid_size; row++) {
            for (int col = 0; col < grid_size; col++) {
                int i = row * 5 + col;
                int x = grid_x + col * (cell_size + cell_gap);
                int y = grid_y + row * (cell_size + cell_gap);

                ap_draw_rounded_rect(x, y, cell_size, cell_size, AP_S(6), ap__picker_colors[i]);

                /* Selection highlight */
                if (row == cy && col == cx) {
                    ap_color border = theme->highlight;
                    int bw = AP_S(3);
                    /* Draw border by drawing a larger rect behind */
                    ap_draw_rounded_rect(x - bw, y - bw,
                        cell_size + bw * 2, cell_size + bw * 2,
                        AP_S(8), border);
                    ap_draw_rounded_rect(x, y, cell_size, cell_size, AP_S(6), ap__picker_colors[i]);
                }
            }
        }

        /* Preview of selected color */
        ap_color sel = ap__picker_colors[cy * 5 + cx];
        int preview_y = grid_y + grid_w + AP_S(20);
        int preview_w = AP_S(100);
        int preview_h = AP_S(40);
        ap_draw_pill((screen_w - preview_w) / 2, preview_y, preview_w, preview_h, sel);

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    return AP_CANCELLED;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HELP OVERLAY Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

void ap_show_help_overlay(const char *text) {
    if (!text || !text[0]) return;

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    TTF_Font *font = ap_get_font(AP_FONT_TINY);
    if (!font) return;

    int margin = AP_S(40);
    int line_h = TTF_FontLineSkip(font);
    int max_w  = screen_w - margin * 2;

    /* Estimate content height */
    int chars_per_line = max_w / (AP_S(10));
    if (chars_per_line < 1) chars_per_line = 1;
    int est_lines = ((int)strlen(text) / chars_per_line) + 2;
    int content_h = est_lines * line_h;
    int scroll = 0;
    int max_scroll = content_h - (screen_h - margin * 2);
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
                    /* Any other button closes the overlay */
                    running = false;
                    break;
            }
        }

        /* Semi-transparent background */
        ap_color overlay_bg = {0, 0, 0, 200};
        ap_draw_rect(0, 0, screen_w, screen_h, overlay_bg);

        /* Scrollable text */
        SDL_Rect clip = {margin, margin, max_w, screen_h - margin * 2};
        SDL_RenderSetClipRect(ap_get_renderer(), &clip);

        ap_draw_text_wrapped(font, text,
            margin, margin - scroll,
            max_w, theme->text, AP_ALIGN_LEFT);

        SDL_RenderSetClipRect(ap_get_renderer(), NULL);

        /* Scrollbar */
        if (max_scroll > 0) {
            ap_draw_scrollbar(screen_w - margin + AP_S(8), margin,
                screen_h - margin * 2,
                screen_h - margin * 2, content_h, scroll);
        }

        /* "Press any button to close" hint */
        TTF_Font *hint_font = ap_get_font(AP_FONT_MICRO);
        if (hint_font) {
            const char *hint = "Press any button to close";
            int hw = ap_measure_text(hint_font, hint);
            ap_draw_text(hint_font, hint,
                (screen_w - hw) / 2,
                screen_h - margin + AP_S(8),
                theme->hint);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }
}

#endif /* AP_WIDGETS_IMPLEMENTATION */
#endif /* APOSTROPHE_WIDGETS_H */
