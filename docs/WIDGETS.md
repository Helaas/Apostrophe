# Apostrophe Widget Catalog

Visual guide to every widget available in `apostrophe_widgets.h`.

This catalog documents Apostrophe **v1.1.0** (2026-03-30).

Widgets that render footer hints inherit the core footer overflow behaviour from `ap_draw_footer()`: overflowing hints stay on one line, show a `+N` marker, and, in widgets that handle the Menu button, can be inspected via the Menu button.

---

## List (`ap_list`)

A scrollable list of items with cursor navigation.

```
┌─────────────────────────────┐
│  NATO Alphabet              │ ← Title
│─────────────────────────────│
│  ┌─────────────────────┐    │
│  │ ▸ Alpha             │    │ ← Highlighted (pill)
│  └─────────────────────┘    │
│    Bravo                    │
│    Charlie                  │
│    Delta                    │
│    Echo                     │
│    Foxtrot                  │
│    Golf                     │
│    ↓ more...                │ ← Scroll indicator
│─────────────────────────────│
│  [B] Back         [A] Select│ ← Footer
└─────────────────────────────┘
```

**Features**:
- Single select: D-Pad + A to confirm
- Multi-select: Toggle with A, show checkboxes
- Reorder: Toggle reorder mode with a button, then D-Pad to move items
- Images: Optional thumbnail column on the left
- Optional hidden scrollbar: keep scrolling behavior without drawing the scrollbar
- Live footer labels: update existing footer text from a callback while the list is open
- Background preview: Per-item fullscreen background image shown when the item is focused
- Trailing hint: Optional right-aligned text for item status/type
- Text scroll: Long labels auto-scroll horizontally
- Help overlay: Menu shows scrollable help text
- Explicit action bindings for Start/Y/Menu via `ap_list_opts` action fields

**Usage**:
```c
ap_list_item items[] = {
    { .label = "Alpha", .metadata = "/path/alpha", .trailing_text = "NEW" },
    AP_LIST_ITEM("Bravo", "/path/bravo"),
    { .label = "Charlie", .metadata = "/path/charlie",
      .background_image = bg_tex, .trailing_text = "PNG" },
};
ap_list_opts opts = ap_list_default_opts("Title", items, 3);
opts.confirm_button = AP_BTN_START; // Footer hints are visual-only
ap_list_result result;
ap_list(&opts, &result);
```

To hide the scrollbar while preserving normal list scrolling:

```c
ap_list_opts opts = ap_list_default_opts("Title", items, count);
opts.hide_scrollbar = true;
```

To update a footer label from the focused row:

```c
static void update_footer(ap_list_opts *opts, int cursor, void *userdata) {
    ap_footer_item *footer = userdata;

    (void)opts;
    footer[1].label = (cursor == 0) ? "STOP PREVIEW" : "PREVIEW";
}

ap_footer_item footer[] = {
    { .button = AP_BTN_B, .label = "BACK" },
    { .button = AP_BTN_Y, .label = "PREVIEW" },
};

ap_list_opts opts = ap_list_default_opts("Title", items, count);
opts.footer = footer;
opts.footer_count = 2;
opts.footer_update = update_footer;
opts.footer_update_userdata = footer;
```

If the label depends on external state rather than cursor changes alone, call `ap_request_frame_in(ms)` or `ap_request_frame()` inside the callback so the list redraws while idle.

> **Note:** Always use designated initializers (e.g. `{ .label = "Foo" }`) or the
> `AP_LIST_ITEM` / `AP_LIST_ITEM_BG` convenience macros when creating `ap_list_item`
> values. New fields may be added in future releases; positional initializers
> (e.g. `{ "Foo", NULL, NULL, false, NULL }`) are fragile across releases and may
> need updates when fields are added.

`metadata` stays hidden and is useful for paths, IDs, or other internal payloads. `trailing_text` is the visible right-aligned hint. Trailing hints use the item font and are omitted for a row if showing them would leave less than `AP_S(96)` for the label.

**Font override**: Set `item_font` in `ap_list_opts` to override the list item text font (default: `AP_FONT_LARGE`). Pass `NULL` (zero-init default) to keep the widget default.

**Live footer contract**: `footer_update` runs after the focused row and scroll position settle, but before the footer is drawn. Use it to update existing footer labels or `button_text` in place. Keep it cheap, and do not change `item_count`, `footer_count`, or swap out the items/footer arrays while the widget is running.

**Navigation shortcuts**:

| Input | Action | Notes |
|-------|--------|-------|
| D-Pad Up/Down | Move one item | Wraps at boundaries (fresh press only) |
| D-Pad Left/Right | Skip one page | Always available; disabled in reorder mode |
| L1 / R1 | Jump to previous/next letter group | Always available; items should be pre-sorted for best results. Disabled in reorder mode |
| Menu | Show help overlay / footer overflow | Shows help if `help_text` is set, then footer overflow if hidden items exist |

---

## Options List (`ap_options_list`)

Settings-style list with per-row option values.

```
┌─────────────────────────────┐
│  Settings                   │
│─────────────────────────────│
│  ┌─────────────────────┐    │
│  │ Volume        ◂ Mid ▸│   │ ← Standard: L/R cycle
│  └─────────────────────┘    │
│    Theme          Dark      │ ← Standard: L/R cycle
│    Name           [tap]     │ ← Keyboard: A to type
│    About          →         │ ← Clickable: A to navigate
│─────────────────────────────│
│  [B] Back       [START] Save│
└─────────────────────────────┘
```

**Option types**:
| Type | Interaction |
|------|------------|
| `AP_OPT_STANDARD` | Left/Right to cycle values |
| `AP_OPT_KEYBOARD` | A opens on-screen keyboard |
| `AP_OPT_CLICKABLE` | A triggers action |
| `AP_OPT_COLOR_PICKER` | A opens color picker |

Footer hints are visual-only; configure behavior with `action_button`, `secondary_action_button`, and `confirm_button`.
Use `ap_footer_item.button_text` when you want custom footer pill text such as `←/→` for a shared "Change" hint.
If an item has invalid option storage (`options == NULL` or an out-of-range `selected_option`), the widget clamps/ignores it safely.
Long labels and long right-side values are ellipsized to avoid overlap on narrow layouts. `AP_OPT_CLICKABLE` rows always reserve and render the trailing `>` indicator, even when the row is not focused.

**Usage**:
```c
ap_option values[] = {
    { .label = "Off", .value = "0" },
    { .label = "On",  .value = "1" },
};
ap_options_item items[] = {
    { .label = "Sound", .type = AP_OPT_STANDARD,
      .options = values, .option_count = 2, .selected_option = 1 },
};
ap_footer_item footer[] = {
    { .button = AP_BTN_B,    .label = "BACK" },
    { .button = AP_BTN_LEFT, .label = "CHANGE", .button_text = "←/→" },
    { .button = AP_BTN_A,    .label = "SAVE", .is_confirm = true },
};
ap_options_list_opts opts = {
    .title = "Settings", .items = items, .item_count = 1,
    .footer = footer, .footer_count = 3,
    .confirm_button = AP_BTN_A,
};
ap_options_list_result result;
ap_options_list(&opts, &result);
```

**Font overrides**: Set `label_font` and `value_font` in `ap_options_list_opts` to override the option label font (default: `AP_FONT_LARGE`) and option value font (default: `AP_FONT_TINY`) respectively. Pass `NULL` (zero-init default) to keep the widget defaults.

---

## Keyboard (`ap_keyboard`)

5-row on-screen keyboard matching Gabagool's layout.

```
┌───────────────────────────────────────┐
│  [ H e l l o _ ]                      │ ← Text field with cursor
│                                       │
│  1  2  3  4  5  6  7  8  9  0  [⌫]    │ ← Numbers + backspace
│     q  w  e  r  t  y  u  i  o  p      │ ← QWERTY (centered)
│     a  s  d  f  g  h  j  k  l  [↵]    │ ← ASDF + enter
│  [⇧]  z  x  c  v  b  n  m  [#+=]      │ ← Shift + ZXCV + symbol
│           [     space     ]           │ ← Space bar
│                                       │
│             [MENU] Help               │ ← Footer
└───────────────────────────────────────┘
```

**Layouts**: `AP_KB_GENERAL`, `AP_KB_URL`, `AP_KB_NUMERIC`

**Controls** (Gabagool-compatible):
| Button | Action |
|--------|--------|
| D-Pad | Navigate keys |
| A | Type selected key / activate special key |
| B | Backspace |
| X | Space (general) / Toggle URL shortcuts (URL mode) |
| Y | Exit without saving |
| Select | Toggle shift |
| Start | Confirm text |
| L1 / R1 | Move text cursor left / right |
| Menu | Show help overlay |

**Text overflow**: When input text exceeds the field width, the field scrolls horizontally to keep the cursor visible.

**`help_text`**: Content shown in the **Menu help overlay**. Pass `NULL` to use the built-in keyboard instructions. This is not an on-screen prompt — the string is shown verbatim in the overlay when the user presses Menu.

**Usage**:
```c
ap_keyboard_result result;
int rc = ap_keyboard("initial", NULL, AP_KB_GENERAL, &result);
if (rc == AP_OK) printf("Got: %s\n", result.text);
```

**URL Keyboard** adds shortcut rows (e.g. `https://`, `.com`) above the QWERTY keys and a bottom-row `123` / `abc` toggle for switching between URL-friendly input and a number/symbol grid:
```c
ap_keyboard_result result;
int rc = ap_url_keyboard("https://", NULL, NULL, &result);
```

In URL mode:
- `X` swaps the shortcut rows between the default URL presets and alternate presets
- The `123` key switches the URL rows to digits and symbols
- The `abc` key restores the URL character rows and QWERTY letters

---

## Confirmation (`ap_confirmation`)

Modal message dialog with optional image.

```
┌─────────────────────────────┐
│                             │
│                             │
│    Are you sure you want    │
│    to delete this file?     │
│                             │
│                             │
│─────────────────────────────│
│  [B] No           [A] Yes   │
└─────────────────────────────┘
```

**Usage**:
```c
ap_footer_item footer[] = {
    { .button = AP_BTN_B, .label = "No" },
    { .button = AP_BTN_A, .label = "Yes", .is_confirm = true },
};
ap_message_opts opts = {
    .message = "Delete this file?",
    .footer = footer, .footer_count = 2,
};
ap_confirm_result result;
ap_confirmation(&opts, &result);
if (result.confirmed) { /* delete it */ }
```

---

## Selection (`ap_selection`)

Horizontal pill-style option chooser.

```
┌─────────────────────────────┐
│                             │
│    Choose difficulty:       │
│                             │
│    ◂ [Easy] Normal  Hard ▸  │
│                             │
│─────────────────────────────│
│  [B] Cancel   [A] Choose    │
└─────────────────────────────┘
```

**Usage**:
```c
ap_selection_option opts[] = {
    { .label = "Easy",   .value = "1" },
    { .label = "Normal", .value = "2" },
    { .label = "Hard",   .value = "3" },
};
ap_selection_result result;
ap_selection("Difficulty:", opts, 3, footer, 2, &result);
```

---

## Process Message (`ap_process_message`)

Async worker with progress bar.

```
┌─────────────────────────────┐
│                             │
│                             │
│    Downloading update...    │
│                             │
│    ████████████░░░░░ 68%    │ ← Progress bar
│                             │
│                             │
└─────────────────────────────┘
```

**Features**:
- Background thread runs your worker function
- Optional progress bar (0.0–1.0)
- Optional cancel button
- Dynamic message updates from worker

**Usage**:
```c
static int my_worker(void *userdata) {
    float *prog = (float *)userdata;
    for (int i = 0; i <= 100; i++) {
        *prog = i / 100.0f;
        SDL_Delay(50);
    }
    return AP_OK;
}

float progress = 0;
ap_process_opts opts = {
    .message = "Working...",
    .show_progress = true,
    .progress = &progress,
};
ap_process_message(&opts, my_worker, &progress);
```

---

## Detail Screen (`ap_detail_screen`)

Scrollable multi-section information view.

```
┌─────────────────────────────┐
│  Game Info                  │
│─────────────────────────────│
│  ── Info ──                 │
│  Name:      Super Game      │
│  Platform:  SNES            │
│  Year:      1994            │
│                             │
│  ── Description ──          │
│  A classic platformer that  │
│  defined a generation of... │
│                             │
│  ↓ Scroll for more          │
│─────────────────────────────│
│  [B] Back                   │
└─────────────────────────────┘
```

**Section types**: Info (key-value), Description (text), Image, Table

Scrolling is animated over 80ms using linear interpolation for smooth visual feedback.

Image sections are loaded once per detail-screen session and reused every frame for better performance.

**Optional styling** (all opt-in, zero-initialize safe):

| Option | Effect |
|--------|--------|
| `center_title = true` | Center the screen title instead of left-aligning it |
| `show_section_separator = true` | Draw an accent-colored line under each section header |
| `key_color = &color` | Override the info-pair key color (default: `theme->hint`) |
| `body_font = ap_get_font(tier)` | Override body/value text font (default: `AP_FONT_TINY`) |
| `section_title_font = ap_get_font(tier)` | Override section header font (default: `AP_FONT_SMALL`) |
| `key_font = ap_get_font(tier)` | Override info-pair key text font (default: `AP_FONT_TINY`) |

```c
ap_color key_col = ap_get_theme()->text;
ap_detail_opts opts = {
    .title                  = "Styled Detail",
    .sections               = sections,
    .section_count          = 2,
    .footer                 = footer,
    .footer_count           = 2,
    .center_title           = true,
    .show_section_separator = true,
    .key_color              = &key_col,
};
```

---

## Color Picker (`ap_color_picker`)

5×5 grid of predefined colors.

```
┌─────────────────────────────┐
│                             │
│    ■ ■ ■ ■ ■                │
│    ■ ■ [■] ■ ■              │ ← Cursor highlights one
│    ■ ■ ■ ■ ■                │
│    ■ ■ ■ ■ ■                │
│    ■ ■ ■ ■ ■                │
│                             │
│─────────────────────────────│
│  [B] Cancel   [A] Pick      │
└─────────────────────────────┘
```

**Usage**:
```c
ap_color initial = { 255, 100, 50, 255 };
ap_color result;
if (ap_color_picker(initial, &result) == AP_OK) {
    // Use result.r, result.g, result.b
}
```

---

## Help Overlay (`ap_show_help_overlay`)

Full-screen scrollable text overlay. Triggered automatically by Menu in widgets that set `help_text`.

```
┌─────────────────────────────┐
│ ╔═══════════════════════╗   │
│ ║  Navigate with D-Pad  ║   │
│ ║  Press A to select    ║   │
│ ║  Press B to go back   ║   │
│ ║  Press X to reorder   ║   │
│ ║ Menu shows this help  ║   │
│ ╚═══════════════════════╝   │
│                             │
│  Press any button to close  │
└─────────────────────────────┘
```

**Usage** (usually automatic via widgets, but can be called directly):
```c
ap_show_help_overlay("Navigate with D-Pad.\nPress A to select.\nPress B to go back.");
```

---

## File Picker (`ap_file_picker`)

Filesystem browser for selecting files or directories. Built on `ap_list()` — inherits all list navigation (scrolling, pill animation, L1/R1 letter-skip). Folders sort first and show a trailing `>` chevron; files show their uppercase extension. Supports inline folder creation via `ap_keyboard()` in directory-capable modes.

```
┌─────────────────────────────┐
│  Select ROM                 │ ← Title (or relative path)
│─────────────────────────────│
│  ┌─────────────────────┐    │
│  │ ▸ Roms              > │  │ ← Folder (highlighted, chevron)
│  └─────────────────────┘    │
│    Saves                 >  │ ← Folder
│    readme.txt          TXT  │ ← File (extension label)
│    game.zip            ZIP  │ ← File (extension label)
│    photo.png           PNG  │
│─────────────────────────────│
│ [A] ENTER [X] NEW DIR [B] BACK │
│                [START] HERE │ ← Footer
└─────────────────────────────┘
```

**Modes**:
- `AP_FILE_PICKER_FILES` — Only files are selectable (A on folder enters it)
- `AP_FILE_PICKER_DIRS` — Only directories are selectable (A enters, START selects current dir)
- `AP_FILE_PICKER_BOTH` — Files and directories are selectable

**Features**:
- Folders first, then files, both alphabetically sorted (case-insensitive)
- Extension filter: restrict visible files to specific extensions
- Hidden file option: set `show_hidden = true` to include dotfiles like `.env` and dotdirs like `.config`
- Inline folder creation: X button opens keyboard, creates directory in `AP_FILE_PICKER_DIRS` / `AP_FILE_PICKER_BOTH`
- In `AP_FILE_PICKER_BOTH`, the `A` footer hint follows the focused row: `ENTER` for directories, `OPEN` for files
- Root enforcement: on device the picker never leaves `SDCARD_PATH`; on desktop it defaults to `$HOME` unless you pass `root_path`
- Relative-path header when `title == NULL` (for example `SDCARD/roms`)
- Empty directory placeholder when no entries match filters

Dotfiles still participate in the normal extension filter. For example, `.env` is treated like an `env` extension when `show_hidden` is enabled.

**Usage**:
```c
ap_file_picker_opts opts = ap_file_picker_default_opts("Select ROM");
opts.mode = AP_FILE_PICKER_FILES;
opts.extensions = (const char *[]){"zip", "7z"};
opts.extension_count = 2;

ap_file_picker_result result;
int rc = ap_file_picker(&opts, &result);
if (rc == AP_OK) {
    printf("Selected: %s\n", result.path);
}
```

---

## Queue Viewer (`ap_queue_viewer`)

Live-updating queue of background jobs with animated pill selection, filter cycling, and optional progress bars. The widget is a pure display layer — the caller supplies a thread-safe snapshot callback; all threading and job logic remain in the caller.

```
┌───────────────────────────────────────┐
│  DOWNLOADS [ALL]                      │ ← Title with active filter
│───────────────────────────────────────│
│  ┌──────────────────────────────────┐ │
│  │ Mega Man - Dr. Wily's Revenge    │ │ ← Pill on selected row
│  │ Game Boy  [cht]  ████████░░░░░░  │ │ ← Subtitle + inline progress bar
│  └──────────────────────────────────┘ │
│    Maru's Mission       DOWNLOADING...│
│    Game Boy  [cht]  ████████████████  │
│    Looney Tunes                  DONE │
│    Game Boy  [cht]  ████████████████  │
│───────────────────────────────────────│
│    3/10 COMPLETE, 1 FAILED            │ ← Summary bar
│───────────────────────────────────────│
│  [Y] FILTER  [A] DETAILS  [B] BACK    │
└───────────────────────────────────────┘
```

**Status colors**:

| Status | Color |
|--------|-------|
| `AP_QUEUE_PENDING` / `AP_QUEUE_SKIPPED` | Hint (muted) |
| `AP_QUEUE_RUNNING` | Accent |
| `AP_QUEUE_DONE` | Green `(100,200,100)` |
| `AP_QUEUE_FAILED` | Red `(255,100,100)` |

When the highlight pill is on a row, all row text including `status_text` switches to the highlighted text color, matching Scrapegoat's selected-state behavior.

**Footer visibility**:

| Button | Shown when |
|--------|-----------|
| Y FILTER | `hide_filter` is false (default) |
| A DETAILS | `on_detail` set and selected item is terminal |
| X STOP ALL | `on_cancel` set and any PENDING/RUNNING items remain |
| X CLEAR DONE | `on_clear` set and no PENDING/RUNNING items remain |
| B BACK | Always; emitted last on narrow screens so it overflows first |

Footer order is `Y FILTER`, then `A DETAILS` when available, with `B BACK` emitted last so it is the first left-side action to collapse behind `+N` on narrow screens.

**Features**:
- Animated pill selection (same lerp as `ap_list`)
- Horizontal text scroll on long titles when selected
- Per-item inline progress bars on the subtitle row (omit bar entirely when `progress < 0`)
- Filter cycling: ALL / IN PROGRESS / DONE / FAILED by default, overrideable via `filter_labels[4]` (Y button)
- Summary bar showing `X/Y COMPLETE, Z FAILED`
- Detail callback for terminal items (A button)
- Cancel callback while jobs are active, then clear-done when idle (X button)
- CPU-idle aware: calls `ap_request_frame()` only while jobs are active; goes idle automatically when all items reach a terminal state
- Scrollbar aligned with the visible pill stack, outside the pill area
- Menu / desktop `H`: opens hidden footer actions when the footer shows `+N`

Navigation matches `ap_list()`: `D-Pad Left/Right` skip by one visible page, while `L1/R1` jump to the previous/next first-letter group in the current filter. For best results, keep queue titles pre-sorted.

**Usage**:
```c
/* Thread-safe snapshot: copy your job state into buf each frame */
static int my_snapshot(ap_queue_item *buf, int max, void *ud) {
    int n = 0;
    /* acquire lock, copy items, release lock */
    buf[n].status   = AP_QUEUE_RUNNING;
    buf[n].progress = 0.45f;
    snprintf(buf[n].title,       sizeof(buf[n].title),    "Mega Man");
    snprintf(buf[n].subtitle,    sizeof(buf[n].subtitle),  "Game Boy  [cht]");
    snprintf(buf[n].status_text, sizeof(buf[n].status_text), "DOWNLOADING");
    n++;
    return n;
}

static void my_detail(const ap_queue_item *item, void *ud) {
    /* push another widget to show item details */
}

static void my_cancel(void *ud) {
    /* confirm cancellation, then mark unfinished items as CANCELLED */
}

static void my_clear(void *ud) {
    /* remove completed items from your job list */
}

ap_queue_opts opts = {
    .title         = "DOWNLOADS",
    .snapshot      = my_snapshot,
    .userdata      = &my_ctx,
    .on_detail     = my_detail,
    .on_cancel     = my_cancel,
    .on_clear      = my_clear,
    .filter_labels = { "ALL", "ACTIVE", "DONE", "ERRORS" },
};
ap_queue_viewer(&opts);
```

**Snapshot contract**: `snapshot` is called every frame on the render thread. The callback must be safe to call concurrently with your worker threads — protect shared state with a mutex and copy into the caller-supplied `buf`.

**Cancelled downloads**: the widget is display-only. If your app supports cancellation, handle it in `on_cancel`, then update future snapshots to return `AP_QUEUE_SKIPPED` with a status label such as `"CANCELLED"` for any unfinished items. The demo queue viewer shows this pattern.

**Progress bar**: set `progress` to a value in `[0.0, 1.0]` to draw an inline bar on the subtitle row, or to any negative value to omit the bar for that item.

---

## Download Manager (`ap_download_manager`)

Multi-threaded file downloader with per-file progress bars. Requires libcurl.

Device builds use a bundled curl flow (`USE_BUNDLED_CURL=1`), which caches sources in
`build/third_party/sources`, builds per-platform artifacts under `build/third_party/<platform>/...`,
and stages the CA bundle into `build/<platform>/download/lib/cacert.pem`. Ensure the pak launcher sets
`SSL_CERT_FILE=$PAK_DIR/lib/cacert.pem`.

```
┌───────────────────────────────────────┐
│  Downloading... 2/5                   │
│                                       │
│  game-cover.jpg                       │
│  ███████████████████████████  100%    │ ← Complete
│  1.2 MB/s — Complete                  │
│                                       │
│  metadata.json                        │
│  ████████████░░░░░░░░░░░░░░░   45%    │ ← Downloading
│  340 KB/s                             │
│                                       │
│  [Y] Cancel       [X] Hide Speed      │ ← Footer
└───────────────────────────────────────┘
```

**Features**:
- Thread pool with configurable concurrency (default 3 simultaneous)
- Per-file progress bars (3/4 of screen width)
- Live download speed in MB/s or KB/s
- Auto-scroll to show active downloads
- Cancel all pending with Y
- Custom HTTP headers and SSL options

**Usage**:
```c
ap_download downloads[] = {
    { .url = "https://example.com/file1.zip", .dest_path = "/tmp/file1.zip", .label = "file1.zip" },
    { .url = "https://example.com/file2.zip", .dest_path = "/tmp/file2.zip", .label = "file2.zip" },
};
ap_download_opts opts = { .max_concurrent = 3 };
ap_download_result result;
int rc = ap_download_manager(downloads, 2, &opts, &result);
if (rc == AP_OK) printf("%d/%d succeeded\n", result.completed, result.total);
```
