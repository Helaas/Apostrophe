/* Real H700 kernel/SDL regression test. The virtual device is removed on exit. */
#include <assert.h>
#include <linux/uinput.h>
#define AP_IMPLEMENTATION
#include "apostrophe.h"

static void send_event(int fd, unsigned short type, unsigned short code, int value) {
    struct input_event event = { .type = type, .code = code, .value = value };
    gettimeofday(&event.time, NULL);
    assert(write(fd, &event, sizeof(event)) == sizeof(event));
}

static void emit(int fd, unsigned short type, unsigned short code, int value) {
    send_event(fd, type, code, value);
    send_event(fd, EV_SYN, SYN_REPORT, 0);
}

static int collect(ap_button expected, bool pressed) {
    int matches = 0, unexpected = 0;
    uint32_t start = SDL_GetTicks();
    do {
        ap_input_event event;
        while (ap_poll_input(&event)) {
            printf("EVENT %s pressed=%d repeated=%d\n", ap_button_name(event.button), event.pressed, event.repeated);
            if (event.pressed == pressed && event.button == expected) matches++;
            else unexpected++;
        }
        SDL_Delay(2);
    } while (SDL_GetTicks() - start < 100);
    assert(unexpected == 0);
    return matches;
}

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    assert(fd >= 0);
    assert(ioctl(fd, UI_SET_EVBIT, EV_KEY) == 0);
    assert(ioctl(fd, UI_SET_KEYBIT, BTN_SOUTH) == 0);
    assert(ioctl(fd, UI_SET_EVBIT, EV_ABS) == 0);
    const int axes[] = { ABS_X, ABS_Y, ABS_RX, ABS_RY };
    struct uinput_user_dev dev = {0};
    snprintf(dev.name, UINPUT_MAX_NAME_SIZE, "Apostrophe PR49 virtual pad");
    dev.id.bustype = BUS_USB;
    dev.id.vendor = 0x045e;
    dev.id.product = 0x028e;
    dev.id.version = 0x0114;
    for (int i = 0; i < 4; i++) {
        assert(ioctl(fd, UI_SET_ABSBIT, axes[i]) == 0);
        dev.absmin[axes[i]] = -32768;
        dev.absmax[axes[i]] = 32767;
    }
    assert(write(fd, &dev, sizeof(dev)) == sizeof(dev));
    assert(ioctl(fd, UI_DEV_CREATE) == 0);
    usleep(300000);

    setenv("PLATFORM", "h700", 1);
    assert(SDL_Init(SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS) == 0);
    SDL_Joystick *joy = NULL;
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (strcmp(SDL_JoystickNameForIndex(i), dev.name) == 0) joy = SDL_JoystickOpen(i);
    }
    assert(joy != NULL);
    printf("SDL opened: %s\n", SDL_JoystickName(joy));
    for (int i = 0; i < 16; i++) ap__g.h700_input_fds[i] = -1;
    ap__g.input_repeat_delay_ms = 300;
    ap__g.input_repeat_rate_ms = 100;
    ap__h700_scan_inputs();
    int builtin_count = 0;
    for (int i = 0; i < 16; i++) {
        int input = ap__g.h700_input_fds[i];
        if (input < 0) continue;
        char name[128] = {0};
        assert(ioctl(input, EVIOCGNAME(sizeof(name)), name) >= 0);
        assert(strcmp(name, dev.name) != 0);
        if (strcmp(name, "ANBERNIC-keys") == 0) builtin_count++;
    }
    assert(builtin_count == 1);
    assert(collect(AP_BTN_NONE, true) == 0);
    emit(fd, EV_KEY, BTN_SOUTH, 1);
    assert(collect(AP_BTN_A, true) == 1);
    emit(fd, EV_KEY, BTN_SOUTH, 0);
    assert(collect(AP_BTN_A, false) == 1);
    emit(fd, EV_ABS, ABS_RX, 32767);
    assert(collect(AP_BTN_NONE, true) == 0);
    emit(fd, EV_ABS, ABS_RX, 0);
    assert(collect(AP_BTN_NONE, true) == 0);
    ap_flip_face_buttons(true);
    emit(fd, EV_KEY, BTN_SOUTH, 1);
    assert(collect(AP_BTN_B, true) == 1);
    emit(fd, EV_KEY, BTN_SOUTH, 0);
    assert(collect(AP_BTN_B, false) == 1);
    ap__h700_close_inputs();
    SDL_JoystickClose(joy);
    SDL_Quit();
    assert(ioctl(fd, UI_DEV_DESTROY) == 0);
    close(fd);
    puts("PASS: one press/release, no right-stick navigation, and flipped A -> B through real uinput/SDL");
    return 0;
}
