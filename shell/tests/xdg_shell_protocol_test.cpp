// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/shell_unittest.cc and
//           chromium/src/components/exo/xdg_shell_surface_unittest.cc, driven through a real
//           libwayland-client connection rather than Exo's test client.
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: exercises the corresponding behavior with native
// compositor test doubles and APIs.

#include "base/geometry.h"
#include "wayland/protocols/xdg_activation.h"
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
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>
#include <wayland-server-core.h>
#include <lingmo-window-hints-v1-client-protocol.h>
#include <lingmo-window-placement-v1-client-protocol.h>
#include <xdg-activation-v1-client-protocol.h>
#include <xdg-dialog-v1-client-protocol.h>
#include <xdg-shell-client-protocol.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "display/display_manager.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "shm_test_util.h"
#include "wayland/privileged_client.h"
#include "wayland/protocols/window_hints.h"
#include "wayland/protocols/window_placement.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/protocols/xdg_dialog.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/serial_tracker.h"
#include "wayland/server.h"
#include "wm/activation.h"
#include "wm/shell_surface.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_modality_controller.h"
#include "wm/window_state.h"

using namespace Lingmo;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 800, 600);

struct ToplevelConfigure {
    int32_t width = 0;
    int32_t height = 0;
    std::vector<uint32_t> states;

    bool has(uint32_t state) const
    {
        return std::find(states.begin(), states.end(), state) != states.end();
    }
};

struct PopupConfigure {
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
};

struct Client;

// One xdg_surface plus whichever role was given to it, and everything the
// server sent about it.
struct XdgWindow {
    Client *client = nullptr;
    wl_surface *surface = nullptr;
    xdg_surface *xdgSurface = nullptr;
    xdg_toplevel *toplevel = nullptr;
    xdg_popup *popup = nullptr;
    wl_buffer *buffer = nullptr;

    std::vector<uint32_t> surfaceConfigures;
    std::vector<ToplevelConfigure> toplevelConfigures;
    std::vector<PopupConfigure> popupConfigures;
    std::vector<uint32_t> repositioned;
    int closes = 0;
    int popupDones = 0;
    std::vector<base::Size> bounds;
    std::vector<std::vector<uint32_t>> capabilities;
    // Every xdg_toplevel event in order, to check what precedes a configure.
    std::vector<std::string> toplevelEvents;

    uint32_t lastSerial() const { return surfaceConfigures.back(); }

    const ToplevelConfigure &lastToplevel() const { return toplevelConfigures.back(); }
};

const xdg_surface_listener surfaceListener = {
        [](void *data, xdg_surface *, uint32_t serial) {
            static_cast<XdgWindow *>(data)->surfaceConfigures.push_back(serial);
        },
};

const xdg_toplevel_listener toplevelListener = {
        [](void *data, xdg_toplevel *, int32_t width, int32_t height, wl_array *states) {
            auto *window = static_cast<XdgWindow *>(data);
            window->toplevelEvents.push_back("configure");
            ToplevelConfigure configure {width, height, {}};
            const auto *values = static_cast<const uint32_t *>(states->data);
            for (size_t i = 0; i < states->size / sizeof(uint32_t); ++i)
                configure.states.push_back(values[i]);
            window->toplevelConfigures.push_back(std::move(configure));
        },
        [](void *data, xdg_toplevel *) {
            ++static_cast<XdgWindow *>(data)->closes;
        },
        [](void *data, xdg_toplevel *, int32_t width, int32_t height) {
            auto *window = static_cast<XdgWindow *>(data);
            window->toplevelEvents.push_back("configure_bounds");
            window->bounds.emplace_back(width, height);
        },
        [](void *data, xdg_toplevel *, wl_array *capabilities) {
            auto *window = static_cast<XdgWindow *>(data);
            window->toplevelEvents.push_back("wm_capabilities");
            const auto *values = static_cast<const uint32_t *>(capabilities->data);
            window->capabilities.emplace_back(
                    values, values + capabilities->size / sizeof(uint32_t));
        },
};

const xdg_popup_listener popupListener = {
        [](void *data, xdg_popup *, int32_t x, int32_t y, int32_t width, int32_t height) {
            static_cast<XdgWindow *>(data)->popupConfigures.push_back({x, y, width, height});
        },
        [](void *data, xdg_popup *) {
            ++static_cast<XdgWindow *>(data)->popupDones;
        },
        [](void *data, xdg_popup *, uint32_t token) {
            static_cast<XdgWindow *>(data)->repositioned.push_back(token);
        },
};

struct Client {
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_compositor *compositor = nullptr;
    wl_shm *shm = nullptr;
    wl_seat *seat = nullptr;
    wl_pointer *pointer = nullptr;
    wl_touch *touch = nullptr;
    xdg_wm_base *wmBase = nullptr;
    xdg_wm_dialog_v1 *dialogManager = nullptr;
    std::map<std::string, std::pair<uint32_t, uint32_t>> globals;

    bool answerPings = true;
    std::vector<uint32_t> pings;
    uint32_t lastButtonSerial = 0;
    uint32_t lastTouchDownSerial = 0;
    int touchCancels = 0;
    uint32_t lastEnterSerial = 0;
    int buttonPresses = 0;
    int buttonReleases = 0;
    int axisEvents = 0;
    int axisStops = 0;
    int motions = 0;
    wl_surface *pointerFocus = nullptr;
    base::PointF pointerPosition;

    std::vector<std::unique_ptr<XdgWindow>> windows;

    explicit Client(int fd)
        : display(wl_display_connect_to_fd(fd))
    {
        registry = wl_display_get_registry(display);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *, uint32_t name, const char *interface,
                   uint32_t version) {
                    static_cast<Client *>(data)->globals[interface] = {name, version};
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        wl_display_roundtrip(display);

        compositor = static_cast<wl_compositor *>(wl_registry_bind(
                registry, globals["wl_compositor"].first, &wl_compositor_interface, 4));
        shm = static_cast<wl_shm *>(
                wl_registry_bind(registry, globals["wl_shm"].first, &wl_shm_interface, 1));
        if (globals.contains("xdg_wm_base")) {
            wmBase = static_cast<xdg_wm_base *>(
                    wl_registry_bind(registry, globals["xdg_wm_base"].first, &xdg_wm_base_interface,
                                     globals["xdg_wm_base"].second));
            static const xdg_wm_base_listener wmBaseListener = {
                    [](void *data, xdg_wm_base *base, uint32_t serial) {
                        auto *client = static_cast<Client *>(data);
                        client->pings.push_back(serial);
                        if (client->answerPings)
                            xdg_wm_base_pong(base, serial);
                    }};
            xdg_wm_base_add_listener(wmBase, &wmBaseListener, this);
        }
        if (globals.contains("xdg_wm_dialog_v1")) {
            dialogManager = static_cast<xdg_wm_dialog_v1 *>(wl_registry_bind(
                    registry, globals["xdg_wm_dialog_v1"].first, &xdg_wm_dialog_v1_interface, 1));
        }
        wl_display_roundtrip(display);
    }

    ~Client()
    {
        for (auto &window : windows)
            destroyWindowObjects(window.get());
        windows.clear();
        if (touch)
            wl_touch_destroy(touch);
        if (pointer)
            wl_pointer_destroy(pointer);
        if (seat)
            wl_seat_destroy(seat);
        if (dialogManager)
            xdg_wm_dialog_v1_destroy(dialogManager);
        if (wmBase)
            xdg_wm_base_destroy(wmBase);
        wl_shm_destroy(shm);
        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }

    // After bindSeat().
    void bindTouch()
    {
        touch = wl_seat_get_touch(seat);
        static const wl_touch_listener touchListener = {
                [](void *data, wl_touch *, uint32_t serial, uint32_t, wl_surface *, int32_t,
                   wl_fixed_t, wl_fixed_t) {
                    static_cast<Client *>(data)->lastTouchDownSerial = serial;
                },
                [](void *, wl_touch *, uint32_t, uint32_t, int32_t) {},
                [](void *, wl_touch *, uint32_t, int32_t, wl_fixed_t, wl_fixed_t) {},
                [](void *, wl_touch *) {},
                [](void *data, wl_touch *) {
                    ++static_cast<Client *>(data)->touchCancels;
                },
                nullptr,
                nullptr};
        wl_touch_add_listener(touch, &touchListener, this);
        sync();
    }

    void bindSeat()
    {
        seat = static_cast<wl_seat *>(
                wl_registry_bind(registry, globals["wl_seat"].first, &wl_seat_interface, 5));
        pointer = wl_seat_get_pointer(seat);
        static const wl_pointer_listener pointerListener = {
                [](void *data, wl_pointer *, uint32_t serial, wl_surface *surface, wl_fixed_t x,
                   wl_fixed_t y) {
                    static_cast<Client *>(data)->lastEnterSerial = serial;
                    static_cast<Client *>(data)->pointerFocus = surface;
                    static_cast<Client *>(data)->pointerPosition =
                            base::PointF(wl_fixed_to_double(x), wl_fixed_to_double(y));
                },
                [](void *data, wl_pointer *, uint32_t, wl_surface *) {
                    static_cast<Client *>(data)->pointerFocus = nullptr;
                },
                [](void *data, wl_pointer *, uint32_t, wl_fixed_t x, wl_fixed_t y) {
                    ++static_cast<Client *>(data)->motions;
                    static_cast<Client *>(data)->pointerPosition =
                            base::PointF(wl_fixed_to_double(x), wl_fixed_to_double(y));
                },
                [](void *data, wl_pointer *, uint32_t serial, uint32_t, uint32_t, uint32_t state) {
                    auto *client = static_cast<Client *>(data);
                    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
                        client->lastButtonSerial = serial;
                        ++client->buttonPresses;
                    } else {
                        ++client->buttonReleases;
                    }
                },
                [](void *data, wl_pointer *, uint32_t, uint32_t, wl_fixed_t) {
                    ++static_cast<Client *>(data)->axisEvents;
                },
                [](void *, wl_pointer *) {},
                [](void *, wl_pointer *, uint32_t) {},
                [](void *data, wl_pointer *, uint32_t, uint32_t) {
                    ++static_cast<Client *>(data)->axisStops;
                },
                [](void *, wl_pointer *, uint32_t, int32_t) {},
                nullptr,
                nullptr,
        };
        wl_pointer_add_listener(pointer, &pointerListener, this);
        sync();
    }

    wl_buffer *makeBuffer(const base::Size &size)
    {
        const int stride = size.width() * 4;
        const size_t bytes = static_cast<size_t>(stride) * size.height();
        const int fd = testsupport::makeMemfd(bytes);
        wl_shm_pool *pool = wl_shm_create_pool(shm, fd, static_cast<int32_t>(bytes));
        wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, size.width(), size.height(), stride,
                                                      WL_SHM_FORMAT_XRGB8888);
        wl_shm_pool_destroy(pool);
        close(fd);
        return buffer;
    }

    // A surface with an xdg_surface but no role yet.
    XdgWindow *makeXdgSurface()
    {
        windows.push_back(std::make_unique<XdgWindow>());
        XdgWindow *window = windows.back().get();
        window->client = this;
        window->surface = wl_compositor_create_surface(compositor);
        window->xdgSurface = xdg_wm_base_get_xdg_surface(wmBase, window->surface);
        xdg_surface_add_listener(window->xdgSurface, &surfaceListener, window);
        return window;
    }

    XdgWindow *makeToplevel()
    {
        XdgWindow *window = makeXdgSurface();
        window->toplevel = xdg_surface_get_toplevel(window->xdgSurface);
        xdg_toplevel_add_listener(window->toplevel, &toplevelListener, window);
        return window;
    }

    // Runs the whole handshake and leaves the toplevel mapped.
    XdgWindow *makeMappedToplevel(const base::Size &size)
    {
        XdgWindow *window = makeToplevel();
        wl_surface_commit(window->surface);
        sync();
        EXPECT_EQ(1u, window->surfaceConfigures.size());
        ackAndAttach(window, size);
        return window;
    }

    void ackAndAttach(XdgWindow *window, const base::Size &size)
    {
        xdg_surface_ack_configure(window->xdgSurface, window->lastSerial());
        if (!window->buffer)
            window->buffer = makeBuffer(size);
        wl_surface_attach(window->surface, window->buffer, 0, 0);
        wl_surface_damage(window->surface, 0, 0, size.width(), size.height());
        wl_surface_commit(window->surface);
        sync();
    }

    void destroyWindowObjects(XdgWindow *window)
    {
        if (window->popup) {
            xdg_popup_destroy(window->popup);
            window->popup = nullptr;
        }
        if (window->toplevel) {
            xdg_toplevel_destroy(window->toplevel);
            window->toplevel = nullptr;
        }
        if (window->xdgSurface) {
            xdg_surface_destroy(window->xdgSurface);
            window->xdgSurface = nullptr;
        }
        if (window->surface) {
            wl_surface_destroy(window->surface);
            window->surface = nullptr;
        }
        if (window->buffer) {
            wl_buffer_destroy(window->buffer);
            window->buffer = nullptr;
        }
    }

    void sync() { wl_display_roundtrip(display); }

    // Round-trips and reports the protocol error the server raised, if any.
    uint32_t syncExpectingError(const wl_interface **interface = nullptr)
    {
        if (wl_display_roundtrip(display) >= 0)
            return 0;
        if (wl_display_get_error(display) != EPROTO)
            return 0;
        const wl_interface *raisedOn = nullptr;
        uint32_t id = 0;
        const uint32_t code = wl_display_get_protocol_error(display, &raisedOn, &id);
        if (interface)
            *interface = raisedOn;
        return code;
    }
};

class XdgShellProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node *cursorContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("cursor"));
    compositor::Compositor compositor {surfaceContainer};
    input::Seat seat {&compositor};
    input::InputManager manager {&seat};
    wm::WindowManager windowManager {&compositor, &seat};
    wayland::Server server;

    std::mutex mutex;
    std::function<void()> pendingCheck;
    int pingTimeoutMs = wayland::kXdgPingTimeoutMs;
    // Whether the client may bind lingmo_window_placement_v1.
    bool placementAuthorized = true;
    // While run() runs.
    wayland::WaylandXdgActivation *xdgActivation = nullptr;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        seat.setCursorContainer(cursorContainer);
        manager.setBounds(kOutput.size());
        manager.setEventSink(&windowManager);
        windowManager.setOutputBounds(kOutput);
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

    wm::Window *onlyWindow()
    {
        return windowManager.windows().empty() ? nullptr : windowManager.windows().front();
    }

    // Polls from the client thread until the server-side window reports `responsive`.
    bool waitForResponsive(bool responsive)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (std::chrono::steady_clock::now() < deadline) {
            bool current = !responsive;
            check([&] {
                current = onlyWindow()->isResponsive();
            });
            if (current == responsive)
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return false;
    }

    // Another client of the same server, from inside run()'s body.
    std::unique_ptr<Client> connectClient()
    {
        int fds[2];
        if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds) != 0)
            return nullptr;
        check([&] {
            EXPECT_TRUE(wl_client_create(server.display(), fds[0]));
        });
        return std::make_unique<Client>(fds[1]);
    }

    void run(std::function<void(Client &)> body,
             std::chrono::seconds timeout = std::chrono::seconds(5))
    {
        auto waylandSeat = std::make_unique<wayland::WaylandSeat>(&seat, server.serialTracker());
        wayland::WaylandXdgActivation activation(&windowManager, waylandSeat.get());
        ASSERT_TRUE(wayland::createXdgActivationGlobal(server.display(), &activation));
        xdgActivation = &activation;
        auto xdgShell = std::make_unique<wayland::WaylandXdgShell>(&compositor, &windowManager,
                                                                   &seat, server.serialTracker());
        xdgShell->setPingTimeout(pingTimeoutMs);
        ASSERT_TRUE(wayland::createCompositorGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSubcompositorGlobal(server.display()));
        ASSERT_TRUE(wayland::createShmGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSeatGlobal(server.display(), waylandSeat.get()));
        ASSERT_TRUE(wayland::createXdgShellGlobal(server.display(), xdgShell.get()));
        ASSERT_TRUE(wayland::createXdgDialogGlobal(server.display()));
        ASSERT_TRUE(wayland::createWindowHintsGlobal(server.display(), &windowManager));
        wayland::PrivilegedClients privileged(server.display());
        const auto authorize = [this](const wl_client *) {
            return placementAuthorized;
        };
        wayland::WaylandWindowPlacement placement(server.display(), &windowManager, authorize);
        privileged.addRestrictedGlobal(placement.global(), authorize);

        int fds[2];
        ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds));
        ASSERT_TRUE(wl_client_create(server.display(), fds[0]));

        std::atomic<bool> done = false;
        std::thread client([&] {
            {
                Client c(fds[1]);
                body(c);
            }
            done = true;
        });

        auto deadline = std::chrono::steady_clock::now() + timeout;
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
        EXPECT_EQ(nullptr, seat.focusedSurface());
        EXPECT_TRUE(windowManager.windows().empty());
        EXPECT_TRUE(windowManager.popupGrabs().empty());
    }
};

////////////////////////////////////////////////////////////////////////////////
// Registry and roles

TEST_F(XdgShellProtocolTest, TheRegistryListsXdgWmBase)
{
    run([&](Client &c) {
        ASSERT_TRUE(c.globals.contains("xdg_wm_base"));
        EXPECT_EQ(wayland::kXdgWmBaseVersion, c.globals["xdg_wm_base"].second);
    });
}

TEST_F(XdgShellProtocolTest, ASurfaceCannotTakeTwoRoles)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeXdgSurface();
        c.sync();
        // A second xdg_surface for the same wl_surface is a role conflict.
        xdg_surface *second = xdg_wm_base_get_xdg_surface(c.wmBase, window->surface);
        EXPECT_EQ(uint32_t(XDG_WM_BASE_ERROR_ROLE), c.syncExpectingError());
        xdg_surface_destroy(second);
    });
}

TEST_F(XdgShellProtocolTest, AnXdgSurfaceCannotTakeTwoRoleObjects)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeToplevel();
        c.sync();
        xdg_toplevel *second = xdg_surface_get_toplevel(window->xdgSurface);
        EXPECT_EQ(uint32_t(XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED), c.syncExpectingError());
        xdg_toplevel_destroy(second);
    });
}

TEST_F(XdgShellProtocolTest, DestroyingTheXdgSurfaceBeforeItsRoleObjectIsAnError)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeToplevel();
        c.sync();
        xdg_surface_destroy(window->xdgSurface);
        EXPECT_EQ(uint32_t(XDG_SURFACE_ERROR_DEFUNCT_ROLE_OBJECT), c.syncExpectingError());
        window->xdgSurface = nullptr;
    });
}

////////////////////////////////////////////////////////////////////////////////
// The configure sequence

TEST_F(XdgShellProtocolTest, TheFirstCommitProducesAnInitialConfigure)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeToplevel();
        wl_surface_commit(window->surface);
        c.sync();

        ASSERT_EQ(1u, window->surfaceConfigures.size());
        ASSERT_EQ(1u, window->toplevelConfigures.size());
        // The compositor has no size to impose before the client picks one.
        EXPECT_EQ(0, window->lastToplevel().width);
        EXPECT_EQ(0, window->lastToplevel().height);
        EXPECT_TRUE(window->lastToplevel().states.empty());

        check([&] {
            EXPECT_TRUE(windowManager.windows().empty());
        });
    });
}

TEST_F(XdgShellProtocolTest, AckingAndAttachingMapsAWindow)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));

        check([&] {
            wm::Window *managed = onlyWindow();
            ASSERT_NE(nullptr, managed);
            EXPECT_EQ(base::Size(300, 200), managed->bounds().size());
            EXPECT_TRUE(managed->isActive());
        });
        // Mapping activates the window, which the client is told about.
        c.sync();
        EXPECT_TRUE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_ACTIVATED));
    });
}

TEST_F(XdgShellProtocolTest, AnUnknownAckConfigureSerialIsAProtocolError)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeToplevel();
        wl_surface_commit(window->surface);
        c.sync();

        xdg_surface_ack_configure(window->xdgSurface, window->lastSerial() + 12345);
        EXPECT_EQ(uint32_t(XDG_SURFACE_ERROR_INVALID_SERIAL), c.syncExpectingError());
    });
}

TEST_F(XdgShellProtocolTest, AStaleAckConfigureSerialIsAProtocolError)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        c.sync();
        const uint32_t stale = window->surfaceConfigures.front();

        xdg_toplevel_set_maximized(window->toplevel);
        c.sync();
        ASSERT_GE(window->surfaceConfigures.size(), 3u);
        xdg_surface_ack_configure(window->xdgSurface, window->lastSerial());
        c.sync();

        xdg_surface_ack_configure(window->xdgSurface, stale);
        EXPECT_EQ(uint32_t(XDG_SURFACE_ERROR_INVALID_SERIAL), c.syncExpectingError());
    });
}

////////////////////////////////////////////////////////////////////////////////
// Toplevel state

TEST_F(XdgShellProtocolTest, MaximizeAndRestoreAreConfiguredWithTheirStates)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));

        xdg_toplevel_set_maximized(window->toplevel);
        c.sync();
        EXPECT_TRUE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_MAXIMIZED));
        EXPECT_EQ(kOutput.width(), window->lastToplevel().width);
        EXPECT_EQ(kOutput.height(), window->lastToplevel().height);
        check([&] {
            EXPECT_EQ(wm::WindowStateType::Maximized, onlyWindow()->stateType());
        });

        xdg_toplevel_unset_maximized(window->toplevel);
        c.sync();
        EXPECT_FALSE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_MAXIMIZED));
        EXPECT_EQ(300, window->lastToplevel().width);
    });
}

TEST_F(XdgShellProtocolTest, FullscreenKeepsTheMaximizedStateItCameFrom)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));

        xdg_toplevel_set_maximized(window->toplevel);
        xdg_toplevel_set_fullscreen(window->toplevel, nullptr);
        c.sync();

        EXPECT_TRUE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_FULLSCREEN));
        EXPECT_TRUE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_MAXIMIZED));

        xdg_toplevel_unset_fullscreen(window->toplevel);
        c.sync();
        EXPECT_FALSE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_FULLSCREEN));
        EXPECT_TRUE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_MAXIMIZED));
    });
}

TEST_F(XdgShellProtocolTest, MinimizeHidesAndSuspendsTheWindow)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));

        xdg_toplevel_set_minimized(window->toplevel);
        c.sync();

        check([&] {
            wm::Window *managed = onlyWindow();
            ASSERT_NE(nullptr, managed);
            EXPECT_EQ(wm::WindowStateType::Minimized, managed->stateType());
            EXPECT_FALSE(managed->isVisible());
        });
        EXPECT_FALSE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_ACTIVATED));
        EXPECT_TRUE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_SUSPENDED));

        check([&] {
            onlyWindow()->restore();
        });
        c.sync();
        EXPECT_FALSE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_SUSPENDED));
    });
}

TEST_F(XdgShellProtocolTest, AShellHiddenWindowIsSuspended)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        EXPECT_FALSE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_SUSPENDED));

        check([&] {
            onlyWindow()->setHidden(true);
        });
        c.sync();
        EXPECT_TRUE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_SUSPENDED));

        check([&] {
            onlyWindow()->setHidden(false);
        });
        c.sync();
        EXPECT_FALSE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_SUSPENDED));
    });
}

TEST_F(XdgShellProtocolTest, CapabilitiesAndBoundsPrecedeTheFirstConfigure)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeToplevel();
        wl_surface_commit(window->surface);
        c.sync();

        const std::vector<std::string> expected {"wm_capabilities", "configure_bounds",
                                                 "configure"};
        EXPECT_EQ(expected, window->toplevelEvents);
        ASSERT_EQ(1u, window->capabilities.size());
        EXPECT_EQ((std::vector<uint32_t> {XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE,
                                          XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN,
                                          XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE}),
                  window->capabilities[0]);
        ASSERT_EQ(1u, window->bounds.size());
        EXPECT_EQ(kOutput.size(), window->bounds[0]);

        // Capabilities are only repeated when they change.
        c.ackAndAttach(window, base::Size(300, 200));
        check([&] {
            onlyWindow()->maximize();
        });
        c.sync();
        EXPECT_EQ(1u, window->capabilities.size());
        EXPECT_EQ(window->toplevelConfigures.size(), window->bounds.size());
    });
}

TEST_F(XdgShellProtocolTest, AFixedSizeWindowIsNotOfferedMaximize)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        ASSERT_EQ(1u, window->capabilities.size());

        xdg_toplevel_set_min_size(window->toplevel, 300, 200);
        xdg_toplevel_set_max_size(window->toplevel, 300, 200);
        wl_surface_commit(window->surface);
        c.sync();
        ASSERT_EQ(2u, window->capabilities.size());
        EXPECT_EQ((std::vector<uint32_t> {XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN,
                                          XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE}),
                  window->capabilities[1]);
        // A new wm_capabilities is always followed by a configure.
        EXPECT_EQ("configure", window->toplevelEvents.back());

        xdg_toplevel_set_max_size(window->toplevel, 0, 0);
        wl_surface_commit(window->surface);
        c.sync();
        ASSERT_EQ(3u, window->capabilities.size());
        EXPECT_EQ(3u, window->capabilities[2].size());
    });
}

TEST_F(XdgShellProtocolTest, ActivationMovesTheActivatedStateBetweenToplevels)
{
    run([&](Client &c) {
        XdgWindow *first = c.makeMappedToplevel(base::Size(300, 200));
        XdgWindow *second = c.makeMappedToplevel(base::Size(300, 200));
        c.sync();
        ASSERT_TRUE(second->lastToplevel().has(XDG_TOPLEVEL_STATE_ACTIVATED));
        ASSERT_FALSE(first->lastToplevel().has(XDG_TOPLEVEL_STATE_ACTIVATED));

        check([&] {
            windowManager.activation()->activate(windowManager.windows().front());
        });
        c.sync();

        EXPECT_TRUE(first->lastToplevel().has(XDG_TOPLEVEL_STATE_ACTIVATED));
        EXPECT_FALSE(second->lastToplevel().has(XDG_TOPLEVEL_STATE_ACTIVATED));
    });
}

////////////////////////////////////////////////////////////////////////////////
// Ping and unresponsive clients

TEST_F(XdgShellProtocolTest, APressOnAWindowPingsItsClient)
{
    run([&](Client &c) {
        c.bindSeat();
        c.makeMappedToplevel(base::Size(300, 200));
        check([&] {
            onlyWindow()->setBounds(base::Rect(100, 100, 300, 200));
            manager.onPointerMotionAbsolute(at(1), base::PointF(150, 150));
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(1u, c.pings.size());
        EXPECT_EQ(1, c.buttonPresses);
    });
}

TEST_F(XdgShellProtocolTest, AnUnansweredPingMakesTheWindowUnresponsiveUntilThePong)
{
    pingTimeoutMs = 20;
    run([&](Client &c) {
        c.answerPings = false;
        c.makeMappedToplevel(base::Size(300, 200));
        check([&] {
            EXPECT_TRUE(onlyWindow()->isResponsive());
            onlyWindow()->ping();
            onlyWindow()->ping();
        });
        c.sync();
        ASSERT_EQ(1u, c.pings.size());
        ASSERT_TRUE(waitForResponsive(false));

        // Only the outstanding serial answers it.
        xdg_wm_base_pong(c.wmBase, c.pings.front() + 1);
        c.sync();
        check([&] {
            EXPECT_FALSE(onlyWindow()->isResponsive());
        });
        xdg_wm_base_pong(c.wmBase, c.pings.front());
        c.sync();
        check([&] {
            EXPECT_TRUE(onlyWindow()->isResponsive());
            onlyWindow()->ping();
        });
        c.sync();
        EXPECT_EQ(2u, c.pings.size());
    });
}

TEST_F(XdgShellProtocolTest, ClosingAnUnresponsiveWindowNoLongerAsksTheClient)
{
    pingTimeoutMs = 20;
    run([&](Client &c) {
        c.answerPings = false;
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        check([&] {
            onlyWindow()->requestClose();
        });
        c.sync();
        EXPECT_EQ(1, window->closes);
        ASSERT_EQ(1u, c.pings.size());
        ASSERT_TRUE(waitForResponsive(false));

        // The kill callback refuses the compositor's own pid, which this
        // in-process client shares, so the window survives to be checked.
        check([&] {
            onlyWindow()->requestClose();
        });
        c.sync();
        EXPECT_EQ(1, window->closes);
    });
}

TEST_F(XdgShellProtocolTest, TitleAndAppIdReachTheWindow)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        xdg_toplevel_set_title(window->toplevel, "Notes");
        xdg_toplevel_set_app_id(window->toplevel, "org.lingmo.Notes");
        c.sync();

        check([&] {
            EXPECT_EQ("Notes", onlyWindow()->title());
            EXPECT_EQ("org.lingmo.Notes", onlyWindow()->applicationId());
        });
    });
}

TEST_F(XdgShellProtocolTest, MinAndMaxSizeReachTheWindowOnCommit)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        xdg_toplevel_set_min_size(window->toplevel, 150, 100);
        xdg_toplevel_set_max_size(window->toplevel, 600, 500);
        wl_surface_commit(window->surface);
        c.sync();

        check([&] {
            EXPECT_EQ(base::Size(150, 100), onlyWindow()->minimumSize());
            EXPECT_EQ(base::Size(600, 500), onlyWindow()->maximumSize());
            EXPECT_TRUE(onlyWindow()->isResizable());
        });
    });
}

TEST_F(XdgShellProtocolTest, ANegativeMinSizeIsAProtocolError)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        xdg_toplevel_set_min_size(window->toplevel, -1, 100);
        EXPECT_EQ(uint32_t(XDG_TOPLEVEL_ERROR_INVALID_SIZE), c.syncExpectingError());
    });
}

TEST_F(XdgShellProtocolTest, SetParentMapsOntoTheTransientRelationship)
{
    run([&](Client &c) {
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        XdgWindow *child = c.makeMappedToplevel(base::Size(200, 150));
        xdg_toplevel_set_parent(child->toplevel, parent->toplevel);
        c.sync();

        check([&] {
            ASSERT_EQ(2u, windowManager.windows().size());
            wm::Window *parentWindow = windowManager.windows()[0];
            wm::Window *childWindow = windowManager.windows()[1];
            EXPECT_EQ(parentWindow, childWindow->transientParent());
        });
    });
}

TEST_F(XdgShellProtocolTest, AModalDialogIsCenteredOverAndBlocksItsParent)
{
    run([&](Client &c) {
        ASSERT_TRUE(c.dialogManager);
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        XdgWindow *child = c.makeToplevel();
        xdg_toplevel_set_parent(child->toplevel, parent->toplevel);
        xdg_dialog_v1 *dialog = xdg_wm_dialog_v1_get_xdg_dialog(c.dialogManager, child->toplevel);
        xdg_dialog_v1_set_modal(dialog);
        wl_surface_commit(child->surface);
        c.sync();
        c.ackAndAttach(child, base::Size(200, 100));

        wm::Window *parentWindow = nullptr;
        wm::Window *childWindow = nullptr;
        check([&] {
            ASSERT_EQ(2u, windowManager.windows().size());
            parentWindow = windowManager.windows()[0];
            childWindow = windowManager.windows()[1];
            EXPECT_TRUE(childWindow->isModal());
            EXPECT_EQ(childWindow, wm::modalTransient(parentWindow));
            EXPECT_EQ(wm::centerPixel(parentWindow->bounds()), wm::centerPixel(childWindow->bounds()));
            EXPECT_TRUE(childWindow->isActive());
            windowManager.activation()->activate(parentWindow);
            EXPECT_TRUE(childWindow->isActive());
        });

        xdg_dialog_v1_unset_modal(dialog);
        c.sync();
        check([&] {
            EXPECT_FALSE(childWindow->isModal());
        });

        xdg_dialog_v1_set_modal(dialog);
        c.sync();
        check([&] {
            EXPECT_TRUE(childWindow->isModal());
        });

        // The dialog object takes the modality with it.
        xdg_dialog_v1_destroy(dialog);
        c.sync();
        check([&] {
            EXPECT_FALSE(childWindow->isModal());
            windowManager.activation()->activate(parentWindow);
            EXPECT_TRUE(parentWindow->isActive());
        });
    });
}

TEST_F(XdgShellProtocolTest, ASecondDialogObjectIsAProtocolError)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeToplevel();
        xdg_dialog_v1 *first = xdg_wm_dialog_v1_get_xdg_dialog(c.dialogManager, window->toplevel);
        xdg_dialog_v1 *second = xdg_wm_dialog_v1_get_xdg_dialog(c.dialogManager, window->toplevel);
        const wl_interface *interface = nullptr;
        EXPECT_EQ(static_cast<uint32_t>(XDG_WM_DIALOG_V1_ERROR_ALREADY_USED),
                  c.syncExpectingError(&interface));
        EXPECT_EQ(&xdg_wm_dialog_v1_interface, interface);
        xdg_dialog_v1_destroy(second);
        xdg_dialog_v1_destroy(first);
    });
}

TEST_F(XdgShellProtocolTest, ADialogOutlivingItsToplevelIsInert)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(200, 100));
        xdg_dialog_v1 *dialog = xdg_wm_dialog_v1_get_xdg_dialog(c.dialogManager, window->toplevel);
        xdg_toplevel_destroy(window->toplevel);
        window->toplevel = nullptr;
        xdg_dialog_v1_set_modal(dialog);
        xdg_dialog_v1_destroy(dialog);
        EXPECT_EQ(0u, c.syncExpectingError());
    });
}

TEST_F(XdgShellProtocolTest, UnmappingAParentHandsItsDialogToTheGrandparent)
{
    run([&](Client &c) {
        XdgWindow *root = c.makeMappedToplevel(base::Size(400, 300));
        XdgWindow *parent = c.makeMappedToplevel(base::Size(300, 200));
        XdgWindow *child = c.makeMappedToplevel(base::Size(200, 100));
        xdg_toplevel_set_parent(parent->toplevel, root->toplevel);
        xdg_toplevel_set_parent(child->toplevel, parent->toplevel);
        c.sync();

        wl_surface_attach(parent->surface, nullptr, 0, 0);
        wl_surface_commit(parent->surface);
        c.sync();
        check([&] {
            ASSERT_EQ(2u, windowManager.windows().size());
            wm::Window *rootWindow = nullptr;
            wm::Window *childWindow = nullptr;
            for (wm::Window *window : windowManager.windows()) {
                if (window->contentBounds().size() == base::Size(400, 300))
                    rootWindow = window;
                else
                    childWindow = window;
            }
            ASSERT_TRUE(rootWindow && childWindow);
            EXPECT_EQ(rootWindow, childWindow->transientParent());
        });

        // An unmapped toplevel is no parent, and remapping restores nothing.
        XdgWindow *orphan = c.makeMappedToplevel(base::Size(150, 100));
        xdg_toplevel_set_parent(orphan->toplevel, parent->toplevel);
        wl_surface_commit(parent->surface);
        c.sync();
        c.ackAndAttach(parent, base::Size(300, 200));
        check([&] {
            for (wm::Window *window : windowManager.windows()) {
                if (window->contentBounds().size() == base::Size(150, 100))
                    EXPECT_EQ(nullptr, window->transientParent());
                if (window->contentBounds().size() == base::Size(200, 100))
                    EXPECT_EQ(base::Size(400, 300), window->transientParent()->contentBounds().size());
            }
        });
    });
}

////////////////////////////////////////////////////////////////////////////////
// Modal input

TEST_F(XdgShellProtocolTest, APressAModalBlocksKeepsItsReleaseForEveryButton)
{
    run([&](Client &c) {
        c.bindSeat();
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        check([&] {
            onlyWindow()->setBounds(base::Rect(0, 0, 400, 300));
        });
        XdgWindow *child = c.makeToplevel();
        xdg_toplevel_set_parent(child->toplevel, parent->toplevel);
        xdg_dialog_v1 *dialog = xdg_wm_dialog_v1_get_xdg_dialog(c.dialogManager, child->toplevel);
        xdg_dialog_v1_set_modal(dialog);
        wl_surface_commit(child->surface);
        c.sync();
        c.ackAndAttach(child, base::Size(100, 80));

        check([&] {
            manager.onPointerMotionAbsolute(at(1), base::PointF(10, 10));
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_RIGHT, true);
            manager.onPointerButton(at(4), BTN_LEFT, false);
            manager.onPointerButton(at(5), BTN_RIGHT, false);
            // Scrolling reaches nothing and activates nothing.
            windowManager.activation()->activate(nullptr);
            manager.onPointerAxis(at(6), base::PointF(0, 10), false);
            manager.onPointerAxisStop(at(7));
            EXPECT_EQ(nullptr, windowManager.activation()->activeWindow());
        });
        c.sync();
        EXPECT_EQ(0, c.buttonPresses);
        EXPECT_EQ(0, c.buttonReleases);
        EXPECT_EQ(0, c.axisEvents);
        EXPECT_EQ(0, c.axisStops);
        EXPECT_NE(parent->surface, c.pointerFocus);

        // Once the modal is gone, the parent gets whole clicks again.
        xdg_dialog_v1_destroy(dialog);
        c.sync();
        check([&] {
            manager.onPointerMotionAbsolute(at(8), base::PointF(12, 12));
            manager.onPointerButton(at(9), BTN_LEFT, true);
            manager.onPointerButton(at(10), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(parent->surface, c.pointerFocus);
        EXPECT_EQ(1, c.buttonPresses);
        EXPECT_EQ(1, c.buttonReleases);
    });
}

TEST_F(XdgShellProtocolTest, ThePressThatDismissesAPopupKeepsItsRelease)
{
    run([&](Client &c) {
        c.bindSeat();
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        check([&] {
            onlyWindow()->setBounds(base::Rect(0, 0, 400, 300));
            manager.onPointerMotionAbsolute(at(1), base::PointF(20, 30));
        });
        c.sync();

        XdgWindow *popup = c.makeXdgSurface();
        xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
        xdg_positioner_set_size(positioner, 100, 80);
        xdg_positioner_set_anchor_rect(positioner, 10, 20, 1, 1);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, parent->xdgSurface, positioner);
        xdg_popup_add_listener(popup->popup, &popupListener, popup);
        xdg_popup_grab(popup->popup, c.seat, c.lastEnterSerial);
        xdg_positioner_destroy(positioner);
        c.sync();
        c.ackAndAttach(popup, base::Size(100, 80));

        const int presses = c.buttonPresses;
        const int releases = c.buttonReleases;
        check([&] {
            // Over the parent, but outside the popup.
            manager.onPointerMotionAbsolute(at(2), base::PointF(350, 250));
            manager.onPointerButton(at(3), BTN_LEFT, true);
        });
        c.sync();
        EXPECT_EQ(1, popup->popupDones);
        check([&] {
            manager.onPointerButton(at(4), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(presses, c.buttonPresses);
        EXPECT_EQ(releases, c.buttonReleases);
    });
}

TEST_F(XdgShellProtocolTest, AStillPointerIsMovedWhenTheResizedContentLandsUnderIt)
{
    const base::Subscription synthesize = windowManager.pointerTargetsChanged.connect([this] {
        manager.synthesizeMotion();
    });
    run([&](Client &c) {
        c.bindSeat();
        XdgWindow *window = c.makeMappedToplevel(base::Size(400, 300));
        check([&] {
            onlyWindow()->setBounds(base::Rect(200, 100, 400, 300));
            manager.onPointerMotionAbsolute(at(1), base::PointF(250, 120));
            test::runFor(std::chrono::milliseconds(1));
        });
        c.sync();
        ASSERT_EQ(window->surface, c.pointerFocus);
        EXPECT_EQ(base::PointF(50, 20), c.pointerPosition);
        const int motions = c.motions;

        // The old contents stay where they were until the client draws the new
        // size, so the pointer is still over the same point of them.
        check([&] {
            onlyWindow()->maximize();
            test::runFor(std::chrono::milliseconds(1));
        });
        c.sync();
        EXPECT_EQ(motions, c.motions);
        EXPECT_EQ(base::PointF(50, 20), c.pointerPosition);

        wl_buffer_destroy(window->buffer);
        window->buffer = nullptr;
        c.ackAndAttach(window, base::Size(window->lastToplevel().width, window->lastToplevel().height));
        check([&] {
            EXPECT_EQ(kOutput, onlyWindow()->bounds());
            test::runFor(std::chrono::milliseconds(1));
        });
        c.sync();
        EXPECT_EQ(motions + 1, c.motions);
        EXPECT_EQ(base::PointF(250, 120), c.pointerPosition);

        check([&] {
            onlyWindow()->restore();
            test::runFor(std::chrono::milliseconds(1));
        });
        c.sync();
        EXPECT_EQ(motions + 1, c.motions);

        wl_buffer_destroy(window->buffer);
        window->buffer = nullptr;
        c.ackAndAttach(window, base::Size(window->lastToplevel().width, window->lastToplevel().height));
        check([&] {
            EXPECT_EQ(base::Rect(200, 100, 400, 300), onlyWindow()->bounds());
            test::runFor(std::chrono::milliseconds(1));
        });
        c.sync();
        EXPECT_EQ(motions + 2, c.motions);
        EXPECT_EQ(base::PointF(50, 20), c.pointerPosition);
        EXPECT_EQ(window->surface, c.pointerFocus);
    });
}

////////////////////////////////////////////////////////////////////////////////
// Window positioner

TEST_F(XdgShellProtocolTest, ToplevelsOfOneApplicationCascade)
{
    run([&](Client &c) {
        for (int i = 0; i < 2; ++i) {
            XdgWindow *window = c.makeToplevel();
            xdg_toplevel_set_app_id(window->toplevel, "org.lingmo.test");
            wl_surface_commit(window->surface);
            c.sync();
            c.ackAndAttach(window, base::Size(300, 200));
        }
        check([&] {
            ASSERT_EQ(2u, windowManager.windows().size());
            const base::Rect first = windowManager.windows()[0]->bounds();
            EXPECT_EQ(base::Rect(250, 200, 300, 200), first);
            EXPECT_EQ(first.translated(32, 32), windowManager.windows()[1]->bounds());
        });
    });
}

TEST_F(XdgShellProtocolTest, AToplevelWithAPlacementAnchorMapsNextToIt)
{
    run([&](Client &c) {
        auto *hints = static_cast<lingmo_window_hints_v1 *>(
                wl_registry_bind(c.registry, c.globals.at("lingmo_window_hints_v1").first,
                                 &lingmo_window_hints_v1_interface, 1));
        XdgWindow *window = c.makeToplevel();
        xdg_toplevel_set_app_id(window->toplevel, "org.lingmo.test");
        lingmo_window_hints_v1_set_placement_anchor(
                hints, window->surface, 700, 0, 0, 0, LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_BOTTOM,
                LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_BOTTOM);
        wl_surface_commit(window->surface);
        c.sync();
        c.ackAndAttach(window, base::Size(300, 200));
        // Centred under the point, then slid back from the right edge.
        check([&] {
            ASSERT_EQ(1u, windowManager.windows().size());
            EXPECT_EQ(base::Rect(500, 0, 300, 200), windowManager.windows()[0]->bounds());
        });
        lingmo_window_hints_v1_destroy(hints);
    });
}

TEST_F(XdgShellProtocolTest, ANewToplevelIsConfiguredMaximizedAfterAMaximizedOne)
{
    run([&](Client &c) {
        XdgWindow *first = c.makeToplevel();
        xdg_toplevel_set_app_id(first->toplevel, "org.lingmo.test");
        wl_surface_commit(first->surface);
        c.sync();
        c.ackAndAttach(first, base::Size(300, 200));
        xdg_toplevel_set_maximized(first->toplevel);
        c.sync();

        XdgWindow *second = c.makeToplevel();
        xdg_toplevel_set_app_id(second->toplevel, "org.lingmo.test");
        wl_surface_commit(second->surface);
        c.sync();
        ASSERT_FALSE(second->toplevelConfigures.empty());
        EXPECT_TRUE(second->lastToplevel().has(XDG_TOPLEVEL_STATE_MAXIMIZED));
        EXPECT_EQ(kOutput.width(), second->lastToplevel().width);

        // Neither another application nor a dialog is.
        XdgWindow *other = c.makeToplevel();
        xdg_toplevel_set_app_id(other->toplevel, "org.lingmo.other");
        wl_surface_commit(other->surface);
        XdgWindow *dialog = c.makeToplevel();
        xdg_toplevel_set_app_id(dialog->toplevel, "org.lingmo.test");
        xdg_toplevel_set_parent(dialog->toplevel, first->toplevel);
        wl_surface_commit(dialog->surface);
        c.sync();
        ASSERT_FALSE(other->toplevelConfigures.empty());
        EXPECT_FALSE(other->lastToplevel().has(XDG_TOPLEVEL_STATE_MAXIMIZED));
        ASSERT_FALSE(dialog->toplevelConfigures.empty());
        EXPECT_FALSE(dialog->lastToplevel().has(XDG_TOPLEVEL_STATE_MAXIMIZED));
    });
}

////////////////////////////////////////////////////////////////////////////////
// Window geometry

TEST_F(XdgShellProtocolTest, WindowGeometryDecidesTheWindowSizeAndSurfaceOrigin)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeToplevel();
        wl_surface_commit(window->surface);
        c.sync();

        // A CSD client with a 10px shadow all round.
        xdg_surface_set_window_geometry(window->xdgSurface, 10, 10, 300, 200);
        c.ackAndAttach(window, base::Size(320, 220));

        check([&] {
            wm::Window *managed = onlyWindow();
            ASSERT_NE(nullptr, managed);
            EXPECT_EQ(base::Size(300, 200), managed->bounds().size());
            EXPECT_EQ(base::Point(-10, -10), managed->surfaceOrigin());
        });
    });
}

TEST_F(XdgShellProtocolTest, ANegativeWindowGeometrySizeIsAProtocolError)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        xdg_surface_set_window_geometry(window->xdgSurface, 0, 0, -1, 200);
        EXPECT_EQ(uint32_t(XDG_SURFACE_ERROR_INVALID_SIZE), c.syncExpectingError());
    });
}

////////////////////////////////////////////////////////////////////////////////
// Interactive move and resize

TEST_F(XdgShellProtocolTest, MoveWithASerialThatIsNotAPointerPressIsIgnored)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));

        xdg_toplevel_move(window->toplevel, c.seat, 0xabcdef);
        c.sync();

        check([&] {
            EXPECT_FALSE(windowManager.eventHandler()->isDragging());
        });
    });
}

TEST_F(XdgShellProtocolTest, MoveWithAPointerPressSerialStartsADrag)
{
    run([&](Client &c) {
        c.bindSeat();
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        check([&] {
            onlyWindow()->setBounds(base::Rect(100, 100, 300, 200));
            manager.onPointerMotionAbsolute(at(1), base::PointF(150, 150));
            manager.onPointerButton(at(2), BTN_LEFT, true);
        });
        c.sync();
        ASSERT_NE(0u, c.lastButtonSerial);

        xdg_toplevel_move(window->toplevel, c.seat, c.lastButtonSerial);
        c.sync();

        check([&] {
            EXPECT_TRUE(windowManager.eventHandler()->isDragging());
            EXPECT_EQ(wm::ResizeEdge::None, windowManager.eventHandler()->draggedEdge());
            manager.onPointerMotionAbsolute(at(3), base::PointF(200, 180));
            EXPECT_EQ(base::Rect(150, 130, 300, 200), onlyWindow()->bounds());
            manager.onPointerButton(at(4), BTN_LEFT, false);
            EXPECT_FALSE(windowManager.eventHandler()->isDragging());
            EXPECT_EQ(input::NoButton, seat.pressedButtons());
        });
        c.sync();
        xdg_toplevel_move(window->toplevel, c.seat, c.lastButtonSerial);
        c.sync();
        check([&] {
            EXPECT_FALSE(windowManager.eventHandler()->isDragging());
        });
    });
}

TEST_F(XdgShellProtocolTest, MoveWithATouchDownSerialFollowsTheFinger)
{
    const auto normalized = [](const base::PointF &location) {
        return base::PointF(location.x() / kOutput.width(), location.y() / kOutput.height());
    };
    run([&](Client &c) {
        c.bindSeat();
        c.bindTouch();
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        check([&] {
            onlyWindow()->setBounds(base::Rect(100, 100, 300, 200));
            manager.onTouchDown(at(1), 4, normalized(base::PointF(150, 150)));
            manager.onTouchFrame(at(1));
        });
        c.sync();
        ASSERT_NE(0u, c.lastTouchDownSerial);

        xdg_toplevel_move(window->toplevel, c.seat, c.lastTouchDownSerial);
        c.sync();
        EXPECT_EQ(1, c.touchCancels);

        check([&] {
            EXPECT_TRUE(windowManager.eventHandler()->isTouchDrag());
            manager.onTouchMotion(at(2), 4, normalized(base::PointF(200, 180)));
            manager.onTouchFrame(at(2));
            EXPECT_EQ(base::Rect(150, 130, 300, 200), onlyWindow()->bounds());
            manager.onTouchUp(at(3), 4);
            manager.onTouchFrame(at(3));
            EXPECT_FALSE(windowManager.eventHandler()->isDragging());
        });
        c.sync();
        // The finger is up: its serial moves nothing any more.
        xdg_toplevel_move(window->toplevel, c.seat, c.lastTouchDownSerial);
        c.sync();
        check([&] {
            EXPECT_FALSE(windowManager.eventHandler()->isDragging());
        });
    });
}

TEST_F(XdgShellProtocolTest, DragToRestoreLeavesClientPointerFocusBeforeResizing)
{
    run([&](Client &c) {
        c.bindSeat();
        XdgWindow *window = c.makeMappedToplevel(base::Size(420, 300));
        xdg_toplevel_set_maximized(window->toplevel);
        c.sync();
        c.ackAndAttach(window, kOutput.size());
        check([&] {
            ASSERT_EQ(nullptr, onlyWindow()->frame());
            manager.onPointerMotionAbsolute(at(1), base::PointF(400, 15));
            manager.onPointerButton(at(2), BTN_LEFT, true);
        });
        c.sync();
        ASSERT_EQ(window->surface, c.pointerFocus);
        ASSERT_EQ(base::PointF(400, 15), c.pointerPosition);
        ASSERT_EQ(1, c.buttonPresses);

        xdg_toplevel_move(window->toplevel, c.seat, c.lastButtonSerial);
        c.sync();
        EXPECT_EQ(nullptr, c.pointerFocus);
        check([&] {
            EXPECT_EQ(input::LeftButton, seat.pressedButtons());
            manager.onPointerMotionAbsolute(at(3), base::PointF(400, 115));
            EXPECT_TRUE(onlyWindow()->isRestoringFromDrag());
        });
        c.sync();
        EXPECT_EQ(nullptr, c.pointerFocus);
        c.ackAndAttach(window, base::Size(420, 300));

        // Scrolling must not re-enter the resized client during the move grab.
        check([&] {
            manager.onPointerAxis(at(4), base::PointF(0, 10), false);
            manager.onPointerAxisStop(at(5));
        });
        c.sync();
        EXPECT_EQ(nullptr, c.pointerFocus);
        EXPECT_EQ(0, c.axisEvents);
        EXPECT_EQ(0, c.axisStops);

        check([&] {
            manager.onPointerButton(at(6), BTN_LEFT, false);
            EXPECT_FALSE(windowManager.eventHandler()->isDragging());
            EXPECT_EQ(wm::WindowStateType::Normal, onlyWindow()->stateType());
            EXPECT_EQ(input::NoButton, seat.pressedButtons());
            EXPECT_EQ(nullptr, seat.pointerGrabSurface());
        });
        c.sync();
        EXPECT_EQ(1, c.buttonPresses);
        EXPECT_EQ(0, c.buttonReleases);
        check([&] {
            manager.onPointerMotionAbsolute(at(7), base::PointF(401, 115));
        });
        c.sync();
        EXPECT_EQ(window->surface, c.pointerFocus);
        EXPECT_EQ(base::PointF(211, 15), c.pointerPosition);
        check([&] {
            manager.onPointerButton(at(8), BTN_LEFT, true);
            manager.onPointerButton(at(9), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(2, c.buttonPresses);
        EXPECT_EQ(1, c.buttonReleases);
    });
}

TEST_F(XdgShellProtocolTest, ResizeMapsTheXdgEdgeOntoTheWindowManagerEdge)
{
    run([&](Client &c) {
        c.bindSeat();
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        check([&] {
            onlyWindow()->setBounds(base::Rect(100, 100, 300, 200));
            manager.onPointerMotionAbsolute(at(1), base::PointF(200, 200));
            manager.onPointerButton(at(2), BTN_LEFT, true);
        });
        c.sync();

        xdg_toplevel_resize(window->toplevel, c.seat, c.lastButtonSerial,
                            XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT);
        c.sync();

        check([&] {
            EXPECT_TRUE(windowManager.eventHandler()->isDragging());
            EXPECT_EQ(wm::ResizeEdge::BottomRight, windowManager.eventHandler()->draggedEdge());
        });
        c.sync();
        EXPECT_TRUE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_RESIZING));

        check([&] {
            manager.onPointerButton(at(3), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_FALSE(window->lastToplevel().has(XDG_TOPLEVEL_STATE_RESIZING));
    });
}

TEST_F(XdgShellProtocolTest, AnInvalidResizeEdgeIsAProtocolError)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));
        xdg_toplevel_resize(window->toplevel, c.seat, 1, 3 /* not a valid edge */);
        EXPECT_EQ(uint32_t(XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE), c.syncExpectingError());
    });
}

////////////////////////////////////////////////////////////////////////////////
// Popups

TEST_F(XdgShellProtocolTest, APopupIsConfiguredRelativeToItsParent)
{
    run([&](Client &c) {
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        check([&] {
            onlyWindow()->setBounds(base::Rect(100, 100, 400, 300));
        });
        c.sync();

        XdgWindow *popup = c.makeXdgSurface();
        xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
        xdg_positioner_set_size(positioner, 120, 90);
        xdg_positioner_set_anchor_rect(positioner, 20, 40, 1, 1);
        xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_BOTTOM_LEFT);
        xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, parent->xdgSurface, positioner);
        xdg_popup_add_listener(popup->popup, &popupListener, popup);
        xdg_positioner_destroy(positioner);
        c.sync();

        ASSERT_EQ(1u, popup->popupConfigures.size());
        EXPECT_EQ(20, popup->popupConfigures.front().x);
        EXPECT_EQ(41, popup->popupConfigures.front().y);
        EXPECT_EQ(120, popup->popupConfigures.front().width);
        ASSERT_EQ(1u, popup->surfaceConfigures.size());

        c.ackAndAttach(popup, base::Size(120, 90));

        check([&] {
            EXPECT_EQ(2u, windowManager.windows().size());
            ASSERT_EQ(2u, compositor.surfaces().size());
            compositor::Surface *popupSurface = compositor.surfaces()[1];
            EXPECT_EQ(base::Point(120, 141), popupSurface->node()->absolutePosition());
            auto *popupWindow = windowManager.windowForSurface(popupSurface);
            ASSERT_NE(nullptr, popupWindow);
            EXPECT_TRUE(popupWindow->isPopup());
            EXPECT_EQ(onlyWindow(), popupWindow->transientParent());
        });
    });
}

TEST_F(XdgShellProtocolTest, APopupWithAGrabIsDismissedByAPressOutsideIt)
{
    run([&](Client &c) {
        c.bindSeat();
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        check([&] {
            onlyWindow()->setBounds(base::Rect(0, 0, 400, 300));
        });
        c.sync();

        XdgWindow *popup = c.makeXdgSurface();
        xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
        xdg_positioner_set_size(positioner, 100, 80);
        xdg_positioner_set_anchor_rect(positioner, 10, 20, 1, 1);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, parent->xdgSurface, positioner);
        xdg_popup_add_listener(popup->popup, &popupListener, popup);
        xdg_popup_grab(popup->popup, c.seat, c.lastEnterSerial);
        xdg_positioner_destroy(positioner);
        c.sync();
        c.ackAndAttach(popup, base::Size(100, 80));

        check([&] {
            EXPECT_EQ(1u, windowManager.popupGrabs().size());
            // An application's popup leaves the keyboard with the application.
            EXPECT_EQ(compositor.surfaces()[0], seat.focusedSurface());
            EXPECT_FALSE(seat.focusIsBorrowed());
            // Well outside the popup, and outside the parent too.
            manager.onPointerMotionAbsolute(at(1), base::PointF(700, 500));
            manager.onPointerButton(at(2), BTN_LEFT, true);
        });
        c.sync();

        EXPECT_EQ(1, popup->popupDones);
        check([&] {
            EXPECT_TRUE(windowManager.popupGrabs().empty());
        });
    });
}

// A grabbing popup on `parent`, mapped, anchored at `anchor` in its parent.
XdgWindow *makeGrabbingPopup(Client &c, XdgWindow *parent, const base::Point &anchor)
{
    XdgWindow *popup = c.makeXdgSurface();
    xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
    xdg_positioner_set_size(positioner, 100, 80);
    xdg_positioner_set_anchor_rect(positioner, anchor.x(), anchor.y(), 1, 1);
    xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
    xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
    popup->popup = xdg_surface_get_popup(popup->xdgSurface, parent->xdgSurface, positioner);
    xdg_popup_add_listener(popup->popup, &popupListener, popup);
    xdg_popup_grab(popup->popup, c.seat, c.lastEnterSerial);
    xdg_positioner_destroy(positioner);
    c.sync();
    c.ackAndAttach(popup, base::Size(100, 80));
    return popup;
}

TEST_F(XdgShellProtocolTest, AGrabFromAnotherClientDismissesTheGrabChainHeldNow)
{
    run([&](Client &a) {
        a.bindSeat();
        XdgWindow *parentA = a.makeMappedToplevel(base::Size(300, 300));
        check([&] {
            onlyWindow()->setBounds(base::Rect(0, 0, 300, 300));
        });
        a.sync();
        XdgWindow *menu = makeGrabbingPopup(a, parentA, base::Point(10, 10));
        XdgWindow *submenu = makeGrabbingPopup(a, menu, base::Point(90, 10));
        check([&] {
            EXPECT_EQ(2u, windowManager.popupGrabs().size());
        });
        EXPECT_EQ(0, menu->popupDones);
        EXPECT_EQ(0, submenu->popupDones);

        std::unique_ptr<Client> b = connectClient();
        b->bindSeat();
        XdgWindow *parentB = b->makeMappedToplevel(base::Size(300, 300));
        check([&] {
            windowManager.windowForSurface(compositor.surfaces()[3])
                    ->setBounds(base::Rect(400, 0, 300, 300));
        });
        b->sync();
        XdgWindow *popupB = makeGrabbingPopup(*b, parentB, base::Point(10, 10));
        a.sync();

        EXPECT_EQ(1, menu->popupDones);
        EXPECT_EQ(1, submenu->popupDones);
        EXPECT_EQ(0, popupB->popupDones);
        check([&] {
            ASSERT_EQ(1u, windowManager.popupGrabs().size());
            EXPECT_EQ(windowManager.windowForSurface(compositor.surfaces()[4]),
                      windowManager.popupGrabs().front()->window());
        });

        // The new chain works as a chain of its own: a press inside it reaches
        // the client and leaves it open.
        const int pressesBefore = b->buttonPresses;
        check([&] {
            manager.onPointerMotionAbsolute(at(1), base::PointF(450, 50));
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
        });
        b->sync();
        EXPECT_EQ(pressesBefore + 1, b->buttonPresses);
        EXPECT_EQ(0, popupB->popupDones);
        check([&] {
            EXPECT_EQ(1u, windowManager.popupGrabs().size());
        });

        a.destroyWindowObjects(submenu);
        a.destroyWindowObjects(menu);
        a.sync();
        b.reset();
        a.sync();
    });
}

TEST_F(XdgShellProtocolTest, AClientsNestedGrabsStackOnItsOwnChain)
{
    run([&](Client &c) {
        c.bindSeat();
        XdgWindow *parent = c.makeMappedToplevel(base::Size(300, 300));
        check([&] {
            onlyWindow()->setBounds(base::Rect(0, 0, 300, 300));
        });
        c.sync();
        XdgWindow *menu = makeGrabbingPopup(c, parent, base::Point(10, 10));
        XdgWindow *submenu = makeGrabbingPopup(c, menu, base::Point(90, 10));
        XdgWindow *subsubmenu = makeGrabbingPopup(c, submenu, base::Point(90, 10));

        EXPECT_EQ(0, menu->popupDones);
        EXPECT_EQ(0, submenu->popupDones);
        EXPECT_EQ(0, subsubmenu->popupDones);
        check([&] {
            EXPECT_EQ(3u, windowManager.popupGrabs().size());
        });

        c.destroyWindowObjects(subsubmenu);
        c.destroyWindowObjects(submenu);
        c.destroyWindowObjects(menu);
        c.sync();
    });
}

TEST_F(XdgShellProtocolTest, AStatusBarPopupBorrowsTheKeyboardAndLeavesTheBarItsPresses)
{
    run([&](Client &c) {
        c.bindSeat();
        XdgWindow *application = c.makeMappedToplevel(base::Size(400, 300));
        XdgWindow *bar = c.makeMappedToplevel(base::Size(800, 30));
        wm::Window *applicationWindow = nullptr;
        compositor::Surface *applicationSurface = nullptr;
        compositor::Surface *barSurface = nullptr;
        check([&] {
            ASSERT_EQ(2u, compositor.surfaces().size());
            applicationSurface = compositor.surfaces()[0];
            barSurface = compositor.surfaces()[1];
            applicationWindow = windowManager.windowForSurface(applicationSurface);
            wm::Window *barWindow = windowManager.windowForSurface(compositor.surfaces()[1]);
            applicationWindow->setBounds(base::Rect(100, 100, 400, 300));
            barWindow->setBounds(base::Rect(0, 0, 800, 30));
            barWindow->setRole(wm::WindowRole::Panel);
            barWindow->setFocusPolicy(wm::FocusPolicy::NoFocus);
            windowManager.activation()->activate(applicationWindow);
            EXPECT_EQ(applicationSurface, seat.focusedSurface());
        });
        c.sync();

        XdgWindow *popup = c.makeXdgSurface();
        xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
        xdg_positioner_set_size(positioner, 100, 80);
        xdg_positioner_set_anchor_rect(positioner, 600, 30, 1, 1);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, bar->xdgSurface, positioner);
        xdg_popup_add_listener(popup->popup, &popupListener, popup);
        xdg_popup_grab(popup->popup, c.seat, c.lastEnterSerial);
        xdg_positioner_destroy(positioner);
        c.sync();
        // From the grab on, before the popup maps: the client hands the keys on.
        check([&] {
            EXPECT_EQ(barSurface, seat.focusedSurface());
            EXPECT_TRUE(seat.focusIsBorrowed());
        });
        c.ackAndAttach(popup, base::Size(100, 80));

        const int pressesBefore = c.buttonPresses;
        check([&] {
            EXPECT_EQ(barSurface, seat.focusedSurface());
            EXPECT_EQ(applicationWindow, windowManager.activation()->activeWindow());

            // On the bar, beside the popup: the bar gets it, the popup stays.
            manager.onPointerMotionAbsolute(at(1), base::PointF(100, 10));
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
        });
        c.sync();

        EXPECT_EQ(0, popup->popupDones);
        EXPECT_EQ(pressesBefore + 1, c.buttonPresses);
        check([&] {
            EXPECT_EQ(1u, windowManager.popupGrabs().size());
            EXPECT_EQ(barSurface, seat.focusedSurface());

            // Anywhere else the press only dismisses the popup.
            manager.onPointerMotionAbsolute(at(4), base::PointF(300, 250));
            manager.onPointerButton(at(5), BTN_LEFT, true);
            manager.onPointerButton(at(6), BTN_LEFT, false);
        });
        c.sync();

        EXPECT_EQ(1, popup->popupDones);
        EXPECT_EQ(pressesBefore + 1, c.buttonPresses);

        c.destroyWindowObjects(popup);
        c.sync();
        check([&] {
            EXPECT_EQ(applicationSurface, seat.focusedSurface());
            EXPECT_FALSE(seat.focusIsBorrowed());
        });
    });
}

TEST_F(XdgShellProtocolTest, RepositionAnnouncesTheTokenAndConfiguresAgain)
{
    run([&](Client &c) {
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        XdgWindow *popup = c.makeXdgSurface();
        xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
        xdg_positioner_set_size(positioner, 100, 80);
        xdg_positioner_set_anchor_rect(positioner, 10, 20, 1, 1);
        xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
        xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, parent->xdgSurface, positioner);
        xdg_popup_add_listener(popup->popup, &popupListener, popup);
        c.sync();
        c.ackAndAttach(popup, base::Size(100, 80));

        xdg_positioner_set_anchor_rect(positioner, 50, 60, 1, 1);
        xdg_popup_reposition(popup->popup, positioner, 77);
        xdg_positioner_destroy(positioner);
        c.sync();

        ASSERT_EQ(1u, popup->repositioned.size());
        EXPECT_EQ(77u, popup->repositioned.front());
        ASSERT_EQ(2u, popup->popupConfigures.size());
        EXPECT_EQ(50, popup->popupConfigures.back().x);
    });
}

TEST_F(XdgShellProtocolTest, APopupWithoutAPositionerSizeIsAProtocolError)
{
    run([&](Client &c) {
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        XdgWindow *popup = c.makeXdgSurface();
        xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, parent->xdgSurface, positioner);
        EXPECT_EQ(uint32_t(XDG_WM_BASE_ERROR_INVALID_POSITIONER), c.syncExpectingError());
        xdg_positioner_destroy(positioner);
    });
}

TEST_F(XdgShellProtocolTest, DestroyingThePopupParentLeavesTheCompositorStanding)
{
    run([&](Client &c) {
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        XdgWindow *popup = c.makeXdgSurface();
        xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
        xdg_positioner_set_size(positioner, 100, 80);
        xdg_positioner_set_anchor_rect(positioner, 10, 20, 1, 1);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, parent->xdgSurface, positioner);
        xdg_popup_add_listener(popup->popup, &popupListener, popup);
        xdg_positioner_destroy(positioner);
        c.sync();
        c.ackAndAttach(popup, base::Size(100, 80));

        // The parent goes first, which the protocol forbids but a crashing
        // toolkit still manages.
        xdg_toplevel_destroy(parent->toplevel);
        parent->toplevel = nullptr;
        xdg_surface_destroy(parent->xdgSurface);
        parent->xdgSurface = nullptr;
        c.sync();

        check([&] {
            EXPECT_TRUE(windowManager.windows().empty());
        });
    });
}

////////////////////////////////////////////////////////////////////////////////
// Unmapping and teardown

TEST_F(XdgShellProtocolTest, ANullBufferCommitUnmapsAndACommitAfterItRemaps)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));

        wl_surface_attach(window->surface, nullptr, 0, 0);
        wl_surface_commit(window->surface);
        c.sync();
        check([&] {
            EXPECT_TRUE(windowManager.windows().empty());
        });

        const size_t before = window->surfaceConfigures.size();
        wl_surface_commit(window->surface);
        c.sync();
        ASSERT_GT(window->surfaceConfigures.size(), before);
        c.ackAndAttach(window, base::Size(300, 200));

        check([&] {
            EXPECT_EQ(1u, windowManager.windows().size());
        });
    });
}

TEST_F(XdgShellProtocolTest, APopupSurfaceThatKeptItsBufferCanTakeANewPopup)
{
    run([&](Client &c) {
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));

        auto givePopup = [&](XdgWindow *window) {
            xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
            xdg_positioner_set_size(positioner, 120, 90);
            xdg_positioner_set_anchor_rect(positioner, 20, 40, 1, 1);
            window->popup = xdg_surface_get_popup(window->xdgSurface, parent->xdgSurface,
                                                  positioner);
            xdg_popup_add_listener(window->popup, &popupListener, window);
            xdg_positioner_destroy(positioner);
            wl_surface_commit(window->surface);
            c.sync();
        };

        XdgWindow *popup = c.makeXdgSurface();
        givePopup(popup);
        c.ackAndAttach(popup, base::Size(120, 90));

        // Qt strips a child popup of its role this way and leaves the buffer
        // on the wl_surface.
        xdg_popup_destroy(popup->popup);
        xdg_surface_destroy(popup->xdgSurface);
        c.sync();
        check([&] {
            EXPECT_EQ(1u, windowManager.windows().size());
        });

        popup->surfaceConfigures.clear();
        popup->xdgSurface = xdg_wm_base_get_xdg_surface(c.wmBase, popup->surface);
        xdg_surface_add_listener(popup->xdgSurface, &surfaceListener, popup);
        givePopup(popup);
        EXPECT_EQ(0u, c.syncExpectingError());
        ASSERT_FALSE(popup->surfaceConfigures.empty());

        c.ackAndAttach(popup, base::Size(120, 90));
        check([&] {
            EXPECT_EQ(2u, windowManager.windows().size());
        });
    });
}

TEST_F(XdgShellProtocolTest, DestroyingTheWlSurfaceFirstIsRefusedButLeavesNoWindow)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));

        wl_surface_destroy(window->surface);
        window->surface = nullptr;
        // The server tears the window down and tells the client it broke the
        // protocol.
        c.syncExpectingError();

        check([&] {
            EXPECT_TRUE(windowManager.windows().empty());
        });
    });
}

TEST_F(XdgShellProtocolTest, DestroyingTheToplevelKeepsTheXdgSurfaceUsable)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(300, 200));

        xdg_toplevel_destroy(window->toplevel);
        window->toplevel = nullptr;
        c.sync();

        check([&] {
            EXPECT_TRUE(windowManager.windows().empty());
        });

        // The xdg_surface is roleless again, so it can be given a new one.
        window->toplevel = xdg_surface_get_toplevel(window->xdgSurface);
        xdg_toplevel_add_listener(window->toplevel, &toplevelListener, window);
        wl_surface_attach(window->surface, nullptr, 0, 0);
        wl_surface_commit(window->surface);
        c.sync();
        EXPECT_EQ(0u, c.syncExpectingError());
    });
}

// Exo's ShellDestruction: whether the client closes the window itself or does
// so because the window manager asked, nothing of the window is left behind.
TEST_F(XdgShellProtocolTest, AToplevelClosedOnRequestLeavesNothingBehind)
{
    run([&](Client &c) {
        for (bool onRequest : {false, true}) {
            XdgWindow *window = c.makeMappedToplevel(base::Size(256, 256));
            if (onRequest) {
                check([&] {
                    ASSERT_NE(nullptr, onlyWindow());
                    onlyWindow()->requestClose();
                });
                c.sync();
                EXPECT_EQ(1, window->closes);
            }
            c.destroyWindowObjects(window);
            c.sync();
            check([&] {
                EXPECT_TRUE(windowManager.windows().empty());
                EXPECT_TRUE(compositor.surfaces().empty());
                EXPECT_EQ(nullptr, windowManager.activation()->activeWindow());
            });
        }
    });
}

TEST_F(XdgShellProtocolTest, DestroyingTheWmBaseBeforeItsSurfacesIsAnError)
{
    run([&](Client &c) {
        c.makeMappedToplevel(base::Size(100, 100));
        xdg_wm_base_destroy(c.wmBase);
        c.wmBase = nullptr;
        EXPECT_EQ(uint32_t(XDG_WM_BASE_ERROR_DEFUNCT_SURFACES), c.syncExpectingError());
    });
}

TEST_F(XdgShellProtocolTest, DestroyingTheWmBaseAfterItsSurfacesIsAllowed)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeMappedToplevel(base::Size(100, 100));
        c.destroyWindowObjects(window);
        xdg_wm_base_destroy(c.wmBase);
        c.wmBase = nullptr;
        EXPECT_EQ(0u, c.syncExpectingError());
    });
}

// Exo's DestroyRootSurfaceBeforeCommit: a surface torn down with a buffer
// attached but never committed leaves no surface and no window.
TEST_F(XdgShellProtocolTest, DestroyingARoleSurfaceBeforeItsFirstCommit)
{
    run([&](Client &c) {
        XdgWindow *window = c.makeToplevel();
        window->buffer = c.makeBuffer(base::Size(256, 256));
        wl_surface_attach(window->surface, window->buffer, 0, 0);
        c.sync();
        check([&] {
            EXPECT_EQ(1u, compositor.surfaces().size());
            EXPECT_TRUE(windowManager.windows().empty());
        });
        c.destroyWindowObjects(window);
        EXPECT_EQ(0u, c.syncExpectingError());
        check([&] {
            EXPECT_TRUE(compositor.surfaces().empty());
            EXPECT_TRUE(windowManager.windows().empty());
        });
    });
}

TEST_F(XdgShellProtocolTest, DisconnectingWithAnActiveToplevelLeavesNothingBehind)
{
    run([&](Client &c) {
        c.makeMappedToplevel(base::Size(300, 200));
        c.makeMappedToplevel(base::Size(200, 150));
        check([&] {
            EXPECT_EQ(2u, windowManager.windows().size());
            EXPECT_NE(nullptr, windowManager.activation()->activeWindow());
        });
        // The client destructor disconnects; run()'s teardown checks that the
        // compositor is left with no surfaces, no windows and no focus.
    });
}

} // namespace

TEST_F(XdgShellProtocolTest, DesktopPopupRespectsPanelAndDockExclusiveZones)
{
    run([&](Client &c) {
        XdgWindow *parent = c.makeMappedToplevel(base::Size(800, 600));
        check([&] {
            onlyWindow()->setRole(wm::WindowRole::Desktop);
            onlyWindow()->setBounds(base::Rect(0, 0, 800, 600));
            windowManager.setExclusiveZone(&compositor, wm::ScreenEdge::Top, 28);
            windowManager.setExclusiveZone(&windowManager, wm::ScreenEdge::Bottom, 60);
            EXPECT_EQ(base::Rect(0, 28, 800, 512), windowManager.workAreaFor(nullptr));
            EXPECT_EQ(base::Rect(0, 0, 800, 600), windowManager.workAreaFor(onlyWindow()));
        });
        c.sync();
        XdgWindow *popup = c.makeXdgSurface();
        auto *positioner = xdg_wm_base_create_positioner(c.wmBase);
        xdg_positioner_set_size(positioner, 120, 90);
        xdg_positioner_set_anchor_rect(positioner, 10, 570, 1, 1);
        xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_BOTTOM_LEFT);
        xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
        xdg_positioner_set_constraint_adjustment(positioner,
                                                 XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, parent->xdgSurface, positioner);
        xdg_popup_add_listener(popup->popup, &popupListener, popup);
        c.sync();
        ASSERT_EQ(1u, popup->popupConfigures.size());
        EXPECT_EQ(450, popup->popupConfigures.back().y);
        xdg_positioner_set_anchor_rect(positioner, 10, 0, 1, 1);
        xdg_popup_reposition(popup->popup, positioner, 1);
        c.sync();
        ASSERT_EQ(2u, popup->popupConfigures.size());
        EXPECT_EQ(28, popup->popupConfigures.back().y);
        xdg_positioner_destroy(positioner);
    });
}

TEST_F(XdgShellProtocolTest, APopupIsConstrainedToItsParentsOutput)
{
    display::DisplayManager displays;
    std::vector<display::DisplaySnapshot> physical;
    for (const char *name : {"a", "b"}) {
        display::DisplaySnapshot snapshot;
        snapshot.sessionId = display::DisplayId(name);
        snapshot.persistentKey = std::string("edid-") + name;
        snapshot.portKey = std::string("port-") + name;
        snapshot.connectorName = name;
        snapshot.devicePath = "/dev/dri/card0";
        snapshot.physicalSizeMm = {600, 340};
        snapshot.modes = {{kOutput.size(), 60000, 148500, kOutput.width() + 100,
                           kOutput.height() + 50, 0, "mode", true}};
        snapshot.currentMode = snapshot.modes.front();
        physical.push_back(snapshot);
    }
    ASSERT_TRUE(displays.installHardware({"instance", "seat0", 1, physical}));
    const base::Rect secondary(800, 0, 800, 600);
    const auto logical = [](const char *name, const base::Rect &bounds, bool primary) {
        return display::LogicalDisplay {display::DisplayId(name),
                                        {display::DisplayId(name)},
                                        std::string("Lingmo-") + name,
                                        bounds,
                                        bounds.adjusted(0, 24, 0, 0),
                                        {},
                                        display::DisplayRotation::Normal,
                                        primary};
    };
    ASSERT_TRUE(displays.installApplied(
            {"instance",
             1,
             1,
             {},
             {logical("a", kOutput, true), logical("b", secondary, false)},
             {{display::DisplayId("b"), display::DisplayId("a"), display::LayoutEdge::Right, 0}}}));
    windowManager.setDisplayManager(&displays);

    run([&](Client &c) {
        XdgWindow *parent = c.makeMappedToplevel(base::Size(400, 300));
        check([&] {
            onlyWindow()->setBounds(base::Rect(900, 100, 400, 300));
        });
        c.sync();

        XdgWindow *popup = c.makeXdgSurface();
        xdg_positioner *positioner = xdg_wm_base_create_positioner(c.wmBase);
        xdg_positioner_set_size(positioner, 120, 90);
        xdg_positioner_set_anchor_rect(positioner, 20, 40, 1, 1);
        xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_BOTTOM_LEFT);
        xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
        xdg_positioner_set_constraint_adjustment(positioner,
                                                 XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X
                                                         | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, parent->xdgSurface, positioner);
        xdg_popup_add_listener(popup->popup, &popupListener, popup);
        xdg_positioner_destroy(positioner);
        c.sync();

        // Well inside the secondary output, so nothing moves it.
        ASSERT_EQ(1u, popup->popupConfigures.size());
        EXPECT_EQ(20, popup->popupConfigures.front().x);
        EXPECT_EQ(41, popup->popupConfigures.front().y);
    });
    windowManager.setDisplayManager(nullptr);
}

namespace {

xdg_activation_v1 *bindActivation(Client &client)
{
    return static_cast<xdg_activation_v1 *>(
            wl_registry_bind(client.registry, client.globals.at("xdg_activation_v1").first,
                             &xdg_activation_v1_interface, 1));
}

const xdg_activation_token_v1_listener activationTokenListener = {
        [](void *data, xdg_activation_token_v1 *, const char *token) {
            *static_cast<std::string *>(data) = token;
        }};

std::string requestActivationToken(Client &client, xdg_activation_v1 *activation,
                                   wl_surface *surface, uint32_t serial)
{
    std::string result;
    auto *request = xdg_activation_v1_get_activation_token(activation);
    xdg_activation_token_v1_add_listener(request, &activationTokenListener, &result);
    if (client.seat)
        xdg_activation_token_v1_set_serial(request, serial, client.seat);
    if (surface)
        xdg_activation_token_v1_set_surface(request, surface);
    xdg_activation_token_v1_set_app_id(request, "org.lingmo.ActivationTest");
    xdg_activation_token_v1_commit(request);
    client.sync();
    xdg_activation_token_v1_destroy(request);
    EXPECT_FALSE(result.empty());
    return result;
}

} // namespace

TEST_F(XdgShellProtocolTest, ActivationTokenIsSingleUseAndRestoresMinimizedTarget)
{
    run([&](Client &c) {
        c.bindSeat();
        auto *target = c.makeMappedToplevel({200, 150});
        wm::Window *targetWindow = nullptr;
        check([&] {
            targetWindow = onlyWindow();
        });
        auto *source = c.makeMappedToplevel({200, 150});
        wm::Window *sourceWindow = nullptr;
        check([&] {
            sourceWindow = windowManager.activation()->activeWindow();
            sourceWindow->setBounds({100, 100, 200, 150});
            manager.onPointerMotionAbsolute(at(1), {150, 150});
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
            targetWindow->minimize();
        });
        c.sync();
        ASSERT_NE(0u, c.lastButtonSerial);
        auto *activation = bindActivation(c);
        const auto token =
                requestActivationToken(c, activation, source->surface, c.lastButtonSerial);
        xdg_activation_v1_destroy(activation);
        activation = bindActivation(c);
        xdg_activation_v1_activate(activation, token.c_str(), target->surface);
        c.sync();
        check([&] {
            EXPECT_EQ(targetWindow, windowManager.activation()->activeWindow());
            EXPECT_EQ(targetWindow->rootSurface(), seat.focusedSurface());
            EXPECT_FALSE(targetWindow->state()->isMinimized());
            sourceWindow->activate();
        });
        xdg_activation_v1_activate(activation, token.c_str(), target->surface);
        c.sync();
        check([&] {
            EXPECT_EQ(sourceWindow, windowManager.activation()->activeWindow());
        });
        const auto reused =
                requestActivationToken(c, activation, source->surface, c.lastButtonSerial);
        xdg_activation_v1_activate(activation, reused.c_str(), target->surface);
        c.sync();
        check([&] {
            EXPECT_EQ(sourceWindow, windowManager.activation()->activeWindow());
        });
        xdg_activation_v1_destroy(activation);
    });
}

TEST_F(XdgShellProtocolTest, ShellTokenRestoresAMinimizedWindowOnce)
{
    run([&](Client &c) {
        auto *target = c.makeMappedToplevel({200, 150});
        wm::Window *targetWindow = nullptr;
        check([&] {
            targetWindow = onlyWindow();
        });
        c.makeMappedToplevel({200, 150});
        wm::Window *sourceWindow = nullptr;
        std::string token;
        check([&] {
            sourceWindow = windowManager.activation()->activeWindow();
            targetWindow->minimize();
            token = xdgActivation->createShellToken();
        });
        auto *activation = bindActivation(c);
        xdg_activation_v1_activate(activation, token.c_str(), target->surface);
        c.sync();
        check([&] {
            EXPECT_EQ(targetWindow, windowManager.activation()->activeWindow());
            EXPECT_FALSE(targetWindow->state()->isMinimized());
            sourceWindow->activate();
        });
        xdg_activation_v1_activate(activation, token.c_str(), target->surface);
        c.sync();
        check([&] {
            EXPECT_EQ(sourceWindow, windowManager.activation()->activeWindow());
        });
        xdg_activation_v1_destroy(activation);
    });
}

TEST_F(XdgShellProtocolTest, ActivationRejectsMissingAndInventedSerials)
{
    run([&](Client &c) {
        auto *target = c.makeMappedToplevel({200, 150});
        auto *source = c.makeMappedToplevel({200, 150});
        wm::Window *sourceWindow = nullptr;
        check([&] {
            sourceWindow = windowManager.activation()->activeWindow();
        });
        auto *activation = bindActivation(c);
        auto token = requestActivationToken(c, activation, source->surface, 0);
        xdg_activation_v1_activate(activation, token.c_str(), target->surface);
        c.bindSeat();
        token = requestActivationToken(c, activation, source->surface, 0xffffffffu);
        xdg_activation_v1_activate(activation, token.c_str(), target->surface);
        xdg_activation_v1_activate(activation, "unknown", target->surface);
        c.sync();
        check([&] {
            EXPECT_EQ(sourceWindow, windowManager.activation()->activeWindow());
        });
        xdg_activation_v1_destroy(activation);
    });
}

TEST_F(XdgShellProtocolTest, ActivationTokenCanBeTransferredToAnotherClient)
{
    run([&](Client &c) {
        c.bindSeat();
        auto *source = c.makeMappedToplevel({200, 150});
        check([&] {
            onlyWindow()->setBounds({100, 100, 200, 150});
            manager.onPointerMotionAbsolute(at(1), {150, 150});
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
        });
        c.sync();
        ASSERT_NE(0u, c.lastEnterSerial);
        auto *activation = bindActivation(c);
        const auto token =
                requestActivationToken(c, activation, source->surface, c.lastEnterSerial);
        int fds[2];
        ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds));
        check([&] {
            ASSERT_TRUE(wl_client_create(server.display(), fds[0]));
        });
        {
            Client other(fds[1]);
            auto *target = other.makeMappedToplevel({200, 150});
            wm::Window *targetWindow = nullptr;
            wm::Window *sourceWindow = nullptr;
            check([&] {
                targetWindow = windowManager.activation()->activeWindow();
                sourceWindow = windowManager.windows().front();
                sourceWindow->activate();
            });
            auto *otherActivation = bindActivation(other);
            xdg_activation_v1_activate(otherActivation, token.c_str(), target->surface);
            other.sync();
            check([&] {
                EXPECT_EQ(targetWindow, windowManager.activation()->activeWindow());
            });
            other.bindSeat();
            const auto forged = requestActivationToken(other, otherActivation, target->surface,
                                                       c.lastButtonSerial);
            check([&] {
                sourceWindow->activate();
            });
            xdg_activation_v1_activate(otherActivation, forged.c_str(), target->surface);
            other.sync();
            check([&] {
                EXPECT_EQ(sourceWindow, windowManager.activation()->activeWindow());
            });
            xdg_activation_v1_destroy(otherActivation);
        }
        xdg_activation_v1_destroy(activation);
    });
}

TEST_F(XdgShellProtocolTest, ActivationRequestSurvivesManagerAndParameterDestruction)
{
    run([&](Client &c) {
        c.bindSeat();
        auto *source = c.makeMappedToplevel({200, 150});
        auto *activation = bindActivation(c);
        auto *request = xdg_activation_v1_get_activation_token(activation);
        std::string token;
        xdg_activation_token_v1_add_listener(request, &activationTokenListener, &token);
        xdg_activation_token_v1_set_serial(request, 1, c.seat);
        xdg_activation_token_v1_set_surface(request, source->surface);
        xdg_activation_v1_destroy(activation);
        c.destroyWindowObjects(source);
        wl_seat_release(c.seat);
        c.seat = nullptr;
        xdg_activation_token_v1_commit(request);
        c.sync();
        EXPECT_FALSE(token.empty());
        EXPECT_EQ(0, wl_display_get_error(c.display));
        xdg_activation_token_v1_destroy(request);
    });
}

class ActivationCommittedTest : public XdgShellProtocolTest, public testing::WithParamInterface<int>
{
};

TEST_P(ActivationCommittedTest, RejectsRequestsAfterCommit)
{
    const int operation = GetParam();
    run([&](Client &c) {
        c.bindSeat();
        auto *surface = wl_compositor_create_surface(c.compositor);
        auto *activation = bindActivation(c);
        auto *request = xdg_activation_v1_get_activation_token(activation);
        std::string token;
        xdg_activation_token_v1_add_listener(request, &activationTokenListener, &token);
        xdg_activation_token_v1_commit(request);
        c.sync();
        switch (operation) {
        case 0:
            xdg_activation_token_v1_commit(request);
            break;
        case 1:
            xdg_activation_token_v1_set_app_id(request, "test");
            break;
        case 2:
            xdg_activation_token_v1_set_surface(request, surface);
            break;
        case 3:
            xdg_activation_token_v1_set_serial(request, 1, c.seat);
            break;
        }
        const wl_interface *errorInterface = nullptr;
        EXPECT_EQ(XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED,
                  c.syncExpectingError(&errorInterface));
        ASSERT_NE(nullptr, errorInterface);
        EXPECT_STREQ("xdg_activation_token_v1", errorInterface->name);
        xdg_activation_token_v1_destroy(request);
        xdg_activation_v1_destroy(activation);
        wl_surface_destroy(surface);
    });
}

INSTANTIATE_TEST_SUITE_P(Activation, ActivationCommittedTest, testing::Range(0, 4));

TEST_F(XdgShellProtocolTest, ActivationBeforeMappingRestoresAnInitiallyMinimizedWindow)
{
    run([&](Client &c) {
        c.bindSeat();
        auto *source = c.makeMappedToplevel({200, 150});
        check([&] {
            onlyWindow()->setBounds({100, 100, 200, 150});
            manager.onPointerMotionAbsolute(at(1), {150, 150});
        });
        c.sync();
        auto *activation = bindActivation(c);
        const auto token =
                requestActivationToken(c, activation, source->surface, c.lastEnterSerial);
        auto *target = c.makeToplevel();
        xdg_toplevel_set_minimized(target->toplevel);
        xdg_activation_v1_activate(activation, token.c_str(), target->surface);
        wl_surface_commit(target->surface);
        c.sync();
        c.ackAndAttach(target, {200, 150});
        check([&] {
            ASSERT_EQ(2u, windowManager.windows().size());
            auto *active = windowManager.activation()->activeWindow();
            ASSERT_NE(nullptr, active);
            EXPECT_FALSE(active->state()->isMinimized());
            EXPECT_EQ(windowManager.windows().back(), active);
            EXPECT_NE(windowManager.windows().front(), active);
        });
        xdg_activation_v1_destroy(activation);
    });
}

TEST_F(XdgShellProtocolTest, PendingActivationIsRemovedWhenTargetIsDestroyed)
{
    run([&](Client &c) {
        c.bindSeat();
        auto *source = c.makeMappedToplevel({200, 150});
        check([&] {
            onlyWindow()->setBounds({100, 100, 200, 150});
            manager.onPointerMotionAbsolute(at(1), {150, 150});
        });
        c.sync();
        auto *activation = bindActivation(c);
        const auto token = requestActivationToken(c, activation, nullptr, c.lastEnterSerial);
        auto *target = c.makeToplevel();
        xdg_activation_v1_activate(activation, token.c_str(), target->surface);
        c.sync();
        c.destroyWindowObjects(target);
        c.sync();
        check([&] {
            EXPECT_EQ(1u, windowManager.windows().size());
            EXPECT_EQ(onlyWindow(), windowManager.activation()->activeWindow());
        });
        xdg_activation_v1_destroy(activation);
    });
}

TEST_F(XdgShellProtocolTest, ActivationTokenAndInputSerialExpire)
{
    run(
            [&](Client &c) {
                c.bindSeat();
                auto *target = c.makeMappedToplevel({200, 150});
                auto *source = c.makeMappedToplevel({200, 150});
                wm::Window *sourceWindow = nullptr;
                check([&] {
                    sourceWindow = windowManager.activation()->activeWindow();
                    sourceWindow->setBounds({100, 100, 200, 150});
                    manager.onPointerMotionAbsolute(at(1), {150, 150});
                    manager.onPointerButton(at(2), BTN_LEFT, true);
                    manager.onPointerButton(at(3), BTN_LEFT, false);
                });
                c.sync();
                auto *activation = bindActivation(c);
                const auto token =
                        requestActivationToken(c, activation, source->surface, c.lastEnterSerial);
                std::this_thread::sleep_for(std::chrono::milliseconds(10100));
                xdg_activation_v1_activate(activation, token.c_str(), target->surface);
                const auto stale =
                        requestActivationToken(c, activation, source->surface, c.lastButtonSerial);
                xdg_activation_v1_activate(activation, stale.c_str(), target->surface);
                c.sync();
                check([&] {
                    EXPECT_EQ(sourceWindow, windowManager.activation()->activeWindow());
                });
                xdg_activation_v1_destroy(activation);
            },
            std::chrono::seconds(20));
}

////////////////////////////////////////////////////////////////////////////////
// Windows placed by their client

namespace {

struct PlacementEvents {
    std::vector<std::pair<wl_surface *, base::Point>> positions;

    base::Point last() const { return positions.empty() ? base::Point(-1, -1) : positions.back().second; }
};

lingmo_window_placement_v1 *bindPlacement(Client &c, PlacementEvents *events)
{
    auto *placement = static_cast<lingmo_window_placement_v1 *>(
            wl_registry_bind(c.registry, c.globals.at("lingmo_window_placement_v1").first,
                             &lingmo_window_placement_v1_interface, 1));
    static const lingmo_window_placement_v1_listener listener = {
            [](void *data, lingmo_window_placement_v1 *, wl_surface *surface, int32_t x, int32_t y) {
                static_cast<PlacementEvents *>(data)->positions.push_back({surface, base::Point(x, y)});
            }};
    lingmo_window_placement_v1_add_listener(placement, &listener, events);
    return placement;
}

} // namespace

TEST_F(XdgShellProtocolTest, OnlyATrustedClientSeesTheWindowPlacementGlobal)
{
    placementAuthorized = false;
    run([&](Client &c) {
        EXPECT_FALSE(c.globals.contains("lingmo_window_placement_v1"));
        EXPECT_TRUE(c.globals.contains("lingmo_window_hints_v1"));
    });
}

TEST_F(XdgShellProtocolTest, AWindowItsClientPlacedMapsThereAndHearsWhereItIs)
{
    run([&](Client &c) {
        PlacementEvents events;
        auto *placement = bindPlacement(c, &events);
        XdgWindow *window = c.makeToplevel();
        xdg_toplevel_set_app_id(window->toplevel, "org.lingmo.test");
        lingmo_window_placement_v1_set_position(placement, window->surface, 40, 60);
        wl_surface_commit(window->surface);
        c.sync();
        c.ackAndAttach(window, base::Size(300, 200));
        c.sync();
        check([&] {
            ASSERT_EQ(1u, windowManager.windows().size());
            EXPECT_EQ(base::Rect(40, 60, 300, 200), windowManager.windows()[0]->bounds());
        });
        ASSERT_FALSE(events.positions.empty());
        EXPECT_EQ(window->surface, events.positions.back().first);
        EXPECT_EQ(base::Point(40, 60), events.last());

        // A mapped window moves at once.
        lingmo_window_placement_v1_set_position(placement, window->surface, 200, 100);
        c.sync();
        c.sync();
        check([&] {
            EXPECT_EQ(base::Rect(200, 100, 300, 200), windowManager.windows()[0]->bounds());
        });
        EXPECT_EQ(base::Point(200, 100), events.last());

        // Whoever moves it, the client hears.
        check([&] {
            windowManager.windows()[0]->setBounds({120, 80, 300, 200});
        });
        c.sync();
        EXPECT_EQ(base::Point(120, 80), events.last());
        lingmo_window_placement_v1_destroy(placement);
    });
}

TEST_F(XdgShellProtocolTest, AWindowCannotBePlacedOutOfReach)
{
    run([&](Client &c) {
        PlacementEvents events;
        auto *placement = bindPlacement(c, &events);
        XdgWindow *window = c.makeToplevel();
        xdg_toplevel_set_app_id(window->toplevel, "org.lingmo.test");
        lingmo_window_placement_v1_set_position(placement, window->surface, -5000, -5000);
        wl_surface_commit(window->surface);
        c.sync();
        c.ackAndAttach(window, base::Size(300, 200));
        c.sync();
        check([&] {
            ASSERT_EQ(1u, windowManager.windows().size());
            const base::Rect bounds = windowManager.windows()[0]->bounds();
            EXPECT_GE(bounds.y(), kOutput.y());
            EXPECT_GE(bounds.right(), kOutput.x() + wm::kMinimumOnScreenArea);
        });
        lingmo_window_placement_v1_set_position(placement, window->surface, 5000, 5000);
        c.sync();
        c.sync();
        check([&] {
            const base::Rect bounds = windowManager.windows()[0]->bounds();
            EXPECT_LE(bounds.x(), kOutput.right() - wm::kMinimumOnScreenArea);
            EXPECT_LE(bounds.y(), kOutput.bottom() - wm::kMinimumOnScreenArea);
            EXPECT_EQ(bounds.topLeft(), events.last());
        });
        lingmo_window_placement_v1_destroy(placement);
    });
}

TEST_F(XdgShellProtocolTest, AMaximizedWindowStaysWhereItIsWhenPlaced)
{
    run([&](Client &c) {
        PlacementEvents events;
        auto *placement = bindPlacement(c, &events);
        XdgWindow *window = c.makeToplevel();
        xdg_toplevel_set_app_id(window->toplevel, "org.lingmo.test");
        wl_surface_commit(window->surface);
        c.sync();
        c.ackAndAttach(window, base::Size(300, 200));
        xdg_toplevel_set_maximized(window->toplevel);
        c.sync();
        base::Rect maximized;
        check([&] {
            maximized = windowManager.windows()[0]->bounds();
        });
        lingmo_window_placement_v1_set_position(placement, window->surface, 100, 100);
        c.sync();
        check([&] {
            EXPECT_EQ(maximized, windowManager.windows()[0]->bounds());
        });
        lingmo_window_placement_v1_destroy(placement);
    });
}

TEST_F(XdgShellProtocolTest, WindowHintsTellEachOutputsWorkArea)
{
    run([&](Client &c) {
        struct WorkArea {
            std::string output;
            base::Rect area;
            uint32_t primary;
        };
        std::vector<WorkArea> areas;
        auto *hints = static_cast<lingmo_window_hints_v1 *>(
                wl_registry_bind(c.registry, c.globals.at("lingmo_window_hints_v1").first,
                                 &lingmo_window_hints_v1_interface, 1));
        static const lingmo_window_hints_v1_listener listener = {
                [](void *, lingmo_window_hints_v1 *, wl_surface *, uint32_t) {},
                [](void *data, lingmo_window_hints_v1 *, const char *output, int32_t x, int32_t y,
                   int32_t width, int32_t height, uint32_t primary) {
                    static_cast<std::vector<WorkArea> *>(data)->push_back(
                            {output, base::Rect(x, y, width, height), primary});
                }};
        lingmo_window_hints_v1_add_listener(hints, &listener, &areas);
        c.sync();
        ASSERT_EQ(1u, areas.size());
        EXPECT_FALSE(areas[0].output.empty());
        EXPECT_EQ(kOutput, areas[0].area);
        EXPECT_EQ(1u, areas[0].primary);

        // A reservation shrinks it, and only the change is told.
        check([&] {
            windowManager.setExclusiveZone(this, wm::ScreenEdge::Top, 28);
        });
        c.sync();
        ASSERT_EQ(2u, areas.size());
        EXPECT_EQ(base::Rect(0, 28, 800, 572), areas[1].area);
        check([&] {
            windowManager.clearExclusiveZone(this);
        });
        c.sync();
        lingmo_window_hints_v1_destroy(hints);
    });
}

TEST_F(XdgShellProtocolTest, AClientsOpacityCombinesWithTheWindowManagers)
{
    run([&](Client &c) {
        auto *hints = static_cast<lingmo_window_hints_v1 *>(
                wl_registry_bind(c.registry, c.globals.at("lingmo_window_hints_v1").first,
                                 &lingmo_window_hints_v1_interface, 1));
        XdgWindow *window = c.makeToplevel();
        lingmo_window_hints_v1_set_opacity(hints, window->surface, wl_fixed_from_double(0.5));
        wl_surface_commit(window->surface);
        c.sync();
        c.ackAndAttach(window, base::Size(300, 200));
        const auto drawn = [&] {
            auto *animation = dynamic_cast<scene::WindowAnimation *>(windowManager.windows()[0]->node());
            return animation ? animation->opacity() : -1.0f;
        };
        check([&] {
            ASSERT_EQ(1u, windowManager.windows().size());
            EXPECT_FLOAT_EQ(0.5f, drawn());
            windowManager.windows()[0]->setOpacity(0.5f);
            EXPECT_FLOAT_EQ(0.25f, drawn());
            windowManager.windows()[0]->setOpacity(1);
            EXPECT_FLOAT_EQ(0.5f, drawn());
        });
        lingmo_window_hints_v1_set_opacity(hints, window->surface, wl_fixed_from_double(1));
        c.sync();
        check([&] {
            EXPECT_FLOAT_EQ(1.0f, drawn());
        });
        lingmo_window_hints_v1_destroy(hints);
    });
}
