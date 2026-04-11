# Changelog

All notable changes to Apostrophe will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- **File Picker widget** (`apostrophe_widgets.h`): new `ap_file_picker` widget for browsing the filesystem and selecting files or directories. Features: configurable mode (files only, directories only, or both), sorted directory listing with folders first, visual folder/file differentiation via trailing chevron `>` and uppercase extension labels, inline folder creation in dir-capable modes via `ap_keyboard()`, extension filtering, hidden-file option, and enforced rooted browsing (`SDCARD_PATH` on device, `$HOME` by default on desktop, or a caller-provided `root_path`). Demo entries added for all three modes.
- **Live footer demo** (`examples/demo/main.c`, `docs/DEMO_COVERAGE.md`): added a `Live Footer` list demo that simulates preview state, updates the `Y` footer label while the cursor moves, and uses `ap_request_frame_in(100)` so the label reverts automatically when preview expires.

### Changed

- **List live footer callback** (`apostrophe_widgets.h`, `docs/API.md`, `docs/WIDGETS.md`): `ap_list_opts` now includes an optional `footer_update` callback plus userdata. The callback runs after cursor/scroll settle and before footer draw so callers can update existing footer labels or `button_text` in place.
- **File picker mixed-mode footer label** (`apostrophe_widgets.h`, `docs/API.md`, `docs/WIDGETS.md`, `docs/DEMO_COVERAGE.md`): `ap_file_picker()` now uses the live footer hook in `AP_FILE_PICKER_BOTH`, switching the `A` hint between `ENTER` for directories and `OPEN` for files as focus moves.
- **Options list long-value truncation** (`apostrophe_widgets.h`, `docs/API.md`, `docs/WIDGETS.md`): `ap_options_list()` now applies the same width budgeting to focused and unfocused rows, ellipsizes long right-side values instead of letting them overlap labels, and keeps the clickable-row chevron `>` visible in both states.
- **Options list demo regression coverage** (`examples/demo/main.c`, `docs/DEMO_COVERAGE.md`): the `Options List` demo now includes long clickable path/URL rows so 640x480 layouts exercise both long-value truncation and long label+value splitting.
- **Selection wrapped-message layout** (`apostrophe_widgets.h`, `examples/demo/main.c`, `docs/DEMO_COVERAGE.md`): `ap_selection()` now places its option pills below the full wrapped message height instead of assuming a single text line, preventing overlap on narrow layouts and adding demo coverage for the wrapped-prompt case.

## [v1.1.0] - 2026-03-30

### Added

- **Desktop NextUI preview cache** (`Makefile`, `README.md`, `docs/GETTING_STARTED.md`): added `make setup-nextui-preview-cache` and `make clean-nextui-preview-cache` to populate a local `.cache/nextui-preview/` via sparse checkout from `https://github.com/LoveRetro/NextUI.git` pinned to `7d201cf293f3a253e09749b8bb002e0b9f66d652`. The cache generates local `nextval.json` and `minuisettings.txt` fixtures for desktop preview use without bundling upstream GPL sprite assets.

### Changed

- **Footer overflow chord disabled by default** (`apostrophe.h`): the L1+R1 shoulder button chord no longer opens the hidden-actions overlay, and there is no default Menu→overflow binding in core. Apps and widgets that draw footers may wire the Menu button to `ap_show_footer_overflow()` (and/or a custom chord via `ap_set_footer_overflow_opts()`), but this is opt-in per screen.
- **Desktop demo NextUI parity** (`Makefile`, `examples/demo/main.c`, `examples/download/main.c`): `run-mac-demo` and `run-mac-download` now auto-wire `AP_STATUS_ASSETS_DIR`, `AP_NEXTVAL_PATH`, `AP_MINUI_SETTINGS_PATH`, and preview battery/wifi defaults from `.cache/nextui-preview/` when no overrides are provided. The demo and download examples now opt into NextUI styling on desktop as well as on device.
- **Queue Viewer filter labels** (`apostrophe_widgets.h`): `ap_queue_opts` now accepts an optional `filter_labels[4]` override so downstream apps can rename the built-in queue filters without forking the widget. Unset entries still fall back to `ALL`, `IN PROGRESS`, `DONE`, and `FAILED`.
- **Queue viewer navigation parity** (`apostrophe_widgets.h`, `docs/API.md`, `docs/WIDGETS.md`): `ap_queue_viewer()` now matches `ap_list()` navigation semantics. D-Pad Left/Right still page by visible rows, while `L1/R1` jump between first-letter groups inside the active filter.
- **Single-icon status bar pills** (`apostrophe.h`, `examples/download/main.c`, `docs/API.md`): status bars now keep width calculation and drawing aligned when only one battery/wifi icon is visible with the clock hidden. Sprite-backed status bars render that case as a centered square pill, and the download demo includes a wifi-only repro screen.
- **Download manager scrollbar sizing and alignment** (`apostrophe_widgets.h`): `ap_download_manager()` now feeds row counts into `ap_draw_scrollbar()`, clamps scroll after row-height changes, and limits the scrollbar track to the rendered row stack so the thumb size and track height match the visible-item model instead of inflating on completion and speed-toggle screens.

## [v1.0.0] - 2026-03-28

### Added

- **Queue Viewer widget** (`apostrophe_widgets.h`): new `ap_queue_viewer` widget for displaying live-updating background job queues. Features: animated pill selection (same interpolation as `ap_list`), horizontal text scroll on long titles, inline per-item progress bars, filter cycling (All / In Progress / Done / Failed, Y button), summary bar showing "X/Y complete, Z failed" above the footer, detail callback for terminal items (A button), cancel callback while jobs are active plus clear-done callback when idle (X button), Menu/footer-overflow support, and idle-aware frame requests — CPU goes idle automatically when all jobs are terminal. Caller supplies a thread-safe snapshot callback; all threading stays in the caller. Demo entry added.

- **Smooth detail screen scrolling** (`apostrophe_widgets.h`): `ap_detail_screen` now animates scroll position changes using linear interpolation over 80ms (`AP__SCROLL_ANIM_MS`), replacing the previous instant-jump behavior. Scroll target and display position are tracked separately, with each input press starting a new animation from the current position.
- **Expanded options demo** (`examples/demo/main.c`): Options List demo now has 15 items (was 5) including Brightness, Language, Screen Timeout, WiFi, Bluetooth, Notifications, Font Size, Auto-Save, Storage, and Reset All — enough to exercise scrolling on any screen size.

- **Optional list scrollbar hiding** (`apostrophe_widgets.h`, `examples/demo/main.c`): `ap_list_opts` now includes `hide_scrollbar` so lists can keep the same scrolling behavior while omitting the scrollbar gutter and thumb. The demo app includes a dedicated `List (No Scrollbar)` example.
- **URL keyboard number/symbol toggle** (`apostrophe_widgets.h`): `ap_url_keyboard()` now exposes a bottom-row `123` / `abc` toggle that swaps the URL key rows between URL-friendly characters/QWERTY input and a number/symbol grid, making it possible to enter digits and additional punctuation without leaving URL mode.
- **List trailing hints** (`apostrophe_widgets.h`): `ap_list_item` now supports optional right-aligned `trailing_text` without changing `metadata` semantics, keeping existing list consumers source-compatible while enabling downstream-style row hints.
- **Keyboard input field scrolling** (`apostrophe_widgets.h`): Text in the keyboard input field now scrolls horizontally to keep the caret always visible when text exceeds the field width. Works for all keyboard types (general, URL, numeric). Demo entry for long text scrolling.
- **Idle rendering** (`apostrophe.h`, `apostrophe_widgets.h`): `ap_present()` now sleeps the thread via `SDL_WaitEventTimeout` when no frame is requested, dropping idle CPU usage to near zero. New public API: `ap_request_frame()` for continuous 60fps rendering (animations), `ap_request_frame_in(ms)` for scheduling future redraws (caret blink, spinners). All widget loops instrumented: `ap_list` (pill animation, text scroll), keyboards (caret blink every 500ms), `ap_process_message` (spinner/progress), `ap_download_manager` (active downloads). Static widgets (options, confirmation, detail, etc.) idle automatically.
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

### Changed

- **`ap_draw_text_wrapped` return type** (`apostrophe.h`): now returns `int` (rendered height in pixels) instead of `void`, enabling callers to measure wrapped text layout.

### Fixed

- **Keyboard demo flow and coverage** (`examples/demo/main.c`): exiting the on-screen keyboard now returns to the `Keyboard Mode` picker instead of the top-level demo menu, the built-in URL demo now uses `ap_url_keyboard()` rather than the generic keyboard, the custom URL demo is wired to the correct menu entry, and the UTF-8 / long-text cases were merged into a single demo entry.
- **MY355 analog stick sensitivity** (`apostrophe.h`): `AP_AXIS_DEADZONE` is now platform-specific, using `20000` on Miyoo Flip (`my355`) builds to reduce accidental left/right activation when moving vertically while keeping the previous deadzone on other targets.
- **Demo background preview** (#10): use selected item metadata and guard on reload return value.
- **Keyboard UTF-8 editing**: backspace, caret movement, and input-field measurement now operate on UTF-8 codepoint boundaries, fixing broken deletion/rendering for multibyte characters such as `€`.
- **Bundled font glyph coverage**: added `✓` and `€` to `res/font.ttf`, fixing the multi-select checkmark glyph and adding Euro symbol support. On a device, the font that is configured in NextUI will still be used, so this only affects desktop development builds that use the bundled font.
- **NextUI background color compatibility**: `ap_theme_load_nextui()` now accepts both current `color7` and legacy `bgcolor` for the fallback solid background color.
- **Documentation terminology and controls**: standardized `my355` platform naming to `Miyoo Flip` and corrected stale help-overlay docs to use the Menu button instead of L1.

## [v0.0.1] - 2026-03-10

### Added

- **Core library** (`apostrophe.h`): init/quit lifecycle, SDL2 window/renderer management, input abstraction (virtual buttons, combos), drawing primitives (text, shapes, pills, images), theming (NextUI integration), font system (6 tiers matching NextUI), status bar, footer, scrollbar, progress bar, screen fade, texture cache, text scrolling, CPU/fan control, power button handling, logging
- **Widget library** (`apostrophe_widgets.h`): list (single/multi-select, reorder, images), options list (cycle/keyboard/clickable/color picker), keyboard (general/URL/numeric), confirmation dialog, selection dialog, process message (async worker with progress), download manager (multi-threaded, requires libcurl), detail screen (info/description/image/table sections), color picker, help overlay
- **Platform support**: TrimUI Smart Pro (`tg5040`), TrimUI Smart Brick (`tg5040`/`tg3040`), TrimUI Smart Pro S (`tg5050`), Miyoo Flip (`my355`), macOS (development)
- **Build system**: GNU Make with Docker-based cross-compilation for all device platforms
- **Examples**: hello (minimal), demo (comprehensive widget showcase), download (status bar + download manager), combo (chord/sequence input), perf (CPU/fan control)
- **Documentation**: Getting Started guide, API reference, Widget catalog, Porting from Gabagool guide, Gabagool parity matrix
