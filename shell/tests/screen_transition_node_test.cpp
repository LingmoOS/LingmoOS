// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "fake_renderer.h"
#include "scene/scene.h"
#include "scene/screen_transition_node.h"
#include "base/geometry.h"

using namespace Lingmo;
using Transition = scene::ScreenTransitionNode;

namespace {

class SnapshotTexture : public render::Texture
{
public:
    explicit SnapshotTexture(base::Size size)
        : m_size(size)
    {
    }

    base::Size size() const override { return m_size; }

private:
    base::Size m_size;
};

class SnapshotRenderer : public testsupport::FakeRenderer
{
public:
    int captures = 0;

    bool renderToTexture(std::unique_ptr<render::Texture> &texture, const base::Rect &bounds,
                         const std::function<void()> &paint) override
    {
        ++captures;
        paint();
        texture = std::make_unique<SnapshotTexture>(
                base::Size(bounds.width() * 2, bounds.height() * 2));
        return true;
    }
};

struct Fixture {
    int64_t now = 1000;
    scene::Scene scene;
    Transition *transition = nullptr;
    SnapshotRenderer renderer;
    render::Texture *outgoing = nullptr;
    int incomingPaints = 0;
    bool ready = false;

    Fixture()
    {
        scene.setSize({1920, 1080});
        transition = static_cast<Transition *>(
                scene.root()->addChild(std::make_unique<Transition>([this] {
                    return now;
                })));
        transition->setGeometry({0, 0, 1920, 1080});
    }

    void start(int degrees = 90, base::Size snapshot = {1920, 1080})
    {
        Transition::Outputs outputs;
        auto texture = std::make_unique<SnapshotTexture>(snapshot);
        outgoing = texture.get();
        outputs["Virtual-1"] = {std::move(texture), degrees};
        transition->start(
                std::move(outputs),
                [this](render::Renderer &) {
                    ++incomingPaints;
                },
                [this] {
                    return ready;
                },
                1000);
    }

    auto paint(const std::string &output = "Virtual-1",
          const base::Rect &viewport = {0, 0, 960, 540})
    {
        renderer.calls.clear();
        renderer.setOutputContext({output, viewport, base::Size(1920, 1080), 2, 0, 0, {}});
        transition->render(renderer);
        return renderer.calls;
    }
};

} // namespace

TEST(ScreenTransitionNode, RotationHoldsTheOutgoingFrameOverAMask)
{
    Fixture f;
    f.start(-180);
    EXPECT_TRUE(f.transition->paintsSubtree());

    f.now += 500;
    const auto calls = f.paint();
    ASSERT_EQ(calls.size(), 2);
    EXPECT_EQ(calls[0].kind, "fill");
    EXPECT_EQ(calls[0].rect, base::Rect(0, 0, 960, 540));
    EXPECT_EQ(calls[1].texture, f.outgoing);
    EXPECT_EQ(calls[1].target, base::RectF(0, 0, 960, 540));
    EXPECT_EQ(calls[1].mapping, Transition::aboutCenter({480, 270}, -180));
    EXPECT_FLOAT_EQ(calls[1].alpha, 1);
    EXPECT_FALSE(f.transition->isAnimating());
    EXPECT_EQ(f.incomingPaints, 0);
}

TEST(ScreenTransitionNode, RotationCopiesTheNewContentAndAnimatesBothLayers)
{
    Fixture f;
    f.start(90);
    f.paint();
    f.ready = true;

    auto calls = f.paint();
    EXPECT_TRUE(f.transition->isAnimating());
    EXPECT_EQ(f.renderer.captures, 1);
    EXPECT_EQ(f.incomingPaints, 1);
    ASSERT_EQ(calls.size(), 3);
    EXPECT_EQ(calls[0].kind, "fill");
    EXPECT_NE(calls[1].texture, f.outgoing);
    EXPECT_EQ(calls[2].texture, f.outgoing);
    // The outgoing landscape frame is turned upright over the portrait viewport.
    EXPECT_EQ(calls[1].mapping, Transition::aboutCenter({480, 270}, 20));
    EXPECT_EQ(calls[2].mapping, Transition::aboutCenter({480, 270}, 90));

    f.now += Transition::kRotationDurationMs / 2;
    calls = f.paint();
    EXPECT_EQ(f.renderer.captures, 1);
    ASSERT_EQ(calls.size(), 3);
    EXPECT_GT(calls[2].alpha, 0);
    EXPECT_LT(calls[2].alpha, 1);

    f.now += Transition::kRotationDurationMs;
    EXPECT_TRUE(f.paint().empty());
    EXPECT_FALSE(f.transition->isActive());
}

TEST(ScreenTransitionNode, UnturnedFrameHoldsThenFadesOverTheNewContent)
{
    // The startup cover: another DRM master's frame, not turned, held until the
    // session is ready and then faded out over it.
    Fixture f;
    f.start(0);
    auto calls = f.paint();
    ASSERT_EQ(calls.size(), 2);
    EXPECT_EQ(calls[1].texture, f.outgoing);
    EXPECT_EQ(calls[1].target, base::RectF(0, 0, 960, 540));
    EXPECT_EQ(calls[1].mapping, Transition::aboutCenter({480, 270}, 0));
    EXPECT_FLOAT_EQ(calls[1].alpha, 1);

    f.ready = true;
    calls = f.paint();
    ASSERT_EQ(calls.size(), 3);
    EXPECT_EQ(calls[1].mapping, Transition::aboutCenter({480, 270}, 0));
    EXPECT_EQ(calls[2].texture, f.outgoing);
    f.now += Transition::kRotationDurationMs / 2;
    calls = f.paint();
    ASSERT_EQ(calls.size(), 3);
    EXPECT_GT(calls[2].alpha, 0);
    EXPECT_LT(calls[2].alpha, 1);
    f.now += Transition::kRotationDurationMs;
    EXPECT_TRUE(f.paint().empty());
}

TEST(ScreenTransitionNode, RotationStartsAtTheDeadlineWithoutReadyContent)
{
    Fixture f;
    f.start(-90);
    f.paint();
    EXPECT_FALSE(f.transition->isAnimating());
    f.now += 1000;
    f.paint();
    EXPECT_TRUE(f.transition->isAnimating());
}

TEST(ScreenTransitionNode, CapturingNeverAdvancesTheTransition)
{
    Fixture f;
    f.start();
    f.ready = true;
    f.renderer.capturing = true;
    f.paint();
    EXPECT_FALSE(f.transition->isAnimating());
    EXPECT_EQ(f.renderer.captures, 0);
}

TEST(ScreenTransitionNode, StopAnimatingKeepsAHeldFrame)
{
    Fixture f;
    f.start();
    f.transition->stopAnimating();
    EXPECT_TRUE(f.transition->isActive());
    f.ready = true;
    f.paint();
    f.transition->stopAnimating();
    EXPECT_FALSE(f.transition->isActive());
}

TEST(ScreenTransitionNode, RotationSampleMatchesUpstreamLayers)
{
    auto sample = Transition::sampleRotation(90, 0);
    EXPECT_DOUBLE_EQ(sample.oldDegrees, 90);
    EXPECT_DOUBLE_EQ(sample.newDegrees, 20);
    EXPECT_FLOAT_EQ(sample.oldOpacity, 1);
    sample = Transition::sampleRotation(90, 1);
    EXPECT_DOUBLE_EQ(sample.oldDegrees, 70);
    EXPECT_DOUBLE_EQ(sample.newDegrees, 0);
    EXPECT_FLOAT_EQ(sample.oldOpacity, 0);
    sample = Transition::sampleRotation(-180, 1);
    EXPECT_DOUBLE_EQ(sample.oldDegrees, -160);
}

TEST(ScreenTransitionNode, CoverRectTurnsTheOutgoingFrame)
{
    const base::RectF cover = Transition::coverRect({1920, 1080}, 90, {0, 0, 1080, 1920});
    EXPECT_EQ(cover, base::RectF(-420, 420, 1920, 1080));
    EXPECT_EQ(Transition::coverRect({1920, 1080}, 0, {0, 0, 960, 540}), base::RectF(0, 0, 960, 540));
}

TEST(ScreenTransitionNode, FastOutLinearInMatchesChromium)
{
    EXPECT_DOUBLE_EQ(Transition::fastOutLinearIn(0), 0);
    EXPECT_DOUBLE_EQ(Transition::fastOutLinearIn(1), 1);
    // gfx::CubicBezier(0.4, 0, 1, 1) at x = 0.5.
    EXPECT_NEAR(Transition::fastOutLinearIn(.5), 0.3248, 1e-3);
    double last = 0;
    for (int i = 1; i <= 100; ++i) {
        const double value = Transition::fastOutLinearIn(i / 100.0);
        EXPECT_GE(value, last);
        last = value;
    }
}
