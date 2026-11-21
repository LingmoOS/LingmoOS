// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on chromium/src/components/exo/extended_drag_source_unittest.cc
// (revision 7acd40ddefff49c9d36144999aafdbb674dbd07a), driven through
// xdg_toplevel_drag_v1 on a real libwayland-client connection.

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <linux/input-event-codes.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>
#include <wayland-server-core.h>
#include <xdg-shell-client-protocol.h>
#include <xdg-toplevel-drag-v1-client-protocol.h>

#include "base/geometry.h"
#include "compositor/compositor.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "scene/window_animation.h"
#include "shm_test_util.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_data_device_manager.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/protocols/xdg_toplevel_drag.h"
#include "wayland/server.h"
#include "wm/toplevel_drag.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 800, 600);

struct Toplevel {
    wl_surface *surface = nullptr;
    xdg_surface *xdgSurface = nullptr;
    xdg_toplevel *toplevel = nullptr;
    wl_buffer *buffer = nullptr;
    uint32_t configureSerial = 0;
    bool maximized = false;
    base::Size configuredSize;
};

struct Client {
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_compositor *compositor = nullptr;
    wl_subcompositor *subcompositor = nullptr;
    wl_shm *shm = nullptr;
    wl_seat *seat = nullptr;
    wl_pointer *pointer = nullptr;
    xdg_wm_base *wmBase = nullptr;
    wl_data_device_manager *dataDeviceManager = nullptr;
    wl_data_device *dataDevice = nullptr;
    xdg_toplevel_drag_manager_v1 *dragManager = nullptr;
    std::map<std::string, std::pair<uint32_t, uint32_t>> globals;
    std::vector<std::unique_ptr<Toplevel>> toplevels;

    uint32_t buttonSerial = 0;
    wl_surface *dragEnterSurface = nullptr;
    wl_data_offer *dragOffer = nullptr;
    std::vector<wl_data_offer *> offers;
    uint32_t dragEnterSerial = 0;
    int dropsPerformed = 0;
    int cancels = 0;

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

        compositor = static_cast<wl_compositor *>(
                bind("wl_compositor", &wl_compositor_interface, 4));
        subcompositor = static_cast<wl_subcompositor *>(
                bind("wl_subcompositor", &wl_subcompositor_interface, 1));
        shm = static_cast<wl_shm *>(bind("wl_shm", &wl_shm_interface, 1));
        wmBase = static_cast<xdg_wm_base *>(bind("xdg_wm_base", &xdg_wm_base_interface, 1));
        static const xdg_wm_base_listener wmBaseListener = {
                [](void *, xdg_wm_base *base, uint32_t serial) {
                    xdg_wm_base_pong(base, serial);
                }};
        xdg_wm_base_add_listener(wmBase, &wmBaseListener, this);
        dataDeviceManager = static_cast<wl_data_device_manager *>(
                bind("wl_data_device_manager", &wl_data_device_manager_interface, 3));
        dragManager = static_cast<xdg_toplevel_drag_manager_v1 *>(
                bind("xdg_toplevel_drag_manager_v1", &xdg_toplevel_drag_manager_v1_interface, 1));

        seat = static_cast<wl_seat *>(bind("wl_seat", &wl_seat_interface, 5));
        pointer = wl_seat_get_pointer(seat);
        static const wl_pointer_listener pointerListener = {
                [](void *, wl_pointer *, uint32_t, wl_surface *, wl_fixed_t, wl_fixed_t) {},
                [](void *, wl_pointer *, uint32_t, wl_surface *) {},
                [](void *, wl_pointer *, uint32_t, wl_fixed_t, wl_fixed_t) {},
                [](void *data, wl_pointer *, uint32_t serial, uint32_t, uint32_t, uint32_t state) {
                    if (state == WL_POINTER_BUTTON_STATE_PRESSED)
                        static_cast<Client *>(data)->buttonSerial = serial;
                },
                [](void *, wl_pointer *, uint32_t, uint32_t, wl_fixed_t) {},
                [](void *, wl_pointer *) {},
                [](void *, wl_pointer *, uint32_t) {},
                [](void *, wl_pointer *, uint32_t, uint32_t) {},
                [](void *, wl_pointer *, uint32_t, int32_t) {},
                nullptr,
                nullptr,
        };
        wl_pointer_add_listener(pointer, &pointerListener, this);

        dataDevice = wl_data_device_manager_get_data_device(dataDeviceManager, seat);
        static const wl_data_device_listener deviceListener = {
                [](void *data, wl_data_device *, wl_data_offer *offer) {
                    static_cast<Client *>(data)->offers.push_back(offer);
                },
                [](void *data, wl_data_device *, uint32_t serial, wl_surface *surface, wl_fixed_t,
                   wl_fixed_t, wl_data_offer *offer) {
                    auto *client = static_cast<Client *>(data);
                    client->dragEnterSurface = surface;
                    client->dragOffer = offer;
                    client->dragEnterSerial = serial;
                },
                [](void *data, wl_data_device *) {
                    static_cast<Client *>(data)->dragEnterSurface = nullptr;
                },
                [](void *, wl_data_device *, uint32_t, wl_fixed_t, wl_fixed_t) {},
                [](void *, wl_data_device *) {},
                [](void *, wl_data_device *, wl_data_offer *) {},
        };
        wl_data_device_add_listener(dataDevice, &deviceListener, this);
        wl_display_roundtrip(display);
    }

    ~Client()
    {
        for (auto &toplevel : toplevels)
            destroy(toplevel.get());
        for (wl_data_offer *offer : offers)
            wl_data_offer_destroy(offer);
        wl_data_device_release(dataDevice);
        wl_pointer_release(pointer);
        wl_seat_release(seat);
        xdg_toplevel_drag_manager_v1_destroy(dragManager);
        wl_data_device_manager_destroy(dataDeviceManager);
        xdg_wm_base_destroy(wmBase);
        wl_shm_destroy(shm);
        wl_subcompositor_destroy(subcompositor);
        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }

    void *bind(const char *interface, const wl_interface *type, uint32_t version)
    {
        return wl_registry_bind(registry, globals[interface].first, type, version);
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

    // A toplevel that has had its initial configure but no buffer yet.
    Toplevel *makeToplevel(const char *appId = nullptr)
    {
        toplevels.push_back(std::make_unique<Toplevel>());
        Toplevel *toplevel = toplevels.back().get();
        toplevel->surface = wl_compositor_create_surface(compositor);
        toplevel->xdgSurface = xdg_wm_base_get_xdg_surface(wmBase, toplevel->surface);
        static const xdg_surface_listener surfaceListener = {
                [](void *data, xdg_surface *, uint32_t serial) {
                    static_cast<Toplevel *>(data)->configureSerial = serial;
                },
        };
        xdg_surface_add_listener(toplevel->xdgSurface, &surfaceListener, toplevel);
        toplevel->toplevel = xdg_surface_get_toplevel(toplevel->xdgSurface);
        static const xdg_toplevel_listener toplevelListener = {
                [](void *data, xdg_toplevel *, int32_t width, int32_t height, wl_array *states) {
                    auto *toplevel = static_cast<Toplevel *>(data);
                    toplevel->configuredSize = base::Size(width, height);
                    toplevel->maximized = false;
                    const auto *state = static_cast<const uint32_t *>(states->data);
                    for (size_t i = 0; i < states->size / sizeof(uint32_t); ++i)
                        toplevel->maximized |= state[i] == XDG_TOPLEVEL_STATE_MAXIMIZED;
                },
                [](void *, xdg_toplevel *) {},
                nullptr,
                nullptr,
        };
        xdg_toplevel_add_listener(toplevel->toplevel, &toplevelListener, toplevel);
        if (appId)
            xdg_toplevel_set_app_id(toplevel->toplevel, appId);
        wl_surface_commit(toplevel->surface);
        sync();
        return toplevel;
    }

    void map(Toplevel *toplevel, const base::Size &size)
    {
        xdg_surface_ack_configure(toplevel->xdgSurface, toplevel->configureSerial);
        toplevel->buffer = makeBuffer(size);
        wl_surface_attach(toplevel->surface, toplevel->buffer, 0, 0);
        wl_surface_damage(toplevel->surface, 0, 0, size.width(), size.height());
        wl_surface_commit(toplevel->surface);
        sync();
    }

    void destroy(Toplevel *toplevel)
    {
        if (toplevel->toplevel)
            xdg_toplevel_destroy(toplevel->toplevel);
        if (toplevel->xdgSurface)
            xdg_surface_destroy(toplevel->xdgSurface);
        if (toplevel->surface)
            wl_surface_destroy(toplevel->surface);
        if (toplevel->buffer)
            wl_buffer_destroy(toplevel->buffer);
        *toplevel = {};
    }

    wl_data_source *makeSource()
    {
        wl_data_source *source = wl_data_device_manager_create_data_source(dataDeviceManager);
        static const wl_data_source_listener sourceListener = {
                [](void *, wl_data_source *, const char *) {},
                [](void *, wl_data_source *, const char *, int32_t fd) {
                    close(fd);
                },
                [](void *data, wl_data_source *) {
                    ++static_cast<Client *>(data)->cancels;
                },
                [](void *data, wl_data_source *) {
                    ++static_cast<Client *>(data)->dropsPerformed;
                },
                [](void *, wl_data_source *) {},
                [](void *, wl_data_source *, uint32_t) {},
        };
        wl_data_source_add_listener(source, &sourceListener, this);
        wl_data_source_offer(source, "application/x-lingmo-tab");
        wl_data_source_set_actions(source, WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE);
        return source;
    }

    void sync() { wl_display_roundtrip(display); }

    uint32_t syncExpectingError()
    {
        if (wl_display_roundtrip(display) >= 0 || wl_display_get_error(display) != EPROTO)
            return UINT32_MAX;
        const wl_interface *interface = nullptr;
        uint32_t id = 0;
        return wl_display_get_protocol_error(display, &interface, &id);
    }
};

class XdgToplevelDragProtocolTest : public testing::Test
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

    wm::Window *windowSized(const base::Size &size)
    {
        for (wm::Window *window : windowManager.windows()) {
            if (window->bounds().size() == size)
                return window;
        }
        return nullptr;
    }

    static float opacityOf(wm::Window *window)
    {
        return static_cast<scene::WindowAnimation *>(window->node())->opacity();
    }

    wm::Window *windowAt(size_t index)
    {
        const auto &windows = windowManager.windows();
        return index < windows.size() ? windows[index] : nullptr;
    }

    // Maps a 300x200 window at (100, 100) and presses the button over it at
    // (150, 150), which is where every drag here starts.
    Toplevel *mapSourceWindowAndPress(Client &c, const char *appId = nullptr)
    {
        Toplevel *source = c.makeToplevel(appId);
        c.map(source, base::Size(300, 200));
        check([&] {
            // Just mapped, so on top.
            windowManager.windows().back()->setBounds(base::Rect(100, 100, 300, 200));
            manager.onPointerMotionAbsolute(at(1), base::PointF(150, 150));
            manager.onPointerButton(at(2), BTN_LEFT, true);
        });
        c.sync();
        EXPECT_NE(0u, c.buttonSerial);
        return source;
    }

    void run(std::function<void(Client &)> body)
    {
        auto waylandSeat = std::make_unique<wayland::WaylandSeat>(&seat, server.serialTracker());
        auto xdgShell = std::make_unique<wayland::WaylandXdgShell>(&compositor, &windowManager,
                                                                   &seat, server.serialTracker());
        ASSERT_TRUE(wayland::createCompositorGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSubcompositorGlobal(server.display()));
        ASSERT_TRUE(wayland::createShmGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSeatGlobal(server.display(), waylandSeat.get()));
        ASSERT_TRUE(wayland::createDataDeviceManagerGlobal(server.display(), waylandSeat.get()));
        ASSERT_TRUE(wayland::createXdgShellGlobal(server.display(), xdgShell.get()));
        ASSERT_TRUE(wayland::createXdgToplevelDragGlobal(server.display(), &windowManager));

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

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
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
        EXPECT_FALSE(seat.drag()->active());
        EXPECT_FALSE(windowManager.eventHandler()->isDragging());
        EXPECT_TRUE(windowManager.windows().empty());
    }
};

} // namespace

TEST_F(XdgToplevelDragProtocolTest, ANewWindowMapsUnderThePointerAndStaysWhereItIsDropped)
{
    run([&](Client &c) {
        Toplevel *origin = mapSourceWindowAndPress(c);
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        wl_data_device_start_drag(c.dataDevice, source, origin->surface, nullptr, c.buttonSerial);
        c.sync();

        Toplevel *detached = c.makeToplevel();
        xdg_toplevel_drag_v1_attach(drag, detached->toplevel, 20, 10);
        c.map(detached, base::Size(200, 150));
        check([&] {
            wm::Window *window = windowSized(base::Size(200, 150));
            ASSERT_NE(nullptr, window);
            EXPECT_EQ(base::Rect(130, 140, 200, 150), window->bounds());
            // A translucent preview, and the window it came out of keeps the keyboard.
            EXPECT_TRUE(window->isDragPreview());
            EXPECT_FALSE(window->isActive());
            EXPECT_FLOAT_EQ(wm::ToplevelDrag::kPreviewOpacity, opacityOf(window));
            // A thumbnail: 320 wide at most, and never more than half size.
            EXPECT_DOUBLE_EQ(wm::ToplevelDrag::kPreviewMaxScale,
                             static_cast<scene::WindowAnimation *>(window->node())
                                     ->presentationScale());
            EXPECT_TRUE(windowSized(base::Size(300, 200))->isActive());
            EXPECT_TRUE(windowManager.eventHandler()->isDragging());
            EXPECT_EQ(window, windowManager.eventHandler()->draggedWindow());

            manager.onPointerMotionAbsolute(at(3), base::PointF(250, 200));
            EXPECT_EQ(base::Rect(230, 190, 200, 150), window->bounds());
        });
        c.sync();
        // The window under the pointer is the one being dragged; the drop
        // target is what lies beneath it.
        EXPECT_EQ(origin->surface, c.dragEnterSurface);

        check([&] {
            manager.onPointerButton(at(4), BTN_LEFT, false);
            EXPECT_FALSE(seat.drag()->active());
            EXPECT_FALSE(windowManager.eventHandler()->isDragging());
            wm::Window *window = windowSized(base::Size(200, 150));
            ASSERT_NE(nullptr, window);
            EXPECT_EQ(base::Point(230, 190), window->bounds().topLeft());
            // Let go, it is a window of its own.
            EXPECT_FALSE(window->isDragPreview());
            EXPECT_TRUE(window->isActive());
            EXPECT_FLOAT_EQ(1, opacityOf(window));
        });
        c.sync();
        // Nothing took the data, but it was a drop, not a cancel.
        EXPECT_EQ(1, c.dropsPerformed);
        EXPECT_EQ(1, c.cancels);

        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
        c.sync();
    });
}

// A new window of an application with a maximized window is told to maximize
// in its initial configure. Qt maps it at that size before the attach reaches
// the compositor, so it must neither be maximized after it is placed under the
// pointer nor have its client's resize undone by the move.
TEST_F(XdgToplevelDragProtocolTest, AnAttachedWindowTakesNoInheritedStateAndMayResize)
{
    run([&](Client &c) {
        Toplevel *origin = mapSourceWindowAndPress(c, "tabs");
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        wl_data_device_start_drag(c.dataDevice, source, origin->surface, nullptr, c.buttonSerial);
        c.sync();
        check([&] {
            ASSERT_TRUE(seat.drag()->active());
            windowAt(0)->maximize();
        });

        Toplevel *detached = c.makeToplevel("tabs");
        EXPECT_TRUE(detached->maximized);
        const uint32_t maximizedSerial = detached->configureSerial;
        xdg_toplevel_drag_v1_attach(drag, detached->toplevel, 20, 10);
        c.sync();
        // Normal, at a size of the client's choosing.
        EXPECT_FALSE(detached->maximized);
        EXPECT_EQ(base::Size(0, 0), detached->configuredSize);
        EXPECT_NE(maximizedSerial, detached->configureSerial);

        // Drawn for the configure the attach overtook.
        xdg_surface_ack_configure(detached->xdgSurface, maximizedSerial);
        wl_buffer *maximizedBuffer = c.makeBuffer(kOutput.size());
        wl_surface_attach(detached->surface, maximizedBuffer, 0, 0);
        wl_surface_damage(detached->surface, 0, 0, kOutput.width(), kOutput.height());
        wl_surface_commit(detached->surface);
        c.sync();
        // Nothing holds the client to the size it mapped at.
        EXPECT_FALSE(detached->maximized);
        EXPECT_EQ(base::Size(0, 0), detached->configuredSize);
        check([&] {
            wm::Window *window = windowAt(1);
            ASSERT_NE(nullptr, window);
            EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());
            EXPECT_EQ(base::Rect(base::Point(130, 140), kOutput.size()), window->bounds());
            EXPECT_TRUE(window->isDragPreview());
            EXPECT_DOUBLE_EQ(wm::ToplevelDrag::kPreviewWidth / kOutput.width(),
                             static_cast<scene::WindowAnimation *>(window->node())
                                     ->presentationScale());
        });

        c.map(detached, base::Size(200, 150));
        check([&] {
            wm::Window *window = windowAt(1);
            ASSERT_NE(nullptr, window);
            EXPECT_EQ(base::Rect(130, 140, 200, 150), window->bounds());
            EXPECT_DOUBLE_EQ(wm::ToplevelDrag::kPreviewMaxScale,
                             static_cast<scene::WindowAnimation *>(window->node())
                                     ->presentationScale());
            manager.onPointerMotionAbsolute(at(3), base::PointF(250, 200));
            EXPECT_EQ(base::Rect(230, 190, 200, 150), window->bounds());
            manager.onPointerButton(at(4), BTN_LEFT, false);
            EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());
            EXPECT_EQ(base::Rect(230, 190, 200, 150), window->bounds());
        });
        c.sync();

        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
        wl_buffer_destroy(maximizedBuffer);
        c.sync();
    });
}

TEST_F(XdgToplevelDragProtocolTest, EscapeRevertsAnExistingWindowAndIsNoDrop)
{
    run([&](Client &c) {
        Toplevel *origin = mapSourceWindowAndPress(c);
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        xdg_toplevel_drag_v1_attach(drag, origin->toplevel, 50, 50);
        wl_data_device_start_drag(c.dataDevice, source, origin->surface, nullptr, c.buttonSerial);
        c.sync();

        check([&] {
            wm::Window *window = windowAt(0);
            EXPECT_EQ(window, windowManager.eventHandler()->draggedWindow());
            manager.onPointerMotionAbsolute(at(3), base::PointF(300, 250));
            EXPECT_EQ(base::Rect(250, 200, 300, 200), window->bounds());
            manager.onKey(at(4), KEY_ESC, true);
            manager.onKey(at(5), KEY_ESC, false);
            EXPECT_FALSE(seat.drag()->active());
            EXPECT_FALSE(windowManager.eventHandler()->isDragging());
            EXPECT_EQ(base::Rect(100, 100, 300, 200), window->bounds());
            manager.onPointerButton(at(6), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(0, c.dropsPerformed);
        EXPECT_EQ(1, c.cancels);

        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
        c.sync();
    });
}

TEST_F(XdgToplevelDragProtocolTest, UnmappingTheWindowDetachesItAndTheDragGoesOn)
{
    run([&](Client &c) {
        Toplevel *origin = mapSourceWindowAndPress(c);
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        wl_data_device_start_drag(c.dataDevice, source, origin->surface, nullptr, c.buttonSerial);
        c.sync();
        Toplevel *detached = c.makeToplevel();
        xdg_toplevel_drag_v1_attach(drag, detached->toplevel, 20, 10);
        c.map(detached, base::Size(200, 150));

        // Snapping the tab back in: the client unmaps the window it tore off.
        wl_surface_attach(detached->surface, nullptr, 0, 0);
        wl_surface_commit(detached->surface);
        c.sync();
        check([&] {
            EXPECT_EQ(1u, windowManager.windows().size());
            EXPECT_TRUE(seat.drag()->active());
            EXPECT_FALSE(windowManager.eventHandler()->isDragging());
            manager.onPointerMotionAbsolute(at(3), base::PointF(200, 200));
            EXPECT_EQ(base::Rect(100, 100, 300, 200), windowAt(0)->bounds());
        });

        // Once unmapped, another toplevel may be attached.
        xdg_toplevel_drag_v1_attach(drag, detached->toplevel, 20, 10);
        c.sync();
        check([&] {
            manager.onPointerButton(at(4), BTN_LEFT, false);
        });
        c.sync();
        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
        c.sync();
    });
}

// Chrome drags a window by its only tab, starting the drag from that window,
// and hides it once the tab is merged into another window's tab strip.
TEST_F(XdgToplevelDragProtocolTest, HidingTheWindowTheDragStartedFromDoesNotEndIt)
{
    run([&](Client &c) {
        Toplevel *target = c.makeToplevel();
        c.map(target, base::Size(300, 200));
        check([&] {
            windowAt(0)->setBounds(base::Rect(500, 100, 300, 200));
        });
        Toplevel *origin = mapSourceWindowAndPress(c);
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        wl_data_device_start_drag(c.dataDevice, source, origin->surface, nullptr, c.buttonSerial);
        xdg_toplevel_drag_v1_attach(drag, origin->toplevel, 50, 50);
        c.sync();
        check([&] {
            manager.onPointerMotionAbsolute(at(3), base::PointF(550, 150));
        });
        c.sync();
        ASSERT_EQ(target->surface, c.dragEnterSurface);
        wl_data_offer_accept(c.dragOffer, c.dragEnterSerial, "application/x-lingmo-tab");
        wl_data_offer_set_actions(c.dragOffer, WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
                                  WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE);
        c.sync();

        // The client keeps the surface of the window it hides, as Chrome does.
        wl_surface_attach(origin->surface, nullptr, 0, 0);
        wl_surface_commit(origin->surface);
        xdg_toplevel_destroy(std::exchange(origin->toplevel, nullptr));
        xdg_surface_destroy(std::exchange(origin->xdgSurface, nullptr));
        c.sync();
        check([&] {
            EXPECT_EQ(1u, windowManager.windows().size());
            EXPECT_TRUE(seat.drag()->active());
            manager.onPointerMotionAbsolute(at(4), base::PointF(560, 160));
        });
        c.sync();
        EXPECT_EQ(target->surface, c.dragEnterSurface);
        EXPECT_EQ(0, c.cancels);

        check([&] {
            manager.onPointerButton(at(5), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(1, c.dropsPerformed);
        EXPECT_EQ(0, c.cancels);
        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
        c.sync();
    });
}

TEST_F(XdgToplevelDragProtocolTest, UnmappingADroppedWindowActivatesTheWindowItWasDroppedOn)
{
    run([&](Client &c) {
        // The tab goes from the source window into this one, to the right of it.
        Toplevel *target = c.makeToplevel();
        c.map(target, base::Size(300, 200));
        check([&] {
            windowAt(0)->setBounds(base::Rect(500, 100, 300, 200));
        });
        Toplevel *origin = mapSourceWindowAndPress(c);
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        wl_data_device_start_drag(c.dataDevice, source, origin->surface, nullptr, c.buttonSerial);
        c.sync();
        Toplevel *detached = c.makeToplevel();
        xdg_toplevel_drag_v1_attach(drag, detached->toplevel, 20, 10);
        c.map(detached, base::Size(200, 150));
        check([&] {
            EXPECT_FALSE(windowSized(base::Size(200, 150))->isActive());
            manager.onPointerMotionAbsolute(at(3), base::PointF(600, 200));
            manager.onPointerButton(at(4), BTN_LEFT, false);
        });
        c.sync();

        // The client takes the tab into the window below and lets the dragged one go.
        wl_surface_attach(detached->surface, nullptr, 0, 0);
        wl_surface_commit(detached->surface);
        c.sync();
        check([&] {
            ASSERT_EQ(2u, windowManager.windows().size());
            for (wm::Window *window : windowManager.windows())
                EXPECT_EQ(window->bounds().x() == 500, window->isActive());
        });

        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
        c.sync();
    });
}

TEST_F(XdgToplevelDragProtocolTest, OverAWindowOfItsOwnThatTakesTheDropThePreviewGoesOnItsNextCommit)
{
    run([&](Client &c) {
        Toplevel *origin = mapSourceWindowAndPress(c);
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        wl_data_device_start_drag(c.dataDevice, source, origin->surface, nullptr, c.buttonSerial);
        c.sync();
        Toplevel *detached = c.makeToplevel();
        xdg_toplevel_drag_v1_attach(drag, detached->toplevel, 20, 10);
        c.map(detached, base::Size(200, 150));

        // Over the window it came from, which takes the tab back.
        ASSERT_EQ(origin->surface, c.dragEnterSurface);
        wl_data_offer_accept(c.dragOffer, c.dragEnterSerial, "application/x-lingmo-tab");
        wl_data_offer_set_actions(c.dragOffer, WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
                                  WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE);
        c.sync();
        check([&] {
            EXPECT_FLOAT_EQ(wm::ToplevelDrag::kPreviewOpacity,
                            opacityOf(windowSized(base::Size(200, 150))));
        });
        // The frame that shows where the tab would go.
        wl_surface_damage(origin->surface, 0, 0, 10, 10);
        wl_surface_commit(origin->surface);
        c.sync();
        check([&] {
            EXPECT_FLOAT_EQ(0, opacityOf(windowSized(base::Size(200, 150))));
            // Off it, the preview is back at once.
            manager.onPointerMotionAbsolute(at(3), base::PointF(600, 500));
            EXPECT_FLOAT_EQ(wm::ToplevelDrag::kPreviewOpacity,
                            opacityOf(windowSized(base::Size(200, 150))));
            manager.onKey(at(4), KEY_ESC, true);
            manager.onKey(at(5), KEY_ESC, false);
            // Cancelled, its client takes it back: not drawn meanwhile.
            EXPECT_FLOAT_EQ(0, opacityOf(windowSized(base::Size(200, 150))));
            EXPECT_FALSE(windowSized(base::Size(200, 150))->isActive());
            manager.onPointerButton(at(6), BTN_LEFT, false);
        });
        c.sync();
        c.destroy(detached);
        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
        c.sync();
    });
}

// Attached after the drag docked on it, a window shares the dock watch; its
// destruction must not leave the drag holding its surface (caught by ASan).
TEST_F(XdgToplevelDragProtocolTest, AWindowDockedOnThenAttachedAndDestroyedIsLetGo)
{
    run([&](Client &c) {
        // Another window of the client, whose subsurface takes the drop.
        Toplevel *target = c.makeToplevel();
        c.map(target, base::Size(300, 200));
        check([&] {
            windowAt(0)->setBounds(base::Rect(500, 100, 300, 200));
        });
        wl_surface *child = wl_compositor_create_surface(c.compositor);
        wl_subsurface *subsurface =
                wl_subcompositor_get_subsurface(c.subcompositor, child, target->surface);
        wl_buffer *childBuffer = c.makeBuffer(base::Size(100, 100));
        wl_surface_attach(child, childBuffer, 0, 0);
        wl_surface_damage(child, 0, 0, 100, 100);
        wl_surface_commit(child);
        wl_surface_commit(target->surface);

        Toplevel *origin = mapSourceWindowAndPress(c);
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        wl_data_device_start_drag(c.dataDevice, source, origin->surface, nullptr, c.buttonSerial);
        c.sync();
        Toplevel *detached = c.makeToplevel();
        xdg_toplevel_drag_v1_attach(drag, detached->toplevel, 20, 10);
        c.map(detached, base::Size(200, 150));
        check([&] {
            manager.onPointerMotionAbsolute(at(3), base::PointF(550, 150));
        });
        c.sync();
        ASSERT_EQ(child, c.dragEnterSurface);
        wl_data_offer_accept(c.dragOffer, c.dragEnterSerial, "application/x-lingmo-tab");
        wl_data_offer_set_actions(c.dragOffer, WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
                                  WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE);
        c.sync();

        // The client lets the preview go and attaches the window it is docked
        // on, then unmaps and destroys that one, all before the pointer moves.
        wl_surface_attach(detached->surface, nullptr, 0, 0);
        wl_surface_commit(detached->surface);
        xdg_toplevel_drag_v1_attach(drag, target->toplevel, 0, 0);
        c.sync();
        wl_surface_attach(target->surface, nullptr, 0, 0);
        wl_surface_commit(target->surface);
        c.destroy(target);
        c.sync();
        check([&] {
            EXPECT_EQ(1u, windowManager.windows().size());
            EXPECT_TRUE(seat.drag()->active());
            manager.onPointerMotionAbsolute(at(4), base::PointF(200, 150));
            manager.onPointerButton(at(5), BTN_LEFT, false);
            EXPECT_FALSE(seat.drag()->active());
        });
        c.sync();
        wl_subsurface_destroy(subsurface);
        wl_surface_destroy(child);
        wl_buffer_destroy(childBuffer);
        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
        c.sync();
    });
}

TEST_F(XdgToplevelDragProtocolTest, AToplevelWhoseSurfaceIsGoneIsNotAttached)
{
    run([&](Client &c) {
        Toplevel *gone = c.makeToplevel();
        wl_surface_destroy(gone->surface);
        gone->surface = nullptr;
        Toplevel *next = c.makeToplevel();
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        xdg_toplevel_drag_v1_attach(drag, gone->toplevel, 0, 0);
        // Nothing was attached, so another toplevel may be.
        xdg_toplevel_drag_v1_attach(drag, next->toplevel, 0, 0);
        c.sync();
        EXPECT_EQ(0, wl_display_get_error(c.display));

        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
        c.sync();
    });
}

TEST_F(XdgToplevelDragProtocolTest, ASourceTakesOneToplevelDrag)
{
    run([&](Client &c) {
        wl_data_source *source = c.makeSource();
        auto *first = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        auto *second = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        EXPECT_EQ(uint32_t(XDG_TOPLEVEL_DRAG_MANAGER_V1_ERROR_INVALID_SOURCE),
                  c.syncExpectingError());
        xdg_toplevel_drag_v1_destroy(second);
        xdg_toplevel_drag_v1_destroy(first);
        wl_data_source_destroy(source);
    });
}

TEST_F(XdgToplevelDragProtocolTest, AToplevelDragSourceIsNoSelection)
{
    run([&](Client &c) {
        Toplevel *origin = mapSourceWindowAndPress(c);
        (void)origin;
        wl_data_source *source = wl_data_device_manager_create_data_source(c.dataDeviceManager);
        wl_data_source_offer(source, "text/plain");
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        wl_data_device_set_selection(c.dataDevice, source, c.buttonSerial);
        EXPECT_EQ(uint32_t(WL_DATA_SOURCE_ERROR_INVALID_SOURCE), c.syncExpectingError());
        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
    });
}

TEST_F(XdgToplevelDragProtocolTest, ASecondAttachWhileOneIsAttachedIsAnError)
{
    run([&](Client &c) {
        Toplevel *first = c.makeToplevel();
        Toplevel *second = c.makeToplevel();
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        xdg_toplevel_drag_v1_attach(drag, first->toplevel, 0, 0);
        xdg_toplevel_drag_v1_attach(drag, second->toplevel, 0, 0);
        EXPECT_EQ(uint32_t(XDG_TOPLEVEL_DRAG_V1_ERROR_TOPLEVEL_ATTACHED), c.syncExpectingError());
        xdg_toplevel_drag_v1_destroy(drag);
        wl_data_source_destroy(source);
    });
}

TEST_F(XdgToplevelDragProtocolTest, DestroyingTheDragBeforeItEndsIsAnError)
{
    run([&](Client &c) {
        Toplevel *origin = mapSourceWindowAndPress(c);
        wl_data_source *source = c.makeSource();
        auto *drag = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(c.dragManager, source);
        xdg_toplevel_drag_v1_attach(drag, origin->toplevel, 50, 50);
        wl_data_device_start_drag(c.dataDevice, source, origin->surface, nullptr, c.buttonSerial);
        xdg_toplevel_drag_v1_destroy(drag);
        EXPECT_EQ(uint32_t(XDG_TOPLEVEL_DRAG_V1_ERROR_ONGOING_DRAG), c.syncExpectingError());
        wl_data_source_destroy(source);
    });
}
