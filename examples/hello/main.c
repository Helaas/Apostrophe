/*
 * Apostrophe Hello World
 *
 * Minimal example: displays a list with a few items, lets the user select
 * one, and shows a confirmation dialog with the result.
 */

#define AP_IMPLEMENTATION
#include "apostrophe.h"
#define AP_WIDGETS_IMPLEMENTATION
#include "apostrophe_widgets.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    /* Initialise Apostrophe — auto-detects screen size & loads theme */
    ap_config cfg = {
        .window_title = "Hello Apostrophe",
        .log_path     = ap_resolve_log_path("hello"),
        .is_nextui    = AP_PLATFORM_IS_DEVICE,
    };
    if (ap_init(&cfg) != AP_OK) {
        fprintf(stderr, "Failed to initialise Apostrophe\n");
        return 1;
    }
    ap_log("hello: startup");

    /* Build a simple list of items */
    ap_list_item items[] = {
        { .label = "Say Hello",       .metadata = NULL },
        { .label = "Greet the World", .metadata = NULL },
        { .label = "Wave Goodbye",    .metadata = NULL },
    };
    int item_count = sizeof(items) / sizeof(items[0]);

    /* Define footer hints */
    ap_footer_item footer[] = {
        { .button = AP_BTN_B, .label = "BACK" },
        { .button = AP_BTN_A, .label = "SELECT", .is_confirm = true },
    };

    /* Configure and show the list */
    ap_list_opts opts = ap_list_default_opts("Hello!", items, item_count);
    opts.footer       = footer;
    opts.footer_count = 2;

    ap_list_result result;
    int rc = ap_list(&opts, &result);

    if (rc == AP_OK && result.selected_index >= 0) {
        /* Show a confirmation with the selected item */
        char msg[128];
        snprintf(msg, sizeof(msg), "You picked: %s", items[result.selected_index].label);

        ap_footer_item conf_footer[] = {
            { .button = AP_BTN_A, .label = "OK", .is_confirm = true },
        };
        ap_message_opts conf = {
            .message      = msg,
            .image_path   = NULL,
            .footer       = conf_footer,
            .footer_count = 1,
        };
        ap_confirm_result conf_result;
        ap_confirmation(&conf, &conf_result);
    }

    ap_quit();
    return 0;
}
