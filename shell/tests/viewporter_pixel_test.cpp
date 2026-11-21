// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <string>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <gbm.h>
#include <gtest/gtest.h>
#include <vector>

#include "compositor/compositor.h"
#include "compositor/dmabuf_buffer.h"
#include "compositor/shm_pool.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "input/pointer.h"
#include "input/pointer_delegate.h"
#include "input/seat.h"
#include "render/egl_renderer.h"
#include "render/gl_texture.h"
#include "scene/scene.h"
#include "shm_test_util.h"

#include "egl_test_util.h"

using namespace Lingmo;

namespace {

class ViewporterPixelTest : public testing::Test
{
protected:
    static constexpr int width = 128;
    static constexpr int height = 128;
    std::unique_ptr<render::EglRenderer> renderer;
    std::unique_ptr<render::EglRenderTarget> target;

    virtual int alphaSize() const { return 8; }

    void SetUp() override
    {
        renderer = render::EglRendererTestAccess::create(alphaSize());
        ASSERT_TRUE(renderer);
        const EGLint attributes[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};
        const EGLSurface surface =
                eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attributes);
        ASSERT_NE(surface, EGL_NO_SURFACE);
        target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), surface, nullptr,
                                                           base::Size(width, height));
        ASSERT_TRUE(renderer->beginFrame(target.get()));
    }

    void TearDown() override
    {
        if (renderer) {
            renderer->clearTextureCache();
            renderer->releaseTarget(target.get());
        }
        target.reset();
        renderer.reset();
    }

    std::vector<uint8_t> pixels()
    {
        std::vector<uint8_t> result(width * height * 4);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, result.data());
        EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
        return result;
    }

    int channel(const std::vector<uint8_t> &image, int x, int y, int c = 0)
    {
        return image[((height - 1 - y) * width + x) * 4 + c];
    }
};

TEST_F(ViewporterPixelTest, OutputContextChangedAfterBeginFrameUpdatesOriginAndViewport)
{
    renderer->setOutputContext(
            {"external", {200, 100, 32, 32}, {128, 128}, 2.0, 0, 1, {32, 32, 64, 64}});
    renderer->clear({0, 0, 0, 1});
    renderer->fillRect({200, 100, 8, 8}, {1, 0, 0, 1});
    const auto image = pixels();
    EXPECT_EQ(channel(image, 33, 33), 255);
    EXPECT_EQ(channel(image, 47, 47), 255);
    EXPECT_EQ(channel(image, 48, 48), 0);
    EXPECT_EQ(channel(image, 1, 1), 0);
}

TEST_F(ViewporterPixelTest, OffscreenCaptureRestoresLetterboxViewport)
{
    renderer->setOutputContext(
            {"mirror", {200, 100, 32, 32}, {128, 128}, 2.0, 0, 1, {32, 32, 64, 64}});
    renderer->clear({0, 0, 0, 1});
    std::unique_ptr<render::Texture> snapshot;
    ASSERT_TRUE(renderer->renderToTexture(snapshot, {200, 100, 8, 8}, [&] {
        renderer->fillRect({200, 100, 8, 8}, {1, 0, 0, 1});
    }));
    renderer->drawTexture(snapshot.get(), {200, 100, 8, 8}, 1);
    const auto image = pixels();
    EXPECT_EQ(channel(image, 33, 33), 255);
    EXPECT_EQ(channel(image, 1, 65), 0);
}

TEST_F(ViewporterPixelTest, RoundedClipUsesLetterboxOriginAndRotation)
{
    for (uint32_t rotation = 0; rotation < 4; ++rotation) {
        renderer->setOutputContext(
                {"mirror", {200, 100, 32, 32}, {128, 128}, 2.0, rotation, 1, {32, 32, 64, 64}});
        renderer->clear({0, 0, 0, 1});
        renderer->pushRoundedClip({base::RectF(200, 100, 32, 32), 4});
        renderer->fillRect({200, 100, 32, 32}, {1, 0, 0, 1});
        renderer->popRoundedClip();
        const auto image = pixels();
        EXPECT_EQ(channel(image, 64, 64), 255) << rotation;
        EXPECT_EQ(channel(image, 32, 32), 0) << rotation;
        EXPECT_EQ(channel(image, 1, 1), 0) << rotation;
    }
}

TEST_F(ViewporterPixelTest, OffscreenCaptureDoesNotRecordPresentation)
{
    std::vector<render::PresentationResult> results;
    scene::Scene scene;
    scene.setSize({width, height});
    compositor::Compositor compositor(scene.root());
    auto surface = compositor.createSurface();
    auto pool = testsupport::makePool(4096);
    auto buffer = pool->createBuffer({16, 16}, 0, 64, render::PixelFormat::Rgba8888, nullptr);
    surface->attach(buffer, {});
    surface->requestPresentationCallback([&](const auto &r) {
        results.push_back(r);
    });
    surface->commit();
    surface->updateTexture(*renderer);
    render::PresentationSubmission submission;
    renderer->presentationSubmission = &submission;
    std::unique_ptr<render::Texture> snapshot;
    ASSERT_TRUE(renderer->renderToTexture(snapshot, {0, 0, 32, 32}, [&] {
        scene.render(*renderer);
    }));
    render::PresentationResult result;
    result.presented = true;
    submission.complete(result);
    EXPECT_TRUE(results.empty());
    scene.render(*renderer);
    submission.complete(result);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_TRUE(results[0].valid());
    renderer->presentationSubmission = nullptr;
}

TEST_F(ViewporterPixelTest, ShmGridAllTransformsScalesAndFractionalCrops)
{
    std::vector<uint8_t> storage(128 * 64 * 4);
    auto *data = storage.data();
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 128; ++x) {
            const int index = (y * 128 + x) * 4;
            data[index] = x * 2;
            data[index + 1] = y * 3;
            data[index + 2] = 73;
            data[index + 3] = 255;
        }
    }
    const int fd = testsupport::makeMemfd(storage.size());
    ASSERT_GE(fd, 0);
    ASSERT_EQ(pwrite(fd, storage.data(), storage.size(), 0), ssize_t(storage.size()));
    auto pool = compositor::ShmPool::create(fd, storage.size());
    ASSERT_TRUE(pool);
    auto buffer = pool->createBuffer({128, 64}, 0, 512, render::PixelFormat::Rgba8888, nullptr);
    scene::Scene scene;
    scene.setSize({width, height});
    compositor::Compositor compositor(scene.root());
    auto surface = compositor.createSurface();
    surface->attach(buffer, {});
    for (int scale : {1, 2}) {
        surface->setBufferScale(scale);
        surface->setCrop(base::RectF(2.25, 6.25, 26.5, 2.5));
        for (const base::Size destination : {base::Size(79, 11), base::Size(13, 1), base::Size(26, 5)}) {
            surface->setViewport(destination);
            for (int i = 0; i < 8; ++i) {
                SCOPED_TRACE(::testing::Message() << "transform=" << i << " scale=" << scale
                                                  << " destination=" << destination.width() << 'x'
                                                  << destination.height());
                surface->setBufferTransform(static_cast<compositor::Transform>(i));
                surface->commit();
                surface->updateTexture(*renderer);
                renderer->clear({0, 0, 0, 0});
                scene.root()->render(*renderer);
                const auto image = pixels();
                for (int y = 0; y < destination.height(); ++y) {
                    for (int x = 0; x < destination.width(); ++x) {
                        // Independent inverse transform oracle in raw pixel coordinates.
                        const double u = (2.25 + (x + 0.5) * 26.5 / destination.width()) * scale;
                        const double v = (6.25 + (y + 0.5) * 2.5 / destination.height()) * scale;
                        const base::PointF raw[] = {{u, v},      {128 - v, u},     {128 - u, 64 - v},
                                               {v, 64 - u}, {128 - u, v},     {v, u},
                                               {u, 64 - v}, {128 - v, 64 - u}};
                        EXPECT_NEAR(channel(image, x, y, 0), (raw[i].x() - 0.5) * 2, 2);
                        EXPECT_NEAR(channel(image, x, y, 1), (raw[i].y() - 0.5) * 3, 2);
                        EXPECT_EQ(channel(image, x, y, 2), 73);
                        EXPECT_EQ(channel(image, x, y, 3), 255);
                    }
                }
                EXPECT_EQ(channel(image, destination.width(), 0, 3), 0);
            }
        }
    }
}

TEST_F(ViewporterPixelTest, FractionalOutputAllClientTransformsAndCrops)
{
    for (double outputScale : {1.25, 1.5, 1.75, 2.0}) {
        renderer->setRasterScale(outputScale);
        std::vector<uint8_t> storage(128 * 64 * 4);
        auto *data = storage.data();
        for (int y = 0; y < 64; ++y) {
            for (int x = 0; x < 128; ++x) {
                const int index = (y * 128 + x) * 4;
                data[index] = x * 2;
                data[index + 1] = y * 3;
                data[index + 2] = 73;
                data[index + 3] = 255;
            }
        }
        const int fd = testsupport::makeMemfd(storage.size());
        ASSERT_GE(fd, 0);
        ASSERT_EQ(pwrite(fd, storage.data(), storage.size(), 0), ssize_t(storage.size()));
        auto pool = compositor::ShmPool::create(fd, storage.size());
        ASSERT_TRUE(pool);
        auto buffer = pool->createBuffer({128, 64}, 0, 512, render::PixelFormat::Rgba8888, nullptr);
        scene::Scene scene;
        scene.setSize({width, height});
        compositor::Compositor compositor(scene.root());
        auto surface = compositor.createSurface();
        surface->attach(buffer, {});
        for (int scale : {1, 2}) {
            surface->setBufferScale(scale);
            surface->setCrop(base::RectF(2.25, 6.25, 26.5, 2.5));
            for (const base::Size destination : {base::Size(39, 11), base::Size(13, 1), base::Size(26, 5)}) {
                surface->setViewport(destination);
                for (int i = 0; i < 8; ++i) {
                    SCOPED_TRACE(::testing::Message() << "transform=" << i << " scale=" << scale
                                                      << " destination=" << destination.width()
                                                      << 'x' << destination.height());
                    surface->setBufferTransform(static_cast<compositor::Transform>(i));
                    surface->commit();
                    surface->updateTexture(*renderer);
                    renderer->clear({0, 0, 0, 0});
                    scene.root()->render(*renderer);
                    const auto image = pixels();
                    for (int y = 0; y < int(destination.height() * outputScale); ++y) {
                        for (int x = 0; x < int(destination.width() * outputScale); ++x) {
                            // Independent inverse transform oracle in raw pixel coordinates.
                            const double u =
                                    (2.25 + (x + 0.5) / outputScale * 26.5 / destination.width())
                                    * scale;
                            const double v =
                                    (6.25 + (y + 0.5) / outputScale * 2.5 / destination.height())
                                    * scale;
                            const base::PointF raw[] = {{u, v},      {128 - v, u},     {128 - u, 64 - v},
                                                   {v, 64 - u}, {128 - u, v},     {v, u},
                                                   {u, 64 - v}, {128 - v, 64 - u}};
                            EXPECT_NEAR(channel(image, x, y, 0), (raw[i].x() - 0.5) * 2, 2);
                            EXPECT_NEAR(channel(image, x, y, 1), (raw[i].y() - 0.5) * 3, 2);
                            EXPECT_EQ(channel(image, x, y, 2), 73);
                            EXPECT_EQ(channel(image, x, y, 3), 255);
                        }
                    }
                    EXPECT_EQ(
                            channel(image, int(std::ceil(destination.width() * outputScale)), 0, 3),
                            0);
                }
            }
        }
    }
}

TEST_F(ViewporterPixelTest, ParentViewportDoesNotCropChildAndSnapshotRetainsSampling)
{
    std::vector<uint8_t> storage(16 * 8 * 4);
    auto *data = storage.data();
    for (int i = 0; i < 16 * 8; ++i) {
        data[i * 4] = (i % 16 < 8) ? 255 : 0;
        data[i * 4 + 1] = (i % 16 < 8) ? 0 : 255;
        data[i * 4 + 2] = 0;
        data[i * 4 + 3] = 255;
    }
    const int fd = testsupport::makeMemfd(storage.size());
    ASSERT_GE(fd, 0);
    ASSERT_EQ(pwrite(fd, storage.data(), storage.size(), 0), ssize_t(storage.size()));
    auto pool = compositor::ShmPool::create(fd, storage.size());
    ASSERT_TRUE(pool);
    auto buffer = pool->createBuffer({16, 8}, 0, 64, render::PixelFormat::Rgba8888, nullptr);
    scene::Scene scene;
    scene.setSize({width, height});
    compositor::Compositor compositor(scene.root());
    auto parent = compositor.createSurface();
    auto child = compositor.createSurface();
    auto sub = compositor::SubSurface::create(child.get(), parent.get());
    sub->setPosition({30, 20});
    parent->attach(buffer, {});
    parent->setCrop(base::RectF(8, 0, 8, 8));
    parent->setViewport(base::Size(20, 20));
    child->attach(buffer, {});
    child->setCrop(base::RectF(0, 0, 8, 8));
    child->setViewport(base::Size(16, 16));
    child->commit();
    parent->commit();
    parent->updateTexture(*renderer);
    child->updateTexture(*renderer);
    renderer->clear({0, 0, 0, 0});
    scene.root()->render(*renderer);
    auto image = pixels();
    EXPECT_EQ(channel(image, 10, 10, 1), 255);
    EXPECT_EQ(channel(image, 38, 28, 0), 255);
    EXPECT_EQ(channel(image, 25, 15, 3), 0);
    std::unique_ptr<render::Texture> snapshot;
    ASSERT_TRUE(renderer->renderToTexture(snapshot, base::Rect(0, 0, 50, 40), [&] {
        scene.root()->render(*renderer);
    }));
    renderer->clear({0, 0, 0, 0});
    renderer->drawTexture(snapshot.get(), base::Rect(0, 0, 100, 80), 1);
    image = pixels();
    EXPECT_EQ(channel(image, 20, 20, 1), 255);
    EXPECT_EQ(channel(image, 76, 56, 0), 255);
}

TEST_F(ViewporterPixelTest, FractionalOutputParentChildMappingsAndSnapshot)
{
    renderer->setRasterScale(1.25);
    std::vector<uint8_t> storage(16 * 8 * 4);
    auto *data = storage.data();
    for (int i = 0; i < 16 * 8; ++i) {
        data[i * 4] = (i % 16 < 8) ? 255 : 0;
        data[i * 4 + 1] = (i % 16 < 8) ? 0 : 255;
        data[i * 4 + 2] = 0;
        data[i * 4 + 3] = 255;
    }
    const int fd = testsupport::makeMemfd(storage.size());
    ASSERT_GE(fd, 0);
    ASSERT_EQ(pwrite(fd, storage.data(), storage.size(), 0), ssize_t(storage.size()));
    auto pool = compositor::ShmPool::create(fd, storage.size());
    ASSERT_TRUE(pool);
    auto buffer = pool->createBuffer({16, 8}, 0, 64, render::PixelFormat::Rgba8888, nullptr);
    scene::Scene scene;
    scene.setSize({width, height});
    compositor::Compositor compositor(scene.root());
    auto parent = compositor.createSurface();
    auto child = compositor.createSurface();
    auto sub = compositor::SubSurface::create(child.get(), parent.get());
    sub->setPosition({30, 20});
    parent->attach(buffer, {});
    parent->setCrop(base::RectF(8, 0, 8, 8));
    parent->setViewport(base::Size(20, 20));
    child->attach(buffer, {});
    child->setCrop(base::RectF(0, 0, 8, 8));
    child->setViewport(base::Size(16, 16));
    child->commit();
    parent->commit();
    parent->updateTexture(*renderer);
    child->updateTexture(*renderer);
    renderer->clear({0, 0, 0, 0});
    scene.root()->render(*renderer);
    auto image = pixels();
    EXPECT_EQ(channel(image, 12, 12, 1), 255);
    EXPECT_EQ(channel(image, 47, 35, 0), 255);
    EXPECT_EQ(channel(image, 31, 18, 3), 0);
    std::unique_ptr<render::Texture> snapshot;
    ASSERT_TRUE(renderer->renderToTexture(snapshot, base::Rect(0, 0, 50, 40), [&] {
        scene.root()->render(*renderer);
    }));
    renderer->clear({0, 0, 0, 0});
    renderer->drawTexture(snapshot.get(), base::Rect(0, 0, 100, 80), 1);
    image = pixels();
    EXPECT_EQ(channel(image, 25, 25, 1), 255);
    EXPECT_EQ(channel(image, 95, 70, 0), 255);
}

TEST_F(ViewporterPixelTest, ViewportBlurAndAnimationCaptureAgree)
{
    auto pool = testsupport::makePool(16 * 8 * 4);
    auto buffer = pool->createBuffer({16, 8}, 0, 64, render::PixelFormat::Rgba8888, nullptr);
    scene::Scene scene;
    compositor::Compositor compositor(scene.root());
    auto surface = compositor.createSurface();
    surface->attach(buffer, {});
    surface->setCrop(base::RectF(2.5, 1.5, 10, 5));
    surface->setViewport(base::Size(80, 60));
    surface->setBackgroundBlur(base::Rect(0, 0, 80, 60), 10);
    surface->commit();
    surface->setPosition({20, 30});
    surface->updateTexture(*renderer);
    const auto background = [&] {
        renderer->clear({0, 0, 0, 1});
        renderer->fillRect({64, 0, 64, 128}, {1, 0, 0, 1});
    };
    background();
    scene.root()->render(*renderer);
    const auto direct = pixels();
    background();
    std::unique_ptr<render::Texture> snapshot;
    ASSERT_TRUE(renderer->renderToTexture(snapshot, base::Rect(0, 0, 128, 128), [&] {
        scene.root()->render(*renderer);
    }));
    renderer->drawTexture(snapshot.get(), base::Rect(0, 0, 128, 128), 1);
    const auto captured = pixels();
    for (base::Point point : {base::Point(64, 60), base::Point(59, 60), base::Point(69, 60), base::Point(64, 29)}) {
        for (int c = 0; c < 4; ++c)
            EXPECT_NEAR(channel(direct, point.x(), point.y(), c),
                        channel(captured, point.x(), point.y(), c), 3);
    }
    EXPECT_GT(channel(captured, 64, 60), 100);
    EXPECT_LT(channel(captured, 64, 60), 160);
    EXPECT_EQ(channel(captured, 64, 29), 255);
}

TEST_F(ViewporterPixelTest, FractionalOutputBlurCaptureAgree)
{
    renderer->setRasterScale(1.5);
    auto pool = testsupport::makePool(16 * 8 * 4);
    auto buffer = pool->createBuffer({16, 8}, 0, 64, render::PixelFormat::Rgba8888, nullptr);
    scene::Scene scene;
    compositor::Compositor compositor(scene.root());
    auto surface = compositor.createSurface();
    surface->attach(buffer, {});
    surface->setCrop(base::RectF(2.5, 1.5, 10, 5));
    surface->setViewport(base::Size(80, 60));
    surface->setBackgroundBlur(base::Rect(0, 0, 80, 60), 10);
    surface->commit();
    surface->setPosition({20, 30});
    surface->updateTexture(*renderer);
    const auto background = [&] {
        renderer->clear({0, 0, 0, 1});
        renderer->fillRect({64, 0, 64, 128}, {1, 0, 0, 1});
    };
    background();
    scene.root()->render(*renderer);
    const auto direct = pixels();
    background();
    std::unique_ptr<render::Texture> snapshot;
    ASSERT_TRUE(renderer->renderToTexture(snapshot, base::Rect(0, 0, 128, 128), [&] {
        scene.root()->render(*renderer);
    }));
    renderer->drawTexture(snapshot.get(), base::Rect(0, 0, 128, 128), 1);
    const auto captured = pixels();
    for (base::Point point : {base::Point(96, 90), base::Point(89, 90), base::Point(104, 90), base::Point(96, 43)}) {
        for (int c = 0; c < 4; ++c)
            EXPECT_NEAR(channel(direct, point.x(), point.y(), c),
                        channel(captured, point.x(), point.y(), c), 3);
    }
    EXPECT_GT(channel(captured, 96, 90), 100);
    EXPECT_LT(channel(captured, 96, 90), 160);
    EXPECT_EQ(channel(captured, 96, 43), 255);
}

class FractionalRgbPixelTest : public ViewporterPixelTest
{
    int alphaSize() const override { return 0; }
};

TEST_F(FractionalRgbPixelTest, FractionalOutputBlurCaptureAgree)
{
    renderer->setRasterScale(1.5);
    auto pool = testsupport::makePool(16 * 8 * 4);
    auto buffer = pool->createBuffer({16, 8}, 0, 64, render::PixelFormat::Rgba8888, nullptr);
    scene::Scene scene;
    compositor::Compositor compositor(scene.root());
    auto surface = compositor.createSurface();
    surface->attach(buffer, {});
    surface->setCrop(base::RectF(2.5, 1.5, 10, 5));
    surface->setViewport(base::Size(80, 60));
    surface->setBackgroundBlur(base::Rect(0, 0, 80, 60), 10);
    surface->commit();
    surface->setPosition({20, 30});
    surface->updateTexture(*renderer);
    const auto background = [&] {
        renderer->clear({0, 0, 0, 1});
        renderer->fillRect({64, 0, 64, 128}, {1, 0, 0, 1});
    };
    background();
    scene.root()->render(*renderer);
    const auto direct = pixels();
    background();
    std::unique_ptr<render::Texture> snapshot;
    ASSERT_TRUE(renderer->renderToTexture(snapshot, base::Rect(0, 0, 128, 128), [&] {
        scene.root()->render(*renderer);
    }));
    renderer->drawTexture(snapshot.get(), base::Rect(0, 0, 128, 128), 1);
    const auto captured = pixels();
    for (base::Point point : {base::Point(96, 90), base::Point(89, 90), base::Point(104, 90), base::Point(96, 43)}) {
        for (int c = 0; c < 4; ++c)
            EXPECT_NEAR(channel(direct, point.x(), point.y(), c),
                        channel(captured, point.x(), point.y(), c), 3);
    }
    EXPECT_GT(channel(captured, 96, 90), 100);
    EXPECT_LT(channel(captured, 96, 90), 160);
    EXPECT_EQ(channel(captured, 96, 43), 255);
}

TEST_F(ViewporterPixelTest, RealDmaBufImportWhenRenderNodeIsAvailable)
{
    std::printf("GL renderer: %s\n", glGetString(GL_RENDERER));
    ScopedFd fd;
    std::vector<std::string> nodes;
    std::error_code error;
    for (const auto &entry : std::filesystem::directory_iterator("/dev/dri", error)) {
        if (entry.path().filename().string().starts_with("renderD"))
            nodes.push_back(entry.path().string());
    }
    std::sort(nodes.begin(), nodes.end());
    for (const auto &node : nodes) {
        fd.reset(open(node.c_str(), O_RDWR | O_CLOEXEC));
        if (fd.isValid())
            break;
    }
    if (!fd.isValid())
        GTEST_SKIP() << "No accessible DRM render node; real DMA-BUF import is not verified";
    const char *extensions = eglQueryString(renderer->eglDisplay(), EGL_EXTENSIONS);
    if (!extensions || !std::strstr(extensions, "EGL_EXT_image_dma_buf_import"))
        GTEST_SKIP() << "EGL display does not advertise DMA-BUF import";
    std::unique_ptr<gbm_device, decltype(&gbm_device_destroy)> device(gbm_create_device(fd.get()),
                                                                      gbm_device_destroy);
    ASSERT_TRUE(device);
    if (!gbm_device_is_format_supported(device.get(), GBM_FORMAT_XRGB8888,
                                        GBM_BO_USE_LINEAR | GBM_BO_USE_RENDERING))
        GTEST_SKIP() << "Render node does not support linear XRGB8888 allocation";
    std::unique_ptr<gbm_bo, decltype(&gbm_bo_destroy)> bo(
            gbm_bo_create(device.get(), 64, 32, GBM_FORMAT_XRGB8888,
                          GBM_BO_USE_LINEAR | GBM_BO_USE_RENDERING),
            gbm_bo_destroy);
    ASSERT_TRUE(bo);
    uint32_t stride = 0;
    void *mapData = nullptr;
    auto *mapped = static_cast<uint8_t *>(
            gbm_bo_map(bo.get(), 0, 0, 64, 32, GBM_BO_TRANSFER_WRITE, &stride, &mapData));
    ASSERT_TRUE(mapped);
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            const uint32_t pixel = 0xff000049u | uint32_t(x * 3) << 16 | uint32_t(y * 7) << 8;
            std::memcpy(mapped + y * stride + x * 4, &pixel, 4);
        }
    }
    gbm_bo_unmap(bo.get(), mapData);
    render::DmaBufAttributes attributes;
    attributes.size = {64, 32};
    attributes.format = DRM_FORMAT_XRGB8888;
    attributes.modifier = DRM_FORMAT_MOD_INVALID;
    attributes.planes.push_back(
            {ScopedFd(gbm_bo_get_fd(bo.get())), 0, gbm_bo_get_stride(bo.get())});
    ASSERT_TRUE(attributes.planes.front().fd.isValid());
    std::shared_ptr<compositor::DmaBufBuffer> buffer =
            compositor::DmaBufBuffer::create(renderer.get(), std::move(attributes));
    ASSERT_TRUE(buffer);
    scene::Scene scene;
    compositor::Compositor compositor(scene.root());
    auto surface = compositor.createSurface();
    surface->attach(buffer, {});
    surface->setBufferScale(2);
    surface->setCrop(base::RectF(2, 1, 10, 5));
    surface->setViewport(base::Size(40, 20));
    for (double outputScale : {1.0, 1.25, 1.5, 1.75, 2.0, 1.0}) {
        renderer->setRasterScale(outputScale);
        for (int i = 0; i < 8; ++i) {
            surface->setBufferTransform(static_cast<compositor::Transform>(i));
            surface->commit();
            surface->updateTexture(*renderer);
            renderer->clear({0, 0, 0, 0});
            scene.root()->render(*renderer);
            const auto image = pixels();
            for (base::Point point : {base::Point(5, 5), base::Point(20, 10), base::Point(35, 15)}) {
                const double u = 4 + (point.x() + 0.5) / (2 * outputScale);
                const double v = 2 + (point.y() + 0.5) / (2 * outputScale);
                const base::PointF raw[] = {{u, v},      {64 - v, u},     {64 - u, 32 - v},
                                       {v, 32 - u}, {64 - u, v},     {v, u},
                                       {u, 32 - v}, {64 - v, 32 - u}};
                EXPECT_NEAR(channel(image, point.x(), point.y(), 0), (raw[i].x() - 0.5) * 3, 2);
                EXPECT_NEAR(channel(image, point.x(), point.y(), 1), (raw[i].y() - 0.5) * 7, 2);
            }
        }
    }
}

TEST_F(ViewporterPixelTest, FractionalOutputNativeDensityIntegratedCase)
{
    renderer->endFrame();
    renderer->releaseTarget(target.get());
    target.reset();
    const EGLint attributes[] = {EGL_WIDTH, 1920, EGL_HEIGHT, 1080, EGL_NONE};
    const EGLSurface eglSurface =
            eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attributes);
    ASSERT_NE(eglSurface, EGL_NO_SURFACE);
    target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), eglSurface, nullptr,
                                                       base::Size(1920, 1080));
    ASSERT_TRUE(renderer->beginFrame(target.get()));
    renderer->setRasterScale(1.5);
    std::vector<uint8_t> storage(1200 * 900 * 4);
    for (int y = 0; y < 900; ++y) {
        for (int x = 0; x < 1200; ++x) {
            const int offset = (y * 1200 + x) * 4;
            storage[offset] = x % 2 ? 255 : 0;
            storage[offset + 1] = y % 3 ? 0 : 255;
            storage[offset + 2] = 73;
            storage[offset + 3] = 255;
        }
    }
    const int fd = testsupport::makeMemfd(storage.size());
    ASSERT_GE(fd, 0);
    ASSERT_EQ(pwrite(fd, storage.data(), storage.size(), 0), ssize_t(storage.size()));
    auto pool = compositor::ShmPool::create(fd, storage.size());
    auto buffer = pool->createBuffer({1200, 900}, 0, 4800, render::PixelFormat::Rgba8888, nullptr);
    scene::Scene scene;
    scene.setSize({1280, 720});
    compositor::Compositor compositor(scene.root());
    auto surface = compositor.createSurface();
    surface->setPosition({100, 50});
    surface->setViewport(base::Size(800, 600));
    surface->attach(buffer, {});
    surface->commit();
    surface->updateTexture(*renderer);
    renderer->clear({0, 0, 0, 0});
    scene.root()->render(*renderer);
    std::vector<uint8_t> image(1920 * 1080 * 4);
    glReadPixels(0, 0, 1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    ASSERT_EQ(glGetError(), GLenum(GL_NO_ERROR));
    for (int y = 0; y < 1080; ++y) {
        for (int x = 0; x < 1920; ++x) {
            const int offset = ((1079 - y) * 1920 + x) * 4;
            if (x >= 150 && x < 1350 && y >= 75 && y < 975) {
                const int source = ((y - 75) * 1200 + x - 150) * 4;
                for (int channel = 0; channel < 4; ++channel)
                    ASSERT_NEAR(image[offset + channel], storage[source + channel], 1)
                            << x << "," << y << " channel " << channel;
            } else {
                ASSERT_EQ(image[offset + 3], 0) << x << "," << y;
            }
        }
    }
    EXPECT_EQ(surface->contentSize(), base::Size(800, 600));
}

TEST_F(ViewporterPixelTest, FractionalOutputEdgesAndPhysicalSnapshotAllocation)
{
    for (double scale : {1.0, 1.25, 1.5, 1.75, 2.0, 1.0}) {
        renderer->setRasterScale(scale);
        renderer->clear({0, 0, 0, 0});
        const int logicalExtent = int(std::ceil(128 / scale));
        renderer->fillRect({0, 0, logicalExtent, logicalExtent}, {1, 0, 0, 1});
        auto image = pixels();
        EXPECT_EQ(channel(image, 127, 127), 255);
        std::unique_ptr<render::Texture> snapshot;
        ASSERT_TRUE(renderer->renderToTexture(snapshot, {3, 5, 17, 19}, [&] {
            renderer->fillRect({3, 5, 17, 19}, {0, 1, 0, 1});
        }));
        // At a fractional scale the texture is sized for the widest coverage
        // the bounds can round out to, so a move does not reallocate it.
        const int spare = scale != std::floor(scale) ? 1 : 0;
        EXPECT_EQ(snapshot->size(),
                  base::Size(int(std::ceil(17 * scale)) + spare, int(std::ceil(19 * scale)) + spare));
        const base::RectF content = base::RectF(3, 5, 17, 19).scaled(scale);
        const base::Point origin = content.toEnclosingRect().topLeft();
        EXPECT_TRUE(base::fuzzyCompare(static_cast<render::GlTexture *>(snapshot.get())->contentPixels,
                                       content.translated(-origin.x(), -origin.y())));
        renderer->drawTexture(snapshot.get(), {3, 5, 17, 19}, 1);
        image = pixels();
        EXPECT_EQ(channel(image, int(10 * scale), int(10 * scale), 1), 255);
    }
}

TEST_F(ViewporterPixelTest, FractionalClientCursorAndDragIconPixels)
{
    renderer->setRasterScale(1.5);
    scene::Scene scene;
    auto *surfaces = scene.root()->addChild(std::make_unique<scene::Node>());
    auto *icons = scene.root()->addChild(std::make_unique<scene::Node>());
    compositor::Compositor compositor(surfaces);
    compositor.setOutputScale(*compositor::OutputScale::parse("1.5"));
    input::Seat seat(&compositor);
    auto *dragIcons = icons->addChild(std::make_unique<scene::Node>());
    auto *clientCursors = icons->addChild(std::make_unique<scene::Node>());
    seat.setDragIconContainer(dragIcons);
    seat.setCursorContainer(clientCursors);
    std::vector<uint8_t> storage(30 * 24 * 4, 0);
    for (int i = 0; i < 30 * 24; ++i) {
        storage[i * 4 + 1] = 255;
        storage[i * 4 + 3] = 255;
    }
    const int fd = testsupport::makeMemfd(storage.size());
    ASSERT_GE(fd, 0);
    ASSERT_EQ(pwrite(fd, storage.data(), storage.size(), 0), ssize_t(storage.size()));
    auto pool = compositor::ShmPool::create(fd, storage.size());
    auto buffer = pool->createBuffer({30, 24}, 0, 120, render::PixelFormat::Rgba8888, nullptr);
    auto origin = compositor.createSurface();
    origin->attach(buffer, {});
    origin->setViewport(base::Size(80, 80));
    origin->commit();
    auto cursor = compositor.createSurface();
    cursor->attach(buffer, {});
    cursor->setViewport(base::Size(20, 16));
    cursor->commit();
    cursor->updateTexture(*renderer);

    struct Delegate : input::PointerDelegate {
        void onPointerDestroying(input::Pointer *) override { }

        bool canAcceptPointerEventsForSurface(compositor::Surface *) const override { return true; }

        void onPointerEnter(compositor::Surface *, const base::PointF &, input::ButtonFlags) override
        {
        }

        void onPointerLeave(compositor::Surface *) override { }

        void onPointerMotion(input::Timestamp, const base::PointF &) override { }

        void onPointerButton(input::Timestamp, input::ButtonFlags, bool) override { }

        void onPointerScroll(input::Timestamp, const base::PointF &, bool) override { }

        void onFingerScrollStop(input::Timestamp) override { }

        void onPointerFrame() override { }
    } delegate;

    input::Pointer pointer(&delegate, &seat);
    seat.onPointerMotion({}, {20, 20});
    pointer.setCursor(cursor.get(), {2, 3});
    ASSERT_EQ(pointer.cursorSurface(), cursor.get());
    EXPECT_EQ(cursor->node()->parent(), clientCursors);
    EXPECT_EQ(cursor->position(), base::Point(18, 17));
    renderer->clear({0, 0, 0, 0});
    icons->render(*renderer);
    auto image = pixels();
    EXPECT_EQ(channel(image, 30, 30, 1), 255);
    EXPECT_EQ(channel(image, 26, 30, 3), 0);
    auto icon = compositor.createSurface();
    icon->attach(buffer, {-4, -6});
    icon->setViewport(base::Size(20, 16));
    icon->commit();
    icon->updateTexture(*renderer);
    seat.drag()->start(origin.get(), icon.get(), {}, 0, input::LeftButton);
    seat.drag()->motion({}, {50, 50});
    ASSERT_TRUE(seat.drag()->active());
    EXPECT_EQ(icon->node()->parent(), dragIcons);
    EXPECT_EQ(cursor->node()->parent(), nullptr);
    EXPECT_EQ(icon->position(), base::Point(46, 44));
    EXPECT_EQ(icon->preferredScale(), 180u);
    renderer->clear({0, 0, 0, 0});
    icons->render(*renderer);
    image = pixels();
    EXPECT_EQ(channel(image, 70, 70, 1), 255);
    EXPECT_EQ(channel(image, 68, 70, 3), 0);
    seat.drag()->cancel();
    EXPECT_EQ(icon->node()->parent(), nullptr);
    renderer->clear({0, 0, 0, 0});
    icons->render(*renderer);
    EXPECT_EQ(channel(pixels(), 70, 70, 3), 0);
    seat.shutdown();
}

} // namespace
