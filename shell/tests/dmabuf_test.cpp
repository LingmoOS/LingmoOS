// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A Lingmo counterpart to chromium/src/components/exo/wayland/zwp_linux_dmabuf.cc
// and wayland_dmabuf_feedback_manager.cc: a real libwayland client driving the
// real server over a socket, plus unit coverage of the buffer lifetime.
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include <gtest/gtest.h>

#include <drm_fourcc.h>
#include <linux-dmabuf-v1-client-protocol.h>
#include <wayland-client.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "compositor/compositor.h"
#include "compositor/dmabuf_buffer.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "loop_test_util.h"
#include "render/dmabuf.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "temporary_directory.h"
#include "wayland/protocols/dmabuf_feedback_manager.h"
#include "wayland/protocols/linux_dmabuf.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/server.h"

using namespace Lingmo;

namespace {

constexpr uint64_t kModifierA = 0x0100000000000001ull;
constexpr uint64_t kModifierB = 0x0100000000000002ull;

// Stands in for the imported EGLImage.
class FakeDmaBufTexture : public render::Texture
{
public:
    FakeDmaBufTexture(const base::Size& size, render::PixelFormat format)
        : m_size(size)
        , m_format(format)
    {
    }

    base::Size size() const override { return m_size; }
    render::PixelFormat format() const override { return m_format; }

private:
    base::Size m_size;
    render::PixelFormat m_format;
};

// A DmaBufDevice with the capabilities a test wants, recording what it was
// asked to import. Runs on the server (main) thread only.
class FakeDmaBufDevice : public render::DmaBufDevice
{
public:
    struct ImportRecord {
        base::Size size;
        uint32_t format = 0;
        uint64_t modifier = 0;
        bool yInvert = false;
        std::vector<uint32_t> offsets;
        std::vector<uint32_t> strides;
        std::vector<int> fds;
    };

    FakeDmaBufDevice()
    {
        m_formats.emplace(DRM_FORMAT_ARGB8888,
                          std::vector<uint64_t> { kModifierA, kModifierB });
        m_formats.emplace(DRM_FORMAT_XRGB8888, std::vector<uint64_t> { kModifierA });
        m_formats.emplace(DRM_FORMAT_NV12, std::vector<uint64_t> { kModifierA });
    }

    const render::DmaBufFormats& dmaBufFormats() const override { return m_formats; }

    dev_t dmaBufMainDevice() const override { return m_mainDevice; }

    std::shared_ptr<render::Texture> importDmaBuf(
        const render::DmaBufAttributes& attributes) override
    {
        ImportRecord record;
        record.size = attributes.size;
        record.format = attributes.format;
        record.modifier = attributes.modifier;
        record.yInvert = attributes.yInvert;
        for (const render::DmaBufPlane& plane : attributes.planes) {
            record.offsets.push_back(plane.offset);
            record.strides.push_back(plane.stride);
            record.fds.push_back(plane.fd.get());
        }
        imports.push_back(std::move(record));

        if (failImports)
            return nullptr;
        return std::make_shared<FakeDmaBufTexture>(attributes.size,
                                                   render::PixelFormat::Rgba8888);
    }

    void setMainDevice(dev_t device) { m_mainDevice = device; }

    std::vector<ImportRecord> imports;
    bool failImports = false;

private:
    render::DmaBufFormats m_formats;
    dev_t m_mainDevice = 0x1234;
};

// One entry of the zwp_linux_dmabuf_feedback_v1 format table.
struct FormatTableEntry {
    uint32_t format;
    uint32_t padding;
    uint64_t modifier;
};

// Everything the client learns, plus whether its work finished.
struct ClientResult {
    std::atomic<uint32_t> compositorName { 0 };
    std::atomic<uint32_t> compositorVersion { 0 };
    std::atomic<uint32_t> dmabufName { 0 };
    std::atomic<uint32_t> dmabufVersion { 0 };
    std::atomic<bool> connected { false };
    std::atomic<bool> finished { false };
    std::atomic<bool> failed { false };
    std::atomic<int> protocolErrorCode { -1 };
    std::atomic<int> formatEvents { 0 };
    std::atomic<int> modifierEvents { 0 };
    std::atomic<int> createdEvents { 0 };
    std::atomic<int> failedEvents { 0 };
    std::atomic<int> feedbackDone { 0 };
    std::atomic<int> trancheDone { 0 };
    std::atomic<uint32_t> trancheFlags { 0 };
    std::atomic<uint64_t> mainDevice { 0 };
    std::atomic<uint64_t> trancheDevice { 0 };
    // Lets a test inspect server state while the client is still connected.
    std::atomic<bool> reachedHold { false };
    std::atomic<bool> releaseHold { false };

    void release()
    {
        std::lock_guard lock(mutex);
        releaseHold = true;
        holdChanged.notify_all();
    }
    bool waitForRelease()
    {
        std::unique_lock lock(mutex);
        return holdChanged.wait_for(lock, std::chrono::seconds(5), [&] { return releaseHold.load(); });
    }
    std::condition_variable holdChanged;
    std::mutex mutex;
    std::vector<FormatTableEntry> formatTable;
    std::vector<uint16_t> trancheFormats;
    // Every tranche in the order received: its flags and its format indices.
    struct Tranche {
        uint32_t flags = 0;
        std::vector<uint16_t> formats;
    };
    std::vector<Tranche> tranches;
};

template<class T, void (*Destroy)(T*)>
class ClientProxy
{
public:
    ClientProxy(wl_display* display, T* proxy) : m_display(display), m_proxy(proxy) {}
    ~ClientProxy() { reset(); }
    ClientProxy(const ClientProxy&) = delete;
    ClientProxy& operator=(const ClientProxy&) = delete;
    operator T*() const { return m_proxy; }
    T* get() const { return m_proxy; }
    void reset()
    {
        if (!m_proxy)
            return;
        if (wl_display_get_error(m_display))
            wl_proxy_destroy(reinterpret_cast<wl_proxy*>(m_proxy));
        else
            Destroy(m_proxy);
        m_proxy = nullptr;
    }
private:
    wl_display* m_display;
    T* m_proxy;
};

struct ClientContext {
    wl_display* display = nullptr;
    wl_compositor* compositor = nullptr;
    zwp_linux_dmabuf_v1* dmabuf = nullptr;
    ClientResult* result = nullptr;
};

// Runs a client on its own thread so the server can keep dispatching on the
// event loop of the main thread.
class TestClient
{
public:
    using Body = std::function<void(ClientContext&)>;

    void start(const std::string& socketName, Body body, uint32_t bindVersion)
    {
        m_thread = std::thread(
            [this, name = socketName, body = std::move(body), bindVersion] {
                run(name, body, bindVersion);
            });
    }

    ~TestClient()
    {
        stop();
        join();
    }

    void stop()
    {
        result.release();
        std::lock_guard lock(m_connectionMutex);
        m_stopping = true;
        if (m_fd >= 0)
            shutdown(m_fd, SHUT_RDWR);
    }

    void join()
    {
        if (m_thread.joinable())
            m_thread.join();
    }

    ClientResult result;

private:
    static void handleGlobal(void* data, wl_registry*, uint32_t name, const char* interface,
                             uint32_t version)
    {
        auto* result = static_cast<ClientResult*>(data);
        if (std::string(interface) == "wl_compositor") {
            result->compositorName = name;
            result->compositorVersion = version;
        } else if (std::string(interface) == "zwp_linux_dmabuf_v1") {
            result->dmabufName = name;
            result->dmabufVersion = version;
        }
    }

    void run(const std::string& socketName, const Body& body, uint32_t bindVersion)
    {
        wl_display* display = wl_display_connect(socketName.c_str());
        if (!display) {
            result.failed = true;
            result.finished = true;
            return;
        }
        {
            std::lock_guard lock(m_connectionMutex);
            m_fd = wl_display_get_fd(display);
            if (m_stopping)
                shutdown(m_fd, SHUT_RDWR);
        }
        auto disconnect = [this](wl_display* connection) {
            std::lock_guard lock(m_connectionMutex);
            m_fd = -1;
            wl_display_disconnect(connection);
            result.finished = true;
        };
        std::unique_ptr<wl_display, decltype(disconnect)> displayOwner(display, disconnect);
        result.connected = true;

        static const wl_registry_listener registryListener = {
            &TestClient::handleGlobal,
            [](void*, wl_registry*, uint32_t) {},
        };
        wl_registry* registry = wl_display_get_registry(display);
        ClientProxy<wl_registry, wl_registry_destroy> registryOwner(display, registry);
        if (!registry) { result.failed = true; return; }
        wl_registry_add_listener(registry, &registryListener, &result);
        wl_display_roundtrip(display);

        ClientContext context;
        context.display = display;
        context.result = &result;

        if (result.compositorName) {
            context.compositor = static_cast<wl_compositor*>(
                wl_registry_bind(registry, result.compositorName, &wl_compositor_interface,
                                 std::min<uint32_t>(result.compositorVersion, 4)));
        }
        ClientProxy<wl_compositor, wl_compositor_destroy> compositorOwner(display, context.compositor);
        if (result.dmabufName) {
            context.dmabuf = static_cast<zwp_linux_dmabuf_v1*>(wl_registry_bind(
                registry, result.dmabufName, &zwp_linux_dmabuf_v1_interface,
                std::min<uint32_t>(result.dmabufVersion, bindVersion)));
        }
        ClientProxy<zwp_linux_dmabuf_v1, zwp_linux_dmabuf_v1_destroy> dmabufOwner(display, context.dmabuf);
        if (context.dmabuf) {
            static const zwp_linux_dmabuf_v1_listener dmabufListener = {
                [](void* data, zwp_linux_dmabuf_v1*, uint32_t) {
                    ++static_cast<ClientResult*>(data)->formatEvents;
                },
                [](void* data, zwp_linux_dmabuf_v1*, uint32_t, uint32_t, uint32_t) {
                    ++static_cast<ClientResult*>(data)->modifierEvents;
                },
            };
            zwp_linux_dmabuf_v1_add_listener(context.dmabuf, &dmabufListener, &result);
            wl_display_roundtrip(display);
        }

        if (!context.compositor || !context.dmabuf)
            result.failed = true;
        else if (body)
            body(context);

        if (wl_display_get_error(display)) {
            uint32_t id = 0;
            const wl_interface* interface = nullptr;
            result.protocolErrorCode =
                static_cast<int>(wl_display_get_protocol_error(display, &interface, &id));
        }
    }

    std::thread m_thread;
    std::mutex m_connectionMutex;
    int m_fd = -1;
    bool m_stopping = false;
};

// A memfd of `size` bytes standing in for a DMA-BUF. The compositor only ever
// looks at its length, which is what the out-of-bounds check needs.
int makeFakeDmaBufFd(size_t size)
{
    const int fd = memfd_create("lingmo-dmabuf-test", MFD_CLOEXEC);
    if (fd < 0 || ftruncate(fd, static_cast<off_t>(size)) != 0) {
        if (fd >= 0)
            close(fd);
        return -1;
    }
    return fd;
}

const zwp_linux_dmabuf_feedback_v1_listener& feedbackListener()
{
    static const zwp_linux_dmabuf_feedback_v1_listener listener = {
        [](void* data, zwp_linux_dmabuf_feedback_v1*) {
            ++static_cast<ClientResult*>(data)->feedbackDone;
        },
        [](void* data, zwp_linux_dmabuf_feedback_v1*, int32_t fd, uint32_t size) {
            auto* result = static_cast<ClientResult*>(data);
            void* table = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
            if (table != MAP_FAILED) {
                const auto* entries = static_cast<const FormatTableEntry*>(table);
                std::lock_guard<std::mutex> lock(result->mutex);
                result->formatTable.assign(entries,
                                           entries + size / sizeof(FormatTableEntry));
                munmap(table, size);
            }
            close(fd);
        },
        [](void* data, zwp_linux_dmabuf_feedback_v1*, wl_array* device) {
            auto* result = static_cast<ClientResult*>(data);
            dev_t id = 0;
            memcpy(&id, device->data, std::min(sizeof(id), device->size));
            result->mainDevice = id;
        },
        [](void* data, zwp_linux_dmabuf_feedback_v1*) {
            auto* result = static_cast<ClientResult*>(data);
            std::lock_guard<std::mutex> lock(result->mutex);
            result->tranches.push_back({ result->trancheFlags, result->trancheFormats });
            ++result->trancheDone;
        },
        [](void* data, zwp_linux_dmabuf_feedback_v1*, wl_array* device) {
            auto* result = static_cast<ClientResult*>(data);
            dev_t id = 0;
            memcpy(&id, device->data, std::min(sizeof(id), device->size));
            result->trancheDevice = id;
        },
        [](void* data, zwp_linux_dmabuf_feedback_v1*, wl_array* indices) {
            auto* result = static_cast<ClientResult*>(data);
            const auto* values = static_cast<const uint16_t*>(indices->data);
            std::lock_guard<std::mutex> lock(result->mutex);
            result->trancheFormats.assign(values, values + indices->size / sizeof(uint16_t));
        },
        [](void* data, zwp_linux_dmabuf_feedback_v1*, uint32_t flags) {
            static_cast<ClientResult*>(data)->trancheFlags = flags;
        },
    };
    return listener;
}

const zwp_linux_buffer_params_v1_listener& paramsListener()
{
    static const zwp_linux_buffer_params_v1_listener listener = {
        [](void* data, zwp_linux_buffer_params_v1*, wl_buffer* buffer) {
            ++static_cast<ClientResult*>(data)->createdEvents;
            wl_buffer_destroy(buffer);
        },
        [](void* data, zwp_linux_buffer_params_v1*) {
            ++static_cast<ClientResult*>(data)->failedEvents;
        },
    };
    return listener;
}

class DmaBufTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_runtimeDir.isValid());
        if (const char* previous = std::getenv("XDG_RUNTIME_DIR"))
            m_previousRuntime = previous;
        setenv("XDG_RUNTIME_DIR", m_runtimeDir.path().c_str(), 1);

        m_scene.setSize(base::Size(800, 600));
        m_compositor = std::make_unique<compositor::Compositor>(m_scene.root());
        m_compositor->setDmaBufDevice(&m_device);

        m_server = std::make_unique<wayland::Server>();
        ASSERT_TRUE(m_server->display());
        ASSERT_TRUE(wayland::createCompositorGlobal(m_server->display(), m_compositor.get()));
        m_dmabufGlobal = wayland::LinuxDmabufGlobal::create(m_server->display(),
                                                            m_compositor.get(), &m_device);
        ASSERT_TRUE(m_dmabufGlobal);
        ASSERT_TRUE(m_server->addSocket());
        ASSERT_TRUE(m_server->start());
    }

    void TearDown() override
    {
        m_dmabufGlobal.reset();
        m_server.reset();
        m_compositor.reset();
        if (!m_previousRuntime) unsetenv("XDG_RUNTIME_DIR");
        else setenv("XDG_RUNTIME_DIR", m_previousRuntime->c_str(), 1);
    }

    template<class Predicate>
    bool waitFor(Predicate predicate, int timeoutMs = 5000)
    {
        return Lingmo::test::runUntil(predicate, std::chrono::milliseconds(timeoutMs));
    }

    bool runClient(TestClient& client, TestClient::Body body, uint32_t bindVersion = 4)
    {
        client.start(m_server->socketName(), std::move(body), bindVersion);
        const bool ok = waitFor([&client] { return client.result.finished.load(); });
        if (!ok)
            client.stop();
        client.join();
        return ok && !client.result.failed;
    }

    Lingmo::test::TemporaryDirectory m_runtimeDir;
    std::optional<std::string> m_previousRuntime;
    scene::Scene m_scene;
    FakeDmaBufDevice m_device;
    std::unique_ptr<compositor::Compositor> m_compositor;
    std::unique_ptr<wayland::Server> m_server;
    std::unique_ptr<wayland::LinuxDmabufGlobal> m_dmabufGlobal;
};

// Adds one valid plane to `params`, closing the fd afterwards as clients do.
void addPlane(zwp_linux_buffer_params_v1* params, uint32_t index, uint32_t offset,
              uint32_t stride, uint64_t modifier, size_t fdSize = 64 * 1024)
{
    const int fd = makeFakeDmaBufFd(fdSize);
    ASSERT_GE(fd, 0);
    zwp_linux_buffer_params_v1_add(params, fd, index, offset, stride, modifier >> 32,
                                   modifier & 0xffffffff);
    close(fd);
}

////////////////////////////////////////////////////////////////////////////////
// Globals and feedback.

TEST_F(DmaBufTest, TheGlobalIsAdvertisedAtTheVersionThePlatformSupports)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, nullptr));
    EXPECT_EQ(client.result.dmabufVersion, 4u);
    // Version 4 replaces the format/modifier events with feedback.
    EXPECT_EQ(client.result.formatEvents, 0);
    EXPECT_EQ(client.result.modifierEvents, 0);
}

TEST_F(DmaBufTest, OlderClientsStillGetFormatAndModifierEvents)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, nullptr, /*bindVersion=*/3));
    EXPECT_EQ(client.result.formatEvents, 3);
    // ARGB8888 has two modifiers, XRGB8888 and NV12 one; each also gets the
    // implicit modifier.
    EXPECT_EQ(client.result.modifierEvents, 7);
}

TEST_F(DmaBufTest, DefaultFeedbackDescribesTheDeviceCapabilities)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_dmabuf_feedback_v1, zwp_linux_dmabuf_feedback_v1_destroy> feedback(
            context.display, zwp_linux_dmabuf_v1_get_default_feedback(context.dmabuf));
        ASSERT_TRUE(feedback.get());
        zwp_linux_dmabuf_feedback_v1_add_listener(feedback, &feedbackListener(),
                                                  context.result);
        wl_display_roundtrip(context.display);
        feedback.reset();
        wl_display_roundtrip(context.display);
    }));

    EXPECT_EQ(client.result.feedbackDone, 1);
    EXPECT_EQ(client.result.trancheDone, 1);
    EXPECT_EQ(client.result.mainDevice, 0x1234u);
    EXPECT_EQ(client.result.trancheDevice, 0x1234u);
    EXPECT_EQ(client.result.trancheFlags, 0u); // No scanout tranche yet.

    std::lock_guard<std::mutex> lock(client.result.mutex);
    // Three formats, each with the implicit modifier plus its own.
    ASSERT_EQ(client.result.formatTable.size(), 7u);
    ASSERT_EQ(client.result.trancheFormats.size(), 7u);

    bool sawArgbImplicit = false;
    bool sawArgbModifierB = false;
    for (uint16_t index : client.result.trancheFormats) {
        ASSERT_LT(index, client.result.formatTable.size());
        const FormatTableEntry& entry = client.result.formatTable[index];
        if (entry.format == DRM_FORMAT_ARGB8888 && entry.modifier == DRM_FORMAT_MOD_INVALID)
            sawArgbImplicit = true;
        if (entry.format == DRM_FORMAT_ARGB8888 && entry.modifier == kModifierB)
            sawArgbModifierB = true;
    }
    EXPECT_TRUE(sawArgbImplicit);
    EXPECT_TRUE(sawArgbModifierB);
}

TEST_F(DmaBufTest, SurfaceFeedbackIsDeliveredForASurface)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<wl_surface, wl_surface_destroy> surface(
            context.display, wl_compositor_create_surface(context.compositor));
        ASSERT_TRUE(surface.get());
        ClientProxy<zwp_linux_dmabuf_feedback_v1, zwp_linux_dmabuf_feedback_v1_destroy> feedback(
            context.display, zwp_linux_dmabuf_v1_get_surface_feedback(context.dmabuf, surface));
        ASSERT_TRUE(feedback.get());
        zwp_linux_dmabuf_feedback_v1_add_listener(feedback, &feedbackListener(),
                                                  context.result);
        wl_display_roundtrip(context.display);
        feedback.reset();
        surface.reset();
        wl_display_roundtrip(context.display);
    }));

    EXPECT_EQ(client.result.feedbackDone, 1);
    EXPECT_EQ(client.result.trancheDone, 1);
    EXPECT_EQ(client.result.mainDevice, 0x1234u);
    EXPECT_EQ(client.result.protocolErrorCode, -1);
}

// The first tranche is the scanout one, restricted to what the primary plane
// takes: the modifiers both sides list, the implicit modifier where the plane
// has no explicit ones, and nothing for a format the plane cannot show.
TEST_F(DmaBufTest, AScanoutCandidateGetsAScanoutTrancheFirstAndLosesItAgain)
{
    auto* manager = m_dmabufGlobal->feedbackManager();
    manager->setScanoutFormats([] {
        return wayland::ScanoutFormats { { DRM_FORMAT_ARGB8888, { kModifierA } },
                                         { DRM_FORMAT_NV12, {} } };
    });

    TestClient client;
    client.start(m_server->socketName(), [](ClientContext& context) {
        ClientProxy<wl_surface, wl_surface_destroy> surface(
            context.display, wl_compositor_create_surface(context.compositor));
        ASSERT_TRUE(surface.get());
        ClientProxy<zwp_linux_dmabuf_feedback_v1, zwp_linux_dmabuf_feedback_v1_destroy> feedback(
            context.display, zwp_linux_dmabuf_v1_get_surface_feedback(context.dmabuf, surface));
        ASSERT_TRUE(feedback.get());
        zwp_linux_dmabuf_feedback_v1_add_listener(feedback, &feedbackListener(),
                                                  context.result);
        wl_display_roundtrip(context.display);

        context.result->reachedHold = true;
        EXPECT_TRUE(context.result->waitForRelease());
        wl_display_roundtrip(context.display);

        feedback.reset();
        surface.reset();
        wl_display_roundtrip(context.display);
    }, 4);
    ASSERT_TRUE(waitFor([&client] { return client.result.reachedHold.load(); }));
    EXPECT_EQ(client.result.feedbackDone, 1);
    EXPECT_EQ(client.result.trancheDone, 1);
    ASSERT_EQ(m_compositor->surfaces().size(), 1u);
    compositor::Surface* surface = m_compositor->surfaces().front();
    EXPECT_EQ(manager->surfaceFeedbackCount(), 1u);

    manager->setScanoutCandidate(surface, true);
    EXPECT_TRUE(manager->isScanoutCandidate(surface));
    // Marking it again re-sends nothing.
    manager->setScanoutCandidate(surface, true);
    // Dropping the candidate re-sends the plain feedback.
    manager->setScanoutCandidate(surface, false);
    manager->setScanoutCandidate(surface, true);
    client.result.release();
    ASSERT_TRUE(waitFor([&client] { return client.result.finished.load(); }));
    client.join();
    ASSERT_FALSE(client.result.failed);

    EXPECT_EQ(client.result.feedbackDone, 4);
    std::lock_guard<std::mutex> lock(client.result.mutex);
    // Plain, scanout + default, plain, scanout + default.
    ASSERT_EQ(client.result.tranches.size(), 6u);
    EXPECT_EQ(client.result.tranches[0].flags, 0u);
    EXPECT_EQ(client.result.tranches[1].flags, ZWP_LINUX_DMABUF_FEEDBACK_V1_TRANCHE_FLAGS_SCANOUT);
    EXPECT_EQ(client.result.tranches[2].flags, 0u);
    EXPECT_EQ(client.result.tranches[3].flags, 0u);
    EXPECT_EQ(client.result.tranches[4].flags, ZWP_LINUX_DMABUF_FEEDBACK_V1_TRANCHE_FLAGS_SCANOUT);
    EXPECT_EQ(client.result.tranches[5].flags, 0u);
    EXPECT_EQ(client.result.tranches[5].formats.size(), 7u);

    std::vector<std::pair<uint32_t, uint64_t>> scanout;
    for (uint16_t index : client.result.tranches[1].formats) {
        ASSERT_LT(index, client.result.formatTable.size());
        const FormatTableEntry& entry = client.result.formatTable[index];
        scanout.emplace_back(entry.format, entry.modifier);
    }
    // In format table order, which is the device's fourcc order.
    const std::vector<std::pair<uint32_t, uint64_t>> expected = {
        { DRM_FORMAT_NV12, DRM_FORMAT_MOD_INVALID },
        { DRM_FORMAT_ARGB8888, kModifierA },
    };
    EXPECT_EQ(scanout, expected);

    // The surface went with its resource: nothing is left behind.
    EXPECT_TRUE(waitFor([&] { return m_compositor->surfaces().empty(); }));
    EXPECT_EQ(manager->surfaceFeedbackCount(), 0u);
    EXPECT_FALSE(manager->isScanoutCandidate(surface));
}

TEST_F(DmaBufTest, NoGlobalWithoutAnImportableFormat)
{
    class EmptyDevice : public render::DmaBufDevice
    {
    public:
        const render::DmaBufFormats& dmaBufFormats() const override { return m_formats; }
        dev_t dmaBufMainDevice() const override { return 0; }
        std::shared_ptr<render::Texture> importDmaBuf(const render::DmaBufAttributes&) override
        {
            return nullptr;
        }

    private:
        render::DmaBufFormats m_formats;
    } device;

    wayland::Server server;
    ASSERT_TRUE(server.display());
    EXPECT_FALSE(
        wayland::LinuxDmabufGlobal::create(server.display(), m_compositor.get(), &device));
}

////////////////////////////////////////////////////////////////////////////////
// Buffer creation.

TEST_F(DmaBufTest, CreateImmedImportsTheBufferAndTheSurfaceSamplesIt)
{
    TestClient client;
    TestClient::Body body = [](ClientContext& context) {
        ClientProxy<wl_surface, wl_surface_destroy> surface(
            context.display, wl_compositor_create_surface(context.compositor));
        ASSERT_TRUE(surface.get());
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 0, 128, 1024, kModifierA);
        ClientProxy<wl_buffer, wl_buffer_destroy> buffer(context.display,
            zwp_linux_buffer_params_v1_create_immed(params, 32, 16, DRM_FORMAT_ARGB8888, 0));
        ASSERT_TRUE(buffer.get());
        params.reset();

        wl_surface_attach(surface, buffer, 0, 0);
        wl_surface_damage(surface, 0, 0, 32, 16);
        wl_surface_commit(surface);
        wl_display_roundtrip(context.display);

        // Stay connected while the test looks at the committed surface.
        context.result->reachedHold = true;
        EXPECT_TRUE(context.result->waitForRelease());

        buffer.reset();
        surface.reset();
        wl_display_roundtrip(context.display);
    };

    client.start(m_server->socketName(), std::move(body), 4);
    ASSERT_TRUE(waitFor([&client] { return client.result.reachedHold.load(); }));

    ASSERT_EQ(m_device.imports.size(), 1u);
    const auto& import = m_device.imports.front();
    EXPECT_EQ(import.size, base::Size(32, 16));
    EXPECT_EQ(import.format, uint32_t(DRM_FORMAT_ARGB8888));
    EXPECT_EQ(import.modifier, kModifierA);
    EXPECT_FALSE(import.yInvert);
    ASSERT_EQ(import.offsets.size(), 1u);
    EXPECT_EQ(import.offsets[0], 128u);
    EXPECT_EQ(import.strides[0], 1024u);

    ASSERT_EQ(m_compositor->surfaces().size(), 1u);
    compositor::Surface* surface = m_compositor->surfaces().front();
    testsupport::FakeRenderer renderer;
    m_compositor->updateTextures(renderer);
    ASSERT_TRUE(surface->node());
    ASSERT_TRUE(surface->node()->texture());
    // Sampled straight from the client's DMA-BUF: nothing was uploaded.
    EXPECT_EQ(renderer.createdTextures, 0);
    EXPECT_EQ(surface->node()->texture()->size(), base::Size(32, 16));

    client.result.release();
    ASSERT_TRUE(waitFor([&client] { return client.result.finished.load(); }));
    client.join();
    EXPECT_EQ(client.result.protocolErrorCode, -1);
}

TEST_F(DmaBufTest, CreateSendsCreatedAndAcceptsMultiplePlanes)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        zwp_linux_buffer_params_v1_add_listener(params, &paramsListener(), context.result);
        addPlane(params, 0, 0, 64, kModifierA);
        addPlane(params, 1, 4096, 64, kModifierA);
        zwp_linux_buffer_params_v1_create(params, 64, 32, DRM_FORMAT_NV12,
                                          ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_Y_INVERT);
        wl_display_roundtrip(context.display);
        params.reset();
        wl_display_roundtrip(context.display);
    }));

    EXPECT_EQ(client.result.createdEvents, 1);
    EXPECT_EQ(client.result.failedEvents, 0);
    ASSERT_EQ(m_device.imports.size(), 1u);
    EXPECT_EQ(m_device.imports.front().offsets.size(), 2u);
    EXPECT_EQ(m_device.imports.front().offsets[1], 4096u);
    EXPECT_TRUE(m_device.imports.front().yInvert);
}

TEST_F(DmaBufTest, AFailedImportSendsFailedRatherThanAProtocolError)
{
    m_device.failImports = true;

    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        zwp_linux_buffer_params_v1_add_listener(params, &paramsListener(), context.result);
        addPlane(params, 0, 0, 1024, kModifierA);
        zwp_linux_buffer_params_v1_create(params, 32, 16, DRM_FORMAT_ARGB8888, 0);
        wl_display_roundtrip(context.display);
        params.reset();
        wl_display_roundtrip(context.display);
    }));

    EXPECT_EQ(client.result.createdEvents, 0);
    EXPECT_EQ(client.result.failedEvents, 1);
    EXPECT_EQ(client.result.protocolErrorCode, -1);
}

////////////////////////////////////////////////////////////////////////////////
// Protocol errors. Each one disconnects the client, so each gets its own.

TEST_F(DmaBufTest, ZeroWidthIsInvalidDimensions)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 0, 0, 1024, kModifierA);
        zwp_linux_buffer_params_v1_create(params, 0, 16, DRM_FORMAT_ARGB8888, 0);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode,
              ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS);
}

TEST_F(DmaBufTest, TheInterlacedFlagIsRejectedAsIncomplete)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 0, 0, 1024, kModifierA);
        zwp_linux_buffer_params_v1_create(params, 32, 16, DRM_FORMAT_ARGB8888,
                                          ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_INTERLACED);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE);
}

TEST_F(DmaBufTest, NoPlanesIsIncomplete)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        zwp_linux_buffer_params_v1_create(params, 32, 16, DRM_FORMAT_ARGB8888, 0);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE);
}

TEST_F(DmaBufTest, AGapInThePlaneIndicesIsIncomplete)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 1, 0, 1024, kModifierA);
        zwp_linux_buffer_params_v1_create(params, 32, 16, DRM_FORMAT_ARGB8888, 0);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE);
}

TEST_F(DmaBufTest, SettingAPlaneTwiceIsPlaneSet)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 0, 0, 1024, kModifierA);
        addPlane(params, 0, 0, 1024, kModifierA);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_SET);
}

TEST_F(DmaBufTest, APlaneIndexBeyondTheLastPlaneIsPlaneIdx)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 4, 0, 1024, kModifierA);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX);
}

TEST_F(DmaBufTest, PlanesWithDifferentModifiersAreInvalidFormat)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 0, 0, 64, kModifierA);
        addPlane(params, 1, 4096, 64, kModifierB);
        zwp_linux_buffer_params_v1_create(params, 64, 32, DRM_FORMAT_NV12, 0);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode,
              ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_FORMAT);
}

TEST_F(DmaBufTest, AnUnadvertisedFormatIsInvalidFormat)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 0, 0, 1024, kModifierA);
        zwp_linux_buffer_params_v1_create(params, 32, 16, DRM_FORMAT_YUV420, 0);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode,
              ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_FORMAT);
    EXPECT_TRUE(m_device.imports.empty());
}

TEST_F(DmaBufTest, APlaneReachingPastItsFdIsOutOfBounds)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        // 16 rows of 1024 bytes need 16 KiB; the fd only holds 4 KiB.
        addPlane(params, 0, 0, 1024, kModifierA, /*fdSize=*/4096);
        zwp_linux_buffer_params_v1_create(params, 32, 16, DRM_FORMAT_ARGB8888, 0);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode,
              ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS);
    EXPECT_TRUE(m_device.imports.empty());
}

TEST_F(DmaBufTest, AZeroStrideIsOutOfBounds)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 0, 0, 0, kModifierA);
        zwp_linux_buffer_params_v1_create(params, 32, 16, DRM_FORMAT_ARGB8888, 0);
        wl_display_roundtrip(context.display);
    }));
    EXPECT_EQ(client.result.protocolErrorCode,
              ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS);
}

TEST_F(DmaBufTest, ReusingAParamsObjectIsAlreadyUsed)
{
    TestClient client;
    ASSERT_TRUE(runClient(client, [](ClientContext& context) {
        ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
            context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
        ASSERT_TRUE(params.get());
        addPlane(params, 0, 0, 1024, kModifierA);
        ClientProxy<wl_buffer, wl_buffer_destroy> buffer(context.display,
            zwp_linux_buffer_params_v1_create_immed(params, 32, 16, DRM_FORMAT_ARGB8888, 0));
        ASSERT_TRUE(buffer.get());
        wl_display_roundtrip(context.display);
        zwp_linux_buffer_params_v1_create(params, 32, 16, DRM_FORMAT_ARGB8888, 0);
        wl_display_roundtrip(context.display);
        buffer.reset();
    }));
    EXPECT_EQ(client.result.protocolErrorCode, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED);
    EXPECT_EQ(m_device.imports.size(), 1u);
}

TEST_F(DmaBufTest, DestructionJoinsAHeldClientAndCleansPartialObjects)
{
    std::atomic<bool> bodyReturned = false;
    {
        TestClient client;
        client.start(m_server->socketName(), [&](ClientContext& context) {
            ClientProxy<wl_surface, wl_surface_destroy> surface(
                context.display, wl_compositor_create_surface(context.compositor));
            ASSERT_TRUE(surface.get());
            ClientProxy<zwp_linux_buffer_params_v1, zwp_linux_buffer_params_v1_destroy> params(
                context.display, zwp_linux_dmabuf_v1_create_params(context.dmabuf));
            ASSERT_TRUE(params.get());
            wl_display_roundtrip(context.display);
            context.result->reachedHold = true;
            EXPECT_TRUE(context.result->waitForRelease());
            bodyReturned = true;
        }, 4);
        ASSERT_TRUE(waitFor([&] { return client.result.reachedHold.load(); }));
    }
    EXPECT_TRUE(bodyReturned);
    EXPECT_TRUE(waitFor([&] { return m_compositor->surfaces().empty(); }));
}

TEST_F(DmaBufTest, DestructionIsSafeWithoutAConnection)
{
    TestClient client;
    client.stop();
}

////////////////////////////////////////////////////////////////////////////////
// Buffer lifetime, independent of the protocol.

render::DmaBufAttributes makeAttributes(const base::Size& size)
{
    render::DmaBufAttributes attributes;
    attributes.size = size;
    attributes.format = DRM_FORMAT_ARGB8888;
    attributes.modifier = kModifierA;
    render::DmaBufPlane plane;
    plane.fd = ScopedFd(makeFakeDmaBufFd(64 * 1024));
    plane.offset = 0;
    plane.stride = static_cast<uint32_t>(size.width() * 4);
    attributes.planes.push_back(std::move(plane));
    return attributes;
}

TEST(DmaBufBufferTest, TheImportedTextureIsReusedAcrossCommits)
{
    FakeDmaBufDevice device;
    auto buffer = compositor::DmaBufBuffer::create(&device, makeAttributes(base::Size(8, 8)));
    ASSERT_TRUE(buffer);

    EXPECT_EQ(device.imports.size(), 1u);
    EXPECT_EQ(buffer->pixels(), nullptr);
    EXPECT_EQ(buffer->stride(), 32);
    ASSERT_TRUE(buffer->texture());
    EXPECT_EQ(buffer->texture()->size(), base::Size(8, 8));
    // Imported once; later commits reuse it rather than importing again.
    EXPECT_EQ(buffer->texture().get(), buffer->texture().get());
    EXPECT_EQ(buffer->attributes().modifier, kModifierA);
    EXPECT_EQ(buffer->attributes().planes.size(), 1u);
}

TEST(DmaBufBufferTest, ReleaseWaitsForDetachBecauseClientStorageIsSampled)
{
    FakeDmaBufDevice device;
    auto buffer = compositor::DmaBufBuffer::create(&device, makeAttributes(base::Size(8, 8)));
    ASSERT_TRUE(buffer);

    int releases = 0;
    buffer->setReleaseCallback([&releases] { ++releases; });

    buffer->onAttach();
    EXPECT_FALSE(buffer->isReleased());

    // Drawing a frame does not hand the storage back: the GPU keeps reading it.
    buffer->onContentsConsumed();
    EXPECT_EQ(releases, 0);
    EXPECT_FALSE(buffer->isReleased());

    buffer->onDetach();
    EXPECT_EQ(releases, 1);
    EXPECT_TRUE(buffer->isReleased());
}

TEST(DmaBufBufferTest, AFailedImportProducesNoBufferAndClosesThePlaneFds)
{
    FakeDmaBufDevice device;
    device.failImports = true;

    render::DmaBufAttributes attributes = makeAttributes(base::Size(8, 8));
    const int fd = attributes.planes.front().fd.get();
    EXPECT_GE(fcntl(fd, F_GETFD), 0);

    EXPECT_FALSE(compositor::DmaBufBuffer::create(&device, std::move(attributes)));
    // The attributes were consumed and destroyed with them, so the fd is gone.
    EXPECT_EQ(fcntl(fd, F_GETFD), -1);
    EXPECT_EQ(errno, EBADF);
}

TEST(DmaBufBufferTest, WithoutADeviceNoBufferIsCreated)
{
    EXPECT_FALSE(compositor::DmaBufBuffer::create(nullptr, makeAttributes(base::Size(8, 8))));
}

} // namespace

TEST(DmaBufBufferTest, ViewportOnlyCommitKeepsImportAndReleaseOwnership)
{
    FakeDmaBufDevice device;
    std::shared_ptr<compositor::DmaBufBuffer> buffer =
        compositor::DmaBufBuffer::create(&device, makeAttributes(base::Size(32, 16)));
    ASSERT_TRUE(buffer);
    scene::Scene scene;
    compositor::Compositor compositor(scene.root());
    auto surface = compositor.createSurface();
    testsupport::FakeRenderer renderer;
    int releases = 0;
    buffer->setReleaseCallback([&] {
        ++releases;
    });
    surface->attach(buffer, {});
    surface->commit();
    surface->updateTexture(renderer);
    auto scanout = surface->node()->scanoutBuffer();
    auto* texture = surface->node()->texture();
    surface->setCrop(base::RectF(4, 2, 16, 8));
    surface->setViewport(base::Size(32, 16));
    surface->commit();
    EXPECT_FALSE(surface->updateTexture(renderer));
    EXPECT_EQ(surface->node()->texture(), texture);
    EXPECT_EQ(surface->node()->scanoutBuffer(), scanout);
    EXPECT_FALSE(surface->node()->textureMapping().isIdentity());
    EXPECT_EQ(device.imports.size(), 1u);
    EXPECT_EQ(releases, 0);
    EXPECT_EQ(renderer.createdTextures, 0);
    surface->setCrop(std::nullopt);
    surface->setViewport(std::nullopt);
    surface->commit();
    EXPECT_TRUE(surface->node()->textureMapping().isIdentity());
    EXPECT_EQ(releases, 0);
    surface->attach(nullptr, {});
    surface->commit();
    EXPECT_EQ(releases, 1);
}
