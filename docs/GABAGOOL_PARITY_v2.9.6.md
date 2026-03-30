# Gabagool Parity Matrix (`v2.9.6`)

This document tracks Apostrophe **v1.1.0** parity against Gabagool `v2.9.6`.

## Sources

- `go doc` output for `github.com/BrandonKowalski/gabagool/v2/pkg/gabagool` and `.../constants`
- Usage scan of a NextUI pak project built with Gabagool
- Apostrophe public headers in `include/apostrophe.h` and `include/apostrophe_widgets.h`

## Status Legend

- `Implemented`
- `Implemented (behavior differs)`
- `Missing`

## Priority This Cycle

| Area | Status | Notes |
|---|---|---|
| Core directional hold repeat | Implemented | Added digital held repeat in input core for keyboard/D-pad/button-mapped directions; dedupes against hat/axis repeats. |
| Runtime repeat API (`ap_set_input_repeat`) | Implemented | Delay/rate now configurable at runtime, initialized from defaults. |
| List explicit action outcomes | Implemented | Added triggered/secondary/confirmed/tertiary actions. `AP_ACTION_CUSTOM` kept as alias of `AP_ACTION_TRIGGERED`. |
| List scroll restoration (`visible_start_index`) | Implemented | Input and output support added to list options/result. |
| Options list action/scroll parity | Implemented | Added `initial_selected_index`, `visible_start_index`, `action_button`, `secondary_action_button` plus result scroll position. |
| Example crash logging | Implemented | SDLReader-style launcher logging + app `log_path` wiring in examples. |

## APIs Used by `nextui-scrapegoat-pak`

| Gabagool API usage | Apostrophe parity | Status |
|---|---|---|
| `DefaultListOptions` + `List` (`ReorderButton`, `ActionButton`) | `ap_list_default_opts` + `ap_list` (`reorder_button`, `action_button`) | Implemented |
| `ListAction*` outcomes | `ap_list_action` expanded | Implemented |
| `OptionsList` (`ConfirmButton`) | `ap_options_list` (`confirm_button`) | Implemented |
| `OptionsList` (`ActionButton`, `SecondaryActionButton`, index restoration) | Added in this cycle | Implemented |
| `Keyboard` | `ap_keyboard` | Implemented |
| `ProcessMessage` (progress, interrupt, dynamic message, message lines) | `ap_process_message` | Implemented (behavior differs) |
| `ConfirmationMessage` | `ap_confirmation` | Implemented |
| `SetLogPath`/`LogPath` option | `ap_set_log_path` / `ap_config.log_path` | Implemented |

## Broader Parity Matrix

| Gabagool capability | Apostrophe equivalent | Status | Notes |
|---|---|---|---|
| Init/close lifecycle | `ap_init`/`ap_quit` | Implemented | |
| Input mapping + face button flip | `ap_poll_input`, `ap_flip_face_buttons` | Implemented | |
| Directional repeat control | `ap_set_input_repeat` | Implemented | |
| Debounce control | `ap_set_input_delay` | Implemented | |
| List widget | `ap_list` | Implemented | Includes reorder, help, images, multi-select. |
| Options list widget | `ap_options_list` | Implemented | Includes standard/keyboard/clickable/color picker. |
| Keyboard + URL keyboard | `ap_keyboard`, `ap_url_keyboard` | Implemented | |
| Selection message | `ap_selection` | Implemented | |
| Confirmation message | `ap_confirmation` | Implemented | |
| Process message | `ap_process_message` | Implemented (behavior differs) | Generic Go return values are mapped to C callback/userdata patterns. |
| Idle rendering | `ap_present()` + `ap_request_frame()` | Implemented | Dirty-flag system: thread sleeps via `SDL_WaitEventTimeout` when idle, wakes on input or scheduled redraw. 60fps during animations, near-zero CPU when static. |
| Download manager | `ap_download_manager` | Implemented | |
| Detail/info screen | `ap_detail_screen` | Implemented | |
| Color picker | `ap_color_picker` | Implemented | |
| Help overlay | `ap_show_help_overlay` | Implemented | |
| Status bar | `ap_draw_status_bar` + per-widget status options | Implemented | |
| Theme loading from NextUI | `ap_theme_load_nextui` | Implemented | |
| Logging | `ap_log`, `ap_set_log_path`, `ap_config.log_path` | Implemented | |
| Combo registration/polling | `ap_register_chord`, `ap_register_sequence`, `ap_poll_combo` + `_ex` variants | Implemented | Polling + optional callbacks via `_ex` variants. `ap_combo_event` includes `type` field. |
| Window visibility controls (`ShowWindow`, `HideWindow`) | `ap_show_window`, `ap_hide_window` | Implemented | |
| Input logger/mapping bytes APIs | None | Missing | No direct equivalent for Gabagool input remap byte loader. |
| Infrastructure error typing helpers | None | Missing | Apostrophe uses integer return codes + `ap_get_error()`. |

## Out of Scope

1. Add user-configurable input mapping loader equivalent to `SetInputMappingBytes`.
2. Add optional infrastructure-error style wrappers around current integer return model for migration ergonomics.
