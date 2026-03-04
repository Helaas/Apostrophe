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
    ap_button        secondary_action_button; /* Secondary action (e.g. Y) */
    ap_button        confirm_button;   /* Confirm/accept action (e.g. START) */
    ap_button        tertiary_action_button; /* Tertiary action (e.g. MENU) */
    bool             show_images;      /* Show image column */
    const char      *help_text;        /* Help overlay text (L1 to show) */
    uint32_t         input_delay;      /* Override input debounce (0 = default) */
    int              initial_index;    /* Starting cursor position */
    int              visible_start_index; /* Initial scroll top index */
} ap_list_opts;

/* Result from closing a list */
typedef struct {
    int             selected_index;    /* Index of selected/confirmed item, or -1 */
    ap_list_action  action;            /* What caused the list to close */
    ap_list_item   *items;             /* Items array (potentially reordered) */
    int             item_count;
    int             visible_start_index; /* Final scroll top index for restoration */
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
    int                initial_selected_index;  /* Initial focused row */
    int                visible_start_index;     /* Initial scroll top index */
    ap_button          action_button;           /* Primary trigger button */
    ap_button          secondary_action_button; /* Secondary trigger button */
    ap_button          confirm_button;  /* Button that confirms/exits (e.g. START) */
    const char        *help_text;
    uint32_t           input_delay;
} ap_options_list_opts;

typedef struct {
    int              focused_index;
    ap_list_action   action;
    ap_options_item *items;
    int              item_count;
    int              visible_start_index;
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
 * Download Manager — multi-threaded file downloader with progress UI
 *
 * Requires libcurl. Link with -lcurl. The widget spawns a thread pool
 * (default 3 concurrent) and displays per-file progress bars with speed.
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    AP_DL_PENDING = 0,     /* Waiting to start */
    AP_DL_DOWNLOADING,     /* In progress */
    AP_DL_COMPLETE,        /* Finished successfully */
    AP_DL_FAILED           /* Error occurred */
} ap_download_status;

/* A single download job */
typedef struct {
    const char          *url;            /* Source URL */
    const char          *dest_path;      /* Destination file path */
    const char          *label;          /* Display label (e.g. filename) */
    ap_download_status   status;         /* Set by download manager */
    float                progress;       /* 0.0–1.0, updated during download */
    double               speed_bps;      /* Bytes per second, updated live */
    int                  http_code;      /* HTTP response code (0 before completion) */
    char                 error[256];     /* Error message if status == AP_DL_FAILED */
} ap_download;

/* Overall result */
typedef struct {
    int  total;
    int  completed;
    int  failed;
    bool cancelled;   /* True if user cancelled */
} ap_download_result;

/* Options */
typedef struct {
    int   max_concurrent;     /* Max simultaneous downloads (default 3) */
    bool  skip_ssl_verify;    /* Disable SSL cert verification */
    const char **headers;     /* NULL-terminated array of "Header: Value" strings */
    int   header_count;
} ap_download_opts;

/* Run the download manager. Blocks until all downloads finish or user cancels.
 * Returns AP_OK when all complete, AP_CANCELLED if user cancelled, AP_ERROR on error. */
int ap_download_manager(ap_download *downloads, int count,
                        ap_download_opts *opts, ap_download_result *result);

/* ═══════════════════════════════════════════════════════════════════════════
 * IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifdef AP_WIDGETS_IMPLEMENTATION

/* ─── Internal widget helpers ────────────────────────────────────────────── */

/* Standard widget loop timing */
#define AP__FRAME_DELAY 16  /* ~60 fps */

/* Draw title bar text at top of screen — ExtraLarge font matching Gabagool */
static void ap__draw_title(const char *title) {
    if (!title || !title[0]) return;
    TTF_Font *font = ap_get_font(AP_FONT_EXTRA_LARGE);
    if (!font) return;

    int margin = AP_DS(ap__g.device_padding + 5); /* inset from screen edge */
    int title_y = 0;
    ap_draw_text(font, title, margin, title_y, ap_get_theme()->text);
}

/* Draw title with width reduced by status bar to prevent overlap */
static void ap__draw_title_clipped(const char *title, int status_bar_w) {
    if (!title || !title[0]) return;
    TTF_Font *font = ap_get_font(AP_FONT_EXTRA_LARGE);
    if (!font) return;

    int margin = AP_DS(ap__g.device_padding + 5);
    int title_y = 0;
    int max_w = ap_get_screen_width() - margin * 2 - status_bar_w;
    ap_draw_text_clipped(font, title, margin, title_y, ap_get_theme()->text, max_w);
}

/* Calculate the usable content area (below title, above footer) */
static void ap__content_area(int *y, int *h, bool has_title, bool has_footer) {
    int top = 0;
    if (has_title) top = AP_DS(40);  /* title zone: EXTRA_LARGE font height + gap */
    int bottom = 0;
    if (has_footer) bottom = ap_get_footer_height();
    *y = top;
    *h = ap_get_screen_height() - top - bottom;
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
    opts.secondary_action_button = AP_BTN_NONE;
    opts.confirm_button = AP_BTN_NONE;
    opts.tertiary_action_button = AP_BTN_NONE;
    opts.show_images = false;
    opts.help_text = NULL;
    opts.input_delay = 0;
    opts.initial_index = 0;
    opts.visible_start_index = 0;
    return opts;
}

int ap_list(ap_list_opts *opts, ap_list_result *result) {
    if (!opts || !result) return AP_ERROR;
    if (!opts->items || opts->item_count <= 0) return AP_ERROR;

    memset(result, 0, sizeof(*result));
    result->items = opts->items;
    result->item_count = opts->item_count;
    result->selected_index = -1;
    result->visible_start_index = 0;

    /* Save/restore input delay */
    uint32_t saved_delay = 0;
    if (opts->input_delay > 0) {
        saved_delay = opts->input_delay;
        ap_set_input_delay(opts->input_delay);
    }

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();

    TTF_Font *title_font = ap_get_font(AP_FONT_EXTRA_LARGE);
    TTF_Font *item_font  = ap_get_font(AP_FONT_LARGE); /* NextUI uses font.large for menu items */
    if (!title_font || !item_font) return AP_ERROR;

    /* Layout constants — match NextUI: PILL_SIZE × device_scale, no gap */
    int margin     = AP_DS(ap__g.device_padding);
    int pill_h     = AP_DS(AP__PILL_SIZE);
    int pill_pad   = AP_DS(AP__BUTTON_PADDING);
    int item_gap   = 0;
    int image_size = opts->show_images ? AP_DS(24) : 0;
    int image_pad  = opts->show_images ? AP_DS(6) : 0;

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
    int scroll_top = opts->visible_start_index;
    if (scroll_top < 0) scroll_top = 0;
    if (scroll_top > opts->item_count - max_visible) scroll_top = opts->item_count - max_visible;
    if (scroll_top < 0) scroll_top = 0;
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
                        break;
                    }
                    /* Check action button */
                    if (opts->action_button != AP_BTN_NONE && ev.button == opts->action_button) {
                        result->selected_index = cursor;
                        result->action = AP_ACTION_TRIGGERED;
                        running = false;
                        break;
                    }
                    if (opts->secondary_action_button != AP_BTN_NONE && ev.button == opts->secondary_action_button) {
                        result->selected_index = cursor;
                        result->action = AP_ACTION_SECONDARY_TRIGGERED;
                        running = false;
                        break;
                    }
                    if (opts->confirm_button != AP_BTN_NONE && ev.button == opts->confirm_button) {
                        result->selected_index = cursor;
                        result->action = AP_ACTION_CONFIRMED;
                        running = false;
                        break;
                    }
                    if (opts->tertiary_action_button != AP_BTN_NONE && ev.button == opts->tertiary_action_button) {
                        result->selected_index = cursor;
                        result->action = AP_ACTION_TERTIARY_TRIGGERED;
                        running = false;
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

        /* Title (clipped if status bar present) */
        if (opts->title) {
            int sb_w = opts->status_bar ? ap_get_status_bar_width(opts->status_bar) : 0;
            if (sb_w > 0)
                ap__draw_title_clipped(opts->title, sb_w + AP_S(10));
            else
                ap__draw_title(opts->title);
        }

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
                        ap_draw_rect(cb_x, cb_y, cb_size, cb_size, theme->accent);
                        ap_draw_text(item_font, "✓", cb_x + AP_S(2), cb_y - AP_S(2), theme->highlighted_text);
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

    result->visible_start_index = scroll_top;

    /* Restore input delay */
    if (saved_delay > 0) {
        ap_set_input_delay(AP_INPUT_DEBOUNCE);
    }

    return result->action == AP_ACTION_BACK ? AP_CANCELLED : AP_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * OPTIONS LIST Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

static int ap__options_valid_index(ap_options_item *item) {
    if (!item || item->option_count <= 0 || !item->options) return -1;
    if (item->selected_option < 0) item->selected_option = 0;
    if (item->selected_option >= item->option_count) {
        item->selected_option = item->option_count - 1;
    }
    return item->selected_option;
}

int ap_options_list(ap_options_list_opts *opts, ap_options_list_result *result) {
    if (!opts || !result) return AP_ERROR;
    if (!opts->items || opts->item_count <= 0) return AP_ERROR;

    memset(result, 0, sizeof(*result));
    result->items = opts->items;
    result->item_count = opts->item_count;
    result->focused_index = 0;
    result->visible_start_index = 0;

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();

    TTF_Font *label_font = ap_get_font(AP_FONT_LARGE); /* NextUI uses font.large for option labels */
    TTF_Font *value_font = ap_get_font(AP_FONT_TINY);
    if (!label_font || !value_font) return AP_ERROR;

    int margin   = AP_DS(ap__g.device_padding);
    int pill_h   = AP_DS(AP__PILL_SIZE);
    int pill_pad = AP_DS(AP__BUTTON_PADDING);
    int item_gap = AP_DS(2);
    int arrow_w  = AP_DS(12);

    int content_y, content_h;
    ap__content_area(&content_y, &content_h, opts->title != NULL, opts->footer_count > 0);
    int max_visible = content_h / (pill_h + item_gap);
    if (max_visible < 1) max_visible = 1;

    int cursor = opts->initial_selected_index;
    if (cursor < 0) cursor = 0;
    if (cursor >= opts->item_count) cursor = opts->item_count - 1;

    int scroll_top = opts->visible_start_index;
    if (scroll_top < 0) scroll_top = 0;
    if (scroll_top > opts->item_count - max_visible) scroll_top = opts->item_count - max_visible;
    if (scroll_top < 0) scroll_top = 0;
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
                    int sel = ap__options_valid_index(item);
                    if (item->type == AP_OPT_STANDARD && sel >= 0) {
                        sel--;
                        if (sel < 0) sel = item->option_count - 1;
                        item->selected_option = sel;
                    }
                    break;
                }

                case AP_BTN_RIGHT: {
                    ap_options_item *item = &opts->items[cursor];
                    int sel = ap__options_valid_index(item);
                    if (item->type == AP_OPT_STANDARD && sel >= 0) {
                        sel++;
                        if (sel >= item->option_count) sel = 0;
                        item->selected_option = sel;
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
                        int sel = ap__options_valid_index(item);
                        if (sel >= 0 && item->options[sel].value) {
                            initial = item->options[sel].value;
                        }
                        int kb_ret = ap_keyboard(initial, "B: Cancel", AP_KB_GENERAL, &kb_result);
                        if (kb_ret == AP_OK) {
                            /* Update the option value — caller must manage memory */
                            if (sel >= 0) {
                                item->options[sel].value = strdup(kb_result.text);
                                item->options[sel].label = strdup(kb_result.text);
                            }
                        }
                    } else if (item->type == AP_OPT_COLOR_PICKER) {
                        ap_color initial_color = {255, 255, 255, 255};
                        ap_color picked;
                        if (ap_color_picker(initial_color, &picked) == AP_OK) {
                            /* Store color as hex string */
                            /* Caller handles this via result */
                        }
                    } else if (item->type == AP_OPT_STANDARD) {
                        /* Cycle forward on A for standard options */
                        int sel = ap__options_valid_index(item);
                        if (sel >= 0) {
                            sel++;
                            if (sel >= item->option_count) sel = 0;
                            item->selected_option = sel;
                        }
                    }
                    break;
                }

                case AP_BTN_B:
                    result->focused_index = cursor;
                    result->action = AP_ACTION_BACK;
                    running = false;
                    break;

                default:
                    if (opts->confirm_button != AP_BTN_NONE && ev.button == opts->confirm_button) {
                        result->focused_index = cursor;
                        result->action = AP_ACTION_CONFIRMED;
                        running = false;
                    } else if (opts->action_button != AP_BTN_NONE && ev.button == opts->action_button) {
                        result->focused_index = cursor;
                        result->action = AP_ACTION_TRIGGERED;
                        running = false;
                    } else if (opts->secondary_action_button != AP_BTN_NONE && ev.button == opts->secondary_action_button) {
                        result->focused_index = cursor;
                        result->action = AP_ACTION_SECONDARY_TRIGGERED;
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
            int valid_opt = ap__options_valid_index(item);
            if (valid_opt >= 0) {
                value = item->options[valid_opt].label;
                if (!value) value = item->options[valid_opt].value;
                if (!value) value = "";
            }

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
                if (item->type == AP_OPT_STANDARD && valid_opt >= 0 && item->option_count > 1) {
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

    result->visible_start_index = scroll_top;

    return result->action == AP_ACTION_BACK ? AP_CANCELLED : AP_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * KEYBOARD Implementation — matches Gabagool 5-row layout with special keys
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Special key identifiers used internally for cursor tracking.
 * These are negative to distinguish them from character-key grid indices. */
#define AP__KB_KEY_BACKSPACE  (-1)
#define AP__KB_KEY_ENTER      (-2)
#define AP__KB_KEY_SHIFT      (-3)
#define AP__KB_KEY_SYMBOL     (-4)
#define AP__KB_KEY_SPACE      (-5)

/* 5-row general layout — 12-column virtual grid.
 * Row 0: 1-0 keys (10) + backspace (2 cols)
 * Row 1: qwertyuiop (10 cols, centered)
 * Row 2: asdfghjkl (9 cols) + enter (1.5 cols)
 * Row 3: shift (2 cols) + zxcvbnm (7 cols) + symbol (2 cols)
 * Row 4: space (8 cols, centered)
 */

/* Lower-case keys per row (NULL-terminated) */
static const char *ap__kb5_row0_lower[] = {"1","2","3","4","5","6","7","8","9","0", NULL};
static const char *ap__kb5_row1_lower[] = {"q","w","e","r","t","y","u","i","o","p", NULL};
static const char *ap__kb5_row2_lower[] = {"a","s","d","f","g","h","j","k","l", NULL};
static const char *ap__kb5_row3_lower[] = {"z","x","c","v","b","n","m", NULL};

/* Upper-case */
static const char *ap__kb5_row0_upper[] = {"1","2","3","4","5","6","7","8","9","0", NULL};
static const char *ap__kb5_row1_upper[] = {"Q","W","E","R","T","Y","U","I","O","P", NULL};
static const char *ap__kb5_row2_upper[] = {"A","S","D","F","G","H","J","K","L", NULL};
static const char *ap__kb5_row3_upper[] = {"Z","X","C","V","B","N","M", NULL};

/* Symbols mode */
static const char *ap__kb5_row0_sym[] = {"!","@","#","$","%","^","&","*","(",")", NULL};
static const char *ap__kb5_row1_sym[] = {"`","~","[","]","\\","|","{","}",";",":", NULL};
static const char *ap__kb5_row2_sym[] = {"'","\"","<",">","?","/","+","=","_", NULL};
static const char *ap__kb5_row3_sym[] = {",",".","-","\xe2\x82\xac","\xc2\xa3","\xc2\xa5","\xc2\xa2", NULL};

/* Numeric layout */
static const char *ap__kb_numeric[] = {
    "1", "2", "3",
    "4", "5", "6",
    "7", "8", "9",
    NULL, "0", NULL,
};
#define AP__KB_COLS_NUMERIC  3
#define AP__KB_ROWS_NUMERIC  4

/* Keyboard row metadata for 5-row general layout */
typedef struct {
    const char **chars;       /* pointer to char array for this row */
    int          char_count;  /* number of character keys */
    int          special;     /* special key at end: AP__KB_KEY_* or 0 = none */
    int          special_pre; /* special key at start: AP__KB_KEY_* or 0 = none */
} ap__kb_row_info;

static int ap__kb5_count(const char **row) {
    int n = 0;
    while (row[n]) n++;
    return n;
}

/* Helper: insert a string at text_cursor in result->text */
static void ap__kb_insert(ap_keyboard_result *result, int *text_cursor, const char *str) {
    int len = (int)strlen(result->text);
    int slen = (int)strlen(str);
    if (len + slen < (int)sizeof(result->text) - 1) {
        memmove(result->text + *text_cursor + slen,
                result->text + *text_cursor,
                len - *text_cursor + 1);
        memcpy(result->text + *text_cursor, str, slen);
        *text_cursor += slen;
    }
}

/* Helper: backspace at text_cursor */
static void ap__kb_backspace(ap_keyboard_result *result, int *text_cursor) {
    if (*text_cursor > 0) {
        int len = (int)strlen(result->text);
        memmove(result->text + *text_cursor - 1,
                result->text + *text_cursor,
                len - *text_cursor + 1);
        (*text_cursor)--;
    }
}

/* Built-in keyboard help text matching Gabagool's instructions */
static const char *ap__kb_help_default =
    "D-Pad: Navigate between keys\n"
    "A: Type the selected key\n"
    "B: Backspace\n"
    "X: Space\n"
    "L1 / R1: Move cursor within text\n"
    "Select: Toggle Shift (uppercase/symbols)\n"
    "Y: Exit keyboard without saving\n"
    "Start: Enter (confirm input)";

static const char *ap__kb_help_numeric =
    "D-Pad: Navigate between keys\n"
    "A: Type the selected digit\n"
    "B: Backspace\n"
    "L1 / R1: Move cursor within text\n"
    "Y: Exit keyboard without saving\n"
    "Start: Enter (confirm input)";

static const char *ap__kb_help_url =
    "D-Pad: Navigate between keys\n"
    "A: Type the selected key\n"
    "B: Backspace\n"
    "X: Toggle symbols (0-9)\n"
    "L1 / R1: Move cursor within text\n"
    "Select: Toggle Shift (uppercase)\n"
    "Y: Exit keyboard without saving\n"
    "Start: Enter (confirm input)";

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

    TTF_Font *text_font    = ap_get_font(AP_FONT_MEDIUM);
    TTF_Font *key_font     = ap_get_font(AP_FONT_MEDIUM);   /* Gabagool: MediumFont for keys */
    TTF_Font *special_font = ap_get_font(AP_FONT_LARGE);    /* Gabagool: LargeFont for special keys */
    if (!text_font || !key_font) return AP_ERROR;

    bool is_numeric = (layout == AP_KB_NUMERIC);

    /* ── Numeric keyboard (simple 3×4 grid) ── */
    if (is_numeric) {
        int kb_cols = AP__KB_COLS_NUMERIC;
        int kb_rows = AP__KB_ROWS_NUMERIC;
        int key_w = AP_S(80);
        int key_h = AP_S(52);
        int key_gap = AP_S(6);
        int key_r = AP_S(8);
        int grid_w = kb_cols * (key_w + key_gap) - key_gap;
        int grid_x = (screen_w - grid_w) / 2;
        int grid_y = screen_h - (kb_rows * (key_h + key_gap)) - ap_get_footer_height();
        int input_y = AP_S(40), input_h = AP_S(60), input_x = AP_S(40);
        int input_w = screen_w - AP_S(80);
        int cursor_x = 0, cursor_y = 0;
        int text_cursor = (int)strlen(result->text);
        bool running = true;
        uint32_t caret_blink = SDL_GetTicks();
        bool caret_visible = true;

        while (running) {
            uint32_t now = SDL_GetTicks();
            if (now - caret_blink > 500) { caret_visible = !caret_visible; caret_blink = now; }

            ap_input_event ev;
            while (ap_poll_input(&ev)) {
                if (!ev.pressed) continue;
                switch (ev.button) {
                    case AP_BTN_UP:    cursor_y = (cursor_y - 1 + kb_rows) % kb_rows; break;
                    case AP_BTN_DOWN:  cursor_y = (cursor_y + 1) % kb_rows; break;
                    case AP_BTN_LEFT:  cursor_x = (cursor_x - 1 + kb_cols) % kb_cols; break;
                    case AP_BTN_RIGHT: cursor_x = (cursor_x + 1) % kb_cols; break;
                    case AP_BTN_A: {
                        int ki = cursor_y * kb_cols + cursor_x;
                        if (ki < kb_rows * kb_cols && ap__kb_numeric[ki])
                            ap__kb_insert(result, &text_cursor, ap__kb_numeric[ki]);
                        break;
                    }
                    case AP_BTN_B: ap__kb_backspace(result, &text_cursor); break;
                    case AP_BTN_Y: return AP_CANCELLED;
                    case AP_BTN_START: return AP_OK;
                    case AP_BTN_L1: if (text_cursor > 0) text_cursor--; break;
                    case AP_BTN_R1: if (text_cursor < (int)strlen(result->text)) text_cursor++; break;
                    case AP_BTN_MENU:
                        ap_show_help_overlay(help_text ? help_text : ap__kb_help_numeric);
                        break;
                    default: break;
                }
            }

            ap_draw_background();
            ap_draw_pill(input_x, input_y, input_w, input_h, theme->highlight);
            {
                int ty = input_y + (input_h - TTF_FontHeight(text_font)) / 2;
                int tx = input_x + AP_S(16);
                if (result->text[0])
                    ap_draw_text_clipped(text_font, result->text, tx, ty, theme->highlighted_text, input_w - AP_S(32));
                if (caret_visible) {
                    char saved = result->text[text_cursor]; result->text[text_cursor] = '\0';
                    int cx = tx + ap_measure_text(text_font, result->text);
                    result->text[text_cursor] = saved;
                    ap_draw_rect(cx, ty, AP_S(2), TTF_FontHeight(text_font), theme->highlighted_text);
                }
            }
            for (int row = 0; row < kb_rows; row++) {
                for (int col = 0; col < kb_cols; col++) {
                    int ki = row * kb_cols + col;
                    const char *key = ap__kb_numeric[ki];
                    if (!key) continue;
                    int kx = grid_x + col * (key_w + key_gap);
                    int ky = grid_y + row * (key_h + key_gap);
                    bool sel = (row == cursor_y && col == cursor_x);
                    ap_draw_rounded_rect(kx, ky, key_w, key_h, key_r, sel ? theme->highlight : theme->accent);
                    int tw = ap_measure_text(key_font, key);
                    ap_draw_text(key_font, key, kx + (key_w - tw)/2, ky + (key_h - TTF_FontHeight(key_font))/2,
                                 sel ? theme->highlighted_text : theme->hint);
                }
            }
            ap_present();
            SDL_Delay(AP__FRAME_DELAY);
        }
        return AP_CANCELLED;
    }

    /* ═══════════════════════════════════════════════════════════════════════
     * 5-Row General Keyboard (matches Gabagool)
     * ═══════════════════════════════════════════════════════════════════════ */

    /* Keyboard occupies available space minus footer, text input = screen_h/10 */
    int footer_h = ap_get_footer_height();
    int kb_area_h  = screen_h * 85 / 100 - footer_h;
    int input_h    = screen_h / 10;
    int input_y    = (screen_h - kb_area_h - footer_h - input_h) / 2;
    int input_x    = AP_S(40);
    int input_w    = screen_w - AP_S(80);

    /* 12-column grid, 5 key rows (footer handled separately) */
    int grid_cols = 12;
    int grid_rows_total = 5;
    int key_spacing = AP_S(4);
    int key_w = (screen_w - AP_S(40) - key_spacing * (grid_cols - 1)) / grid_cols;
    int key_h = (kb_area_h - key_spacing * (grid_rows_total - 1)) / grid_rows_total;
    int key_r = AP_S(6);

    /* Grid top-left */
    int grid_x = (screen_w - (grid_cols * (key_w + key_spacing) - key_spacing)) / 2;
    int grid_y = input_y + input_h + AP_S(10);

    /* State */
    int cursor_row = 1;  /* Start on qwerty row */
    int cursor_col = 0;
    bool shift = false;
    bool symbols = false;
    int text_cursor = (int)strlen(result->text);
    bool running = true;

    uint32_t caret_blink = SDL_GetTicks();
    bool caret_visible = true;

    /* Current key arrays per row */
    #define AP__KB5_ROW_COUNT 5

    while (running) {
        uint32_t now = SDL_GetTicks();
        if (now - caret_blink > 500) { caret_visible = !caret_visible; caret_blink = now; }

        /* Build current row info from mode */
        const char **r0 = symbols ? ap__kb5_row0_sym : (shift ? ap__kb5_row0_upper : ap__kb5_row0_lower);
        const char **r1 = symbols ? ap__kb5_row1_sym : (shift ? ap__kb5_row1_upper : ap__kb5_row1_lower);
        const char **r2 = symbols ? ap__kb5_row2_sym : (shift ? ap__kb5_row2_upper : ap__kb5_row2_lower);
        const char **r3 = symbols ? ap__kb5_row3_sym : (shift ? ap__kb5_row3_upper : ap__kb5_row3_lower);

        int r0n = ap__kb5_count(r0);  /* 10 */
        int r1n = ap__kb5_count(r1);  /* 10 */
        int r2n = ap__kb5_count(r2);  /* 9 */
        int r3n = ap__kb5_count(r3);  /* 7 */

        /* Effective navigation column limits per row:
         * Row 0: 0..10 (10 chars + backspace at col 10)
         * Row 1: 0..9  (10 chars)
         * Row 2: 0..9  (9 chars + enter at col 9)
         * Row 3: 0..8  (shift at 0, then 7 chars at 1..7, symbol at 8)
         * Row 4: 0 only (space bar)  */
        int row_max_col[AP__KB5_ROW_COUNT] = {10, 9, 9, 8, 0};

        /* Input */
        ap_input_event iev;
        while (ap_poll_input(&iev)) {
            if (!iev.pressed) continue;

            switch (iev.button) {
                case AP_BTN_UP:
                    cursor_row = (cursor_row - 1 + AP__KB5_ROW_COUNT) % AP__KB5_ROW_COUNT;
                    if (cursor_col > row_max_col[cursor_row])
                        cursor_col = row_max_col[cursor_row];
                    break;
                case AP_BTN_DOWN:
                    cursor_row = (cursor_row + 1) % AP__KB5_ROW_COUNT;
                    if (cursor_col > row_max_col[cursor_row])
                        cursor_col = row_max_col[cursor_row];
                    break;
                case AP_BTN_LEFT:
                    cursor_col--;
                    if (cursor_col < 0) cursor_col = row_max_col[cursor_row];
                    break;
                case AP_BTN_RIGHT:
                    cursor_col++;
                    if (cursor_col > row_max_col[cursor_row]) cursor_col = 0;
                    break;

                case AP_BTN_A: {
                    /* Type the selected character or activate special key */
                    if (cursor_row == 0) {
                        if (cursor_col < r0n)
                            ap__kb_insert(result, &text_cursor, r0[cursor_col]);
                        else /* backspace */
                            ap__kb_backspace(result, &text_cursor);
                    } else if (cursor_row == 1) {
                        if (cursor_col < r1n)
                            ap__kb_insert(result, &text_cursor, r1[cursor_col]);
                    } else if (cursor_row == 2) {
                        if (cursor_col < r2n)
                            ap__kb_insert(result, &text_cursor, r2[cursor_col]);
                        else /* enter = confirm */
                            return AP_OK;
                    } else if (cursor_row == 3) {
                        if (cursor_col == 0) {
                            /* shift */
                            shift = !shift; symbols = false;
                        } else if (cursor_col <= r3n) {
                            ap__kb_insert(result, &text_cursor, r3[cursor_col - 1]);
                        } else {
                            /* symbol */
                            symbols = !symbols; shift = false;
                        }
                    } else if (cursor_row == 4) {
                        /* space bar */
                        ap__kb_insert(result, &text_cursor, " ");
                    }
                    break;
                }

                case AP_BTN_B:
                    /* Backspace (Gabagool mapping) */
                    ap__kb_backspace(result, &text_cursor);
                    break;

                case AP_BTN_X:
                    /* Space (Gabagool mapping for general keyboard) */
                    ap__kb_insert(result, &text_cursor, " ");
                    break;

                case AP_BTN_Y:
                    /* Exit without saving (Gabagool mapping) */
                    return AP_CANCELLED;

                case AP_BTN_SELECT:
                    /* Toggle shift (Gabagool mapping) */
                    shift = !shift; symbols = false;
                    break;

                case AP_BTN_START:
                    /* Enter / confirm */
                    return AP_OK;

                case AP_BTN_L1:
                    /* Move text cursor left */
                    if (text_cursor > 0) text_cursor--;
                    break;

                case AP_BTN_R1:
                    /* Move text cursor right */
                    if (text_cursor < (int)strlen(result->text)) text_cursor++;
                    break;

                case AP_BTN_MENU:
                    /* Help overlay — use built-in instructions */
                    ap_show_help_overlay(ap__kb_help_default);
                    break;

                default: break;
            }
        }

        /* ── Render ── */
        ap_draw_background();

        /* Text input field */
        ap_draw_rounded_rect(input_x, input_y, input_w, input_h, AP_S(8), theme->highlight);

        {
            int ty = input_y + (input_h - TTF_FontHeight(text_font)) / 2;
            int tx = input_x + AP_S(16);
            if (result->text[0])
                ap_draw_text_clipped(text_font, result->text, tx, ty,
                    theme->highlighted_text, input_w - AP_S(32));
            if (caret_visible) {
                char saved = result->text[text_cursor]; result->text[text_cursor] = '\0';
                int cx = tx + ap_measure_text(text_font, result->text);
                result->text[text_cursor] = saved;
                ap_draw_rect(cx, ty, AP_S(2), TTF_FontHeight(text_font), theme->highlighted_text);
            }
        }

        /* ── Key rendering ── */
        ap_color key_bg_normal  = theme->accent;
        ap_color key_bg_sel     = theme->highlight;
        ap_color key_fg_normal  = theme->hint;
        ap_color key_fg_sel     = theme->highlighted_text;

        /* Helper macro: draw a single key rectangle with text */
        #define AP__KB_DRAW_KEY(x, y, w, h, label, is_sel, font_to_use) do { \
            ap_color _bg = (is_sel) ? key_bg_sel : key_bg_normal; \
            ap_color _fg = (is_sel) ? key_fg_sel : key_fg_normal; \
            ap_draw_rounded_rect((x), (y), (w), (h), key_r, _bg); \
            if (label) { \
                int _tw = ap_measure_text((font_to_use), (label)); \
                int _th = TTF_FontHeight((font_to_use)); \
                ap_draw_text((font_to_use), (label), \
                    (x) + ((w) - _tw) / 2, (y) + ((h) - _th) / 2, _fg); \
            } \
        } while(0)

        /* Row 0: numbers + backspace */
        {
            int row_y = grid_y;
            int cx = grid_x;
            for (int i = 0; i < r0n; i++) {
                bool sel = (cursor_row == 0 && cursor_col == i);
                AP__KB_DRAW_KEY(cx, row_y, key_w, key_h, r0[i], sel, key_font);
                cx += key_w + key_spacing;
            }
            /* Backspace — 2× width */
            {
                int bw = key_w * 2 + key_spacing;
                bool sel = (cursor_row == 0 && cursor_col == r0n);
                AP__KB_DRAW_KEY(cx, row_y, bw, key_h, "\xe2\x86\x90", sel, special_font ? special_font : key_font); /* ← */
            }
        }

        /* Row 1: qwertyuiop (centered) */
        {
            int row_y = grid_y + (key_h + key_spacing);
            int row_w = r1n * (key_w + key_spacing) - key_spacing;
            int cx = (screen_w - row_w) / 2;
            for (int i = 0; i < r1n; i++) {
                bool sel = (cursor_row == 1 && cursor_col == i);
                AP__KB_DRAW_KEY(cx, row_y, key_w, key_h, r1[i], sel, key_font);
                cx += key_w + key_spacing;
            }
        }

        /* Row 2: asdfghjkl + enter */
        {
            int row_y = grid_y + 2 * (key_h + key_spacing);
            /* 9 keys + enter (1.5× width) */
            int enter_w = key_w * 3 / 2 + key_spacing / 2;
            int row_w = r2n * (key_w + key_spacing) + enter_w;
            int cx = (screen_w - row_w) / 2;
            for (int i = 0; i < r2n; i++) {
                bool sel = (cursor_row == 2 && cursor_col == i);
                AP__KB_DRAW_KEY(cx, row_y, key_w, key_h, r2[i], sel, key_font);
                cx += key_w + key_spacing;
            }
            /* Enter */
            {
                bool sel = (cursor_row == 2 && cursor_col == r2n);
                AP__KB_DRAW_KEY(cx, row_y, enter_w, key_h, "\xe2\x86\xb5", sel, special_font ? special_font : key_font); /* ↵ */
            }
        }

        /* Row 3: shift + zxcvbnm + symbol */
        {
            int row_y = grid_y + 3 * (key_h + key_spacing);
            int shift_w = key_w * 2 + key_spacing;
            int sym_w   = key_w * 2 + key_spacing;
            int row_w = shift_w + key_spacing + r3n * (key_w + key_spacing) - key_spacing + key_spacing + sym_w;
            int cx = (screen_w - row_w) / 2;
            /* Shift */
            {
                bool sel = (cursor_row == 3 && cursor_col == 0);
                AP__KB_DRAW_KEY(cx, row_y, shift_w, key_h, "\xe2\x87\xa7", sel, special_font ? special_font : key_font); /* ⇧ */
                cx += shift_w + key_spacing;
            }
            /* Character keys */
            for (int i = 0; i < r3n; i++) {
                bool sel = (cursor_row == 3 && cursor_col == i + 1);
                AP__KB_DRAW_KEY(cx, row_y, key_w, key_h, r3[i], sel, key_font);
                cx += key_w + key_spacing;
            }
            /* Symbol toggle */
            {
                bool sel = (cursor_row == 3 && cursor_col == r3n + 1);
                AP__KB_DRAW_KEY(cx, row_y, sym_w, key_h, symbols ? "ABC" : "#+=", sel, key_font);
            }
        }

        /* Row 4: space bar (8× width, centered) */
        {
            int row_y = grid_y + 4 * (key_h + key_spacing);
            int space_w = key_w * 8 + key_spacing * 7;
            int sx = (screen_w - space_w) / 2;
            bool sel = (cursor_row == 4);
            ap_color bg = sel ? key_bg_sel : key_bg_normal;
            ap_draw_rounded_rect(sx, row_y, space_w, key_h, key_r, bg);
            /* Space indicator: centered line */
            int line_w = space_w / 3;
            int line_h = AP_S(3);
            int line_x = sx + (space_w - line_w) / 2;
            int line_y = row_y + (key_h - line_h) / 2;
            ap_color line_c = sel ? key_fg_sel : key_fg_normal;
            ap_draw_rect(line_x, line_y, line_w, line_h, line_c);
        }

        #undef AP__KB_DRAW_KEY

        /* Footer: always show Help hint (built-in instructions available) */
        {
            ap_footer_item kb_footer[] = {
                {AP_BTN_MENU, "HELP", false},
            };
            ap_draw_footer(kb_footer, 1);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    return AP_CANCELLED;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * URL KEYBOARD Implementation — Gabagool-matching with shortcut rows
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Default URL shortcuts (normal mode) */
static const char *ap__url_shortcuts_default[] = {
    "https://", "www.", ".com", ".org", ".net",
    ".io", ".dev", ".app", ".edu", ".gov",
};
/* Symbol-alternate shortcuts */
static const char *ap__url_shortcuts_alt[] = {
    "http://", "ftp://", ".co", ".tv", ".me",
    ".gg", ".uk", ".de", ".ca", ".au",
};
/* URL-specific character row */
static const char *ap__url_chars[] = {"/",":","@","-","_",".","~","?","#","&", NULL};

int ap_url_keyboard(const char *initial_text, const char *help_text,
                    ap_url_keyboard_config *cfg, ap_keyboard_result *result) {
    if (!result) return AP_ERROR;

    memset(result, 0, sizeof(*result));
    if (initial_text)
        strncpy(result->text, initial_text, sizeof(result->text) - 1);

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    TTF_Font *text_font    = ap_get_font(AP_FONT_MEDIUM);
    TTF_Font *key_font     = ap_get_font(AP_FONT_MEDIUM);
    TTF_Font *shortcut_font = ap_get_font(AP_FONT_SMALL);
    TTF_Font *special_font = ap_get_font(AP_FONT_LARGE);
    if (!text_font || !key_font) return AP_ERROR;

    /* Use provided shortcuts or defaults */
    const char **shortcuts = ap__url_shortcuts_default;
    const char **shortcuts_alt = ap__url_shortcuts_alt;
    int shortcut_count = 10;
    if (cfg && cfg->shortcut_keys && cfg->shortcut_count > 0) {
        shortcuts = cfg->shortcut_keys;
        shortcut_count = cfg->shortcut_count;
        if (shortcut_count > 10) shortcut_count = 10;
        shortcuts_alt = NULL; /* No alternates for custom shortcuts */
    }

    /* Layout: 5-row if ≤5 shortcuts, 6-row if 6-10 */
    int shortcut_rows = (shortcut_count > 5) ? 2 : 1;
    int shortcuts_per_row = (shortcut_rows == 2) ? (shortcut_count + 1) / 2 : shortcut_count;
    if (shortcuts_per_row > 5) shortcuts_per_row = 5;

    /* Total rows: shortcut_rows + url_chars + qwerty + asdf + zxcv (no space bar in URL mode) */
    int total_rows = shortcut_rows + 4; /* url chars + 3 QWERTY rows */

    /* Keyboard sizing */
    int kb_area_h = screen_h * 85 / 100;
    int input_h = screen_h / 10;
    int input_y = (screen_h - kb_area_h - input_h) / 2;
    int input_x = AP_S(40);
    int input_w = screen_w - AP_S(80);
    int key_spacing = AP_S(4);
    int grid_cols = 12;
    int key_w = (screen_w - AP_S(40) - key_spacing * (grid_cols - 1)) / grid_cols;
    int key_h = (kb_area_h - key_spacing * (total_rows + 1)) / (total_rows + 1);
    int key_r = AP_S(6);
    int grid_y = input_y + input_h + AP_S(10);

    /* State */
    int cursor_row = shortcut_rows; /* start on url chars row */
    int cursor_col = 0;
    bool shift = false;
    bool sym_alt = false; /* toggle for shortcut alternates */
    int text_cursor = (int)strlen(result->text);
    bool running = true;
    uint32_t caret_blink = SDL_GetTicks();
    bool caret_visible = true;

    /* Row max columns for navigation */
    /* shortcut rows: shortcuts_per_row-1 (+ backspace on last shortcut row) */
    /* url_chars row: 9 */
    /* qwerty rows: same as general kb rows 1,2,3 but no space */

    while (running) {
        uint32_t now = SDL_GetTicks();
        if (now - caret_blink > 500) { caret_visible = !caret_visible; caret_blink = now; }

        /* Current active shortcuts */
        const char **active_shortcuts = (sym_alt && shortcuts_alt) ? shortcuts_alt : shortcuts;

        /* Row max col computation */
        int row_max[8] = {0}; /* up to 8 rows max */
        int ri = 0;
        for (int sr = 0; sr < shortcut_rows; sr++, ri++) {
            int first = sr * shortcuts_per_row;
            int cnt = 0;
            for (int i = first; i < shortcut_count && i < first + shortcuts_per_row; i++) cnt++;
            row_max[ri] = (sr == shortcut_rows - 1) ? cnt : cnt - 1; /* last shortcut row has backspace */
        }
        int url_chars_row = ri; ri++;
        row_max[url_chars_row] = 9;
        int qwerty_row = ri; ri++;
        row_max[qwerty_row] = 9;
        int asdf_row = ri; ri++;
        row_max[asdf_row] = 9; /* 9 + enter */
        int zxcv_row = ri;
        row_max[zxcv_row] = 8; /* shift + 7 + symbol */
        int actual_total_rows = ri + 1;

        /* Input */
        ap_input_event iev;
        while (ap_poll_input(&iev)) {
            if (!iev.pressed) continue;
            switch (iev.button) {
                case AP_BTN_UP:
                    cursor_row = (cursor_row - 1 + actual_total_rows) % actual_total_rows;
                    if (cursor_col > row_max[cursor_row]) cursor_col = row_max[cursor_row];
                    break;
                case AP_BTN_DOWN:
                    cursor_row = (cursor_row + 1) % actual_total_rows;
                    if (cursor_col > row_max[cursor_row]) cursor_col = row_max[cursor_row];
                    break;
                case AP_BTN_LEFT:
                    cursor_col--;
                    if (cursor_col < 0) cursor_col = row_max[cursor_row];
                    break;
                case AP_BTN_RIGHT:
                    cursor_col++;
                    if (cursor_col > row_max[cursor_row]) cursor_col = 0;
                    break;

                case AP_BTN_A: {
                    /* Shortcut rows */
                    if (cursor_row < shortcut_rows) {
                        int first = cursor_row * shortcuts_per_row;
                        if (cursor_row == shortcut_rows - 1 && cursor_col == row_max[cursor_row]) {
                            /* Backspace on last shortcut row */
                            ap__kb_backspace(result, &text_cursor);
                        } else {
                            int si = first + cursor_col;
                            if (si < shortcut_count)
                                ap__kb_insert(result, &text_cursor, active_shortcuts[si]);
                        }
                    }
                    /* URL chars row */
                    else if (cursor_row == url_chars_row) {
                        if (cursor_col < ap__kb5_count((const char **)ap__url_chars))
                            ap__kb_insert(result, &text_cursor, ap__url_chars[cursor_col]);
                    }
                    /* QWERTY row */
                    else if (cursor_row == qwerty_row) {
                        const char **r = shift ? ap__kb5_row1_upper : ap__kb5_row1_lower;
                        if (cursor_col < ap__kb5_count(r))
                            ap__kb_insert(result, &text_cursor, r[cursor_col]);
                    }
                    /* ASDF row */
                    else if (cursor_row == asdf_row) {
                        const char **r = shift ? ap__kb5_row2_upper : ap__kb5_row2_lower;
                        int rn = ap__kb5_count(r);
                        if (cursor_col < rn)
                            ap__kb_insert(result, &text_cursor, r[cursor_col]);
                        else /* enter */
                            return AP_OK;
                    }
                    /* ZXCV row */
                    else if (cursor_row == zxcv_row) {
                        if (cursor_col == 0) {
                            shift = !shift;
                        } else {
                            const char **r = shift ? ap__kb5_row3_upper : ap__kb5_row3_lower;
                            int rn = ap__kb5_count(r);
                            if (cursor_col - 1 < rn)
                                ap__kb_insert(result, &text_cursor, r[cursor_col - 1]);
                            /* else: symbol area — no-op in URL mode */
                        }
                    }
                    break;
                }

                case AP_BTN_B:
                    ap__kb_backspace(result, &text_cursor);
                    break;
                case AP_BTN_X:
                    /* Toggle symbol alternates for shortcuts (not space in URL mode) */
                    sym_alt = !sym_alt;
                    break;
                case AP_BTN_Y:
                    return AP_CANCELLED;
                case AP_BTN_SELECT:
                    shift = !shift;
                    break;
                case AP_BTN_START:
                    return AP_OK;
                case AP_BTN_L1:
                    if (text_cursor > 0) text_cursor--;
                    break;
                case AP_BTN_R1:
                    if (text_cursor < (int)strlen(result->text)) text_cursor++;
                    break;
                case AP_BTN_MENU:
                    ap_show_help_overlay(ap__kb_help_url);
                    break;
                default: break;
            }
        }

        /* ── Render ── */
        ap_draw_background();

        /* Input field */
        ap_draw_rounded_rect(input_x, input_y, input_w, input_h, AP_S(8), theme->highlight);
        {
            int ty = input_y + (input_h - TTF_FontHeight(text_font)) / 2;
            int tx = input_x + AP_S(16);
            if (result->text[0])
                ap_draw_text_clipped(text_font, result->text, tx, ty, theme->highlighted_text, input_w - AP_S(32));
            if (caret_visible) {
                char saved = result->text[text_cursor]; result->text[text_cursor] = '\0';
                int cx = tx + ap_measure_text(text_font, result->text);
                result->text[text_cursor] = saved;
                ap_draw_rect(cx, ty, AP_S(2), TTF_FontHeight(text_font), theme->highlighted_text);
            }
        }

        ap_color key_bg_normal = theme->accent;
        ap_color key_bg_sel    = theme->highlight;
        ap_color key_fg_normal = theme->hint;
        ap_color key_fg_sel    = theme->highlighted_text;

        #define AP__KB_DRAW_KEY2(x, y, w, h, label, is_sel, fnt) do { \
            ap_color _bg = (is_sel) ? key_bg_sel : key_bg_normal; \
            ap_color _fg = (is_sel) ? key_fg_sel : key_fg_normal; \
            ap_draw_rounded_rect((x), (y), (w), (h), key_r, _bg); \
            if (label) { \
                int _tw = ap_measure_text((fnt), (label)); \
                int _th = TTF_FontHeight((fnt)); \
                ap_draw_text((fnt), (label), (x)+((w)-_tw)/2, (y)+((h)-_th)/2, _fg); \
            } \
        } while(0)

        int ry = grid_y;

        /* Shortcut rows */
        for (int sr = 0; sr < shortcut_rows; sr++) {
            int first = sr * shortcuts_per_row;
            int cnt = 0;
            for (int i = first; i < shortcut_count && i < first + shortcuts_per_row; i++) cnt++;

            /* Each shortcut uses 2 key widths */
            int short_w = key_w * 2 + key_spacing;
            bool has_backspace = (sr == shortcut_rows - 1);
            int bksp_w = has_backspace ? (key_w * 2 + key_spacing) : 0;
            int row_w = cnt * (short_w + key_spacing) - key_spacing + (has_backspace ? key_spacing + bksp_w : 0);
            int cx = (screen_w - row_w) / 2;

            for (int i = 0; i < cnt; i++) {
                int si = first + i;
                bool sel = (cursor_row == sr && cursor_col == i);
                AP__KB_DRAW_KEY2(cx, ry, short_w, key_h, active_shortcuts[si], sel, shortcut_font ? shortcut_font : key_font);
                cx += short_w + key_spacing;
            }
            if (has_backspace) {
                bool sel = (cursor_row == sr && cursor_col == cnt);
                AP__KB_DRAW_KEY2(cx, ry, bksp_w, key_h, "\xe2\x86\x90", sel, special_font ? special_font : key_font); /* ← */
            }
            ry += key_h + key_spacing;
        }

        /* URL chars row */
        {
            int ucn = ap__kb5_count((const char **)ap__url_chars);
            int row_w = ucn * (key_w + key_spacing) - key_spacing;
            int cx = (screen_w - row_w) / 2;
            for (int i = 0; i < ucn; i++) {
                bool sel = (cursor_row == url_chars_row && cursor_col == i);
                AP__KB_DRAW_KEY2(cx, ry, key_w, key_h, ap__url_chars[i], sel, key_font);
                cx += key_w + key_spacing;
            }
            ry += key_h + key_spacing;
        }

        /* QWERTY row */
        {
            const char **r = shift ? ap__kb5_row1_upper : ap__kb5_row1_lower;
            int rn = ap__kb5_count(r);
            int row_w = rn * (key_w + key_spacing) - key_spacing;
            int cx = (screen_w - row_w) / 2;
            for (int i = 0; i < rn; i++) {
                bool sel = (cursor_row == qwerty_row && cursor_col == i);
                AP__KB_DRAW_KEY2(cx, ry, key_w, key_h, r[i], sel, key_font);
                cx += key_w + key_spacing;
            }
            ry += key_h + key_spacing;
        }

        /* ASDF + enter row */
        {
            const char **r = shift ? ap__kb5_row2_upper : ap__kb5_row2_lower;
            int rn = ap__kb5_count(r);
            int enter_w = key_w * 3 / 2;
            int row_w = rn * (key_w + key_spacing) + enter_w;
            int cx = (screen_w - row_w) / 2;
            for (int i = 0; i < rn; i++) {
                bool sel = (cursor_row == asdf_row && cursor_col == i);
                AP__KB_DRAW_KEY2(cx, ry, key_w, key_h, r[i], sel, key_font);
                cx += key_w + key_spacing;
            }
            {
                bool sel = (cursor_row == asdf_row && cursor_col == rn);
                AP__KB_DRAW_KEY2(cx, ry, enter_w, key_h, "\xe2\x86\xb5", sel, special_font ? special_font : key_font); /* ↵ */
            }
            ry += key_h + key_spacing;
        }

        /* ZXCV row: shift + chars + (no symbol toggle in URL mode — use the space) */
        {
            const char **r = shift ? ap__kb5_row3_upper : ap__kb5_row3_lower;
            int rn = ap__kb5_count(r);
            int shift_w = key_w * 2 + key_spacing;
            int row_w = shift_w + key_spacing + rn * (key_w + key_spacing) - key_spacing;
            int cx = (screen_w - row_w) / 2;
            {
                bool sel = (cursor_row == zxcv_row && cursor_col == 0);
                AP__KB_DRAW_KEY2(cx, ry, shift_w, key_h, "\xe2\x87\xa7", sel, special_font ? special_font : key_font); /* ⇧ */
                cx += shift_w + key_spacing;
            }
            for (int i = 0; i < rn; i++) {
                bool sel = (cursor_row == zxcv_row && cursor_col == i + 1);
                AP__KB_DRAW_KEY2(cx, ry, key_w, key_h, r[i], sel, key_font);
                cx += key_w + key_spacing;
            }
        }

        #undef AP__KB_DRAW_KEY2

        /* Footer: always show Help hint */
        {
            ap_footer_item kb_footer[] = {{AP_BTN_MENU, "HELP", false}};
            ap_draw_footer(kb_footer, 1);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

    return AP_CANCELLED;
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
    pthread_mutex_t mutex;
} ap__process_thread_data;

static void *ap__process_worker(void *arg) {
    ap__process_thread_data *d = (ap__process_thread_data *)arg;
    int worker_result = d->fn(d->userdata);
    pthread_mutex_lock(&d->mutex);
    d->result = worker_result;
    d->done = true;
    pthread_mutex_unlock(&d->mutex);
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
    if (pthread_mutex_init(&thread_data.mutex, NULL) != 0) {
        return AP_ERROR;
    }

    pthread_t worker;
    if (pthread_create(&worker, NULL, ap__process_worker, &thread_data) != 0) {
        pthread_mutex_destroy(&thread_data.mutex);
        return AP_ERROR;
    }

    int bar_w = AP_S(400);
    int bar_h = AP_S(16);
    int bar_x = (screen_w - bar_w) / 2;

    bool running = true;
    while (running) {
        /* Check if worker is done */
        bool worker_done = false;
        pthread_mutex_lock(&thread_data.mutex);
        worker_done = thread_data.done;
        pthread_mutex_unlock(&thread_data.mutex);
        if (worker_done) {
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
    int final_result = 0;
    pthread_mutex_lock(&thread_data.mutex);
    final_result = thread_data.result;
    pthread_mutex_unlock(&thread_data.mutex);
    pthread_mutex_destroy(&thread_data.mutex);

    return final_result;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DETAIL SCREEN Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

int ap_detail_screen(ap_detail_opts *opts, ap_detail_result *result) {
    if (!opts || !result) return AP_ERROR;

    memset(result, 0, sizeof(*result));

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();

    TTF_Font *title_font   = ap_get_font(AP_FONT_SMALL);
    TTF_Font *section_font = ap_get_font(AP_FONT_SMALL);
    TTF_Font *body_font    = ap_get_font(AP_FONT_TINY);
    TTF_Font *key_font     = ap_get_font(AP_FONT_TINY);
    if (!title_font || !body_font) return AP_ERROR;

    int margin = AP_S(20);
    int section_gap = AP_S(24);
    int line_h = TTF_FontLineSkip(body_font);

    SDL_Texture **section_images = NULL;
    int *section_image_w = NULL;
    int *section_image_h = NULL;
    if (opts->section_count > 0) {
        size_t section_count = (size_t)opts->section_count;
        section_images = (SDL_Texture **)calloc(section_count, sizeof(*section_images));
        section_image_w = (int *)calloc(section_count, sizeof(*section_image_w));
        section_image_h = (int *)calloc(section_count, sizeof(*section_image_h));
        if (!section_images || !section_image_w || !section_image_h) {
            free(section_images);
            free(section_image_w);
            free(section_image_h);
            return AP_ERROR;
        }

        for (int s = 0; s < opts->section_count; s++) {
            ap_detail_section *sec = &opts->sections[s];
            if (sec->type != AP_SECTION_IMAGE) continue;
            if (!sec->image_path || !sec->image_path[0]) continue;

            section_images[s] = ap_load_image(sec->image_path);
            if (section_images[s]) {
                section_image_w[s] = sec->image_w > 0 ? sec->image_w : AP_S(300);
                section_image_h[s] = sec->image_h > 0 ? sec->image_h : AP_S(200);
            }
        }
    }

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
                if (section_images && section_images[s]) {
                    total_content_h += section_image_h[s];
                }
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
                    SDL_Texture *img = (section_images) ? section_images[s] : NULL;
                    if (img) {
                        int iw = section_image_w[s];
                        int ih = section_image_h[s];
                        ap_draw_image(img, (screen_w - iw) / 2, draw_y, iw, ih);
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

    int rc = (result->action == AP_DETAIL_BACK) ? AP_CANCELLED : AP_OK;
    if (section_images) {
        for (int s = 0; s < opts->section_count; s++) {
            if (section_images[s]) SDL_DestroyTexture(section_images[s]);
        }
    }
    free(section_images);
    free(section_image_w);
    free(section_image_h);

    return rc;
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

        /* Footer */
        ap_footer_item picker_footer[] = {
            {AP_BTN_B, "BACK", false},
            {AP_BTN_A, "SELECT", true},
        };
        ap_draw_footer(picker_footer, 2);

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

/* ═══════════════════════════════════════════════════════════════════════════
 * DOWNLOAD MANAGER Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

#ifdef AP_ENABLE_CURL
#include <curl/curl.h>

/* Worker thread context */
typedef struct {
    ap_download      *downloads;
    int               count;
    ap_download_opts *opts;
    int               next_index;     /* Next download to start */
    int               active;         /* Currently running downloads */
    int               completed;      /* Finished (success + fail) */
    bool              cancel;         /* Set by UI to abort */
    pthread_mutex_t   mutex;
    pthread_t         threads[16];    /* Joinable worker thread IDs */
    int               thread_count;
} ap__dl_context;

/* Per-download thread data */
typedef struct {
    ap_download    *dl;
    ap__dl_context *ctx;
} ap__dl_thread_data;

typedef struct {
    ap_download_status status;
    float              progress;
    double             speed_bps;
    int                http_code;
    char               error[256];
} ap__dl_snapshot;

/* libcurl progress callback */
static int ap__dl_progress_cb(void *clientp,
                               double dltotal, double dlnow,
                               double ultotal, double ulnow) {
    (void)ultotal; (void)ulnow;
    ap__dl_thread_data *td = (ap__dl_thread_data *)clientp;

    /* Check cancellation */
    pthread_mutex_lock(&td->ctx->mutex);
    bool cancel = td->ctx->cancel;
    if (!cancel) {
        if (dltotal > 0.0) {
            td->dl->progress = (float)(dlnow / dltotal);
        }
        td->dl->speed_bps = dlnow; /* Will be divided by elapsed time later */
    }
    pthread_mutex_unlock(&td->ctx->mutex);
    if (cancel) return 1; /* Abort transfer */
    return 0;
}

/* libcurl write callback — write to file */
static size_t ap__dl_write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    FILE *fp = (FILE *)userdata;
    return fwrite(ptr, size, nmemb, fp);
}

/* Worker thread function */
static void *ap__dl_worker(void *arg) {
    ap__dl_thread_data *td = (ap__dl_thread_data *)arg;
    ap_download *dl = td->dl;

    pthread_mutex_lock(&td->ctx->mutex);
    dl->status = AP_DL_DOWNLOADING;
    dl->progress = 0.0f;
    dl->speed_bps = 0.0;
    dl->http_code = 0;
    dl->error[0] = '\0';
    pthread_mutex_unlock(&td->ctx->mutex);

    FILE *fp = fopen(dl->dest_path, "wb");
    if (!fp) {
        char err[256];
        snprintf(err, sizeof(err), "Cannot open: %s", dl->dest_path);
        pthread_mutex_lock(&td->ctx->mutex);
        snprintf(dl->error, sizeof(dl->error), "%s", err);
        dl->status = AP_DL_FAILED;
        pthread_mutex_unlock(&td->ctx->mutex);
        ap_log("download: %s failed: %s", dl->label ? dl->label : dl->url, err);
        goto done;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        pthread_mutex_lock(&td->ctx->mutex);
        snprintf(dl->error, sizeof(dl->error), "curl_easy_init failed");
        dl->status = AP_DL_FAILED;
        pthread_mutex_unlock(&td->ctx->mutex);
        ap_log("download: %s failed: curl_easy_init failed", dl->label ? dl->label : dl->url);
        fclose(fp);
        goto done;
    }

    curl_easy_setopt(curl, CURLOPT_URL, dl->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ap__dl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ap__dl_progress_cb);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, td);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 100L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);

    if (td->ctx->opts && td->ctx->opts->skip_ssl_verify) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    } else {
        const char *ca = getenv("SSL_CERT_FILE");
        if (ca) curl_easy_setopt(curl, CURLOPT_CAINFO, ca);
    }

    /* Custom headers */
    struct curl_slist *hdrs = NULL;
    if (td->ctx->opts && td->ctx->opts->headers) {
        for (int i = 0; i < td->ctx->opts->header_count; i++) {
            hdrs = curl_slist_append(hdrs, td->ctx->opts->headers[i]);
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
    }

    CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    /* Get final speed */
    double speed = 0;
    curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed);
    pthread_mutex_lock(&td->ctx->mutex);
    dl->http_code = (int)http_code;
    dl->speed_bps = speed;
    pthread_mutex_unlock(&td->ctx->mutex);

    if (hdrs) curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    fclose(fp);

    if (res == CURLE_ABORTED_BY_CALLBACK) {
        pthread_mutex_lock(&td->ctx->mutex);
        dl->status = AP_DL_FAILED;
        snprintf(dl->error, sizeof(dl->error), "Cancelled");
        pthread_mutex_unlock(&td->ctx->mutex);
        remove(dl->dest_path);
        ap_log("download: %s cancelled", dl->label ? dl->label : dl->url);
    } else if (res != CURLE_OK) {
        const char *curl_err = curl_easy_strerror(res);
        pthread_mutex_lock(&td->ctx->mutex);
        dl->status = AP_DL_FAILED;
        snprintf(dl->error, sizeof(dl->error), "%s", curl_err);
        pthread_mutex_unlock(&td->ctx->mutex);
        remove(dl->dest_path);
        ap_log("download: %s failed: %s", dl->label ? dl->label : dl->url, curl_err);
    } else if (http_code >= 400) {
        char err[256];
        snprintf(err, sizeof(err), "HTTP %d", (int)http_code);
        pthread_mutex_lock(&td->ctx->mutex);
        dl->status = AP_DL_FAILED;
        snprintf(dl->error, sizeof(dl->error), "%s", err);
        pthread_mutex_unlock(&td->ctx->mutex);
        remove(dl->dest_path);
        ap_log("download: %s failed: %s", dl->label ? dl->label : dl->url, err);
    } else {
        pthread_mutex_lock(&td->ctx->mutex);
        dl->status = AP_DL_COMPLETE;
        dl->progress = 1.0f;
        dl->error[0] = '\0';
        pthread_mutex_unlock(&td->ctx->mutex);
    }

done:
    pthread_mutex_lock(&td->ctx->mutex);
    td->ctx->active--;
    td->ctx->completed++;
    pthread_mutex_unlock(&td->ctx->mutex);

    free(td);
    return NULL;
}

/* Start the next pending download if capacity allows */
static void ap__dl_dispatch(ap__dl_context *ctx) {
    pthread_mutex_lock(&ctx->mutex);
    int max_c = (ctx->opts && ctx->opts->max_concurrent > 0)
                 ? ctx->opts->max_concurrent : 3;

    while (ctx->active < max_c && ctx->next_index < ctx->count) {
        ap_download *dl = &ctx->downloads[ctx->next_index];
        ctx->next_index++;

        if (dl->status != AP_DL_PENDING) continue;

        ap__dl_thread_data *td = (ap__dl_thread_data *)calloc(1, sizeof(*td));
        if (!td) break;
        td->dl = dl;
        td->ctx = ctx;

        ctx->active++;

        pthread_t thr;
        if (pthread_create(&thr, NULL, ap__dl_worker, td) != 0) {
            free(td);
            ctx->active--;
            dl->status = AP_DL_FAILED;
            snprintf(dl->error, sizeof(dl->error), "Thread creation failed");
            ctx->completed++;
        } else if (ctx->thread_count < 16) {
            ctx->threads[ctx->thread_count++] = thr;
        } else {
            pthread_detach(thr); /* fallback: more threads than slots */
        }
    }
    pthread_mutex_unlock(&ctx->mutex);
}

/* Format bytes per second as human-readable speed */
static void ap__dl_format_speed(double bps, char *buf, int bufsize) {
    if (bps >= 1048576.0)
        snprintf(buf, bufsize, "%.1f MB/s", bps / 1048576.0);
    else if (bps >= 1024.0)
        snprintf(buf, bufsize, "%.0f KB/s", bps / 1024.0);
    else
        snprintf(buf, bufsize, "%.0f B/s", bps);
}

int ap_download_manager(ap_download *downloads, int count,
                        ap_download_opts *opts, ap_download_result *result) {
    if (!downloads || count <= 0 || !result) return AP_ERROR;

    memset(result, 0, sizeof(*result));
    result->total = count;

    /* Initialize all downloads to pending */
    for (int i = 0; i < count; i++) {
        downloads[i].status = AP_DL_PENDING;
        downloads[i].progress = 0.0f;
        downloads[i].speed_bps = 0;
        downloads[i].http_code = 0;
        downloads[i].error[0] = '\0';
    }

    /* Init curl globally (safe to call multiple times) */
    curl_global_init(CURL_GLOBAL_DEFAULT);

    ap__dl_context ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.downloads = downloads;
    ctx.count = count;
    ctx.opts = opts;
    pthread_mutex_init(&ctx.mutex, NULL);

    ap__dl_snapshot *snapshots = (ap__dl_snapshot *)calloc((size_t)count, sizeof(*snapshots));
    if (!snapshots) {
        pthread_mutex_destroy(&ctx.mutex);
        curl_global_cleanup();
        return AP_ERROR;
    }

    ap_theme *theme = ap_get_theme();
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();
    TTF_Font *label_font = ap_get_font(AP_FONT_MEDIUM);
    TTF_Font *speed_font = ap_get_font(AP_FONT_SMALL);

    bool show_speed = true;
    bool all_done = false;
    int scroll = 0;

    /* Progress bar sizing — 3/4 of screen width, max 900px */
    int bar_w = screen_w * 3 / 4;
    if (bar_w > AP_S(900)) bar_w = AP_S(900);
    int bar_h = AP_S(16);
    int bar_r = AP_S(8);
    int item_gap = AP_S(12);
    int label_h = label_font ? TTF_FontHeight(label_font) : AP_S(30);
    int speed_h = speed_font ? TTF_FontHeight(speed_font) : AP_S(20);
    int item_h = label_h + AP_S(4) + bar_h + (show_speed ? AP_S(2) + speed_h : 0);

    /* Visible area */
    int title_zone = AP_S(85);
    int footer_zone = ap_get_footer_height();
    int content_y = title_zone;
    int content_h = screen_h - title_zone - footer_zone;
    int max_visible = content_h / (item_h + item_gap);
    if (max_visible < 1) max_visible = 1;

    while (!all_done) {
        /* Dispatch pending downloads */
        ap__dl_dispatch(&ctx);

        /* Check overall status */
        int active_count = 0;
        pthread_mutex_lock(&ctx.mutex);
        int done_count = ctx.completed;
        bool cancelled = ctx.cancel;
        active_count = ctx.active;
        for (int i = 0; i < count; i++) {
            snapshots[i].status = downloads[i].status;
            snapshots[i].progress = downloads[i].progress;
            snapshots[i].speed_bps = downloads[i].speed_bps;
            snapshots[i].http_code = downloads[i].http_code;
            strncpy(snapshots[i].error, downloads[i].error, sizeof(snapshots[i].error) - 1);
            snapshots[i].error[sizeof(snapshots[i].error) - 1] = '\0';
        }
        pthread_mutex_unlock(&ctx.mutex);

        int comp_ok = 0, comp_fail = 0;
        for (int i = 0; i < count; i++) {
            if (snapshots[i].status == AP_DL_COMPLETE) comp_ok++;
            if (snapshots[i].status == AP_DL_FAILED) comp_fail++;
        }
        result->completed = comp_ok;
        result->failed = comp_fail;

        if (done_count >= count) all_done = true;
        if (cancelled && active_count <= 0) { all_done = true; result->cancelled = true; }

        /* Input */
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_Y:
                    if (!all_done) {
                        pthread_mutex_lock(&ctx.mutex);
                        ctx.cancel = true;
                        pthread_mutex_unlock(&ctx.mutex);
                    }
                    break;
                case AP_BTN_X:
                    show_speed = !show_speed;
                    item_h = label_h + AP_S(4) + bar_h + (show_speed ? AP_S(2) + speed_h : 0);
                    max_visible = content_h / (item_h + item_gap);
                    if (max_visible < 1) max_visible = 1;
                    break;
                case AP_BTN_A:
                    if (all_done) goto dm_exit;
                    break;
                case AP_BTN_UP:
                    if (scroll > 0) scroll--;
                    break;
                case AP_BTN_DOWN:
                    if (scroll < count - max_visible) scroll++;
                    break;
                default: break;
            }
        }

        /* Auto-scroll to show active downloads */
        if (!all_done) {
            for (int i = count - 1; i >= 0; i--) {
                if (snapshots[i].status == AP_DL_DOWNLOADING) {
                    if (i >= scroll + max_visible) scroll = i - max_visible + 1;
                    if (i < scroll) scroll = i;
                    break;
                }
            }
        }

        /* ── Render ── */
        ap_draw_background();

        /* Title: "Downloading..." or completion summary */
        {
            char title_buf[128];
            if (all_done) {
                snprintf(title_buf, sizeof(title_buf), "Downloads Complete (%d/%d)", comp_ok, count);
            } else {
                snprintf(title_buf, sizeof(title_buf), "Downloading... %d/%d", done_count, count);
            }
            ap__draw_title(title_buf);
        }

        /* Download items */
        int bar_x = (screen_w - bar_w) / 2;
        for (int vi = 0; vi < max_visible && vi + scroll < count; vi++) {
            int i = vi + scroll;
            ap_download *dl = &downloads[i];
            ap__dl_snapshot *dls = &snapshots[i];
            int iy = content_y + vi * (item_h + item_gap);

            /* Label */
            const char *lbl = dl->label ? dl->label : dl->url;
            ap_color lbl_color = theme->text;
            if (dls->status == AP_DL_COMPLETE) lbl_color = (ap_color){100, 255, 100, 255};
            if (dls->status == AP_DL_FAILED) lbl_color = (ap_color){255, 100, 100, 255};

            if (label_font) {
                ap_draw_text_clipped(label_font, lbl, bar_x, iy, lbl_color, bar_w);
            }

            /* Progress bar */
            int bar_y = iy + label_h + AP_S(4);
            ap_color bar_bg = {40, 40, 50, 255};
            ap_draw_rounded_rect(bar_x, bar_y, bar_w, bar_h, bar_r, bar_bg);

            if (dls->progress > 0.001f) {
                int fill_w = (int)(dls->progress * bar_w);
                if (fill_w < bar_h) fill_w = bar_h; /* Minimum for rounding */
                ap_color fill_c;
                if (dls->status == AP_DL_COMPLETE) fill_c = (ap_color){80, 200, 80, 255};
                else if (dls->status == AP_DL_FAILED) fill_c = (ap_color){200, 80, 80, 255};
                else fill_c = theme->highlight;
                ap_draw_rounded_rect(bar_x, bar_y, fill_w, bar_h, bar_r, fill_c);
            }

            /* Speed / status text */
            if (show_speed && speed_font) {
                int sy = bar_y + bar_h + AP_S(2);
                char speed_buf[64];
                if (dls->status == AP_DL_DOWNLOADING) {
                    ap__dl_format_speed(dls->speed_bps, speed_buf, sizeof(speed_buf));
                    char pct_buf[80];
                    snprintf(pct_buf, sizeof(pct_buf), "%.0f%% — %s", dls->progress * 100.0f, speed_buf);
                    ap_draw_text(speed_font, pct_buf, bar_x, sy, theme->hint);
                } else if (dls->status == AP_DL_COMPLETE) {
                    ap_draw_text(speed_font, "Complete", bar_x, sy, (ap_color){100,255,100,255});
                } else if (dls->status == AP_DL_FAILED) {
                    ap_draw_text_clipped(speed_font, dls->error, bar_x, sy,
                        (ap_color){255,100,100,255}, bar_w);
                } else {
                    ap_draw_text(speed_font, "Pending...", bar_x, sy, theme->hint);
                }
            }
        }

        /* Scrollbar */
        if (count > max_visible) {
            ap_draw_scrollbar(bar_x + bar_w + AP_S(12), content_y,
                content_h, content_h, count * (item_h + item_gap), scroll * (item_h + item_gap));
        }

        /* Footer */
        if (all_done) {
            ap_footer_item dm_footer[] = {{AP_BTN_A, "CLOSE", true}};
            ap_draw_footer(dm_footer, 1);
        } else {
            ap_footer_item dm_footer[] = {
                {AP_BTN_Y, "CANCEL", false},
                {AP_BTN_X, show_speed ? "HIDE SPEED" : "SHOW SPEED", false},
            };
            ap_draw_footer(dm_footer, 2);
        }

        ap_present();
        SDL_Delay(AP__FRAME_DELAY);
    }

dm_exit:
    for (int i = 0; i < ctx.thread_count; i++)
        pthread_join(ctx.threads[i], NULL);
    pthread_mutex_destroy(&ctx.mutex);
    curl_global_cleanup();
    free(snapshots);

    if (result->cancelled) return AP_CANCELLED;
    if (result->failed > 0 && result->completed == 0) return AP_ERROR;
    return AP_OK;
}

#endif /* AP_ENABLE_CURL */

#endif /* AP_WIDGETS_IMPLEMENTATION */
#endif /* APOSTROPHE_WIDGETS_H */
