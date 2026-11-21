// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// xdg-foreign-unstable-v2, driven through real libwayland-client connections:
// one client exports its toplevel, another imports the handle and parents its
// own toplevel onto it. This is what the file chooser portal needs.

#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>
#include <xdg-foreign-unstable-v2-client-protocol.h>
#include <xdg-shell-client-protocol.h>

#include "compositor/compositor.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/xdg_foreign.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/server.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

const base::Rect kOutput(0, 0, 800, 600);

struct XdgWindow {
    wl_surface *surface = nullptr;
    xdg_surface *xdgSurface = nullptr;
    xdg_toplevel *toplevel = nullptr;
    wl_buffer *buffer = nullptr;
    uint32_t lastSerial = 0;
};

const xdg_surface_listener surfaceListener = {
        [](void *data, xdg_surface *, uint32_t serial) {
            static_cast<XdgWindow *>(data)->lastSerial = serial;
        },
};

const xdg_toplevel_listener toplevelListener = {
        [](void *, xdg_toplevel *, int32_t, int32_t, wl_array *) { },
        [](void *, xdg_toplevel *) { },
        [](void *, xdg_toplevel *, int32_t, int32_t) { },
        [](void *, xdg_toplevel *, wl_array *) { },
};

struct Client {
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_compositor *compositor = nullptr;
    wl_shm *shm = nullptr;
    xdg_wm_base *wmBase = nullptr;
    zxdg_exporter_v2 *exporter = nullptr;
    zxdg_importer_v2 *importer = nullptr;
    std::map<std::string, std::pair<uint32_t, uint32_t>> globals;
    std::vector<std::unique_ptr<XdgWindow>> windows;

    std::string handle;
    int handles = 0;
    int destroyedEvents = 0;

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
        sync();

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
        if (globals.contains("zxdg_exporter_v2")) {
            exporter = static_cast<zxdg_exporter_v2 *>(wl_registry_bind(
                    registry, globals["zxdg_exporter_v2"].first, &zxdg_exporter_v2_interface, 1));
        }
        if (globals.contains("zxdg_importer_v2")) {
            importer = static_cast<zxdg_importer_v2 *>(wl_registry_bind(
                    registry, globals["zxdg_importer_v2"].first, &zxdg_importer_v2_interface, 1));
        }
        sync();
    }

    ~Client()
    {
        for (auto &window : windows) {
            if (window->toplevel)
                xdg_toplevel_destroy(window->toplevel);
            if (window->xdgSurface)
                xdg_surface_destroy(window->xdgSurface);
            if (window->surface)
                wl_surface_destroy(window->surface);
            if (window->buffer)
                wl_buffer_destroy(window->buffer);
        }
        windows.clear();
        if (importer)
            zxdg_importer_v2_destroy(importer);
        if (exporter)
            zxdg_exporter_v2_destroy(exporter);
        xdg_wm_base_destroy(wmBase);
        wl_shm_destroy(shm);
        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
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

    XdgWindow *makeMappedToplevel(const std::string &title)
    {
        windows.push_back(std::make_unique<XdgWindow>());
        XdgWindow *window = windows.back().get();
        window->surface = wl_compositor_create_surface(compositor);
        window->xdgSurface = xdg_wm_base_get_xdg_surface(wmBase, window->surface);
        xdg_surface_add_listener(window->xdgSurface, &surfaceListener, window);
        window->toplevel = xdg_surface_get_toplevel(window->xdgSurface);
        xdg_toplevel_add_listener(window->toplevel, &toplevelListener, window);
        xdg_toplevel_set_title(window->toplevel, title.c_str());
        wl_surface_commit(window->surface);
        sync();

        const base::Size size(200, 100);
        xdg_surface_ack_configure(window->xdgSurface, window->lastSerial);
        window->buffer = makeBuffer(size);
        wl_surface_attach(window->surface, window->buffer, 0, 0);
        wl_surface_damage(window->surface, 0, 0, size.width(), size.height());
        wl_surface_commit(window->surface);
        sync();
        return window;
    }

    // Exports the toplevel and returns the handle the compositor sent.
    zxdg_exported_v2 *exportToplevel(XdgWindow *window)
    {
        zxdg_exported_v2 *exported = zxdg_exporter_v2_export_toplevel(exporter, window->surface);
        static const zxdg_exported_v2_listener listener = {
                [](void *data, zxdg_exported_v2 *, const char *handle) {
                    auto *self = static_cast<Client *>(data);
                    self->handle = handle;
                    ++self->handles;
                }};
        zxdg_exported_v2_add_listener(exported, &listener, this);
        sync();
        return exported;
    }

    zxdg_imported_v2 *importToplevel(const std::string &handle)
    {
        zxdg_imported_v2 *imported = zxdg_importer_v2_import_toplevel(importer, handle.c_str());
        static const zxdg_imported_v2_listener listener = {[](void *data, zxdg_imported_v2 *) {
            ++static_cast<Client *>(data)->destroyedEvents;
        }};
        zxdg_imported_v2_add_listener(imported, &listener, this);
        sync();
        return imported;
    }

    void sync() { wl_display_roundtrip(display); }

    uint32_t syncExpectingError()
    {
        if (wl_display_roundtrip(display) >= 0)
            return 0;
        if (wl_display_get_error(display) != EPROTO)
            return 0;
        const wl_interface *raisedOn = nullptr;
        uint32_t id = 0;
        return wl_display_get_protocol_error(display, &raisedOn, &id);
    }
};

class XdgForeignProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
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

    wm::Window *windowTitled(const std::string &title)
    {
        for (wm::Window *window : windowManager.windows()) {
            if (window->title() == title)
                return window;
        }
        return nullptr;
    }

    // Both clients run on one thread: each connection is driven in turn.
    void run(std::function<void(Client &, Client &)> body,
             std::chrono::seconds timeout = std::chrono::seconds(5))
    {
        auto xdgShell = std::make_unique<wayland::WaylandXdgShell>(&compositor, &windowManager,
                                                                   &seat, server.serialTracker());
        ASSERT_TRUE(wayland::createCompositorGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createShmGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createXdgShellGlobal(server.display(), xdgShell.get()));
        ASSERT_TRUE(wayland::createXdgExporterV2Global(server.display()));
        ASSERT_TRUE(wayland::createXdgImporterV2Global(server.display()));

        int first[2];
        int second[2];
        ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, first));
        ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, second));
        ASSERT_TRUE(wl_client_create(server.display(), first[0]));
        ASSERT_TRUE(wl_client_create(server.display(), second[0]));

        std::atomic<bool> done = false;
        std::thread clients([&] {
            {
                Client exporter(first[1]);
                Client importer(second[1]);
                body(exporter, importer);
            }
            done = true;
        });

        const auto deadline = std::chrono::steady_clock::now() + timeout;
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
                shutdown(first[1], SHUT_RDWR);
                shutdown(second[1], SHUT_RDWR);
                break;
            }
        }
        clients.join();
        wl_display_destroy_clients(server.display());
        EXPECT_TRUE(windowManager.windows().empty());
    }
};

TEST_F(XdgForeignProtocolTest, TheRegistryListsBothGlobals)
{
    run([&](Client &exporter, Client &) {
        ASSERT_TRUE(exporter.globals.contains("zxdg_exporter_v2"));
        ASSERT_TRUE(exporter.globals.contains("zxdg_importer_v2"));
        EXPECT_EQ(wayland::kXdgExporterV2Version, exporter.globals["zxdg_exporter_v2"].second);
        EXPECT_EQ(wayland::kXdgImporterV2Version, exporter.globals["zxdg_importer_v2"].second);
    });
}

TEST_F(XdgForeignProtocolTest, AnImportedHandleParentsTheOtherClientsToplevel)
{
    run([&](Client &exporterClient, Client &importerClient) {
        XdgWindow *parent = exporterClient.makeMappedToplevel("parent");
        zxdg_exported_v2 *exported = exporterClient.exportToplevel(parent);
        ASSERT_EQ(1, exporterClient.handles);
        ASSERT_FALSE(exporterClient.handle.empty());

        XdgWindow *child = importerClient.makeMappedToplevel("child");
        zxdg_imported_v2 *imported = importerClient.importToplevel(exporterClient.handle);
        EXPECT_EQ(0, importerClient.destroyedEvents);
        zxdg_imported_v2_set_parent_of(imported, child->surface);
        importerClient.sync();

        check([&] {
            wm::Window *parentWindow = windowTitled("parent");
            wm::Window *childWindow = windowTitled("child");
            ASSERT_TRUE(parentWindow);
            ASSERT_TRUE(childWindow);
            EXPECT_EQ(parentWindow, childWindow->transientParent());
        });

        // Dropping the export invalidates the relationship it set up.
        zxdg_exported_v2_destroy(exported);
        exporterClient.sync();
        importerClient.sync();
        EXPECT_EQ(1, importerClient.destroyedEvents);

        check([&] {
            wm::Window *childWindow = windowTitled("child");
            ASSERT_TRUE(childWindow);
            EXPECT_EQ(nullptr, childWindow->transientParent());
        });

        zxdg_imported_v2_destroy(imported);
        importerClient.sync();
    });
}

TEST_F(XdgForeignProtocolTest, TheChildMayDropItsToplevelRoleBeforeTheExportGoes)
{
    run([&](Client &exporterClient, Client &importerClient) {
        XdgWindow *parent = exporterClient.makeMappedToplevel("parent");
        zxdg_exported_v2 *exported = exporterClient.exportToplevel(parent);
        XdgWindow *child = importerClient.makeMappedToplevel("child");
        zxdg_imported_v2 *imported = importerClient.importToplevel(exporterClient.handle);
        zxdg_imported_v2_set_parent_of(imported, child->surface);
        importerClient.sync();

        // The wl_surface outlives its role, so the parent can only be dropped
        // by looking the shell surface up again.
        xdg_toplevel_destroy(child->toplevel);
        child->toplevel = nullptr;
        xdg_surface_destroy(child->xdgSurface);
        child->xdgSurface = nullptr;
        importerClient.sync();

        zxdg_exported_v2_destroy(exported);
        exporterClient.sync();
        importerClient.sync();
        EXPECT_EQ(1, importerClient.destroyedEvents);

        zxdg_imported_v2_destroy(imported);
        importerClient.sync();
        check([&] {
            EXPECT_EQ(nullptr, windowTitled("child"));
        });
    });
}

TEST_F(XdgForeignProtocolTest, AnUnknownHandleIsImportedInert)
{
    run([&](Client &, Client &importerClient) {
        XdgWindow *child = importerClient.makeMappedToplevel("child");
        zxdg_imported_v2 *imported = importerClient.importToplevel("not-a-handle");
        EXPECT_EQ(1, importerClient.destroyedEvents);

        // The object stays usable; the request simply has no effect.
        zxdg_imported_v2_set_parent_of(imported, child->surface);
        importerClient.sync();
        check([&] {
            wm::Window *childWindow = windowTitled("child");
            ASSERT_TRUE(childWindow);
            EXPECT_EQ(nullptr, childWindow->transientParent());
        });
        zxdg_imported_v2_destroy(imported);
        importerClient.sync();
    });
}

TEST_F(XdgForeignProtocolTest, ExportingASurfaceWithoutAToplevelRoleIsAnError)
{
    run([&](Client &exporterClient, Client &) {
        wl_surface *surface = wl_compositor_create_surface(exporterClient.compositor);
        zxdg_exported_v2 *exported =
                zxdg_exporter_v2_export_toplevel(exporterClient.exporter, surface);
        EXPECT_EQ(uint32_t(ZXDG_EXPORTER_V2_ERROR_INVALID_SURFACE),
                  exporterClient.syncExpectingError());
        zxdg_exported_v2_destroy(exported);
        wl_surface_destroy(surface);
    });
}

TEST_F(XdgForeignProtocolTest, ParentingASurfaceWithoutAToplevelRoleIsAnError)
{
    run([&](Client &exporterClient, Client &importerClient) {
        XdgWindow *parent = exporterClient.makeMappedToplevel("parent");
        zxdg_exported_v2 *exported = exporterClient.exportToplevel(parent);
        ASSERT_FALSE(exporterClient.handle.empty());

        zxdg_imported_v2 *imported = importerClient.importToplevel(exporterClient.handle);
        wl_surface *surface = wl_compositor_create_surface(importerClient.compositor);
        zxdg_imported_v2_set_parent_of(imported, surface);
        EXPECT_EQ(uint32_t(ZXDG_IMPORTED_V2_ERROR_INVALID_SURFACE),
                  importerClient.syncExpectingError());
        zxdg_imported_v2_destroy(imported);
        wl_surface_destroy(surface);
        zxdg_exported_v2_destroy(exported);
    });
}

} // namespace
