// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The overview through the real GL renderer: a window is drawn from its own
// texture at its place, over the dimmed desktop, and at rest where it was.

#include <GLES3/gl3.h>
#include <gtest/gtest.h>

#include "egl_test_util.h"
#include "pixel_test_util.h"
#include "scene/overview_node.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "base/geometry.h"

using namespace Lingmo;
using namespace Lingmo::Render;

namespace {

const base::Rect kViewport(0, 0, 32, 16);
constexpr base::Size kNative(64, 32);
const base::Rect kWindow(0, 0, 16, 16);
const base::RectF kPlace(16, 4, 8, 8);

class OverviewPixels : public testing::Test
{
protected:
    void SetUp() override
    {
        renderer = EglRendererTestAccess::create();
        ASSERT_TRUE(renderer);
        const EGLint attributes[] = {EGL_WIDTH, kNative.width(), EGL_HEIGHT, kNative.height(),
                                     EGL_NONE};
        const auto surface =
                eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attributes);
        ASSERT_NE(surface, EGL_NO_SURFACE);
        target = std::make_unique<EglRenderTarget>(renderer->eglDisplay(), surface, nullptr,
                                                   kNative);
        renderer->setOutputContext({"out", kViewport, kNative, 2, 0, 1});
        ASSERT_TRUE(renderer->beginFrame(target.get()));

        scene.setSize(kViewport.size());
        desktop->setGeometry(kViewport);
        window->setGeometry(kWindow);
        window->addChild(std::make_unique<scene::ColorNode>(Color {1, 0, 0, 1}))
                ->setGeometry(base::Rect(base::Point(), kWindow.size()));
        window->finish();
        node->setGeometry(kViewport);
        node->setBackdrop({desktop});
        scene::OverviewNode::Item item;
        item.windows = {window};
        item.to = kPlace;
        node->setItems({item}, false);
    }

    void TearDown() override
    {
        if (renderer && target)
            renderer->releaseTarget(target.get());
        target.reset();
        renderer.reset();
    }

    ui::Bitmap paint()
    {
        renderer->setOutputContext({"out", kViewport, kNative, 2, 0, 1});
        renderer->frameTime = double(now);
        renderer->clear({1, 0, 1, 1});
        container->render(*renderer);
        return test::readFramebuffer(kNative).mirrored(false, true);
    }

    // Logical coordinates to the pixel at their centre.
    static base::Point pixel(double x, double y) { return base::Point(int(x * 2), int(y * 2)); }

    int64_t now = 1000;
    scene::Scene scene;
    scene::Node *container =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::ColorNode *desktop = static_cast<scene::ColorNode *>(
            container->addChild(std::make_unique<scene::ColorNode>(Color {0, 0, 1, 1})));
    scene::WindowAnimation *window = static_cast<scene::WindowAnimation *>(
            container->addChild(std::make_unique<scene::WindowAnimation>([this] {
                return double(now);
            })));
    scene::OverviewNode *node = static_cast<scene::OverviewNode *>(
            container->addChild(std::make_unique<scene::OverviewNode>([this] {
                return now;
            })));
    std::unique_ptr<EglRenderer> renderer;
    std::unique_ptr<EglRenderTarget> target;
};

} // namespace

TEST_F(OverviewPixels, AtRestTheDeskIsAsItIs)
{
    const ui::Bitmap image = paint();
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(test::kRed, test::pixelAt(image, pixel(8, 8)));
    EXPECT_EQ(test::kBlue, test::pixelAt(image, pixel(28, 8)));
}

TEST_F(OverviewPixels, TheWindowIsAtItsPlaceOverTheDimmedDesktop)
{
    node->jumpTo(1);
    const ui::Bitmap image = paint();
    ASSERT_FALSE(image.isNull());
    // Its place, edge to edge.
    EXPECT_EQ(test::kRed, test::pixelAt(image, pixel(20, 8)));
    EXPECT_EQ(test::kRed, image.pixel(32, 8));
    EXPECT_EQ(test::kRed, image.pixel(47, 23));
    // Where it was, and around its place, the desktop dimmed.
    const uint32_t dimmed = test::pixelAt(image, pixel(4, 4));
    EXPECT_EQ(0, test::red(dimmed));
    EXPECT_NEAR(255 * (1 - scene::OverviewNode::kDimOpacity), test::blue(dimmed), 2);
    EXPECT_EQ(dimmed, image.pixel(31, 8));
    EXPECT_EQ(dimmed, image.pixel(48, 8));
    EXPECT_EQ(dimmed, image.pixel(40, 7));
    EXPECT_EQ(dimmed, image.pixel(40, 24));
}

TEST_F(OverviewPixels, HalfwayTheWindowIsHalfwayThere)
{
    node->animateTo(1);
    // Halfway along the tween, whenever that is.
    while (node->progress(double(now)) < 0.5)
        ++now;
    const double progress = node->progress(double(now));
    const base::RectF rect = node->presentedRect(0, progress, double(now));
    const ui::Bitmap image = paint();
    EXPECT_EQ(test::kRed, test::pixelAt(image, pixel(rect.center().x(), rect.center().y())));
    EXPECT_NE(test::kRed, test::pixelAt(image, pixel(rect.right() + 1, rect.center().y())));
}

TEST_F(OverviewPixels, AnOpaqueRegionOverAnAlphaBufferReplacesTheLastFrame)
{
    // Translucent texels under the opaque region would blend over the frame before.
    auto *surface = static_cast<scene::SurfaceNode *>(
            container->addChild(std::make_unique<scene::SurfaceNode>()));
    container->reorderChildren({surface, desktop, window, node});
    surface->setGeometry(kViewport);
    surface->setOpaque(true);
    std::vector<uint8_t> pixels(kNative.width() * kNative.height() * 4, 0);
    for (size_t i = 0; i < pixels.size(); i += 4) {
        pixels[i + 1] = 128;
        pixels[i + 3] = 128;
    }
    surface->setTexture(renderer->createTexture(kNative, pixels.data(), kNative.width() * 4,
                                                PixelFormat::Rgba8888));
    node->setBackdrop({surface});
    node->jumpTo(1);
    const ui::Bitmap image = paint();
    ASSERT_FALSE(image.isNull());
    const uint32_t dimmed = test::pixelAt(image, pixel(4, 4));
    EXPECT_EQ(0, test::red(dimmed));
    EXPECT_EQ(0, test::blue(dimmed));
    EXPECT_EQ(255, test::alpha(dimmed));
}

TEST_F(OverviewPixels, ADesktopDrawnThroughItsSnapshotReplacesTheLastFrame)
{
    // As live: the desktop surface sits in a window drawn from a snapshot texture.
    auto *desk = static_cast<scene::WindowAnimation *>(
            container->addChild(std::make_unique<scene::WindowAnimation>([this] {
                return double(now);
            })));
    container->reorderChildren({desk, desktop, window, node});
    desk->setGeometry(kViewport);
    auto *surface = static_cast<scene::SurfaceNode *>(
            desk->addChild(std::make_unique<scene::SurfaceNode>()));
    surface->setGeometry(base::Rect(base::Point(), kViewport.size()));
    surface->setOpaque(true);
    const std::vector<uint8_t> pixels(kNative.width() * kNative.height() * 4, 255);
    surface->setTexture(renderer->createTexture(kNative, pixels.data(), kNative.width() * 4,
                                                PixelFormat::Rgbx8888));
    desk->finish();
    node->setBackdrop({desk});
    node->jumpTo(1);
    const ui::Bitmap image = paint();
    ASSERT_FALSE(image.isNull());
    const uint32_t dimmed = test::pixelAt(image, pixel(4, 4));
    EXPECT_EQ(test::green(dimmed), test::red(dimmed));
    EXPECT_EQ(test::green(dimmed), test::blue(dimmed));
    EXPECT_NEAR(255 * (1 - scene::OverviewNode::kDimOpacity), test::green(dimmed), 2);
    EXPECT_EQ(255, test::alpha(dimmed));
}

TEST_F(OverviewPixels, AnOpaqueDesktopDarkenedInPlaceStaysOpaque)
{
    // A blur over a pixel left below alpha 1 lets the sharp backdrop through.
    auto *surface = static_cast<scene::SurfaceNode *>(
            container->addChild(std::make_unique<scene::SurfaceNode>()));
    container->reorderChildren({surface, desktop, window, node});
    surface->setGeometry(kViewport);
    surface->setOpaque(true);
    const std::vector<uint8_t> pixels(kNative.width() * kNative.height() * 4, 255);
    surface->setTexture(renderer->createTexture(kNative, pixels.data(), kNative.width() * 4,
                                                PixelFormat::Rgbx8888));
    node->setBackdrop({surface});
    node->jumpTo(1);
    const ui::Bitmap image = paint();
    ASSERT_FALSE(image.isNull());
    const uint32_t dimmed = test::pixelAt(image, pixel(4, 4));
    EXPECT_EQ(255, test::alpha(dimmed));
    EXPECT_NEAR(255 * (1 - scene::OverviewNode::kDimOpacity), test::green(dimmed), 2);
}

TEST_F(OverviewPixels, TheDesktopIconsFadeOutOnTheWayIn)
{
    auto *icons = static_cast<scene::ColorNode *>(
            container->addChild(std::make_unique<scene::ColorNode>(Color {0, 1, 0, 1})));
    container->reorderChildren({desktop, icons, window, node});
    icons->setGeometry(base::Rect(0, 0, 8, 8));
    node->setFadingBackdrop({icons});

    node->animateTo(1);
    while (node->progress(double(now)) < 0.5)
        ++now;
    const double progress = node->progress(double(now));
    const uint32_t halfway = test::pixelAt(paint(), pixel(4, 4));
    const float dim = 1 - scene::OverviewNode::kDimOpacity * float(progress);
    EXPECT_NEAR(255 * dim * (1 - progress), test::green(halfway), 3);
    EXPECT_NEAR(255 * dim * (1 - dim * (1 - progress)), test::blue(halfway), 3);

    node->jumpTo(1);
    const ui::Bitmap image = paint();
    ASSERT_FALSE(image.isNull());
    const uint32_t shown = test::pixelAt(image, pixel(4, 4));
    EXPECT_EQ(0, test::green(shown));
    EXPECT_EQ(test::pixelAt(image, pixel(12, 4)), shown);
}
