// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <cstring>
#include <format>
#include <string>
#include <ext-foreign-toplevel-list-v1-client-protocol.h>
#include <ext-image-capture-source-v1-client-protocol.h>
#include <ext-image-copy-capture-v1-client-protocol.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-server-core.h>

#include "compositor/capture_source.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "wayland/protocols/foreign_toplevel_list.h"
#include "wayland/protocols/image_capture.h"
#include "wayland/protocols/wl_shm.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

const base::Rect kOutput(0, 0, 1000, 800);

struct Handle {
    ext_foreign_toplevel_handle_v1 *proxy = nullptr;
    std::string identifier;
    std::string title;
    std::string appId;
    int done = 0;
    bool closed = false;
};

class ForeignToplevelListTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    wm::WindowManager manager {&compositor, &seat};
    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);

    wl_display *server = nullptr;
    wl_display *client = nullptr;
    wl_registry *registry = nullptr;
    ext_foreign_toplevel_list_v1 *list = nullptr;
    uint32_t listName = 0;
    wl_shm *shm = nullptr;
    ext_foreign_toplevel_image_capture_source_manager_v1 *toplevelSources = nullptr;
    ext_image_copy_capture_manager_v1 *copies = nullptr;
    std::unique_ptr<wayland::ImageCaptureManager> capture;
    std::unique_ptr<wayland::ForeignToplevelList> toplevels;
    std::vector<std::unique_ptr<Handle>> handles;
    bool finished = false;
    bool allowed = true;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        compositor.setOutputViews(
                {{"output-1", kOutput,
                  *compositor::OutputScale::fromNumerator(240), "eDP-1"}});

        server = wl_display_create();
        ASSERT_NE(server, nullptr);
        ASSERT_NE(wayland::createShmGlobal(server, &compositor), nullptr);
        capture = std::make_unique<wayland::ImageCaptureManager>(
                server,
                [&](const wl_client *) {
                    return allowed;
                },
                [] { });
        toplevels = std::make_unique<wayland::ForeignToplevelList>(
                server, &manager, &compositor, [&](const wl_client *) {
                    return allowed;
                });
        capture->setToplevelSourceResolver([this](wl_resource *handle) {
            return toplevels->sourceFor(handle);
        });
        int sockets[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
        ASSERT_NE(wl_client_create(server, sockets[0]), nullptr);
        client = wl_display_connect_to_fd(sockets[1]);
        ASSERT_NE(client, nullptr);
        registry = wl_display_get_registry(client);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t) {
                    auto &test = *static_cast<ForeignToplevelListTest *>(data);
                    if (!strcmp(interface, ext_foreign_toplevel_list_v1_interface.name))
                        test.bindList(registry, name);
                    else if (!strcmp(interface, wl_shm_interface.name))
                        test.shm = static_cast<wl_shm *>(
                                wl_registry_bind(registry, name, &wl_shm_interface, 1));
                    else if (!strcmp(interface,
                                     ext_foreign_toplevel_image_capture_source_manager_v1_interface
                                             .name))
                        test.toplevelSources = static_cast<
                                ext_foreign_toplevel_image_capture_source_manager_v1 *>(
                                wl_registry_bind(
                                        registry, name,
                                        &ext_foreign_toplevel_image_capture_source_manager_v1_interface,
                                        1));
                    else if (!strcmp(interface, ext_image_copy_capture_manager_v1_interface.name))
                        test.copies = static_cast<ext_image_copy_capture_manager_v1 *>(
                                wl_registry_bind(registry, name,
                                                 &ext_image_copy_capture_manager_v1_interface, 1));
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        ASSERT_TRUE(roundtrip());
    }

    void TearDown() override
    {
        for (auto &handle : handles)
            ext_foreign_toplevel_handle_v1_destroy(handle->proxy);
        if (list)
            ext_foreign_toplevel_list_v1_destroy(list);
        if (copies)
            ext_image_copy_capture_manager_v1_destroy(copies);
        if (toplevelSources)
            ext_foreign_toplevel_image_capture_source_manager_v1_destroy(toplevelSources);
        if (shm)
            wl_shm_destroy(shm);
        wl_registry_destroy(registry);
        wl_display_disconnect(client);
        wl_display_destroy_clients(server);
        toplevels.reset();
        capture.reset();
        wl_display_destroy(server);
        seat.shutdown();
    }

    void bindList(wl_registry *registry, uint32_t name)
    {
        listName = name;
        list = static_cast<ext_foreign_toplevel_list_v1 *>(
                wl_registry_bind(registry, name, &ext_foreign_toplevel_list_v1_interface, 1));
        static const ext_foreign_toplevel_list_v1_listener listener = {
                [](void *data, ext_foreign_toplevel_list_v1 *,
                   ext_foreign_toplevel_handle_v1 *proxy) {
                    static_cast<ForeignToplevelListTest *>(data)->track(proxy);
                },
                [](void *data, ext_foreign_toplevel_list_v1 *) {
                    static_cast<ForeignToplevelListTest *>(data)->finished = true;
                }};
        ext_foreign_toplevel_list_v1_add_listener(list, &listener, this);
    }

    void track(ext_foreign_toplevel_handle_v1 *proxy)
    {
        auto handle = std::make_unique<Handle>();
        handle->proxy = proxy;
        static const ext_foreign_toplevel_handle_v1_listener listener = {
                [](void *data, ext_foreign_toplevel_handle_v1 *) {
                    static_cast<Handle *>(data)->closed = true;
                },
                [](void *data, ext_foreign_toplevel_handle_v1 *) {
                    ++static_cast<Handle *>(data)->done;
                },
                [](void *data, ext_foreign_toplevel_handle_v1 *, const char *title) {
                    static_cast<Handle *>(data)->title = title;
                },
                [](void *data, ext_foreign_toplevel_handle_v1 *, const char *appId) {
                    static_cast<Handle *>(data)->appId = appId;
                },
                [](void *data, ext_foreign_toplevel_handle_v1 *, const char *identifier) {
                    static_cast<Handle *>(data)->identifier = identifier;
                }};
        ext_foreign_toplevel_handle_v1_add_listener(proxy, &listener, handle.get());
        handles.push_back(std::move(handle));
    }

    bool roundtrip()
    {
        bool received = false;
        auto *sync = wl_display_sync(client);
        static const wl_callback_listener listener = {[](void *data, wl_callback *, uint32_t) {
            *static_cast<bool *>(data) = true;
        }};
        wl_callback_add_listener(sync, &listener, &received);
        wl_display_flush(client);
        wl_event_loop_dispatch(wl_display_get_event_loop(server), 0);
        wl_display_flush_clients(server);
        const int result = wl_display_dispatch(client);
        wl_callback_destroy(sync);
        return result >= 0 && received;
    }

    compositor::Surface *makeSurface(const base::Size &size)
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        auto buffer = pool->createBuffer(size, 0, size.width() * 4,
                                         render::PixelFormat::Bgrx8888,
                                         nullptr);
        surface->attach(buffer, {});
        surface->damage(base::Rect(base::Point(), size));
        surface->commit();
        compositor.updateTextures(renderer);
        return surface;
    }

    wm::Window *makeWindow(const base::Rect &bounds, const std::string &title = "Editor",
                           const std::string &appId = "org.example.Editor")
    {
        wm::Window *window = manager.createWindow(
                makeSurface(bounds.size()),
                {.requestedSize = bounds.size(), .title = title, .applicationId = appId});
        window->setBounds(bounds);
        return window;
    }

    // Resolves a client handle through the server-side proxy of the same id.
    std::shared_ptr<compositor::CaptureSource> sourceOf(const Handle &handle)
    {
        wl_client *wlClient = nullptr;
        wl_client_for_each(wlClient, wl_display_get_client_list(server)) break;
        wl_resource *resource = wl_client_get_object(
                wlClient, wl_proxy_get_id(reinterpret_cast<wl_proxy *>(handle.proxy)));
        return toplevels->sourceFor(resource);
    }
};

TEST_F(ForeignToplevelListTest, ListsApplicationWindowsWithIdentityAndSource)
{
    wm::Window *window = makeWindow(base::Rect(100, 50, 300, 200));
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(handles.size(), 1u);
    const Handle &handle = *handles.front();
    EXPECT_EQ(handle.title, "Editor");
    EXPECT_EQ(handle.appId, "org.example.Editor");
    // The shell names a window to capture clients by the id it knows it by.
    EXPECT_EQ(handle.identifier, std::format("lingmo-{}", window->id()));
    EXPECT_GE(handle.done, 1);
    EXPECT_FALSE(handle.closed);

    auto source = sourceOf(handle);
    ASSERT_TRUE(source);
    EXPECT_EQ(source->kind, compositor::CaptureSource::Kind::Window);
    EXPECT_EQ(source->node, window->node());
    EXPECT_EQ(source->bounds, base::Rect(100, 50, 300, 200));
    EXPECT_EQ(source->renderedOn(), "eDP-1");
    // Pixels follow the output's scale.
    EXPECT_EQ(source->pixelSize(), base::Size(600, 400));

    window->setTitle("Editor - file.txt");
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(handle.title, "Editor - file.txt");
    EXPECT_GE(handle.done, 2);

    // A resize changes the buffer size the session must announce.
    int changes = 0;
    // The subscription dies with the locals the lambda captures.
    const auto scope = source->changed.connect([&] {
        ++changes;
    });
    window->setBounds(base::Rect(100, 50, 400, 200));
    EXPECT_EQ(source->pixelSize(), base::Size(800, 400));
    EXPECT_EQ(changes, 1);
    window->setBounds(base::Rect(120, 60, 400, 200));
    EXPECT_EQ(changes, 1);
}

TEST_F(ForeignToplevelListTest, PopupsAndSystemDialogsAreNotListed)
{
    manager.setSystemDialogPolicy([](int64_t pid) {
        return pid == 42;
    });
    wm::Window *app = makeWindow(base::Rect(0, 0, 300, 200));
    manager.createWindow(makeSurface(base::Size(100, 40)),
                         {.type = wm::WindowType::Popup, .transientParent = app});
    wm::Window *prompt = manager.createWindow(
            makeSurface(base::Size(200, 100)),
            {.requestedSize = {200, 100}, .processId = 42, .transientParent = app});
    ASSERT_TRUE(prompt->isSystemDialog());
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(handles.size(), 1u);
    manager.destroyWindow(prompt);
    manager.destroyWindow(app);
    ASSERT_TRUE(roundtrip());
    for (const auto &handle : handles)
        EXPECT_TRUE(handle->closed);
}

TEST_F(ForeignToplevelListTest, ClosingAWindowStopsItsSource)
{
    wm::Window *window = makeWindow(base::Rect(0, 0, 300, 200));
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(handles.size(), 1u);
    auto source = sourceOf(*handles.front());
    ASSERT_TRUE(source);
    int changes = 0;
    // The subscription dies with the locals the lambda captures.
    const auto scope = source->changed.connect([&] {
        ++changes;
    });
    manager.destroyWindow(window);
    ASSERT_TRUE(roundtrip());
    EXPECT_TRUE(handles.front()->closed);
    EXPECT_FALSE(source->active);
    EXPECT_EQ(source->node, nullptr);
    EXPECT_EQ(changes, 1);
    EXPECT_FALSE(sourceOf(*handles.front()));
}

TEST_F(ForeignToplevelListTest, AShadowedSourceCoversTheShadowBounds)
{
    wm::Window *window = makeWindow(base::Rect(100, 50, 300, 200));
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(handles.size(), 1u);
    auto plain = sourceOf(*handles.front());
    auto shadowed = toplevels->shadowedSourceFor(window);
    ASSERT_TRUE(shadowed);
    EXPECT_NE(shadowed, plain);
    EXPECT_EQ(shadowed->kind, compositor::CaptureSource::Kind::Window);
    EXPECT_EQ(shadowed->node, window->node());
    // Without a shadow the picture is the window.
    EXPECT_EQ(shadowed->bounds, base::Rect(100, 50, 300, 200));
    EXPECT_EQ(toplevels->shadowedSourceFor(nullptr), nullptr);

    int changes = 0;
    // The subscription dies with the locals the lambda captures.
    const auto scope = shadowed->changed.connect([&] {
        ++changes;
    });
    toplevels->setShadowBounds([](const wm::Window *window) {
        return window->bounds().adjusted(-20, -10, 20, 40);
    });
    EXPECT_EQ(shadowed->bounds, base::Rect(80, 40, 340, 250));
    EXPECT_EQ(shadowed->pixelSize(), base::Size(680, 500));
    EXPECT_EQ(shadowed->renderedOn(), "eDP-1");
    EXPECT_EQ(changes, 1);
    EXPECT_EQ(plain->bounds, base::Rect(100, 50, 300, 200));

    // The margins may reach outside the output.
    window->setBounds(base::Rect(0, 0, 300, 200));
    EXPECT_EQ(shadowed->bounds, base::Rect(-20, -10, 340, 250));
    EXPECT_EQ(changes, 1);
    EXPECT_EQ(toplevels->shadowedSourceFor(window), shadowed);

    manager.destroyWindow(window);
    ASSERT_TRUE(roundtrip());
    EXPECT_FALSE(shadowed->active);
    EXPECT_EQ(shadowed->node, nullptr);
    EXPECT_EQ(changes, 2);
}

TEST_F(ForeignToplevelListTest, AnUnlistedWindowStillHasAShadowedSource)
{
    wm::Window *app = makeWindow(base::Rect(0, 0, 300, 200));
    wm::Window *popup = manager.createWindow(makeSurface(base::Size(100, 40)),
                                             {.type = wm::WindowType::Popup, .transientParent = app});
    popup->setBounds(base::Rect(50, 60, 100, 40));
    toplevels->setShadowBounds([](const wm::Window *window) {
        return window->bounds().adjusted(-8, -8, 8, 8);
    });
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(handles.size(), 1u);

    auto shadowed = toplevels->shadowedSourceFor(popup);
    ASSERT_TRUE(shadowed);
    EXPECT_EQ(shadowed->node, popup->node());
    EXPECT_EQ(shadowed->bounds, base::Rect(42, 52, 116, 56));
    EXPECT_EQ(toplevels->shadowedSourceFor(popup), shadowed);
    // A late list still sees only the application window.
    bindList(registry, listName);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(handles.size(), 2u);

    manager.destroyWindow(popup);
    EXPECT_FALSE(shadowed->active);
    EXPECT_EQ(shadowed->node, nullptr);
    toplevels->shutdown();
    EXPECT_EQ(toplevels->shadowedSourceFor(app), nullptr);
}

TEST_F(ForeignToplevelListTest, AWindowSourceIsPaintedOnItsOwnAtItsScale)
{
    struct WindowRenderer : testsupport::FakeRenderer {
        render::OutputRenderContext seen;
        base::Size viewportSize() const override { return {1000, 800}; }
        ui::Bitmap captureImage(const render::OutputRenderContext &context, const base::Size &size,
                                const std::function<void()> &paint) override
        {
            seen = context;
            paint();
            ui::Bitmap image(size, ui::Bitmap::Format::Argb32Premultiplied);
            image.fill(0xff010203);
            return image;
        }
    } windowRenderer;
    windowRenderer.setOutputContext(
            {"eDP-1", kOutput, {2000, 1600}, 2, 0, 17, base::Rect(0, 0, 2000, 1600)});

    makeWindow(base::Rect(100, 50, 300, 200));
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(handles.size(), 1u);
    ASSERT_NE(toplevelSources, nullptr);
    ASSERT_NE(copies, nullptr);

    auto *source = ext_foreign_toplevel_image_capture_source_manager_v1_create_source(
            toplevelSources, handles.front()->proxy);
    auto *session = ext_image_copy_capture_manager_v1_create_session(copies, source, 0);
    base::Size constraint;
    static const ext_image_copy_capture_session_v1_listener sessionListener = {
            [](void *data, ext_image_copy_capture_session_v1 *, uint32_t w, uint32_t h) {
                *static_cast<base::Size *>(data) = base::Size(int(w), int(h));
            },
            [](void *, ext_image_copy_capture_session_v1 *, uint32_t) {},
            [](void *, ext_image_copy_capture_session_v1 *, wl_array *) {},
            [](void *, ext_image_copy_capture_session_v1 *, uint32_t, wl_array *) {},
            [](void *, ext_image_copy_capture_session_v1 *) {},
            [](void *, ext_image_copy_capture_session_v1 *) {}};
    ext_image_copy_capture_session_v1_add_listener(session, &sessionListener, &constraint);
    ASSERT_TRUE(roundtrip());
    // 300x200 logical on a 2x output.
    EXPECT_EQ(constraint, base::Size(600, 400));

    const int size = 600 * 400 * 4;
    const int fd = memfd_create("capture-test", MFD_CLOEXEC);
    ASSERT_EQ(ftruncate(fd, size), 0);
    void *pixels = mmap(nullptr, size_t(size), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ASSERT_NE(pixels, MAP_FAILED);
    auto *pool = wl_shm_create_pool(shm, fd, size);
    auto *buffer = wl_shm_pool_create_buffer(pool, 0, 600, 400, 2400, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    auto *frame = ext_image_copy_capture_session_v1_create_frame(session);
    struct Result {
        uint32_t transform = 99;
        int ready = 0;
        int failed = -1;
    } result;
    static const ext_image_copy_capture_frame_v1_listener frameListener = {
            [](void *data, ext_image_copy_capture_frame_v1 *, uint32_t value) {
                static_cast<Result *>(data)->transform = value;
            },
            [](void *, ext_image_copy_capture_frame_v1 *, int32_t, int32_t, int32_t, int32_t) {},
            [](void *, ext_image_copy_capture_frame_v1 *, uint32_t, uint32_t, uint32_t) {},
            [](void *data, ext_image_copy_capture_frame_v1 *) {
                ++static_cast<Result *>(data)->ready;
            },
            [](void *data, ext_image_copy_capture_frame_v1 *, uint32_t reason) {
                static_cast<Result *>(data)->failed = int(reason);
            }};
    ext_image_copy_capture_frame_v1_add_listener(frame, &frameListener, &result);
    ext_image_copy_capture_frame_v1_attach_buffer(frame, buffer);
    ext_image_copy_capture_frame_v1_capture(frame);
    ASSERT_TRUE(roundtrip());
    wayland::ImageCaptureManager::Painters painters;
    painters.scene = [](bool) {
        FAIL() << "a window source must not paint the whole scene";
    };
    capture->renderPending(windowRenderer, painters);
    capture->presented(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(result.failed, -1);
    EXPECT_EQ(result.ready, 1);
    EXPECT_EQ(result.transform, 0u);
    EXPECT_EQ(windowRenderer.seen.logicalViewport, base::Rect(100, 50, 300, 200));
    EXPECT_EQ(windowRenderer.seen.nativeTargetSize, base::Size(600, 400));
    EXPECT_DOUBLE_EQ(windowRenderer.seen.scale, 2.0);
    EXPECT_EQ(windowRenderer.seen.transform, 0u);
    auto *bytes = static_cast<uint8_t *>(pixels);
    EXPECT_EQ(bytes[0], 3);
    EXPECT_EQ(bytes[1], 2);
    EXPECT_EQ(bytes[2], 1);

    ext_image_copy_capture_frame_v1_destroy(frame);
    wl_buffer_destroy(buffer);
    ext_image_copy_capture_session_v1_destroy(session);
    ext_image_capture_source_v1_destroy(source);
    munmap(pixels, size_t(size));
    close(fd);
    ASSERT_TRUE(roundtrip());
}

TEST_F(ForeignToplevelListTest, StopFinishesTheListAndLateBindersSeeExistingWindows)
{
    makeWindow(base::Rect(0, 0, 300, 200));
    ASSERT_TRUE(roundtrip());
    ext_foreign_toplevel_list_v1_stop(list);
    ASSERT_TRUE(roundtrip());
    EXPECT_TRUE(finished);
    makeWindow(base::Rect(0, 0, 300, 200));
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(handles.size(), 1u);
}

// A screen recorder binds a list for each recording and drops it when it exits.
TEST_F(ForeignToplevelListTest, ADestroyedListLeavesNothingBehind)
{
    // SetUp's roundtrip only delivers the globals; the list bind they send reaches the server now.
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(toplevels->boundListCount(), 1);
    for (int recording = 0; recording < 3; ++recording) {
        auto *extra = static_cast<ext_foreign_toplevel_list_v1 *>(
                wl_registry_bind(registry, listName, &ext_foreign_toplevel_list_v1_interface, 1));
        ASSERT_TRUE(roundtrip());
        EXPECT_EQ(toplevels->boundListCount(), 2);
        ext_foreign_toplevel_list_v1_destroy(extra);
        ASSERT_TRUE(roundtrip());
        EXPECT_EQ(toplevels->boundListCount(), 1);
    }
    ext_foreign_toplevel_list_v1_destroy(std::exchange(list, nullptr));
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(toplevels->boundListCount(), 0);
    makeWindow(base::Rect(0, 0, 300, 200));
    ASSERT_TRUE(roundtrip());
    EXPECT_TRUE(handles.empty());
}

} // namespace
