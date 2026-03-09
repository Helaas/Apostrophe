# Demo Coverage Matrix

This matrix maps Apostrophe's public API surface to the demos that exercise it.

## Core API Coverage

| API Group | Demo Coverage |
|----------|---------------|
| Lifecycle, renderer/window accessors, logging, error helpers | `examples/demo/main.c` → `Core API Lab` |
| Scaling, content layout, footer/status metrics | `examples/demo/main.c` → `Core API Lab` |
| Drawing primitives, background, status bar, texture cache, text scrolling | `examples/demo/main.c` → `Drawing Primitives`, `Core API Lab` |
| Input delay/repeat, face-button flip, button names, theme color | `examples/demo/main.c` → `Input & Theme` |
| Screen fade | `examples/demo/main.c` → `Screen Fade` |
| Combo registration, polling, callbacks, unregister | `examples/combo/main.c` |
| CPU, fan, power-handler toggle | `examples/demo/main.c` → `CPU & Fan`, `Core API Lab` |

## Widget Coverage

| Widget / Feature | Demo Coverage |
|------------------|---------------|
| List: basic select, help overlay | `examples/demo/main.c` → `Basic List` |
| List: images, primary/secondary/tertiary actions | `examples/demo/main.c` → `Image List` |
| List: multi-select | `examples/demo/main.c` → `Multi-Select List` |
| List: reorder | `examples/demo/main.c` → `Reorderable List` |
| Options list: standard, keyboard, clickable, color picker, action buttons | `examples/demo/main.c` → `Options List` |
| Keyboard: general, URL, numeric, custom URL help/shortcuts | `examples/demo/main.c` → `Keyboard` |
| Confirmation | `examples/demo/main.c` → `Confirmation` |
| Selection | `examples/demo/main.c` → `Selection` |
| Process message: progress, cancel, dynamic text | `examples/demo/main.c` → `Process Message`, `Advanced Process` |
| Detail screen | `examples/demo/main.c` → `Detail Screen` |
| Color picker | `examples/demo/main.c` → `Color Picker` |
| Help overlay | `examples/demo/main.c` → `Help Overlay` |
| Download manager | `examples/download/main.c` |

## Platform Notes

- Desktop-only window visibility actions are shown in `Core API Lab`.
- Device-only theme reload and power-handler toggles are shown conditionally in `Core API Lab`.
- The download demo uses relative destination paths so it remains portable across desktop and device builds.
