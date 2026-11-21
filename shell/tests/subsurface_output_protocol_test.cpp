// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_display_output_unittest.cc
//           chromium/src/components/exo/sub_surface_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: exercises the corresponding behavior with a real
// libwayland client and Lingmo compositor objects.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <future>
#include <gtest/gtest.h>
#include <map>
#include <mutex>
#include <sys/socket.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>
#include <wayland-server-core.h>

#include "compositor/compositor.h"
#include "compositor/output.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "loop_test_util.h"
#include "scene/scene.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/server.h"

using namespace Lingmo;

namespace {

struct Client {
    wl_display* display;
    wl_registry* registry;
    wl_compositor* compositor = nullptr;
    wl_subcompositor* subcompositor = nullptr;
    std::map<std::string, std::pair<uint32_t, uint32_t>> globals;
    std::vector<uint32_t> removedGlobals;

    explicit Client(int fd)
        : display(wl_display_connect_to_fd(fd))
    {
        registry = wl_display_get_registry(display);
        static const wl_registry_listener listener = {
            [](void* data, wl_registry*, uint32_t name, const char* interface, uint32_t version) {
                static_cast<Client*>(data)->globals[interface] = {name, version};
            },
            [](void* data, wl_registry*, uint32_t name) {
                static_cast<Client*>(data)->removedGlobals.push_back(name);
            }};
        wl_registry_add_listener(registry, &listener, this);
        wl_display_roundtrip(display);
        compositor = static_cast<wl_compositor*>(wl_registry_bind(
            registry, globals["wl_compositor"].first, &wl_compositor_interface, 4));
        subcompositor = static_cast<wl_subcompositor*>(wl_registry_bind(
            registry, globals["wl_subcompositor"].first, &wl_subcompositor_interface, 1));
    }

    ~Client()
    {
        wl_subcompositor_destroy(subcompositor);
        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }

    void sync() { ASSERT_GE(wl_display_roundtrip(display), 0); }
};

class SubsurfaceOutputProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    compositor::Output output {{13, 17},
                               {510, 290},
                               {1920, 1080},
                               "ACME",
                               "DP-7",
                               59940,
                               true,
                               compositor::Output::Subpixel::HorizontalRgb};
    wayland::Server server;
    wl_global* outputGlobal = nullptr;
    std::mutex mutex;
    std::function<void()> pendingCheck;

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

    void run(std::function<void(Client&)> body)
    {
        ASSERT_TRUE(wayland::createCompositorGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSubcompositorGlobal(server.display()));
        ASSERT_TRUE(wayland::createShmGlobal(server.display(), &compositor));
        outputGlobal = wayland::createOutputGlobal(server.display(), &output);
        ASSERT_TRUE(outputGlobal);
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
    }
};

TEST_F(SubsurfaceOutputProtocolTest, RegistryHasOnlySurfaceAndOutputGlobals)
{
    run([](Client& c) {
        EXPECT_EQ(4u, c.globals.size());
        EXPECT_EQ(4u, c.globals["wl_compositor"].second);
        EXPECT_EQ(1u, c.globals["wl_subcompositor"].second);
        EXPECT_EQ(1u, c.globals["wl_shm"].second);
        EXPECT_EQ(4u, c.globals["wl_output"].second);
        EXPECT_FALSE(c.globals.contains("wl_seat"));
        EXPECT_FALSE(c.globals.contains("xdg_wm_base"));
    });
}

TEST_F(SubsurfaceOutputProtocolTest, SyncDesyncPositionStackingAndInertResources)
{
    run([&](Client& c) {
        auto* p = wl_compositor_create_surface(c.compositor);
        auto* a = wl_compositor_create_surface(c.compositor);
        auto* b = wl_compositor_create_surface(c.compositor);
        auto* sa = wl_subcompositor_get_subsurface(c.subcompositor, a, p);
        auto* sb = wl_subcompositor_get_subsurface(c.subcompositor, b, a);
        wl_subsurface_set_desync(sb);
        wl_surface_set_buffer_scale(b, 3);
        wl_surface_commit(b);
        wl_subsurface_set_position(sa, 30, 40);
        wl_surface_commit(a);
        c.sync();
        check([&] {
            EXPECT_EQ(1, compositor.surfaces()[2]->bufferScale());
            EXPECT_EQ(base::Point(), compositor.surfaces()[1]->position());
        });
        wl_surface_commit(p);
        c.sync();
        check([&] {
            EXPECT_EQ(3, compositor.surfaces()[2]->bufferScale());
            EXPECT_EQ(base::Point(30, 40), compositor.surfaces()[1]->position());
        });
        wl_surface_set_buffer_scale(a, 2);
        wl_surface_commit(a);
        c.sync();
        check([&] {
            EXPECT_EQ(1, compositor.surfaces()[1]->bufferScale());
        });
        wl_subsurface_set_desync(sa);
        c.sync();
        check([&] {
            EXPECT_EQ(2, compositor.surfaces()[1]->bufferScale());
        });
        wl_subsurface_set_sync(sa);
        wl_surface_set_buffer_scale(b, 4);
        wl_surface_commit(b);
        c.sync();
        check([&] {
            EXPECT_EQ(3, compositor.surfaces()[2]->bufferScale());
        });
        wl_subsurface_place_below(sa, p);
        wl_surface_commit(p);
        c.sync();
        check([&] {
            EXPECT_EQ(4, compositor.surfaces()[2]->bufferScale());
        });
        wl_subsurface_place_above(sa, p);
        wl_surface_commit(p);
        c.sync();
        wl_surface_destroy(p);
        c.sync();
        wl_subsurface_set_position(sa, 1, 2);
        wl_subsurface_set_desync(sa);
        c.sync();
        wl_surface_destroy(a);
        wl_subsurface_set_sync(sa);
        wl_subsurface_destroy(sa);
        wl_surface_destroy(b);
        wl_subsurface_destroy(sb);
        c.sync();
    });
}

// Exercise the exact hardware-demo binary without a DRM dependency. This is
// protocol/render-path coverage, not a substitute for the real-VT acceptance.
TEST_F(SubsurfaceOutputProtocolTest, DemoRunsAgainstPresentationDrivenFakeOutput)
{
    ASSERT_TRUE(wayland::createCompositorGlobal(server.display(), &compositor));
    ASSERT_TRUE(wayland::createSubcompositorGlobal(server.display()));
    ASSERT_TRUE(wayland::createShmGlobal(server.display(), &compositor));
    ASSERT_TRUE(wayland::createOutputGlobal(server.display(), &output));
    int fds[2];
    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    pid_t pid = fork();
    ASSERT_GE(pid, 0);
    if (pid == 0) {
        close(fds[0]);
        auto fd = std::to_string(fds[1]);
        setenv("WAYLAND_SOCKET", fd.c_str(), 1);
        execl(SUBSURFACE_DEMO_PATH, SUBSURFACE_DEMO_PATH, nullptr);
        _exit(127);
    }
    close(fds[1]);
    ASSERT_TRUE(wl_client_create(server.display(), fds[0]));
    testsupport::FakeRenderer renderer;
    const auto start = std::chrono::steady_clock::now();
    int status = 0;
    bool exited = false;
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(10)) {
        if (waitpid(pid, &status, WNOHANG) == pid) {
            exited = true;
            break;
        }
        server.dispatch(std::chrono::milliseconds(5));
        compositor.updateTextures(renderer);
        scene.root()->render(renderer);
        compositor.frameSubmitted();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - start)
                      .count();
        compositor.frameDrawn(ms);
        server.flush();
        renderer.calls.clear();
    }
    if (!exited) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
    }
    ASSERT_TRUE(exited);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(0, WEXITSTATUS(status));
    wl_display_destroy_clients(server.display());
}

class InvalidSubsurfaceTest : public SubsurfaceOutputProtocolTest,
                              public testing::WithParamInterface<int>
{
};

TEST_P(InvalidSubsurfaceTest, PostsSpecificProtocolError)
{
    run([&](Client& c) {
        auto* p = wl_compositor_create_surface(c.compositor);
        auto* a = wl_compositor_create_surface(c.compositor);
        auto* b = wl_compositor_create_surface(c.compositor);
        auto* sa = wl_subcompositor_get_subsurface(c.subcompositor, a, p);
        wl_subsurface* invalid = nullptr;
        wl_subsurface* grand = nullptr;
        const int test = GetParam();
        if (test == 0)
            invalid = wl_subcompositor_get_subsurface(c.subcompositor, p, p);
        if (test == 1)
            invalid = wl_subcompositor_get_subsurface(c.subcompositor, a, b);
        if (test == 2)
            invalid = wl_subcompositor_get_subsurface(c.subcompositor, p, a);
        if (test == 3) {
            grand = wl_subcompositor_get_subsurface(c.subcompositor, b, a);
            invalid = wl_subcompositor_get_subsurface(c.subcompositor, p, b);
        }
        if (test == 4)
            wl_subsurface_place_above(sa, a);
        if (test == 5)
            wl_subsurface_place_below(sa, a);
        if (test == 6)
            wl_subsurface_place_above(sa, b);
        if (test == 7)
            wl_subsurface_place_below(sa, b);
        EXPECT_EQ(-1, wl_display_roundtrip(c.display));
        const wl_interface* interface = nullptr;
        uint32_t id = 0;
        EXPECT_EQ(0u, wl_display_get_protocol_error(c.display, &interface, &id));
        ASSERT_NE(nullptr, interface);
        EXPECT_STREQ(test < 4 ? "wl_subcompositor" : "wl_subsurface", interface->name);
        EXPECT_EQ(test < 4 ? wl_proxy_get_id(reinterpret_cast<wl_proxy*>(c.subcompositor))
                           : wl_proxy_get_id(reinterpret_cast<wl_proxy*>(sa)),
                  id);
        if (invalid)
            wl_subsurface_destroy(invalid);
        if (grand)
            wl_subsurface_destroy(grand);
        wl_subsurface_destroy(sa);
        wl_surface_destroy(a);
        wl_surface_destroy(b);
        wl_surface_destroy(p);
    });
}

INSTANTIATE_TEST_SUITE_P(Relationships, InvalidSubsurfaceTest, testing::Range(0, 8));

class OutputProtocolTest : public SubsurfaceOutputProtocolTest,
                           public testing::WithParamInterface<int>
{
};

TEST_P(OutputProtocolTest, FakeOutputMetricsAndVersionGates)
{
    output.preferred = GetParam() != 2;
    run([&](Client& c) {
        struct Metrics {
            int geometry = 0, mode = 0, scale = 0, done = 0;
            uint32_t flags = 0;
            std::string order;
            std::string name;
            std::string description;
        } metrics;
        static const wl_output_listener listener = {
            [](void* data, wl_output*, int x, int y, int w, int h, int subpixel, const char* make,
               const char* model, int transform) {
                auto& m = *static_cast<Metrics*>(data);
                ++m.geometry;
                m.order += 'g';
                EXPECT_EQ(13, x);
                EXPECT_EQ(17, y);
                EXPECT_EQ(510, w);
                EXPECT_EQ(290, h);
                EXPECT_EQ(WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB, subpixel);
                EXPECT_STREQ("ACME", make);
                EXPECT_STREQ("DP-7", model);
                EXPECT_EQ(WL_OUTPUT_TRANSFORM_NORMAL, transform);
            },
            [](void* data, wl_output*, uint32_t flags, int w, int h, int refresh) {
                auto& m = *static_cast<Metrics*>(data);
                ++m.mode;
                m.flags = flags;
                m.order += 'm';
                EXPECT_EQ(1920, w);
                EXPECT_EQ(1080, h);
                EXPECT_EQ(59940, refresh);
            },
            [](void* data, wl_output*) {
                auto& m = *static_cast<Metrics*>(data);
                ++m.done;
                m.order += 'd';
            },
            [](void* data, wl_output*, int factor) {
                auto& m = *static_cast<Metrics*>(data);
                ++m.scale;
                m.order += 's';
                EXPECT_EQ(1, factor);
            },
            [](void* data, wl_output*, const char* name) {
                static_cast<Metrics*>(data)->name = name;
            },
            [](void* data, wl_output*, const char* description) {
                static_cast<Metrics*>(data)->description = description;
            },
        };
        // Multiple resources per client must each receive a complete snapshot.
        for (int i = 0; i < 2; ++i) {
            metrics = {};
            auto* out = static_cast<wl_output*>(wl_registry_bind(
                c.registry, c.globals["wl_output"].first, &wl_output_interface, GetParam()));
            wl_output_add_listener(out, &listener, &metrics);
            c.sync();
            EXPECT_EQ(1, metrics.geometry);
            EXPECT_EQ(1, metrics.mode);
            EXPECT_EQ(GetParam() >= 2 ? "gmsd" : "gm", metrics.order);
            EXPECT_EQ(static_cast<uint32_t>(WL_OUTPUT_MODE_CURRENT |
                                            (output.preferred ? WL_OUTPUT_MODE_PREFERRED : 0)),
                      metrics.flags);
            if (GetParam() >= 4) {
                EXPECT_EQ("Lingmo-1", metrics.name);
                EXPECT_EQ("DP-7", metrics.description);
            }
            if (GetParam() >= 3)
                wl_output_release(out);
            else
                wl_output_destroy(out);
            c.sync();
        }
    });
}

INSTANTIATE_TEST_SUITE_P(Versions, OutputProtocolTest, testing::Range(1, 5));

TEST_F(SubsurfaceOutputProtocolTest, BindingAWithdrawnOutputKeepsTheClientConnected)
{
    run([&](Client& c) {
        const uint32_t globalName = c.globals["wl_output"].first;
        check([&] {
            wayland::removeOutputGlobal(outputGlobal);
        });
        // The withdrawal is still unread when the client binds the global.
        auto* out = static_cast<wl_output*>(
            wl_registry_bind(c.registry, globalName, &wl_output_interface, 4));
        c.sync();
        EXPECT_EQ(0, wl_display_get_error(c.display));
        EXPECT_NE(c.removedGlobals.end(),
                  std::find(c.removedGlobals.begin(), c.removedGlobals.end(), globalName));
        wl_output_release(out);
        c.sync();
    });
}

TEST_F(SubsurfaceOutputProtocolTest, OutputUpdatesAndRemovalKeepExistingResourceSafe)
{
    run([&](Client& c) {
        struct Metrics {
            int geometry = 0;
            int mode = 0;
            base::Point position;
            base::Size pixelSize;
        } metrics;
        static const wl_output_listener listener = {
            [](void* data, wl_output*, int x, int y, int, int, int, const char*, const char*, int) {
                auto& metrics = *static_cast<Metrics*>(data);
                ++metrics.geometry;
                metrics.position = base::Point(x, y);
            },
            [](void* data, wl_output*, uint32_t, int width, int height, int) {
                auto& metrics = *static_cast<Metrics*>(data);
                ++metrics.mode;
                metrics.pixelSize = base::Size(width, height);
            },
            [](void*, wl_output*) {},
            [](void*, wl_output*, int) {},
            [](void*, wl_output*, const char*) {},
            [](void*, wl_output*, const char*) {},
        };
        const uint32_t globalName = c.globals["wl_output"].first;
        auto* out = static_cast<wl_output*>(
            wl_registry_bind(c.registry, globalName, &wl_output_interface, 4));
        wl_output_add_listener(out, &listener, &metrics);
        c.sync();
        EXPECT_EQ(base::Point(13, 17), metrics.position);
        EXPECT_EQ(base::Size(1920, 1080), metrics.pixelSize);

        check([&] {
            output.position = base::Point(-1280, 0);
            output.pixelSize = base::Size(1280, 720);
            wayland::updateOutputGlobal(outputGlobal, &output);
        });
        c.sync();
        EXPECT_EQ(2, metrics.geometry);
        EXPECT_EQ(2, metrics.mode);
        EXPECT_EQ(base::Point(-1280, 0), metrics.position);
        EXPECT_EQ(base::Size(1280, 720), metrics.pixelSize);

        check([&] {
            wayland::removeOutputGlobal(outputGlobal);
        });
        c.sync();
        EXPECT_NE(c.removedGlobals.end(),
                  std::find(c.removedGlobals.begin(), c.removedGlobals.end(), globalName));
        wl_output_release(out);
        c.sync();
    });
}

TEST_F(SubsurfaceOutputProtocolTest, AWithdrawnOutputIsDestroyedOnceWhicheverComesFirst)
{
    wl_display* display = wl_display_create();
    ASSERT_TRUE(display);
    wl_global* expiring = wayland::createOutputGlobal(display, &output);
    wl_global* outlived = wayland::createOutputGlobal(display, &output);
    ASSERT_TRUE(expiring);
    ASSERT_TRUE(outlived);
    wayland::removeOutputGlobal(expiring);
    test::runFor(std::chrono::milliseconds(3200));
    wayland::removeOutputGlobal(outlived);
    wl_display_destroy(display);
    test::runFor(std::chrono::milliseconds(3200));
}
} // namespace
