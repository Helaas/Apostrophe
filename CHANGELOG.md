# Changelog

All notable changes to Apostrophe will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- **Keyboard input field scrolling** (`apostrophe_widgets.h`): Text in the keyboard input field now scrolls horizontally to keep the caret always visible when text exceeds the field width. Works for all keyboard types (general, URL, numeric). Demo entry for long text scrolling.
- **Vsync-aware frame pacing** (`apostrophe.h`): `ap_present()` now detects whether the renderer actually has vsync and only adds a ~60 fps software delay when vsync is missing. This prevents CPU hot-spinning on software-fallback renderers and brings present-loop behavior closer to Gabagool during process-message screens.
- **Confirmation message layout improvements** (#20): vertical centering for confirmation messages, support for long wrapped text, and clamping for narrow/overflowing dialogs.
- **macOS resolution handling** (#19): improved resolution detection and device metrics for macOS development builds.
- **Custom footer button text** (#17, #18): `button_text` field on `ap_footer_item` for custom pill labels. Bundled font license file. OFL end marker fix, codepoint function rename, uppercase footer labels.
- **Honor A-confirm in options list** (#16): pressing A now confirms the current selection for keyboard and color picker option items.
- **Ellipsized text rendering** (#15): `ap_draw_text_ellipsized()` for drawing text truncated with an ellipsis. Fixes for buffer allocation and non-negative label width in options list.
- **WiFi signal strength caching** (#14): RSSI reads are now cached with a 5-second TTL to stabilize the status bar display and reduce subprocess overhead.
- **Font override options** (#13): `title_font` / `item_font` overrides for list and detail widgets.
- **Font bump** (#12): automatic font size bump based on logical screen resolution. `ap_get_font_bump()` accessor and `disable_font_bump` config option.
- **Detail screen styling options** (#11): `center_title`, `show_section_separator`, `key_color` on `ap_detail_opts`. `ap_draw_screen_title_centered()` core drawing function. Demo entry for styled detail screen.
- **`AP_LIST_ITEM` / `AP_LIST_ITEM_BG` helper macros** (#9): convenience macros and designated-initializer guidance for `ap_list_item`.
- **Background reload and list background previews**: `ap_reload_background()` support and background preview in list widget demo (#10).
- **List page & letter skip navigation**: D-Pad Left/Right skip by one page in `ap_list`. L1/R1 jump between alphabetical letter groups. Help overlay moved from L1 to Menu button; when both `help_text` and hidden footer items exist, Menu shows help first then footer overflow sequentially. Demo entry for navigation features.
- **`ap_show_footer_overflow()`**: Public API to programmatically open the hidden-actions overlay. Useful for screens with custom input loops that handle Menu independently of `ap_list`.

### Fixed

- **Demo background preview** (#10): use selected item metadata and guard on reload return value.

## [v0.0.1] - 2026-03-10

### Added

- **Core library** (`apostrophe.h`): init/quit lifecycle, SDL2 window/renderer management, input abstraction (virtual buttons, combos), drawing primitives (text, shapes, pills, images), theming (NextUI integration), font system (6 tiers matching NextUI), status bar, footer, scrollbar, progress bar, screen fade, texture cache, text scrolling, CPU/fan control, power button handling, logging
- **Widget library** (`apostrophe_widgets.h`): list (single/multi-select, reorder, images), options list (cycle/keyboard/clickable/color picker), keyboard (general/URL/numeric), confirmation dialog, selection dialog, process message (async worker with progress), download manager (multi-threaded, requires libcurl), detail screen (info/description/image/table sections), color picker, help overlay
- **Platform support**: TrimUI Smart Pro (`tg5040`), TrimUI Smart Brick (`tg5040`/`tg3040`), TrimUI Smart Pro S (`tg5050`), Miyoo Flip (`my355`), macOS (development)
- **Build system**: GNU Make with Docker-based cross-compilation for all device platforms
- **Examples**: hello (minimal), demo (comprehensive widget showcase), download (status bar + download manager), combo (chord/sequence input), perf (CPU/fan control)
- **Documentation**: Getting Started guide, API reference, Widget catalog, Porting from Gabagool guide, Gabagool parity matrix
