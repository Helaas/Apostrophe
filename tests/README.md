# Input regression checks

Build both checks with the same universal toolchain used by the examples:

```sh
docker run --rm -v "$PWD":/workspace \
  ghcr.io/loveretro/tg5040-toolchain@sha256:f131c6af64029a8723d0ce8d3c2682642f5f091b04714f6beedda9bec18477ab \
  sh -c 'for name in input h700_uinput; do
    make -C /workspace -f ports/tg5040/Makefile \
      PLATFORM_DEFINE=PLATFORM_NEXTUI EXAMPLE="$name" \
      SRC="/workspace/tests/$name.c" BUILD_DIR=/workspace/build/tests || exit
  done'
```

Copy `build/tests/input` to a supported NextUI device and run it with that
firmware's SDL libraries in `LD_LIBRARY_PATH`. It checks normal and flipped
face-button mappings across all four platform IDs, and mocks device discovery
to verify that H700 keeps its built-in controls and power key while rejecting
external controllers and failed name queries. It does not open a display.

`build/tests/h700_uinput` runs on H700 as root with `/dev/uinput`. Pause the
frontend while running it so the generated A presses cannot activate its menus.
The test creates a temporary USB-style controller, opens it through the real
SDL backend, and verifies one press/release, no navigation from its right stick,
and flipped A-to-B mapping. The virtual device disappears when the process exits.
Resume the frontend afterward, including if the test fails.
