// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// xdg-decoration over a real libwayland connection.
//
// The window-manager side of the frame is covered by decoration_test.cpp; what
// is asserted here is the protocol: the negotiation, its errors, and what the
// compositor's window ends up looking like as a result.

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <map>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>
#include <wayland-server-core.h>
#include <xdg-decoration-client-protocol.h>
#include <xdg-shell-client-protocol.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/xdg_decoration.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/serial_tracker.h"
#include "wayland/server.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

using namespace Lingmo;

namespace {

const base::Rect kOutput(0, 0, 800, 600);

struct Client;

struct XdgWindow {
    wl_surface* surface = nullptr;
    xdg_surface* xdgSurface = nullptr;
    xdg_toplevel* toplevel = nullptr;
    zxdg_toplevel_decoration_v1* decoration = nullptr;
    wl_buffer* buffer = nullptr;

    std::vector<uint32_t> surfaceConfigures;
    std::vector<base::Size> toplevelConfigures;
    std::vector<uint32_t> decorationConfigures;

    uint32_t lastSerial() const { return surfaceConfigures.back(); }
    uint32_t lastMode() const
    {
        return decorationConfigures.empty() ? 0 : decorationConfigures.back();
    }
};

const xdg_surface_listener surfaceListener = {
    [](void* data, xdg_surface*, uint32_t serial) {
        static_cast<XdgWindow*>(data)->surfaceConfigures.push_back(serial);
    },
};

const xdg_toplevel_listener toplevelListener = {
    [](void* data, xdg_toplevel*, int32_t width, int32_t height, wl_array*) {
        static_cast<XdgWindow*>(data)->toplevelConfigures.push_back(base::Size(width, height));
    },
    [](void*, xdg_toplevel*) {},
    [](void*, xdg_toplevel*, int32_t, int32_t) {},
    [](void*, xdg_toplevel*, wl_array*) {},
};

const zxdg_toplevel_decoration_v1_listener decorationListener = {
    [](void* data, zxdg_toplevel_decoration_v1*, uint32_t mode) {
        static_cast<XdgWindow*>(data)->decorationConfigures.push_back(mode);
    },
};

struct Client {
    wl_display* display = nullptr;
    wl_registry* registry = nullptr;
    wl_compositor* compositor = nullptr;
    wl_shm* shm = nullptr;
    xdg_wm_base* wmBase = nullptr;
    zxdg_decoration_manager_v1* decorationManager = nullptr;
    std::map<std::string, std::pair<uint32_t, uint32_t>> globals;

    std::vector<std::unique_ptr<XdgWindow>> windows;

    explicit Client(int fd)
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
        shm = static_cast<wl_shm*>(
            wl_registry_bind(registry, globals["wl_shm"].first, &wl_shm_interface, 1));
        wmBase = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry, globals["xdg_wm_base"].first, &xdg_wm_base_interface,
                             globals["xdg_wm_base"].second));
        if (globals.contains("zxdg_decoration_manager_v1")) {
            decorationManager = static_cast<zxdg_decoration_manager_v1*>(
                wl_registry_bind(registry, globals["zxdg_decoration_manager_v1"].first,
                                 &zxdg_decoration_manager_v1_interface, 1));
        }
        wl_display_roundtrip(display);
    }

    ~Client()
    {
        for (auto& window : windows)
            destroyWindowObjects(window.get());
        windows.clear();
        if (decorationManager)
            zxdg_decoration_manager_v1_destroy(decorationManager);
        xdg_wm_base_destroy(wmBase);
        wl_shm_destroy(shm);
        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }

    wl_buffer* makeBuffer(const base::Size& size)
    {
        const int stride = size.width() * 4;
        const size_t bytes = static_cast<size_t>(stride) * size.height();
        const int fd = testsupport::makeMemfd(bytes);
        wl_shm_pool* pool = wl_shm_create_pool(shm, fd, static_cast<int32_t>(bytes));
        wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, size.width(), size.height(), stride,
                                                      WL_SHM_FORMAT_XRGB8888);
        wl_shm_pool_destroy(pool);
        close(fd);
        return buffer;
    }

    XdgWindow* makeToplevel()
    {
        windows.push_back(std::make_unique<XdgWindow>());
        XdgWindow* window = windows.back().get();
        window->surface = wl_compositor_create_surface(compositor);
        window->xdgSurface = xdg_wm_base_get_xdg_surface(wmBase, window->surface);
        xdg_surface_add_listener(window->xdgSurface, &surfaceListener, window);
        window->toplevel = xdg_surface_get_toplevel(window->xdgSurface);
        xdg_toplevel_add_listener(window->toplevel, &toplevelListener, window);
        return window;
    }

    void addDecoration(XdgWindow* window)
    {
        window->decoration =
            zxdg_decoration_manager_v1_get_toplevel_decoration(decorationManager,
                                                               window->toplevel);
        zxdg_toplevel_decoration_v1_add_listener(window->decoration, &decorationListener, window);
    }

    // The whole handshake, with the decoration negotiated before the first
    // buffer as the protocol requires.
    XdgWindow* makeDecoratedToplevel(const base::Size& size, bool serverSide = true)
    {
        XdgWindow* window = makeToplevel();
        addDecoration(window);
        zxdg_toplevel_decoration_v1_set_mode(
            window->decoration, serverSide ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
                                           : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
        wl_surface_commit(window->surface);
        sync();
        ackAndAttach(window, size);
        return window;
    }

    void ackAndAttach(XdgWindow* window, const base::Size& size)
    {
        xdg_surface_ack_configure(window->xdgSurface, window->lastSerial());
        if (!window->buffer)
            window->buffer = makeBuffer(size);
        xdg_surface_set_window_geometry(window->xdgSurface, 0, 0, size.width(), size.height());
        wl_surface_attach(window->surface, window->buffer, 0, 0);
        wl_surface_damage(window->surface, 0, 0, size.width(), size.height());
        wl_surface_commit(window->surface);
        sync();
    }

    void destroyWindowObjects(XdgWindow* window)
    {
        if (window->decoration) {
            zxdg_toplevel_decoration_v1_destroy(window->decoration);
            window->decoration = nullptr;
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

    uint32_t syncExpectingError(const wl_interface** interface = nullptr)
    {
        if (wl_display_roundtrip(display) >= 0)
            return 0;
        if (wl_display_get_error(display) != EPROTO)
            return 0;
        const wl_interface* raisedOn = nullptr;
        uint32_t id = 0;
        const uint32_t code = wl_display_get_protocol_error(display, &raisedOn, &id);
        if (interface)
            *interface = raisedOn;
        return code;
    }
};

class XdgDecorationProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node* surfaceContainer =
        scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    input::Seat seat {&compositor};
    input::InputManager manager {&seat};
    wm::WindowManager windowManager {&compositor, &seat};
    decoration::Theme theme;
    decoration::DecorationProvider provider {&theme};
    wayland::Server server;

    std::mutex mutex;
    std::function<void()> pendingCheck;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setBounds(kOutput.size());
        manager.setEventSink(&windowManager);
        windowManager.setOutputBounds(kOutput);
        windowManager.setFrameProvider(&provider);
    }

    wm::Insets insets() const { return theme.metrics().insets(); }

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

    wm::Window* onlyWindow()
    {
        return windowManager.windows().empty() ? nullptr : windowManager.windows().front();
    }

    void run(std::function<void(Client&)> body)
    {
        auto waylandSeat = std::make_unique<wayland::WaylandSeat>(&seat, server.serialTracker());
        auto xdgShell = std::make_unique<wayland::WaylandXdgShell>(
            &compositor, &windowManager, &seat, server.serialTracker());
        ASSERT_TRUE(wayland::createCompositorGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createShmGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSeatGlobal(server.display(), waylandSeat.get()));
        ASSERT_TRUE(wayland::createXdgShellGlobal(server.display(), xdgShell.get()));
        ASSERT_TRUE(wayland::createXdgDecorationGlobal(server.display()));

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
        EXPECT_TRUE(windowManager.windows().empty());
    }
};

////////////////////////////////////////////////////////////////////////////////
// The registry and the negotiation

TEST_F(XdgDecorationProtocolTest, TheRegistryListsTheDecorationManager)
{
    run([&](Client& c) {
        ASSERT_TRUE(c.globals.contains("zxdg_decoration_manager_v1"));
        EXPECT_EQ(wayland::kZxdgDecorationManagerVersion,
                  c.globals["zxdg_decoration_manager_v1"].second);
    });
}

TEST_F(XdgDecorationProtocolTest, TheCompositorConfiguresServerSideByDefault)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeToplevel();
        c.addDecoration(window);
        c.sync();

        // The mode is configured before the client may attach anything.
        ASSERT_EQ(1u, window->decorationConfigures.size());
        EXPECT_EQ(uint32_t(ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE), window->lastMode());
        EXPECT_TRUE(window->surfaceConfigures.empty());
    });
}

TEST_F(XdgDecorationProtocolTest, AServerSideToplevelIsDecoratedAndKeepsItsOwnGeometry)
{
    run([&](Client& c) {
        c.makeDecoratedToplevel(base::Size(400, 300));
        check([&] {
            wm::Window* window = onlyWindow();
            ASSERT_NE(nullptr, window);
            EXPECT_NE(nullptr, window->frame());
            EXPECT_EQ(base::Size(400, 300), window->contentBounds().size());
            EXPECT_EQ(insets().outset(base::Size(400, 300)), window->bounds().size());
        });
    });
}

TEST_F(XdgDecorationProtocolTest, AClientSideToplevelIsNotDecorated)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeDecoratedToplevel(base::Size(400, 300), /*serverSide=*/false);
        EXPECT_EQ(uint32_t(ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE), window->lastMode());
        check([&] {
            wm::Window* managed = onlyWindow();
            ASSERT_NE(nullptr, managed);
            EXPECT_EQ(nullptr, managed->frame());
            EXPECT_EQ(base::Size(400, 300), managed->bounds().size());
        });
    });
}

TEST_F(XdgDecorationProtocolTest, ATopLevelWithNoDecorationObjectIsLeftAlone)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeToplevel();
        wl_surface_commit(window->surface);
        c.sync();
        c.ackAndAttach(window, base::Size(400, 300));
        check([&] {
            wm::Window* managed = onlyWindow();
            ASSERT_NE(nullptr, managed);
            EXPECT_EQ(nullptr, managed->frame());
            EXPECT_EQ(base::Size(400, 300), managed->bounds().size());
        });
    });
}

TEST_F(XdgDecorationProtocolTest, SwitchingModeConfiguresTheDecorationAndThenTheSurface)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeDecoratedToplevel(base::Size(400, 300));
        const size_t configures = window->surfaceConfigures.size();

        zxdg_toplevel_decoration_v1_set_mode(window->decoration,
                                             ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
        c.sync();

        EXPECT_EQ(uint32_t(ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE), window->lastMode());
        // Exactly one xdg_surface.configure follows the mode change.
        EXPECT_EQ(configures + 1, window->surfaceConfigures.size());
        // The client area did not move, so the size it is asked for is the same.
        EXPECT_EQ(base::Size(400, 300), window->toplevelConfigures.back());

        check([&] {
            EXPECT_EQ(nullptr, onlyWindow()->frame());
            EXPECT_EQ(base::Size(400, 300), onlyWindow()->bounds().size());
        });
    });
}

TEST_F(XdgDecorationProtocolTest, UnsettingTheModeFallsBackToTheCompositorsPreference)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeDecoratedToplevel(base::Size(400, 300), /*serverSide=*/false);
        ASSERT_EQ(uint32_t(ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE), window->lastMode());

        zxdg_toplevel_decoration_v1_unset_mode(window->decoration);
        c.sync();

        EXPECT_EQ(uint32_t(ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE), window->lastMode());
        check([&] {
            EXPECT_NE(nullptr, onlyWindow()->frame());
        });
    });
}

TEST_F(XdgDecorationProtocolTest, DestroyingTheDecorationObjectGivesTheFrameUp)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeDecoratedToplevel(base::Size(400, 300));
        check([&] {
            EXPECT_NE(nullptr, onlyWindow()->frame());
        });

        zxdg_toplevel_decoration_v1_destroy(window->decoration);
        window->decoration = nullptr;
        c.sync();
        // The protocol drops the frame at the next commit.
        check([&] {
            EXPECT_NE(nullptr, onlyWindow()->frame());
        });

        wl_surface_commit(window->surface);
        c.sync();
        check([&] {
            EXPECT_EQ(nullptr, onlyWindow()->frame());
            EXPECT_EQ(base::Size(400, 300), onlyWindow()->bounds().size());
        });
    });
}

TEST_F(XdgDecorationProtocolTest, AMaximizedServerSideToplevelIsAskedForTheWorkAreaLessTheFrame)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeDecoratedToplevel(base::Size(400, 300));
        xdg_toplevel_set_maximized(window->toplevel);
        c.sync();
        const wm::Insets maximizedInsets {0, insets().top, 0, 0};
        EXPECT_EQ(maximizedInsets.inset(kOutput.size()), window->toplevelConfigures.back());

        xdg_toplevel_set_fullscreen(window->toplevel, nullptr);
        c.sync();
        // Fullscreen has no frame at all.
        EXPECT_EQ(kOutput.size(), window->toplevelConfigures.back());
        check([&] {
            EXPECT_TRUE(onlyWindow()->frameInsets().isNull());
        });
    });
}

////////////////////////////////////////////////////////////////////////////////
// Protocol errors

TEST_F(XdgDecorationProtocolTest, ATopLevelMayOnlyHaveOneDecorationObject)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeToplevel();
        c.addDecoration(window);
        c.sync();

        zxdg_toplevel_decoration_v1* second =
            zxdg_decoration_manager_v1_get_toplevel_decoration(c.decorationManager,
                                                              window->toplevel);
        EXPECT_EQ(uint32_t(ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ALREADY_CONSTRUCTED),
                  c.syncExpectingError());
        zxdg_toplevel_decoration_v1_destroy(second);
    });
}

TEST_F(XdgDecorationProtocolTest, ADecorationCannotBeCreatedAfterTheFirstBuffer)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeToplevel();
        wl_surface_commit(window->surface);
        c.sync();
        c.ackAndAttach(window, base::Size(400, 300));

        c.addDecoration(window);
        EXPECT_EQ(uint32_t(ZXDG_TOPLEVEL_DECORATION_V1_ERROR_UNCONFIGURED_BUFFER),
                  c.syncExpectingError());
    });
}

TEST_F(XdgDecorationProtocolTest, AnInvalidModeIsAnError)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeToplevel();
        c.addDecoration(window);
        c.sync();

        zxdg_toplevel_decoration_v1_set_mode(window->decoration, 42);
        EXPECT_EQ(uint32_t(ZXDG_TOPLEVEL_DECORATION_V1_ERROR_INVALID_MODE),
                  c.syncExpectingError());
    });
}

TEST_F(XdgDecorationProtocolTest, DestroyingTheToplevelBeforeItsDecorationIsAnError)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeDecoratedToplevel(base::Size(400, 300));

        xdg_toplevel_destroy(window->toplevel);
        window->toplevel = nullptr;
        EXPECT_EQ(uint32_t(ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ORPHANED), c.syncExpectingError());
    });
}

////////////////////////////////////////////////////////////////////////////////
// Lifetime

TEST_F(XdgDecorationProtocolTest, ADecoratedClientCanBeTornDownInAnyOrder)
{
    run([&](Client& c) {
        XdgWindow* window = c.makeDecoratedToplevel(base::Size(400, 300));
        check([&] {
            EXPECT_NE(nullptr, onlyWindow()->frame());
        });
        // The client disconnects with the decoration still up; the run()
        // epilogue is what checks nothing was left behind.
        (void)window;
    });
}

TEST_F(XdgDecorationProtocolTest, ANewWindowMappedWhileAnotherIsTornDownIsFine)
{
    run([&](Client& c) {
        XdgWindow* first = c.makeDecoratedToplevel(base::Size(400, 300));
        c.destroyWindowObjects(first);
        XdgWindow* second = c.makeDecoratedToplevel(base::Size(200, 100));
        c.sync();

        check([&] {
            ASSERT_EQ(1u, windowManager.windows().size());
            EXPECT_EQ(insets().outset(base::Size(200, 100)), onlyWindow()->bounds().size());
        });
        (void)second;
    });
}

} // namespace
