// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A Lingmo counterpart to chromium/src/components/exo/wayland/surface_unittest.cc:
// a real libwayland client driving the real server over a socket.
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include <gtest/gtest.h>

#include <wayland-client.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <linux-drm-syncobj-v1-client-protocol.h>

#include <atomic>
#include <cstdlib>
#include <functional>
#include <thread>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "loop_test_util.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "temporary_directory.h"
#include "wayland/protocols/linux_drm_syncobj.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/server.h"

using namespace Lingmo;

namespace {

// Everything a client learns from the registry, plus whether its work finished.
struct ClientResult {
    std::atomic<uint32_t> compositorName { 0 };
    std::atomic<uint32_t> compositorVersion { 0 };
    std::atomic<uint32_t> shmName { 0 };
    std::atomic<uint32_t> shmVersion { 0 };
    std::atomic<int> formatCount { 0 };
    std::atomic<bool> sawArgb { false };
    std::atomic<bool> sawXrgb { false };
    std::atomic<bool> connected { false };
    std::atomic<bool> finished { false };
    std::atomic<bool> failed { false };
    std::atomic<int> protocolError { -1 };
    std::atomic<int> frameCallbacks { 0 };
    std::atomic<int> bufferReleases { 0 };
};

// Runs a client on its own thread so the server can keep dispatching on the
// event loop of the main thread.
class TestClient
{
public:
    using Body = std::function<void(wl_display*, wl_compositor*, wl_shm*, ClientResult&)>;

    void start(const std::string& socketName, Body body)
    {
        m_thread = std::thread([this, name = socketName, body = std::move(body)] {
            run(name, body);
        });
    }

    void join()
    {
        if (m_thread.joinable())
            m_thread.join();
    }

    ClientResult result;

private:
    static void handleGlobal(void* data, wl_registry* registry, uint32_t name,
                             const char* interface, uint32_t version)
    {
        auto* result = static_cast<ClientResult*>(data);
        if (std::string(interface) == "wl_compositor") {
            result->compositorName = name;
            result->compositorVersion = version;
        } else if (std::string(interface) == "wl_shm") {
            result->shmName = name;
            result->shmVersion = version;
        }
    }

    static void handleFormat(void* data, wl_shm*, uint32_t format)
    {
        auto* result = static_cast<ClientResult*>(data);
        ++result->formatCount;
        if (format == WL_SHM_FORMAT_ARGB8888)
            result->sawArgb = true;
        if (format == WL_SHM_FORMAT_XRGB8888)
            result->sawXrgb = true;
    }

    void run(const std::string& socketName, const Body& body)
    {
        wl_display* display = wl_display_connect(socketName.c_str());
        if (!display) {
            result.failed = true;
            result.finished = true;
            return;
        }
        result.connected = true;

        static const wl_registry_listener registryListener = {
            &TestClient::handleGlobal,
            [](void*, wl_registry*, uint32_t) {},
        };
        wl_registry* registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &registryListener, &result);
        wl_display_roundtrip(display);

        wl_compositor* compositor = nullptr;
        wl_shm* shm = nullptr;
        if (result.compositorName) {
            compositor = static_cast<wl_compositor*>(
                wl_registry_bind(registry, result.compositorName, &wl_compositor_interface,
                                 std::min<uint32_t>(result.compositorVersion, 4)));
        }
        if (result.shmName) {
            shm = static_cast<wl_shm*>(
                wl_registry_bind(registry, result.shmName, &wl_shm_interface, 1));
            static const wl_shm_listener shmListener = { &TestClient::handleFormat };
            wl_shm_add_listener(shm, &shmListener, &result);
            wl_display_roundtrip(display);
        }

        if (body)
            body(display, compositor, shm, result);

        if (const int error = wl_display_get_error(display))
            result.protocolError = error;

        if (shm)
            wl_shm_destroy(shm);
        if (compositor)
            wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
        result.finished = true;
    }

    std::thread m_thread;
};

int makeMemfd(size_t size)
{
    const int fd = memfd_create("lingmo-protocol-test", MFD_CLOEXEC);
    if (fd < 0 || ftruncate(fd, static_cast<off_t>(size)) != 0) {
        if (fd >= 0)
            close(fd);
        return -1;
    }
    return fd;
}

class WaylandProtocolTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_runtimeDir.isValid());
        setenv("XDG_RUNTIME_DIR", m_runtimeDir.path().c_str(), 1);

        m_scene.setSize(base::Size(800, 600));
        m_compositor = std::make_unique<compositor::Compositor>(m_scene.root());

        m_server = std::make_unique<wayland::Server>();
        ASSERT_TRUE(m_server->display());
        ASSERT_TRUE(wayland::createCompositorGlobal(m_server->display(), m_compositor.get()));
        ASSERT_TRUE(wayland::createShmGlobal(m_server->display(), m_compositor.get()));
        ASSERT_TRUE(m_server->addSocket());
        ASSERT_TRUE(m_server->start());
    }

    void TearDown() override
    {
        m_server.reset();
        m_compositor.reset();
    }

    // Runs the event loops, which drive the server, until `predicate` holds.
    template<class Predicate>
    bool waitFor(Predicate predicate, int timeoutMs = 5000)
    {
        return Lingmo::test::runUntil(predicate, std::chrono::milliseconds(timeoutMs));
    }

    bool runClient(TestClient& client, TestClient::Body body)
    {
        client.start(m_server->socketName(), std::move(body));
        const bool ok = waitFor([&client] { return client.result.finished.load(); });
        client.join();
        return ok;
    }

    Lingmo::test::TemporaryDirectory m_runtimeDir;
    scene::Scene m_scene;
    std::unique_ptr<compositor::Compositor> m_compositor;
    std::unique_ptr<wayland::Server> m_server;
};

TEST_F(WaylandProtocolTest, TheRegistryListsTheStepTwoGlobals)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, nullptr));

    EXPECT_TRUE(client.result.connected);
    EXPECT_NE(0u, client.result.compositorName);
    EXPECT_EQ(wayland::kWlCompositorVersion, client.result.compositorVersion);
    EXPECT_NE(0u, client.result.shmName);
    EXPECT_EQ(wayland::kWlShmVersion, client.result.shmVersion);
}

TEST_F(WaylandProtocolTest, ShmAdvertisesOnlyTheSupportedFormats)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, nullptr));

    EXPECT_EQ(2, client.result.formatCount);
    EXPECT_TRUE(client.result.sawArgb);
    EXPECT_TRUE(client.result.sawXrgb);
}

TEST_F(WaylandProtocolTest, AClientCreatesASurfaceAndAShmBuffer)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](wl_display* display, wl_compositor* compositor, wl_shm* shm,
                                     ClientResult& result) {
        ASSERT_TRUE(compositor);
        ASSERT_TRUE(shm);

        constexpr int kWidth = 40;
        constexpr int kHeight = 20;
        const size_t poolSize = kWidth * kHeight * 4;
        const int fd = makeMemfd(poolSize);
        ASSERT_GE(fd, 0);

        auto* pixels = static_cast<uint32_t*>(
            mmap(nullptr, poolSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        ASSERT_NE(MAP_FAILED, static_cast<void*>(pixels));
        for (size_t i = 0; i < poolSize / 4; ++i)
            pixels[i] = 0xff3366ccu;

        wl_surface* surface = wl_compositor_create_surface(compositor);
        wl_shm_pool* pool = wl_shm_create_pool(shm, fd, poolSize);
        wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, kWidth, kHeight, kWidth * 4,
                                                      WL_SHM_FORMAT_ARGB8888);

        static const wl_buffer_listener bufferListener = {
            [](void* data, wl_buffer*) { ++static_cast<ClientResult*>(data)->bufferReleases; },
        };
        wl_buffer_add_listener(buffer, &bufferListener, &result);

        wl_surface_attach(surface, buffer, 0, 0);
        wl_surface_damage_buffer(surface, 0, 0, kWidth, kHeight);
        wl_surface_commit(surface);
        wl_display_roundtrip(display);

        wl_buffer_destroy(buffer);
        wl_shm_pool_destroy(pool);
        wl_surface_destroy(surface);
        wl_display_roundtrip(display);

        munmap(pixels, poolSize);
        close(fd);
    }));

    EXPECT_EQ(-1, client.result.protocolError) << "the exchange must be error-free";
}

TEST_F(WaylandProtocolTest, TheCommittedBufferReachesTheSceneAndReleasesTheClient)
{
    // The surface has to outlive the client thread, so the client parks after
    // committing and this test drives the frame from the server side.
    std::atomic<bool> committed { false };
    std::atomic<bool> mayFinish { false };

    TestClient client;
    client.start(m_server->socketName(), [&committed,
                 &mayFinish](wl_display* display,
                                                                  wl_compositor* compositor,
                                                                  wl_shm* shm,
                                                                  ClientResult& result) {
        constexpr int kWidth = 8;
        constexpr int kHeight = 4;
        const size_t poolSize = kWidth * kHeight * 4;
        const int fd = makeMemfd(poolSize);
        ASSERT_GE(fd, 0);
        auto* pixels = static_cast<uint32_t*>(
            mmap(nullptr, poolSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        ASSERT_NE(MAP_FAILED, static_cast<void*>(pixels));
        for (size_t i = 0; i < poolSize / 4; ++i)
            pixels[i] = 0xff112233u;

        wl_surface* surface = wl_compositor_create_surface(compositor);
        wl_shm_pool* pool = wl_shm_create_pool(shm, fd, poolSize);
        wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, kWidth, kHeight, kWidth * 4,
                                                      WL_SHM_FORMAT_XRGB8888);
        static const wl_buffer_listener bufferListener = {
            [](void* data, wl_buffer*) { ++static_cast<ClientResult*>(data)->bufferReleases; },
        };
        wl_buffer_add_listener(buffer, &bufferListener, &result);

        static const wl_callback_listener frameListener = {
            [](void* data, wl_callback* callback, uint32_t) {
                ++static_cast<ClientResult*>(data)->frameCallbacks;
                wl_callback_destroy(callback);
            },
        };
        wl_callback* frame = wl_surface_frame(surface);
        wl_callback_add_listener(frame, &frameListener, &result);

        wl_surface_attach(surface, buffer, 0, 0);
        wl_surface_damage(surface, 0, 0, kWidth, kHeight);
        wl_surface_commit(surface);
        wl_display_roundtrip(display);
        committed = true;

        while (!mayFinish) {
            if (wl_display_prepare_read(display) == 0) {
                wl_display_flush(display);
                wl_display_read_events(display);
            }
            wl_display_dispatch_pending(display);
        }
        wl_display_roundtrip(display);

        wl_buffer_destroy(buffer);
        wl_shm_pool_destroy(pool);
        wl_surface_destroy(surface);
        munmap(pixels, poolSize);
        close(fd);
    });

    ASSERT_TRUE(waitFor([&committed] { return committed.load(); }));
    ASSERT_EQ(1u, m_compositor->surfaces().size());

    compositor::Surface* surface = m_compositor->surfaces().front();
    EXPECT_EQ(base::Size(8, 4), surface->contentSize());
    EXPECT_EQ(base::Region(base::Rect(0, 0, 8, 4)), surface->damageRegion());

    Lingmo::testsupport::FakeRenderer renderer;
    m_compositor->updateTextures(renderer);
    ASSERT_NE(nullptr, surface->node());
    ASSERT_NE(nullptr, surface->node()->texture());

    // The upload copied the pixels, so the client gets its buffer back.
    EXPECT_TRUE(waitFor([&client] { return client.result.bufferReleases.load() == 1; }));

    // Frame callbacks wait for a presented frame, not for the commit.
    EXPECT_EQ(0, client.result.frameCallbacks);
    m_compositor->frameSubmitted();
    EXPECT_EQ(0, client.result.frameCallbacks);
    m_compositor->frameDrawn(1234);
    EXPECT_TRUE(waitFor([&client] { return client.result.frameCallbacks.load() == 1; }));

    mayFinish = true;
    ASSERT_TRUE(waitFor([&client] { return client.result.finished.load(); }));
    client.join();
    EXPECT_EQ(-1, client.result.protocolError);
}

TEST_F(WaylandProtocolTest, AnInvalidBufferScaleIsAProtocolError)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](wl_display* display, wl_compositor* compositor, wl_shm*,
                                     ClientResult&) {
        wl_surface* surface = wl_compositor_create_surface(compositor);
        wl_surface_set_buffer_scale(surface, 0);
        wl_display_roundtrip(display);
        wl_surface_destroy(surface);
    }));
    EXPECT_NE(-1, client.result.protocolError);
}

TEST_F(WaylandProtocolTest, AnInvalidBufferTransformIsAProtocolError)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](wl_display* display, wl_compositor* compositor, wl_shm*,
                                     ClientResult&) {
        wl_surface* surface = wl_compositor_create_surface(compositor);
        wl_surface_set_buffer_transform(surface, 42);
        wl_display_roundtrip(display);
        wl_surface_destroy(surface);
    }));
    EXPECT_NE(-1, client.result.protocolError);
}

TEST_F(WaylandProtocolTest, AnOutOfPoolBufferIsAProtocolError)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](wl_display* display, wl_compositor*, wl_shm* shm,
                                     ClientResult&) {
        const int fd = makeMemfd(4096);
        ASSERT_GE(fd, 0);
        wl_shm_pool* pool = wl_shm_create_pool(shm, fd, 4096);
        // 64 * 64 * 4 is far more than the pool holds.
        wl_buffer* buffer =
            wl_shm_pool_create_buffer(pool, 0, 64, 64, 64 * 4, WL_SHM_FORMAT_ARGB8888);
        wl_display_roundtrip(display);
        wl_buffer_destroy(buffer);
        wl_shm_pool_destroy(pool);
        close(fd);
    }));
    EXPECT_NE(-1, client.result.protocolError);
}

TEST_F(WaylandProtocolTest, AnUnsupportedFormatIsAProtocolError)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](wl_display* display, wl_compositor*, wl_shm* shm,
                                     ClientResult&) {
        const int fd = makeMemfd(4096);
        ASSERT_GE(fd, 0);
        wl_shm_pool* pool = wl_shm_create_pool(shm, fd, 4096);
        wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, 4, 4, 16, WL_SHM_FORMAT_YUV420);
        wl_display_roundtrip(display);
        wl_buffer_destroy(buffer);
        wl_shm_pool_destroy(pool);
        close(fd);
    }));
    EXPECT_NE(-1, client.result.protocolError);
}

TEST_F(WaylandProtocolTest, ShrinkingAPoolIsAProtocolError)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](wl_display* display, wl_compositor*, wl_shm* shm,
                                     ClientResult&) {
        const int fd = makeMemfd(8192);
        ASSERT_GE(fd, 0);
        wl_shm_pool* pool = wl_shm_create_pool(shm, fd, 8192);
        wl_shm_pool_resize(pool, 4096);
        wl_display_roundtrip(display);
        wl_shm_pool_destroy(pool);
        close(fd);
    }));
    EXPECT_NE(-1, client.result.protocolError);
}

TEST_F(WaylandProtocolTest, TheCompositorSurvivesAClientProtocolError)
{
    TestClient bad;
    ASSERT_TRUE(runClient(bad, [](wl_display* display, wl_compositor* compositor, wl_shm*,
                                  ClientResult&) {
        wl_surface* surface = wl_compositor_create_surface(compositor);
        wl_surface_set_buffer_scale(surface, -5);
        wl_display_roundtrip(display);
        wl_surface_destroy(surface);
    }));
    ASSERT_NE(-1, bad.result.protocolError);

    // The offending client is gone and its surface with it.
    EXPECT_TRUE(waitFor([this] { return m_compositor->surfaces().empty(); }));

    TestClient good;
    ASSERT_TRUE(runClient(good, nullptr));
    EXPECT_TRUE(good.result.connected);
    EXPECT_NE(0u, good.result.compositorName);
    EXPECT_EQ(-1, good.result.protocolError);
}

TEST_F(WaylandProtocolTest, ADisconnectingClientTakesItsSurfacesWithIt)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](wl_display* display, wl_compositor* compositor, wl_shm*,
                                     ClientResult&) {
        wl_surface* first = wl_compositor_create_surface(compositor);
        wl_surface* second = wl_compositor_create_surface(compositor);
        wl_display_roundtrip(display);
        wl_surface_destroy(first);
        wl_surface_destroy(second);
    }));

    EXPECT_TRUE(waitFor([this] { return m_compositor->surfaces().empty(); }));
}

} // namespace

TEST(LinuxDrmSyncobj, EveryBufferCommitNeedsFencesAndBothPoints)
{
    using wayland::SyncobjCommit;
    EXPECT_EQ(wayland::syncobjCommitError({}), -1);
    EXPECT_EQ(wayland::syncobjCommitError({false, false, true, false, false}),
              WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_BUFFER);
    EXPECT_EQ(wayland::syncobjCommitError({true, false, false, false, false}),
              WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_UNSUPPORTED_BUFFER);
    EXPECT_EQ(wayland::syncobjCommitError({true, true, false, false, false}),
              WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_ACQUIRE_POINT);
    EXPECT_EQ(wayland::syncobjCommitError({true, true, true, false, false}),
              WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_RELEASE_POINT);
    EXPECT_EQ(wayland::syncobjCommitError({true, true, true, true, true}),
              WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_CONFLICTING_POINTS);
    EXPECT_EQ(wayland::syncobjCommitError({true, true, true, true, false}), -1);
}

TEST_F(WaylandProtocolTest, AShmBufferUnderASyncobjSurfaceIsUnsupported)
{
    const int drmFd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    if (drmFd < 0)
        GTEST_SKIP() << "No render node for DRM syncobj timelines";
    if (!wayland::createLinuxDrmSyncobjGlobal(m_server->display(), drmFd)) {
        close(drmFd);
        GTEST_SKIP() << "The render node has no syncobj timelines";
    }
    std::atomic<uint32_t> code {0};
    TestClient client;
    ASSERT_TRUE(runClient(client, [&code](wl_display* display, wl_compositor* compositor,
                                          wl_shm* shm, ClientResult&) {
        uint32_t managerName = 0;
        static const wl_registry_listener listener = {
            [](void* data, wl_registry*, uint32_t name, const char* interface, uint32_t) {
                if (std::string(interface) == wp_linux_drm_syncobj_manager_v1_interface.name)
                    *static_cast<uint32_t*>(data) = name;
            },
            [](void*, wl_registry*, uint32_t) {},
        };
        wl_registry* registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &listener, &managerName);
        wl_display_roundtrip(display);
        ASSERT_NE(managerName, 0u);
        auto* manager = static_cast<wp_linux_drm_syncobj_manager_v1*>(wl_registry_bind(
            registry, managerName, &wp_linux_drm_syncobj_manager_v1_interface, 1));
        wl_surface* surface = wl_compositor_create_surface(compositor);
        auto* syncobjSurface = wp_linux_drm_syncobj_manager_v1_get_surface(manager, surface);
        const int fd = makeMemfd(16 * 16 * 4);
        ASSERT_GE(fd, 0);
        wl_shm_pool* pool = wl_shm_create_pool(shm, fd, 16 * 16 * 4);
        wl_buffer* buffer =
            wl_shm_pool_create_buffer(pool, 0, 16, 16, 16 * 4, WL_SHM_FORMAT_ARGB8888);
        // A commit without a buffer is still fine.
        wl_surface_commit(surface);
        wl_display_roundtrip(display);
        EXPECT_EQ(wl_display_get_error(display), 0);
        wl_surface_attach(surface, buffer, 0, 0);
        wl_surface_commit(surface);
        wl_display_roundtrip(display);
        const wl_interface* interface = nullptr;
        code = wl_display_get_protocol_error(display, &interface, nullptr);
        EXPECT_EQ(interface, &wp_linux_drm_syncobj_surface_v1_interface);
        close(fd);
        // The display is in error; the proxies still have to go.
        wp_linux_drm_syncobj_surface_v1_destroy(syncobjSurface);
        wl_buffer_destroy(buffer);
        wl_shm_pool_destroy(pool);
        wl_surface_destroy(surface);
        wp_linux_drm_syncobj_manager_v1_destroy(manager);
        wl_registry_destroy(registry);
    }));
    close(drmFd);
    EXPECT_NE(-1, client.result.protocolError);
    EXPECT_EQ(code.load(), uint32_t(WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_UNSUPPORTED_BUFFER));
}
