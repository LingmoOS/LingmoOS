// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <map>
#include <ext-image-capture-source-v1-client-protocol.h>
#include <ext-image-copy-capture-v1-client-protocol.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-server-core.h>

#include "compositor/compositor.h"
#include "fake_renderer.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "session/secure_container.h"
#include "session/session_controller.h"
#include "ui/bitmap.h"
#include "wayland/protocols/image_capture.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/protocols/wl_shm.h"

using namespace Lingmo;

namespace {
class CaptureRenderer : public testsupport::FakeRenderer
{
public:
    base::Size viewportSize() const override { return {8, 6}; }

    ui::Bitmap captureFrame(const std::function<void()> &paint) override
    {
        paint();
        ui::Bitmap image(viewportSize(), ui::Bitmap::Format::Argb32Premultiplied);
        image.fill(0xff0c2238);
        return image;
    }

    // A readback the test collects by hand, as the GPU would finish it.
    struct Pending : render::PendingImage {
        ui::Bitmap image;
        int *taken = nullptr;

        ui::Bitmap take() override
        {
            ++*taken;
            return image;
        }
    };

    bool readsLater = false;
    int taken = 0;
    bool copiesOutput = false;

    bool copyOutputFrame(const base::Size &) override { return copiesOutput; }

    std::shared_ptr<render::PendingImage>
    captureFrameLater(const std::function<void()> &paint) override
    {
        if (!readsLater)
            return nullptr;
        auto pending = std::make_shared<Pending>();
        pending->image = captureFrame(paint);
        pending->taken = &taken;
        return pending;
    }
};

class ImageCaptureTest : public testing::Test
{
protected:
    wl_display *server = nullptr;
    wl_display *client = nullptr;
    wl_registry *registry = nullptr;
    wl_output *output = nullptr;
    wl_shm *shm = nullptr;
    wl_global *outputGlobal = nullptr;
    ext_output_image_capture_source_manager_v1 *sources = nullptr;
    ext_image_copy_capture_manager_v1 *copies = nullptr;
    ext_image_capture_source_v1 *source = nullptr;
    ext_image_copy_capture_session_v1 *session = nullptr;
    ext_image_copy_capture_frame_v1 *frame = nullptr;
    wl_buffer *buffer = nullptr;
    compositor::Compositor compositor {nullptr};
    scene::Scene scene;
    scene::Node *secureNode =
            scene.root()->addChild(std::make_unique<scene::Node>("secure"));
    session::SecureContainer secure {secureNode};
    session::SessionController sessionController {&secure};
    compositor::Output model;
    std::unique_ptr<wayland::ImageCaptureManager> manager;
    CaptureRenderer renderer;
    void *pixels = MAP_FAILED;
    int fd = -1;
    int ready = 0, failed = -1, done = 0, stopped = 0, releases = 0;
    uint64_t timestamp = 0;
    base::Size constraintSize;
    std::vector<base::Rect> damageRects;
    dev_t dmabufDevice = 0;
    std::map<uint32_t, std::vector<uint64_t>> dmabufFormats;
    bool repaint = false;
    bool allowed = true;

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

    void SetUp() override
    {
        server = wl_display_create();
        ASSERT_NE(server, nullptr);
        model.pixelSize = {8, 6};
        sessionController.setOutputs({{model.name, base::Rect(0, 0, 8, 6)}});
        outputGlobal = wayland::createOutputGlobal(server, &model);
        ASSERT_NE(outputGlobal, nullptr);
        ASSERT_NE(wayland::createShmGlobal(server, &compositor), nullptr);
        manager = std::make_unique<wayland::ImageCaptureManager>(
                server,
                [&](const wl_client *) {
                    return allowed;
                },
                [&] {
                    repaint = true;
                },
                [](const wl_client *) {
                    return true;
                });
        manager->setCaptureAllowed([&] {
            return sessionController.allowsCapture();
        });
        int sockets[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
        ASSERT_NE(wl_client_create(server, sockets[0]), nullptr);
        client = wl_display_connect_to_fd(sockets[1]);
        ASSERT_NE(client, nullptr);
        registry = wl_display_get_registry(client);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t version) {
                    auto &test = *static_cast<ImageCaptureTest *>(data);
                    if (!strcmp(interface, "wl_output"))
                        test.output = static_cast<wl_output *>(
                                wl_registry_bind(registry, name, &wl_output_interface, 4));
                    else if (!strcmp(interface, "wl_shm"))
                        test.shm = static_cast<wl_shm *>(
                                wl_registry_bind(registry, name, &wl_shm_interface, 1));
                    else if (!strcmp(interface, "ext_output_image_capture_source_manager_v1"))
                        test.sources = static_cast<ext_output_image_capture_source_manager_v1 *>(
                                wl_registry_bind(
                                        registry, name,
                                        &ext_output_image_capture_source_manager_v1_interface, 1));
                    else if (!strcmp(interface, "ext_image_copy_capture_manager_v1"))
                        test.copies = static_cast<ext_image_copy_capture_manager_v1 *>(
                                wl_registry_bind(registry, name,
                                                 &ext_image_copy_capture_manager_v1_interface, 1));
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        ASSERT_TRUE(roundtrip());
        ASSERT_TRUE(roundtrip());
        source = ext_output_image_capture_source_manager_v1_create_source(sources, output);
        session = ext_image_copy_capture_manager_v1_create_session(copies, source, 0);
        ext_image_copy_capture_session_v1_add_listener(session, sessionListener(), this);
        ASSERT_TRUE(roundtrip());
        renderer.setOutputContext({model.name, base::Rect(0, 0, 8, 6), {8, 6}, 1, 0, 17});
    }

    static const ext_image_copy_capture_session_v1_listener *sessionListener()
    {
        static const ext_image_copy_capture_session_v1_listener listener = {
                [](void *data, ext_image_copy_capture_session_v1 *, uint32_t w, uint32_t h) {
                    static_cast<ImageCaptureTest *>(data)->constraintSize = base::Size(w, h);
                },
                [](void *, ext_image_copy_capture_session_v1 *, uint32_t) {},
                [](void *data, ext_image_copy_capture_session_v1 *, wl_array *device) {
                    if (device->size == sizeof(dev_t))
                        memcpy(&static_cast<ImageCaptureTest *>(data)->dmabufDevice, device->data,
                               sizeof(dev_t));
                },
                [](void *data, ext_image_copy_capture_session_v1 *, uint32_t format,
                   wl_array *modifiers) {
                    auto &list = static_cast<ImageCaptureTest *>(data)->dmabufFormats[format];
                    const auto *begin = static_cast<const uint64_t *>(modifiers->data);
                    list.assign(begin, begin + modifiers->size / sizeof(uint64_t));
                },
                [](void *data, ext_image_copy_capture_session_v1 *) {
                    ++static_cast<ImageCaptureTest *>(data)->done;
                },
                [](void *data, ext_image_copy_capture_session_v1 *) {
                    ++static_cast<ImageCaptureTest *>(data)->stopped;
                }};
        return &listener;
    }

    void makeFrame(int width = 8)
    {
        fd = memfd_create("capture-test", MFD_CLOEXEC);
        ASSERT_GE(fd, 0);
        ASSERT_EQ(ftruncate(fd, 4096), 0);
        pixels = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        ASSERT_NE(pixels, MAP_FAILED);
        memset(pixels, 0xab, 4096);
        auto *pool = wl_shm_create_pool(shm, fd, 4096);
        buffer = wl_shm_pool_create_buffer(pool, 16, width, 6, 64, WL_SHM_FORMAT_ARGB8888);
        wl_shm_pool_destroy(pool);
        static const wl_buffer_listener bufferListener = {[](void *data, wl_buffer *) {
            ++static_cast<ImageCaptureTest *>(data)->releases;
        }};
        wl_buffer_add_listener(buffer, &bufferListener, this);
        frame = ext_image_copy_capture_session_v1_create_frame(session);
        static const ext_image_copy_capture_frame_v1_listener listener = {
                [](void *, ext_image_copy_capture_frame_v1 *, uint32_t) {},
                [](void *data, ext_image_copy_capture_frame_v1 *, int32_t x, int32_t y, int32_t w,
                   int32_t h) {
                    static_cast<ImageCaptureTest *>(data)->damageRects.push_back(base::Rect(x, y, w, h));
                },
                [](void *data, ext_image_copy_capture_frame_v1 *, uint32_t hi, uint32_t lo,
                   uint32_t ns) {
                    static_cast<ImageCaptureTest *>(data)->timestamp =
                            ((uint64_t(hi) << 32) | lo) * 1000000000 + ns;
                },
                [](void *data, ext_image_copy_capture_frame_v1 *) {
                    ++static_cast<ImageCaptureTest *>(data)->ready;
                },
                [](void *data, ext_image_copy_capture_frame_v1 *, uint32_t reason) {
                    static_cast<ImageCaptureTest *>(data)->failed = reason;
                }};
        ext_image_copy_capture_frame_v1_add_listener(frame, &listener, this);
        ext_image_copy_capture_frame_v1_attach_buffer(frame, buffer);
    }

    void submit()
    {
        ext_image_copy_capture_frame_v1_capture(frame);
        ASSERT_TRUE(roundtrip());
        manager->renderPending(renderer, [](bool) { });
    }

    void TearDown() override
    {
        if (frame)
            ext_image_copy_capture_frame_v1_destroy(frame);
        if (buffer)
            wl_buffer_destroy(buffer);
        if (session)
            ext_image_copy_capture_session_v1_destroy(session);
        if (source)
            ext_image_capture_source_v1_destroy(source);
        if (copies)
            ext_image_copy_capture_manager_v1_destroy(copies);
        if (sources)
            ext_output_image_capture_source_manager_v1_destroy(sources);
        if (output)
            wl_output_destroy(output);
        if (shm)
            wl_shm_destroy(shm);
        if (registry)
            wl_registry_destroy(registry);
        if (client)
            wl_display_disconnect(client);
        wl_display_destroy_clients(server);
        manager.reset();
        wl_display_destroy(server);
        if (pixels != MAP_FAILED)
            munmap(pixels, 4096);
        if (fd >= 0)
            close(fd);
    }
};

TEST_F(ImageCaptureTest, PixelsStrideAndPresentation)
{
    EXPECT_EQ(constraintSize, base::Size(8, 6));
    EXPECT_FALSE(manager->hasPending(model.name));
    makeFrame();
    submit();
    EXPECT_TRUE(manager->hasPending(model.name));
    EXPECT_FALSE(manager->hasPending("another-output"));
    EXPECT_TRUE(repaint);
    repaint = false;
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 0);
    manager->presented(16, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 0);
    manager->presented(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 1);
    EXPECT_FALSE(manager->hasPending(model.name));
    EXPECT_TRUE(repaint);
    EXPECT_EQ(timestamp, 1000000123u);
    EXPECT_EQ(releases, 0);
    EXPECT_EQ(failed, -1);
    auto *bytes = static_cast<uint8_t *>(pixels);
    EXPECT_EQ(bytes[0], 0xab);
    EXPECT_EQ(bytes[16], 56);
    EXPECT_EQ(bytes[17], 34);
    EXPECT_EQ(bytes[18], 12);
    EXPECT_EQ(bytes[19], 255);
    EXPECT_EQ(bytes[48], 0xab);
    EXPECT_EQ(bytes[16 + 5 * 64], 56);
}

TEST_F(ImageCaptureTest, AFrameInItsBufferIsReadyOnceDrawn)
{
    makeFrame();
    submit();
    manager->drawn(16, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 0);
    // Not held back for the flip, behind which the next composition is queued.
    manager->drawn(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 1);
    EXPECT_EQ(timestamp, 1000000123u);
    EXPECT_FALSE(manager->hasPending(model.name));
    manager->presented(17, 1000000456);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 1);
    EXPECT_EQ(failed, -1);
}

TEST_F(ImageCaptureTest, AReadbackWaitsForPresentationRatherThanTheDraw)
{
    renderer.readsLater = true;
    makeFrame();
    submit();
    manager->drawn(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 0);
    EXPECT_EQ(renderer.taken, 0);
    manager->presented(17, 1000000456);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 1);
    EXPECT_EQ(timestamp, 1000000456u);
}

TEST_F(ImageCaptureTest, AReadbackReachesTheBufferWhenTheFrameIsPresented)
{
    renderer.readsLater = true;
    makeFrame();
    submit();
    ASSERT_TRUE(roundtrip());
    // Nothing waited on the GPU while composing.
    EXPECT_EQ(renderer.taken, 0);
    auto *bytes = static_cast<uint8_t *>(pixels);
    EXPECT_EQ(bytes[16], 0xab);
    manager->presented(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(renderer.taken, 1);
    EXPECT_EQ(ready, 1);
    EXPECT_EQ(failed, -1);
    EXPECT_EQ(bytes[0], 0xab);
    EXPECT_EQ(bytes[16], 56);
    EXPECT_EQ(bytes[17], 34);
    EXPECT_EQ(bytes[18], 12);
    EXPECT_EQ(bytes[16 + 5 * 64], 56);
}

TEST_F(ImageCaptureTest, AReadbackWhoseBufferIsGoneByPresentationFails)
{
    renderer.readsLater = true;
    makeFrame();
    submit();
    wl_buffer_destroy(buffer);
    buffer = nullptr;
    ASSERT_TRUE(roundtrip());
    manager->presented(17, 42);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 0);
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_UNKNOWN);
}

TEST_F(ImageCaptureTest, SourceAndSessionDestructionPreserveFrame)
{
    makeFrame();
    ext_image_capture_source_v1_destroy(source);
    source = nullptr;
    ext_image_copy_capture_session_v1_destroy(session);
    session = nullptr;
    submit();
    manager->presented(17, 42);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 1);
}

TEST_F(ImageCaptureTest, OutputRemovalStopsPendingCapture)
{
    makeFrame();
    submit();
    wayland::removeOutputGlobal(outputGlobal);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(stopped, 1);
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_STOPPED);
    manager->presented(17, 42);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 0);
}

TEST_F(ImageCaptureTest, OutputChangeUpdatesConstraints)
{
    model.pixelSize = {12, 10};
    wayland::updateOutputGlobal(outputGlobal, &model);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(constraintSize, base::Size(12, 10));
    EXPECT_EQ(done, 2);
    makeFrame();
    submit();
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_BUFFER_CONSTRAINTS);
}

TEST_F(ImageCaptureTest, WrongBufferSizeFails)
{
    makeFrame(7);
    submit();
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_BUFFER_CONSTRAINTS);
}

TEST_F(ImageCaptureTest, DestroyedBufferFails)
{
    makeFrame();
    wl_buffer_destroy(buffer);
    buffer = nullptr;
    submit();
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_BUFFER_CONSTRAINTS);
}

TEST_F(ImageCaptureTest, DiscardedPresentationFails)
{
    makeFrame();
    submit();
    manager->discarded(17);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_UNKNOWN);
}

TEST_F(ImageCaptureTest, DestroyedFrameCancelsReadback)
{
    makeFrame();
    ext_image_copy_capture_frame_v1_capture(frame);
    ext_image_copy_capture_frame_v1_destroy(frame);
    frame = nullptr;
    ASSERT_TRUE(roundtrip());
    manager->renderPending(renderer, [](bool) {
        FAIL() << "cancelled frame rendered";
    });
    manager->presented(17, 42);
}

TEST_F(ImageCaptureTest, LaterFramesWaitForACompositionInsteadOfForcingOne)
{
    makeFrame();
    submit();
    EXPECT_TRUE(repaint);
    manager->presented(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(ready, 1);
    repaint = false;

    // A video consumer asks again at once; a static screen must not be
    // recomposed for it.
    ext_image_copy_capture_frame_v1_destroy(frame);
    frame = nullptr;
    wl_buffer_destroy(buffer);
    buffer = nullptr;
    munmap(pixels, 4096);
    pixels = MAP_FAILED;
    close(fd);
    fd = -1;
    makeFrame();
    ext_image_copy_capture_frame_v1_capture(frame);
    ASSERT_TRUE(roundtrip());
    EXPECT_FALSE(repaint);
    EXPECT_TRUE(manager->hasPending(model.name));
    // Nor is cursor motion damage to a session that does not paint cursors.
    manager->cursorChanged();
    EXPECT_FALSE(repaint);

    manager->renderPending(renderer, [](bool) { });
    manager->presented(17, 1000000456);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 2);
}

TEST_F(ImageCaptureTest, CursorMotionIsDamageToACursorPaintingSession)
{
    auto *withCursors = ext_image_copy_capture_manager_v1_create_session(
            copies, source, EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_OPTIONS_PAINT_CURSORS);
    ASSERT_TRUE(roundtrip());
    fd = memfd_create("capture-test", MFD_CLOEXEC);
    ASSERT_EQ(ftruncate(fd, 4096), 0);
    auto *pool = wl_shm_create_pool(shm, fd, 4096);
    buffer = wl_shm_pool_create_buffer(pool, 0, 8, 6, 32, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    int readyFrames = 0;
    static const ext_image_copy_capture_frame_v1_listener listener = {
            [](void *, ext_image_copy_capture_frame_v1 *, uint32_t) {},
            [](void *, ext_image_copy_capture_frame_v1 *, int32_t, int32_t, int32_t, int32_t) {},
            [](void *, ext_image_copy_capture_frame_v1 *, uint32_t, uint32_t, uint32_t) {},
            [](void *data, ext_image_copy_capture_frame_v1 *) {
                ++*static_cast<int *>(data);
            },
            [](void *, ext_image_copy_capture_frame_v1 *, uint32_t) {}};
    auto capture = [&] {
        auto *cursorFrame = ext_image_copy_capture_session_v1_create_frame(withCursors);
        ext_image_copy_capture_frame_v1_add_listener(cursorFrame, &listener, &readyFrames);
        ext_image_copy_capture_frame_v1_attach_buffer(cursorFrame, buffer);
        ext_image_copy_capture_frame_v1_capture(cursorFrame);
        return cursorFrame;
    };

    auto *first = capture();
    ASSERT_TRUE(roundtrip());
    EXPECT_TRUE(repaint);
    manager->renderPending(renderer, [](bool) { });
    manager->presented(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(readyFrames, 1);
    ext_image_copy_capture_frame_v1_destroy(first);
    repaint = false;

    auto *second = capture();
    ASSERT_TRUE(roundtrip());
    EXPECT_FALSE(repaint);
    manager->cursorChanged();
    EXPECT_TRUE(repaint);
    ext_image_copy_capture_frame_v1_destroy(second);
    ext_image_copy_capture_session_v1_destroy(withCursors);
    ASSERT_TRUE(roundtrip());
}

TEST_F(ImageCaptureTest, DamageIsReportedSinceTheSessionsPreviousFrame)
{
    // The tree an output session tracks damage against.
    scene::Node root("root");
    root.setGeometry(base::Rect(0, 0, 8, 6));
    auto *child = root.addChild(std::make_unique<scene::Node>("child"));
    child->setGeometry(base::Rect(0, 0, 8, 6));
    wayland::ImageCaptureManager::Painters painters;
    painters.root = &root;
    painters.scene = [](bool) { };

    makeFrame();
    ext_image_copy_capture_frame_v1_capture(frame);
    ASSERT_TRUE(roundtrip());
    manager->renderPending(renderer, painters);
    manager->presented(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(ready, 1);
    // Nothing was known before the first frame: all of it changed.
    ASSERT_EQ(damageRects.size(), 1u);
    EXPECT_EQ(damageRects.front(), base::Rect(0, 0, 8, 6));

    damageRects.clear();
    ext_image_copy_capture_frame_v1_destroy(frame);
    frame = nullptr;
    wl_buffer_destroy(buffer);
    buffer = nullptr;
    munmap(pixels, 4096);
    pixels = MAP_FAILED;
    close(fd);
    fd = -1;
    child->notifyDamage(base::Region(base::Rect(1, 1, 2, 2)));
    makeFrame();
    ext_image_copy_capture_frame_v1_capture(frame);
    ASSERT_TRUE(roundtrip());
    manager->renderPending(renderer, painters);
    manager->presented(17, 1000000456);
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(ready, 2);
    ASSERT_EQ(damageRects.size(), 1u);
    EXPECT_EQ(damageRects.front(), base::Rect(1, 1, 2, 2));
}

TEST_F(ImageCaptureTest, ACopiedOutputFrameStandsInOnlyWhenItHoldsWhatTheSessionPaints)
{
    using Composited = wayland::ImageCaptureManager::Composited;
    scene::Node root("root");
    root.setGeometry(base::Rect(0, 0, 8, 6));
    int scenes = 0, cursors = 0;
    Composited composited = Composited::Unusable;
    wayland::ImageCaptureManager::Painters painters;
    painters.root = &root;
    painters.scene = [&](bool) { ++scenes; };
    painters.cursors = [&] { ++cursors; };
    painters.composited = [&] { return composited; };
    renderer.copiesOutput = true;

    fd = memfd_create("capture-test", MFD_CLOEXEC);
    ASSERT_EQ(ftruncate(fd, 4096), 0);
    auto *pool = wl_shm_create_pool(shm, fd, 4096);
    buffer = wl_shm_pool_create_buffer(pool, 0, 8, 6, 32, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    // Paints one frame of a fresh session and returns {scene paints, cursor paints}.
    auto capture = [&](uint32_t options) {
        scenes = cursors = 0;
        auto *paintSession =
                ext_image_copy_capture_manager_v1_create_session(copies, source, options);
        auto *paintFrame = ext_image_copy_capture_session_v1_create_frame(paintSession);
        ext_image_copy_capture_frame_v1_attach_buffer(paintFrame, buffer);
        ext_image_copy_capture_frame_v1_capture(paintFrame);
        EXPECT_TRUE(roundtrip());
        manager->renderPending(renderer, painters);
        ext_image_copy_capture_frame_v1_destroy(paintFrame);
        ext_image_copy_capture_session_v1_destroy(paintSession);
        EXPECT_TRUE(roundtrip());
        return std::pair(scenes, cursors);
    };
    const uint32_t withCursors = EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_OPTIONS_PAINT_CURSORS;

    composited = Composited::Unusable;
    EXPECT_EQ(capture(0), std::pair(1, 0));
    composited = Composited::WithoutCursor;
    EXPECT_EQ(capture(0), std::pair(0, 0));
    EXPECT_EQ(capture(withCursors), std::pair(0, 1));
    // A frame with a software cursor in it is no capture for a session without one.
    composited = Composited::WithCursor;
    EXPECT_EQ(capture(0), std::pair(1, 0));
    EXPECT_EQ(capture(withCursors), std::pair(0, 0));

    renderer.copiesOutput = false;
    composited = Composited::WithoutCursor;
    EXPECT_EQ(capture(0), std::pair(1, 0));
}

TEST_F(ImageCaptureTest, DmaBufTargetsAreAdvertisedFromTheRendererProbe)
{
    struct DmaBufRenderer : CaptureRenderer {
        const render::DmaBufFormats &captureDmaBufFormats() const override
        {
            static const render::DmaBufFormats formats {{0x34325241u, {0u}}};
            return formats;
        }

        dev_t captureDmaBufDevice() const override { return 42; }
    } withDmaBuf;
    // Sessions created before there was a renderer stay SHM-only.
    EXPECT_EQ(dmabufDevice, 0u);
    EXPECT_TRUE(dmabufFormats.empty());

    manager->setRenderer(&withDmaBuf);
    // The fixture's session listener records the constraints of any session.
    auto *second = ext_image_copy_capture_manager_v1_create_session(copies, source, 0);
    ext_image_copy_capture_session_v1_add_listener(second, sessionListener(), this);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(dmabufDevice, 42u);
    ASSERT_EQ(dmabufFormats.size(), 1u);
    EXPECT_EQ(dmabufFormats.begin()->first, 0x34325241u);
    EXPECT_EQ(dmabufFormats.begin()->second, std::vector<uint64_t> {0u});
    ext_image_copy_capture_session_v1_destroy(second);
    ASSERT_TRUE(roundtrip());
}

TEST_F(ImageCaptureTest, RevokedAuthorizationFails)
{
    makeFrame();
    allowed = false;
    submit();
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_STOPPED);
}

// The computer-use service binds the managers before the user lets it capture.
class ImageCaptureBeforeConsentTest : public ImageCaptureTest
{
protected:
    void SetUp() override
    {
        allowed = false;
        ImageCaptureTest::SetUp();
    }
};

TEST_F(ImageCaptureBeforeConsentTest, ABoundClientCapturesOnlyOnceAuthorized)
{
    EXPECT_EQ(stopped, 1);
    EXPECT_EQ(wl_display_get_error(client), 0);

    allowed = true;
    ext_image_copy_capture_session_v1_destroy(session);
    ext_image_capture_source_v1_destroy(source);
    source = ext_output_image_capture_source_manager_v1_create_source(sources, output);
    session = ext_image_copy_capture_manager_v1_create_session(copies, source, 0);
    ext_image_copy_capture_session_v1_add_listener(session, sessionListener(), this);
    ASSERT_TRUE(roundtrip());
    makeFrame();
    submit();
    manager->presented(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 1);
    EXPECT_EQ(failed, -1);
}

TEST_F(ImageCaptureTest, ALockedSessionHoldsCaptureUntilUnlocked)
{
    makeFrame();
    ASSERT_TRUE(sessionController.requestLock());
    submit();
    ASSERT_TRUE(roundtrip());
    // Not rendered rather than filtered: no capture path has to be trusted to
    // leave the desktop out of the frame. The request outlives the lock.
    EXPECT_EQ(failed, -1);
    EXPECT_EQ(ready, 0);
    EXPECT_TRUE(manager->hasPending(model.name));
    manager->presented(17, 42);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 0);

    sessionController.notifyFramePresented();
    ASSERT_TRUE(sessionController.requestUnlock());
    sessionController.notifyFramePresented();
    ASSERT_TRUE(sessionController.allowsCapture());
    manager->renderPending(renderer, [](bool) { });
    manager->presented(17, 42);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 1);
}

TEST_F(ImageCaptureTest, ACaptureInFlightWhenLockingStartsFails)
{
    // Asked for, rendered and copied while the session was still active: the
    // frame is finished in every sense except being handed over.
    makeFrame();
    submit();
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(ready, 0);
    EXPECT_TRUE(manager->hasPending(model.name));

    ASSERT_TRUE(sessionController.requestLock());
    manager->failPending();
    ASSERT_TRUE(roundtrip());

    // Rejecting only new requests would let this one complete after the screen
    // was supposed to be secure. The session lives on, so the client may retry.
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_UNKNOWN);
    EXPECT_EQ(ready, 0);
    EXPECT_FALSE(manager->hasPending(model.name));

    // And the presentation it was waiting for cannot revive it.
    manager->presented(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 0);
}

TEST_F(ImageCaptureTest, CaptureResumesOnceTheSessionIsUnlocked)
{
    ASSERT_TRUE(sessionController.requestLock());
    sessionController.notifyFramePresented();
    ASSERT_TRUE(sessionController.requestUnlock());
    sessionController.notifyFramePresented();

    makeFrame();
    submit();
    manager->presented(17, 1000000123);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 1);
    EXPECT_EQ(failed, -1);
}

TEST_F(ImageCaptureTest, DuplicateFrameIsProtocolError)
{
    makeFrame();
    auto *duplicate = ext_image_copy_capture_session_v1_create_frame(session);
    EXPECT_FALSE(roundtrip());
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              EXT_IMAGE_COPY_CAPTURE_SESSION_V1_ERROR_DUPLICATE_FRAME);
    ext_image_copy_capture_frame_v1_destroy(duplicate);
}

TEST_F(ImageCaptureTest, RepeatedCaptureIsProtocolError)
{
    makeFrame();
    ext_image_copy_capture_frame_v1_capture(frame);
    ext_image_copy_capture_frame_v1_capture(frame);
    EXPECT_FALSE(roundtrip());
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              EXT_IMAGE_COPY_CAPTURE_FRAME_V1_ERROR_ALREADY_CAPTURED);
}

TEST_F(ImageCaptureTest, InvalidDamageIsProtocolError)
{
    makeFrame();
    ext_image_copy_capture_frame_v1_damage_buffer(frame, -1, 0, 1, 1);
    EXPECT_FALSE(roundtrip());
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              EXT_IMAGE_COPY_CAPTURE_FRAME_V1_ERROR_INVALID_BUFFER_DAMAGE);
}

TEST_F(ImageCaptureTest, TruncatedDestinationFailsWithoutExtendingFile)
{
    makeFrame();
    ASSERT_TRUE(roundtrip());
    ASSERT_EQ(ftruncate(fd, 0), 0);
    submit();
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_UNKNOWN);
    EXPECT_EQ(lseek(fd, 0, SEEK_END), 0);
}

TEST_F(ImageCaptureTest, ManagerShutdownStopsPendingCapture)
{
    makeFrame();
    submit();
    manager.reset();
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(stopped, 1);
    EXPECT_EQ(failed, EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_STOPPED);
}

TEST_F(ImageCaptureTest, CaptureWithoutBufferIsProtocolError)
{
    auto *empty = ext_image_copy_capture_session_v1_create_frame(session);
    ext_image_copy_capture_frame_v1_capture(empty);
    EXPECT_FALSE(roundtrip());
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              EXT_IMAGE_COPY_CAPTURE_FRAME_V1_ERROR_NO_BUFFER);
    ext_image_copy_capture_frame_v1_destroy(empty);
}

TEST_F(ImageCaptureTest, InvalidOptionsAreProtocolError)
{
    auto *invalid = ext_image_copy_capture_manager_v1_create_session(copies, source, 2);
    EXPECT_FALSE(roundtrip());
    EXPECT_EQ(wl_display_get_protocol_error(client, nullptr, nullptr),
              EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_ERROR_INVALID_OPTION);
    ext_image_copy_capture_session_v1_destroy(invalid);
}

TEST_F(ImageCaptureTest, FrameDestructionDuringPaintCancelsCompletion)
{
    makeFrame();
    ext_image_copy_capture_frame_v1_capture(frame);
    ASSERT_TRUE(roundtrip());
    manager->renderPending(renderer, [&](bool) {
        ext_image_copy_capture_frame_v1_destroy(frame);
        frame = nullptr;
        EXPECT_TRUE(roundtrip());
    });
    manager->presented(17, 42);
    ASSERT_TRUE(roundtrip());
    EXPECT_EQ(ready, 0);
    EXPECT_EQ(static_cast<uint8_t *>(pixels)[16], 0xab);
}

} // namespace
