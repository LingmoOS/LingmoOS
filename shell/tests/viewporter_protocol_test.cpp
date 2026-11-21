// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <thread>
#include <viewporter-client-protocol.h>
#include <wayland-client.h>
#include <wayland-server-core.h>

#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/protocols/wp_viewporter.h"

using namespace Lingmo;

namespace {

class ViewporterProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    wl_display* server = nullptr;
    wl_display* client = nullptr;
    wl_registry* registry = nullptr;
    wl_compositor* clientCompositor = nullptr;
    wp_viewporter* manager = nullptr;

    wl_shm* shm = nullptr;
    wl_subcompositor* subcompositor = nullptr;
    wl_surface* surface = nullptr;
    wp_viewport* viewport = nullptr;
    wl_buffer* buffer = nullptr;
    compositor::Surface* model = nullptr;
    uint32_t advertisedVersion = 0;

    void SetUp() override
    {
        server = wl_display_create();
        ASSERT_TRUE(server);
        ASSERT_TRUE(wayland::createCompositorGlobal(server, &compositor));
        ASSERT_TRUE(wayland::createViewporterGlobal(server));
        ASSERT_TRUE(wayland::createShmGlobal(server, &compositor));
        ASSERT_TRUE(wayland::createSubcompositorGlobal(server));
        int sockets[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
        ASSERT_TRUE(wl_client_create(server, sockets[0])) << std::strerror(errno);
        client = wl_display_connect_to_fd(sockets[1]);
        ASSERT_TRUE(client);
        registry = wl_display_get_registry(client);
        static const wl_registry_listener listener = {
            [](void* data, wl_registry* registry, uint32_t name, const char* interface,
               uint32_t version) {
                auto* self = static_cast<ViewporterProtocolTest*>(data);
                if (std::strcmp(interface, "wl_compositor") == 0)
                    self->clientCompositor = static_cast<wl_compositor*>(
                        wl_registry_bind(registry, name, &wl_compositor_interface, 4));
                else if (std::strcmp(interface, "wp_viewporter") == 0) {
                    self->advertisedVersion = version;
                    self->manager = static_cast<wp_viewporter*>(
                        wl_registry_bind(registry, name, &wp_viewporter_interface, 1));
                } else if (std::strcmp(interface, "wl_shm") == 0) {
                    self->shm = static_cast<wl_shm*>(
                        wl_registry_bind(registry, name, &wl_shm_interface, 1));
                } else if (std::strcmp(interface, "wl_subcompositor") == 0) {
                    self->subcompositor = static_cast<wl_subcompositor*>(
                        wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
                }
            },
            [](void*, wl_registry*, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        ASSERT_GE(roundtrip(), 0);
        ASSERT_TRUE(manager);
        ASSERT_TRUE(clientCompositor);
        ASSERT_TRUE(shm);
        ASSERT_TRUE(subcompositor);
        surface = wl_compositor_create_surface(clientCompositor);
        viewport = wp_viewporter_get_viewport(manager, surface);
        ASSERT_GE(roundtrip(), 0);
        model = compositor.surfaces().front();
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

    void source(double x, double y, double w, double h)
    {
        wp_viewport_set_source(viewport, wl_fixed_from_double(x), wl_fixed_from_double(y),
                               wl_fixed_from_double(w), wl_fixed_from_double(h));
    }

    void attach(int width, int height)
    {
        if (buffer)
            wl_buffer_destroy(buffer);
        int fd = testsupport::makeMemfd(width * height * 4);
        ASSERT_GE(fd, 0);
        auto* pool = wl_shm_create_pool(shm, fd, width * height * 4);
        close(fd);
        buffer =
            wl_shm_pool_create_buffer(pool, 0, width, height, width * 4, WL_SHM_FORMAT_XRGB8888);
        wl_shm_pool_destroy(pool);
        wl_surface_attach(surface, buffer, 0, 0);
    }

    void error(uint32_t code, const char* name = "wp_viewport", wl_proxy* object = nullptr)
    {
        const uint32_t expectedId =
            wl_proxy_get_id(object ? object : reinterpret_cast<wl_proxy*>(viewport));
        EXPECT_LT(roundtrip(), 0);
        const wl_interface* interface = nullptr;
        uint32_t id = 0;
        EXPECT_EQ(wl_display_get_protocol_error(client, &interface, &id), code);
        ASSERT_TRUE(interface);
        EXPECT_STREQ(interface->name, name);
        EXPECT_EQ(id, expectedId);
    }

    void TearDown() override
    {
        if (viewport)
            wp_viewport_destroy(viewport);
        if (surface)
            wl_surface_destroy(surface);
        if (buffer)
            wl_buffer_destroy(buffer);
        if (shm)
            wl_shm_destroy(shm);
        if (subcompositor)
            wl_subcompositor_destroy(subcompositor);
        if (manager)
            wp_viewporter_destroy(manager);
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

TEST_F(ViewporterProtocolTest, DiscoveryIndependentStateAndManagerLifetime)
{
    EXPECT_EQ(advertisedVersion, 1u);
    EXPECT_FALSE(model->hasRole());
    attach(400, 200);
    wl_surface_set_buffer_scale(surface, 2);
    source(25, 10, 100, 50);
    wp_viewport_set_destination(viewport, 200, 100);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->crop());
    EXPECT_FALSE(model->viewport());
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(200, 100));
    EXPECT_EQ(model->crop(), std::optional<base::RectF>({25, 10, 100, 50}));
    wp_viewporter_destroy(manager);
    manager = nullptr;
    wp_viewport_set_destination(viewport, -1, -1);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(100, 50));
    EXPECT_TRUE(model->crop());
    source(-1, -1, -1, -1);
    wp_viewport_set_destination(viewport, 80, 30);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->crop());
    EXPECT_EQ(model->contentSize(), base::Size(80, 30));
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(80, 30));
}

TEST_F(ViewporterProtocolTest, DuplicateViewport)
{
    auto* duplicate = wp_viewporter_get_viewport(manager, surface);
    error(WP_VIEWPORTER_ERROR_VIEWPORT_EXISTS, "wp_viewporter",
          reinterpret_cast<wl_proxy*>(manager));
    wl_proxy_destroy(reinterpret_cast<wl_proxy*>(duplicate));
}

class InvalidSourceTest : public ViewporterProtocolTest,
                          public testing::WithParamInterface<std::array<double, 4>>
{
};

TEST_P(InvalidSourceTest, BadValue)
{
    const auto p = GetParam();
    source(p[0], p[1], p[2], p[3]);
    error(WP_VIEWPORT_ERROR_BAD_VALUE);
}

INSTANTIATE_TEST_SUITE_P(Arguments, InvalidSourceTest,
                         testing::Values(std::array<double, 4> {-1, 0, 10, 10},
                                         std::array<double, 4> {0, -0.25, 10, 10},
                                         std::array<double, 4> {0, 0, 0, 1},
                                         std::array<double, 4> {0, 0, 1, -1},
                                         std::array<double, 4> {-1, -1, -1, 1}));

class InvalidDestinationTest : public ViewporterProtocolTest,
                               public testing::WithParamInterface<base::Size>
{
};

TEST_P(InvalidDestinationTest, BadValue)
{
    wp_viewport_set_destination(viewport, GetParam().width(), GetParam().height());
    error(WP_VIEWPORT_ERROR_BAD_VALUE);
}

INSTANTIATE_TEST_SUITE_P(Arguments, InvalidDestinationTest,
                         testing::Values(base::Size(-1, 3), base::Size(3, -1), base::Size(0, 3),
                                         base::Size(3, 0)));

TEST_F(ViewporterProtocolTest, BadSizeAtApplication)
{
    source(0.25, 0.5, 5.5, 8);
    ASSERT_GE(roundtrip(), 0);
    wl_surface_commit(surface);
    error(WP_VIEWPORT_ERROR_BAD_SIZE);
}

TEST_F(ViewporterProtocolTest, OutOfBufferAtApplication)
{
    attach(16, 8);
    source(10, 0, 7, 8);
    ASSERT_GE(roundtrip(), 0);
    wl_surface_commit(surface);
    error(WP_VIEWPORT_ERROR_OUT_OF_BUFFER);
}

TEST_F(ViewporterProtocolTest, EffectiveBufferScaleTransformAndViewportAreOneSnapshot)
{
    attach(4, 4);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    attach(400, 200);
    wl_surface_set_buffer_scale(surface, 2);
    wl_surface_set_buffer_transform(surface, WL_OUTPUT_TRANSFORM_90);
    source(25.25, 10.5, 74.75, 189.5);
    wp_viewport_set_destination(viewport, 200, 100);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(200, 100));
    source(25.25, 10.5, 75, 189.5);
    wl_surface_commit(surface);
    error(WP_VIEWPORT_ERROR_OUT_OF_BUFFER);
}

TEST_F(ViewporterProtocolTest, NullBufferHasNoOutOfBufferAndRemaps)
{
    source(100, 100, 10, 10);
    wp_viewport_set_destination(viewport, 50, 50);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->isMapped());
    EXPECT_TRUE(model->contentSize().isEmpty());
    attach(200, 200);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(50, 50));
    wl_surface_attach(surface, nullptr, 0, 0);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->isMapped());
}

TEST_F(ViewporterProtocolTest, DestructionAndRecreationAreCommitBuffered)
{
    attach(16, 8);
    source(1, 2, 10, 5);
    wp_viewport_set_destination(viewport, 30, 20);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    wp_viewport_destroy(viewport);
    viewport = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(30, 20));
    EXPECT_FALSE(model->hasViewport());
    viewport = wp_viewporter_get_viewport(manager, surface);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->crop());
    EXPECT_FALSE(model->viewport());
    EXPECT_EQ(model->contentSize(), base::Size(16, 8));
}

class NoSurfaceTest : public ViewporterProtocolTest, public testing::WithParamInterface<bool>
{
};

TEST_P(NoSurfaceTest, RequestsReportNoSurface)
{
    wl_surface_destroy(surface);
    surface = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(compositor.surfaces().empty());
    if (GetParam())
        source(-1, -1, -1, -1);
    else
        wp_viewport_set_destination(viewport, -1, -1);
    error(WP_VIEWPORT_ERROR_NO_SURFACE);
}

INSTANTIATE_TEST_SUITE_P(Requests, NoSurfaceTest, testing::Bool());

TEST_F(ViewporterProtocolTest, SynchronizedValidationWaitsForParent)
{
    auto* parent = wl_compositor_create_surface(clientCompositor);
    auto* sub = wl_subcompositor_get_subsurface(subcompositor, surface, parent);
    attach(16, 8);
    source(0, 0, 20, 8);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->contentSize().isEmpty());
    wl_surface_commit(parent);
    error(WP_VIEWPORT_ERROR_OUT_OF_BUFFER);
    wl_proxy_destroy(reinterpret_cast<wl_proxy*>(sub));
    wl_proxy_destroy(reinterpret_cast<wl_proxy*>(parent));
}

TEST_F(ViewporterProtocolTest, MultipleCachedCommitsAndViewportRecreation)
{
    auto* parent = wl_compositor_create_surface(clientCompositor);
    auto* sub = wl_subcompositor_get_subsurface(subcompositor, surface, parent);
    attach(16, 8);
    source(0, 0, 20, 8);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    source(1, 1, 10, 5);
    wp_viewport_set_destination(viewport, 30, 20);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->contentSize().isEmpty());
    wl_surface_commit(parent);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(30, 20));
    wp_viewport_destroy(viewport);
    viewport = wp_viewporter_get_viewport(manager, surface);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(30, 20));
    wl_surface_commit(parent);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(16, 8));
    wl_subsurface_set_desync(sub);
    wp_viewport_set_destination(viewport, 25, 11);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->contentSize(), base::Size(25, 11));
    wl_subsurface_destroy(sub);
    wl_surface_destroy(parent);
}

TEST_F(ViewporterProtocolTest, DisconnectWithCachedBufferAndLiveViewport)
{
    auto* parent = wl_compositor_create_surface(clientCompositor);
    auto* sub = wl_subcompositor_get_subsurface(subcompositor, surface, parent);
    attach(16, 8);
    wp_viewport_set_destination(viewport, 30, 20);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    wl_display_destroy_clients(server);
    EXPECT_TRUE(compositor.surfaces().empty());
    wl_proxy_destroy(reinterpret_cast<wl_proxy*>(sub));
    wl_proxy_destroy(reinterpret_cast<wl_proxy*>(parent));
}

} // namespace
