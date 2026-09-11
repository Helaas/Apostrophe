/* Linux regression check: build with PLATFORM_NEXTUI and the SDL libraries.
 * Runs without opening a display or reading physical input devices. */
#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>

static const char *input_names[] = { "ANBERNIC-keys", "axp2202-pek", "Xbox Wireless Controller", NULL };
static unsigned closed_inputs;

static int input_index(const char *path) {
    int index = -1;
    if (sscanf(path, "/dev/input/event%d", &index) != 1) return -1;
    return index >= 0 && index < 4 ? index : -1;
}

static int test_access(const char *path, int mode) {
    (void)mode;
    return input_index(path) >= 0 ? 0 : -1;
}

static int test_open(const char *path, int flags, ...) {
    (void)flags;
    int index = input_index(path);
    return index >= 0 ? 100 + index : -1;
}

static int test_close(int fd) {
    assert(fd >= 100 && fd < 104);
    closed_inputs |= 1u << (fd - 100);
    return 0;
}

static int test_ioctl(int fd, unsigned long request, ...) {
    assert(fd >= 100 && fd < 104);
    assert(request == EVIOCGNAME(128));
    const char *name = input_names[fd - 100];
    if (!name) return -1;
    va_list args;
    va_start(args, request);
    strcpy(va_arg(args, char *), name);
    va_end(args);
    return (int)strlen(name) + 1;
}

#define access test_access
#define open test_open
#define close test_close
#define ioctl test_ioctl
#define AP_IMPLEMENTATION
#include "apostrophe.h"

int main(void) {
    assert(SDL_Init(SDL_INIT_TIMER) == 0);
    const char *platforms[] = { "tg5040", "tg5050", "my355", "h700" };
    const ap_button faces[] = { AP_BTN_A, AP_BTN_B, AP_BTN_X, AP_BTN_Y };
    const uint16_t codes[] = { 304, 305, 307, 306 };
    const uint8_t controller[] = { SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B,
                                  SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y };
    const SDL_Keycode keys[] = { SDLK_a, SDLK_b, SDLK_x, SDLK_y };
    const SDL_Scancode scancodes[] = { 44, 224, 225, 226 };

    for (int platform = 0; platform < 4; platform++) {
        setenv("PLATFORM", platforms[platform], 1);
        for (int flipped = 0; flipped < 2; flipped++) {
            ap_flip_face_buttons(flipped);
            for (int face = 0; face < 4; face++) {
                ap_button expected = faces[face ^ flipped];
                uint8_t joy = face < 2 ? (uint8_t)(face ^ (platform != 3)) : (uint8_t)(face ^ 1);
                SDL_KeyboardEvent key = {0};
                key.keysym.sym = keys[face];
                key.keysym.scancode = scancodes[face];
                assert(ap__map_joy_button(joy) == expected);
                assert(ap__map_controller_button(controller[face]) == expected);
                assert(ap__map_key_event(&key) == expected);
                assert(ap__h700_button_from_code(codes[face]) == expected);
            }
            assert(ap__map_joy_button(AP__JOY_BTN_L1) == AP_BTN_L1);
            assert(ap__h700_button_from_code(116) == AP_BTN_POWER);
        }
    }

    for (int i = 0; i < 16; i++) ap__g.h700_input_fds[i] = -1;
    ap__h700_scan_inputs();
    assert(ap__g.h700_input_fds[0] == 100);
    assert(ap__g.h700_input_fds[1] == 101);
    for (int i = 2; i < 16; i++) assert(ap__g.h700_input_fds[i] == -1);
    assert(closed_inputs == ((1u << 2) | (1u << 3)));
    ap__h700_close_inputs();
    assert(closed_inputs == 15);
    SDL_Quit();
    puts("PASS: face-button mappings and H700 input-device filtering");
    return 0;
}
