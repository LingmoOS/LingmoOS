// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/output_metrics_unittest.cc
//           chromium/src/components/exo/wayland/wayland_display_observer_unittest.cc
//           chromium/src/components/exo/wayland/zxdg_output_manager_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <fractional-scale-v1-client-protocol.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <thread>
#include <viewporter-client-protocol.h>
#include <wayland-client.h>
#include <wayland-server-core.h>
#include <xdg-output-unstable-v1-client-protocol.h>

#include "compositor/compositor.h"
#include "compositor/output.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/protocols/wp_fractional_scale.h"
#include "wayland/protocols/wp_viewporter.h"

using namespace Lingmo;

namespace {

class FractionalScaleProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    base::Subscription scaleChanged;
    wl_display *server = nullptr;
    wl_display *client = nullptr;
    wl_registry *registry = nullptr;
    wl_compositor *clientCompositor = nullptr;
    wp_viewporter *manager = nullptr;

    compositor::Output output;
    wl_global *outputGlobal = nullptr;
    wp_fractional_scale_manager_v1 *scaleManager = nullptr;
    wp_fractional_scale_v1 *fractional = nullptr;
    uint32_t outputName = 0;
    uint32_t xdgName = 0;
    std::vector<uint32_t> scales;
    wl_shm *shm = nullptr;
    wl_subcompositor *subcompositor = nullptr;
    wl_surface *surface = nullptr;
    wp_viewport *viewport = nullptr;
    wl_buffer *buffer = nullptr;
    compositor::Surface *model = nullptr;
    uint32_t advertisedVersion = 0;

    void SetUp() override
    {
        server = wl_display_create();
        ASSERT_TRUE(server);
        ASSERT_TRUE(wayland::createCompositorGlobal(server, &compositor));
        ASSERT_TRUE(wayland::createViewporterGlobal(server));
        ASSERT_TRUE(wayland::createFractionalScaleGlobal(server));
        ASSERT_TRUE(wayland::createXdgOutputManagerGlobal(server));
        output.pixelSize = base::Size(1920, 1080);
        output.physicalSizeMm = base::Size(509, 286);
        output.refreshMilliHz = 60000;
        outputGlobal = wayland::createOutputGlobal(server, &output);
        ASSERT_TRUE(outputGlobal);
        scene.setSize(output.logicalSize());
        scaleChanged = compositor.outputScaleChanged.connect([&] {
            output.scale = compositor.outputScale();
            scene.setSize(output.logicalSize());
            wayland::updateOutputGlobal(outputGlobal, &output);
        });
        ASSERT_TRUE(wayland::createShmGlobal(server, &compositor));
        ASSERT_TRUE(wayland::createSubcompositorGlobal(server));
        int sockets[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
        ASSERT_TRUE(wl_client_create(server, sockets[0])) << std::strerror(errno);
        client = wl_display_connect_to_fd(sockets[1]);
        ASSERT_TRUE(client);
        registry = wl_display_get_registry(client);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t version) {
                    auto *self = static_cast<FractionalScaleProtocolTest *>(data);
                    if (std::strcmp(interface, "wp_fractional_scale_manager_v1") == 0) {
                        EXPECT_EQ(version, 1u);
                        self->scaleManager = static_cast<wp_fractional_scale_manager_v1 *>(
                                wl_registry_bind(registry, name,
                                                 &wp_fractional_scale_manager_v1_interface, 1));
                    } else if (std::strcmp(interface, "wl_output") == 0) {
                        EXPECT_EQ(version, 4u);
                        self->outputName = name;
                    } else if (std::strcmp(interface, "zxdg_output_manager_v1") == 0) {
                        EXPECT_EQ(version, 3u);
                        self->xdgName = name;
                    } else if (std::strcmp(interface, "wl_compositor") == 0)
                        self->clientCompositor = static_cast<wl_compositor *>(
                                wl_registry_bind(registry, name, &wl_compositor_interface, 4));
                    else if (std::strcmp(interface, "wp_viewporter") == 0) {
                        self->advertisedVersion = version;
                        self->manager = static_cast<wp_viewporter *>(
                                wl_registry_bind(registry, name, &wp_viewporter_interface, 1));
                    } else if (std::strcmp(interface, "wl_shm") == 0) {
                        self->shm = static_cast<wl_shm *>(
                                wl_registry_bind(registry, name, &wl_shm_interface, 1));
                    } else if (std::strcmp(interface, "wl_subcompositor") == 0) {
                        self->subcompositor = static_cast<wl_subcompositor *>(
                                wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
                    }
                },
                [](void *, wl_registry *, uint32_t) {}};
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
        createFractional();
        ASSERT_GE(roundtrip(), 0);
    }

    void createFractional()
    {
        fractional = wp_fractional_scale_manager_v1_get_fractional_scale(scaleManager, surface);
        static const wp_fractional_scale_v1_listener listener = {
                [](void *data, wp_fractional_scale_v1 *, uint32_t scale) {
                    static_cast<FractionalScaleProtocolTest *>(data)->scales.push_back(scale);
                }};
        wp_fractional_scale_v1_add_listener(fractional, &listener, this);
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
        auto *pool = wl_shm_create_pool(shm, fd, width * height * 4);
        close(fd);
        buffer = wl_shm_pool_create_buffer(pool, 0, width, height, width * 4,
                                           WL_SHM_FORMAT_XRGB8888);
        wl_shm_pool_destroy(pool);
        wl_surface_attach(surface, buffer, 0, 0);
    }

    void error(uint32_t code, const char *name = "wp_viewport", wl_proxy *object = nullptr)
    {
        const uint32_t expectedId =
                wl_proxy_get_id(object ? object : reinterpret_cast<wl_proxy *>(viewport));
        EXPECT_LT(roundtrip(), 0);
        const wl_interface *interface = nullptr;
        uint32_t id = 0;
        EXPECT_EQ(wl_display_get_protocol_error(client, &interface, &id), code);
        ASSERT_TRUE(interface);
        EXPECT_STREQ(interface->name, name);
        EXPECT_EQ(id, expectedId);
    }

    void TearDown() override
    {
        if (fractional)
            wp_fractional_scale_v1_destroy(fractional);
        if (scaleManager)
            wp_fractional_scale_manager_v1_destroy(scaleManager);
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

TEST_F(FractionalScaleProtocolTest, InitialUnmappedAndTransitionsPreserveClientState)
{
    EXPECT_EQ(scales, std::vector<uint32_t>({120}));
    EXPECT_FALSE(model->hasRole());
    attach(1202, 902);
    wp_viewport_set_destination(viewport, 801, 601);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    const auto mapping = model->textureMapping();
    for (uint32_t value : {180u, 210u, 240u, 120u}) {
        compositor.setOutputScale(*compositor::OutputScale::fromNumerator(value));
        ASSERT_GE(roundtrip(), 0);
        EXPECT_EQ(scales.back(), value);
        EXPECT_EQ(model->contentSize(), base::Size(801, 601));
        EXPECT_EQ(model->textureMapping(), mapping);
        const size_t count = scales.size();
        compositor.setOutputScale(*compositor::OutputScale::fromNumerator(value));
        ASSERT_GE(roundtrip(), 0);
        EXPECT_EQ(scales.size(), count);
        EXPECT_EQ(output.pixelSize, base::Size(1920, 1080));
    }
}

TEST_F(FractionalScaleProtocolTest, LiveOutputViewsRefreshUnmappedAndMovedSurfaces)
{
    const auto scale = *compositor::OutputScale::parse("1.5");
    compositor.setOutputViews({{"display", {0, 0, 1280, 720}, scale}});
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(scales.back(), 180u);

    scene.root()->setChangeCallback([&] {
        compositor.refreshOutputScales();
    });
    attach(100, 100);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    model->setPosition({1500, 100});
    compositor.setOutputViews(
            {{"display", {0, 0, 1280, 720}, scale}, {"second", {1280, 0, 1920, 1080}, {}}});
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(scales.back(), 120u);
    model->setPosition({100, 100});
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(scales.back(), 180u);
    const auto count = scales.size();
    model->setPosition({110, 110});
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(scales.size(), count);
    scene.root()->setChangeCallback({});
}

TEST_F(FractionalScaleProtocolTest, ManagerExtensionAndSurfaceLifetimes)
{
    wp_fractional_scale_v1_destroy(fractional);
    fractional = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(model->hasFractionalScale());
    compositor.setOutputScale(*compositor::OutputScale::parse("1.5"));
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(scales.size(), 1u);
    createFractional();
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(scales.back(), 180u);
    wp_fractional_scale_manager_v1_destroy(scaleManager);
    scaleManager = nullptr;
    compositor.setOutputScale(*compositor::OutputScale::parse("2"));
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(scales.back(), 240u);
    wl_surface_destroy(surface);
    surface = nullptr;
    ASSERT_GE(roundtrip(), 0);
    compositor.setOutputScale(compositor::OutputScale());
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(scales.size(), 3u);
}

TEST_F(FractionalScaleProtocolTest, DuplicateErrorOnManager)
{
    auto *duplicate = wp_fractional_scale_manager_v1_get_fractional_scale(scaleManager, surface);
    error(WP_FRACTIONAL_SCALE_MANAGER_V1_ERROR_FRACTIONAL_SCALE_EXISTS,
          "wp_fractional_scale_manager_v1", reinterpret_cast<wl_proxy *>(scaleManager));
    wp_fractional_scale_v1_destroy(duplicate);
}

struct Metrics {
    base::Size native;
    base::Size logical;
    int scale = 0;
    int outputDone = 0;
    int xdgDone = 0;
    int names = 0;
    int descriptions = 0;
};

const wl_output_listener outputListener = {
        [](void *, wl_output *, int32_t, int32_t, int32_t, int32_t, int32_t, const char *,
           const char *, int32_t) {},
        [](void *data, wl_output *, uint32_t, int32_t w, int32_t h, int32_t) {
            static_cast<Metrics *>(data)->native = base::Size(w, h);
        },
        [](void *data, wl_output *) {
            ++static_cast<Metrics *>(data)->outputDone;
        },
        [](void *data, wl_output *, int32_t scale) {
            static_cast<Metrics *>(data)->scale = scale;
        },
        [](void *, wl_output *, const char *) {},
        [](void *, wl_output *, const char *) {}};

const zxdg_output_v1_listener xdgListener = {
        [](void *, zxdg_output_v1 *, int32_t x, int32_t y) {
            EXPECT_EQ(x, 0);
            EXPECT_EQ(y, 0);
        },
        [](void *data, zxdg_output_v1 *, int32_t w, int32_t h) {
            static_cast<Metrics *>(data)->logical = base::Size(w, h);
        },
        [](void *data, zxdg_output_v1 *) {
            ++static_cast<Metrics *>(data)->xdgDone;
        },
        [](void *data, zxdg_output_v1 *, const char *) {
            ++static_cast<Metrics *>(data)->names;
        },
        [](void *data, zxdg_output_v1 *, const char *) {
            ++static_cast<Metrics *>(data)->descriptions;
        }};

TEST_F(FractionalScaleProtocolTest, OutputMetricsVersionsAndMultipleExtensions)
{
    for (uint32_t version : {1u, 2u, 3u}) {
        Metrics a, b;
        auto *out = static_cast<wl_output *>(
                wl_registry_bind(registry, outputName, &wl_output_interface, 3));
        wl_output_add_listener(out, &outputListener, &a);
        auto *manager = static_cast<zxdg_output_manager_v1 *>(
                wl_registry_bind(registry, xdgName, &zxdg_output_manager_v1_interface, version));
        auto *first = zxdg_output_manager_v1_get_xdg_output(manager, out);
        auto *second = zxdg_output_manager_v1_get_xdg_output(manager, out);
        zxdg_output_v1_add_listener(first, &xdgListener, &a);
        zxdg_output_v1_add_listener(second, &xdgListener, &b);
        zxdg_output_manager_v1_destroy(manager);
        ASSERT_GE(roundtrip(), 0);
        EXPECT_EQ(a.native, base::Size(1920, 1080));
        EXPECT_EQ(a.names, version >= 2 ? 1 : 0);
        EXPECT_EQ(a.descriptions, version >= 2 ? 1 : 0);
        EXPECT_EQ(a.xdgDone, version < 3 ? 1 : 0);
        compositor.setOutputScale(*compositor::OutputScale::parse("1.5"));
        ASSERT_GE(roundtrip(), 0);
        EXPECT_EQ(a.logical, base::Size(1280, 720));
        EXPECT_EQ(b.logical, a.logical);
        EXPECT_EQ(a.scale, 2);
        EXPECT_EQ(a.names, version >= 2 ? 1 : 0);
        EXPECT_EQ(a.descriptions, version >= 2 ? 1 : 0);
        EXPECT_EQ(a.xdgDone, version < 3 ? 2 : 0);
        EXPECT_GE(a.outputDone, 4);
        wl_output_release(out);
        ASSERT_GE(roundtrip(), 0);
        compositor.setOutputScale(compositor::OutputScale());
        ASSERT_GE(roundtrip(), 0);
        zxdg_output_v1_destroy(first);
        zxdg_output_v1_destroy(second);
        ASSERT_GE(roundtrip(), 0);
    }
}

// Exo's DuplicateGetXdgOutputDoesNotUAF: a client dying with two xdg_outputs
// on one wl_output tears them down after the wl_output (lower object id).
TEST_F(FractionalScaleProtocolTest, DisconnectingWithDuplicateXdgOutputsIsSafe)
{
    for (bool withdrawFirst : {false, true}) {
        int sockets[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
        wl_client *serverClient = wl_client_create(server, sockets[0]);
        ASSERT_TRUE(serverClient);
        wl_display *other = wl_display_connect_to_fd(sockets[1]);
        ASSERT_TRUE(other);
        const auto otherRoundtrip = [&] {
            std::atomic<bool> done = false;
            int result = -1;
            std::thread worker([&] {
                result = wl_display_roundtrip(other);
                done = true;
            });
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
            while (!done && std::chrono::steady_clock::now() < deadline) {
                wl_event_loop_dispatch(wl_display_get_event_loop(server), 1);
                wl_display_flush_clients(server);
            }
            if (!done)
                shutdown(wl_display_get_fd(other), SHUT_RDWR);
            worker.join();
            return result;
        };

        auto *otherRegistry = wl_display_get_registry(other);
        auto *out = static_cast<wl_output *>(
                wl_registry_bind(otherRegistry, outputName, &wl_output_interface, 4));
        auto *manager = static_cast<zxdg_output_manager_v1 *>(
                wl_registry_bind(otherRegistry, xdgName, &zxdg_output_manager_v1_interface, 3));
        auto *first = zxdg_output_manager_v1_get_xdg_output(manager, out);
        auto *second = zxdg_output_manager_v1_get_xdg_output(manager, out);
        ASSERT_GE(otherRoundtrip(), 0);
        EXPECT_EQ(wl_display_get_error(other), 0);

        wl_global *replacement = nullptr;
        if (withdrawFirst) {
            wayland::removeOutputGlobal(outputGlobal);
            replacement = wayland::createOutputGlobal(server, &output);
            ASSERT_TRUE(replacement);
            ASSERT_GE(otherRoundtrip(), 0);
        }

        bool destroyed = false;
        struct Listener {
            wl_listener listener;
            bool *destroyed;
        } listener {{}, &destroyed};
        listener.listener.notify = [](wl_listener *listener, void *) {
            *reinterpret_cast<Listener *>(listener)->destroyed = true;
        };
        wl_client_add_destroy_listener(serverClient, &listener.listener);

        // Local proxy destruction only: nothing reaches the server.
        for (auto *proxy : {reinterpret_cast<wl_proxy *>(first), reinterpret_cast<wl_proxy *>(second),
                            reinterpret_cast<wl_proxy *>(manager), reinterpret_cast<wl_proxy *>(out),
                            reinterpret_cast<wl_proxy *>(otherRegistry)})
            wl_proxy_destroy(proxy);
        wl_display_disconnect(other);
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (!destroyed && std::chrono::steady_clock::now() < deadline)
            wl_event_loop_dispatch(wl_display_get_event_loop(server), 1);
        EXPECT_TRUE(destroyed);

        if (replacement)
            outputGlobal = replacement;
        // The output and its other bindings carry on.
        wayland::updateOutputGlobal(outputGlobal, &output);
        ASSERT_GE(roundtrip(), 0);
    }
}

TEST_F(FractionalScaleProtocolTest, LateOutputBindingEnterLeaveAndScaleDoesNotCycle)
{
    int enters = 0, leaves = 0;
    std::pair<int *, int *> events {&enters, &leaves};
    static const wl_surface_listener listener = {
            [](void *data, wl_surface *, wl_output *) {
                ++*static_cast<std::pair<int *, int *> *>(data)->first;
            },
            [](void *data, wl_surface *, wl_output *) {
                ++*static_cast<std::pair<int *, int *> *>(data)->second;
            },
            [](void *, wl_surface *, int32_t) {}, [](void *, wl_surface *, uint32_t) {}};
    wl_surface_add_listener(surface, &listener, &events);
    attach(100, 80);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    Metrics metrics;
    auto *out = static_cast<wl_output *>(
            wl_registry_bind(registry, outputName, &wl_output_interface, 2));
    wl_output_add_listener(out, &outputListener, &metrics);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(enters, 1);
    compositor.setOutputScale(*compositor::OutputScale::parse("1.5"));
    wayland::updateOutputMembership(outputGlobal);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(enters, 1);
    EXPECT_EQ(leaves, 0);
    wl_surface_attach(surface, nullptr, 0, 0);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    wayland::updateOutputMembership(outputGlobal);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(leaves, 1);
    wl_output_destroy(out);
}

TEST_F(FractionalScaleProtocolTest, EnterAndLeaveFollowTheSurfacesThatMoved)
{
    int enters = 0, leaves = 0;
    std::pair<int *, int *> events {&enters, &leaves};
    static const wl_surface_listener listener = {
            [](void *data, wl_surface *, wl_output *) {
                ++*static_cast<std::pair<int *, int *> *>(data)->first;
            },
            [](void *data, wl_surface *, wl_output *) {
                ++*static_cast<std::pair<int *, int *> *>(data)->second;
            },
            [](void *, wl_surface *, int32_t) {}, [](void *, wl_surface *, uint32_t) {}};
    wl_surface_add_listener(surface, &listener, &events);
    attach(100, 80);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    Metrics metrics;
    auto *out = static_cast<wl_output *>(
            wl_registry_bind(registry, outputName, &wl_output_interface, 2));
    wl_output_add_listener(out, &outputListener, &metrics);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(enters, 1);
    compositor.takeMovedSurfaces();
    attach(100, 80);
    wl_surface_commit(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(compositor.takeMovedSurfaces().empty());
    model->node()->setPosition({5000, 0});
    auto moved = compositor.takeMovedSurfaces();
    ASSERT_EQ(moved.size(), 1u);
    wayland::updateOutputMembership(outputGlobal, moved);
    wayland::updateOutputMembership(outputGlobal, moved);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(leaves, 1);
    model->node()->setPosition({0, 0});
    moved = compositor.takeMovedSurfaces();
    wayland::updateOutputMembership(outputGlobal, moved);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(enters, 2);
    // A full pass finds nothing left to announce.
    wayland::updateOutputMembership(outputGlobal);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(enters, 2);
    EXPECT_EQ(leaves, 1);
    wl_output_destroy(out);
}

TEST_F(FractionalScaleProtocolTest, LegacyDensitiesRemainValid)
{
    wp_viewport_destroy(viewport);
    viewport = nullptr;
    compositor.setOutputScale(*compositor::OutputScale::parse("1.5"));
    for (int bufferScale : {1, 2, 1}) {
        attach(400 * bufferScale, 300 * bufferScale);
        wl_surface_set_buffer_scale(surface, bufferScale);
        wl_surface_commit(surface);
        ASSERT_GE(roundtrip(), 0);
        EXPECT_EQ(model->contentSize(), base::Size(400, 300));
        EXPECT_FALSE(model->viewport());
        EXPECT_EQ(model->preferredScale(), 180u);
    }
}

TEST_F(FractionalScaleProtocolTest, CoreOutputVersionOneNeverReceivesScaleOrDone)
{
    Metrics metrics;
    auto *out = static_cast<wl_output *>(
            wl_registry_bind(registry, outputName, &wl_output_interface, 1));
    wl_output_add_listener(out, &outputListener, &metrics);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(metrics.native, base::Size(1920, 1080));
    compositor.setOutputScale(*compositor::OutputScale::parse("1.75"));
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(metrics.scale, 0);
    EXPECT_EQ(metrics.outputDone, 0);
    wl_output_destroy(out);
}

TEST_F(FractionalScaleProtocolTest, SecondClientReceivesIndependentBoundResourceUpdates)
{
    int sockets[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
    ASSERT_TRUE(wl_client_create(server, sockets[0]));
    auto *firstClient = client;
    client = wl_display_connect_to_fd(sockets[1]);
    ASSERT_TRUE(client);
    auto *secondRegistry = wl_display_get_registry(client);
    Metrics metrics;
    auto *out = static_cast<wl_output *>(
            wl_registry_bind(secondRegistry, outputName, &wl_output_interface, 3));
    wl_output_add_listener(out, &outputListener, &metrics);
    ASSERT_GE(roundtrip(), 0);
    compositor.setOutputScale(*compositor::OutputScale::parse("1.5"));
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(metrics.scale, 2);
    EXPECT_EQ(metrics.native, base::Size(1920, 1080));
    wl_output_release(out);
    wl_registry_destroy(secondRegistry);
    ASSERT_GE(roundtrip(), 0);
    wl_display_disconnect(client);
    client = firstClient;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(scales.back(), 180u);
}

TEST_F(FractionalScaleProtocolTest, ChildCreatedBeforeParentRoleGetsSameOutputRecommendation)
{
    auto *child = wl_compositor_create_surface(clientCompositor);
    auto *extension = wp_fractional_scale_manager_v1_get_fractional_scale(scaleManager, child);
    std::vector<uint32_t> childScales;
    static const wp_fractional_scale_v1_listener listener = {
            [](void *data, wp_fractional_scale_v1 *, uint32_t scale) {
                static_cast<std::vector<uint32_t> *>(data)->push_back(scale);
            }};
    wp_fractional_scale_v1_add_listener(extension, &listener, &childScales);
    auto *childViewport = wp_viewporter_get_viewport(manager, child);
    auto *sub = wl_subcompositor_get_subsurface(subcompositor, child, surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(childScales, std::vector<uint32_t>({120}));
    compositor.setOutputScale(*compositor::OutputScale::parse("1.5"));
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(childScales, std::vector<uint32_t>({120, 180}));
    wp_fractional_scale_v1_destroy(extension);
    wp_viewport_destroy(childViewport);
    wl_subsurface_destroy(sub);
    wl_surface_destroy(child);
    ASSERT_GE(roundtrip(), 0);
}

} // namespace
