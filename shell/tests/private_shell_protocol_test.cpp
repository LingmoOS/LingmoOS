// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// End-to-end tests for the private Lingmo shell protocol, which is down to the
// desktop surface, driven through real libwayland-client connections: one
// trusted connection created the way the session creates it, and one ordinary
// application connection.

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <linux/input-event-codes.h>
#include <map>
#include <tuple>
#include <memory>
#include <mutex>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include <lingmo-shell-v1-client-protocol.h>
#include <wayland-client.h>
#include <wayland-server-core.h>
#include <xdg-shell-client-protocol.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "display/display_manager.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "wayland/privileged_client.h"
#include "wayland/protocols/lingmo_shell.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/server.h"
#include "wm/activation.h"
#include "wm/stacking.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

using namespace Lingmo;

namespace {

const base::Rect kOutput(0, 0, 800, 600);

struct Client;

struct XdgWindow {
    wl_surface *surface = nullptr;
    xdg_surface *xdgSurface = nullptr;
    xdg_toplevel *toplevel = nullptr;
    lingmo_shell_surface_v1 *shellSurface = nullptr;
    wl_buffer *buffer = nullptr;
    std::vector<uint32_t> surfaceConfigures;
    std::vector<base::Size> bounds;
    int closes = 0;

    uint32_t lastSerial() const { return surfaceConfigures.back(); }
};

const xdg_surface_listener surfaceListener = {
        [](void *data, xdg_surface *, uint32_t serial) {
            static_cast<XdgWindow *>(data)->surfaceConfigures.push_back(serial);
        },
};

const xdg_toplevel_listener toplevelListener = {
        [](void *, xdg_toplevel *, int32_t, int32_t, wl_array *) {},
        [](void *data, xdg_toplevel *) {
            ++static_cast<XdgWindow *>(data)->closes;
        },
        [](void *data, xdg_toplevel *, int32_t width, int32_t height) {
            static_cast<XdgWindow *>(data)->bounds.emplace_back(width, height);
        },
        [](void *, xdg_toplevel *, wl_array *) {},
};

struct Client {
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_compositor *compositor = nullptr;
    wl_shm *shm = nullptr;
    xdg_wm_base *wmBase = nullptr;
    lingmo_shell_v1 *shell = nullptr;
    std::map<std::string, std::pair<uint32_t, uint32_t>> globals;

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
        roundtrip();

        compositor = static_cast<wl_compositor *>(wl_registry_bind(
                registry, globals["wl_compositor"].first, &wl_compositor_interface, 4));
        shm = static_cast<wl_shm *>(
                wl_registry_bind(registry, globals["wl_shm"].first, &wl_shm_interface, 1));
        wmBase = static_cast<xdg_wm_base *>(wl_registry_bind(registry, globals["xdg_wm_base"].first,
                                                             &xdg_wm_base_interface,
                                                             globals["xdg_wm_base"].second));
        static const xdg_wm_base_listener wmBaseListener = {
                [](void *, xdg_wm_base *base, uint32_t serial) {
                    xdg_wm_base_pong(base, serial);
                }};
        xdg_wm_base_add_listener(wmBase, &wmBaseListener, this);
        roundtrip();
    }

    ~Client()
    {
        for (auto &window : windows)
            destroyWindowObjects(window.get());
        windows.clear();
        if (shell)
            lingmo_shell_v1_destroy(shell);
        xdg_wm_base_destroy(wmBase);
        wl_shm_destroy(shm);
        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }

    bool bindShell()
    {
        if (!globals.contains("lingmo_shell_v1"))
            return false;
        shell = static_cast<lingmo_shell_v1 *>(wl_registry_bind(
                registry, globals["lingmo_shell_v1"].first, &lingmo_shell_v1_interface,
                globals["lingmo_shell_v1"].second));
        sync();
        return true;
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

    XdgWindow *makeToplevel(const char *title, const char *appId)
    {
        windows.push_back(std::make_unique<XdgWindow>());
        XdgWindow *window = windows.back().get();
        window->surface = wl_compositor_create_surface(compositor);
        window->xdgSurface = xdg_wm_base_get_xdg_surface(wmBase, window->surface);
        xdg_surface_add_listener(window->xdgSurface, &surfaceListener, window);
        window->toplevel = xdg_surface_get_toplevel(window->xdgSurface);
        xdg_toplevel_add_listener(window->toplevel, &toplevelListener, window);
        if (title)
            xdg_toplevel_set_title(window->toplevel, title);
        if (appId)
            xdg_toplevel_set_app_id(window->toplevel, appId);
        return window;
    }

    void map(XdgWindow *window, const base::Size &size)
    {
        wl_surface_commit(window->surface);
        sync();
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
        if (window->shellSurface) {
            lingmo_shell_surface_v1_destroy(window->shellSurface);
            window->shellSurface = nullptr;
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

    bool waitFor(const std::function<bool()> &predicate)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!predicate()) {
            if (std::chrono::steady_clock::now() >= deadline || roundtrip() < 0)
                return false;
        }
        return true;
    }

    int roundtrip()
    {
        bool done = false;
        auto *callback = wl_display_sync(display);
        if (!callback)
            return -1;
        static const wl_callback_listener listener = {[](void *data, wl_callback *, uint32_t) {
            *static_cast<bool *>(data) = true;
        }};
        wl_callback_add_listener(callback, &listener, &done);
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!done && !wl_display_get_error(display)) {
            if (wl_display_dispatch_pending(display) < 0 || done)
                break;
            if (wl_display_prepare_read(display) != 0)
                continue;
            wl_display_flush(display);
            const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                                           deadline - std::chrono::steady_clock::now())
                                           .count();
            pollfd fd {wl_display_get_fd(display), POLLIN, 0};
            if (remaining <= 0 || poll(&fd, 1, static_cast<int>(remaining)) <= 0) {
                wl_display_cancel_read(display);
                break;
            }
            if (wl_display_read_events(display) < 0)
                break;
        }
        wl_callback_destroy(callback);
        return done ? 0 : -1;
    }

    void sync() { roundtrip(); }

    uint32_t syncExpectingError()
    {
        if (roundtrip() >= 0)
            return 0;
        if (wl_display_get_error(display) != EPROTO)
            return 0;
        const wl_interface *raisedOn = nullptr;
        uint32_t id = 0;
        return wl_display_get_protocol_error(display, &raisedOn, &id);
    }
};

class PrivateShellProtocolTest : public testing::Test
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
    std::deque<std::function<void()>> pendingChecks;

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
        struct Work {
            std::mutex mutex;
            std::promise<void> done;
            bool cancelled = false;
        };

        auto state = std::make_shared<Work>();
        auto future = state->done.get_future();
        {
            std::lock_guard lock(mutex);
            pendingChecks.emplace_back([state, body = std::move(body)] {
                std::lock_guard lock(state->mutex);
                if (!state->cancelled)
                    body();
                state->done.set_value();
            });
        }
        if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
            std::lock_guard lock(state->mutex);
            state->cancelled = true;
            ADD_FAILURE() << "server check did not execute before its deadline";
        }
    }

    bool awaitRole(wm::WindowRole role)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        bool mapped = false;
        while (!mapped && std::chrono::steady_clock::now() < deadline)
            check([&] {
                mapped = windowWithRole(role) != nullptr;
            });
        return mapped;
    }

    wm::Window *windowWithRole(wm::WindowRole role)
    {
        for (wm::Window *window : windowManager.windows()) {
            if (window->role() == role)
                return window;
        }
        return nullptr;
    }

    // `shellBody` runs on a trusted connection created the way the session
    // creates one; `appBody` runs on an ordinary connection to the public
    // socket. Either may be empty.
    void run(std::function<void(Client &)> shellBody, std::function<void(Client &)> appBody = {})
    {
        auto privileged = std::make_unique<wayland::PrivilegedClients>(server.display());
        auto waylandSeat = std::make_unique<wayland::WaylandSeat>(&seat, server.serialTracker());
        auto xdgShell = std::make_unique<wayland::WaylandXdgShell>(&compositor, &windowManager,
                                                                   &seat, server.serialTracker());
        auto lingmoShell =
                std::make_unique<wayland::WaylandLingmoShell>(&windowManager, privileged.get());

        ASSERT_TRUE(wayland::createCompositorGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSubcompositorGlobal(server.display()));
        ASSERT_TRUE(wayland::createShmGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSeatGlobal(server.display(), waylandSeat.get()));
        ASSERT_TRUE(wayland::createXdgShellGlobal(server.display(), xdgShell.get()));
        ASSERT_TRUE(wayland::createLingmoShellGlobal(server.display(), lingmoShell.get()));
        int shellFds[2];
        ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, shellFds));
        ASSERT_TRUE(privileged->adoptShellConnection(shellFds[0]));

        int appFds[2] = {-1, -1};
        if (appBody) {
            ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, appFds));
            ASSERT_TRUE(wl_client_create(server.display(), appFds[0]));
        }

        std::atomic<int> running = appBody ? 2 : 1;
        std::mutex lifetimeMutex;
        std::condition_variable lifetimeChanged;
        int unfinishedBodies = running;
        auto retainUntilAssertionsFinish = [&] {
            std::unique_lock lock(lifetimeMutex);
            --unfinishedBodies;
            lifetimeChanged.notify_all();
            EXPECT_TRUE(lifetimeChanged.wait_for(lock, std::chrono::seconds(8), [&] {
                return unfinishedBodies == 0;
            })) << "peer client body did not finish";
        };
        std::thread shellThread([&] {
            {
                Client c(shellFds[1]);
                if (shellBody)
                    shellBody(c);
                retainUntilAssertionsFinish();
            }
            --running;
        });
        std::thread appThread;
        if (appBody) {
            appThread = std::thread([&] {
                {
                    Client c(appFds[1]);
                    appBody(c);
                    retainUntilAssertionsFinish();
                }
                --running;
            });
        }

        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (running > 0) {
            server.dispatch(std::chrono::milliseconds(1));
            server.flush();
            std::function<void()> work;
            {
                std::lock_guard lock(mutex);
                if (!pendingChecks.empty()) {
                    work = std::move(pendingChecks.front());
                    pendingChecks.pop_front();
                }
            }
            if (work)
                work();
            if (std::chrono::steady_clock::now() > deadline) {
                ADD_FAILURE() << "client timed out";
                break;
            }
        }
        shellThread.join();
        if (appThread.joinable())
            appThread.join();
        wl_display_destroy_clients(server.display());
        EXPECT_TRUE(windowManager.windows().empty());
        EXPECT_TRUE(compositor.surfaces().empty());
        EXPECT_EQ(privileged->shellClient(), nullptr);
    }
};

////////////////////////////////////////////////////////////////////////////////
// Privilege

TEST_F(PrivateShellProtocolTest, TheTrustedClientSeesThePrivateGlobal)
{
    run([&](Client &c) {
        EXPECT_TRUE(c.globals.contains("lingmo_shell_v1"));
        // The rest of the private shell protocols are gone.
        EXPECT_FALSE(c.globals.contains("lingmo_window_management_v1"));
        // Every public global is still there for it.
        EXPECT_TRUE(c.globals.contains("wl_compositor"));
        EXPECT_TRUE(c.globals.contains("xdg_wm_base"));
        EXPECT_TRUE(c.globals.contains("wl_seat"));
    });
}

TEST_F(PrivateShellProtocolTest, AnOrdinaryClientNeverSeesThePrivateGlobal)
{
    run({}, [&](Client &c) {
        EXPECT_FALSE(c.globals.contains("lingmo_shell_v1"));
        EXPECT_TRUE(c.globals.contains("wl_compositor"));
        EXPECT_TRUE(c.globals.contains("xdg_wm_base"));
    });
}

TEST_F(PrivateShellProtocolTest, AnOrdinaryClientCannotBindThePrivateGlobalByName)
{
    // The name is guessed the way a hostile client would have to guess it: the
    // trusted client learns the real one, and the application tries to bind it.
    std::promise<std::pair<uint32_t, uint32_t>> namePromise;
    auto nameFuture = namePromise.get_future();

    run(
            [&](Client &c) {
                namePromise.set_value(c.globals["lingmo_shell_v1"]);
            },
            [&](Client &c) {
                ASSERT_EQ(nameFuture.wait_for(std::chrono::seconds(5)), std::future_status::ready);
                const auto [name, version] = nameFuture.get();
                auto *shell = static_cast<lingmo_shell_v1 *>(
                        wl_registry_bind(c.registry, name, &lingmo_shell_v1_interface, version));
                // libwayland raises wl_display.error rather than a per-interface
                // code, so any protocol error at all is the pass condition.
                EXPECT_NE(0, c.roundtrip());
                EXPECT_EQ(EPROTO, wl_display_get_error(c.display));
                // wl_proxy_destroy rather than the generated destructor: no request
                // can be marshalled on a display that has already errored out.
                wl_proxy_destroy(reinterpret_cast<wl_proxy *>(shell));
            });
}

////////////////////////////////////////////////////////////////////////////////
// The desktop

namespace {

XdgWindow *makeDesktop(Client &c)
{
    XdgWindow *desktop = c.makeToplevel("Desktop", "lingmo-filemanager");
    desktop->shellSurface = lingmo_shell_v1_get_shell_surface(c.shell, desktop->surface);
    lingmo_shell_surface_v1_set_role(desktop->shellSurface, LINGMO_SHELL_SURFACE_V1_ROLE_DESKTOP);
    return desktop;
}

} // namespace

TEST_F(PrivateShellProtocolTest, TheDesktopIsPlacedAtItsOutputBelowApplications)
{
    run(
            [&](Client &c) {
                ASSERT_TRUE(c.bindShell());
                XdgWindow *desktop = makeDesktop(c);
                c.map(desktop, kOutput.size());
                check([&] {
                    wm::Window *window = windowWithRole(wm::WindowRole::Desktop);
                    ASSERT_NE(nullptr, window);
                    EXPECT_EQ(kOutput, window->bounds());
                    EXPECT_FALSE(window->isDecorated());
                    EXPECT_EQ(wm::StackingLayer::Desktop, wm::layerFor(window));
                });
            },
            [&](Client &c) {
                XdgWindow *window = c.makeToplevel("App", "app");
                c.map(window, base::Size(300, 200));
                ASSERT_TRUE(awaitRole(wm::WindowRole::Desktop));
                check([&] {
                    wm::Window *desktop = windowWithRole(wm::WindowRole::Desktop);
                    ASSERT_NE(nullptr, desktop);
                    // Raising the desktop never puts it over the application.
                    windowManager.raise(desktop);
                    EXPECT_EQ(desktop, windowManager.windows().front());
                });
            });
}

TEST_F(PrivateShellProtocolTest, AnUnknownRoleIsAnError)
{
    run([&](Client &c) {
        ASSERT_TRUE(c.bindShell());
        XdgWindow *window = c.makeToplevel("Panel", "lingmo-filemanager");
        window->shellSurface = lingmo_shell_v1_get_shell_surface(c.shell, window->surface);
        // The panel role of the separate shell, which is gone.
        lingmo_shell_surface_v1_set_role(window->shellSurface, 1);
        EXPECT_EQ(uint32_t(LINGMO_SHELL_SURFACE_V1_ERROR_INVALID_ROLE), c.syncExpectingError());
        // Nothing more can be sent on the broken connection.
        for (auto &w : c.windows) {
            for (wl_proxy *proxy : {reinterpret_cast<wl_proxy *>(w->shellSurface),
                                    reinterpret_cast<wl_proxy *>(w->toplevel),
                                    reinterpret_cast<wl_proxy *>(w->xdgSurface),
                                    reinterpret_cast<wl_proxy *>(w->surface)}) {
                if (proxy)
                    wl_proxy_destroy(proxy);
            }
            w->shellSurface = nullptr;
            w->toplevel = nullptr;
            w->xdgSurface = nullptr;
            w->surface = nullptr;
        }
    });
}

TEST_F(PrivateShellProtocolTest, AnUnknownOutputIsAnError)
{
    run([&](Client &c) {
        ASSERT_TRUE(c.bindShell());
        XdgWindow *desktop = makeDesktop(c);
        lingmo_shell_surface_v1_set_output(desktop->shellSurface, "nowhere");
        EXPECT_EQ(uint32_t(LINGMO_SHELL_SURFACE_V1_ERROR_INVALID_OUTPUT), c.syncExpectingError());
        for (wl_proxy *proxy : {reinterpret_cast<wl_proxy *>(desktop->shellSurface),
                                reinterpret_cast<wl_proxy *>(desktop->toplevel),
                                reinterpret_cast<wl_proxy *>(desktop->xdgSurface),
                                reinterpret_cast<wl_proxy *>(desktop->surface)})
            wl_proxy_destroy(proxy);
        desktop->shellSurface = nullptr;
        desktop->toplevel = nullptr;
        desktop->xdgSurface = nullptr;
        desktop->surface = nullptr;
    });
}

TEST_F(PrivateShellProtocolTest, TheDesktopGetsNoConfigureBounds)
{
    run([&](Client &c) {
        ASSERT_TRUE(c.bindShell());
        // A client clamping a size it picks itself to the bounds would shrink
        // the desktop off the strip the dock reserves.
        XdgWindow *desktop = makeDesktop(c);
        c.map(desktop, kOutput.size());
        XdgWindow *app = c.makeToplevel("App", "app");
        c.map(app, base::Size(300, 200));

        ASSERT_FALSE(desktop->bounds.empty());
        for (const base::Size &bounds : desktop->bounds)
            EXPECT_EQ(base::Size(0, 0), bounds);
        ASSERT_FALSE(app->bounds.empty());
        EXPECT_FALSE(app->bounds.front().isEmpty());
    });
}

TEST_F(PrivateShellProtocolTest, DestroyingTheShellSurfaceLeavesTheMappedDesktopInItsLayer)
{
    run([&](Client &c) {
        ASSERT_TRUE(c.bindShell());
        XdgWindow *desktop = makeDesktop(c);
        c.map(desktop, kOutput.size());
        check([&] { ASSERT_NE(nullptr, windowWithRole(wm::WindowRole::Desktop)); });

        // Only the extension object goes; the wl_surface and its xdg_toplevel
        // stay mapped.
        lingmo_shell_surface_v1_destroy(desktop->shellSurface);
        desktop->shellSurface = nullptr;
        c.sync();

        check([&] {
            wm::Window *window = windowWithRole(wm::WindowRole::Desktop);
            ASSERT_NE(nullptr, window) << "the mapped desktop lost its role";
            EXPECT_EQ(wm::StackingLayer::Desktop, wm::layerFor(window));
        });
    });
}

TEST_F(PrivateShellProtocolTest, TheDesktopTakesTheKeyboardThroughActivate)
{
    run([&](Client &c) {
        ASSERT_TRUE(c.bindShell());
        XdgWindow *desktop = makeDesktop(c);
        lingmo_shell_surface_v1_set_focus_policy(desktop->shellSurface,
                                                   LINGMO_SHELL_SURFACE_V1_FOCUS_POLICY_NORMAL);
        c.map(desktop, kOutput.size());

        check([&] {
            wm::Window *window = windowWithRole(wm::WindowRole::Desktop);
            ASSERT_NE(nullptr, window);
            windowManager.activation()->activate(nullptr);
            EXPECT_FALSE(window->isActive());
        });

        lingmo_shell_surface_v1_activate(desktop->shellSurface);
        c.sync();
        check([&] {
            wm::Window *window = windowWithRole(wm::WindowRole::Desktop);
            ASSERT_NE(nullptr, window);
            EXPECT_TRUE(window->isActive());
            EXPECT_EQ(window->rootSurface(), seat.focusedSurface());
        });
    });
}

TEST_F(PrivateShellProtocolTest, ADesktopHearsItsWorkArea)
{
    run([&](Client &c) {
        ASSERT_TRUE(c.bindShell());
        static const lingmo_shell_surface_v1_listener listener = {
            [](void *data, lingmo_shell_surface_v1 *, int32_t x, int32_t y, int32_t width,
               int32_t height) {
                static_cast<std::vector<base::Rect> *>(data)->push_back(base::Rect(x, y, width, height));
            },
        };
        std::vector<base::Rect> areas;
        XdgWindow *desktop = c.makeToplevel("Desktop", "lingmo-filemanager");
        desktop->shellSurface = lingmo_shell_v1_get_shell_surface(c.shell, desktop->surface);
        lingmo_shell_surface_v1_add_listener(desktop->shellSurface, &listener, &areas);
        lingmo_shell_surface_v1_set_role(desktop->shellSurface, LINGMO_SHELL_SURFACE_V1_ROLE_DESKTOP);
        c.sync();
        // As soon as it is a desktop, before anything is mapped.
        ASSERT_EQ(1u, areas.size());
        EXPECT_EQ(base::Rect(0, 0, 800, 600), areas.back());
        c.map(desktop, kOutput.size());

        // The compositor's own reservations: the status bar's and the dock's.
        int bar = 0;
        int dock = 0;
        check([&] { windowManager.setExclusiveZone(&bar, wm::ScreenEdge::Top, 32); });
        c.sync();
        ASSERT_EQ(2u, areas.size());
        EXPECT_EQ(base::Rect(0, 32, 800, 568), areas.back());
        check([&] { windowManager.setExclusiveZone(&dock, wm::ScreenEdge::Bottom, 92); });
        c.sync();
        ASSERT_EQ(3u, areas.size());
        EXPECT_EQ(base::Rect(0, 32, 800, 476), areas.back());

        // Unchanged reservations send nothing.
        check([&] { windowManager.setExclusiveZone(&bar, wm::ScreenEdge::Top, 32); });
        c.sync();
        EXPECT_EQ(3u, areas.size());
        check([&] {
            windowManager.clearExclusiveZone(&dock);
            windowManager.clearExclusiveZone(&bar);
        });
        c.sync();
        EXPECT_EQ(base::Rect(0, 0, 800, 600), areas.back());
    });
}

// The session's case: named outputs from the display manager, the desktop of
// the file manager on one of them, and the compositor's status bar and dock
// reserving their strips of it by name, before and after the desktop exists.
TEST_F(PrivateShellProtocolTest, ADesktopOnANamedOutputHearsTheShellsReservations)
{
    display::DisplayManager displays;
    display::DisplaySnapshot physical;
    physical.sessionId = display::DisplayId("a");
    physical.persistentKey = "edid-a";
    physical.portKey = "port-a";
    physical.connectorName = "a";
    physical.devicePath = "/dev/dri/card0";
    physical.physicalSizeMm = {600, 340};
    physical.modes = {{kOutput.size(), 60000, 148500, kOutput.width() + 100, kOutput.height() + 50, 0, "mode", true}};
    physical.currentMode = physical.modes.front();
    ASSERT_TRUE(displays.installHardware({"instance", "seat0", 1, {physical}}));
    const display::LogicalDisplay logical {display::DisplayId("a"),
                                           {display::DisplayId("a")},
                                           "Lingmo-a",
                                           kOutput,
                                           kOutput,
                                           *compositor::OutputScale::fromNumerator(120),
                                           display::DisplayRotation::Normal,
                                           true};
    ASSERT_TRUE(displays.installApplied({"instance", 1, 1, {}, {logical}, {}}));
    windowManager.setDisplayManager(&displays);

    // The status bar is up before the file manager starts.
    int bar = 0;
    int dock = 0;
    windowManager.setExclusiveZone(&bar, wm::ScreenEdge::Top, 32, "Lingmo-a");

    run([&](Client &c) {
        ASSERT_TRUE(c.bindShell());
        static const lingmo_shell_surface_v1_listener listener = {
            [](void *data, lingmo_shell_surface_v1 *, int32_t x, int32_t y, int32_t width,
               int32_t height) {
                static_cast<std::vector<base::Rect> *>(data)->push_back(base::Rect(x, y, width, height));
            },
        };
        std::vector<base::Rect> areas;
        XdgWindow *desktop = c.makeToplevel("Desktop", "lingmo-filemanager");
        desktop->shellSurface = lingmo_shell_v1_get_shell_surface(c.shell, desktop->surface);
        lingmo_shell_surface_v1_add_listener(desktop->shellSurface, &listener, &areas);
        lingmo_shell_surface_v1_set_role(desktop->shellSurface, LINGMO_SHELL_SURFACE_V1_ROLE_DESKTOP);
        lingmo_shell_surface_v1_set_output(desktop->shellSurface, "Lingmo-a");
        c.sync();
        // The bar's strip is already left out of the first one.
        ASSERT_FALSE(areas.empty());
        EXPECT_EQ(base::Rect(0, 32, 800, 568), areas.front());
        EXPECT_EQ(1u, areas.size());
        c.map(desktop, kOutput.size());

        // The dock comes up after the desktop.
        check([&] { windowManager.setExclusiveZone(&dock, wm::ScreenEdge::Bottom, 92, "Lingmo-a"); });
        c.sync();
        EXPECT_EQ(base::Rect(0, 32, 800, 476), areas.back());
        check([&] { windowManager.clearExclusiveZone(&dock); });
        c.sync();
        EXPECT_EQ(base::Rect(0, 32, 800, 568), areas.back());
    });
    windowManager.clearExclusiveZone(&bar);
    windowManager.setDisplayManager(nullptr);
}

} // namespace
