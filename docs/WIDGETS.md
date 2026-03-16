# Apostrophe Widget Catalog

Visual guide to every widget available in `apostrophe_widgets.h`.

Widgets that render footer hints inherit the core footer overflow behaviour from `ap_draw_footer()`: overflowing hints stay on one line, show a `+N` marker, and can be inspected with the configured footer overflow chord.

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
- Background preview: Per-item fullscreen background image shown when the item is focused
- Text scroll: Long labels auto-scroll horizontally
- Help overlay: L1 shows scrollable help text
- Explicit action bindings for Start/Y/Menu via `ap_list_opts` action fields

**Usage**:
```c
ap_list_item items[] = {
    AP_LIST_ITEM("Alpha", "/path/alpha"),
    AP_LIST_ITEM("Bravo", "/path/bravo"),
    AP_LIST_ITEM_BG("Charlie", "/path/charlie", bg_tex), /* with background preview */
};
ap_list_opts opts = ap_list_default_opts("Title", items, 3);
opts.confirm_button = AP_BTN_START; // Footer hints are visual-only
ap_list_result result;
ap_list(&opts, &result);
```

> **Note:** Always use designated initializers (e.g. `{ .label = "Foo" }`) or the
> `AP_LIST_ITEM` / `AP_LIST_ITEM_BG` convenience macros when creating `ap_list_item`
> values. New fields may be added in future releases; positional initializers
> (e.g. `{ "Foo", NULL, NULL, false, NULL }`) will break at compile time if that happens.

**Font override**: Set `item_font` in `ap_list_opts` to override the list item text font (default: `AP_FONT_LARGE`). Pass `NULL` (zero-init default) to keep the widget default.

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
If an item has invalid option storage (`options == NULL` or an out-of-range `selected_option`), the widget clamps/ignores it safely.

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
ap_options_list_opts opts = {
    .title = "Settings", .items = items, .item_count = 1,
    .confirm_button = AP_BTN_START,
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

**Usage**:
```c
ap_keyboard_result result;
int rc = ap_keyboard("initial", "Enter text:", AP_KB_GENERAL, &result);
if (rc == AP_OK) printf("Got: %s\n", result.text);
```

**URL Keyboard** adds shortcut rows (e.g. `https://`, `.com`) above the QWERTY keys:
```c
ap_keyboard_result result;
int rc = ap_url_keyboard("https://", "Enter URL:", NULL, &result);
```

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

Full-screen scrollable text overlay. Triggered automatically by L1 in widgets that set `help_text`.

```
┌─────────────────────────────┐
│ ╔═══════════════════════╗   │
│ ║  Navigate with D-Pad  ║   │
│ ║  Press A to select    ║   │
│ ║  Press B to go back   ║   │
│ ║  Press X to reorder   ║   │
│ ║  L1 shows this help   ║   │
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
