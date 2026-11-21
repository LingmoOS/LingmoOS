// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include <GLES3/gl3.h>
#include <gtest/gtest.h>

#include "egl_test_util.h"
#include "base/geometry.h"
#include "ui/bitmap.h"

using namespace Lingmo;

namespace {

constexpr uint32_t kRed = 0xffff0000;
constexpr uint32_t kGreen = 0xff00ff00;
constexpr uint32_t kBlue = 0xff0000ff;
constexpr uint32_t kBlack = 0xff000000;

// The pixel at (x, y) of `image` turned clockwise by `quarters` right angles.
uint32_t turnedPixel(const ui::Bitmap &image, int quarters, int x, int y)
{
    const int width = image.width();
    const int height = image.height();
    switch (quarters % 4) {
    case 1:
        return image.pixel(y, height - 1 - x);
    case 2:
        return image.pixel(width - 1 - x, height - 1 - y);
    case 3:
        return image.pixel(width - 1 - y, x);
    default:
        return image.pixel(x, y);
    }
}

} // namespace

TEST(CapturePixels, IndependentTargetAndRepeatedFrames)
{
    auto renderer = render::EglRendererTestAccess::create();
    ASSERT_NE(renderer, nullptr);
    const EGLint attrs[] = {EGL_WIDTH, 64, EGL_HEIGHT, 48, EGL_NONE};
    auto surface = eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attrs);
    ASSERT_NE(surface, EGL_NO_SURFACE);
    auto target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), surface,
                                                            nullptr, base::Size(64, 48));
    render::TargetRetirement retire {renderer.get(), target.get()};
    renderer->setOutputContext({"test", base::Rect(100, 50, 64, 48), {64, 48}, 1, 0, 1});
    ASSERT_TRUE(renderer->beginFrame(target.get()));
    renderer->clear({1, 0, 0, 1});
    render::PresentationSubmission submission;
    renderer->presentationSubmission = &submission;
    GLint framebuffer, viewport[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glEnable(GL_SCISSOR_TEST);
    glScissor(20, 20, 2, 2);
    for (int frame = 0; frame < 20; ++frame) {
        const auto image = renderer->captureFrame([&] {
            EXPECT_EQ(renderer->presentationSubmission, nullptr);
            EXPECT_TRUE(renderer->capturing);
            renderer->clear({0, 0, 1, 1});
            renderer->fillRect({100 + frame, 50, 4, 4}, {0, 1, 0, 1});
        });
        ASSERT_EQ(image.size(), base::Size(64, 48));
        EXPECT_EQ(image.pixel(frame, 0), kGreen);
        EXPECT_EQ(image.pixel(frame, 47), kBlue);
        EXPECT_EQ(renderer->presentationSubmission, &submission);
        EXPECT_FALSE(renderer->capturing);
        EXPECT_TRUE(glIsEnabled(GL_SCISSOR_TEST));
        GLint scissor[4];
        glGetIntegerv(GL_SCISSOR_BOX, scissor);
        EXPECT_EQ(scissor[0], 20);
        EXPECT_EQ(scissor[1], 20);
        EXPECT_EQ(scissor[2], 2);
        EXPECT_EQ(scissor[3], 2);
        GLint current, currentViewport[4];
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current);
        glGetIntegerv(GL_VIEWPORT, currentViewport);
        EXPECT_EQ(current, framebuffer);
        for (int i = 0; i < 4; ++i)
            EXPECT_EQ(currentViewport[i], viewport[i]);
        uint8_t pixel[4];
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        EXPECT_EQ(pixel[0], 255);
        EXPECT_EQ(pixel[1], 0);
        EXPECT_EQ(pixel[2], 0);
    }
    renderer->presentationSubmission = nullptr;
    renderer->cancelFrame();
}

TEST(CapturePixels, NativeLetterboxAndTransform)
{
    auto renderer = render::EglRendererTestAccess::create();
    ASSERT_NE(renderer, nullptr);
    const EGLint attrs[] = {EGL_WIDTH, 64, EGL_HEIGHT, 48, EGL_NONE};
    auto surface = eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attrs);
    ASSERT_NE(surface, EGL_NO_SURFACE);
    auto target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), surface,
                                                            nullptr, base::Size(64, 48));
    render::TargetRetirement retire {renderer.get(), target.get()};
    for (uint32_t transform = 0; transform < 4; ++transform) {
        const base::Size logical = transform & 1 ? base::Size(32, 48) : base::Size(48, 32);
        renderer->setOutputContext(
                {"test", base::Rect(base::Point(), logical), {64, 48}, 1, transform, 1, base::Rect(8, 8, 48, 32)});
        ASSERT_TRUE(renderer->beginFrame(target.get()));
        const auto image = renderer->captureFrame([&] {
            renderer->clear({0, 0, 0, 1});
            renderer->fillRect(base::Rect(base::Point(), logical), {0, 1, 0, 1});
            renderer->fillRect(base::Rect(0, 0, 3, 3), {1, 0, 0, 1});
        });
        ASSERT_EQ(image.size(), base::Size(64, 48));
        EXPECT_EQ(image.pixel(0, 0), kBlack);
        EXPECT_EQ(turnedPixel(image, int(transform), 8, 8), kRed);
        EXPECT_EQ(turnedPixel(image, int(transform), 12, 12), kGreen);
        EXPECT_EQ(image.pixel(56, 40), kBlack);
    }
    renderer->cancelFrame();
}

TEST(CapturePixels, AReadbackCollectedLaterMatchesTheWaitingCapture)
{
    auto renderer = render::EglRendererTestAccess::create();
    ASSERT_NE(renderer, nullptr);
    const EGLint attrs[] = {EGL_WIDTH, 64, EGL_HEIGHT, 48, EGL_NONE};
    auto surface = eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attrs);
    ASSERT_NE(surface, EGL_NO_SURFACE);
    auto target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), surface,
                                                            nullptr, base::Size(64, 48));
    render::TargetRetirement retire {renderer.get(), target.get()};
    const render::OutputRenderContext output {"test", base::Rect(100, 50, 64, 48), {64, 48}, 1, 1, 1};
    renderer->setOutputContext(output);
    ASSERT_TRUE(renderer->beginFrame(target.get()));
    const auto paint = [&] {
        renderer->clear({0, 0, 1, 1});
        renderer->fillRect({110, 55, 20, 6}, {0, 1, 0, 1});
        renderer->fillRect({150, 80, 8, 12}, {1, 0, 0, 1});
    };
    GLint packBuffer = -1;
    const auto frame = renderer->captureFrameLater(paint);
    ASSERT_NE(frame, nullptr);
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &packBuffer);
    EXPECT_EQ(packBuffer, 0);
    const render::OutputRenderContext window {"test", base::Rect(105, 52, 30, 20), {45, 30}, 1.5, 0, 1};
    const auto image = renderer->captureImageLater(window, {45, 30}, paint);
    ASSERT_NE(image, nullptr);
    const ui::Bitmap expectedFrame = renderer->captureFrame(paint);
    const ui::Bitmap expectedImage = renderer->captureImage(window, {45, 30}, paint);
    renderer->cancelFrame();
    ASSERT_FALSE(expectedFrame.isNull());
    EXPECT_EQ(frame->take(), expectedFrame);
    EXPECT_EQ(image->take(), expectedImage);
    // Collected once, for every session that shares it.
    EXPECT_EQ(frame->take(), expectedFrame);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST(CapturePixels, ACopiedOutputFrameMatchesThePaintedCapture)
{
    auto renderer = render::EglRendererTestAccess::create();
    ASSERT_NE(renderer, nullptr);
    const EGLint attrs[] = {EGL_WIDTH, 64, EGL_HEIGHT, 48, EGL_NONE};
    auto surface = eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attrs);
    ASSERT_NE(surface, EGL_NO_SURFACE);
    auto target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), surface,
                                                            nullptr, base::Size(64, 48));
    render::TargetRetirement retire {renderer.get(), target.get()};
    for (uint32_t transform = 0; transform < 4; ++transform) {
        const base::Size logical = transform & 1 ? base::Size(32, 48) : base::Size(48, 32);
        renderer->setOutputContext(
                {"test", base::Rect(base::Point(), logical), {64, 48}, 1, transform, 1, base::Rect(8, 8, 48, 32)});
        ASSERT_TRUE(renderer->beginFrame(target.get()));
        const auto paint = [&] {
            renderer->clear({0, 0, 0, 1});
            renderer->fillRect(base::Rect(base::Point(), logical), {0, 1, 0, 1});
            renderer->fillRect(base::Rect(0, 0, 3, 5), {1, 0, 0, 1});
        };
        paint();
        EXPECT_FALSE(renderer->copyOutputFrame({64, 48}));
        // A partial repaint leaves its scissor on; the copy takes the whole frame.
        glEnable(GL_SCISSOR_TEST);
        glScissor(20, 20, 2, 2);
        const ui::Bitmap expected = renderer->captureFrame(paint);
        const ui::Bitmap copied = renderer->captureFrame([&] {
            EXPECT_FALSE(renderer->copyOutputFrame({32, 48}));
            EXPECT_TRUE(renderer->copyOutputFrame({64, 48}));
        });
        EXPECT_TRUE(glIsEnabled(GL_SCISSOR_TEST));
        renderer->cancelFrame();
        ASSERT_FALSE(expected.isNull());
        EXPECT_EQ(copied, expected) << "transform " << transform;
    }
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST(CapturePixels, AWindowAtAFractionalDeviceOriginKeepsItsLayerOnPixels)
{
    auto renderer = render::EglRendererTestAccess::create();
    ASSERT_NE(renderer, nullptr);
    const EGLint attrs[] = {EGL_WIDTH, 64, EGL_HEIGHT, 48, EGL_NONE};
    auto surface = eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attrs);
    ASSERT_NE(surface, EGL_NO_SURFACE);
    auto target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), surface,
                                                            nullptr, base::Size(64, 48));
    render::TargetRetirement retire {renderer.get(), target.get()};
    renderer->setOutputContext({"test", base::Rect(0, 0, 64, 48), {64, 48}, 1, 0, 1});
    ASSERT_TRUE(renderer->beginFrame(target.get()));

    // Columns one device pixel wide, alternately black and white.
    constexpr int kPixels = 56;
    std::vector<uint8_t> columns(kPixels * kPixels * 4);
    for (int y = 0; y < kPixels; ++y) {
        for (int x = 0; x < kPixels; ++x) {
            const uint8_t value = x % 2 ? 255 : 0;
            uint8_t *pixel = &columns[(y * kPixels + x) * 4];
            pixel[0] = pixel[1] = pixel[2] = value;
            pixel[3] = 255;
        }
    }
    auto texture = renderer->createTexture({kPixels, kPixels}, columns.data());
    ASSERT_NE(texture, nullptr);

    // 83 logical units are 145.25 device pixels at 1.75.
    const base::Rect body(83, 0, 32, 32);
    const render::OutputRenderContext window {"test", body, {kPixels, kPixels}, 1.75, 0, 1};
    std::unique_ptr<render::Texture> layer;
    const ui::Bitmap image = renderer->captureImage(window, {kPixels, kPixels}, [&] {
        ASSERT_TRUE(renderer->renderToTexture(layer, body, [&] {
            renderer->drawTextureMapped(texture.get(), body, 1, base::AffineTransform());
        }));
        renderer->drawTexture(layer.get(), body, 1);
    });
    renderer->cancelFrame();
    ASSERT_EQ(image.size(), base::Size(kPixels, kPixels));
    for (int x = 0; x < kPixels; ++x)
        EXPECT_EQ(image.pixel(x, kPixels / 2), x % 2 ? 0xffffffffu : kBlack) << "column " << x;
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
