// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_keyboard_delegate_unittest.cc
//           pointer_unittest.cc, keyboard_unittest.cc. Real libwayland-client adaptation.
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: exercises the corresponding behavior with native
// compositor test doubles and APIs.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <linux/input-event-codes.h>
#include <map>
#include <mutex>
#include <string>
#include <sys/mman.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>
#include <wayland-server-core.h>
#include <xkbcommon/xkbcommon.h>

#include "base/geometry.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "cursor-shape-v1-client-protocol.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "keyboard-shortcuts-inhibit-unstable-v1-client-protocol.h"
#include "pointer-constraints-unstable-v1-client-protocol.h"
#include "pointer-gestures-unstable-v1-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "shm_test_util.h"
#include "wayland/protocols/cursor_shape.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/protocols/zwp_keyboard_shortcuts_inhibit_manager.h"
#include "wayland/protocols/zwp_pointer_constraints.h"
#include "wayland/protocols/zwp_pointer_gestures.h"
#include "wayland/protocols/zwp_relative_pointer_manager.h"
#include "wayland/serial_tracker.h"
#include "wayland/server.h"
#include "wm/types.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

// Everything the client observed, in order where order is what is under test.
struct Observed {
    uint32_t seatCapabilities = 0;
    std::string seatName;

    std::vector<std::string> pointerEvents;
    wl_surface* enterSurface = nullptr;
    wl_fixed_t enterX = 0;
    wl_fixed_t enterY = 0;
    uint32_t enterSerial = 0;
    uint32_t leaveSerial = 0;
    std::vector<std::pair<wl_fixed_t, wl_fixed_t>> motions;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> buttons; // serial, button, state
    std::vector<std::pair<uint32_t, wl_fixed_t>> axes;             // axis, value
    std::vector<std::pair<uint32_t, int32_t>> axisDiscrete;
    std::vector<std::pair<uint32_t, int32_t>> axisValue120;
    std::vector<uint32_t> axisSources;
    std::vector<uint32_t> axisStops;
    int pointerFrames = 0;

    std::vector<std::string> keyboardEvents;
    std::string keymap;
    uint32_t keymapFormat = 0;
    wl_surface* keyboardEnterSurface = nullptr;
    std::vector<uint32_t> enterKeys;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> keys; // serial, key, state
    std::vector<std::array<uint32_t, 4>> modifiers;
    int32_t repeatRate = -1;
    int32_t repeatDelay = -1;

    // locked, unlocked, confined, unconfined, active, inactive.
    std::vector<std::string> constraintEvents;
    struct Relative {
        uint64_t usec;
        double dx, dy, dxUnaccelerated, dyUnaccelerated;
    };
    std::vector<Relative> relativeMotions;

    // "swipe_begin 3", "pinch_update 1 1 1.5 10", "hold_end 1", ...
    std::vector<std::string> gestureEvents;
    std::vector<uint32_t> gestureSerials;
    wl_surface* gestureSurface = nullptr;

    // "down 0 10 20", "motion 0 15 25", "up 0", "frame", "cancel".
    std::vector<std::string> touchEvents;
    wl_surface* touchSurface = nullptr;
};

struct Client {
    wl_display* display;
    wl_registry* registry;
    wl_compositor* compositor = nullptr;
    wl_subcompositor* subcompositor = nullptr;
    wl_shm* shm = nullptr;
    wl_seat* seat = nullptr;
    wl_pointer* pointer = nullptr;
    wl_keyboard* keyboard = nullptr;
    wl_touch* touch = nullptr;
    zwp_pointer_constraints_v1* pointerConstraints = nullptr;
    zwp_relative_pointer_manager_v1* relativePointerManager = nullptr;
    zwp_keyboard_shortcuts_inhibit_manager_v1* shortcutsInhibitManager = nullptr;
    zwp_pointer_gestures_v1* pointerGestures = nullptr;
    std::map<std::string, std::pair<uint32_t, uint32_t>> globals;
    Observed observed;

    static const wl_pointer_listener pointerListener;
    static const wl_keyboard_listener keyboardListener;
    static const wl_touch_listener touchListener;
    static const wl_seat_listener seatListener;

    explicit Client(int fd, uint32_t seatVersion = wayland::kWlSeatVersion)
        : display(wl_display_connect_to_fd(fd))
    {
        registry = wl_display_get_registry(display);
        static const wl_registry_listener listener = {
            [](void* data, wl_registry*, uint32_t name, const char* interface, uint32_t version) {
                static_cast<Client*>(data)->globals[interface] = {name, version};
            },
            [](void*, wl_registry*, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        wl_display_roundtrip(display);

        compositor = static_cast<wl_compositor*>(wl_registry_bind(
            registry, globals["wl_compositor"].first, &wl_compositor_interface, 4));
        subcompositor = static_cast<wl_subcompositor*>(wl_registry_bind(
            registry, globals["wl_subcompositor"].first, &wl_subcompositor_interface, 1));
        shm = static_cast<wl_shm*>(
            wl_registry_bind(registry, globals["wl_shm"].first, &wl_shm_interface, 1));
        if (globals.contains("wl_seat")) {
            seat = static_cast<wl_seat*>(wl_registry_bind(
                registry, globals["wl_seat"].first, &wl_seat_interface, seatVersion));
            wl_seat_add_listener(seat, &seatListener, this);
            wl_display_roundtrip(display);
        }
        if (globals.contains("zwp_pointer_constraints_v1")) {
            pointerConstraints = static_cast<zwp_pointer_constraints_v1*>(
                wl_registry_bind(registry, globals["zwp_pointer_constraints_v1"].first,
                                 &zwp_pointer_constraints_v1_interface, 1));
        }
        if (globals.contains("zwp_relative_pointer_manager_v1")) {
            relativePointerManager = static_cast<zwp_relative_pointer_manager_v1*>(
                wl_registry_bind(registry, globals["zwp_relative_pointer_manager_v1"].first,
                                 &zwp_relative_pointer_manager_v1_interface, 1));
        }
        if (globals.contains("zwp_pointer_gestures_v1")) {
            pointerGestures = static_cast<zwp_pointer_gestures_v1*>(
                wl_registry_bind(registry, globals["zwp_pointer_gestures_v1"].first,
                                 &zwp_pointer_gestures_v1_interface, 3));
        }
        if (globals.contains("zwp_keyboard_shortcuts_inhibit_manager_v1")) {
            shortcutsInhibitManager = static_cast<zwp_keyboard_shortcuts_inhibit_manager_v1*>(
                wl_registry_bind(registry,
                                 globals["zwp_keyboard_shortcuts_inhibit_manager_v1"].first,
                                 &zwp_keyboard_shortcuts_inhibit_manager_v1_interface, 1));
        }
    }

    ~Client()
    {
        for (wl_proxy* proxy : m_proxies)
            wl_proxy_destroy(proxy);
        if (shortcutsInhibitManager)
            zwp_keyboard_shortcuts_inhibit_manager_v1_destroy(shortcutsInhibitManager);
        if (relativePointerManager)
            zwp_relative_pointer_manager_v1_destroy(relativePointerManager);
        if (pointerConstraints)
            zwp_pointer_constraints_v1_destroy(pointerConstraints);
        if (pointerGestures)
            zwp_pointer_gestures_v1_release(pointerGestures);
        for (wl_surface* surface : m_surfaces)
            wl_surface_destroy(surface);
        for (wl_buffer* buffer : m_buffers)
            wl_buffer_destroy(buffer);
        if (touch)
            wl_touch_destroy(touch);
        if (keyboard)
            wl_keyboard_destroy(keyboard);
        if (pointer)
            wl_pointer_destroy(pointer);
        if (seat)
            wl_seat_destroy(seat);
        wl_shm_destroy(shm);
        wl_subcompositor_destroy(subcompositor);
        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }

    void getPointer()
    {
        pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(pointer, &pointerListener, this);
        sync();
    }

    void getKeyboard()
    {
        keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(keyboard, &keyboardListener, this);
        sync();
    }

    void getTouch()
    {
        touch = wl_seat_get_touch(seat);
        wl_touch_add_listener(touch, &touchListener, this);
        sync();
    }

    // A mapped surface of `size`, kept alive for the lifetime of the client.
    wl_surface* makeSurface(const base::Size& size)
    {
        const int stride = size.width() * 4;
        const size_t bytes = static_cast<size_t>(stride) * size.height();
        const int fd = testsupport::makeMemfd(bytes);
        wl_shm_pool* pool = wl_shm_create_pool(shm, fd, static_cast<int32_t>(bytes));
        wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, size.width(), size.height(),
                                                      stride, WL_SHM_FORMAT_XRGB8888);
        wl_shm_pool_destroy(pool);
        close(fd);

        wl_surface* surface = wl_compositor_create_surface(compositor);
        wl_surface_attach(surface, buffer, 0, 0);
        wl_surface_commit(surface);
        m_buffers.push_back(buffer);
        m_surfaces.push_back(surface);
        return surface;
    }

    // Destroys a surface makeSurface() handed out, so the destructor does not
    // destroy it a second time.
    void destroySurface(wl_surface* surface)
    {
        m_surfaces.erase(std::remove(m_surfaces.begin(), m_surfaces.end(), surface),
                         m_surfaces.end());
        wl_surface_destroy(surface);
    }

    void sync() { ASSERT_GE(wl_display_roundtrip(display), 0); }

    zwp_locked_pointer_v1* lock(wl_surface* surface, wl_region* region, uint32_t lifetime)
    {
        auto* locked = zwp_pointer_constraints_v1_lock_pointer(pointerConstraints, surface,
                                                               pointer, region, lifetime);
        static const zwp_locked_pointer_v1_listener listener = {
            [](void* data, zwp_locked_pointer_v1*) {
                static_cast<Client*>(data)->observed.constraintEvents.push_back("locked");
            },
            [](void* data, zwp_locked_pointer_v1*) {
                static_cast<Client*>(data)->observed.constraintEvents.push_back("unlocked");
            },
        };
        zwp_locked_pointer_v1_add_listener(locked, &listener, this);
        m_proxies.push_back(reinterpret_cast<wl_proxy*>(locked));
        return locked;
    }

    zwp_confined_pointer_v1* confine(wl_surface* surface, wl_region* region, uint32_t lifetime)
    {
        auto* confined = zwp_pointer_constraints_v1_confine_pointer(pointerConstraints, surface,
                                                                    pointer, region, lifetime);
        static const zwp_confined_pointer_v1_listener listener = {
            [](void* data, zwp_confined_pointer_v1*) {
                static_cast<Client*>(data)->observed.constraintEvents.push_back("confined");
            },
            [](void* data, zwp_confined_pointer_v1*) {
                static_cast<Client*>(data)->observed.constraintEvents.push_back("unconfined");
            },
        };
        zwp_confined_pointer_v1_add_listener(confined, &listener, this);
        m_proxies.push_back(reinterpret_cast<wl_proxy*>(confined));
        return confined;
    }

    void getRelativePointer()
    {
        auto* relative =
            zwp_relative_pointer_manager_v1_get_relative_pointer(relativePointerManager, pointer);
        static const zwp_relative_pointer_v1_listener listener = {
            [](void* data, zwp_relative_pointer_v1*, uint32_t high, uint32_t low, wl_fixed_t dx,
               wl_fixed_t dy, wl_fixed_t dxUnaccelerated, wl_fixed_t dyUnaccelerated) {
                Observed& o = static_cast<Client*>(data)->observed;
                o.pointerEvents.push_back("relative_motion");
                o.relativeMotions.push_back({(uint64_t(high) << 32) | low, wl_fixed_to_double(dx),
                                             wl_fixed_to_double(dy),
                                             wl_fixed_to_double(dxUnaccelerated),
                                             wl_fixed_to_double(dyUnaccelerated)});
            },
        };
        zwp_relative_pointer_v1_add_listener(relative, &listener, this);
        m_proxies.push_back(reinterpret_cast<wl_proxy*>(relative));
    }

    static std::string number(wl_fixed_t value)
    {
        std::string text = std::to_string(wl_fixed_to_double(value));
        text.erase(text.find_last_not_of('0') + 1);
        if (text.back() == '.')
            text.pop_back();
        return text;
    }

    void begin(const char* name, uint32_t serial, wl_surface* surface, uint32_t fingers)
    {
        observed.gestureEvents.push_back(std::string(name) + "_begin " + std::to_string(fingers));
        observed.gestureSerials.push_back(serial);
        observed.gestureSurface = surface;
    }

    void end(const char* name, uint32_t serial, int32_t cancelled)
    {
        observed.gestureEvents.push_back(std::string(name) + "_end " + std::to_string(cancelled));
        observed.gestureSerials.push_back(serial);
    }

    void getGestures()
    {
        static const zwp_pointer_gesture_swipe_v1_listener swipeListener = {
            [](void* data, zwp_pointer_gesture_swipe_v1*, uint32_t serial, uint32_t,
               wl_surface* surface, uint32_t fingers) {
                static_cast<Client*>(data)->begin("swipe", serial, surface, fingers);
            },
            [](void* data, zwp_pointer_gesture_swipe_v1*, uint32_t, wl_fixed_t dx, wl_fixed_t dy) {
                static_cast<Client*>(data)->observed.gestureEvents.push_back(
                    "swipe_update " + number(dx) + " " + number(dy));
            },
            [](void* data, zwp_pointer_gesture_swipe_v1*, uint32_t serial, uint32_t,
               int32_t cancelled) { static_cast<Client*>(data)->end("swipe", serial, cancelled); },
        };
        static const zwp_pointer_gesture_pinch_v1_listener pinchListener = {
            [](void* data, zwp_pointer_gesture_pinch_v1*, uint32_t serial, uint32_t,
               wl_surface* surface, uint32_t fingers) {
                static_cast<Client*>(data)->begin("pinch", serial, surface, fingers);
            },
            [](void* data, zwp_pointer_gesture_pinch_v1*, uint32_t, wl_fixed_t dx, wl_fixed_t dy,
               wl_fixed_t scale, wl_fixed_t rotation) {
                static_cast<Client*>(data)->observed.gestureEvents.push_back(
                    "pinch_update " + number(dx) + " " + number(dy) + " " + number(scale) + " "
                    + number(rotation));
            },
            [](void* data, zwp_pointer_gesture_pinch_v1*, uint32_t serial, uint32_t,
               int32_t cancelled) { static_cast<Client*>(data)->end("pinch", serial, cancelled); },
        };
        static const zwp_pointer_gesture_hold_v1_listener holdListener = {
            [](void* data, zwp_pointer_gesture_hold_v1*, uint32_t serial, uint32_t,
               wl_surface* surface, uint32_t fingers) {
                static_cast<Client*>(data)->begin("hold", serial, surface, fingers);
            },
            [](void* data, zwp_pointer_gesture_hold_v1*, uint32_t serial, uint32_t,
               int32_t cancelled) { static_cast<Client*>(data)->end("hold", serial, cancelled); },
        };
        auto* swipe = zwp_pointer_gestures_v1_get_swipe_gesture(pointerGestures, pointer);
        zwp_pointer_gesture_swipe_v1_add_listener(swipe, &swipeListener, this);
        auto* pinch = zwp_pointer_gestures_v1_get_pinch_gesture(pointerGestures, pointer);
        zwp_pointer_gesture_pinch_v1_add_listener(pinch, &pinchListener, this);
        auto* hold = zwp_pointer_gestures_v1_get_hold_gesture(pointerGestures, pointer);
        zwp_pointer_gesture_hold_v1_add_listener(hold, &holdListener, this);
        for (void* proxy : {static_cast<void*>(swipe), static_cast<void*>(pinch),
                            static_cast<void*>(hold)})
            m_proxies.push_back(static_cast<wl_proxy*>(proxy));
        sync();
    }

    zwp_keyboard_shortcuts_inhibitor_v1* inhibitShortcuts(wl_surface* surface)
    {
        auto* inhibitor = zwp_keyboard_shortcuts_inhibit_manager_v1_inhibit_shortcuts(
            shortcutsInhibitManager, surface, seat);
        static const zwp_keyboard_shortcuts_inhibitor_v1_listener listener = {
            [](void* data, zwp_keyboard_shortcuts_inhibitor_v1*) {
                static_cast<Client*>(data)->observed.constraintEvents.push_back("active");
            },
            [](void* data, zwp_keyboard_shortcuts_inhibitor_v1*) {
                static_cast<Client*>(data)->observed.constraintEvents.push_back("inactive");
            },
        };
        zwp_keyboard_shortcuts_inhibitor_v1_add_listener(inhibitor, &listener, this);
        m_proxies.push_back(reinterpret_cast<wl_proxy*>(inhibitor));
        return inhibitor;
    }

    // Destroys a protocol object one of the helpers above handed out.
    void destroy(void* proxy)
    {
        std::erase(m_proxies, static_cast<wl_proxy*>(proxy));
        // Every one of these objects has destroy as request 0.
        wl_proxy_marshal_flags(static_cast<wl_proxy*>(proxy), 0, nullptr,
                               wl_proxy_get_version(static_cast<wl_proxy*>(proxy)),
                               WL_MARSHAL_FLAG_DESTROY);
    }

private:
    std::vector<wl_proxy*> m_proxies;
    std::vector<wl_buffer*> m_buffers;
    std::vector<wl_surface*> m_surfaces;
};

const wl_seat_listener Client::seatListener = {
    [](void* data, wl_seat*, uint32_t capabilities) {
        static_cast<Client*>(data)->observed.seatCapabilities = capabilities;
    },
    [](void* data, wl_seat*, const char* name) {
        static_cast<Client*>(data)->observed.seatName = name;
    },
};

const wl_pointer_listener Client::pointerListener = {
    [](void* data, wl_pointer*, uint32_t serial, wl_surface* surface, wl_fixed_t x,
       wl_fixed_t y) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("enter");
        o.enterSurface = surface;
        o.enterSerial = serial;
        o.enterX = x;
        o.enterY = y;
    },
    [](void* data, wl_pointer*, uint32_t serial, wl_surface*) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("leave");
        o.leaveSerial = serial;
    },
    [](void* data, wl_pointer*, uint32_t, wl_fixed_t x, wl_fixed_t y) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("motion");
        o.motions.emplace_back(x, y);
    },
    [](void* data, wl_pointer*, uint32_t serial, uint32_t, uint32_t button, uint32_t state) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("button");
        o.buttons.emplace_back(serial, button, state);
    },
    [](void* data, wl_pointer*, uint32_t, uint32_t axis, wl_fixed_t value) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("axis");
        o.axes.emplace_back(axis, value);
    },
    [](void* data, wl_pointer*) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("frame");
        ++o.pointerFrames;
    },
    [](void* data, wl_pointer*, uint32_t source) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("axis_source");
        o.axisSources.push_back(source);
    },
    [](void* data, wl_pointer*, uint32_t, uint32_t axis) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("axis_stop");
        o.axisStops.push_back(axis);
    },
    [](void* data, wl_pointer*, uint32_t axis, int32_t discrete) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("axis_discrete");
        o.axisDiscrete.emplace_back(axis, discrete);
    },
    [](void* data, wl_pointer*, uint32_t axis, int32_t value120) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.pointerEvents.push_back("axis_value120");
        o.axisValue120.emplace_back(axis, value120);
    },
    nullptr, // axis_relative_direction (v9)
};

const wl_keyboard_listener Client::keyboardListener = {
    [](void* data, wl_keyboard*, uint32_t format, int32_t fd, uint32_t size) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.keyboardEvents.push_back("keymap");
        o.keymapFormat = format;
        void* memory = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (memory != MAP_FAILED) {
            o.keymap = std::string(static_cast<const char*>(memory));
            munmap(memory, size);
        }
        close(fd);
    },
    [](void* data, wl_keyboard*, uint32_t, wl_surface* surface, wl_array* keys) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.keyboardEvents.push_back("enter");
        o.keyboardEnterSurface = surface;
        o.enterKeys.clear();
        const auto* values = static_cast<const uint32_t*>(keys->data);
        for (size_t i = 0; i < keys->size / sizeof(uint32_t); ++i)
            o.enterKeys.push_back(values[i]);
    },
    [](void* data, wl_keyboard*, uint32_t, wl_surface*) {
        static_cast<Client*>(data)->observed.keyboardEvents.push_back("leave");
    },
    [](void* data, wl_keyboard*, uint32_t serial, uint32_t, uint32_t key, uint32_t state) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.keyboardEvents.push_back("key");
        o.keys.emplace_back(serial, key, state);
    },
    [](void* data, wl_keyboard*, uint32_t, uint32_t depressed, uint32_t latched,
       uint32_t locked, uint32_t group) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.keyboardEvents.push_back("modifiers");
        o.modifiers.push_back({depressed, latched, locked, group});
    },
    [](void* data, wl_keyboard*, int32_t rate, int32_t delay) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.keyboardEvents.push_back("repeat_info");
        o.repeatRate = rate;
        o.repeatDelay = delay;
    },
};

std::string touchPoint(const char* name, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
    return std::string(name) + " " + std::to_string(id) + " "
        + std::to_string(wl_fixed_to_int(x)) + " " + std::to_string(wl_fixed_to_int(y));
}

const wl_touch_listener Client::touchListener = {
    [](void* data, wl_touch*, uint32_t, uint32_t, wl_surface* surface, int32_t id,
       wl_fixed_t x, wl_fixed_t y) {
        Observed& o = static_cast<Client*>(data)->observed;
        o.touchEvents.push_back(touchPoint("down", id, x, y));
        o.touchSurface = surface;
    },
    [](void* data, wl_touch*, uint32_t, uint32_t, int32_t id) {
        static_cast<Client*>(data)->observed.touchEvents.push_back("up " + std::to_string(id));
    },
    [](void* data, wl_touch*, uint32_t, int32_t id, wl_fixed_t x, wl_fixed_t y) {
        static_cast<Client*>(data)->observed.touchEvents.push_back(
            touchPoint("motion", id, x, y));
    },
    [](void* data, wl_touch*) {
        static_cast<Client*>(data)->observed.touchEvents.push_back("frame");
    },
    [](void* data, wl_touch*) {
        static_cast<Client*>(data)->observed.touchEvents.push_back("cancel");
    },
    nullptr, // shape (v6)
    nullptr, // orientation (v6)
};

class SeatProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node* surfaceContainer =
        scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node* cursorContainer =
        scene.root()->addChild(std::make_unique<scene::Node>("cursor"));
    compositor::Compositor compositor {surfaceContainer};
    input::Seat seat {&compositor};
    input::InputManager manager {&seat};
    // Keyboard focus follows window activation, so these tests need a window
    // manager.
    wm::WindowManager windowManager {&compositor, &seat};
    wayland::Server server;

    std::mutex mutex;
    std::function<void()> pendingCheck;

    void SetUp() override
    {
        scene.setSize(base::Size(800, 600));
        seat.setCursorContainer(cursorContainer);
        manager.setBounds(base::Size(800, 600));
        manager.setEventSink(&windowManager);
        windowManager.setOutputBounds(base::Rect(0, 0, 800, 600));
    }

    // Runs `body` on the server thread and waits for it.
    void check(std::function<void()> body)
    {
        std::promise<void> done;
        auto future = done.get_future();
        {
            std::lock_guard lock(mutex);
            pendingCheck = [&] {
                body();
                done.set_value();
            };
        }
        future.wait();
    }

    void run(std::function<void(Client&)> body,
             uint32_t seatVersion = wayland::kWlSeatVersion)
    {
        auto waylandSeat =
            std::make_unique<wayland::WaylandSeat>(&seat, server.serialTracker());
        ASSERT_TRUE(wayland::createCompositorGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSubcompositorGlobal(server.display()));
        ASSERT_TRUE(wayland::createShmGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSeatGlobal(server.display(), waylandSeat.get()));
        ASSERT_TRUE(wayland::createCursorShapeGlobal(server.display()));
        ASSERT_TRUE(wayland::createPointerConstraintsGlobal(server.display(), waylandSeat.get()));
        ASSERT_TRUE(wayland::createRelativePointerManagerGlobal(server.display()));
        ASSERT_TRUE(wayland::createPointerGesturesGlobal(server.display(), waylandSeat.get()));
        ASSERT_TRUE(
            wayland::createKeyboardShortcutsInhibitManagerGlobal(server.display(), waylandSeat.get()));

        int fds[2];
        ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds));
        ASSERT_TRUE(wl_client_create(server.display(), fds[0]));

        std::atomic<bool> done = false;
        std::thread client([&] {
            {
                Client c(fds[1], seatVersion);
                body(c);
            }
            done = true;
        });

        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!done) {
            server.dispatch(std::chrono::milliseconds(1));
            server.flush();
            std::function<void()> work;
            {
                std::lock_guard lock(mutex);
                work.swap(pendingCheck);
            }
            if (work)
                work();
            if (std::chrono::steady_clock::now() > deadline) {
                ADD_FAILURE() << "client timed out";
                shutdown(fds[1], SHUT_RDWR);
                break;
            }
        }
        client.join();
        wl_display_destroy_clients(server.display());
        EXPECT_TRUE(compositor.surfaces().empty());
        // Focus cleanup: nothing may outlive the client's surfaces.
        EXPECT_EQ(nullptr, seat.focusedSurface());
        // A window does not outlive its root surface either.
        EXPECT_TRUE(windowManager.windows().empty());
    }

    // The compositor cascades new surfaces; the tests want known positions.
    void place(size_t index, const base::Point& position)
    {
        ASSERT_LT(index, compositor.surfaces().size());
        compositor.surfaces()[index]->setPosition(position);
    }

    // Gives the surface at `index` a window at `position`, standing in for
    // xdg_toplevel, which these surfaces never take.
    void adopt(size_t index, const base::Point& position)
    {
        ASSERT_LT(index, compositor.surfaces().size());
        compositor::Surface* surface = compositor.surfaces()[index];
        wm::Window* window = windowManager.createWindow(surface);
        window->setBounds(base::Rect(position, window->bounds().size()));
    }

    // Adopts the surface at `index` at `position` and activates its window.
    // Returns where its content is.
    base::Rect activateWindow(size_t index, const base::Point& position)
    {
        base::Rect content;
        check([&] {
            if (!windowManager.windowForSurface(compositor.surfaces()[index]))
                adopt(index, position);
            wm::Window* window = windowManager.windowForSurface(compositor.surfaces()[index]);
            window->activate();
            content = window->contentBounds();
        });
        return content;
    }

    base::PointF pointer()
    {
        base::PointF position;
        check([&] { position = manager.pointerPosition(); });
        return position;
    }
};

} // namespace

TEST_F(SeatProtocolTest, TheRegistryListsWlSeatWithPointerKeyboardAndTouch)
{
    run([](Client& c) {
        ASSERT_TRUE(c.globals.contains("wl_seat"));
        EXPECT_EQ(wayland::kWlSeatVersion, c.globals["wl_seat"].second);
        EXPECT_EQ(uint32_t(WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD
                           | WL_SEAT_CAPABILITY_TOUCH),
                  c.observed.seatCapabilities);
        EXPECT_EQ("default", c.observed.seatName);
    });
}

TEST_F(SeatProtocolTest, PointerEnterMotionButtonAndLeaveAreBatchedIntoFrames)
{
    run([&](Client& c) {
        c.getPointer();
        wl_surface* surface = c.makeSurface(base::Size(100, 80));
        c.sync();
        check([&] { place(0, base::Point(200, 100)); });

        check([&] { seat.onPointerMotion(at(1), base::PointF(210, 110)); });
        c.sync();

        EXPECT_EQ(surface, c.observed.enterSurface);
        EXPECT_EQ(wl_fixed_from_double(10), c.observed.enterX);
        EXPECT_EQ(wl_fixed_from_double(10), c.observed.enterY);
        EXPECT_EQ(std::vector<std::string>({"enter", "frame"}), c.observed.pointerEvents);

        check([&] { seat.onPointerMotion(at(2), base::PointF(215.5, 120)); });
        c.sync();
        ASSERT_EQ(1u, c.observed.motions.size());
        EXPECT_EQ(wl_fixed_from_double(15.5), c.observed.motions[0].first);
        EXPECT_EQ(wl_fixed_from_double(20), c.observed.motions[0].second);

        check([&] {
            seat.onPointerButton(at(3), base::PointF(215.5, 120), BTN_LEFT, true);
            seat.onPointerButton(at(4), base::PointF(215.5, 120), BTN_LEFT, false);
        });
        c.sync();
        ASSERT_EQ(2u, c.observed.buttons.size());
        EXPECT_EQ(uint32_t(BTN_LEFT), std::get<1>(c.observed.buttons[0]));
        EXPECT_EQ(uint32_t(WL_POINTER_BUTTON_STATE_PRESSED), std::get<2>(c.observed.buttons[0]));
        EXPECT_EQ(uint32_t(WL_POINTER_BUTTON_STATE_RELEASED),
                  std::get<2>(c.observed.buttons[1]));

        check([&] { seat.onPointerMotion(at(5), base::PointF(700, 500)); });
        c.sync();
        EXPECT_EQ("leave", c.observed.pointerEvents[c.observed.pointerEvents.size() - 2]);
        EXPECT_EQ("frame", c.observed.pointerEvents.back());
        // enter, motion, 2 buttons, leave: one frame each.
        EXPECT_EQ(5, c.observed.pointerFrames);
    });
}

TEST_F(SeatProtocolTest, TouchPointsStayWithTheSurfaceTheyWentDownOn)
{
    run([&](Client& c) {
        c.getTouch();
        wl_surface* surface = c.makeSurface(base::Size(100, 80));
        c.sync();
        check([&] { place(0, base::Point(200, 100)); });

        check([&] {
            seat.onTouchDown(at(1), 0, base::PointF(210, 120));
            seat.onTouchFrame();
        });
        c.sync();
        EXPECT_EQ(surface, c.observed.touchSurface);

        // Motion off the surface still belongs to it, in its coordinates.
        check([&] {
            seat.onTouchMotion(at(2), 0, base::PointF(500, 400));
            seat.onTouchFrame();
            seat.onTouchUp(at(3), 0);
            seat.onTouchFrame();
        });
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"down 0 10 20", "frame", "motion 0 300 300", "frame",
                                            "up 0", "frame"}),
                  c.observed.touchEvents);

        c.observed.touchEvents.clear();
        check([&] {
            seat.onTouchDown(at(4), 1, base::PointF(250, 150));
            seat.onTouchFrame();
            seat.onTouchCancel();
        });
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"down 1 50 50", "frame", "cancel"}),
                  c.observed.touchEvents);
    });
}

TEST_F(SeatProtocolTest, PointerSerialsAreDistinctAndCarryTheirEventType)
{
    run([&](Client& c) {
        c.getPointer();
        c.makeSurface(base::Size(100, 100));
        c.sync();
        check([&] { place(0, base::Point(0, 0)); });

        check([&] {
            seat.onPointerMotion(at(1), base::PointF(10, 10));
            seat.onPointerButton(at(2), base::PointF(10, 10), BTN_RIGHT, true);
            seat.onPointerButton(at(3), base::PointF(10, 10), BTN_RIGHT, false);
            seat.onPointerMotion(at(4), base::PointF(500, 500));
        });
        c.sync();

        ASSERT_EQ(2u, c.observed.buttons.size());
        const uint32_t enterSerial = c.observed.enterSerial;
        const uint32_t downSerial = std::get<0>(c.observed.buttons[0]);
        const uint32_t upSerial = std::get<0>(c.observed.buttons[1]);
        const uint32_t leaveSerial = c.observed.leaveSerial;

        EXPECT_NE(enterSerial, downSerial);
        EXPECT_NE(downSerial, upSerial);
        EXPECT_NE(upSerial, leaveSerial);

        check([&] {
            using ET = wayland::SerialTracker::EventType;
            wayland::SerialTracker* tracker = server.serialTracker();
            EXPECT_EQ(ET::POINTER_ENTER, tracker->eventType(enterSerial));
            EXPECT_EQ(ET::POINTER_RIGHT_BUTTON_DOWN, tracker->eventType(downSerial));
            EXPECT_EQ(ET::POINTER_RIGHT_BUTTON_UP, tracker->eventType(upSerial));
            EXPECT_EQ(ET::POINTER_LEAVE, tracker->eventType(leaveSerial));
        });
    });
}

TEST_F(SeatProtocolTest, AxisEventsCarrySourceDiscreteAndStop)
{
    run(
        [&](Client& c) {
            c.getPointer();
            c.makeSurface(base::Size(100, 100));
            c.sync();
            check([&] {
                place(0, base::Point(0, 0));
                seat.onPointerMotion(at(1), base::PointF(10, 10));
            });
            c.sync();

            // One wheel click down is 10 axis units and one discrete step.
            check([&] { seat.onPointerAxis(at(2), base::PointF(10, 10), base::PointF(0, 10), true); });
            c.sync();
            ASSERT_EQ(1u, c.observed.axisSources.size());
            EXPECT_EQ(uint32_t(WL_POINTER_AXIS_SOURCE_WHEEL), c.observed.axisSources[0]);
            // Only the axis that moved gets a discrete step.
            ASSERT_EQ(1u, c.observed.axisDiscrete.size());
            EXPECT_EQ(uint32_t(WL_POINTER_AXIS_VERTICAL_SCROLL), c.observed.axisDiscrete[0].first);
            EXPECT_EQ(1, c.observed.axisDiscrete[0].second);
            EXPECT_TRUE(c.observed.axisValue120.empty());
            ASSERT_EQ(2u, c.observed.axes.size());
            EXPECT_EQ(uint32_t(WL_POINTER_AXIS_VERTICAL_SCROLL), c.observed.axes[1].first);
            EXPECT_EQ(wl_fixed_from_double(10), c.observed.axes[1].second);
            // axis_discrete must precede its axis event.
            const auto& events = c.observed.pointerEvents;
            const auto discreteAt = std::find(events.begin(), events.end(), "axis_discrete");
            const auto axisAt = std::find(events.begin(), events.end(), "axis");
            EXPECT_LT(discreteAt - events.begin(), axisAt - events.begin());

            // A finger scroll has no discrete steps and ends with axis_stop.
            check([&] {
                seat.onPointerAxis(at(3), base::PointF(10, 10), base::PointF(-4.5, 2.25), false);
                seat.onPointerAxisStop(at(4), base::PointF(10, 10));
            });
            c.sync();
            ASSERT_EQ(2u, c.observed.axisSources.size() - 1);
            EXPECT_EQ(uint32_t(WL_POINTER_AXIS_SOURCE_FINGER), c.observed.axisSources[1]);
            ASSERT_EQ(4u, c.observed.axes.size());
            EXPECT_EQ(wl_fixed_from_double(-4.5), c.observed.axes[2].second);
            EXPECT_EQ(1u, c.observed.axisDiscrete.size()); // unchanged
            EXPECT_EQ(2u, c.observed.axisStops.size());
        },
        7);
}

TEST_F(SeatProtocolTest, AVersionEightSeatGetsWheelFractionsAsValue120)
{
    run([&](Client& c) {
        ASSERT_EQ(8u, c.globals["wl_seat"].second);
        c.getPointer();
        c.makeSurface(base::Size(100, 100));
        c.sync();
        check([&] {
            place(0, base::Point(0, 0));
            seat.onPointerMotion(at(1), base::PointF(10, 10));
            // An eighth of a click down, then a whole click left.
            seat.onPointerAxis(at(2), base::PointF(10, 10), base::PointF(0, 1.25), true);
            seat.onPointerAxis(at(3), base::PointF(10, 10), base::PointF(-10, 0), true);
        });
        c.sync();
        EXPECT_TRUE(c.observed.axisDiscrete.empty());
        ASSERT_EQ(2u, c.observed.axisValue120.size());
        EXPECT_EQ(uint32_t(WL_POINTER_AXIS_VERTICAL_SCROLL), c.observed.axisValue120[0].first);
        EXPECT_EQ(15, c.observed.axisValue120[0].second);
        EXPECT_EQ(uint32_t(WL_POINTER_AXIS_HORIZONTAL_SCROLL), c.observed.axisValue120[1].first);
        EXPECT_EQ(-120, c.observed.axisValue120[1].second);
        EXPECT_EQ(wl_fixed_from_double(1.25), c.observed.axes[1].second);
        // axis_value120 must precede its axis event.
        const auto& events = c.observed.pointerEvents;
        const auto value120At = std::find(events.begin(), events.end(), "axis_value120");
        const auto axisAt = std::find(events.begin(), events.end(), "axis");
        EXPECT_LT(value120At - events.begin(), axisAt - events.begin());
    });
}

TEST_F(SeatProtocolTest, AnOlderSeatCountsWheelFractionsIntoWholeClicks)
{
    run(
        [&](Client& c) {
            c.getPointer();
            c.makeSurface(base::Size(100, 100));
            c.sync();
            check([&] {
                place(0, base::Point(0, 0));
                seat.onPointerMotion(at(1), base::PointF(10, 10));
                // Seven eighths of a click: no step yet, but every axis value.
                for (int i = 0; i < 7; ++i)
                    seat.onPointerAxis(at(2 + i), base::PointF(10, 10), base::PointF(0, 1.25), true);
            });
            c.sync();
            EXPECT_TRUE(c.observed.axisDiscrete.empty());
            EXPECT_EQ(14u, c.observed.axes.size());

            check([&] {
                seat.onPointerAxis(at(9), base::PointF(10, 10), base::PointF(0, 1.25), true);
            });
            c.sync();
            ASSERT_EQ(1u, c.observed.axisDiscrete.size());
            EXPECT_EQ(1, c.observed.axisDiscrete[0].second);

            // Turning back drops the fraction gathered the other way.
            check([&] {
                seat.onPointerAxis(at(10), base::PointF(10, 10), base::PointF(0, 5), true);
                for (int i = 0; i < 8; ++i)
                    seat.onPointerAxis(at(11 + i), base::PointF(10, 10), base::PointF(0, -1.25), true);
            });
            c.sync();
            ASSERT_EQ(2u, c.observed.axisDiscrete.size());
            EXPECT_EQ(-1, c.observed.axisDiscrete[1].second);
            EXPECT_TRUE(c.observed.axisValue120.empty());
        },
        7);
}

TEST_F(SeatProtocolTest, SubsurfaceHitTestingTargetsTheChild)
{
    run([&](Client& c) {
        wl_surface* parent = c.makeSurface(base::Size(120, 120));
        wl_surface* child = c.makeSurface(base::Size(40, 40));
        wl_subsurface* sub = wl_subcompositor_get_subsurface(c.subcompositor, child, parent);
        wl_subsurface_set_position(sub, 20, 20);
        wl_surface_commit(parent);
        c.getPointer();
        c.sync();
        check([&] { place(0, base::Point(100, 100)); });

        // Over the parent only.
        check([&] { seat.onPointerMotion(at(1), base::PointF(110, 110)); });
        c.sync();
        EXPECT_EQ(parent, c.observed.enterSurface);
        EXPECT_EQ(wl_fixed_from_double(10), c.observed.enterX);

        // Over the subsurface: leave the parent, enter the child, with the
        // location relative to the child.
        check([&] { seat.onPointerMotion(at(2), base::PointF(125, 130)); });
        c.sync();
        EXPECT_EQ(child, c.observed.enterSurface);
        EXPECT_EQ(wl_fixed_from_double(5), c.observed.enterX);
        EXPECT_EQ(wl_fixed_from_double(10), c.observed.enterY);

        wl_subsurface_destroy(sub);
    });
}

TEST_F(SeatProtocolTest, TheCursorSurfaceIsPositionedByItsHotspot)
{
    run([&](Client& c) {
        c.getPointer();
        c.makeSurface(base::Size(200, 200));
        wl_surface* cursor = c.makeSurface(base::Size(24, 24));
        c.sync();
        check([&] {
            place(0, base::Point(0, 0));
            // Out of the way: an unassigned cursor surface is an ordinary
            // surface, and the compositor cascades new ones near the origin.
            place(1, base::Point(600, 400));
            seat.onPointerMotion(at(1), base::PointF(50, 60));
        });
        c.sync();

        wl_pointer_set_cursor(c.pointer, c.observed.enterSerial, cursor, 6, 8);
        c.sync();
        check([&] {
            compositor::Surface* cursorSurface = compositor.surfaces()[1];
            EXPECT_TRUE(cursorSurface->hasRole());
            EXPECT_EQ(cursorContainer, cursorSurface->node()->parent());
            EXPECT_EQ(base::Point(44, 52), cursorSurface->position());
        });

        check([&] { seat.onPointerMotion(at(2), base::PointF(100, 100)); });
        c.sync();
        check([&] { EXPECT_EQ(base::Point(94, 92), compositor.surfaces()[1]->position()); });

        // A null surface hides the cursor.
        wl_pointer_set_cursor(c.pointer, c.observed.enterSerial, nullptr, 0, 0);
        c.sync();
        check([&] { EXPECT_FALSE(compositor.surfaces()[1]->hasRole()); });
    });
}

TEST_F(SeatProtocolTest, KeyboardSendsKeymapRepeatInfoEnterKeysAndModifiers)
{
    run([&](Client& c) {
        c.getKeyboard();
        c.sync();

        // Keymap and repeat_info arrive before anything else.
        ASSERT_GE(c.observed.keyboardEvents.size(), 2u);
        EXPECT_EQ("keymap", c.observed.keyboardEvents[0]);
        EXPECT_EQ("repeat_info", c.observed.keyboardEvents[1]);
        EXPECT_EQ(uint32_t(WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1), c.observed.keymapFormat);
        EXPECT_NE(std::string::npos, c.observed.keymap.find("xkb_keymap"));
        EXPECT_EQ(25, c.observed.repeatRate);
        EXPECT_EQ(500, c.observed.repeatDelay);

        // The keymap the client received must compile.
        xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        xkb_keymap* keymap = xkb_keymap_new_from_string(context, c.observed.keymap.c_str(),
                                                        XKB_KEYMAP_FORMAT_TEXT_V1,
                                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
        EXPECT_NE(nullptr, keymap);
        if (keymap)
            xkb_keymap_unref(keymap);
        xkb_context_unref(context);

        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();

        // Two keys held before focus are reported by enter.
        check([&] {
            seat.onKey(at(1), KEY_A, true);
            seat.onKey(at(2), KEY_LEFTSHIFT, true);
            seat.setFocusedSurface(compositor.surfaces()[0]);
        });
        c.sync();

        EXPECT_EQ(surface, c.observed.keyboardEnterSurface);
        ASSERT_EQ(2u, c.observed.enterKeys.size());
        EXPECT_EQ(uint32_t(KEY_A), c.observed.enterKeys[0]);
        EXPECT_EQ(uint32_t(KEY_LEFTSHIFT), c.observed.enterKeys[1]);
        // Modifiers precede enter, and shift is already down.
        ASSERT_FALSE(c.observed.modifiers.empty());
        EXPECT_NE(0u, c.observed.modifiers.back()[0]);

        check([&] {
            seat.onKey(at(3), KEY_B, true);
            seat.onKey(at(4), KEY_B, false);
        });
        c.sync();
        ASSERT_EQ(2u, c.observed.keys.size());
        EXPECT_EQ(uint32_t(KEY_B), std::get<1>(c.observed.keys[0]));
        EXPECT_EQ(uint32_t(WL_KEYBOARD_KEY_STATE_PRESSED), std::get<2>(c.observed.keys[0]));
        EXPECT_EQ(uint32_t(WL_KEYBOARD_KEY_STATE_RELEASED), std::get<2>(c.observed.keys[1]));
        EXPECT_NE(std::get<0>(c.observed.keys[0]), std::get<0>(c.observed.keys[1]));

        // Releasing shift clears the depressed mask.
        const size_t modifierCount = c.observed.modifiers.size();
        check([&] { seat.onKey(at(5), KEY_LEFTSHIFT, false); });
        c.sync();
        ASSERT_GT(c.observed.modifiers.size(), modifierCount);
        EXPECT_EQ(0u, c.observed.modifiers.back()[0]);

        // Caps Lock arrives in the locked mask, not the latched one.
        check([&] {
            seat.onKey(at(6), KEY_CAPSLOCK, true);
            seat.onKey(at(7), KEY_CAPSLOCK, false);
        });
        c.sync();
        EXPECT_EQ(0u, c.observed.modifiers.back()[1]);
        EXPECT_NE(0u, c.observed.modifiers.back()[2]);

        check([&] { seat.setFocusedSurface(nullptr); });
        c.sync();
        EXPECT_EQ("leave", c.observed.keyboardEvents.back());
    });
}

TEST_F(SeatProtocolTest, ClickingASurfaceGivesItKeyboardFocus)
{
    run([&](Client& c) {
        c.getPointer();
        c.getKeyboard();
        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();
        check([&] { adopt(0, base::Point(0, 0)); });

        check([&] {
            manager.onPointerMotionAbsolute(at(1), base::PointF(50, 50));
            manager.onPointerButton(at(2), BTN_LEFT, true);
        });
        c.sync();

        EXPECT_EQ(surface, c.observed.keyboardEnterSurface);
        check([&] { EXPECT_EQ(compositor.surfaces()[0], seat.focusedSurface()); });

        // The key now reaches the surface that was clicked.
        check([&] { seat.onKey(at(3), KEY_X, true); });
        c.sync();
        ASSERT_EQ(1u, c.observed.keys.size());
        EXPECT_EQ(uint32_t(KEY_X), std::get<1>(c.observed.keys[0]));
    });
}

TEST_F(SeatProtocolTest, DestroyingASurfaceClearsPointerAndKeyboardFocus)
{
    run([&](Client& c) {
        c.getPointer();
        c.getKeyboard();
        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();
        check([&] { adopt(0, base::Point(0, 0)); });

        check([&] {
            manager.onPointerMotionAbsolute(at(1), base::PointF(50, 50));
            manager.onPointerButton(at(2), BTN_LEFT, true);
        });
        c.sync();
        ASSERT_EQ(surface, c.observed.enterSurface);

        const size_t pointerEvents = c.observed.pointerEvents.size();
        c.destroySurface(surface);
        c.sync();

        // A leave still reaches the client: the surface is destroyed while its
        // resource is alive, which is what Exo does.
        ASSERT_GT(c.observed.pointerEvents.size(), pointerEvents);
        EXPECT_EQ("leave", c.observed.pointerEvents[pointerEvents]);
        EXPECT_EQ("leave", c.observed.keyboardEvents.back());

        check([&] {
            EXPECT_EQ(nullptr, seat.focusedSurface());
            EXPECT_TRUE(compositor.surfaces().empty());
        });

        // Events after the surface is gone reach nobody and crash nothing.
        check([&] {
            seat.onPointerMotion(at(3), base::PointF(60, 60));
            seat.onKey(at(4), KEY_A, true);
            seat.onKey(at(5), KEY_A, false);
        });
        c.sync();
    });
}

TEST_F(SeatProtocolTest, AVersionFourSeatGetsNoFrameAxisSourceOrDiscrete)
{
    // wl_pointer.frame, axis_source, axis_stop and axis_discrete are all
    // version 5; a version 4 client must see none of them.
    run(
        [&](Client& c) {
            c.getPointer();
            c.getKeyboard();
            c.makeSurface(base::Size(100, 100));
            c.sync();
            check([&] {
                place(0, base::Point(0, 0));
                seat.onPointerMotion(at(1), base::PointF(10, 10));
                seat.onPointerAxis(at(2), base::PointF(10, 10), base::PointF(0, 10), true);
                seat.onPointerAxisStop(at(3), base::PointF(10, 10));
            });
            c.sync();

            EXPECT_EQ(0, c.observed.pointerFrames);
            EXPECT_TRUE(c.observed.axisSources.empty());
            EXPECT_TRUE(c.observed.axisDiscrete.empty());
            EXPECT_TRUE(c.observed.axisStops.empty());
            EXPECT_EQ(2u, c.observed.axes.size());
            // repeat_info is version 4, so it is still sent.
            EXPECT_EQ(25, c.observed.repeatRate);
        },
        4);
}

// Exo's SetCursorType / SetCursorAndSetCursorType behavior, driven through the
// standard protocol with per-pointer enter-serial and resource lifetime checks.
TEST_F(SeatProtocolTest, CursorShapeReplacesSurfaceAndHideAndRejectsOldSerial)
{
    run([&](Client& c) {
        ASSERT_FALSE(c.globals.contains("zcr_cursor_shapes_v1"));
        ASSERT_EQ(c.globals["wp_cursor_shape_manager_v1"].second, 1u);
        auto* manager = static_cast<wp_cursor_shape_manager_v1*>(
            wl_registry_bind(c.registry, c.globals["wp_cursor_shape_manager_v1"].first,
                             &wp_cursor_shape_manager_v1_interface, 1));
        c.getPointer();
        auto* device = wp_cursor_shape_manager_v1_get_pointer(manager, c.pointer);
        c.makeSurface({200, 200});
        auto* image = c.makeSurface({24, 24});
        c.sync();
        check([&] {
            place(0, {0, 0});
            place(1, {600, 400});
        });
        wp_cursor_shape_device_v1_set_shape(device, 0, WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT);
        c.sync();
        check([&] {
            EXPECT_FALSE(seat.hasClientCursorRequest());
            seat.onPointerMotion(at(1), {50, 50});
        });
        c.sync();
        const auto serial = c.observed.enterSerial;
        wl_pointer_set_cursor(c.pointer, serial, image, 6, 8);
        c.sync();
        check([&] {
            EXPECT_TRUE(seat.hasClientCursor());
        });
        wp_cursor_shape_device_v1_set_shape(device, serial, WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT);
        c.sync();
        check([&] {
            EXPECT_EQ(seat.clientCursorShape(), input::CursorShape::Text);
            EXPECT_FALSE(seat.hasClientCursor());
            EXPECT_EQ(compositor.surfaces()[1]->node()->parent(), nullptr);
        });
        wl_pointer_set_cursor(c.pointer, serial, nullptr, 0, 0);
        c.sync();
        check([&] {
            EXPECT_TRUE(seat.hasClientCursorRequest());
            EXPECT_FALSE(seat.clientCursorShape());
        });
        wp_cursor_shape_device_v1_set_shape(device, serial,
                                            WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_POINTER);
        c.sync();
        check([&] {
            EXPECT_EQ(seat.clientCursorShape(), input::CursorShape::Pointer);
        });
        wl_pointer_set_cursor(c.pointer, serial, image, 3, 4);
        c.sync();
        check([&] {
            EXPECT_TRUE(seat.hasClientCursor());
            EXPECT_FALSE(seat.clientCursorShape());
            seat.onPointerMotion(at(2), {400, 300});
        });
        c.sync();
        check([&] {
            EXPECT_FALSE(seat.hasClientCursorRequest());
            seat.onPointerMotion(at(3), {50, 50});
        });
        c.sync();
        ASSERT_NE(c.observed.enterSerial, serial);
        wp_cursor_shape_device_v1_set_shape(device, serial, WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT);
        wl_pointer_set_cursor(c.pointer, serial, image, 0, 0);
        c.sync();
        check([&] {
            EXPECT_FALSE(seat.hasClientCursorRequest());
        });
        wp_cursor_shape_device_v1_set_shape(device, c.observed.enterSerial,
                                            WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT);
        c.sync();
        check([&] {
            EXPECT_EQ(seat.clientCursorShape(), input::CursorShape::Wait);
        });
        // Destroying manager or shape-device must not reset the chosen cursor.
        wp_cursor_shape_manager_v1_destroy(manager);
        wp_cursor_shape_device_v1_destroy(device);
        c.sync();
        check([&] {
            EXPECT_EQ(seat.clientCursorShape(), input::CursorShape::Wait);
        });
    });
}

TEST_F(SeatProtocolTest, CursorShapeDeviceBecomesInertAfterPointerRelease)
{
    run([&](Client& c) {
        auto* manager = static_cast<wp_cursor_shape_manager_v1*>(
            wl_registry_bind(c.registry, c.globals["wp_cursor_shape_manager_v1"].first,
                             &wp_cursor_shape_manager_v1_interface, 1));
        c.getPointer();
        auto* first = wp_cursor_shape_manager_v1_get_pointer(manager, c.pointer);
        auto* second = wp_cursor_shape_manager_v1_get_pointer(manager, c.pointer);
        c.makeSurface({200, 200});
        c.sync();
        check([&] {
            place(0, {0, 0});
            seat.onPointerMotion(at(1), {50, 50});
        });
        c.sync();
        wp_cursor_shape_device_v1_set_shape(first, c.observed.enterSerial,
                                            WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT);
        c.sync();
        check([&] {
            EXPECT_EQ(seat.clientCursorShape(), input::CursorShape::Text);
        });
        wl_pointer_release(c.pointer);
        c.pointer = nullptr;
        wp_cursor_shape_device_v1_set_shape(second, c.observed.enterSerial,
                                            WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT);
        c.sync();
        check([&] {
            EXPECT_FALSE(seat.hasClientCursorRequest());
        });
        wp_cursor_shape_device_v1_destroy(first);
        wp_cursor_shape_device_v1_destroy(second);
        wp_cursor_shape_manager_v1_destroy(manager);
    });
}

TEST_F(SeatProtocolTest, CursorShapeRejectsInvalidShapeEvenWithoutFocus)
{
    run([&](Client& c) {
        auto* manager = static_cast<wp_cursor_shape_manager_v1*>(
            wl_registry_bind(c.registry, c.globals["wp_cursor_shape_manager_v1"].first,
                             &wp_cursor_shape_manager_v1_interface, 1));
        c.getPointer();
        auto* device = wp_cursor_shape_manager_v1_get_pointer(manager, c.pointer);
        wp_cursor_shape_device_v1_set_shape(device, 0, 0);
        EXPECT_EQ(wl_display_roundtrip(c.display), -1);
        const wl_interface* interface = nullptr;
        EXPECT_EQ(wl_display_get_protocol_error(c.display, &interface, nullptr),
                  uint32_t(WP_CURSOR_SHAPE_DEVICE_V1_ERROR_INVALID_SHAPE));
        ASSERT_NE(interface, nullptr);
        EXPECT_STREQ(interface->name, "wp_cursor_shape_device_v1");
        wp_cursor_shape_device_v1_destroy(device);
        wp_cursor_shape_manager_v1_destroy(manager);
    });
}

TEST_F(SeatProtocolTest, NormalWindowManagerDeliversEveryFunctionKeyPair)
{
    run([&](Client& c) {
        c.getKeyboard();
        c.makeSurface(base::Size(100, 100));
        c.sync();
        check([&] {
            adopt(0, base::Point(100, 100));
            auto* window = windowManager.windows().front();
            window->activate();
            const base::Rect bounds = window->bounds();
            for (uint32_t key = KEY_F1; key <= KEY_F9; ++key) {
                manager.onKey(at(1), key, true);
                manager.onKey(at(2), key, false);
                EXPECT_FALSE(seat.keySuppressed(key));
            }
            EXPECT_EQ(bounds, window->bounds());
            EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());
            EXPECT_EQ(window->rootSurface(), seat.focusedSurface());
        });
        c.sync();
        ASSERT_EQ(18u, c.observed.keys.size());
        for (size_t i = 0; i < 9; ++i) {
            EXPECT_EQ(KEY_F1 + i, std::get<1>(c.observed.keys[2 * i]));
            EXPECT_EQ(KEY_F1 + i, std::get<1>(c.observed.keys[2 * i + 1]));
            EXPECT_EQ(uint32_t(WL_KEYBOARD_KEY_STATE_PRESSED), std::get<2>(c.observed.keys[2 * i]));
            EXPECT_EQ(uint32_t(WL_KEYBOARD_KEY_STATE_RELEASED),
                      std::get<2>(c.observed.keys[2 * i + 1]));
        }
    });
}

TEST_F(SeatProtocolTest, ALockHoldsThePointerAndRelativeMotionGoesOn)
{
    run([&](Client& c) {
        c.getPointer();
        c.getRelativePointer();
        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();
        const base::Rect content = activateWindow(0, base::Point(100, 100));
        check([&] { manager.onPointerMotionAbsolute(at(1), base::PointF(wm::centerPixel(content))); });
        c.lock(surface, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"locked"}), c.observed.constraintEvents);

        const size_t motions = c.observed.motions.size();
        c.observed.pointerEvents.clear();
        check([&] { manager.onPointerMotion(at(2), base::PointF(30, -20), base::PointF(10, -5)); });
        c.sync();
        EXPECT_TRUE(base::fuzzyCompare(base::PointF(wm::centerPixel(content)), pointer())) << pointer();
        EXPECT_EQ(motions, c.observed.motions.size());
        EXPECT_EQ(std::vector<std::string>({"relative_motion", "frame"}),
                  c.observed.pointerEvents);
        ASSERT_EQ(1u, c.observed.relativeMotions.size());
        const auto& relative = c.observed.relativeMotions[0];
        // Microseconds, where Exo sends milliseconds.
        EXPECT_EQ(2000u, relative.usec);
        EXPECT_EQ(30, relative.dx);
        EXPECT_EQ(-20, relative.dy);
        EXPECT_EQ(10, relative.dxUnaccelerated);
        EXPECT_EQ(-5, relative.dyUnaccelerated);
    });
}

TEST_F(SeatProtocolTest, BreakingALockWarpsToItsHintAndOnlyAClickLocksAgain)
{
    run([&](Client& c) {
        c.getPointer();
        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();
        const base::Rect content = activateWindow(0, base::Point(100, 100));
        check([&] { manager.onPointerMotionAbsolute(at(1), base::PointF(wm::centerPixel(content))); });
        auto* locked = c.lock(surface, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        zwp_locked_pointer_v1_set_cursor_position_hint(locked, wl_fixed_from_int(20),
                                                       wl_fixed_from_int(30));
        c.sync();
        // The hint is double-buffered.
        check([&] {
            EXPECT_TRUE(seat.breakInputCapture());
            EXPECT_EQ(base::PointF(wm::centerPixel(content)), manager.pointerPosition());
            EXPECT_EQ(nullptr, seat.pointerConstraints()->active());
        });
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"locked", "unlocked"}), c.observed.constraintEvents);

        // Not even the pointer re-entering the region takes it back.
        check([&] {
            manager.onPointerMotion(at(2), base::PointF(1, 1));
            EXPECT_EQ(nullptr, seat.pointerConstraints()->active());
            manager.onPointerButton(at(3), BTN_LEFT, true);
            manager.onPointerButton(at(4), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"locked", "unlocked", "locked"}),
                  c.observed.constraintEvents);

        wl_surface_commit(surface);
        c.sync();
        const size_t motions = c.observed.motions.size();
        check([&] { EXPECT_TRUE(seat.breakInputCapture()); });
        c.sync();
        EXPECT_TRUE(base::fuzzyCompare(base::PointF(content.topLeft() + base::Point(20, 30)), pointer()))
            << pointer();
        // The client hears where the pointer went.
        ASSERT_EQ(motions + 1, c.observed.motions.size());
        EXPECT_EQ(wl_fixed_from_int(20), c.observed.motions.back().first);
        EXPECT_EQ(wl_fixed_from_int(30), c.observed.motions.back().second);
        check([&] {
            manager.onPointerMotion(at(5), base::PointF(5, 5));
            EXPECT_EQ(base::PointF(content.topLeft() + base::Point(25, 35)), manager.pointerPosition());
        });
    });
}

TEST_F(SeatProtocolTest, OnlyTheActiveWindowLocksAndAOneshotLockEndsWithIt)
{
    run([&](Client& c) {
        c.getPointer();
        wl_surface* first = c.makeSurface(base::Size(100, 100));
        c.makeSurface(base::Size(100, 100));
        c.sync();
        activateWindow(1, base::Point(400, 100));
        check([&] {
            adopt(0, base::Point(100, 100));
            manager.onPointerMotionAbsolute(
                at(1),
                base::PointF(wm::centerPixel(windowManager.windowForSurface(compositor.surfaces()[0])->contentBounds())));
        });
        c.lock(first, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);
        c.sync();
        EXPECT_TRUE(c.observed.constraintEvents.empty());

        activateWindow(0, base::Point());
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"locked"}), c.observed.constraintEvents);

        activateWindow(1, base::Point());
        c.sync();
        activateWindow(0, base::Point());
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"locked", "unlocked"}), c.observed.constraintEvents);
        check([&] { EXPECT_EQ(nullptr, seat.pointerConstraints()->active()); });
    });
}

TEST_F(SeatProtocolTest, AConfinementKeepsThePointerInsideItsRegion)
{
    run([&](Client& c) {
        c.getPointer();
        c.getRelativePointer();
        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();
        const base::Rect content = activateWindow(0, base::Point(100, 100));
        const base::PointF origin = content.topLeft();
        check([&] { manager.onPointerMotionAbsolute(at(1), origin + base::PointF(5, 5)); });
        wl_region* region = wl_compositor_create_region(c.compositor);
        wl_region_add(region, 10, 10, 50, 40);
        auto* confined =
            c.confine(surface, region, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        wl_region_destroy(region);
        c.sync();
        // Outside the region it waits for the pointer to come in.
        EXPECT_TRUE(c.observed.constraintEvents.empty());
        check([&] { manager.onPointerMotion(at(2), base::PointF(15, 15)); });
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"confined"}), c.observed.constraintEvents);

        check([&] { manager.onPointerMotion(at(3), base::PointF(100, 100)); });
        c.sync();
        base::PointF local = pointer() - origin;
        EXPECT_NEAR(60, local.x(), 0.01);
        EXPECT_NEAR(50, local.y(), 0.01);
        EXPECT_LT(local.x(), 60);
        EXPECT_EQ(100, c.observed.relativeMotions.back().dx);
        // It still hits the surface: the pointer never left it.
        EXPECT_EQ(std::vector<std::string>({"confined"}), c.observed.constraintEvents);

        check([&] { manager.onPointerMotion(at(4), base::PointF(-200, 0)); });
        local = pointer() - origin;
        EXPECT_EQ(10, local.x());

        // A null region is the whole input region, from the next commit.
        zwp_confined_pointer_v1_set_region(confined, nullptr);
        c.sync();
        check([&] { manager.onPointerMotion(at(5), base::PointF(-200, 0)); });
        EXPECT_EQ(10, (pointer() - origin).x());
        wl_surface_commit(surface);
        c.sync();
        check([&] { manager.onPointerMotion(at(6), base::PointF(-200, 0)); });
        EXPECT_EQ(0, (pointer() - origin).x());
    });
}

TEST_F(SeatProtocolTest, DestroyingTheSurfaceOrTheConstraintReleasesThePointer)
{
    run([&](Client& c) {
        c.getPointer();
        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();
        const base::Rect content = activateWindow(0, base::Point(100, 100));
        check([&] { manager.onPointerMotionAbsolute(at(1), base::PointF(wm::centerPixel(content))); });
        auto* locked = c.lock(surface, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        c.sync();
        ASSERT_EQ(std::vector<std::string>({"locked"}), c.observed.constraintEvents);
        // Destroying the lock is how a client unlocks, so its hint still applies.
        zwp_locked_pointer_v1_set_cursor_position_hint(locked, wl_fixed_from_int(10),
                                                       wl_fixed_from_int(20));
        wl_surface_commit(surface);
        c.destroy(locked);
        c.sync();
        check([&] {
            EXPECT_FALSE(seat.pointerConstraints()->isLocked());
            EXPECT_EQ(base::PointF(content.topLeft() + base::Point(10, 20)), manager.pointerPosition());
            manager.onPointerMotion(at(2), base::PointF(10, 0));
            EXPECT_EQ(base::PointF(content.topLeft() + base::Point(20, 20)), manager.pointerPosition());
        });

        c.lock(surface, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        c.sync();
        c.destroySurface(surface);
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"locked", "locked", "unlocked"}),
                  c.observed.constraintEvents);
        check([&] { EXPECT_FALSE(seat.pointerConstraints()->isLocked()); });
    });
}

// Exo's UAFViaDuplicatePreTargetHandlerOnSubsurfaces: persistent locks on
// several subsurfaces of the active window, broken by the user, then the
// wl_pointer released while they still exist, and input keeps flowing.
TEST_F(SeatProtocolTest, LocksOnSeveralSubsurfacesSurviveBreakingAndPointerRelease)
{
    run([&](Client& c) {
        c.getPointer();
        wl_surface* parent = c.makeSurface(base::Size(120, 120));
        std::vector<wl_subsurface*> subs;
        std::vector<wl_surface*> children;
        for (int i = 0; i < 3; ++i) {
            children.push_back(c.makeSurface(base::Size(20, 20)));
            subs.push_back(wl_subcompositor_get_subsurface(c.subcompositor, children.back(), parent));
            wl_subsurface_set_position(subs.back(), 10 + i * 30, 10);
        }
        wl_surface_commit(parent);
        c.sync();
        const base::Rect content = activateWindow(0, base::Point(100, 100));
        check([&] { manager.onPointerMotionAbsolute(at(1), base::PointF(content.topLeft() + base::Point(15, 15))); });
        for (wl_surface* child : children)
            c.lock(child, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        c.sync();
        EXPECT_EQ(0, wl_display_get_error(c.display));

        check([&] { seat.breakInputCapture(); });
        c.sync();
        wl_pointer_release(c.pointer);
        c.pointer = nullptr;
        c.sync();
        check([&] {
            manager.onPointerMotionAbsolute(at(2), base::PointF(50, 50));
            manager.onPointerMotion(at(3), base::PointF(5, 5));
            manager.onPointerButton(at(4), BTN_LEFT, true);
            manager.onPointerButton(at(5), BTN_LEFT, false);
            EXPECT_FALSE(seat.pointerConstraints()->isLocked());
        });
        c.sync();
        for (wl_subsurface* sub : subs)
            wl_subsurface_destroy(sub);
        for (wl_surface* child : children)
            c.destroySurface(child);
        c.sync();
        check([&] { manager.onPointerMotion(at(6), base::PointF(5, 5)); });
        c.sync();
    });
}

// Exo's ConstrainPointerWithUncommittedShellSurface: a lock on a surface with
// no window yet stays inactive, and tearing it down is safe.
TEST_F(SeatProtocolTest, ALockOnASurfaceWithoutAWindowStaysInactive)
{
    run([&](Client& c) {
        c.getPointer();
        wl_surface* surface = wl_compositor_create_surface(c.compositor);
        c.lock(surface, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        c.sync();
        check([&] {
            manager.onPointerMotionAbsolute(at(1), base::PointF(10, 10));
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
            EXPECT_EQ(nullptr, seat.pointerConstraints()->active());
        });
        c.sync();
        EXPECT_TRUE(c.observed.constraintEvents.empty());
        wl_surface_destroy(surface);
        c.sync();
        check([&] { manager.onPointerMotion(at(4), base::PointF(5, 5)); });
        c.sync();
        EXPECT_EQ(0, wl_display_get_error(c.display));
    });
}

TEST_F(SeatProtocolTest, ASecondConstraintOnASurfaceIsAProtocolError)
{
    run([&](Client& c) {
        c.getPointer();
        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();
        c.lock(surface, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        c.confine(surface, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);
        EXPECT_LT(wl_display_roundtrip(c.display), 0);
        const wl_interface* interface = nullptr;
        EXPECT_EQ(uint32_t(ZWP_POINTER_CONSTRAINTS_V1_ERROR_ALREADY_CONSTRAINED),
                  wl_display_get_protocol_error(c.display, &interface, nullptr));
        EXPECT_EQ(&zwp_pointer_constraints_v1_interface, interface);
    });
}

TEST_F(SeatProtocolTest, AShortcutsInhibitorIsActiveWithKeyboardFocus)
{
    run([&](Client& c) {
        c.getPointer();
        c.getKeyboard();
        wl_surface* first = c.makeSurface(base::Size(100, 100));
        c.makeSurface(base::Size(100, 100));
        c.sync();
        activateWindow(1, base::Point(400, 100));
        const base::Rect content = activateWindow(0, base::Point(100, 100));
        c.inhibitShortcuts(first);
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"active"}), c.observed.constraintEvents);

        activateWindow(1, base::Point());
        activateWindow(0, base::Point());
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"active", "inactive", "active"}),
                  c.observed.constraintEvents);

        // Taken back by the user, it stays inactive until a click on the window.
        check([&] {
            EXPECT_TRUE(seat.breakInputCapture());
            EXPECT_FALSE(seat.shortcutsInhibition()->isActive());
        });
        activateWindow(1, base::Point());
        activateWindow(0, base::Point());
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"active", "inactive", "active", "inactive"}),
                  c.observed.constraintEvents);
        check([&] {
            manager.onPointerMotionAbsolute(at(1), base::PointF(wm::centerPixel(content)));
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
            EXPECT_TRUE(seat.shortcutsInhibition()->isActive());
        });
        c.sync();
        EXPECT_EQ("active", c.observed.constraintEvents.back());

        c.inhibitShortcuts(first);
        EXPECT_LT(wl_display_roundtrip(c.display), 0);
        const wl_interface* interface = nullptr;
        EXPECT_EQ(uint32_t(ZWP_KEYBOARD_SHORTCUTS_INHIBIT_MANAGER_V1_ERROR_ALREADY_INHIBITED),
                  wl_display_get_protocol_error(c.display, &interface, nullptr));
        EXPECT_EQ(&zwp_keyboard_shortcuts_inhibit_manager_v1_interface, interface);
    });
}

TEST_F(SeatProtocolTest, SwipePinchAndHoldGesturesReachTheSurfaceUnderThePointer)
{
    run([&](Client& c) {
        ASSERT_EQ(3u, c.globals["zwp_pointer_gestures_v1"].second);
        c.getPointer();
        c.getGestures();
        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();
        const base::Rect content = activateWindow(0, base::Point(100, 100));
        check([&] {
            manager.onPointerMotionAbsolute(at(1), base::PointF(wm::centerPixel(content)));
            manager.onPointerGestureBegin(at(2), input::GestureType::Swipe, 3);
            manager.onPointerGestureUpdate(at(3), input::GestureType::Swipe, base::PointF(4, -2),
                                           base::PointF(4, -2), 1, 0);
            manager.onPointerGestureEnd(at(4), input::GestureType::Swipe, false);
            manager.onPointerGestureBegin(at(5), input::GestureType::Pinch, 2);
            manager.onPointerGestureUpdate(at(6), input::GestureType::Pinch, base::PointF(1, 1),
                                           base::PointF(1, 1), 1.5, 10);
            manager.onPointerGestureEnd(at(7), input::GestureType::Pinch, true);
            manager.onPointerGestureBegin(at(8), input::GestureType::Hold, 2);
            manager.onPointerGestureEnd(at(9), input::GestureType::Hold, false);
            // The pointer does not move with a gesture.
            EXPECT_EQ(base::PointF(wm::centerPixel(content)), manager.pointerPosition());
        });
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"swipe_begin 3", "swipe_update 4 -2", "swipe_end 0",
                                            "pinch_begin 2", "pinch_update 1 1 1.5 10",
                                            "pinch_end 1", "hold_begin 2", "hold_end 0"}),
                  c.observed.gestureEvents);
        EXPECT_EQ(surface, c.observed.gestureSurface);
        ASSERT_EQ(6u, c.observed.gestureSerials.size());
        for (size_t i = 1; i < c.observed.gestureSerials.size(); ++i)
            EXPECT_GT(c.observed.gestureSerials[i], c.observed.gestureSerials[i - 1]);
    });
}

TEST_F(SeatProtocolTest, AGestureEndsCancelledWhenItsSurfaceGoesAndStartsNowhereElse)
{
    run([&](Client& c) {
        c.getPointer();
        c.getGestures();
        wl_surface* surface = c.makeSurface(base::Size(100, 100));
        c.sync();
        const base::Rect content = activateWindow(0, base::Point(100, 100));
        check([&] {
            // Nothing under the pointer: nothing begins, and nothing ends.
            manager.onPointerMotionAbsolute(at(1), base::PointF(700, 500));
            manager.onPointerGestureBegin(at(2), input::GestureType::Swipe, 3);
            manager.onPointerGestureUpdate(at(3), input::GestureType::Swipe, base::PointF(4, 0),
                                           base::PointF(4, 0), 1, 0);
            manager.onPointerGestureEnd(at(4), input::GestureType::Swipe, false);
        });
        c.sync();
        EXPECT_TRUE(c.observed.gestureEvents.empty());

        check([&] {
            manager.onPointerMotionAbsolute(at(5), base::PointF(wm::centerPixel(content)));
            manager.onPointerGestureBegin(at(6), input::GestureType::Pinch, 2);
        });
        c.sync();
        const size_t pointerEvents = c.observed.pointerEvents.size();
        c.destroySurface(surface);
        c.sync();
        EXPECT_EQ(std::vector<std::string>({"pinch_begin 2", "pinch_end 1"}),
                  c.observed.gestureEvents);
        // The cancellation has its own frame, before the leave.
        ASSERT_GT(c.observed.pointerEvents.size(), pointerEvents + 1);
        EXPECT_EQ("frame", c.observed.pointerEvents[pointerEvents]);
        EXPECT_EQ("leave", c.observed.pointerEvents[pointerEvents + 1]);

        // The rest of the gesture goes nowhere.
        check([&] {
            manager.onPointerGestureUpdate(at(7), input::GestureType::Pinch, base::PointF(1, 1),
                                           base::PointF(1, 1), 2, 0);
            manager.onPointerGestureEnd(at(8), input::GestureType::Pinch, false);
        });
        c.sync();
        EXPECT_EQ(2u, c.observed.gestureEvents.size());
    });
}
