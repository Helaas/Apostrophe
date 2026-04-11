# Demo Coverage Matrix

This matrix maps Apostrophe's public API surface to the demos that exercise it.

This matrix reflects Apostrophe **v1.1.0** (2026-03-30).

## Core API Coverage

| API Group | Demo Coverage |
|----------|---------------|
| Lifecycle, renderer/window accessors, logging, error helpers | `examples/demo/main.c` → `Core API Lab` |
| Scaling, content layout, footer/status metrics, font bump | `examples/demo/main.c` → `Core API Lab` |
| Drawing primitives, background, status bar, texture cache, text scrolling | `examples/demo/main.c` → `Drawing Primitives`, `Core API Lab` |
| Input delay/repeat, face-button flip, button names, theme color | `examples/demo/main.c` → `Input & Theme` |
| Screen fade | `examples/demo/main.c` → `Screen Fade` |
| Combo registration, polling, callbacks, unregister, footer overflow override | `examples/combo/main.c` |
| CPU, fan, power-handler toggle | `examples/demo/main.c` → `CPU & Fan`, `Core API Lab` |

## Widget Coverage

| Widget / Feature | Demo Coverage |
|------------------|---------------|
| List: basic select, help overlay | `examples/demo/main.c` → `Basic List` |
| List: hidden scrollbar | `examples/demo/main.c` → `List (No Scrollbar)` |
| List: live footer labels, idle refresh via `ap_request_frame_in()` | `examples/demo/main.c` → `Live Footer` |
| List: images, primary/secondary/tertiary actions, footer overflow | `examples/demo/main.c` → `Image List` |
| List: background preview, `ap_reload_background()` | `examples/demo/main.c` → `Background Preview` |
| List: multi-select | `examples/demo/main.c` → `Multi-Select List` |
| List: reorder | `examples/demo/main.c` → `Reorderable List` |
| Options list: standard, keyboard, clickable, color picker, action buttons, long clickable-value truncation | `examples/demo/main.c` → `Options List` |
| Keyboard: general, URL, numeric, UTF-8 / long text editing, custom URL help/shortcuts, URL number-symbol toggle, return-to-mode flow | `examples/demo/main.c` → `Keyboard` |
| Confirmation | `examples/demo/main.c` → `Confirmation` |
| Selection: wrapped prompt layout | `examples/demo/main.c` → `Selection` |
| Process message: progress, cancel, dynamic text | `examples/demo/main.c` → `Process Message`, `Advanced Process` |
| Queue viewer: live snapshot, filter, details, cancel, clear, footer overflow | `examples/demo/main.c` → `Queue Viewer` |
| Detail screen | `examples/demo/main.c` → `Detail Screen` |
| Detail screen: centered title, section separators, key color override | `examples/demo/main.c` → `Detail Screen (Styled)` |
| Detail screen: custom font overrides | `examples/demo/main.c` → `Detail Screen (Fonts)` |
| Color picker | `examples/demo/main.c` → `Color Picker` |
| Help overlay | `examples/demo/main.c` → `Help Overlay` |
| File picker: files, dirs, both mode with hidden files, extension filter, folder creation, live `A` footer hint | `examples/demo/main.c` → `File Picker` |
| Download manager | `examples/download/main.c` |

## Platform Notes

- Desktop-only window visibility actions are shown in `Core API Lab`.
- Device-only theme reload and power-handler toggles are shown conditionally in `Core API Lab`.
- The download demo uses relative destination paths so it remains portable across desktop and device builds.
