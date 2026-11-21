// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Libwayland-only input hardware demo.
//
// Two independent shm windows, each containing a subsurface panel, plus a
// cursor surface. Everything it observes is printed, and the colours change so
// that pointer focus, keyboard focus and subsurface hit testing are visible on
// a real screen rather than only in the log.
//
// There is no xdg-shell yet, so the compositor cascades new surfaces by a fixed
// step. The demo creates unmapped spacer surfaces between its two windows to
// push them far enough apart not to overlap.

#include <cstdio>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <linux/input-event-codes.h>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

namespace {

constexpr int kWindowWidth = 220;
constexpr int kWindowHeight = 160;
constexpr int kPanelSize = 60;
constexpr int kPanelInset = 30;
constexpr int kCursorSize = 24;
constexpr int kCursorHotspot = 4;

// Between the two windows, so the compositor's cascade separates them.
constexpr int kSpacerSurfaces = 5;

struct Buffer {
    wl_buffer* resource = nullptr;
    uint32_t* pixels = nullptr;
    int width = 0;
    int height = 0;
};

struct Panel {
    wl_surface* surface = nullptr;
    wl_subsurface* subsurface = nullptr;
    Buffer buffer;
    bool hovered = false;
};

struct Window {
    const char* name = "";
    wl_surface* surface = nullptr;
    Buffer buffer;
    Panel panel;
    bool pointerInside = false;
    bool keyboardFocus = false;
};

struct State {
    wl_display* display = nullptr;
    wl_compositor* compositor = nullptr;
    wl_subcompositor* subcompositor = nullptr;
    wl_shm* shm = nullptr;
    wl_seat* seat = nullptr;
    wl_pointer* pointer = nullptr;
    wl_keyboard* keyboard = nullptr;

    Window windows[2];
    wl_surface* cursorSurface = nullptr;
    Buffer cursorBuffer;

    uint32_t lastEnterSerial = 0;
    bool cursorSet = false;

    xkb_context* xkbContext = nullptr;
    xkb_keymap* xkbKeymap = nullptr;
    xkb_state* xkbState = nullptr;

    bool sawPointerEnter = false;
    bool sawPointerMotion = false;
    bool sawPointerButton = false;
    bool sawPanelHit = false;
    bool sawKeyboardEnter = false;
    bool sawKey = false;
    bool failed = false;
};

Buffer createBuffer(State& state, int width, int height)
{
    const int stride = width * 4;
    const size_t size = static_cast<size_t>(stride) * height;
    const int fd = memfd_create("lingmo-input-demo", MFD_CLOEXEC);
    if (fd < 0 || ftruncate(fd, static_cast<off_t>(size)) != 0) {
        std::printf("failed to create a buffer fd\n");
        state.failed = true;
        return {};
    }

    void* memory = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED) {
        std::printf("failed to map a buffer\n");
        state.failed = true;
        close(fd);
        return {};
    }

    wl_shm_pool* pool = wl_shm_create_pool(state.shm, fd, static_cast<int32_t>(size));
    Buffer buffer;
    buffer.resource = wl_shm_pool_create_buffer(pool, 0, width, height, stride,
                                                WL_SHM_FORMAT_XRGB8888);
    buffer.pixels = static_cast<uint32_t*>(memory);
    buffer.width = width;
    buffer.height = height;
    wl_shm_pool_destroy(pool);
    close(fd);
    return buffer;
}

void fill(const Buffer& buffer, uint32_t color)
{
    for (int i = 0; i < buffer.width * buffer.height; ++i)
        buffer.pixels[i] = color;
}

// A border, so the extent of each surface is unmistakable on screen.
void fillWithBorder(const Buffer& buffer, uint32_t body, uint32_t border)
{
    for (int y = 0; y < buffer.height; ++y) {
        for (int x = 0; x < buffer.width; ++x) {
            const bool edge = x < 3 || y < 3 || x >= buffer.width - 3 || y >= buffer.height - 3;
            buffer.pixels[y * buffer.width + x] = edge ? border : body;
        }
    }
}

void redraw(Window& window)
{
    // Keyboard focus is the brighter body; pointer focus is the bright border.
    const uint32_t body = window.keyboardFocus ? 0xff2e6fbd : 0xff1d2a3a;
    const uint32_t border = window.pointerInside ? 0xffffd166 : 0xff44506a;
    fillWithBorder(window.buffer, body, border);
    wl_surface_attach(window.surface, window.buffer.resource, 0, 0);
    wl_surface_damage(window.surface, 0, 0, window.buffer.width, window.buffer.height);

    fill(window.panel.buffer, window.panel.hovered ? 0xffef476f : 0xff06d6a0);
    wl_surface_attach(window.panel.surface, window.panel.buffer.resource, 0, 0);
    wl_surface_damage(window.panel.surface, 0, 0, window.panel.buffer.width,
                      window.panel.buffer.height);
    wl_surface_commit(window.panel.surface);
    wl_surface_commit(window.surface);
}

// Which window and panel a surface belongs to, or nullptr.
Window* windowForSurface(State& state, wl_surface* surface, bool* isPanel)
{
    for (Window& window : state.windows) {
        if (window.surface == surface) {
            *isPanel = false;
            return &window;
        }
        if (window.panel.surface == surface) {
            *isPanel = true;
            return &window;
        }
    }
    return nullptr;
}

const wl_pointer_listener pointerListener = {
    [](void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t x,
       wl_fixed_t y) {
        auto& state = *static_cast<State*>(data);
        state.lastEnterSerial = serial;
        bool isPanel = false;
        Window* window = windowForSurface(state, surface, &isPanel);
        std::printf("pointer: enter serial=%u %s%s at %.2f,%.2f\n", serial,
                    window ? window->name : "unknown", isPanel ? "/panel" : "",
                    wl_fixed_to_double(x), wl_fixed_to_double(y));
        state.sawPointerEnter = true;

        // The cursor image has to be set again for every enter.
        wl_pointer_set_cursor(pointer, serial, state.cursorSurface, kCursorHotspot,
                              kCursorHotspot);
        state.cursorSet = true;

        if (window) {
            window->pointerInside = true;
            window->panel.hovered = isPanel;
            if (isPanel)
                state.sawPanelHit = true;
            redraw(*window);
        }
    },
    [](void* data, wl_pointer*, uint32_t serial, wl_surface* surface) {
        auto& state = *static_cast<State*>(data);
        bool isPanel = false;
        Window* window = windowForSurface(state, surface, &isPanel);
        std::printf("pointer: leave serial=%u %s%s\n", serial, window ? window->name : "unknown",
                    isPanel ? "/panel" : "");
        if (window) {
            window->pointerInside = false;
            window->panel.hovered = false;
            redraw(*window);
        }
    },
    [](void* data, wl_pointer*, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
        auto& state = *static_cast<State*>(data);
        if (!state.sawPointerMotion)
            std::printf("pointer: first motion at %.2f,%.2f time=%u\n", wl_fixed_to_double(x),
                        wl_fixed_to_double(y), time);
        state.sawPointerMotion = true;
    },
    [](void* data, wl_pointer*, uint32_t serial, uint32_t, uint32_t button, uint32_t buttonState) {
        auto& state = *static_cast<State*>(data);
        std::printf("pointer: button serial=%u code=%u %s\n", serial, button,
                    buttonState == WL_POINTER_BUTTON_STATE_PRESSED ? "pressed" : "released");
        state.sawPointerButton = true;
    },
    [](void*, wl_pointer*, uint32_t, uint32_t axis, wl_fixed_t value) {
        std::printf("pointer: axis %u value=%.2f\n", axis, wl_fixed_to_double(value));
    },
    [](void*, wl_pointer*) {},
    [](void*, wl_pointer*, uint32_t source) {
        std::printf("pointer: axis_source %u\n", source);
    },
    [](void*, wl_pointer*, uint32_t, uint32_t axis) {
        std::printf("pointer: axis_stop %u\n", axis);
    },
    [](void*, wl_pointer*, uint32_t axis, int32_t discrete) {
        std::printf("pointer: axis_discrete %u steps=%d\n", axis, discrete);
    },
    nullptr, // axis_value120
    nullptr, // axis_relative_direction
};

const wl_keyboard_listener keyboardListener = {
    [](void* data, wl_keyboard*, uint32_t format, int32_t fd, uint32_t size) {
        auto& state = *static_cast<State*>(data);
        std::printf("keyboard: keymap format=%u size=%u\n", format, size);
        void* memory = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (memory == MAP_FAILED) {
            close(fd);
            state.failed = true;
            return;
        }
        state.xkbKeymap = xkb_keymap_new_from_string(state.xkbContext,
                                                     static_cast<const char*>(memory),
                                                     XKB_KEYMAP_FORMAT_TEXT_V1,
                                                     XKB_KEYMAP_COMPILE_NO_FLAGS);
        munmap(memory, size);
        close(fd);
        if (!state.xkbKeymap) {
            std::printf("keyboard: the keymap did not compile\n");
            state.failed = true;
            return;
        }
        state.xkbState = xkb_state_new(state.xkbKeymap);
        std::printf("keyboard: keymap compiled\n");
    },
    [](void* data, wl_keyboard*, uint32_t serial, wl_surface* surface, wl_array* keys) {
        auto& state = *static_cast<State*>(data);
        bool isPanel = false;
        Window* window = windowForSurface(state, surface, &isPanel);
        std::printf("keyboard: enter serial=%u %s, %zu key(s) held\n", serial,
                    window ? window->name : "unknown", keys->size / sizeof(uint32_t));
        state.sawKeyboardEnter = true;
        if (window) {
            window->keyboardFocus = true;
            redraw(*window);
        }
    },
    [](void* data, wl_keyboard*, uint32_t serial, wl_surface* surface) {
        auto& state = *static_cast<State*>(data);
        bool isPanel = false;
        Window* window = windowForSurface(state, surface, &isPanel);
        std::printf("keyboard: leave serial=%u %s\n", serial, window ? window->name : "unknown");
        if (window) {
            window->keyboardFocus = false;
            redraw(*window);
        }
    },
    [](void* data, wl_keyboard*, uint32_t serial, uint32_t, uint32_t key, uint32_t keyState) {
        auto& state = *static_cast<State*>(data);
        char name[64] = "?";
        if (state.xkbState) {
            const xkb_keysym_t keysym = xkb_state_key_get_one_sym(state.xkbState, key + 8);
            xkb_keysym_get_name(keysym, name, sizeof(name));
        }
        std::printf("keyboard: key serial=%u code=%u sym=%s %s\n", serial, key, name,
                    keyState == WL_KEYBOARD_KEY_STATE_PRESSED ? "pressed" : "released");
        state.sawKey = true;
    },
    [](void* data, wl_keyboard*, uint32_t serial, uint32_t depressed, uint32_t latched,
       uint32_t locked, uint32_t group) {
        auto& state = *static_cast<State*>(data);
        std::printf("keyboard: modifiers serial=%u depressed=%u latched=%u locked=%u group=%u\n",
                    serial, depressed, latched, locked, group);
        if (state.xkbState)
            xkb_state_update_mask(state.xkbState, depressed, latched, locked, 0, 0, group);
    },
    [](void*, wl_keyboard*, int32_t rate, int32_t delay) {
        std::printf("keyboard: repeat_info rate=%d Hz delay=%d ms\n", rate, delay);
    },
};

const wl_seat_listener seatListener = {
    [](void* data, wl_seat* seat, uint32_t capabilities) {
        auto& state = *static_cast<State*>(data);
        std::printf("seat: capabilities pointer=%d keyboard=%d touch=%d\n",
                    !!(capabilities & WL_SEAT_CAPABILITY_POINTER),
                    !!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD),
                    !!(capabilities & WL_SEAT_CAPABILITY_TOUCH));
        if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !state.pointer) {
            state.pointer = wl_seat_get_pointer(seat);
            wl_pointer_add_listener(state.pointer, &pointerListener, &state);
        }
        if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !state.keyboard) {
            state.keyboard = wl_seat_get_keyboard(seat);
            wl_keyboard_add_listener(state.keyboard, &keyboardListener, &state);
        }
    },
    [](void*, wl_seat*, const char* name) {
        std::printf("seat: name %s\n", name);
    },
};

const wl_registry_listener registryListener = {
    [](void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
        auto& state = *static_cast<State*>(data);
        std::printf("global: %s v%u\n", interface, version);
        if (!std::strcmp(interface, "wl_compositor"))
            state.compositor = static_cast<wl_compositor*>(
                wl_registry_bind(registry, name, &wl_compositor_interface, 4));
        if (!std::strcmp(interface, "wl_subcompositor"))
            state.subcompositor = static_cast<wl_subcompositor*>(
                wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
        if (!std::strcmp(interface, "wl_shm"))
            state.shm =
                static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
        if (!std::strcmp(interface, "wl_seat")) {
            state.seat = static_cast<wl_seat*>(
                wl_registry_bind(registry, name, &wl_seat_interface, version < 7 ? version : 7));
            wl_seat_add_listener(state.seat, &seatListener, &state);
        }
    },
    [](void*, wl_registry*, uint32_t) {},
};

void createWindow(State& state, Window& window, const char* name)
{
    window.name = name;
    window.surface = wl_compositor_create_surface(state.compositor);
    window.buffer = createBuffer(state, kWindowWidth, kWindowHeight);

    window.panel.surface = wl_compositor_create_surface(state.compositor);
    window.panel.buffer = createBuffer(state, kPanelSize, kPanelSize);
    window.panel.subsurface = wl_subcompositor_get_subsurface(state.subcompositor,
                                                              window.panel.surface,
                                                              window.surface);
    wl_subsurface_set_position(window.panel.subsurface, kPanelInset, kPanelInset);
    wl_subsurface_set_desync(window.panel.subsurface);

    redraw(window);
}

volatile std::sig_atomic_t g_stop = 0;

void onStopSignal(int)
{
    g_stop = 1;
}

} // namespace

int main()
{
    // The harness reads this from a log file, and may kill the demo at any
    // point; line buffering keeps everything observed up to that moment.
    setvbuf(stdout, nullptr, _IOLBF, 0);
    std::signal(SIGINT, onStopSignal);
    std::signal(SIGTERM, onStopSignal);

    State state;
    state.xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    state.display = wl_display_connect(nullptr);
    if (!state.display) {
        std::printf("failed to connect to the compositor\n");
        return 1;
    }

    wl_registry* registry = wl_display_get_registry(state.display);
    wl_registry_add_listener(registry, &registryListener, &state);
    wl_display_roundtrip(state.display);
    // The seat's capabilities arrive on a second round trip.
    wl_display_roundtrip(state.display);

    if (!state.compositor || !state.subcompositor || !state.shm || !state.seat) {
        std::printf("the compositor is missing a global this demo needs\n");
        return 1;
    }
    if (!state.pointer || !state.keyboard) {
        std::printf("the seat advertised neither a pointer nor a keyboard\n");
        return 1;
    }

    // A cursor image with a hole in the middle at the hotspot, so a
    // misplaced hotspot is obvious.
    state.cursorSurface = wl_compositor_create_surface(state.compositor);
    state.cursorBuffer = createBuffer(state, kCursorSize, kCursorSize);
    for (int y = 0; y < kCursorSize; ++y) {
        for (int x = 0; x < kCursorSize; ++x) {
            const bool tip = x < 10 && y < 10 && (x + y) < 12;
            state.cursorBuffer.pixels[y * kCursorSize + x] = tip ? 0xffffffff : 0xff000000;
        }
    }
    state.cursorBuffer.pixels[kCursorHotspot * kCursorSize + kCursorHotspot] = 0xffff0000;
    wl_surface_attach(state.cursorSurface, state.cursorBuffer.resource, 0, 0);
    wl_surface_damage(state.cursorSurface, 0, 0, kCursorSize, kCursorSize);
    wl_surface_commit(state.cursorSurface);

    createWindow(state, state.windows[0], "left");

    // Push the second window clear of the first: the compositor still places
    // surfaces on a fixed cascade.
    std::vector<wl_surface*> spacers;
    for (int i = 0; i < kSpacerSurfaces; ++i)
        spacers.push_back(wl_compositor_create_surface(state.compositor));

    createWindow(state, state.windows[1], "right");
    wl_display_roundtrip(state.display);

    if (state.failed) {
        std::printf("setup failed\n");
        return 1;
    }

    std::printf("\nready: move the pointer over both windows, click each one to give it\n"
                "keyboard focus, type, and hover the inner panel to check subsurface hit\n"
                "testing. Ctrl-C, or the harness timeout, ends the demo.\n\n");

    pollfd fds = {wl_display_get_fd(state.display), POLLIN, 0};
    while (!g_stop) {
        while (wl_display_prepare_read(state.display) != 0)
            wl_display_dispatch_pending(state.display);
        if (wl_display_flush(state.display) < 0 && errno != EAGAIN) {
            wl_display_cancel_read(state.display);
            break;
        }
        if (poll(&fds, 1, 200) > 0)
            wl_display_read_events(state.display);
        else
            wl_display_cancel_read(state.display);
        if (wl_display_dispatch_pending(state.display) < 0)
            break;
        if (wl_display_get_error(state.display)) {
            std::printf("protocol error: %d\n", wl_display_get_error(state.display));
            state.failed = true;
            break;
        }
    }

    std::printf("\nsummary: pointer_enter=%d motion=%d button=%d panel_hit=%d "
                "keyboard_enter=%d key=%d cursor_set=%d\n",
                state.sawPointerEnter, state.sawPointerMotion, state.sawPointerButton,
                state.sawPanelHit, state.sawKeyboardEnter, state.sawKey, state.cursorSet);

    for (wl_surface* spacer : spacers)
        wl_surface_destroy(spacer);
    if (state.xkbState)
        xkb_state_unref(state.xkbState);
    if (state.xkbKeymap)
        xkb_keymap_unref(state.xkbKeymap);
    xkb_context_unref(state.xkbContext);
    wl_display_disconnect(state.display);
    return state.failed ? 1 : 0;
}
