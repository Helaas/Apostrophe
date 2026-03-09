# Contributing to Apostrophe

Thank you for your interest in contributing to Apostrophe.

## Getting Started

1. Fork and clone the repository
2. Install prerequisites (see [Getting Started](docs/GETTING_STARTED.md))
3. Build and run the examples on any supported desktop platform:
   ```bash
   make native
   make run-native-demo
   ```

## Code Style

- **C11** standard
- Public API functions: `ap_` prefix (e.g., `ap_draw_text`)
- Internal functions: `ap__` double-underscore prefix (e.g., `ap__render_pill`)
- Constants and macros: `AP_` prefix, uppercase (e.g., `AP_OK`, `AP_DS()`)
- Enum values: `AP_` prefix, uppercase with underscores (e.g., `AP_BTN_START`)
- Indent with 4 spaces, no tabs
- Brace style: opening brace on the same line
- Label strings in `ap_footer_item` and `ap__button_names[]` are UPPERCASE by convention

## Project Structure

```
include/apostrophe.h          Core library (single header)
include/apostrophe_widgets.h  Widget library (single header)
examples/                     Example applications
docs/                         Documentation
res/                          Fonts and resources
ports/                        Platform-specific Makefiles
```

Both headers are **stb-style single-file libraries**. Define `AP_IMPLEMENTATION` / `AP_WIDGETS_IMPLEMENTATION` in exactly one `.c` file.

## Submitting Changes

1. Create a feature branch from `main`
2. Keep commits focused and descriptive
3. Test on at least one desktop platform (`make native && make run-native-demo`, or the equivalent `make mac`, `make linux`, or `make windows` target)
4. Test on at least one supported device
5. Open a pull request against `main` with a clear description of what and why

## Reporting Issues

- Use GitHub Issues
- Include platform, build target, and steps to reproduce
- Attach log output if applicable (`ap_log` writes to the configured log path)

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).
