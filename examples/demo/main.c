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
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "Back" },
        { .button = AP_BTN_A, .label = "Select", .is_confirm = true },
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
        { .button = AP_BTN_B, .label = "Back" },
        { .button = AP_BTN_A, .label = "Toggle" },
        { .button = AP_BTN_START, .label = "Done", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Pick Fruits", items, count);
    opts.multi_select  = true;
    opts.footer        = footer;
    opts.footer_count  = 3;

    ap_list_result result;
    ap_list(&opts, &result);
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
        { .button = AP_BTN_B,     .label = "Back" },
        { .button = AP_BTN_X,     .label = "Reorder" },
        { .button = AP_BTN_START, .label = "Done", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Reorder Items", items, count);
    opts.reorder_button = AP_BTN_X;
    opts.footer         = footer;
    opts.footer_count   = 3;

    ap_list_result result;
    ap_list(&opts, &result);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Options list (settings-style)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_options_list(void) {
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
            .label           = "Name",
            .type            = AP_OPT_KEYBOARD,
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
    };
    int count = sizeof(settings) / sizeof(settings[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B,     .label = "Back" },
        { .button = AP_BTN_START, .label = "Save", .is_confirm = true },
    };

    ap_options_list_opts opts = {
        .title        = "Settings",
        .items        = settings,
        .item_count   = count,
        .footer       = footer,
        .footer_count = 2,
    };

    ap_options_list_result result;
    ap_options_list(&opts, &result);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Keyboard
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_keyboard(void) {
    ap_keyboard_result result;
    int rc = ap_keyboard("Hello", "Enter your name:", AP_KB_GENERAL, &result);

    if (rc == AP_OK) {
        char msg[1100];
        snprintf(msg, sizeof(msg), "You typed:\n%s", result.text);
        ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
        ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
        ap_confirm_result cr;
        ap_confirmation(&m, &cr);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Confirmation
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_confirmation(void) {
    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "No" },
        { .button = AP_BTN_A, .label = "Yes", .is_confirm = true },
    };

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
        { .button = AP_BTN_B, .label = "Cancel" },
        { .button = AP_BTN_A, .label = "Choose", .is_confirm = true },
    };

    ap_selection_result result;
    int rc = ap_selection("Choose difficulty:", opts, 3, footer, 2, &result);

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
        { .key = "Name",     .value = "Apostrophe" },
        { .key = "Version",  .value = "1.0.0"      },
        { .key = "Author",   .value = "Helaas"  },
        { .key = "License",  .value = "MIT"         },
    };

    ap_detail_section sections[] = {
        {
            .type       = AP_SECTION_INFO,
            .title      = "Info",
            .info_pairs = info,
            .info_count = 4,
        },
        {
            .type        = AP_SECTION_DESCRIPTION,
            .title       = "About",
            .description = "Apostrophe is a header-only C UI toolkit for building "
                           "graphical tools on retro gaming handhelds running NextUI. "
                           "It provides a comprehensive set of pre-built widgets "
                           "including lists, keyboards, settings panels, and more.",
        },
    };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "Back" },
    };

    ap_detail_opts opts = {
        .title         = "About Apostrophe",
        .sections      = sections,
        .section_count = 2,
        .footer        = footer,
        .footer_count  = 1,
    };

    ap_detail_result result;
    ap_detail_screen(&opts, &result);
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
 *  Main menu
 * ═══════════════════════════════════════════════════════════════════════════ */
typedef void (*demo_fn)(void);

static const struct {
    const char *label;
    demo_fn     fn;
} demos[] = {
    { "Basic List",          demo_list          },
    { "Multi-Select List",   demo_multi_select  },
    { "Reorderable List",    demo_reorder       },
    { "Options List",        demo_options_list  },
    { "Keyboard",            demo_keyboard      },
    { "Confirmation",        demo_confirmation  },
    { "Selection",           demo_selection      },
    { "Process Message",     demo_process       },
    { "Detail Screen",       demo_detail        },
    { "Color Picker",        demo_color_picker  },
};

#define DEMO_COUNT (int)(sizeof(demos) / sizeof(demos[0]))

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    ap_config cfg = {
        .window_title = "Apostrophe Widget Demo",
        .font_path    = "font.ttf",
        .is_nextui    = AP_PLATFORM_IS_DEVICE,
    };
    if (ap_init(&cfg) != AP_OK) {
        fprintf(stderr, "Failed to initialise Apostrophe\n");
        return 1;
    }

    int last_index = 0;

    while (1) {
        /* Build menu items from demo list */
        ap_list_item items[DEMO_COUNT];
        for (int i = 0; i < DEMO_COUNT; i++) {
            items[i] = (ap_list_item){ .label = demos[i].label };
        }

        ap_footer_item footer[] = {
            { .button = AP_BTN_B, .label = "Quit" },
            { .button = AP_BTN_A, .label = "Open", .is_confirm = true },
        };

        ap_list_opts opts = ap_list_default_opts("Widget Demo", items, DEMO_COUNT);
        opts.footer        = footer;
        opts.footer_count  = 2;
        opts.initial_index = last_index;

        ap_list_result result;
        int rc = ap_list(&opts, &result);

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
