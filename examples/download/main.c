/*
 * Apostrophe Download Manager & Status Bar Demo
 *
 * Demonstrates:
 *   - Status bar with clock, battery, and wifi indicators
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
static ap_status_bar_opts status_bar = {
    /* show_clock not set → AP_CLOCK_AUTO: follows NextUI showclock/clock24h settings */
    .show_battery = true,
    .show_wifi    = true,
};

static ap_status_bar_opts wifi_only_status_bar = {
    .show_clock   = AP_CLOCK_HIDE,
    .show_battery = false,
    .show_wifi    = true,
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
            .dest_path = "apostrophe_demo_dummy.pdf",
            .label     = "dummy.pdf",
        },
        {
            .url       = "https://filesamples.com/samples/document/txt/sample1.txt",
            .dest_path = "apostrophe_demo_sample1.txt",
            .label     = "sample1.txt",
        },
        {
            .url       = "https://filesamples.com/samples/document/txt/sample2.txt",
            .dest_path = "apostrophe_demo_sample2.txt",
            .label     = "sample2.txt",
        },
        {
            .url       = "https://filesamples.com/samples/document/txt/sample3.txt",
            .dest_path = "apostrophe_demo_sample3.txt",
            .label     = "sample3.txt",
        },
    };
    int count = sizeof(downloads) / sizeof(downloads[0]);

    const char *headers[] = { "X-Demo-Header: Apostrophe/1.0", "Accept: */*" };
    ap_download_opts opts = {
        .max_concurrent  = 2,
        .skip_ssl_verify = false,
        .headers         = headers,
        .header_count    = 2,
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
        .message      = "Download Manager is unavailable in this build.\n\n"
                        "Enable bundled curl support and rebuild:\n"
                        "  make <platform>-download USE_BUNDLED_CURL=1",
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
static void demo_status_bar_list_with_config(const char *title, ap_status_bar_opts *bar) {
    ap_list_item items[] = {
        { .label = "Item One"   },
        { .label = "Item Two"   },
        { .label = "Item Three" },
        { .label = "Item Four"  },
        { .label = "Item Five"  },
    };
    int count = sizeof(items) / sizeof(items[0]);

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "SELECT", .is_confirm = true },
    };

    ap_list_opts opts = ap_list_default_opts(title, items, count);
    opts.footer       = footer;
    opts.footer_count = 2;
    opts.status_bar   = bar;

    ap_list_result result;
    ap_list(&opts, &result);
}

static void demo_status_bar_list(void) {
    demo_status_bar_list_with_config("Status Bar Demo", &status_bar);
}

static void demo_status_bar_wifi_only(void) {
    demo_status_bar_list_with_config("WiFi Only Status", &wifi_only_status_bar);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Demo: Status bar with detail screen
 * ═══════════════════════════════════════════════════════════════════════════ */
static void demo_status_bar_detail(void) {
    ap_detail_info_pair info[] = {
        { .key = "Widget",   .value = "Status Bar"      },
        { .key = "Clock",    .value = "24-hour format"   },
        { .key = "Indicators", .value = "Battery + WiFi" },
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
                           "(12h or 24h), plus battery and wifi indicators from the "
                           "active platform theme assets. Any screen "
                           "that accepts a status_bar pointer will render it automatically.",
        },
    };

    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
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
    { "Start Downloads",         demo_downloads             },
    { "List with Status Bar",    demo_status_bar_list       },
    { "List with WiFi-Only Bar", demo_status_bar_wifi_only  },
    { "Detail + Status Bar",     demo_status_bar_detail     },
};

#define DEMO_COUNT (int)(sizeof(demos) / sizeof(demos[0]))

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    ap_config cfg = {
        .window_title = "Download & Status Bar Demo",
        .log_path     = ap_resolve_log_path("download"),
        .is_nextui    = true,
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
            { .button = AP_BTN_B, .label = "QUIT" },
            { .button = AP_BTN_A, .label = "OPEN", .is_confirm = true },
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
