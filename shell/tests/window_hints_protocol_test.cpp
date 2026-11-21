// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <atomic>
#include <chrono>
#include <cstring>
#include <lingmo-window-hints-v1-client-protocol.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <thread>
#include <vector>
#include <wayland-client.h>
#include <wayland-server-core.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "scene/scene.h"
#include "wayland/protocols/window_hints.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/server_util.h"

using namespace Lingmo;

namespace {

class WindowHintsProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    wl_display *server = nullptr;
    wl_display *client = nullptr;
    wl_registry *registry = nullptr;
    wl_compositor *clientCompositor = nullptr;
    lingmo_window_hints_v1 *hints = nullptr;

    void SetUp() override
    {
        server = wl_display_create();
        ASSERT_TRUE(server);
        ASSERT_TRUE(wayland::createCompositorGlobal(server, &compositor));
        ASSERT_TRUE(wayland::createWindowHintsGlobal(server));
        int sockets[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
        ASSERT_TRUE(wl_client_create(server, sockets[0])) << std::strerror(errno);
        client = wl_display_connect_to_fd(sockets[1]);
        ASSERT_TRUE(client);
        registry = wl_display_get_registry(client);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t) {
                    auto *self = static_cast<WindowHintsProtocolTest *>(data);
                    if (std::strcmp(interface, "wl_compositor") == 0)
                        self->clientCompositor = static_cast<wl_compositor *>(
                                wl_registry_bind(registry, name, &wl_compositor_interface, 4));
                    else if (std::strcmp(interface, "lingmo_window_hints_v1") == 0)
                        self->hints = static_cast<lingmo_window_hints_v1 *>(wl_registry_bind(
                                registry, name, &lingmo_window_hints_v1_interface, 1));
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        ASSERT_GE(roundtrip(), 0);
        ASSERT_TRUE(hints);
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
        if (hints)
            lingmo_window_hints_v1_destroy(hints);
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

TEST_F(WindowHintsProtocolTest, MetadataIsImmediateAndPersistsUntilTheSurfaceDies)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_FALSE(model->skipTaskbar());
    lingmo_window_hints_v1_set_skip_taskbar(hints, surface, 1);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->skipTaskbar());
    lingmo_window_hints_v1_set_skip_taskbar(hints, surface, 0);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->skipTaskbar());
    lingmo_window_hints_v1_set_skip_taskbar(hints, surface, 1);
    lingmo_window_hints_v1_destroy(hints);
    hints = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->skipTaskbar());
    wl_surface_destroy(surface);
    surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    ASSERT_EQ(compositor.surfaces().size(), 1u);
    EXPECT_FALSE(compositor.surfaces().front()->skipTaskbar());
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, AlwaysOnTopIsImmediateAndPersistsUntilTheSurfaceDies)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_FALSE(model->alwaysOnTop());
    lingmo_window_hints_v1_set_always_on_top(hints, surface, 1);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->alwaysOnTop());
    lingmo_window_hints_v1_set_always_on_top(hints, surface, 0);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->alwaysOnTop());
    lingmo_window_hints_v1_set_always_on_top(hints, surface, 1);
    lingmo_window_hints_v1_destroy(hints);
    hints = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->alwaysOnTop());
    wl_surface_destroy(surface);
    surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    ASSERT_EQ(compositor.surfaces().size(), 1u);
    EXPECT_FALSE(compositor.surfaces().front()->alwaysOnTop());
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, InvalidAlwaysOnTopIsAProtocolError)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    lingmo_window_hints_v1_set_always_on_top(hints, surface, 2);
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, ResizeAnchorIsImmediateAndPersistsUntilTheSurfaceDies)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_EQ(model->resizeAnchor(), compositor::ResizeAnchor::TopLeft);
    lingmo_window_hints_v1_set_resize_anchor(hints, surface,
                                               LINGMO_WINDOW_HINTS_V1_RESIZE_ANCHOR_CENTER);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->resizeAnchor(), compositor::ResizeAnchor::Center);
    lingmo_window_hints_v1_destroy(hints);
    hints = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->resizeAnchor(), compositor::ResizeAnchor::Center);
    wl_surface_destroy(surface);
    surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    ASSERT_EQ(compositor.surfaces().size(), 1u);
    EXPECT_EQ(compositor.surfaces().front()->resizeAnchor(), compositor::ResizeAnchor::TopLeft);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, UnknownResizeAnchorIsAProtocolError)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    lingmo_window_hints_v1_set_resize_anchor(hints, surface, 2);
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, ServerResizeIsImmediateAndRejectsOtherValues)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_FALSE(model->serverResize());
    lingmo_window_hints_v1_set_server_resize(hints, surface, 1);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->serverResize());
    lingmo_window_hints_v1_set_server_resize(hints, surface, 2);
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, RestoreIdIsImmediateAndPersistsUntilTheSurfaceDies)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_TRUE(model->restoreId().empty());
    lingmo_window_hints_v1_set_restore_id(hints, surface, "preview");
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->restoreId(), "preview");
    lingmo_window_hints_v1_set_restore_id(hints, surface, "");
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->restoreId().empty());
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, PlacementAnchorIsImmediateAndCanBeUnset)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_FALSE(model->placementAnchor());
    lingmo_window_hints_v1_set_placement_anchor(hints, surface, 700, 12, 0, 0,
                                                  LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_BOTTOM,
                                                  LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_BOTTOM);
    ASSERT_GE(roundtrip(), 0);
    ASSERT_TRUE(model->placementAnchor());
    EXPECT_EQ(*model->placementAnchor(),
              (Lingmo::compositor::PlacementAnchor {Lingmo::base::Rect(700, 12, 0, 0),
                                                      Lingmo::compositor::PlacementEdge::Bottom,
                                                      Lingmo::compositor::PlacementEdge::Bottom}));
    lingmo_window_hints_v1_unset_placement_anchor(hints, surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->placementAnchor());
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, BodyShapeIsImmediateAndCanBeUnset)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    // A triangle: move_to, line_to, line_to, close.
    std::vector<int32_t> values = {0, wl_fixed_from_int(10), 0, 1, wl_fixed_from_int(20), wl_fixed_from_int(10),
                                   1, 0, wl_fixed_from_int(10), 3};
    wl_array array;
    wl_array_init(&array);
    auto *data = static_cast<int32_t *>(wl_array_add(&array, values.size() * sizeof(int32_t)));
    std::copy(values.begin(), values.end(), data);
    lingmo_window_hints_v1_set_body_shape(hints, surface, &array);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->bodyShape().isEmpty());
    EXPECT_EQ(model->bodyShape().boundingRect(), (Lingmo::ui::RectF {0, 0, 20, 10}));
    lingmo_window_hints_v1_unset_body_shape(hints, surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->bodyShape().isEmpty());

    // An unknown command.
    values = {7};
    wl_array_release(&array);
    wl_array_init(&array);
    data = static_cast<int32_t *>(wl_array_add(&array, sizeof(int32_t)));
    *data = 7;
    lingmo_window_hints_v1_set_body_shape(hints, surface, &array);
    wl_array_release(&array);
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr), LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, ShadowIsImmediateAndRejectsOtherValues)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_TRUE(model->castsShadow());
    lingmo_window_hints_v1_set_shadow(hints, surface, 0);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->castsShadow());
    lingmo_window_hints_v1_set_shadow(hints, surface, 1);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(model->castsShadow());
    lingmo_window_hints_v1_set_shadow(hints, surface, 2);
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, PlacementAnchorRejectsANegativeSize)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    lingmo_window_hints_v1_set_placement_anchor(hints, surface, 0, 0, -1, 0,
                                                  LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_NONE,
                                                  LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_NONE);
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, PlacementAnchorRejectsAnUnknownEdge)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    lingmo_window_hints_v1_set_placement_anchor(hints, surface, 0, 0, 0, 0, 9,
                                                  LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_NONE);
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, RestoreSizeIsImmediateAndRejectsNegativeValues)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_TRUE(model->restoreSize().isEmpty());
    lingmo_window_hints_v1_set_restore_size(hints, surface, 300, 200);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->restoreSize(), Lingmo::base::Size(300, 200));
    lingmo_window_hints_v1_set_restore_size(hints, surface, -1, 200);
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, InteractiveResizeReachesTheSurfacesClient)
{
    struct Event {
        wl_surface *surface;
        uint32_t active;
    };
    std::vector<Event> events;
    static const lingmo_window_hints_v1_listener listener = {
            [](void *data, lingmo_window_hints_v1 *, wl_surface *surface, uint32_t active) {
                static_cast<std::vector<Event> *>(data)->push_back({surface, active});
            }};
    lingmo_window_hints_v1_add_listener(hints, &listener, &events);
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    wl_resource *resource = wayland::getSurfaceResource(compositor.surfaces().front());
    ASSERT_TRUE(resource);

    wayland::sendInteractiveResize(resource, true);
    wayland::sendInteractiveResize(resource, false);
    ASSERT_GE(roundtrip(), 0);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].surface, surface);
    EXPECT_EQ(events[0].active, 1u);
    EXPECT_EQ(events[1].active, 0u);

    // A destroyed manager hears nothing more.
    lingmo_window_hints_v1_destroy(hints);
    hints = nullptr;
    ASSERT_GE(roundtrip(), 0);
    wayland::sendInteractiveResize(resource, true);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(events.size(), 2u);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, OpacityIsImmediateAndBetweenZeroAndOne)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    ASSERT_GE(roundtrip(), 0);
    auto *model = compositor.surfaces().front();
    EXPECT_EQ(model->opacity(), 1.0f);
    lingmo_window_hints_v1_set_opacity(hints, surface, wl_fixed_from_double(0.5));
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(model->opacity(), 0.5f);
    lingmo_window_hints_v1_set_opacity(hints, surface, wl_fixed_from_double(1.5));
    EXPECT_LT(roundtrip(), 0);
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    EXPECT_EQ(model->opacity(), 0.5f);
    wl_surface_destroy(surface);
}

TEST_F(WindowHintsProtocolTest, InvalidBooleanIsAProtocolError)
{
    auto *surface = wl_compositor_create_surface(clientCompositor);
    lingmo_window_hints_v1_set_skip_taskbar(hints, surface, 2);
    EXPECT_LT(roundtrip(), 0);
    const wl_interface *interface = nullptr;
    uint32_t id = 0;
    EXPECT_EQ(wl_display_get_protocol_error(client, &interface, &id),
              LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE);
    ASSERT_NE(interface, nullptr);
    EXPECT_STREQ(interface->name, "lingmo_window_hints_v1");
    wl_surface_destroy(surface);
}

} // namespace
