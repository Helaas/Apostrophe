/*
 * Apostrophe Download Manager & Status Bar Demo
 *
 * Demonstrates:
 *   - Status bar with clock and custom icons
 *   - Download manager with progress UI (requires libcurl)
 *   - Conditional compilation with AP_ENABLE_CURL
 */

#define AP_IMPLEMENTATION
#include "apostrophe.h"
#define AP_WIDGETS_IMPLEMENTATION
#include "apostrophe_widgets.h"

/* ═══════════════════════════════════════════════════════════════════════════
 *  Status bar configuration (shared across screens)
 * ═══════════════════════════════════════════════════════════════════════════ */
static const char *status_icons[] = { "\xe2\x87\xa9", "\xe2\x98\x85" }; /* ⇩ ★ */

static ap_status_bar_opts status_bar = {
    .show_clock  = true,
    .use_24h     = true,
    .icons       = status_icons,
    .icon_count  = 2,
};

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Download Manager
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_downloads(void) {
#ifdef AP_ENABLE_CURL
    /* Sample public-domain files of varying sizes */
    ap_download downloads[] = {
        {
            .url       = "https://www.w3.org/WAI/ER/tests/xhtml/testfiles/resources/pdf/dummy.pdf",
            .dest_path = "/tmp/apostrophe_demo_dummy.pdf",
            .label     = "dummy.pdf",
        },
        {
            .url       = "https://filesamples.com/samples/document/txt/sample1.txt",
            .dest_path = "/tmp/apostrophe_demo_sample1.txt",
            .label     = "sample1.txt",
        },
        {
            .url       = "https://filesamples.com/samples/document/txt/sample2.txt",
            .dest_path = "/tmp/apostrophe_demo_sample2.txt",
            .label     = "sample2.txt",
        },
        {
            .url       = "https://filesamples.com/samples/document/txt/sample3.txt",
            .dest_path = "/tmp/apostrophe_demo_sample3.txt",
            .label     = "sample3.txt",
        },
    };
    int count = sizeof(downloads) / sizeof(downloads[0]);

    ap_download_opts opts = {
        .max_concurrent  = 2,
        .skip_ssl_verify = false,
        .headers         = NULL,
        .header_count    = 0,
    };

    ap_download_result result;
    int rc = ap_download_manager(downloads, count, &opts, &result);

    /* Show result summary */
    char msg[256];
    if (rc == AP_CANCELLED) {
        snprintf(msg, sizeof(msg), "Downloads cancelled.\n%d/%d completed before cancel.",
                 result.completed, result.total);
    } else if (rc == AP_ERROR) {
        snprintf(msg, sizeof(msg), "Downloads failed.\n%d completed, %d failed.",
                 result.completed, result.failed);
    } else {
        snprintf(msg, sizeof(msg), "All %d downloads completed successfully!", result.total);
    }

    ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
    ap_message_opts m = { .message = msg, .footer = ok_foot, .footer_count = 1 };
    ap_confirm_result cr;
    ap_confirmation(&m, &cr);

#else
    /* libcurl not available — show informational message */
    ap_footer_item ok_foot[] = {{ .button = AP_BTN_A, .label = "OK", .is_confirm = true }};
    ap_message_opts m = {
        .message      = "Download Manager requires libcurl.\n\n"
                        "Rebuild with:\n  make mac CURL=1\n\n"
                        "or install libcurl:\n  brew install curl",
        .footer       = ok_foot,
        .footer_count = 1,
    };
    ap_confirm_result cr;
    ap_confirmation(&m, &cr);
#endif
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Status bar on a list
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_status_bar_list(void) {
    ap_list_item items[] = {
        { .label = "Item One"   },
        { .label = "Item Two"   },
        { .label = "Item Three" },
        { .label = "Item Four"  },
        { .label = "Item Five"  },
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "Back" },
        { .button = AP_BTN_A, .label = "Select", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts("Status Bar Demo", items, count);
    opts.footer       = footer;
    opts.footer_count = 2;
    opts.status_bar   = &status_bar;

    ap_list_result result;
    ap_list(&opts, &result);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Status bar with detail screen
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_status_bar_detail(void) {
    ap_detail_info_pair info[] = {
        { .key = "Widget",   .value = "Status Bar"      },
        { .key = "Clock",    .value = "24-hour format"   },
        { .key = "Icons",    .value = "2 custom icons"   },
    };

    ap_detail_section sections[] = {
        {
            .type       = AP_SECTION_INFO,
            .title      = "Configuration",
            .info_pairs = info,
            .info_count = 3,
        },
        {
            .type        = AP_SECTION_DESCRIPTION,
            .title       = "About",
            .description = "The status bar is rendered in the top-right corner "
                           "as a pill with accent background. It supports a clock "
                           "(12h or 24h) and custom Unicode icon strings. Any screen "
                           "that accepts a status_bar pointer will render it automatically.",
        },
    };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "Back" },
    };

    ap_detail_opts opts = {
        .title         = "Status Bar Info",
        .sections      = sections,
        .section_count = 2,
        .footer        = footer,
        .footer_count  = 1,
        .status_bar    = &status_bar,
    };

    ap_detail_result result;
    ap_detail_screen(&opts, &result);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Main menu
 * ═══════════════════════════════════════════════════════════════════════════ */
typedef void (*demo_fn)(void);

static const struct {
    const char *label;
    demo_fn     fn;
} demos[] = {
    { "Start Downloads",       demo_downloads        },
    { "List with Status Bar",  demo_status_bar_list   },
    { "Detail + Status Bar",   demo_status_bar_detail },
};

#define DEMO_COUNT (int)(sizeof(demos) / sizeof(demos[0]))

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    ap_config cfg = {
        .window_title = "Download & Status Bar Demo",
        .font_path    = "font.ttf",
        .log_path     = ap_resolve_log_path("download"),
        .is_nextui    = AP_PLATFORM_IS_DEVICE,
    };
    if (ap_init(&cfg) != AP_OK) {
        fprintf(stderr, "Failed to initialise Apostrophe\n");
        return 1;
    }
    ap_log("download: startup");

    int last_index = 0;
    int last_visible_start = 0;

    while (1) {
        ap_list_item items[DEMO_COUNT];
        for (int i = 0; i < DEMO_COUNT; i++) {
            items[i] = (ap_list_item){ .label = demos[i].label };
        }

        ap_footer_item footer[] = {
            { .button = AP_BTN_B, .label = "Quit" },
            { .button = AP_BTN_A, .label = "Open", .is_confirm = true },
        };

        ap_list_opts opts = ap_list_default_opts("Downloads", items, DEMO_COUNT);
        opts.footer        = footer;
        opts.footer_count  = 2;
        opts.initial_index = last_index;
        opts.visible_start_index = last_visible_start;
        opts.status_bar    = &status_bar;

        ap_list_result result;
        int rc = ap_list(&opts, &result);
        last_visible_start = result.visible_start_index;

        if (rc != AP_OK || result.action == AP_ACTION_BACK) {
            break;
        }

        if (result.selected_index >= 0 && result.selected_index < DEMO_COUNT) {
            last_index = result.selected_index;
            demos[result.selected_index].fn();
        }
    }

    ap_quit();
    return 0;
}
