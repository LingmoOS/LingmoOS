// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The desktop's wallpaper node through the real GL renderer at 1.5: the
// picture pixel for pixel, the gradient under the status bar along its stops,
// darkened in place by the overview, and drawn small in a desk preview.

#include <GLES3/gl3.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

#include "egl_test_util.h"
#include "pixel_test_util.h"
#include "scene/overview_node.h"
#include "scene/scene.h"
#include "scene/wallpaper_node.h"
#include "base/geometry.h"

using namespace Lingmo;
using namespace Lingmo::Render;

namespace {

const base::Rect kViewport(0, 0, 32, 16);
constexpr base::Size kNative(48, 24);
constexpr int kGradientHeight = 8;

// The status bar gradient's stops.
double strength(double position)
{
    static constexpr double kStops[] = {1.0, 0.896, 0.648, 0.352, 0.104, 0.0};
    const double at = std::clamp(position, 0.0, 1.0) * 5;
    const int index = std::min(int(at), 4);
    return kStops[index] + (kStops[index + 1] - kStops[index]) * (at - index);
}

class WallpaperPixels : public testing::Test
{
protected:
    void SetUp() override
    {
        renderer = EglRendererTestAccess::create();
        ASSERT_TRUE(renderer);
        const EGLint attributes[] = {EGL_WIDTH, kNative.width(), EGL_HEIGHT, kNative.height(), EGL_NONE};
        const auto surface = eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attributes);
        ASSERT_NE(surface, EGL_NO_SURFACE);
        target = std::make_unique<EglRenderTarget>(renderer->eglDisplay(), surface, nullptr, kNative);
        renderer->setOutputContext({"out", kViewport, kNative, 1.5, 0, 1});
        ASSERT_TRUE(renderer->beginFrame(target.get()));
        scene.setSize(kViewport.size());
        wallpaper->setGeometry(kViewport);
        overview->setGeometry(kViewport);

        // Left half grey 0x80, right half 0x40, in device pixels.
        picture = ui::Bitmap(kNative, ui::Bitmap::Format::Argb32Premultiplied);
        for (int y = 0; y < kNative.height(); ++y) {
            for (int x = 0; x < kNative.width(); ++x)
                picture.setPixel(x, y, x < kNative.width() / 2 ? 0xff808080u : 0xff404040u);
        }
    }

    void TearDown() override
    {
        if (renderer) {
            container->releaseRenderResources();
            if (target)
                renderer->releaseTarget(target.get());
        }
        target.reset();
        renderer.reset();
    }

    ui::Bitmap paint()
    {
        renderer->setOutputContext({"out", kViewport, kNative, 1.5, 0, 1});
        renderer->frameTime = double(now);
        renderer->clear({1, 0, 1, 1});
        container->render(*renderer);
        return test::readFramebuffer(kNative).mirrored(false, true);
    }

    int64_t now = 1000;
    scene::Scene scene;
    scene::Node *container = scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::WallpaperNode *wallpaper =
            static_cast<scene::WallpaperNode *>(container->addChild(std::make_unique<scene::WallpaperNode>()));
    scene::OverviewNode *overview = static_cast<scene::OverviewNode *>(
            container->addChild(std::make_unique<scene::OverviewNode>([this] { return now; })));
    ui::Bitmap picture;
    std::unique_ptr<EglRenderer> renderer;
    std::unique_ptr<EglRenderTarget> target;
};

} // namespace

TEST_F(WallpaperPixels, ThePictureCoversTheOutputPixelForPixel)
{
    wallpaper->setPicture(picture);
    const ui::Bitmap image = paint();
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(0xff808080u, image.pixel(0, 0));
    EXPECT_EQ(0xff808080u, image.pixel(23, 23));
    EXPECT_EQ(0xff404040u, image.pixel(24, 0));
    EXPECT_EQ(0xff404040u, image.pixel(47, 23));
}

TEST_F(WallpaperPixels, TheColourShowsWithoutAPicture)
{
    wallpaper->setColor({0.2f, 0.4f, 0.6f, 1});
    const ui::Bitmap image = paint();
    EXPECT_NEAR(51, test::red(image.pixel(10, 10)), 1);
    EXPECT_NEAR(102, test::green(image.pixel(10, 10)), 1);
    EXPECT_NEAR(153, test::blue(image.pixel(10, 10)), 1);
}

TEST_F(WallpaperPixels, TheGradientFadesAlongWallpaperQmlsStops)
{
    wallpaper->setPicture(picture);
    wallpaper->setGradient(scene::WallpaperNode::Gradient {{0, 0, 0, 77}, kGradientHeight});
    const ui::Bitmap image = paint();
    const int rows = int(std::lround(kGradientHeight * 1.5));
    for (int row = 0; row < rows; ++row) {
        const double alpha = 77 / 255.0 * strength((row + 0.5) / rows);
        EXPECT_NEAR(128 * (1 - alpha), test::red(image.pixel(4, row)), 1.5) << "row " << row;
    }
    // Below it, the picture as it is.
    EXPECT_EQ(0xff808080u, image.pixel(4, rows));

    // A light one lifts the picture.
    wallpaper->setGradient(scene::WallpaperNode::Gradient {{255, 255, 255, 115}, kGradientHeight});
    const ui::Bitmap light = paint();
    const double alpha = 115 / 255.0 * strength(0.5 / rows);
    EXPECT_NEAR(64 * (1 - alpha) + 255 * alpha, test::red(light.pixel(40, 0)), 1.5);
}

TEST_F(WallpaperPixels, TheOverviewDarkensItInPlace)
{
    wallpaper->setPicture(picture);
    wallpaper->setGradient(scene::WallpaperNode::Gradient {{0, 0, 0, 77}, kGradientHeight});
    overview->setBackdrop({wallpaper});
    overview->jumpTo(1);
    const ui::Bitmap image = paint();
    const double keep = 1 - scene::OverviewNode::kDimOpacity;
    // Opaque, darkened, nothing of the clear colour through it.
    const uint32_t below = image.pixel(4, 20);
    EXPECT_EQ(255, test::alpha(below));
    EXPECT_NEAR(128 * keep, test::red(below), 1.5);
    EXPECT_EQ(test::red(below), test::blue(below));
    // The gradient darkens with it, as part of the opaque desktop.
    const double alpha = 77 / 255.0 * strength(0.5 / 12);
    EXPECT_NEAR(128 * (1 - alpha) * keep, test::red(image.pixel(4, 0)), 1.5);
}

TEST_F(WallpaperPixels, ADeskPreviewDrawsItSmall)
{
    wallpaper->setPicture(picture);
    renderer->setOutputContext({"out", kViewport, kNative, 1.5, 0, 1});
    renderer->clear({1, 0, 1, 1});
    // The output's left half shows the whole picture.
    wallpaper->renderMapped(*renderer, base::RectF(0, 0, 16, 8));
    const ui::Bitmap image = test::readFramebuffer(kNative).mirrored(false, true);
    EXPECT_EQ(0xff808080u, image.pixel(2, 2));
    EXPECT_EQ(0xff404040u, image.pixel(21, 2));
    EXPECT_EQ(0xffff00ffu, image.pixel(30, 2));
    EXPECT_EQ(0xffff00ffu, image.pixel(2, 16));
}
