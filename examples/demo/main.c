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
        { .button = AP_BTN_B,     .label = "BACK" },
        { .button = AP_BTN_START, .label = "SAVE", .is_confirm = true },
    };

    ap_options_list_opts opts = {
        .title        = "Settings",
        .items        = settings,
        .item_count   = count,
        .footer       = footer,
        .footer_count = 2,
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

        if (rc == AP_OK && result.action == AP_ACTION_SELECTED && result.focused_index == 3) {
            /* "About" was clicked — show detail screen */
            demo_detail();
            continue;
        }
        break;
    }
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
        { "General",  "Enter your name:",  "Hello",              AP_KB_GENERAL },
        { "URL",      "Enter a URL:",      "https://",           AP_KB_URL     },
        { "Numeric",  "Enter a number:",   "42",                 AP_KB_NUMERIC },
    };
    int mode_count = sizeof(modes) / sizeof(modes[0]);

    ap_list_item items[3];
    for (int i = 0; i < mode_count; i++)
        items[i] = (ap_list_item){ .label = modes[i].label };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "OPEN", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Keyboard Mode", items, mode_count);
    opts.footer       = footer;
    opts.footer_count = 2;

    ap_list_result lr;
    int rc = ap_list(&opts, &lr);

    if (rc == AP_OK && lr.selected_index >= 0 && lr.selected_index < mode_count) {
        int idx = lr.selected_index;
        ap_keyboard_result result;
        rc = ap_keyboard(modes[idx].initial, modes[idx].prompt, modes[idx].layout, &result);

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
        { .button = AP_BTN_B, .label = "CANCEL" },
        { .button = AP_BTN_A, .label = "CHOOSE", .is_confirm = true },
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
        { .key = "Name",             .value = "Apostrophe" },
        { .key = "Version",          .value = "1.0.0" },
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

    ap_detail_section sections[] = {
        {
            .type       = AP_SECTION_INFO,
            .title      = "Project Info",
            .info_pairs = info,
            .info_count = (int)(sizeof(info) / sizeof(info[0])),
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
    TTF_Font *title_font = ap_get_font(AP_FONT_EXTRA_LARGE);
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
        int y = pad;
        ap_draw_text(title_font, "CPU & Fan", pad, y, fg);
        y += AP_DS(30);

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
    { "CPU & Fan",           demo_cpu_fan       },
};

#define DEMO_COUNT (int)(sizeof(demos) / sizeof(demos[0]))

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    ap_config cfg = {
        .window_title = "Apostrophe Widget Demo",
        .log_path     = ap_resolve_log_path("demo"),
        .is_nextui    = AP_PLATFORM_IS_DEVICE,
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
