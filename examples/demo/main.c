/*
 * Apostrophe Widget Demo
 *
 * Comprehensive example that showcases every available widget. The user
 * navigates a top-level menu which launches individual widget demonstrations.
 */

#define AP_IMPLEMENTATION
#include "apostrophe.h"
#define AP_WIDGETS_IMPLEMENTATION
#include "apostrophe_widgets.h"

/* Forward declarations */
static void demo_detail(void);
static void demo_detail_custom_fonts(void);
static void demo_image_list(void);
static void demo_process_advanced(void);
static void demo_drawing_primitives(void);
static void demo_screen_fade(void);
static void demo_help_overlay_screen(void);
static void demo_core_api_lab(void);
static void demo_input_theme(void);
static void demo_background_preview(void);

static void demo_show_message(const char *message) {
    ap_footer_item ok_foot[] = {
        { .button = AP_BTN_A, .label = "OK", .is_confirm = true },
    };
    ap_message_opts opts = {
        .message      = message,
        .footer       = ok_foot,
        .footer_count = 1,
    };
    ap_confirm_result result;
    ap_confirmation(&opts, &result);
}

static const char *demo_option_display(ap_options_item *item, const char *fallback) {
    if (!item || !item->options || item->option_count <= 0) return fallback;
    if (item->selected_option < 0 || item->selected_option >= item->option_count) return fallback;
    if (item->options[item->selected_option].label) return item->options[item->selected_option].label;
    if (item->options[item->selected_option].value) return item->options[item->selected_option].value;
    return fallback;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: List (basic)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_list(void) {
    ap_list_item items[] = {
        { .label = "Alpha",     .metadata = "/path/alpha" },
        { .label = "Bravo",     .metadata = "/path/bravo" },
        { .label = "Charlie",   .metadata = "/path/charlie" },
        { .label = "Delta",     .metadata = "/path/delta" },
        { .label = "Echo",      .metadata = "/path/echo" },
        { .label = "Foxtrot",   .metadata = "/path/foxtrot" },
        { .label = "Golf",      .metadata = "/path/golf" },
        { .label = "Hotel",     .metadata = "/path/hotel" },
        { .label = "India",     .metadata = "/path/india" },
        { .label = "Juliet",    .metadata = "/path/juliet" },
        { .label = "Kilo",      .metadata = "/path/kilo" },
        { .label = "Lima",      .metadata = "/path/lima" },
        { .label = "This entry has a very long label that should trigger horizontal text scrolling when selected", .metadata = "/path/long-label" },
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "SELECT", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("NATO Alphabet", items, count);
    opts.footer       = footer;
    opts.footer_count = 2;
    opts.help_text    = "Navigate with D-Pad.\nPress A to select an item.\nPress B to go back.";

    ap_list_result result;
    int rc = ap_list(&opts, &result);

    if (rc == AP_OK && result.selected_index >= 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Selected: %s\nPath: %s",
                 items[result.selected_index].label,
                 items[result.selected_index].metadata);
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: List (hidden scrollbar)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_list_no_scrollbar(void) {
    ap_list_item items[] = {
        { .label = "Mercury"  },
        { .label = "Venus"    },
        { .label = "Earth"    },
        { .label = "Mars"     },
        { .label = "Jupiter"  },
        { .label = "Saturn"   },
        { .label = "Uranus"   },
        { .label = "Neptune"  },
        { .label = "Pluto"    },
        { .label = "Ceres"    },
        { .label = "Eris"     },
        { .label = "Haumea"   },
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "SELECT", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Planets (No Scrollbar)", items, count);
    opts.footer         = footer;
    opts.footer_count   = 2;
    opts.hide_scrollbar = true;

    ap_list_result result;
    int rc = ap_list(&opts, &result);

    if (rc == AP_OK && result.selected_index >= 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Selected: %s", items[result.selected_index].label);
        demo_show_message(msg);
    }
}

typedef struct {
    const char    *preview_path;
    uint32_t       preview_until_ms;
    ap_footer_item *footer;
    int             footer_index;
} demo_live_footer_context;

static bool demo_live_footer_preview_active(demo_live_footer_context *ctx, uint32_t now) {
    if (!ctx || !ctx->preview_path) return false;
    if (now >= ctx->preview_until_ms) {
        ctx->preview_path = NULL;
        ctx->preview_until_ms = 0;
        return false;
    }
    return true;
}

static const char *demo_live_footer_label(demo_live_footer_context *ctx,
                                          const char *selected_path) {
    uint32_t now = SDL_GetTicks();
    bool preview_active = demo_live_footer_preview_active(ctx, now);

    if (!selected_path || !selected_path[0]) return "PREVIEW";

    if (preview_active) {
        if (strcmp(ctx->preview_path, selected_path) == 0)
            return "STOP PREVIEW";
        return "SWITCH PREVIEW";
    }

    return "PREVIEW";
}

static void demo_live_footer_update(ap_list_opts *opts, int cursor, void *userdata) {
    demo_live_footer_context *ctx = (demo_live_footer_context *)userdata;
    const char *selected_path = NULL;
    uint32_t now = SDL_GetTicks();

    if (!ctx || !ctx->footer || ctx->footer_index < 0) return;

    if (demo_live_footer_preview_active(ctx, now)) {
        ap_request_frame_in(100);
    }

    if (cursor >= 0 && cursor < opts->item_count) {
        selected_path = opts->items[cursor].metadata;
    }

    ctx->footer[ctx->footer_index].label =
        demo_live_footer_label(ctx, selected_path);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: List (live footer labels)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_live_footer(void) {
    ap_list_item items[] = {
        { .label = "Acoustic Demo", .metadata = "/preview/acoustic.ogg" },
        { .label = "City Lights",   .metadata = "/preview/city-lights.ogg" },
        { .label = "Night Drive",   .metadata = "/preview/night-drive.ogg" },
        { .label = "Sunrise Theme", .metadata = "/preview/sunrise-theme.ogg" },
    };
    int count = sizeof(items) / sizeof(items[0]);
    int last_index = 0;
    int last_visible = 0;
    demo_live_footer_context preview_ctx = {
        .footer_index = 1,
    };

    for (;;) {
        ap_footer_item footer[] = {
            { .button = AP_BTN_B, .label = "BACK" },
            { .button = AP_BTN_Y, .label = "PREVIEW" },
            { .button = AP_BTN_A, .label = "SELECT", .is_confirm = true },
        };

        preview_ctx.footer = footer;

        ap_list_opts opts = ap_list_default_opts("Live Footer", items, count);
        opts.footer = footer;
        opts.footer_count = 3;
        opts.secondary_action_button = AP_BTN_Y;
        opts.initial_index = last_index;
        opts.visible_start_index = last_visible;
        opts.help_text = "Press Y to simulate a preview.\n"
                         "The Y footer label updates while you move between rows.\n"
                         "Preview also expires on its own after a short delay.";
        opts.footer_update = demo_live_footer_update;
        opts.footer_update_userdata = &preview_ctx;

        ap_list_result result;
        int rc = ap_list(&opts, &result);

        if (result.selected_index >= 0)
            last_index = result.selected_index;
        last_visible = result.visible_start_index;

        if (rc == AP_CANCELLED) break;
        if (rc != AP_OK || result.selected_index < 0) continue;

        if (result.action == AP_ACTION_SECONDARY_TRIGGERED) {
            const char *selected_path = items[result.selected_index].metadata;
            uint32_t now = SDL_GetTicks();

            if (demo_live_footer_preview_active(&preview_ctx, now) &&
                strcmp(preview_ctx.preview_path, selected_path) == 0) {
                preview_ctx.preview_path = NULL;
                preview_ctx.preview_until_ms = 0;
            } else {
                preview_ctx.preview_path = selected_path;
                preview_ctx.preview_until_ms = now + 1500;
            }
            continue;
        }

        if (result.action == AP_ACTION_SELECTED) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Selected: %s", items[result.selected_index].label);
            demo_show_message(msg);
            break;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Multi-select list
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_multi_select(void) {
    ap_list_item items[] = {
        { .label = "Apples",    .selected = false },
        { .label = "Bananas",   .selected = true  },
        { .label = "Cherries",  .selected = false },
        { .label = "Dates",     .selected = false },
        { .label = "Elderberry",.selected = false },
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "TOGGLE" },
        { .button = AP_BTN_START, .label = "DONE", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Pick Fruits", items, count);
    opts.multi_select   = true;
    opts.footer         = footer;
    opts.footer_count   = 3;
    opts.confirm_button = AP_BTN_START;

    ap_list_result result;
    int rc = ap_list(&opts, &result);

    if (rc == AP_OK && result.action == AP_ACTION_CONFIRMED) {
        char msg[512];
        int off = 0;
        off += snprintf(msg + off, sizeof(msg) - off, "Selected:");
        bool any = false;
        for (int i = 0; i < count; i++) {
            if (items[i].selected) {
                off += snprintf(msg + off, sizeof(msg) - off, "\n- %s", items[i].label);
                any = true;
            }
        }
        if (!any) snprintf(msg, sizeof(msg), "No items selected.");
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Reorderable list
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_reorder(void) {
    ap_list_item items[] = {
        { .label = "1. First"  },
        { .label = "2. Second" },
        { .label = "3. Third"  },
        { .label = "4. Fourth" },
        { .label = "5. Fifth"  },
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B,     .label = "BACK" },
        { .button = AP_BTN_X,     .label = "REORDER" },
        { .button = AP_BTN_START, .label = "DONE", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Reorder Items", items, count);
    opts.reorder_button = AP_BTN_X;
    opts.confirm_button = AP_BTN_START;
    opts.footer         = footer;
    opts.footer_count   = 3;

    ap_list_result result;
    int rc = ap_list(&opts, &result);

    if (rc == AP_OK && result.action == AP_ACTION_CONFIRMED) {
        char msg[512];
        int off = snprintf(msg, sizeof(msg), "Final order:");
        for (int i = 0; i < count; i++) {
            off += snprintf(msg + off, sizeof(msg) - off, "\n%d. %s", i + 1, items[i].label);
        }
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Options list (settings-style)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_options_list(void) {
    ap_color orig_accent = ap_get_theme()->accent;
    char orig_hex[8];
    char accent_hex[8];

    snprintf(orig_hex, sizeof(orig_hex), "#%02X%02X%02X",
             orig_accent.r, orig_accent.g, orig_accent.b);
    snprintf(accent_hex, sizeof(accent_hex), "%s", orig_hex);

    /* Volume option: standard cycle */
    ap_option vol_opts[] = {
        { .label = "Off",   .value = "0"   },
        { .label = "Low",   .value = "25"  },
        { .label = "Mid",   .value = "50"  },
        { .label = "High",  .value = "75"  },
        { .label = "Max",   .value = "100" },
    };

    /* Theme option: standard cycle */
    ap_option theme_opts[] = {
        { .label = "Dark",   .value = "dark"  },
        { .label = "Light",  .value = "light" },
        { .label = "Retro",  .value = "retro" },
    };

    ap_option name_opt[] = {
        { .label = strdup("Player 1"), .value = strdup("Player 1") },
    };

    ap_option accent_opt[] = {
        { .label = strdup(accent_hex), .value = strdup(accent_hex) },
    };

    ap_option brightness_opts[] = {
        { .label = "Low",    .value = "25"  },
        { .label = "Medium", .value = "50"  },
        { .label = "High",   .value = "75"  },
        { .label = "Max",    .value = "100" },
    };

    ap_option language_opts[] = {
        { .label = "English",  .value = "en" },
        { .label = "Spanish",  .value = "es" },
        { .label = "French",   .value = "fr" },
        { .label = "German",   .value = "de" },
        { .label = "Japanese", .value = "ja" },
    };

    ap_option timeout_opts[] = {
        { .label = "30s",   .value = "30"    },
        { .label = "1m",    .value = "60"    },
        { .label = "5m",    .value = "300"   },
        { .label = "Never", .value = "0"     },
    };

    ap_option toggle_on_off[] = {
        { .label = "On",  .value = "1" },
        { .label = "Off", .value = "0" },
    };

    ap_option notif_opts[] = {
        { .label = "All",       .value = "all"       },
        { .label = "Important", .value = "important"  },
        { .label = "None",      .value = "none"       },
    };

    ap_option font_size_opts[] = {
        { .label = "Small",  .value = "small"  },
        { .label = "Medium", .value = "medium" },
        { .label = "Large",  .value = "large"  },
    };

    /* Regression coverage for long clickable values on narrow layouts. */
    ap_option manual_dir_opt[] = {
        {
            .label = "/Users/demo/Library/Application Support/Apostrophe/Manuals",
            .value = "manual-dir",
        },
    };

    ap_option upload_dest_opt[] = {
        {
            .label = "https://updates.example.invalid/api/v1/devices/trimui-smart-pro/uploads",
            .value = "upload-destination",
        },
    };

    ap_options_item settings[] = {
        {
            .label           = "Volume",
            .type            = AP_OPT_STANDARD,
            .options         = vol_opts,
            .option_count    = 5,
            .selected_option = 2,
        },
        {
            .label           = "Theme",
            .type            = AP_OPT_STANDARD,
            .options         = theme_opts,
            .option_count    = 3,
            .selected_option = 0,
        },
        {
            .label           = "Brightness",
            .type            = AP_OPT_STANDARD,
            .options         = brightness_opts,
            .option_count    = 4,
            .selected_option = 1,
        },
        {
            .label           = "Language",
            .type            = AP_OPT_STANDARD,
            .options         = language_opts,
            .option_count    = 5,
            .selected_option = 0,
        },
        {
            .label           = "Screen Timeout",
            .type            = AP_OPT_STANDARD,
            .options         = timeout_opts,
            .option_count    = 4,
            .selected_option = 1,
        },
        {
            .label           = "WiFi",
            .type            = AP_OPT_STANDARD,
            .options         = toggle_on_off,
            .option_count    = 2,
            .selected_option = 0,
        },
        {
            .label           = "Bluetooth",
            .type            = AP_OPT_STANDARD,
            .options         = toggle_on_off,
            .option_count    = 2,
            .selected_option = 1,
        },
        {
            .label           = "Notifications",
            .type            = AP_OPT_STANDARD,
            .options         = notif_opts,
            .option_count    = 3,
            .selected_option = 0,
        },
        {
            .label           = "Font Size",
            .type            = AP_OPT_STANDARD,
            .options         = font_size_opts,
            .option_count    = 3,
            .selected_option = 1,
        },
        {
            .label           = "Auto-Save",
            .type            = AP_OPT_STANDARD,
            .options         = toggle_on_off,
            .option_count    = 2,
            .selected_option = 0,
        },
        {
            .label           = "Name",
            .type            = AP_OPT_KEYBOARD,
            .options         = name_opt,
            .option_count    = 1,
            .selected_option = 0,
        },
        {
            .label           = "Accent Color",
            .type            = AP_OPT_COLOR_PICKER,
            .options         = accent_opt,
            .option_count    = 1,
            .selected_option = 0,
        },
        {
            .label           = "Manual download directory",
            .type            = AP_OPT_CLICKABLE,
            .options         = manual_dir_opt,
            .option_count    = 1,
            .selected_option = 0,
        },
        {
            .label           = "Automatic Achievement Screenshot Upload Destination",
            .type            = AP_OPT_CLICKABLE,
            .options         = upload_dest_opt,
            .option_count    = 1,
            .selected_option = 0,
        },
        {
            .label           = "Storage",
            .type            = AP_OPT_CLICKABLE,
            .options         = NULL,
            .option_count    = 0,
            .selected_option = 0,
        },
        {
            .label           = "About",
            .type            = AP_OPT_CLICKABLE,
            .options         = NULL,
            .option_count    = 0,
            .selected_option = 0,
        },
        {
            .label           = "Reset All",
            .type            = AP_OPT_CLICKABLE,
            .options         = NULL,
            .option_count    = 0,
            .selected_option = 0,
        },
    };
    int count = sizeof(settings) / sizeof(settings[0]);

    /* Named indices for clickable items — avoids fragile hard-coded numbers */
    enum {
        IDX_NAME         = 10,
        IDX_ACCENT       = 11,
        IDX_MANUAL_DIR   = 12,
        IDX_UPLOAD_DEST  = 13,
        IDX_STORAGE      = 14,
        IDX_ABOUT        = 15,
        IDX_RESET_ALL    = 16,
    };

    /* Snapshot default selected_option values for Reset All */
    int defaults[sizeof(settings) / sizeof(settings[0])];
    for (int i = 0; i < count; i++)
        defaults[i] = settings[i].selected_option;

    ap_footer_item footer[] = {
        { .button = AP_BTN_B,     .label = "BACK" },
        { .button = AP_BTN_A,     .label = "EDIT" },
        { .button = AP_BTN_X,     .label = "SUMMARY" },
        { .button = AP_BTN_Y,     .label = "RESET" },
        { .button = AP_BTN_START, .label = "SAVE", .is_confirm = true },
    };

    ap_options_list_opts opts = {
        .title        = "Settings",
        .items        = settings,
        .item_count   = count,
        .footer       = footer,
        .footer_count = 5,
        .action_button = AP_BTN_X,
        .secondary_action_button = AP_BTN_Y,
        .confirm_button = AP_BTN_START,
    };

    int last_cursor = 0;
    int last_visible = 0;
    while (1) {
        opts.initial_selected_index = last_cursor;
        opts.visible_start_index = last_visible;

        ap_options_list_result result;
        int rc = ap_options_list(&opts, &result);
        last_cursor = result.focused_index;
        last_visible = result.visible_start_index;

        if (rc == AP_OK && result.action == AP_ACTION_SELECTED) {
            if (result.focused_index == IDX_MANUAL_DIR) {
                demo_show_message("Manual download directory clicked.");
                continue;
            } else if (result.focused_index == IDX_UPLOAD_DEST) {
                demo_show_message("Achievement screenshot upload destination clicked.");
                continue;
            } else if (result.focused_index == IDX_STORAGE) {
                /* "Storage" clicked */
                demo_show_message("Storage: 2.4 GB used of 32 GB");
                continue;
            } else if (result.focused_index == IDX_ABOUT) {
                /* "About" clicked — show detail screen */
                demo_detail();
                continue;
            } else if (result.focused_index == IDX_RESET_ALL) {
                /* "Reset All" clicked — restore all settings to defaults */
                for (int i = 0; i < count; i++)
                    settings[i].selected_option = defaults[i];
                free((void *)name_opt[0].label); free((void *)name_opt[0].value);
                name_opt[0].label = strdup("Player 1");
                name_opt[0].value = strdup("Player 1");
                free((void *)accent_opt[0].label); free((void *)accent_opt[0].value);
                accent_opt[0].label = strdup(accent_hex);
                accent_opt[0].value = strdup(accent_hex);
                demo_show_message("All settings reset to defaults.");
                continue;
            }
        }

        if (rc == AP_OK && result.action == AP_ACTION_TRIGGERED) {
            char msg[512];
            snprintf(msg, sizeof(msg),
                     "Volume: %s\nTheme: %s\nName: %s\nAccent: %s",
                     demo_option_display(&settings[0], "Mid"),
                     demo_option_display(&settings[1], "Dark"),
                     demo_option_display(&settings[IDX_NAME], "Player 1"),
                     demo_option_display(&settings[IDX_ACCENT], orig_hex));
            demo_show_message(msg);
            continue;
        }

        if (rc == AP_OK && result.action == AP_ACTION_SECONDARY_TRIGGERED) {
            switch (result.focused_index) {
                case 0:
                    settings[0].selected_option = 2;
                    demo_show_message("Reset Volume to Mid.");
                    break;
                case 1:
                    settings[1].selected_option = 0;
                    demo_show_message("Reset Theme to Dark.");
                    break;
                case IDX_NAME:
                    free((void *)name_opt[0].label); free((void *)name_opt[0].value);
                    name_opt[0].label = strdup("Player 1");
                    name_opt[0].value = strdup("Player 1");
                    demo_show_message("Reset Name to Player 1.");
                    break;
                case IDX_ACCENT:
                    free((void *)accent_opt[0].label); free((void *)accent_opt[0].value);
                    accent_opt[0].label = strdup(orig_hex);
                    accent_opt[0].value = strdup(orig_hex);
                    demo_show_message("Reset Accent Color to the current theme color.");
                    break;
                default:
                    demo_show_message("Nothing to reset on this row.");
                    break;
            }
            continue;
        }
        break;
    }

    free((void *)name_opt[0].label); free((void *)name_opt[0].value);
    free((void *)accent_opt[0].label); free((void *)accent_opt[0].value);
    ap_set_theme_color(orig_hex);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Keyboard
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_keyboard(void) {
    static const struct {
        const char         *label;
        const char         *prompt;
        const char         *initial;
        ap_keyboard_layout  layout;
    } modes[] = {
        { "General",          "Enter your name:",  "Hello",              AP_KB_GENERAL },
        { "URL",              "Enter a URL:",      "https://",           AP_KB_URL     },
        { "Numeric",          "Enter a number:",   "42",                 AP_KB_NUMERIC },
        { "UTF-8 / Long",    "Edit text:",         "The quick brown fox jumps over the lazy dog — Price: 10€ résumé café", AP_KB_GENERAL },
        { "URL (Custom)",     "Enter a URL:",      "https://example",    AP_KB_URL     },
    };
    int mode_count = sizeof(modes) / sizeof(modes[0]);

    int last_index = 0;
    int last_visible = 0;

    while (1) {
        ap_list_item items[sizeof(modes) / sizeof(modes[0])];
        for (int i = 0; i < mode_count; i++)
            items[i] = (ap_list_item){ .label = modes[i].label };

        ap_footer_item footer[] = {
            { .button = AP_BTN_B, .label = "BACK" },
            { .button = AP_BTN_A, .label = "OPEN", .is_confirm = true },
        };

        ap_list_opts opts = ap_list_default_opts("Keyboard Mode", items, mode_count);
        opts.footer              = footer;
        opts.footer_count        = 2;
        opts.initial_index       = last_index;
        opts.visible_start_index = last_visible;

        ap_list_result lr;
        int rc = ap_list(&opts, &lr);
        last_index   = lr.selected_index >= 0 ? lr.selected_index : last_index;
        last_visible = lr.visible_start_index;

        if (rc != AP_OK || lr.selected_index < 0 || lr.selected_index >= mode_count)
            break;

        int idx = lr.selected_index;
        ap_keyboard_result result;

        if (idx == 1) {
            /* URL keyboard with default shortcuts */
            rc = ap_url_keyboard(modes[idx].initial, NULL, NULL, &result);
        } else if (idx == 4) {
            /* Custom URL keyboard with shortcut keys */
            const char *shortcuts[] = { ".io", ".dev", ".app", "https://", "http://" };
            const char *url_help =
                "Custom URL keyboard.\n\n"
                "Shortcuts: .io, .dev, .app, https://, http://\n"
                "Use MENU to reopen this help overlay.";
            ap_url_keyboard_config url_cfg = {
                .shortcut_keys  = shortcuts,
                .shortcut_count = 5,
            };
            rc = ap_url_keyboard(modes[idx].initial, url_help, &url_cfg, &result);
        } else {
            rc = ap_keyboard(modes[idx].initial, NULL, modes[idx].layout, &result);
        }

        if (rc == AP_OK) {
            char msg[1100];
            snprintf(msg, sizeof(msg), "Mode: %s\nYou typed:\n%s", modes[idx].label, result.text);
            ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
            ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
            ap_confirm_result cr;
            ap_confirmation(&m, &cr);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Confirmation
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_confirmation(void) {
    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "NO" },
        { .button = AP_BTN_A, .label = "YES", .is_confirm = true },
    };

    /* Standard confirmation (no image) */
    ap_message_opts opts = {
        .message      = "Are you sure you want to do this?\nThis action cannot be undone.",
        .image_path   = NULL,
        .footer       = footer,
        .footer_count = 2,
    };

    ap_confirm_result result;
    int rc = ap_confirmation(&opts, &result);

    if (rc == AP_OK) {
        const char *msg = result.confirmed ? "You said YES!" : "You said NO.";
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }

    /* Confirmation with image */
    ap_message_opts img_opts = {
        .message      = "This confirmation has an image above it.\nPretty neat, right?",
        .image_path   = "demo_icon.png",
        .footer       = footer,
        .footer_count = 2,
    };

    ap_confirm_result img_result;
    rc = ap_confirmation(&img_opts, &img_result);

    if (rc == AP_OK) {
        const char *msg = img_result.confirmed ? "You said YES (with image)!" : "You said NO (with image).";
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }

    /* Long wrapped confirmation for vertical centering checks */
    ap_message_opts long_opts = {
        .message      = "Create shortcut?\n\nDragon Warrior Monsters 2: Tara's Adventure (GBC)\n\nConsole: Game Boy Color\nROM: Dragon Warrior Monsters 2 - Tara's Adventure (USA, Europe) (Rev 1).gbc",
        .image_path   = NULL,
        .footer       = footer,
        .footer_count = 2,
    };

    ap_confirm_result long_result;
    rc = ap_confirmation(&long_opts, &long_result);

    if (rc == AP_OK) {
        const char *msg = long_result.confirmed ? "You created the long confirmation case." : "You cancelled the long confirmation case.";
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Selection (horizontal pill chooser)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_selection(void) {
    ap_selection_option opts[] = {
        { .label = "Easy",   .value = "easy"   },
        { .label = "Normal", .value = "normal" },
        { .label = "Hard",   .value = "hard"   },
    };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "CANCEL" },
        { .button = AP_BTN_A, .label = "CHOOSE", .is_confirm = true },
    };

    ap_selection_result result;
    int rc = ap_selection("Choose difficulty for this intentionally long prompt example on narrower displays:",
                          opts, 3, footer, 2, &result);

    if (rc == AP_OK) {
        char msg[128];
        snprintf(msg, sizeof(msg), "You selected: %s", opts[result.selected_index].label);
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Process Message (async worker)
 * ═══════════════════════════════════════════════════════════════════════════ */
static int fake_work(void *userdata) {
    float *progress = (float *)userdata;
    for (int i = 0; i <= 100; i++) {
        *progress = (float)i / 100.0f;
        SDL_Delay(30);
    }
    return AP_OK;
}

static void demo_process(void) {
    float progress = 0.0f;

    ap_process_opts opts = {
        .message        = "Doing important work...",
        .show_progress  = true,
        .progress       = &progress,
    };

    int rc = ap_process_message(&opts, fake_work, &progress);

    const char *msg = (rc == AP_OK) ? "Work complete!" : "Cancelled.";
    ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
    ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
    ap_confirm_result cr;
    ap_confirmation(&m, &cr);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Detail Screen
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_detail(void) {
    ap_detail_info_pair info[] = {
        { .key = "Name",             .value = "Apostrophe" },
        { .key = "Version",          .value = "1.1.0" },
        { .key = "Author",           .value = "Helaas" },
        { .key = "License",          .value = "MIT" },
        { .key = "Language",         .value = "C (header-only)" },
        { .key = "Renderer",         .value = "SDL2" },
        { .key = "Reference Width",  .value = "1024px" },
        { .key = "Widgets",          .value = "List, Options, Keyboard, Detail, Selection, Process, Color Picker" },
        { .key = "Targets",          .value = "Desktop + NextUI handheld devices" },
        { .key = "Repository",       .value = "github.com/Helaas/apostrophe" },
        { .key = "Focus",            .value = "Portable, practical, gamepad-first interfaces" },
        { .key = "Demo Goal",        .value = "Show every widget and stress scrolling behavior" },
    };

    /* Table data: supported platforms */
    const char *table_hdrs[] = { "Platform", "Resolution", "Scale" };
    const char *row0[] = { "MY355",  "640x480",   "2x" };
    const char *row1[] = { "TG5040", "1024x768",  "3x" };
    const char *row2[] = { "TG5050", "1280x720",  "2x" };
    const char *row3[] = { "macOS",  "1024x768",  "3x" };
    const char **table_rows[] = { row0, row1, row2, row3 };

    ap_detail_section sections[] = {
        {
            .type       = AP_SECTION_INFO,
            .title      = "Project Info",
            .info_pairs = info,
            .info_count = (int)(sizeof(info) / sizeof(info[0])),
        },
        {
            .type       = AP_SECTION_IMAGE,
            .title      = "Icon",
            .image_path = "demo_icon.png",
            .image_w    = AP_DS(128),
            .image_h    = AP_DS(128),
        },
        {
            .type        = AP_SECTION_DESCRIPTION,
            .title       = "About",
            .description = "Apostrophe is a header-only C UI toolkit for building "
                           "graphical tools on retro gaming handhelds running NextUI. "
                           "It provides a comprehensive set of pre-built widgets "
                           "including lists, keyboards, settings panels, process views, "
                           "detail screens, and other reusable screens that can be composed "
                           "into complete applications with very little glue code.",
        },
        {
            .type             = AP_SECTION_TABLE,
            .title            = "Supported Platforms",
            .table_headers    = table_hdrs,
            .table_rows       = table_rows,
            .table_cols       = 3,
            .table_rows_count = 4,
        },
        {
            .type        = AP_SECTION_DESCRIPTION,
            .title       = "Why This Screen Is Long",
            .description = "This section intentionally contains extra text to demonstrate "
                           "vertical scrolling in the detail widget. Use D-Pad Up and Down "
                           "to move through content, and watch the scrollbar update as the "
                           "content offset changes. The goal is to make it easy to validate "
                           "scroll feel on both smaller and larger screens without needing to "
                           "edit source code each time.",
        },
        {
            .type        = AP_SECTION_DESCRIPTION,
            .title       = "Input Guide",
            .description = "A/B/X/Y and shoulder buttons are available depending on your "
                           "device mapping, but this screen only needs the basics: B returns "
                           "to the previous menu and D-Pad handles scrolling. If your target "
                           "device has lower resolution, this page should still be comfortably "
                           "readable due to scaling and text wrapping.",
        },
        {
            .type        = AP_SECTION_DESCRIPTION,
            .title       = "Implementation Notes",
            .description = "The demo keeps this content static and local, but the same widget "
                           "can render runtime data such as package metadata, release notes, "
                           "game details, diagnostics, legal notices, or troubleshooting "
                           "instructions. You can split those into multiple sections so users "
                           "can scan headings while scrolling.",
        },
        {
            .type        = AP_SECTION_DESCRIPTION,
            .title       = "Sample Changelog",
            .description = "v1.0.0: Initial public header-only release with list, options, "
                           "keyboard, confirmation, selection, process, detail, and color "
                           "widgets.\n\nv1.1.0: Added scroll position restoration and richer "
                           "footer actions.\n\nv1.2.0: Improved visual parity and tightened "
                           "input behavior on handheld targets.",
        },
    };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
    };

    ap_detail_opts opts = {
        .title         = "About Apostrophe",
        .sections      = sections,
        .section_count = (int)(sizeof(sections) / sizeof(sections[0])),
        .footer        = footer,
        .footer_count  = 1,
    };

    ap_detail_result result;
    ap_detail_screen(&opts, &result);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Detail Screen (Styled) — centered title, separators, custom key color
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_detail_styled(void) {
    ap_detail_info_pair info[] = {
        { .key = "Status",   .value = "Enabled" },
        { .key = "IP",       .value = "192.168.1.42" },
        { .key = "Port",     .value = "22" },
        { .key = "Connect",  .value = "ssh root@192.168.1.42 -p 22" },
        { .key = "Password", .value = "tina" },
    };

    ap_detail_section sections[] = {
        {
            .type       = AP_SECTION_INFO,
            .title      = "SSH",
            .info_pairs = info,
            .info_count = (int)(sizeof(info) / sizeof(info[0])),
        },
        {
            .type        = AP_SECTION_DESCRIPTION,
            .title       = "About",
            .description = "This demo shows the styled detail screen options: "
                           "centered title, section separator lines, and "
                           "custom key colors. These are all opt-in — existing "
                           "apps that zero-initialize ap_detail_opts are unaffected.",
        },
    };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "ACTION", .is_confirm = true },
    };

    ap_color key_col = ap_get_theme()->text;

    ap_detail_opts opts = {
        .title                  = "Styled Detail",
        .sections               = sections,
        .section_count          = (int)(sizeof(sections) / sizeof(sections[0])),
        .footer                 = footer,
        .footer_count           = 2,
        .center_title           = true,
        .show_section_separator = true,
        .key_color              = &key_col,
    };

    ap_detail_result result;
    ap_detail_screen(&opts, &result);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Detail Screen (Custom Fonts) — per-widget font overrides
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_detail_custom_fonts(void) {
    ap_detail_info_pair info[] = {
        { .key = "Name",      .value = "Apostrophe" },
        { .key = "Version",   .value = "1.1.0" },
        { .key = "Platform",  .value = "All supported devices" },
        { .key = "License",   .value = "MIT" },
    };

    ap_detail_section sections[] = {
        {
            .type       = AP_SECTION_INFO,
            .title      = "Project",
            .info_pairs = info,
            .info_count = (int)(sizeof(info) / sizeof(info[0])),
        },
        {
            .type        = AP_SECTION_DESCRIPTION,
            .title       = "About Custom Fonts",
            .description = "This demo uses SMALL for body text instead of the "
                           "default TINY, and MEDIUM for section headers. Compare "
                           "with the default detail screen to see the difference. "
                           "Font overrides are opt-in — omit them or pass NULL to "
                           "keep the widget defaults.",
        },
    };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
    };

    ap_detail_opts opts = {
        .title              = "Custom Fonts",
        .sections           = sections,
        .section_count      = (int)(sizeof(sections) / sizeof(sections[0])),
        .footer             = footer,
        .footer_count       = 1,
        .body_font          = ap_get_font(AP_FONT_SMALL),
        .section_title_font = ap_get_font(AP_FONT_MEDIUM),
        .key_font           = ap_get_font(AP_FONT_SMALL),
    };

    ap_detail_result result;
    ap_detail_screen(&opts, &result);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Image List (list with thumbnails + extra action buttons)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_image_list(void) {
    SDL_Texture *icon = ap_load_image("demo_icon.png");

    ap_list_item items[] = {
        { .label = "Sunset",    .image = icon, .trailing_text = "JPEG" },
        { .label = "Mountain",  .image = icon, .trailing_text = "RAW" },
        { .label = "Ocean",     .image = icon, .trailing_text = "PNG" },
        { .label = "Forest",    .image = icon, .trailing_text = "TIFF" },
        { .label = "Desert",    .image = icon, .trailing_text = "GIF" },
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B,      .label = "BACK" },
        { .button = AP_BTN_START,  .label = "ACTION" },
        { .button = AP_BTN_Y,      .label = "INFO" },
        { .button = AP_BTN_SELECT, .label = "EXTRA" },
        { .button = AP_BTN_A,      .label = "OPEN", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Image Gallery", items, count);
    opts.footer                  = footer;
    opts.footer_count            = 5;
    opts.action_button           = AP_BTN_START;
    opts.show_images             = true;
    opts.secondary_action_button = AP_BTN_Y;
    opts.tertiary_action_button  = AP_BTN_SELECT;

    ap_list_result result;
    int rc = ap_list(&opts, &result);

    if (rc == AP_OK && result.selected_index >= 0) {
        const char *action_name = "Selected";
        if (result.action == AP_ACTION_TRIGGERED) action_name = "Action (START)";
        else if (result.action == AP_ACTION_SECONDARY_TRIGGERED) action_name = "Info (Y)";
        else if (result.action == AP_ACTION_TERTIARY_TRIGGERED) action_name = "Extra (SELECT)";

        char msg[256];
        snprintf(msg, sizeof(msg), "Action: %s\nItem: %s", action_name, items[result.selected_index].label);
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Advanced Process Message (cancel, dynamic message, multi-line)
 * ═══════════════════════════════════════════════════════════════════════════ */
struct cancel_work_data {
    float *progress;
    int   *interrupt;
};

static int cancelable_work(void *userdata) {
    struct cancel_work_data *d = (struct cancel_work_data *)userdata;
    for (int i = 0; i <= 100; i++) {
        if (*d->interrupt) return AP_CANCELLED;
        *d->progress = (float)i / 100.0f;
        SDL_Delay(50);
    }
    return AP_OK;
}

struct dynamic_work_data {
    float  *progress;
    int    *interrupt;
    char  **message;
    char    buf[256];
};

static const char *dynamic_steps[] = {
    "Initializing...",
    "Loading assets...",
    "Processing images...",
    "Compiling shaders...",
    "Optimizing layout...",
    "Building index...",
    "Finalizing...",
    "Cleaning up...",
    "Verifying output...",
    "Almost done...",
};

static int dynamic_work(void *userdata) {
    struct dynamic_work_data *d = (struct dynamic_work_data *)userdata;
    for (int i = 0; i < 10; i++) {
        if (d->interrupt && *d->interrupt) return AP_CANCELLED;
        snprintf(d->buf, sizeof(d->buf), "Step %d of 10:\n%s", i + 1, dynamic_steps[i]);
        *d->message = d->buf;
        *d->progress = (float)i / 10.0f;
        SDL_Delay(400);
    }
    *d->progress = 1.0f;
    return AP_OK;
}

static void demo_process_advanced(void) {
    ap_list_item items[] = {
        { .label = "Cancellable" },
        { .label = "Dynamic Message" },
        { .label = "Cancel + Dynamic" },
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "RUN", .is_confirm = true },
    };

    ap_list_opts list_opts = ap_list_default_opts("Advanced Process", items, count);
    list_opts.footer       = footer;
    list_opts.footer_count = 2;

    ap_list_result lr;
    int rc = ap_list(&list_opts, &lr);
    if (rc != AP_OK || lr.selected_index < 0) return;

    float progress = 0.0f;
    int   interrupt = 0;
    const char *result_msg = NULL;

    if (lr.selected_index == 0) {
        /* Cancellable */
        struct cancel_work_data wd = { .progress = &progress, .interrupt = &interrupt };
        ap_process_opts opts = {
            .message          = "Working (B to cancel)...",
            .show_progress    = true,
            .progress         = &progress,
            .interrupt_signal = &interrupt,
            .interrupt_button = AP_BTN_B,
        };
        rc = ap_process_message(&opts, cancelable_work, &wd);
        result_msg = (rc == AP_OK) ? "Work completed!" : "Cancelled by user.";
    } else if (lr.selected_index == 1) {
        /* Dynamic message */
        char *dyn_msg = NULL;
        struct dynamic_work_data wd = { .progress = &progress, .interrupt = NULL, .message = &dyn_msg };
        ap_process_opts opts = {
            .message         = "Processing...",
            .show_progress   = true,
            .progress        = &progress,
            .dynamic_message = &dyn_msg,
            .message_lines   = 2,
        };
        rc = ap_process_message(&opts, dynamic_work, &wd);
        result_msg = (rc == AP_OK) ? "All steps completed!" : "Process failed.";
    } else {
        /* Cancel + Dynamic */
        char *dyn_msg = NULL;
        struct dynamic_work_data wd = {
            .progress = &progress, .interrupt = &interrupt, .message = &dyn_msg
        };
        ap_process_opts opts = {
            .message          = "Processing (B to cancel)...",
            .show_progress    = true,
            .progress         = &progress,
            .interrupt_signal = &interrupt,
            .interrupt_button = AP_BTN_B,
            .dynamic_message  = &dyn_msg,
            .message_lines    = 2,
        };
        rc = ap_process_message(&opts, dynamic_work, &wd);
        result_msg = (rc == AP_OK) ? "All steps completed!" : "Cancelled by user.";
    }

    ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
    ap_message_opts m = { .message = result_msg, .footer = ok_foot, .footer_count = 1 };
    ap_confirm_result cr;
    ap_confirmation(&m, &cr);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Drawing Primitives (shapes, text, UI components, texture cache)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_drawing_primitives(void) {
    TTF_Font *body_font  = ap_get_font(AP_FONT_LARGE);
    TTF_Font *small_font = ap_get_font(AP_FONT_SMALL);
    ap_theme *theme = ap_get_theme();
    ap_color fg     = theme->text;
    ap_color accent = theme->accent;
    ap_color hint   = theme->hint;

    int screen_w = ap_get_screen_width();
    int pad = AP_DS(12);
    int scrollbar_gutter = AP_S(16);
    int scroll_step = AP_S(40);
    int page = 0;
    int page_count = 4;
    int page_scroll[4] = {0};
    int page_content_h[4] = {0};
    bool running = true;

    /* Text scrolling state */
    ap_text_scroll scroll = {0};
    ap_text_scroll_init(&scroll);
    const char *scroll_text = "This is a long text that will scroll horizontally because it overflows its container width.";
    uint32_t last_tick = SDL_GetTicks();

    /* Animated values */
    float anim_progress = 0.0f;
    int   anim_scroll_offset = 0;

    /* Texture cache demo state */
    bool cache_loaded = false;

    /* Load image once for page 2 (UI Components) demo */
    SDL_Texture *page2_icon = ap_load_image("demo_icon.png");

    ap_footer_item footer[] = {
        { .button = AP_BTN_B,     .label = "BACK" },
        { .button = AP_BTN_LEFT,  .label = "PREV" },
        { .button = AP_BTN_RIGHT, .label = "NEXT" },
        { .button = AP_BTN_X,     .label = "RESET" },
    };

    while (running) {
        uint32_t now = SDL_GetTicks();
        uint32_t dt = now - last_tick;
        last_tick = now;

        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_UP:
                    page_scroll[page] -= scroll_step;
                    if (page_scroll[page] < 0) page_scroll[page] = 0;
                    break;
                case AP_BTN_DOWN:
                    page_scroll[page] += scroll_step;
                    break;
                case AP_BTN_LEFT:
                    page = (page - 1 + page_count) % page_count;
                    page_scroll[page] = 0;
                    break;
                case AP_BTN_RIGHT:
                    page = (page + 1) % page_count;
                    page_scroll[page] = 0;
                    break;
                case AP_BTN_X:
                    if (page == 1) {
                        ap_text_scroll_reset(&scroll);
                    }
                    break;
                case AP_BTN_B:
                    running = false;
                    break;
                default: break;
            }
        }

        ap_clear_screen();
        char title[64];
        const char *page_names[] = { "Shapes", "Text", "UI Components", "Texture Cache" };
        snprintf(title, sizeof(title), "Drawing: %s (%d/%d)", page_names[page], page + 1, page_count);
        ap_draw_screen_title(title, NULL);

        SDL_Rect content_rect = ap_get_content_rect(true, true, false);
        int content_x = pad;
        int content_y = content_rect.y;
        int content_w = screen_w - pad * 2 - scrollbar_gutter;
        if (content_w < 1) content_w = screen_w - pad * 2;
        if (content_w < 1) content_w = 1;
        int max_scroll = page_content_h[page] - content_rect.h;
        if (max_scroll < 0) max_scroll = 0;
        if (page_scroll[page] > max_scroll) page_scroll[page] = max_scroll;

        SDL_Rect page_clip = { content_x, content_y, content_w, content_rect.h };
        int scroll_offset = page_scroll[page];
        int y = content_y - scroll_offset;

        SDL_RenderSetClipRect(ap_get_renderer(), &page_clip);

        if (page == 0) {
            /* ── Shapes ── */
            int cx = content_x;
            int shape_h = AP_DS(50);
            int gap = AP_DS(15);

            ap_draw_text_clipped(small_font, "ap_draw_rect:", cx, y, hint, content_w);
            y += AP_DS(16);
            ap_draw_rect(cx, y, AP_DS(80), shape_h, accent);
            y += shape_h + gap;

            ap_draw_text_clipped(small_font, "ap_draw_rounded_rect:", cx, y, hint, content_w);
            y += AP_DS(16);
            ap_draw_rounded_rect(cx, y, AP_DS(80), shape_h, AP_DS(10), accent);
            y += shape_h + gap;

            ap_draw_text_clipped(small_font, "ap_draw_pill:", cx, y, hint, content_w);
            y += AP_DS(16);
            ap_draw_pill(cx, y, AP_DS(120), AP_DS(30), accent);
            y += AP_DS(30) + gap;

            ap_draw_text_clipped(small_font, "ap_draw_circle:", cx, y, hint, content_w);
            y += AP_DS(16);
            ap_draw_circle(cx + AP_DS(25), y + AP_DS(25), AP_DS(25), accent);
            y += AP_DS(50);

        } else if (page == 1) {
            /* ── Text ── */
            int max_w = content_w;

            ap_draw_text_clipped(small_font, "ap_draw_text_clipped (max 200px):", content_x, y, hint, content_w);
            y += AP_DS(16);
            int clip_w = AP_DS(200);
            if (clip_w > content_w - AP_S(4)) clip_w = content_w - AP_S(4);
            if (clip_w < 1) clip_w = 1;
            ap_draw_text_clipped(body_font, "This text is clipped at a maximum width boundary",
                                 content_x, y, fg, clip_w);
            /* Draw clip boundary */
            ap_draw_rect(content_x + clip_w, y, 1, AP_DS(20), accent);
            y += AP_DS(32);

            ap_draw_text_clipped(small_font, "ap_draw_text_wrapped (LEFT):", content_x, y, hint, content_w);
            y += AP_DS(16);
            const char *left_text = "This paragraph demonstrates wrapped text with left alignment. "
                "The text flows naturally within the given width.";
            ap_draw_text_wrapped(body_font, left_text,
                content_x, y, max_w, fg, AP_ALIGN_LEFT);
            y += ap_measure_wrapped_text_height(body_font, left_text, max_w) + AP_DS(8);

            ap_draw_text_clipped(small_font, "ap_draw_text_wrapped (CENTER):", content_x, y, hint, content_w);
            y += AP_DS(16);
            const char *center_text = "Centered text wrapping in the same responsive content width.";
            ap_draw_text_wrapped(body_font, center_text,
                content_x, y, max_w, fg, AP_ALIGN_CENTER);
            y += ap_measure_wrapped_text_height(body_font, center_text, max_w) + AP_DS(8);

            ap_draw_text_clipped(small_font, "ap_text_scroll (overflow):", content_x, y, hint, content_w);
            y += AP_DS(16);
            int scroll_w = AP_DS(200);
            if (scroll_w > content_w) scroll_w = content_w;
            int text_w = ap_measure_text(body_font, scroll_text);
            ap_text_scroll_update(&scroll, text_w, scroll_w, dt);
            if (scroll.active) ap_request_frame();

            /* Clip the marquee row to both its own box and the page viewport. */
            SDL_Rect scroll_clip = { content_x, y, scroll_w, AP_DS(22) };
            SDL_Rect effective_scroll_clip;
            if (SDL_IntersectRect(&page_clip, &scroll_clip, &effective_scroll_clip)) {
                SDL_RenderSetClipRect(ap_get_renderer(), &effective_scroll_clip);
                ap_draw_text(body_font, scroll_text, content_x - scroll.offset, y, fg);
            }
            SDL_RenderSetClipRect(ap_get_renderer(), &page_clip);
            /* Draw scroll container border */
            ap_draw_rect(content_x + scroll_w, y, 1, AP_DS(22), accent);
            y += AP_DS(22);

        } else if (page == 2) {
            /* ── UI Components ── */
            ap_draw_text_clipped(small_font, "ap_draw_progress_bar:", content_x, y, hint, content_w);
            y += AP_DS(16);
            anim_progress += 0.005f;
            if (anim_progress > 1.0f) anim_progress = 0.0f;
            ap_color bar_bg = { hint.r, hint.g, hint.b, 80 };
            ap_draw_progress_bar(content_x, y, content_w, AP_DS(16), anim_progress, accent, bar_bg);
            y += AP_DS(30);

            ap_draw_text_clipped(small_font, "ap_draw_scrollbar:", content_x, y, hint, content_w);
            y += AP_DS(16);
            int sb_h = AP_DS(120);
            anim_scroll_offset = (anim_scroll_offset + 1) % 50;
            ap_draw_scrollbar(content_x + content_w - AP_S(6), y, sb_h, 10, 50, anim_scroll_offset);
            y += sb_h + AP_DS(15);

            ap_draw_text_clipped(small_font, "ap_load_image + ap_draw_image:", content_x, y, hint, content_w);
            y += AP_DS(16);
            if (page2_icon) {
                ap_draw_image(page2_icon, content_x, y, AP_DS(64), AP_DS(64));
                y += AP_DS(64);
            }

        } else if (page == 3) {
            /* ── Texture Cache ── */
            ap_draw_text_clipped(small_font, "Texture Cache (ap_cache_*)", content_x, y, hint, content_w);
            y += AP_DS(20);

            if (!cache_loaded) {
                SDL_Texture *img = ap_load_image("demo_icon.png");
                if (img) {
                    ap_cache_put("demo_cached_icon", img, AP_DS(64), AP_DS(64));
                    cache_loaded = true;
                }
            }

            int cw = 0, ch = 0;
            SDL_Texture *cached = ap_cache_get("demo_cached_icon", &cw, &ch);

            if (cached) {
                ap_draw_text_clipped(body_font, "ap_cache_get: HIT", content_x, y, fg, content_w);
                y += AP_DS(22);
                char dim[64];
                snprintf(dim, sizeof(dim), "Cached size: %dx%d", cw, ch);
                ap_draw_text_clipped(small_font, dim, content_x, y, hint, content_w);
                y += AP_DS(20);
                ap_draw_image(cached, content_x, y, cw, ch);
                y += ch + AP_DS(15);
            } else {
                ap_draw_text_clipped(body_font, "ap_cache_get: MISS", content_x, y, fg, content_w);
                y += AP_DS(22);
            }

            /* Show miss case for a different key */
            SDL_Texture *miss = ap_cache_get("nonexistent_key", NULL, NULL);
            ap_draw_text_clipped(body_font,
                                 miss ? "nonexistent: HIT" : "nonexistent_key: MISS (expected)",
                                 content_x, y, fg, content_w);
            y += AP_DS(22);

            ap_draw_text_wrapped(small_font,
                                 "Use UP/DOWN to scroll when needed. Press LEFT/RIGHT to browse pages.",
                                 content_x, y, content_w, hint, AP_ALIGN_LEFT);
            y += ap_measure_wrapped_text_height(small_font,
                                                "Use UP/DOWN to scroll when needed. Press LEFT/RIGHT to browse pages.",
                                                content_w);
        }

        SDL_RenderSetClipRect(ap_get_renderer(), NULL);

        page_content_h[page] = y - (content_y - scroll_offset);
        if (page_content_h[page] < 0) page_content_h[page] = 0;

        max_scroll = page_content_h[page] - content_rect.h;
        if (max_scroll < 0) max_scroll = 0;
        if (page_scroll[page] > max_scroll) page_scroll[page] = max_scroll;

        if (max_scroll > 0) {
            int scrollbar_x = content_x + content_w + AP_S(6);
            ap_draw_scrollbar(scrollbar_x, content_y, content_rect.h,
                              content_rect.h, page_content_h[page], page_scroll[page]);
        }

        ap_draw_footer(footer, 4);
        ap_present();
        SDL_Delay(16);
    }

    if (page2_icon) SDL_DestroyTexture(page2_icon);
    ap_cache_clear();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Screen Fade
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_screen_fade(void) {
    TTF_Font *title_font = ap_get_font(AP_FONT_EXTRA_LARGE);
    TTF_Font *body_font  = ap_get_font(AP_FONT_LARGE);
    ap_theme *theme = ap_get_theme();
    ap_color fg = theme->text;

    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();

    ap_fade fade = {0};
    ap_fade_begin_in(&fade, 500);

    bool fading_out = false;
    bool running = true;

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "FADE OUT", .is_confirm = true },
    };

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_A:
                    if (!fading_out) {
                        ap_fade_begin_out(&fade, 500);
                        fading_out = true;
                    }
                    break;
                case AP_BTN_B:
                    running = false;
                    break;
                default: break;
            }
        }

        ap_clear_screen();

        /* Draw scene content */
        const char *t1 = "Fade Demo";
        int tw = ap_measure_text(title_font, t1);
        ap_draw_text(title_font, t1, (screen_w - tw) / 2, screen_h / 2 - AP_DS(30), fg);

        const char *t2 = fading_out ? "Fading out..." : "Press A to fade out";
        tw = ap_measure_text(body_font, t2);
        ap_draw_text(body_font, t2, (screen_w - tw) / 2, screen_h / 2 + AP_DS(10), fg);

        ap_draw_footer(footer, 2);

        /* Draw fade overlay AFTER scene, BEFORE present */
        bool still_active = ap_fade_draw(&fade);

        ap_present();

        /* If fade-out finished, exit */
        if (fading_out && !still_active) {
            running = false;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Help Overlay (standalone)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_help_overlay_screen(void) {
    TTF_Font *body_font = ap_get_font(AP_FONT_LARGE);
    ap_color fg = ap_get_theme()->text;
    int screen_w = ap_get_screen_width();
    int screen_h = ap_get_screen_height();
    bool running = true;

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "SHOW HELP", .is_confirm = true },
    };

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_A:
                    ap_show_help_overlay(
                        "This is a standalone help overlay.\n\n"
                        "It can display any multi-line text.\n"
                        "Useful for in-app instructions,\n"
                        "keyboard shortcuts, or tips.\n\n"
                        "Press any button to dismiss."
                    );
                    break;
                case AP_BTN_B:
                    running = false;
                    break;
                default: break;
            }
        }

        ap_clear_screen();
        const char *prompt = "Press A to show help overlay";
        int tw = ap_measure_text(body_font, prompt);
        ap_draw_text(body_font, prompt, (screen_w - tw) / 2, screen_h / 2, fg);
        ap_draw_footer(footer, 2);
        ap_present();
        SDL_Delay(16);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Core API Lab
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_core_api_lab(void) {
    TTF_Font *title_font = ap_get_font(AP_FONT_LARGE);
    TTF_Font *body_font  = ap_get_font(AP_FONT_SMALL);
    TTF_Font *tiny_font  = ap_get_font(AP_FONT_TINY);
    ap_theme *theme = ap_get_theme();
    ap_color fg   = theme->text;
    ap_color hint = theme->hint;
    ap_status_bar_opts status_bar = {
        .show_clock = AP_CLOCK_SHOW,
        .use_24h    = true,
        .show_battery = true,
        .show_wifi    = true,
    };
    static const char *page_names[] = {
        "Metrics",
        "Font Bump",
        "Log & Error",
        "Window & Device",
    };

    ap_text_scroll scroll = {0};
    ap_text_scroll_init(&scroll);

    char default_log_path[1024] = {0};
    const char *resolved_log = ap_resolve_log_path("demo");
    bool have_default_log = false;
    if (resolved_log) {
        strncpy(default_log_path, resolved_log, sizeof(default_log_path) - 1);
        have_default_log = true;
    }

    bool using_alt_log = false;
    bool power_handler_enabled = AP_PLATFORM_IS_DEVICE;
    int  page = 0;
    int  last_init_rc = AP_OK;
    int  last_theme_rc = AP_OK;
    char last_error[256] = "No duplicate-init check has been run yet.";
    char device_status[256] = "No action triggered yet.";
    int  font_bump_scroll = 0;
    int  font_bump_content_h = 0;
    bool running = true;
    uint32_t last_tick = SDL_GetTicks();

    while (running) {
        uint32_t now = SDL_GetTicks();
        uint32_t dt = now - last_tick;
        last_tick = now;

        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_LEFT:
                    page = (page - 1 + 4) % 4;
                    break;
                case AP_BTN_RIGHT:
                    page = (page + 1) % 4;
                    break;
                case AP_BTN_B:
                    running = false;
                    break;
                case AP_BTN_X:
                    if (page == 0) {
                        ap_text_scroll_reset(&scroll);
                    } else if (page == 2) {
                        last_init_rc = ap_init(NULL);
                        snprintf(last_error, sizeof(last_error),
                                 "Duplicate init rc=%d, error=%.220s",
                                 last_init_rc, ap_get_error());
                    } else if (AP_PLATFORM_IS_DEVICE) {
                        last_theme_rc = ap_theme_load_nextui();
                        snprintf(device_status, sizeof(device_status),
                                 "Theme reload rc=%d", last_theme_rc);
                    } else {
                        snprintf(device_status, sizeof(device_status),
                                 "Theme reload is device-only.");
                    }
                    break;
                case AP_BTN_A:
                    if (page == 2) {
                        if (!using_alt_log) {
                            ap_set_log_path("demo-alt.log");
                            ap_log("core lab: switched to demo-alt.log");
                            using_alt_log = true;
                        } else {
                            ap_set_log_path(have_default_log ? default_log_path : NULL);
                            ap_log("core lab: restored default log target");
                            using_alt_log = false;
                        }
                    } else if (page == 3 && AP_PLATFORM_IS_DEVICE) {
                        power_handler_enabled = !power_handler_enabled;
                        ap_set_power_handler(power_handler_enabled);
                        snprintf(device_status, sizeof(device_status),
                                 "Power handler %s",
                                 power_handler_enabled ? "enabled" : "disabled");
                    }
                    break;
                case AP_BTN_START:
                    if (page == 3 && !AP_PLATFORM_IS_DEVICE) {
                        if (ap_get_window()) {
                            ap_hide_window();
                            SDL_Delay(200);
                            ap_show_window();
                            snprintf(device_status, sizeof(device_status),
                                     "Window blinked via ap_hide_window()/ap_show_window().");
                        } else {
                            snprintf(device_status, sizeof(device_status),
                                     "No SDL window is available.");
                        }
                    }
                    break;
                case AP_BTN_UP:
                    if (page == 1) font_bump_scroll -= AP_DS(30);
                    break;
                case AP_BTN_DOWN:
                    if (page == 1) font_bump_scroll += AP_DS(30);
                    break;
                case AP_BTN_MENU:
                    ap_show_footer_overflow();
                    break;
                default:
                    break;
            }
        }

        ap_draw_background();
        ap_draw_status_bar(&status_bar);

        int pad = AP_DS(12);
        int title_max_w = ap_get_screen_width() - pad * 2 - ap_get_status_bar_width(&status_bar) - AP_S(8);
        if (title_max_w < AP_DS(120)) title_max_w = ap_get_screen_width() - pad * 2;
        char title[96];
        snprintf(title, sizeof(title), "Core API Lab: %s", page_names[page]);
        ap_draw_text_clipped(title_font, title, pad, AP_DS(10), fg, title_max_w);

        SDL_Rect content_rect = ap_get_content_rect(true, true, true);
        int x = pad;
        int y = content_rect.y;
        int w = ap_get_screen_width() - pad * 2;
        if (w < 1) w = 1;

        if (page == 0) {
            char line[256];
            snprintf(line, sizeof(line), "Scale factor: %.3f, Font bump: %d/%d",
                     ap_get_scale_factor(), ap_get_font_bump(), AP_FONT_BUMP_MAX);
            ap_draw_text_clipped(body_font, line, x, y, fg, w);
            y += AP_DS(20);

            snprintf(line, sizeof(line), "ap_scale(20)=%d, ap_font_size_for_resolution(16)=%d",
                     ap_scale(20), ap_font_size_for_resolution(16));
            ap_draw_text_clipped(body_font, line, x, y, fg, w);
            y += AP_DS(20);

            SDL_Rect usable = ap_get_content_rect(true, true, true);
            snprintf(line, sizeof(line), "Content rect: x=%d y=%d w=%d h=%d",
                     usable.x, usable.y, usable.w, usable.h);
            ap_draw_text_clipped(body_font, line, x, y, fg, w);
            y += AP_DS(20);

            snprintf(line, sizeof(line), "Footer=%d  Status=%d x %d",
                     ap_get_footer_height(),
                     ap_get_status_bar_width(&status_bar),
                     ap_get_status_bar_height());
            ap_draw_text_clipped(body_font, line, x, y, fg, w);
            y += AP_DS(24);

            ap_draw_text(tiny_font, "ap_text_scroll_reset(): press X to reset the marquee.", x, y, hint);
            y += AP_DS(18);

            const char *scroll_text =
                "Long text preview for ap_text_scroll_reset, scaling, and direct background rendering.";
            int scroll_w = w - AP_DS(20);
            if (scroll_w < AP_DS(80)) scroll_w = AP_DS(80);
            int text_w = ap_measure_text(body_font, scroll_text);
            ap_text_scroll_update(&scroll, text_w, scroll_w, dt);
            if (scroll.active) ap_request_frame();

            SDL_Rect scroll_clip = { x, y, scroll_w, AP_DS(20) };
            SDL_RenderSetClipRect(ap_get_renderer(), &scroll_clip);
            ap_draw_text(body_font, scroll_text, x - scroll.offset, y, fg);
            SDL_RenderSetClipRect(ap_get_renderer(), NULL);
            ap_draw_rect(x + scroll_w, y, 1, AP_DS(20), theme->accent);
            y += AP_DS(32);

            ap_draw_text(body_font,
                         ap_is_cancelled(AP_CANCELLED) ? "ap_is_cancelled(AP_CANCELLED) => true"
                                                       : "ap_is_cancelled(AP_CANCELLED) => false",
                         x, y, fg);

            ap_footer_item footer[] = {
                { .button = AP_BTN_B,     .label = "BACK" },
                { .button = AP_BTN_LEFT,  .label = "PREV" },
                { .button = AP_BTN_RIGHT, .label = "NEXT" },
                { .button = AP_BTN_X,     .label = "RESET" },
            };
            ap_draw_footer(footer, 4);
        } else if (page == 1) {
            /* Font Bump page — render each tier with its label and effective size */
            static const char *tier_names[] = {
                "EXTRA_LARGE", "LARGE", "MEDIUM", "SMALL", "TINY", "MICRO"
            };

            /* Clamp scroll */
            int max_scroll = font_bump_content_h - content_rect.h;
            if (max_scroll < 0) max_scroll = 0;
            if (font_bump_scroll < 0) font_bump_scroll = 0;
            if (font_bump_scroll > max_scroll) font_bump_scroll = max_scroll;

            SDL_Rect clip = { x, content_rect.y, w, content_rect.h };
            SDL_RenderSetClipRect(ap_get_renderer(), &clip);

            y -= font_bump_scroll;
            int y_start = y;

            char line[256];
            int bump = ap_get_font_bump();
            snprintf(line, sizeof(line), "Font bump: %d/%d  (base sizes + %d before x%d scale)",
                     bump, AP_FONT_BUMP_MAX, bump, ap_font_size_for_resolution(1));
            ap_draw_text_clipped(body_font, line, x, y, hint, w);
            y += AP_DS(20);

            for (int t = 0; t < AP_FONT_TIER_COUNT; t++) {
                TTF_Font *f = ap_get_font((ap_font_tier)t);
                int h;
                TTF_SizeUTF8(f, "Ag", NULL, &h);
                snprintf(line, sizeof(line), "%s (%dpx)", tier_names[t], h);
                ap_draw_text_clipped(f, line, x, y, fg, w);
                y += h + AP_DS(4);
            }

            font_bump_content_h = y - y_start;
            SDL_RenderSetClipRect(ap_get_renderer(), NULL);

            if (max_scroll > 0) {
                ap_draw_scrollbar(x + w - AP_S(6), content_rect.y,
                                  content_rect.h, content_rect.h,
                                  font_bump_content_h, font_bump_scroll);
            }

            ap_footer_item footer[] = {
                { .button = AP_BTN_B,     .label = "BACK" },
                { .button = AP_BTN_LEFT,  .label = "PREV" },
                { .button = AP_BTN_RIGHT, .label = "NEXT" },
            };
            ap_draw_footer(footer, 3);
        } else if (page == 2) {
            char line[256];
            snprintf(line, sizeof(line), "Current log target: %.234s",
                     using_alt_log ? "demo-alt.log" :
                     (have_default_log ? default_log_path : "stderr only"));
            ap_draw_text_clipped(body_font, line, x, y, fg, w);
            y += AP_DS(20);

            ap_draw_text_clipped(body_font, "Press A to toggle ap_set_log_path().", x, y, fg, w);
            y += AP_DS(20);

            snprintf(line, sizeof(line), "Last duplicate init rc: %d", last_init_rc);
            ap_draw_text(body_font, line, x, y, fg);
            y += AP_DS(20);

            ap_draw_text_clipped(body_font, last_error, x, y, fg, w);
            y += ap_measure_wrapped_text_height(body_font, last_error, w) + AP_DS(10);

            snprintf(line, sizeof(line), "ap_is_cancelled(last_rc): %s",
                     ap_is_cancelled(last_init_rc) ? "true" : "false");
            ap_draw_text(body_font, line, x, y, fg);
            y += AP_DS(20);

            ap_draw_text_clipped(tiny_font,
                                 "Press X to call ap_init(NULL) again and read ap_get_error().",
                                 x, y, hint, w);

            ap_footer_item footer[] = {
                { .button = AP_BTN_B,     .label = "BACK" },
                { .button = AP_BTN_LEFT,  .label = "PREV" },
                { .button = AP_BTN_RIGHT, .label = "NEXT" },
                { .button = AP_BTN_A,     .label = "LOG" },
                { .button = AP_BTN_X,     .label = "ERROR" },
            };
            ap_draw_footer(footer, 5);
        } else {
            char line[256];
            snprintf(line, sizeof(line), "Window pointer: %p", (void *)ap_get_window());
            ap_draw_text_clipped(body_font, line, x, y, fg, w);
            y += AP_DS(20);

            if (AP_PLATFORM_IS_DEVICE) {
                snprintf(line, sizeof(line), "Power handler: %s",
                         power_handler_enabled ? "enabled" : "disabled");
                ap_draw_text(body_font, line, x, y, fg);
                y += AP_DS(20);

                snprintf(line, sizeof(line), "Theme reload rc: %d", last_theme_rc);
                ap_draw_text(body_font, line, x, y, fg);
                y += AP_DS(20);

                ap_draw_text_clipped(body_font, device_status, x, y, fg, w);
                y += ap_measure_wrapped_text_height(body_font, device_status, w) + AP_DS(10);

                ap_draw_text_clipped(tiny_font,
                                     "A toggles ap_set_power_handler(). X reloads ap_theme_load_nextui().",
                                     x, y, hint, w);

                ap_footer_item footer[] = {
                    { .button = AP_BTN_B,     .label = "BACK" },
                    { .button = AP_BTN_LEFT,  .label = "PREV" },
                    { .button = AP_BTN_RIGHT, .label = "NEXT" },
                    { .button = AP_BTN_A,     .label = "POWER" },
                    { .button = AP_BTN_X,     .label = "THEME" },
                };
                ap_draw_footer(footer, 5);
            } else {
                ap_draw_text_clipped(body_font,
                                     "Press START to blink the desktop window via ap_hide_window()/ap_show_window().",
                                     x, y, fg, w);
                y += ap_measure_wrapped_text_height(body_font,
                                                    "Press START to blink the desktop window via ap_hide_window()/ap_show_window().",
                                                    w) + AP_DS(10);

                ap_draw_text_clipped(body_font, device_status, x, y, fg, w);

                ap_footer_item footer[] = {
                    { .button = AP_BTN_B,     .label = "BACK" },
                    { .button = AP_BTN_LEFT,  .label = "PREV" },
                    { .button = AP_BTN_RIGHT, .label = "NEXT" },
                    { .button = AP_BTN_START, .label = "BLINK" },
                };
                ap_draw_footer(footer, 4);
            }
        }

        ap_present();
        SDL_Delay(16);
    }

    ap_set_log_path(have_default_log ? default_log_path : NULL);
    if (AP_PLATFORM_IS_DEVICE && !power_handler_enabled) {
        ap_set_power_handler(true);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Input & Theme Configuration
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_input_theme(void) {
    TTF_Font *body_font  = ap_get_font(AP_FONT_LARGE);
    TTF_Font *small_font = ap_get_font(AP_FONT_SMALL);
    ap_theme *theme = ap_get_theme();
    ap_color fg = theme->text;

    /* Save original accent to restore on exit */
    ap_color orig_accent = theme->accent;
    char orig_hex[8];
    snprintf(orig_hex, sizeof(orig_hex), "#%02X%02X%02X",
             orig_accent.r, orig_accent.g, orig_accent.b);

    int pad = AP_DS(12);
    int scrollbar_gutter = AP_S(16);
    int scroll_offset = 0;

    /* Input config state */
    static const int delays[] = { 0, 100, 200, 500 };
    int delay_idx = 0;
    ap_set_input_delay(delays[delay_idx]);

    static const struct { int delay; int rate; const char *label; } repeats[] = {
        { 300, 100, "300/100 (default)" },
        { 200,  50, "200/50 (fast)" },
        { 500, 200, "500/200 (slow)" },
    };
    int repeat_idx = 0;

    bool flipped = false;

    /* Theme color presets */
    static const struct { const char *hex; const char *name; } colors[] = {
        { "#6495ED", "Cornflower" },
        { "#FF6600", "Orange" },
        { "#00CC66", "Green" },
        { "#CC33FF", "Purple" },
        { "#FF3366", "Pink" },
        { "#33CCCC", "Teal" },
    };
    int color_idx = 0;

    /* Menu cursor */
    int sel = 0;
    int item_count = 4; /* delay, repeat, flip, theme color */
    bool running = true;

    ap_footer_item footer[] = {
        { .button = AP_BTN_B,     .label = "BACK" },
        { .button = AP_BTN_LEFT,  .label = "DEC" },
        { .button = AP_BTN_RIGHT, .label = "INC" },
    };

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_UP:   sel = (sel - 1 + item_count) % item_count; break;
                case AP_BTN_DOWN: sel = (sel + 1) % item_count;              break;
                case AP_BTN_LEFT:
                case AP_BTN_RIGHT: {
                    int dir = (ev.button == AP_BTN_RIGHT) ? 1 : -1;
                    if (sel == 0) {
                        delay_idx = (delay_idx + dir + 4) % 4;
                        ap_set_input_delay(delays[delay_idx]);
                    } else if (sel == 1) {
                        repeat_idx = (repeat_idx + dir + 3) % 3;
                        ap_set_input_repeat(repeats[repeat_idx].delay, repeats[repeat_idx].rate);
                    } else if (sel == 2) {
                        flipped = !flipped;
                        ap_flip_face_buttons(flipped);
                    } else if (sel == 3) {
                        color_idx = (color_idx + dir + 6) % 6;
                        ap_set_theme_color(colors[color_idx].hex);
                    }
                    break;
                }
                case AP_BTN_B: running = false; break;
                default: break;
            }
        }

        ap_clear_screen();
        ap_draw_screen_title("Input & Theme", NULL);
        SDL_Rect content_rect = ap_get_content_rect(true, true, false);
        int content_x = pad;
        int content_w = ap_get_screen_width() - pad * 2 - scrollbar_gutter;
        if (content_w < 1) content_w = ap_get_screen_width() - pad * 2;
        if (content_w < 1) content_w = 1;

        static const ap_button btns[] = {
            AP_BTN_A, AP_BTN_B, AP_BTN_X, AP_BTN_Y,
            AP_BTN_L1, AP_BTN_R1, AP_BTN_START, AP_BTN_SELECT
        };
        char names_buf[256];
        int off = 0;
        for (int i = 0; i < 8; i++) {
            if (i > 0) off += snprintf(names_buf + off, sizeof(names_buf) - off, ", ");
            off += snprintf(names_buf + off, sizeof(names_buf) - off, "%s", ap_button_name(btns[i]));
        }

        int row_gap = AP_DS(24);
        int swatch_h = AP_DS(45);
        int label_gap = AP_DS(16);
        int wrapped_names_h = ap_measure_wrapped_text_height(small_font, names_buf, content_w);
        if (wrapped_names_h < TTF_FontLineSkip(small_font)) {
            wrapped_names_h = TTF_FontLineSkip(small_font);
        }

        int row_y[4];
        int row_h[4];
        int content_h_total = 0;
        for (int i = 0; i < item_count; i++) {
            row_y[i] = content_h_total;
            row_h[i] = row_gap;
            if (i == 3) row_h[i] += swatch_h;
            content_h_total += row_h[i];
        }
        int summary_label_y = content_h_total;
        content_h_total += label_gap + wrapped_names_h;

        int max_scroll = content_h_total - content_rect.h;
        if (max_scroll < 0) max_scroll = 0;

        int row_margin = AP_DS(12);
        int selected_top = row_y[sel];
        int selected_bottom = row_y[sel] + row_h[sel];

        if (selected_top - scroll_offset < row_margin) {
            scroll_offset = selected_top - row_margin;
        }
        if (selected_bottom - scroll_offset > content_rect.h - row_margin) {
            scroll_offset = selected_bottom - (content_rect.h - row_margin);
        }
        if (sel >= item_count - 2) {
            int summary_bottom = content_h_total;
            int summary_scroll = summary_bottom - (content_rect.h - row_margin);
            if (summary_scroll > scroll_offset) scroll_offset = summary_scroll;
        }

        if (scroll_offset < 0) scroll_offset = 0;
        if (scroll_offset > max_scroll) scroll_offset = max_scroll;

        SDL_Rect page_clip = { content_x, content_rect.y, content_w, content_rect.h };
        SDL_RenderSetClipRect(ap_get_renderer(), &page_clip);

        int y = content_rect.y - scroll_offset;
        ap_color col = (sel == 0) ? ap_get_theme()->accent : fg;
        char line[128];

        /* Input delay */
        snprintf(line, sizeof(line), "%s Input Delay: %d ms", sel == 0 ? ">" : " ", delays[delay_idx]);
        ap_draw_text_clipped(body_font, line, content_x, y + row_y[0], col, content_w);

        /* Input repeat */
        col = (sel == 1) ? ap_get_theme()->accent : fg;
        snprintf(line, sizeof(line), "%s Repeat: %s", sel == 1 ? ">" : " ", repeats[repeat_idx].label);
        ap_draw_text_clipped(body_font, line, content_x, y + row_y[1], col, content_w);

        /* Face button flip */
        col = (sel == 2) ? ap_get_theme()->accent : fg;
        snprintf(line, sizeof(line), "%s Flip Buttons: %s", sel == 2 ? ">" : " ", flipped ? "ON" : "OFF");
        ap_draw_text_clipped(body_font, line, content_x, y + row_y[2], col, content_w);

        /* Theme color */
        col = (sel == 3) ? ap_get_theme()->accent : fg;
        snprintf(line, sizeof(line), "%s Accent: %s (%s)", sel == 3 ? ">" : " ", colors[color_idx].name, colors[color_idx].hex);
        ap_draw_text_clipped(body_font, line, content_x, y + row_y[3], col, content_w);

        /* Draw color swatch below the theme row */
        ap_color swatch = ap_hex_to_color(colors[color_idx].hex);
        ap_draw_rounded_rect(content_x + AP_DS(20), y + row_y[3] + row_gap,
                             AP_DS(100), AP_DS(30), AP_DS(6), swatch);

        /* Button names display */
        ap_draw_text_clipped(small_font, "ap_button_name() results:",
                             content_x, y + summary_label_y, ap_get_theme()->hint, content_w);
        ap_draw_text_wrapped(small_font, names_buf, content_x,
                             y + summary_label_y + label_gap, content_w, fg, AP_ALIGN_LEFT);

        SDL_RenderSetClipRect(ap_get_renderer(), NULL);

        if (max_scroll > 0) {
            int scrollbar_x = content_x + content_w + AP_S(6);
            ap_draw_scrollbar(scrollbar_x, content_rect.y, content_rect.h,
                              content_rect.h, content_h_total, scroll_offset);
        }

        ap_draw_footer(footer, 3);
        ap_present();
        SDL_Delay(16);
    }

    /* Restore original settings */
    ap_set_theme_color(orig_hex);
    ap_set_input_delay(AP_INPUT_DEBOUNCE);
    ap_set_input_repeat(AP_INPUT_REPEAT_DELAY, AP_INPUT_REPEAT_RATE);
    ap_flip_face_buttons(false);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: CPU & Fan
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Sub-screen: pick a CPU speed preset using the list widget */
static void demo__cpu_pick_speed(void) {
    static const struct { const char *label; ap_cpu_speed preset; } presets[] = {
        { "Menu        (~600-672 MHz)",    AP_CPU_SPEED_MENU        },
        { "Powersave   (~1200 MHz)",       AP_CPU_SPEED_POWERSAVE   },
        { "Normal      (~1608-1680 MHz)",  AP_CPU_SPEED_NORMAL      },
        { "Performance (~2000-2160 MHz)",  AP_CPU_SPEED_PERFORMANCE },
    };
    int count = (int)(sizeof(presets) / sizeof(presets[0]));

    ap_list_item items[4];
    for (int i = 0; i < count; i++)
        items[i] = (ap_list_item){ .label = presets[i].label };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "APPLY", .is_confirm = true },
    };
    ap_list_opts opts = ap_list_default_opts("CPU Speed", items, count);
    opts.footer        = footer;
    opts.footer_count  = 2;
    opts.initial_index = 2; /* Normal */

    ap_list_result result;
    if (ap_list(&opts, &result) == AP_OK && result.selected_index >= 0)
        ap_set_cpu_speed(presets[result.selected_index].preset);
}

#if defined(PLATFORM_TG5050)
static const char *demo__fan_mode_label(ap_fan_mode mode) {
    switch (mode) {
        case AP_FAN_MODE_MANUAL:           return "Manual";
        case AP_FAN_MODE_AUTO_QUIET:       return "Auto Quiet";
        case AP_FAN_MODE_AUTO_NORMAL:      return "Auto Normal";
        case AP_FAN_MODE_AUTO_PERFORMANCE: return "Auto Performance";
        default:                           return "N/A";
    }
}
#endif

/* Sub-screen: pick a fan mode / percentage using the list widget */
static void demo__cpu_pick_fan(void) {
    static const struct {
        const char *label;
        bool        auto_mode;
        ap_fan_mode mode;
        int         percent;
    } levels[] = {
        { "Performance", true,  AP_FAN_MODE_AUTO_PERFORMANCE, -1  },
        { "Normal",      true,  AP_FAN_MODE_AUTO_NORMAL,      -1  },
        { "Quiet",       true,  AP_FAN_MODE_AUTO_QUIET,       -1  },
        { "0%",          false, AP_FAN_MODE_MANUAL,            0  },
        { "10%",         false, AP_FAN_MODE_MANUAL,           10  },
        { "20%",         false, AP_FAN_MODE_MANUAL,           20  },
        { "30%",         false, AP_FAN_MODE_MANUAL,           30  },
        { "40%",         false, AP_FAN_MODE_MANUAL,           40  },
        { "50%",         false, AP_FAN_MODE_MANUAL,           50  },
        { "60%",         false, AP_FAN_MODE_MANUAL,           60  },
        { "70%",         false, AP_FAN_MODE_MANUAL,           70  },
        { "80%",         false, AP_FAN_MODE_MANUAL,           80  },
        { "90%",         false, AP_FAN_MODE_MANUAL,           90  },
        { "100%",        false, AP_FAN_MODE_MANUAL,          100  },
    };
    int count = (int)(sizeof(levels) / sizeof(levels[0]));

    ap_list_item items[14];
    for (int i = 0; i < count; i++)
        items[i] = (ap_list_item){ .label = levels[i].label };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "APPLY", .is_confirm = true },
    };
    ap_list_opts opts = ap_list_default_opts("Fan Speed", items, count);
    opts.footer       = footer;
    opts.footer_count = 2;

    ap_list_result result;
    if (ap_list(&opts, &result) == AP_OK && result.selected_index >= 0) {
        if (levels[result.selected_index].auto_mode)
            ap_set_fan_mode(levels[result.selected_index].mode);
        else
            ap_set_fan_speed(levels[result.selected_index].percent);
    }
}

/* Main CPU & Fan screen: live readout + action menu. Stays open until B. */
static void demo_cpu_fan(void) {
    TTF_Font *body_font  = ap_get_font(AP_FONT_LARGE);
    TTF_Font *hint_font  = ap_get_font(AP_FONT_SMALL);
    ap_color fg     = ap_get_theme()->text;
    ap_color accent = ap_get_theme()->accent;
    int pad = AP_DS(12);

    static const struct { const char *label; void (*fn)(void); } actions[] = {
        { "CPU Speed", demo__cpu_pick_speed },
        { "Fan Speed", demo__cpu_pick_fan   },
    };
    int action_count = (int)(sizeof(actions) / sizeof(actions[0]));

    int  sel     = 0;
    bool running = true;

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "OPEN", .is_confirm = true },
    };

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;
            switch (ev.button) {
                case AP_BTN_UP:   sel = (sel - 1 + action_count) % action_count; break;
                case AP_BTN_DOWN: sel = (sel + 1) % action_count;                break;
                case AP_BTN_A:    actions[sel].fn();                              break;
                case AP_BTN_B:    running = false;                                break;
                default: break;
            }
        }

        /* Read sensors each frame */
        int cpu_mhz  = ap_get_cpu_speed_mhz();
        int cpu_temp = ap_get_cpu_temp_celsius();

        ap_clear_screen();
        ap_draw_screen_title("CPU & Fan", NULL);
        SDL_Rect content_rect = ap_get_content_rect(true, true, false);
        int y = content_rect.y;

        /* ── Live readout ── */
        char row[64];
        if (cpu_mhz > 0) snprintf(row, sizeof(row), "%d MHz", cpu_mhz);
        else              snprintf(row, sizeof(row), "N/A");
        ap_draw_text(hint_font, "CPU speed:", pad, y, fg);
        ap_draw_text(body_font, row, pad + AP_DS(120), y, fg);
        y += AP_DS(22);

        if (cpu_temp > 0) snprintf(row, sizeof(row), "%d C", cpu_temp);
        else               snprintf(row, sizeof(row), "N/A");
        ap_draw_text(hint_font, "CPU temp:", pad, y, fg);
        ap_draw_text(body_font, row, pad + AP_DS(120), y, fg);
        y += AP_DS(22);

#if defined(PLATFORM_TG5050)
        ap_fan_mode fan_mode = ap_get_fan_mode();
        int fan_pct = ap_get_fan_speed();
        ap_draw_text(hint_font, "Fan mode:", pad, y, fg);
        ap_draw_text(body_font, demo__fan_mode_label(fan_mode), pad + AP_DS(120), y, fg);
        y += AP_DS(22);
        if (fan_pct >= 0) snprintf(row, sizeof(row), "%d%%", fan_pct);
        else               snprintf(row, sizeof(row), "N/A");
        ap_draw_text(hint_font, "Fan speed:", pad, y, fg);
        ap_draw_text(body_font, row, pad + AP_DS(120), y, fg);
#else
        ap_draw_text(hint_font, "Fan speed:", pad, y, fg);
        ap_draw_text(body_font, "no fan", pad + AP_DS(120), y, fg);
#endif
        y += AP_DS(30);

        /* ── Action menu ── */
        ap_draw_text(hint_font, "Actions:", pad, y, fg);
        y += AP_DS(20);
        for (int i = 0; i < action_count; i++) {
            ap_color col = (i == sel) ? accent : fg;
            char line[64];
            snprintf(line, sizeof(line), "%s %s", (i == sel) ? ">" : " ", actions[i].label);
            ap_draw_text(body_font, line, pad, y, col);
            y += AP_DS(22);
        }

        ap_draw_footer(footer, 2);
        ap_present();
        SDL_Delay(250); /* slow refresh — sensor reads go through sysfs */
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Color Picker
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_color_picker(void) {
    ap_color initial = { .r = 100, .g = 149, .b = 237, .a = 255 }; /* Cornflower blue */
    ap_color result;

    int rc = ap_color_picker(initial, &result);

    if (rc == AP_OK) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Picked: #%02X%02X%02X", result.r, result.g, result.b);
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Background Preview (list with per-item fullscreen background images)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_background_preview(void) {
    SDL_Texture *icon = ap_load_image("demo_icon.png");

    /* Save the current background path so we can restore it later */
    char prev_bg[512];
    strncpy(prev_bg, ap_get_theme()->bg_image_path, sizeof(prev_bg) - 1);
    prev_bg[sizeof(prev_bg) - 1] = '\0';

    ap_list_item items[] = {
        { .label = "Default",   .metadata = NULL },
        { .label = "Preview",   .metadata = "demo_icon.png", .background_image = icon },
        { .label = "Plain",     .metadata = NULL },
        { .label = "Preview 2", .metadata = "demo_icon.png", .background_image = icon },
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "SELECT", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Background Preview", items, count);
    opts.footer       = footer;
    opts.footer_count = 2;

    ap_list_result result;
    int rc = ap_list(&opts, &result);

    if (rc == AP_OK && result.selected_index >= 0) {
        /* Swap the global background to the selected image */
        const char *path = items[result.selected_index].metadata;
        int reload_rc = ap_reload_background(path);
        if (reload_rc == AP_OK) {
            char msg[640];
            snprintf(msg, sizeof(msg),
                     "Background changed to %s.\n"
                     "Press OK to restore the previous background.",
                     path ? path : "(default)");
            demo_show_message(msg);
        } else {
            demo_show_message("Failed to load background image.");
        }

        /* Restore the previous background */
        ap_reload_background(prev_bg[0] ? prev_bg : NULL);
    }

    if (icon) SDL_DestroyTexture(icon);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: List Navigation (page skip & letter skip)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_list_navigation(void) {
    ap_list_item items[] = {
        AP_LIST_ITEM("Afghanistan",      NULL),
        AP_LIST_ITEM("Albania",          NULL),
        AP_LIST_ITEM("Algeria",          NULL),
        AP_LIST_ITEM("Argentina",        NULL),
        AP_LIST_ITEM("Australia",        NULL),
        AP_LIST_ITEM("Austria",          NULL),
        AP_LIST_ITEM("Belgium",          NULL),
        AP_LIST_ITEM("Bolivia",          NULL),
        AP_LIST_ITEM("Brazil",           NULL),
        AP_LIST_ITEM("Bulgaria",         NULL),
        AP_LIST_ITEM("Cambodia",         NULL),
        AP_LIST_ITEM("Canada",           NULL),
        AP_LIST_ITEM("Chile",            NULL),
        AP_LIST_ITEM("China",            NULL),
        AP_LIST_ITEM("Colombia",         NULL),
        AP_LIST_ITEM("Croatia",          NULL),
        AP_LIST_ITEM("Cuba",             NULL),
        AP_LIST_ITEM("Denmark",          NULL),
        AP_LIST_ITEM("Dominican Republic", NULL),
        AP_LIST_ITEM("Ecuador",          NULL),
        AP_LIST_ITEM("Egypt",            NULL),
        AP_LIST_ITEM("Estonia",          NULL),
        AP_LIST_ITEM("Ethiopia",         NULL),
        AP_LIST_ITEM("Finland",          NULL),
        AP_LIST_ITEM("France",           NULL),
        AP_LIST_ITEM("Georgia",          NULL),
        AP_LIST_ITEM("Germany",          NULL),
        AP_LIST_ITEM("Greece",           NULL),
        AP_LIST_ITEM("Hungary",          NULL),
        AP_LIST_ITEM("Iceland",          NULL),
        AP_LIST_ITEM("India",            NULL),
        AP_LIST_ITEM("Indonesia",        NULL),
        AP_LIST_ITEM("Iran",             NULL),
        AP_LIST_ITEM("Iraq",             NULL),
        AP_LIST_ITEM("Ireland",          NULL),
        AP_LIST_ITEM("Israel",           NULL),
        AP_LIST_ITEM("Italy",            NULL),
        AP_LIST_ITEM("Jamaica",          NULL),
        AP_LIST_ITEM("Japan",            NULL),
        AP_LIST_ITEM("Jordan",           NULL),
        AP_LIST_ITEM("Kenya",            NULL),
        AP_LIST_ITEM("Kuwait",           NULL),
        AP_LIST_ITEM("Latvia",           NULL),
        AP_LIST_ITEM("Lebanon",          NULL),
        AP_LIST_ITEM("Lithuania",        NULL),
        AP_LIST_ITEM("Malaysia",         NULL),
        AP_LIST_ITEM("Mexico",           NULL),
        AP_LIST_ITEM("Morocco",          NULL),
        AP_LIST_ITEM("Nepal",            NULL),
        AP_LIST_ITEM("Netherlands",      NULL),
        AP_LIST_ITEM("New Zealand",      NULL),
        AP_LIST_ITEM("Nigeria",          NULL),
        AP_LIST_ITEM("Norway",           NULL),
        AP_LIST_ITEM("Oman",             NULL),
        AP_LIST_ITEM("Pakistan",         NULL),
        AP_LIST_ITEM("Panama",           NULL),
        AP_LIST_ITEM("Peru",             NULL),
        AP_LIST_ITEM("Philippines",      NULL),
        AP_LIST_ITEM("Poland",           NULL),
        AP_LIST_ITEM("Portugal",         NULL),
        AP_LIST_ITEM("Qatar",            NULL),
        AP_LIST_ITEM("Romania",          NULL),
        AP_LIST_ITEM("Russia",           NULL),
        AP_LIST_ITEM("Saudi Arabia",     NULL),
        AP_LIST_ITEM("Serbia",           NULL),
        AP_LIST_ITEM("Singapore",        NULL),
        AP_LIST_ITEM("Slovakia",         NULL),
        AP_LIST_ITEM("Slovenia",         NULL),
        AP_LIST_ITEM("South Africa",     NULL),
        AP_LIST_ITEM("South Korea",      NULL),
        AP_LIST_ITEM("Spain",            NULL),
        AP_LIST_ITEM("Sweden",           NULL),
        AP_LIST_ITEM("Switzerland",      NULL),
        AP_LIST_ITEM("Thailand",         NULL),
        AP_LIST_ITEM("Tunisia",          NULL),
        AP_LIST_ITEM("Turkey",           NULL),
        AP_LIST_ITEM("Ukraine",          NULL),
        AP_LIST_ITEM("United Kingdom",   NULL),
        AP_LIST_ITEM("United States",    NULL),
        AP_LIST_ITEM("Uruguay",          NULL),
        AP_LIST_ITEM("Venezuela",        NULL),
        AP_LIST_ITEM("Vietnam",          NULL),
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B,     .label = "BACK" },
        { .button = AP_BTN_L1,    .label = "LETTER", .button_text = "L1/R1" },
        { .button = AP_BTN_LEFT,  .label = "PAGE",   .button_text = "\xe2\x97\x80/\xe2\x96\xb6" },
        { .button = AP_BTN_A,     .label = "SELECT", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Countries", items, count);
    opts.footer       = footer;
    opts.footer_count = sizeof(footer) / sizeof(footer[0]);
    opts.help_text    = "L1/R1: Jump to previous/next letter.\n"
                        "D-Pad Left/Right: Skip one page.\n"
                        "D-Pad Up/Down: Move one item.\n"
                        "Menu: Show this help.";

    ap_list_result result;
    int rc = ap_list(&opts, &result);

    if (rc == AP_OK && result.selected_index >= 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Selected: %s", items[result.selected_index].label);
        demo_show_message(msg);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Queue Viewer
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Simulated queue items for the demo */
static const struct {
    const char *title;
    const char *subtitle;
    bool        fails;
} qv_items[] = {
    { "Looney Tunes",                                                     "Game Boy  [cht]",       false },
    { "Maru's Mission",                                                   "Game Boy  [cht]",       false },
    { "Mega Man - Dr. Wily's Revenge",                                    "Game Boy  [cht]",       false },
    { "Mega Man II",                                                      "Game Boy  [cht]",       true  },
    { "Metroid II - Return of Samus",                                     "Game Boy  [cht]",       false },
    { "Final Fantasy Adventure",                                          "Game Boy  [cht]",       false },
    { "Tetris",                                                           "",                     true  },
    { "Super Mario Land",                                                 "Game Boy  [cht]",       false },
    { "Kirby's Dream Land",                                               "Game Boy  [cht]",       false },
    { "Donkey Kong Land - This entry has a very long title to demonstrate horizontal text scrolling when selected", "Game Boy Color  [art]", false },
};
#define QV_ITEM_COUNT ((int)(sizeof(qv_items) / sizeof(qv_items[0])))

typedef struct {
    uint32_t start_ms;
    bool     cleared;
    bool     cancelled;
    uint32_t cancel_elapsed_ms;
} QVDemoCtx;

#define QV_ITEM_DURATION_MS 1500
#define QV_ITEM_STAGGER_MS  700

static int qv_snapshot(ap_queue_item *buf, int max, void *ud) {
    QVDemoCtx *ctx = (QVDemoCtx *)ud;
    if (ctx->cleared) return 0;

    int n = (QV_ITEM_COUNT < max) ? QV_ITEM_COUNT : max;
    uint32_t elapsed = SDL_GetTicks() - ctx->start_ms;
    uint32_t effective_elapsed = ctx->cancelled ? ctx->cancel_elapsed_ms : elapsed;
    for (int i = 0; i < n; i++) {
        uint32_t offset = (uint32_t)(i * QV_ITEM_STAGGER_MS);
        snprintf(buf[i].title, sizeof(buf[i].title), "%s", qv_items[i].title);
        snprintf(buf[i].subtitle, sizeof(buf[i].subtitle), "%s", qv_items[i].subtitle);
        if (effective_elapsed < offset) {
            buf[i].status = AP_QUEUE_PENDING;
            snprintf(buf[i].status_text, sizeof(buf[i].status_text), "%s", "QUEUED");
            buf[i].progress = -1.0f;
        } else if (effective_elapsed < offset + QV_ITEM_DURATION_MS) {
            buf[i].status = AP_QUEUE_RUNNING;
            snprintf(buf[i].status_text, sizeof(buf[i].status_text), "%s", "DOWNLOADING...");
            buf[i].progress = (float)(effective_elapsed - offset) / (float)QV_ITEM_DURATION_MS;
        } else {
            if (qv_items[i].fails) {
                buf[i].status = AP_QUEUE_FAILED;
                snprintf(buf[i].status_text, sizeof(buf[i].status_text), "%s", "ERROR");
                buf[i].progress = 0.4f;
            } else {
                buf[i].status = AP_QUEUE_DONE;
                snprintf(buf[i].status_text, sizeof(buf[i].status_text), "%s", "DONE");
                buf[i].progress = 1.0f;
            }
        }

        /* Example app-side cancellation handling: freeze the queue at the
         * cancel time and convert any unfinished work to "Cancelled". */
        if (ctx->cancelled &&
            (buf[i].status == AP_QUEUE_PENDING || buf[i].status == AP_QUEUE_RUNNING)) {
            buf[i].status = AP_QUEUE_SKIPPED;
            snprintf(buf[i].status_text, sizeof(buf[i].status_text), "%s", "CANCELLED");
            buf[i].progress = -1.0f;
        }

        buf[i].userdata = NULL;
    }
    return n;
}

static void qv_clear(void *ud) {
    QVDemoCtx *ctx = (QVDemoCtx *)ud;
    ctx->cleared = true;
    ctx->cancelled = false;
    ctx->cancel_elapsed_ms = 0;
}

static void qv_cancel(void *ud) {
    QVDemoCtx *ctx = (QVDemoCtx *)ud;
    if (ctx->cancelled) return;

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "NO",  .is_confirm = false },
        { .button = AP_BTN_A, .label = "YES", .is_confirm = true  },
    };
    ap_message_opts opts = {
        .message = "CANCEL ALL DOWNLOADS?\n\nRUNNING ITEMS WILL BE MARKED CANCELLED\nAND QUEUED ITEMS WILL BE SKIPPED.",
        .footer = footer,
        .footer_count = 2,
    };
    ap_confirm_result result;
    ap_confirmation(&opts, &result);

    if (result.confirmed) {
        ctx->cancelled = true;
        ctx->cancel_elapsed_ms = SDL_GetTicks() - ctx->start_ms;
    }
}

static void qv_detail(const ap_queue_item *item, void *ud) {
    (void)ud;
    char msg[512];
    const char *sname = item->status_text[0] ? item->status_text : "UNKNOWN";
    snprintf(msg, sizeof(msg), "%s\n%s\n\nSTATUS: %s",
             item->title, item->subtitle, sname);
    demo_show_message(msg);
}

static void demo_queue_viewer(void) {
    QVDemoCtx ctx = { .start_ms = SDL_GetTicks() };
    ap_queue_opts opts = {
        .title    = "DOWNLOADS",
        .snapshot = qv_snapshot,
        .userdata = &ctx,
        .on_detail = qv_detail,
        .on_cancel = qv_cancel,
        .on_clear  = qv_clear,
    };
    ap_queue_viewer(&opts);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Main menu
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_file_picker(void) {
    const char *mode_labels[] = { "File", "Folder", "Both + Hidden" };
    ap_file_picker_mode modes[] = {
        AP_FILE_PICKER_FILES, AP_FILE_PICKER_DIRS, AP_FILE_PICKER_BOTH
    };

    ap_list_item mode_items[] = {
        AP_LIST_ITEM("File",          NULL),
        AP_LIST_ITEM("Folder",        NULL),
        AP_LIST_ITEM("Both + Hidden", NULL),
    };

    ap_footer_item mode_footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "OPEN", .is_confirm = true },
    };

    int last_idx = 0;
    int last_vis = 0;

    while (1) {
        ap_list_opts mopts = ap_list_default_opts("Picker Mode", mode_items, 3);
        mopts.footer = mode_footer;
        mopts.footer_count = 2;
        mopts.initial_index = last_idx;
        mopts.visible_start_index = last_vis;

        ap_list_result mresult;
        int mrc = ap_list(&mopts, &mresult);
        last_vis = mresult.visible_start_index;
        if (mrc != AP_OK || mresult.action == AP_ACTION_BACK) break;
        if (mresult.selected_index < 0 || mresult.selected_index >= 3) continue;
        last_idx = mresult.selected_index;

        ap_file_picker_opts fp = ap_file_picker_default_opts(mode_labels[mresult.selected_index]);
        fp.mode = modes[mresult.selected_index];
        fp.allow_create = (fp.mode != AP_FILE_PICKER_FILES);
        fp.show_hidden = (fp.mode == AP_FILE_PICKER_BOTH);
        if (fp.mode == AP_FILE_PICKER_DIRS || fp.mode == AP_FILE_PICKER_BOTH)
            fp.title = NULL;

        /* Demonstrate extension filter for file-only mode */
        const char *exts[] = { "zip", "7z", "rar", "txt", "json", "png", "jpg" };
        if (fp.mode == AP_FILE_PICKER_FILES) {
            fp.extensions = exts;
            fp.extension_count = 7;
        }

        ap_file_picker_result fp_result;
        int rc = ap_file_picker(&fp, &fp_result);

        if (rc == AP_OK) {
            char msg[1100];
            snprintf(msg, sizeof(msg), "Selected:\n%s\n\nIs directory: %s",
                     fp_result.path, fp_result.is_directory ? "Yes" : "No");
            demo_show_message(msg);
        }
    }
}

typedef void (*demo_fn)(void);

static const struct {
    const char *label;
    demo_fn     fn;
} demos[] = {
    { "Basic List",          demo_list                },
    { "List (No Scrollbar)", demo_list_no_scrollbar   },
    { "Live Footer",         demo_live_footer         },
    { "List Navigation",     demo_list_navigation     },
    { "Image List",          demo_image_list          },
    { "Multi-Select List",   demo_multi_select        },
    { "Reorderable List",    demo_reorder             },
    { "Options List",        demo_options_list        },
    { "Keyboard",            demo_keyboard            },
    { "Confirmation",        demo_confirmation        },
    { "Selection",           demo_selection           },
    { "Process Message",     demo_process             },
    { "Advanced Process",    demo_process_advanced    },
    { "Detail Screen",       demo_detail              },
    { "Detail Screen (Styled)", demo_detail_styled    },
    { "Detail Screen (Fonts)", demo_detail_custom_fonts },
    { "Color Picker",        demo_color_picker        },
    { "Core API Lab",        demo_core_api_lab        },
    { "Drawing Primitives",  demo_drawing_primitives  },
    { "Screen Fade",         demo_screen_fade         },
    { "Help Overlay",        demo_help_overlay_screen },
    { "Input & Theme",       demo_input_theme         },
    { "CPU & Fan",           demo_cpu_fan             },
    { "Background Preview",  demo_background_preview  },
    { "Queue Viewer",        demo_queue_viewer        },
    { "File Picker",         demo_file_picker         },
};

#define DEMO_COUNT (int)(sizeof(demos) / sizeof(demos[0]))

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    ap_config cfg = {
        .window_title = "Apostrophe Widget Demo",
        .log_path     = ap_resolve_log_path("demo"),
        .is_nextui    = true,
    };
    if (ap_init(&cfg) != AP_OK) {
        fprintf(stderr, "Failed to initialise Apostrophe\n");
        return 1;
    }
    ap_log("demo: startup");

    int last_index = 0;
    int last_visible_start = 0;

    while (1) {
        /* Build menu items from demo list */
        ap_list_item items[DEMO_COUNT];
        for (int i = 0; i < DEMO_COUNT; i++) {
            items[i] = (ap_list_item){ .label = demos[i].label };
        }

        ap_footer_item footer[] = {
            { .button = AP_BTN_B, .label = "QUIT" },
            { .button = AP_BTN_A, .label = "OPEN", .is_confirm = true },
        };

        ap_list_opts opts = ap_list_default_opts("Widget Demo", items, DEMO_COUNT);
        opts.footer        = footer;
        opts.footer_count  = 2;
        opts.initial_index = last_index;
        opts.visible_start_index = last_visible_start;

        ap_list_result result;
        int rc = ap_list(&opts, &result);
        last_visible_start = result.visible_start_index;

        if (rc != AP_OK || result.action == AP_ACTION_BACK) {
            break; /* Quit */
        }

        if (result.selected_index >= 0 && result.selected_index < DEMO_COUNT) {
            last_index = result.selected_index;
            demos[result.selected_index].fn();
        }
    }

    ap_quit();
    return 0;
}
