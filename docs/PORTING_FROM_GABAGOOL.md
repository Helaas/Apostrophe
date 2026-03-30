# Porting from Gabagool (Go) to Apostrophe (C)

This guide helps you migrate UI code from [Gabagool](https://github.com/LoveRetro/gabagool) to Apostrophe.

This guide targets Apostrophe **v1.1.0** (2026-03-30).

## Overview of Changes

| Aspect | Gabagool (Go) | Apostrophe (C) |
|--------|--------------|----------------|
| Language | Go | C11 |
| Prefix | `gabagool.` | `ap_` / `AP_` |
| Init | `gabagool.NewApp()` | `ap_init(&cfg)` |
| Cleanup | `defer app.Cleanup()` | `ap_quit()` |
| Generics | `ProcessMessage[T]` | `void*` + function pointer |
| Goroutines | `go func()` | `pthread_create` |
| Error handling | `error` / `ErrCancelled` | `AP_OK` / `AP_CANCELLED` |
| Scaling | `app.Scale()` | `AP_S()` macro |
| Theme | `app.Theme` | `ap_get_theme()` |

## Initialization

### Gabagool
```go
app := gabagool.NewApp()
defer app.Cleanup()
```

### Apostrophe
```c
ap_config cfg = {
    .window_title = "My Pak",
    .font_path    = "font.ttf",
    .is_nextui    = AP_PLATFORM_IS_DEVICE,
};
ap_init(&cfg);
// ... at end:
ap_quit();
```

## List

### Gabagool
```go
opts := &gabagool.ListOptions{
    Title:       "Pick One",
    Items:       items,
    MultiSelect: false,
    FooterHints: []gabagool.FooterHint{
        {Button: gabagool.ButtonB, Label: "Back"},
        {Button: gabagool.ButtonA, Label: "Select", IsConfirm: true},
    },
}
result, err := app.List(opts)
if errors.Is(err, gabagool.ErrCancelled) { return }
```

### Apostrophe
```c
ap_footer_item footer[] = {
    { .button = AP_BTN_B, .label = "Back" },
    { .button = AP_BTN_A, .label = "Select", .is_confirm = true },
};
ap_list_opts opts = ap_list_default_opts("Pick One", items, count);
opts.footer       = footer;
opts.footer_count = 2;

ap_list_result result;
int rc = ap_list(&opts, &result);
if (rc == AP_CANCELLED) return;
```

## Options List (Settings)

### Gabagool
```go
opts := &gabagool.OptionsListOptions{
    Title: "Settings",
    Items: []gabagool.OptionsItem{
        {Label: "Volume", Type: gabagool.OptionStandard,
         Options: []string{"Off", "Low", "Mid", "High"},
         Selected: 2},
    },
}
result, _ := app.OptionsList(opts)
```

### Apostrophe
```c
ap_option vol_opts[] = {
    { .label = "Off", .value = "0" },
    { .label = "Low", .value = "25" },
    { .label = "Mid", .value = "50" },
    { .label = "High", .value = "75" },
};
ap_options_item items[] = {
    { .label = "Volume", .type = AP_OPT_STANDARD,
      .options = vol_opts, .option_count = 4, .selected_option = 2 },
};
ap_options_list_opts opts = {
    .title = "Settings", .items = items, .item_count = 1,
};
ap_options_list_result result;
ap_options_list(&opts, &result);
```

**Key differences**: In Gabagool, options are just strings. In Apostrophe, each option has a `.label` (displayed) and `.value` (returned).
If you want Gabagool-style "A saves settings, Left/Right changes values" behavior, set `.confirm_button = AP_BTN_A` and rely on Left/Right for `AP_OPT_STANDARD` changes. For `AP_OPT_KEYBOARD` and `AP_OPT_COLOR_PICKER` items, A still opens the sub-editor first, but confirming inside the sub-editor will also confirm and exit the options list. Cancelling the sub-editor returns to the list without confirming.

## Keyboard

### Gabagool
```go
text, err := app.Keyboard("initial", "Enter name:")
```

### Apostrophe
```c
ap_keyboard_result result;
int rc = ap_keyboard("initial", "Enter name:", AP_KB_GENERAL, &result);
if (rc == AP_OK) {
    // result.text contains the string
}
```

The keyboard now matches Gabagool's 5-row layout:
- Row 0: Numbers 1-0 + backspace
- Row 1: QWERTY (centered)
- Row 2: ASDF + enter
- Row 3: Shift + ZXCV + symbol toggle
- Row 4: Space bar

**Button mapping** is Gabagool-compatible: B=backspace, X=space, Y=exit, Select=shift, Start=confirm, L1/R1=cursor, Menu=help.

**URL Keyboard** supports configurable shortcuts:
```c
ap_keyboard_result result;
int rc = ap_url_keyboard("https://", "Enter URL:", NULL, &result);
```

## Process Message (Async Worker)

### Gabagool
```go
result, err := gabagool.ProcessMessage[MyResult](app, &gabagool.ProcessMessageOptions{
    Message:      "Working...",
    ShowProgress: true,
    Task: func(progress *atomic.Value, interrupt *atomic.Bool) (MyResult, error) {
        for i := 0; i <= 100; i++ {
            progress.Store(float64(i) / 100.0)
            time.Sleep(30 * time.Millisecond)
        }
        return MyResult{Data: "done"}, nil
    },
})
```

### Apostrophe
```c
static int my_worker(void *userdata) {
    float *progress = (float *)userdata;
    for (int i = 0; i <= 100; i++) {
        *progress = (float)i / 100.0f;
        SDL_Delay(30);
    }
    return AP_OK;
}

float progress = 0;
ap_process_opts opts = {
    .message       = "Working...",
    .show_progress = true,
    .progress      = &progress,
};
int rc = ap_process_message(&opts, my_worker, &progress);
```

**Key differences**:
- Go generics → C `void*` userdata pattern
- `atomic.Value` → `float*` pointer (worker writes, UI reads)
- `atomic.Bool` for cancel → `int*` interrupt_signal
- goroutine → pthread (managed internally)

## Confirmation

### Gabagool
```go
ok, err := app.Confirm("Are you sure?")
```

### Apostrophe
```c
ap_footer_item footer[] = {
    { .button = AP_BTN_B, .label = "No" },
    { .button = AP_BTN_A, .label = "Yes", .is_confirm = true },
};
ap_message_opts opts = {
    .message = "Are you sure?",
    .footer = footer, .footer_count = 2,
};
ap_confirm_result result;
ap_confirmation(&opts, &result);
if (result.confirmed) { /* yes */ }
```

## Selection

### Gabagool
```go
idx, err := app.Selection("Choose:", []string{"Easy", "Normal", "Hard"})
```

### Apostrophe
```c
ap_selection_option opts[] = {
    { .label = "Easy",   .value = "1" },
    { .label = "Normal", .value = "2" },
    { .label = "Hard",   .value = "3" },
};
ap_selection_result result;
ap_selection("Choose:", opts, 3, footer, 2, &result);
// result.selected_index
```

## Scaling

### Gabagool
```go
margin := app.Scale(20)
```

### Apostrophe
```c
int margin = AP_S(20);
```

## Theme Colors

### Gabagool
```go
color := app.Theme.Highlight
```

### Apostrophe
```c
ap_color color = ap_get_theme()->highlight;
```

## Button Constants

| Gabagool | Apostrophe |
|----------|-----------|
| `gabagool.ButtonA` | `AP_BTN_A` |
| `gabagool.ButtonB` | `AP_BTN_B` |
| `gabagool.ButtonX` | `AP_BTN_X` |
| `gabagool.ButtonY` | `AP_BTN_Y` |
| `gabagool.ButtonUp` | `AP_BTN_UP` |
| `gabagool.ButtonDown` | `AP_BTN_DOWN` |
| `gabagool.ButtonLeft` | `AP_BTN_LEFT` |
| `gabagool.ButtonRight` | `AP_BTN_RIGHT` |
| `gabagool.ButtonL1` | `AP_BTN_L1` |
| `gabagool.ButtonL2` | `AP_BTN_L2` |
| `gabagool.ButtonR1` | `AP_BTN_R1` |
| `gabagool.ButtonR2` | `AP_BTN_R2` |
| `gabagool.ButtonStart` | `AP_BTN_START` |
| `gabagool.ButtonSelect` | `AP_BTN_SELECT` |
| `gabagool.ButtonMenu` | `AP_BTN_MENU` |
| `gabagool.ButtonPower` | `AP_BTN_POWER` |

## Font Tiers

| Gabagool | Apostrophe | Base Size | NextUI equiv |
|----------|-----------|-----------|--------------|
| `gabagool.FontExtraLarge` | `AP_FONT_EXTRA_LARGE` | 24 × device_scale | — |
| `gabagool.FontLarge` | `AP_FONT_LARGE` | 16 × device_scale | FONT_LARGE |
| `gabagool.FontMedium` | `AP_FONT_MEDIUM` | 14 × device_scale | FONT_MEDIUM |
| `gabagool.FontSmall` | `AP_FONT_SMALL` | 12 × device_scale | FONT_SMALL |
| `gabagool.FontTiny` | `AP_FONT_TINY` | 10 × device_scale | FONT_TINY |
| `gabagool.FontMicro` | `AP_FONT_MICRO` | 7 × device_scale | FONT_MICRO |

## Common Patterns

### Error Handling

```go
// Gabagool
result, err := app.List(opts)
if err != nil {
    if errors.Is(err, gabagool.ErrCancelled) {
        return  // user backed out
    }
    log.Fatal(err)
}
```

```c
// Apostrophe
int rc = ap_list(&opts, &result);
if (rc == AP_CANCELLED) return;   // user backed out
if (rc == AP_ERROR) exit(1);      // something broke
// AP_OK — success
```

### Menu Loop

```go
// Gabagool
for {
    result, err := app.List(menuOpts)
    if errors.Is(err, gabagool.ErrCancelled) { break }
    // handle result...
}
```

```c
// Apostrophe
while (1) {
    int rc = ap_list(&opts, &result);
    if (rc != AP_OK || result.action == AP_ACTION_BACK) break;
    // handle result...
}
```

## Window Visibility

### Gabagool
```go
app.ShowWindow()
app.HideWindow()
```

### Apostrophe
```c
ap_show_window();
ap_hide_window();
```

Both are no-ops when called before `ap_init()`.

## Combos

Gabagool supports `ChordOptions.OnTrigger`/`OnRelease` callbacks and `SequenceOptions.OnTrigger`.
Apostrophe matches this with two registration styles:

### Polling (classic)

Register with `ap_register_chord` / `ap_register_sequence` and drain `ap_poll_combo()` each frame.

#### Gabagool
```go
app.RegisterChord(&gabagool.ChordOptions{
    ID:      "shoulders",
    Buttons: []gabagool.Button{gabagool.ButtonL1, gabagool.ButtonR1},
    Window:  150 * time.Millisecond,
})
// poll in loop:
for event := range app.ComboEvents() {
    if event.Triggered { fmt.Println("triggered:", event.ID) }
}
```

#### Apostrophe
```c
ap_button shoulders[] = { AP_BTN_L1, AP_BTN_R1 };
ap_register_chord("shoulders", shoulders, 2, 150);

/* In your main loop: */
ap_combo_event combo;
while (ap_poll_combo(&combo)) {
    const char *kind = (combo.type == AP_COMBO_CHORD) ? "chord" : "seq";
    if (combo.triggered)
        printf("Triggered [%s]: %s\n", kind, combo.id);
}
```

### Callbacks (_ex variants)

Register with `ap_register_chord_ex` / `ap_register_sequence_ex` to get callbacks that fire
automatically at trigger/release time — no poll loop required. Polling still works alongside
callbacks; both are additive.

#### Gabagool
```go
app.RegisterChord(&gabagool.ChordOptions{
    ID:      "shoulders",
    Buttons: []gabagool.Button{gabagool.ButtonL1, gabagool.ButtonR1},
    Window:  150 * time.Millisecond,
    OnTrigger: func(id string) { fmt.Println("triggered:", id) },
    OnRelease: func(id string) { fmt.Println("released:", id) },
})
```

#### Apostrophe
```c
void on_trigger(const char *id, ap_combo_type type, void *userdata) {
    printf("triggered: %s\n", id);
}
void on_release(const char *id, ap_combo_type type, void *userdata) {
    printf("released: %s\n", id);
}

ap_button shoulders[] = { AP_BTN_L1, AP_BTN_R1 };
ap_register_chord_ex("shoulders", shoulders, 2, 150, on_trigger, on_release, NULL);

/* Sequences: on_trigger only (no release phase) */
ap_button uudd[] = { AP_BTN_UP, AP_BTN_UP, AP_BTN_DOWN, AP_BTN_DOWN };
ap_register_sequence_ex("uudd", uudd, 4, 500, false, on_trigger, NULL);
```

The `ap_combo_type` argument in the callback is `AP_COMBO_CHORD` or `AP_COMBO_SEQUENCE`, letting
a single callback handle both kinds.

## Download Manager

Gabagool's `DownloadManager` maps to Apostrophe's `ap_download_manager`. Both use concurrent downloads with progress reporting.

### Gabagool
```go
downloads := []gabagool.Download{
    {URL: "https://example.com/a.zip", DestPath: "/tmp/a.zip", Label: "a.zip"},
    {URL: "https://example.com/b.zip", DestPath: "/tmp/b.zip", Label: "b.zip"},
}
result, err := app.DownloadManager(downloads, &gabagool.DownloadOptions{
    MaxConcurrent: 3,
})
```

### Apostrophe
```c
ap_download downloads[] = {
    { .url = "https://example.com/a.zip", .dest_path = "/tmp/a.zip", .label = "a.zip" },
    { .url = "https://example.com/b.zip", .dest_path = "/tmp/b.zip", .label = "b.zip" },
};
ap_download_opts opts = { .max_concurrent = 3 };
ap_download_result result;
ap_download_manager(downloads, 2, &opts, &result);
```

**Key differences**:
- Go's HTTP client → libcurl (compile with `-DAP_ENABLE_CURL`, link with `-lcurl`)
- Go's goroutines → pthreads (managed internally by the widget)
- Status is tracked per-job via the `ap_download` struct fields (`status`, `progress`, `speed_bps`)
