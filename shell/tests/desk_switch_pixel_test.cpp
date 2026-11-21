// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The desk switch animation through the real GL renderer: the ending desk is
// copied inside a frame, and the strip lands on it pixel for pixel.

#include <GLES3/gl3.h>
#include <gtest/gtest.h>

#include "egl_test_util.h"
#include "pixel_test_util.h"
#include "scene/desk_switch_node.h"
#include "scene/scene.h"
#include "base/geometry.h"

using namespace Lingmo;
using namespace Lingmo::Render;

namespace {

const base::Rect kViewport(0, 0, 32, 16);
constexpr base::Size kNative(64, 32);

class DeskSwitchPixels : public testing::Test
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
        left->setGeometry(base::Rect(0, 0, 16, 16));
        right->setGeometry(base::Rect(16, 0, 16, 16));
        node->setGeometry(kViewport);
    }

    void TearDown() override
    {
        node->stop();
        if (renderer && target)
            renderer->releaseTarget(target.get());
        target.reset();
        renderer.reset();
    }

    // Straight into the output's frame, as the compositor draws it.
    ui::Bitmap paint()
    {
        renderer->setOutputContext({"out", kViewport, kNative, 2, 0, 1});
        renderer->clear({1, 0, 1, 1});
        node->render(*renderer);
        return test::readFramebuffer(kNative).mirrored(false, true);
    }

    int64_t now = 0;
    scene::Scene scene;
    scene::Node *container =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::ColorNode *left = static_cast<scene::ColorNode *>(
            container->addChild(std::make_unique<scene::ColorNode>(Color {1, 0, 0, 1})));
    scene::ColorNode *right = static_cast<scene::ColorNode *>(
            container->addChild(std::make_unique<scene::ColorNode>(Color {0, 1, 0, 1})));
    scene::DeskSwitchNode *node = static_cast<scene::DeskSwitchNode *>(
            container->addChild(std::make_unique<scene::DeskSwitchNode>([this] {
                return now;
            })));
    std::unique_ptr<EglRenderer> renderer;
    std::unique_ptr<EglRenderTarget> target;
};

} // namespace

TEST_F(DeskSwitchPixels, TheStripLandsOnTheEndingDeskCopiedInsideAFrame)
{
    // The starting desk, taken the way the controller takes it.
    std::unique_ptr<Texture> starting;
    ASSERT_TRUE(renderer->renderToTexture(starting, kViewport, [&] {
        node->paintCoveredContent(*renderer);
    }));
    scene::DeskSwitchNode::Frames frames;
    frames["out"] = std::move(starting);
    node->start(scene::DeskSwitchNode::Type::Continuous, 2, 0, 1, std::move(frames),
                kViewport.width());

    // The ending desk is activated underneath.
    left->setColor({0, 0, 1, 1});
    right->setColor({1, 1, 1, 1});
    node->takeEndingScreenshot(1);

    ui::Bitmap image = paint();
    ASSERT_FALSE(image.isNull());
    EXPECT_TRUE(node->endingScreenshotTaken());
    // Still the starting desk, whatever is live below.
    EXPECT_EQ(image.pixel(8, 16), test::kRed);
    EXPECT_EQ(image.pixel(56, 16), test::kGreen);

    node->startAnimation();
    now += 200;
    image = paint();
    // Partway: the starting desk has left through the left edge, and the gap
    // between the desks is black.
    EXPECT_EQ(image.pixel(0, 16), test::kBlack);

    now += 1000;
    image = paint();
    EXPECT_EQ(image.pixel(1, 16), test::kBlue);
    EXPECT_EQ(image.pixel(30, 16), test::kBlue);
    EXPECT_EQ(image.pixel(34, 16), test::kWhite);
    EXPECT_EQ(image.pixel(62, 30), test::kWhite);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(DeskSwitchPixels, AnOpaqueFrameReplacesWhatLiesBelowWithoutBlending)
{
    // Half-transparent texels in a texture marked opaque: its alpha is never
    // read, so nothing of the magenta below shows through.
    std::unique_ptr<Texture> frame;
    ASSERT_TRUE(renderer->renderToTexture(frame, kViewport, [&] {
        renderer->clear({0, 0, 0, 0});
        renderer->fillRect(kViewport, {0, 0, 1, 0.5});
    }));
    frame->contentOpaque = true;
    renderer->setOutputContext({"out", kViewport, kNative, 2, 0, 1});
    renderer->clear({1, 0, 1, 1});
    renderer->drawTexture(frame.get(), kViewport, 1);
    const uint32_t pixel = test::readFramebuffer(kNative).pixel(20, 10);
    EXPECT_EQ(test::red(pixel), 0);
    EXPECT_NEAR(test::blue(pixel), 128, 1);
    EXPECT_EQ(test::alpha(pixel), 255);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
