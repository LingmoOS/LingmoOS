// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <atomic>
#include <chrono>
#include <cstring>
#include <lingmo-surface-blur-v1-client-protocol.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <thread>
#include <wayland-client.h>
#include <wayland-server-core.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "scene/scene.h"
#include "wayland/protocols/surface_blur.h"
#include "wayland/protocols/wl_compositor.h"

using namespace Lingmo;

namespace {

class SurfaceBlurProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    wl_display *server = nullptr;
    wl_display *client = nullptr;
    wl_registry *registry = nullptr;
    wl_compositor *clientCompositor = nullptr;
    lingmo_surface_blur_v1 *blur = nullptr;

    void SetUp() override
    {
        server = wl_display_create();
        ASSERT_TRUE(server);
        ASSERT_TRUE(wayland::createCompositorGlobal(server, &compositor));
        ASSERT_TRUE(wayland::createSurfaceBlurGlobal(server));
        int sockets[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
        ASSERT_TRUE(wl_client_create(server, sockets[0])) << std::strerror(errno);
        client = wl_display_connect_to_fd(sockets[1]);
        ASSERT_TRUE(client);
        registry = wl_display_get_registry(client);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t) {
                    auto *self = static_cast<SurfaceBlurProtocolTest *>(data);
                    if (std::strcmp(interface, "wl_compositor") == 0)
                        self->clientCompositor = static_cast<wl_compositor *>(
                                wl_registry_bind(registry, name, &wl_compositor_interface, 4));
                    else if (std::strcmp(interface, "lingmo_surface_blur_v1") == 0)
                        self->blur = static_cast<lingmo_surface_blur_v1 *>(wl_registry_bind(
                                registry, name, &lingmo_surface_blur_v1_interface, 1));
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        ASSERT_GE(roundtrip(), 0);
        ASSERT_TRUE(blur);
        ASSERT_TRUE(clientCompositor);
    }

    int roundtrip()
    {
        std::atomic<bool> done = false;
        int result = -1;
        std::thread worker([&] {
            result = wl_display_roundtrip(client);
            done = true;
        });
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (!done && std::chrono::steady_clock::now() < deadline) {
            wl_event_loop_dispatch(wl_display_get_event_loop(server), 1);
            wl_display_flush_clients(server);
        }
        if (!done)
            shutdown(wl_display_get_fd(client), SHUT_RDWR);
        worker.join();
        return result;
    }

    void TearDown() override
    {
        if (blur)
            lingmo_surface_blur_v1_destroy(blur);
        if (clientCompositor)
            wl_compositor_destroy(clientCompositor);
        if (registry)
            wl_registry_destroy(registry);
        if (client)
            wl_display_disconnect(client);
        if (server) {
            wl_display_destroy_clients(server);
            wl_display_destroy(server);
        }
    }
};

TEST_F(SurfaceBlurProtocolTest, SetUnsetAndBindingDestructionFollowSurfaceCommits)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    lingmo_surface_blur_v1_set(blur, surface, 3, 4, 120, 80, 100);
    ASSERT_GE(roundtrip(), 0);
    ASSERT_EQ(compositor.surfaces().size(), 1u);
    auto *model = compositor.surfaces().front();
    EXPECT_TRUE(model->backgroundBlurRect().isEmpty());
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->backgroundBlurRect(), base::Rect(3, 4, 120, 80));
    EXPECT_EQ(model->backgroundBlurRadius(), 40);
    lingmo_surface_blur_v1_unset(blur, surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->backgroundBlurRect().isEmpty());
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->backgroundBlurRect().isEmpty());
    lingmo_surface_blur_v1_set(blur, surface, 0, 0, 30, 20, 4);
    lingmo_surface_blur_v1_destroy(blur);
    blur = nullptr;
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->backgroundBlurRect(), base::Rect(0, 0, 30, 20));
    wl_surface_destroy(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(compositor.surfaces().empty());
}

namespace {

void append(wl_array *array, std::initializer_list<int32_t> values)
{
    for (int32_t value : values)
        *static_cast<int32_t *>(wl_array_add(array, sizeof(int32_t))) = value;
}

} // namespace

TEST_F(SurfaceBlurProtocolTest, ShapeIsPendingStateClippedToTheRectangle)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    wl_array path;
    wl_array_init(&path);
    append(&path,
           {LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_MOVE_TO, wl_fixed_from_int(0),
            wl_fixed_from_int(0)});
    append(&path,
           {LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_LINE_TO, wl_fixed_from_int(100),
            wl_fixed_from_int(0)});
    append(&path,
           {LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_CUBIC_TO, wl_fixed_from_int(100),
            wl_fixed_from_int(30), wl_fixed_from_int(80), wl_fixed_from_int(60),
            wl_fixed_from_int(50), wl_fixed_from_int(70)});
    append(&path,
           {LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_LINE_TO, wl_fixed_from_int(0),
            wl_fixed_from_int(60)});
    append(&path, {LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_CLOSE});
    lingmo_surface_blur_v1_set_shape(blur, surface, &path);
    wl_array_release(&path);
    lingmo_surface_blur_v1_set(blur, surface, 0, 0, 100, 60, 8);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_TRUE(model->backgroundBlurShape().isEmpty());
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    const ui::Path shape = model->backgroundBlurShape();
    ASSERT_FALSE(shape.isEmpty());
    EXPECT_TRUE(shape.contains(ui::PointF {90, 10}));
    EXPECT_FALSE(shape.contains(ui::PointF {98, 55}));
    // The part below the rectangle is cut off.
    EXPECT_LE(shape.boundingRect().bottom(), 60.0001);

    lingmo_surface_blur_v1_set_glass(blur, surface);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    // Glass masks its lens by the shape without clipping it to the rectangle.
    EXPECT_FALSE(model->backgroundBlurShape().isEmpty());
    lingmo_surface_blur_v1_unset_glass(blur, surface);
    lingmo_surface_blur_v1_unset_shape(blur, surface);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->backgroundBlurShape().isEmpty());
    wl_surface_destroy(surface);
    ASSERT_GE(roundtrip(), 0);
}

// The element rules the shape has always had: it starts at the origin, a
// move replaces a move, and closing an empty shape does nothing.
TEST_F(SurfaceBlurProtocolTest, ShapeKeepsItsElementRules)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    lingmo_surface_blur_v1_set(blur, surface, 0, 0, 100, 60, 8);
    lingmo_surface_blur_v1_set_glass(blur, surface);
    const auto shapeOf = [&](std::initializer_list<int32_t> values) {
        wl_array path;
        wl_array_init(&path);
        append(&path, values);
        lingmo_surface_blur_v1_set_shape(blur, surface, &path);
        wl_array_release(&path);
        wl_surface_commit(surface);
        EXPECT_GE(roundtrip(), 0);
        return compositor.surfaces().front()->backgroundBlurShape();
    };

    ui::Path shape = shapeOf({LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_LINE_TO, wl_fixed_from_int(10),
                              wl_fixed_from_int(10), LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_LINE_TO,
                              wl_fixed_from_int(10), wl_fixed_from_int(0)});
    EXPECT_FALSE(shape.isEmpty());
    EXPECT_EQ(shape.controlPointRect(), (ui::RectF {0, 0, 10, 10}));

    shape = shapeOf({LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_MOVE_TO, wl_fixed_from_int(90),
                     wl_fixed_from_int(50), LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_MOVE_TO,
                     wl_fixed_from_int(5), wl_fixed_from_int(5),
                     LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_LINE_TO, wl_fixed_from_int(10),
                     wl_fixed_from_int(10)});
    EXPECT_EQ(shape.controlPointRect(), (ui::RectF {5, 5, 5, 5}));

    shape = shapeOf({LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_MOVE_TO, wl_fixed_from_int(1),
                     wl_fixed_from_int(1), LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_CLOSE});
    EXPECT_TRUE(shape.isEmpty());

    // An empty segment is dropped, so the shape stays a lone move.
    shape = shapeOf({LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_MOVE_TO, wl_fixed_from_int(1),
                     wl_fixed_from_int(1), LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_LINE_TO,
                     wl_fixed_from_int(1), wl_fixed_from_int(1)});
    EXPECT_TRUE(shape.isEmpty());
    wl_surface_destroy(surface);
    ASSERT_GE(roundtrip(), 0);
}

TEST_F(SurfaceBlurProtocolTest, SaturationAndRimArePendingMaterialState)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    lingmo_surface_blur_v1_set(blur, surface, 0, 0, 100, 60, 8);
    lingmo_surface_blur_v1_set_glass_saturation(blur, surface, wl_fixed_from_double(1.5));
    // Past full intensity is clamped rather than refused.
    lingmo_surface_blur_v1_set_glass_rim(blur, surface, wl_fixed_from_int(3));
    lingmo_surface_blur_v1_set_glass(blur, surface);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_FALSE(model->glass().enabled);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->glass().enabled);
    EXPECT_FLOAT_EQ(model->glass().saturation, 1.5f);
    EXPECT_FLOAT_EQ(model->glass().rim, 1.0f);
    wl_surface_destroy(surface);
    ASSERT_GE(roundtrip(), 0);
}

TEST_F(SurfaceBlurProtocolTest, NegativeRimPostsProtocolError)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    lingmo_surface_blur_v1_set_glass_rim(blur, surface, wl_fixed_from_int(-1));
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_SURFACE_BLUR_V1_ERROR_INVALID_GEOMETRY);
    wl_proxy_destroy(reinterpret_cast<wl_proxy *>(surface));
}

TEST_F(SurfaceBlurProtocolTest, MalformedShapePostsProtocolError)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    wl_array path;
    wl_array_init(&path);
    // Cut short: a line needs two coordinates.
    append(&path,
           {LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_MOVE_TO, 0, 0,
            LINGMO_SURFACE_BLUR_V1_PATH_COMMAND_LINE_TO, 0});
    lingmo_surface_blur_v1_set_shape(blur, surface, &path);
    wl_array_release(&path);
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_SURFACE_BLUR_V1_ERROR_INVALID_GEOMETRY);
    wl_proxy_destroy(reinterpret_cast<wl_proxy *>(surface));
}

TEST_F(SurfaceBlurProtocolTest, InvalidGeometryPostsProtocolError)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    lingmo_surface_blur_v1_set(blur, surface, 0, 0, -1, 80, 5);
    EXPECT_LT(roundtrip(), 0);
    const wl_interface *interface = nullptr;
    EXPECT_EQ(wl_display_get_protocol_error(client, &interface, nullptr),
              LINGMO_SURFACE_BLUR_V1_ERROR_INVALID_GEOMETRY);
    ASSERT_TRUE(interface);
    EXPECT_STREQ(interface->name, "lingmo_surface_blur_v1");
    wl_proxy_destroy(reinterpret_cast<wl_proxy *>(surface));
}

} // namespace
