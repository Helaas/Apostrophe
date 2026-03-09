# Changelog

All notable changes to Apostrophe will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- **Core library** (`apostrophe.h`): init/quit lifecycle, SDL2 window/renderer management, input abstraction (virtual buttons, combos), drawing primitives (text, shapes, pills, images), theming (NextUI integration), font system (6 tiers matching NextUI), status bar, footer, scrollbar, progress bar, screen fade, texture cache, text scrolling, CPU/fan control, power button handling, logging
- **Widget library** (`apostrophe_widgets.h`): list (single/multi-select, reorder, images), options list (cycle/keyboard/clickable/color picker), keyboard (general/URL/numeric), confirmation dialog, selection dialog, process message (async worker with progress), download manager (multi-threaded, requires libcurl), detail screen (info/description/image/table sections), color picker, help overlay
- **Platform support**: TrimUI Smart Pro (`tg5040`), TrimUI Smart Brick (`tg5040`/`tg3040`), TrimUI Smart Pro S (`tg5050`), Miyoo Flip (`my355`), macOS (development)
- **Build system**: GNU Make with Docker-based cross-compilation for all device platforms
- **Examples**: hello (minimal), demo (comprehensive widget showcase), download (status bar + download manager), combo (chord/sequence input), perf (CPU/fan control)
- **Documentation**: Getting Started guide, API reference, Widget catalog, Porting from Gabagool guide, Gabagool parity matrix
