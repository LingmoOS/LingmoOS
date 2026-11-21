// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/viz/service/display/overlay_strategy_fullscreen.cc
//           chromium/src/components/viz/service/display/overlay_strategy_single_on_top.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Scene-level scanout eligibility: what the compositor is willing to hand to a
// hardware plane before any hardware is consulted.
//
// Adapted for Lingmo: exercises the corresponding behavior with native
// compositor test doubles and APIs.

#include <gtest/gtest.h>

#include <memory>

#include "render/scanout.h"
#include "scene/node.h"
#include "scene/overlay_candidate.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "base/geometry.h"

namespace Lingmo {

namespace {

constexpr base::Size kOutputSize {1920, 1080};

class TestScanoutBuffer final : public render::ScanoutBuffer
{
public:
    explicit TestScanoutBuffer(const base::Size &size, bool yInvert = false)
    {
        m_attributes.size = size;
        m_attributes.yInvert = yInvert;
        m_attributes.format = 0x34325258; // DRM_FORMAT_XRGB8888
        m_attributes.modifier = 0;
        m_attributes.planes.push_back(render::DmaBufPlane {ScopedFd(), 0, 4096});
    }

    const render::DmaBufAttributes &scanoutAttributes() const override { return m_attributes; }

private:
    render::DmaBufAttributes m_attributes;
};

// A texture stands in for real content; the planner only checks that one is
// there, never what is in it.
class TestTexture final : public render::Texture
{
public:
    explicit TestTexture(const base::Size &size)
        : m_size(size)
    {
    }

    base::Size size() const override { return m_size; }

private:
    base::Size m_size;
};

// Builds a surface node with everything scanout needs, ready to be spoiled one
// property at a time.
scene::SurfaceNode *addSurface(scene::Node *parent, const base::Rect &geometry, bool opaque = true)
{
    auto *node = static_cast<scene::SurfaceNode *>(
            parent->addChild(std::make_unique<scene::SurfaceNode>("surface")));
    node->setGeometry(geometry);
    node->setTexture(std::make_shared<TestTexture>(geometry.size()));
    node->setScanoutBuffer(std::make_shared<TestScanoutBuffer>(geometry.size()));
    node->setOpaque(opaque);
    return node;
}

class OverlayCandidateTest : public ::testing::Test
{
protected:
    void SetUp() override { m_scene.setSize(kOutputSize); }

    scene::OverlayPlan propose(size_t maxOverlays = 2)
    {
        return scene::proposeOverlays(*m_scene.root(), kOutputSize, maxOverlays);
    }

    scene::Scene m_scene;
};

} // namespace

TEST_F(OverlayCandidateTest, AFullscreenOpaqueSurfaceTakesThePrimaryPlane)
{
    scene::SurfaceNode *surface = addSurface(m_scene.root(), base::Rect(base::Point(0, 0), kOutputSize));

    const scene::OverlayPlan plan = propose();

    ASSERT_TRUE(plan.hasDirectScanout());
    EXPECT_EQ(plan.primary.node, surface);
    EXPECT_EQ(plan.primary.zOrder, 0);
    EXPECT_TRUE(plan.primary.opaque);
    EXPECT_EQ(plan.primary.displayRect, base::Rect(base::Point(0, 0), kOutputSize));
    EXPECT_EQ(plan.primary.sourceRect, base::Rect(base::Point(0, 0), kOutputSize));
    // With the whole output on one plane there is nothing left to overlay.
    EXPECT_TRUE(plan.overlays.empty());
}

TEST_F(OverlayCandidateTest, ATranslucentFullscreenSurfaceIsNotScannedOut)
{
    addSurface(m_scene.root(), base::Rect(base::Point(0, 0), kOutputSize), /*opaque=*/false);

    EXPECT_TRUE(propose().isEmpty());
}

TEST_F(OverlayCandidateTest, AFadedSurfaceIsNotScannedOut)
{
    scene::SurfaceNode *surface = addSurface(m_scene.root(), base::Rect(base::Point(0, 0), kOutputSize));
    surface->setAlpha(0.5f);

    EXPECT_TRUE(propose().isEmpty());
}

TEST_F(OverlayCandidateTest, AScaledSurfaceIsNotScannedOut)
{
    auto *node = static_cast<scene::SurfaceNode *>(
            m_scene.root()->addChild(std::make_unique<scene::SurfaceNode>()));
    node->setGeometry(base::Rect(base::Point(0, 0), kOutputSize));
    node->setTexture(std::make_shared<TestTexture>(kOutputSize));
    // The buffer is half the size of the geometry, so a plane would have to
    // scale it. The planner never asks a plane to do that.
    node->setScanoutBuffer(std::make_shared<TestScanoutBuffer>(base::Size(960, 540)));
    node->setOpaque(true);

    EXPECT_FALSE(propose().hasDirectScanout());
    EXPECT_TRUE(propose().overlays.empty());
}

TEST_F(OverlayCandidateTest, ASurfaceClippedByTheOutputIsNotScannedOut)
{
    addSurface(m_scene.root(), base::Rect(base::Point(-10, 0), kOutputSize));

    const scene::OverlayPlan plan = propose();
    EXPECT_FALSE(plan.hasDirectScanout());
    EXPECT_TRUE(plan.overlays.empty());
}

TEST_F(OverlayCandidateTest, ASurfaceWithoutADmaBufIsNotScannedOut)
{
    scene::SurfaceNode *surface = addSurface(m_scene.root(), base::Rect(base::Point(0, 0), kOutputSize));
    // Shm content: composited from a compositor-owned texture, never scanned out.
    surface->setScanoutBuffer(nullptr);

    EXPECT_FALSE(propose().hasDirectScanout());
}

TEST_F(OverlayCandidateTest, AHiddenSurfaceIsNotProposed)
{
    scene::SurfaceNode *surface = addSurface(m_scene.root(), base::Rect(base::Point(0, 0), kOutputSize));
    surface->setVisible(false);

    EXPECT_TRUE(propose().isEmpty());
}

TEST_F(OverlayCandidateTest, ContentAboveTheFullscreenSurfaceBlocksDirectScanout)
{
    addSurface(m_scene.root(), base::Rect(base::Point(0, 0), kOutputSize));
    // A cursor, a panel, anything drawn afterwards: the fullscreen surface is
    // no longer the topmost thing on screen.
    auto *overlayColor = static_cast<scene::ColorNode *>(m_scene.root()->addChild(
            std::make_unique<scene::ColorNode>(render::Color {1.0f, 0.0f, 0.0f, 1.0f})));
    overlayColor->setGeometry(base::Rect(100, 100, 64, 64));

    EXPECT_FALSE(propose().hasDirectScanout());
}

TEST_F(OverlayCandidateTest, AnUnoccludedSurfaceGoesOnAnOverlayPlane)
{
    auto *backdrop = static_cast<scene::ColorNode *>(m_scene.root()->addChild(
            std::make_unique<scene::ColorNode>(render::Color {0.0f, 0.0f, 0.0f, 1.0f})));
    backdrop->setGeometry(base::Rect(base::Point(0, 0), kOutputSize));
    scene::SurfaceNode *video = addSurface(m_scene.root(), base::Rect(200, 200, 640, 480));

    const scene::OverlayPlan plan = propose();

    EXPECT_FALSE(plan.hasDirectScanout());
    ASSERT_EQ(plan.overlays.size(), 1u);
    EXPECT_EQ(plan.overlays[0].node, video);
    EXPECT_EQ(plan.overlays[0].zOrder, 1);
    EXPECT_EQ(plan.overlays[0].displayRect, base::Rect(200, 200, 640, 480));
}

TEST_F(OverlayCandidateTest, AnOccludedSurfaceStaysComposited)
{
    scene::SurfaceNode *lower = addSurface(m_scene.root(), base::Rect(200, 200, 640, 480));
    auto *above = static_cast<scene::ColorNode *>(m_scene.root()->addChild(
            std::make_unique<scene::ColorNode>(render::Color {1.0f, 1.0f, 1.0f, 0.5f})));
    above->setGeometry(base::Rect(400, 400, 100, 100));

    const scene::OverlayPlan plan = propose();

    for (const scene::OverlayCandidate &candidate : plan.overlays)
        EXPECT_NE(candidate.node, lower);
}

TEST_F(OverlayCandidateTest, OverlaysAreCappedAtTheAvailablePlaneCount)
{
    addSurface(m_scene.root(), base::Rect(0, 0, 100, 100));
    addSurface(m_scene.root(), base::Rect(200, 0, 100, 100));
    addSurface(m_scene.root(), base::Rect(400, 0, 100, 100));

    EXPECT_EQ(propose(/*maxOverlays=*/1).overlays.size(), 1u);
    EXPECT_EQ(propose(/*maxOverlays=*/3).overlays.size(), 3u);
    EXPECT_TRUE(propose(/*maxOverlays=*/0).overlays.empty());
}

TEST_F(OverlayCandidateTest, OverlaysAreProposedInAscendingZOrder)
{
    scene::SurfaceNode *lower = addSurface(m_scene.root(), base::Rect(0, 0, 100, 100));
    scene::SurfaceNode *upper = addSurface(m_scene.root(), base::Rect(200, 0, 100, 100));

    const scene::OverlayPlan plan = propose();

    ASSERT_EQ(plan.overlays.size(), 2u);
    EXPECT_EQ(plan.overlays[0].node, lower);
    EXPECT_EQ(plan.overlays[0].zOrder, 1);
    EXPECT_EQ(plan.overlays[1].node, upper);
    EXPECT_EQ(plan.overlays[1].zOrder, 2);
}

TEST_F(OverlayCandidateTest, AnAnimationOverTheSceneBlocksEveryPlane)
{
    auto *animation = static_cast<scene::WindowAnimation *>(
            m_scene.root()->addChild(std::make_unique<scene::WindowAnimation>()));
    animation->setGeometry(base::Rect(base::Point(0, 0), kOutputSize));
    // The surface sits inside the effect, which composites its whole subtree.
    addSurface(animation, base::Rect(base::Point(0, 0), kOutputSize));

    EXPECT_TRUE(propose().isEmpty());
}

TEST_F(OverlayCandidateTest, ScanoutIsGivenUpAndTakenBackAsTheSceneChanges)
{
    scene::SurfaceNode *surface = addSurface(m_scene.root(), base::Rect(base::Point(0, 0), kOutputSize));
    ASSERT_TRUE(propose().hasDirectScanout());

    // Something is drawn over it: back to composition, with no intermediate
    // state in which neither path owns the content.
    auto *notification = static_cast<scene::ColorNode *>(m_scene.root()->addChild(
            std::make_unique<scene::ColorNode>(render::Color {1.0f, 1.0f, 1.0f, 1.0f})));
    notification->setGeometry(base::Rect(0, 0, 300, 80));
    EXPECT_FALSE(propose().hasDirectScanout());

    // And away again: scanout resumes on the very next plan.
    m_scene.root()->removeChild(notification);
    const scene::OverlayPlan plan = propose();
    ASSERT_TRUE(plan.hasDirectScanout());
    EXPECT_EQ(plan.primary.node, surface);
}

TEST_F(OverlayCandidateTest, PromotedContentIsSkippedByComposition)
{
    scene::SurfaceNode *surface = addSurface(m_scene.root(), base::Rect(base::Point(0, 0), kOutputSize));

    EXPECT_FALSE(surface->isPromotedToOverlay());
    surface->setPromotedToOverlay(true);
    EXPECT_TRUE(surface->isPromotedToOverlay());
}

TEST_F(OverlayCandidateTest, BackgroundBlurKeepsUnderlyingSurfacesInComposition)
{
    addSurface(m_scene.root(), base::Rect(base::Point(), kOutputSize));
    auto *popup = addSurface(m_scene.root(), {100, 100, 200, 100}, false);
    popup->setBackgroundBlur({0, 0, 200, 100}, 12);
    auto plan = propose();
    EXPECT_FALSE(plan.hasDirectScanout());
    EXPECT_TRUE(plan.overlays.empty());
    popup->setVisible(false);
    EXPECT_TRUE(propose().hasDirectScanout());
}

} // namespace Lingmo

namespace Lingmo {
TEST_F(OverlayCandidateTest, CroppingAndRotationFallBackAndResetRestoresPromotion)
{
    auto *surface = addSurface(m_scene.root(), base::Rect(base::Point(), kOutputSize));
    EXPECT_TRUE(propose().primary.isValid());
    surface->setTextureMapping(base::AffineTransform(0.5, 0, 0, 0.5, 0.25, 0.25));
    EXPECT_FALSE(propose().primary.isValid());
    EXPECT_TRUE(propose().overlays.empty());
    surface->setTextureMapping(base::AffineTransform(0, -1, 1, 0, 0, 1));
    EXPECT_FALSE(propose().primary.isValid());
    surface->setTextureMapping({});
    EXPECT_TRUE(propose().primary.isValid());
}

TEST_F(OverlayCandidateTest, OutputScaleFallsBackAndResetRestoresPromotion)
{
    addSurface(m_scene.root(), base::Rect(base::Point(), kOutputSize));
    for (double scale : {1.0, 1.25, 1.5, 1.75, 2.0, 1.0}) {
        const auto plan = scene::proposeOverlays(*m_scene.root(), kOutputSize, 2, scale);
        EXPECT_EQ(plan.hasDirectScanout(), scale == 1);
        EXPECT_TRUE(plan.overlays.empty());
    }
}

TEST_F(OverlayCandidateTest, AFullscreenBufferOfTheModeSizeScansOutAtAFractionalScale)
{
    // A 2160x1440 mode at 150% is 1440x960 logical; the fullscreen client
    // draws a buffer of the mode's size.
    const base::Size pixels(2160, 1440);
    const base::Rect logical(base::Point(), base::Size(1440, 960));
    m_scene.setSize(logical.size());
    scene::SurfaceNode *surface = addSurface(m_scene.root(), logical);
    surface->setScanoutBuffer(std::make_shared<TestScanoutBuffer>(pixels));

    const scene::OverlayPlan plan = scene::proposeOverlays(*m_scene.root(), pixels, 2, 1.5);
    ASSERT_TRUE(plan.hasDirectScanout());
    EXPECT_EQ(plan.primary.displayRect, base::Rect(base::Point(), pixels));
    EXPECT_EQ(plan.primary.sourceRect, base::Rect(base::Point(), pixels));
    EXPECT_TRUE(plan.overlays.empty());

    // A buffer of the logical size would have to be scaled by the plane.
    surface->setScanoutBuffer(std::make_shared<TestScanoutBuffer>(logical.size()));
    EXPECT_FALSE(scene::proposeOverlays(*m_scene.root(), pixels, 2, 1.5).hasDirectScanout());

    // Not fullscreen: an unoccluded opaque surface is not an overlay either, as
    // its logical bounds are not device pixels.
    surface->setScanoutBuffer(std::make_shared<TestScanoutBuffer>(pixels));
    surface->setGeometry(base::Rect(base::Point(0, 0), base::Size(1440, 900)));
    const scene::OverlayPlan partial = scene::proposeOverlays(*m_scene.root(), pixels, 2, 1.5);
    EXPECT_FALSE(partial.hasDirectScanout());
    EXPECT_TRUE(partial.overlays.empty());
}

TEST_F(OverlayCandidateTest, WindowVisualEffectsBlockOnlyTheirSubtreeAndRestoreEligibility)
{
    auto animation = std::make_unique<scene::WindowAnimation>();
    auto *root = animation.get();
    root->setGeometry({base::Point(), kOutputSize});
    root->finish();
    auto visual = std::make_unique<scene::WindowVisualNode>();
    auto *body = visual.get();
    scene::WindowVisualState state;
    state.shape = {{base::Point(), kOutputSize}, 14};
    state.clipContent = state.drawOutline = true;
    body->setState(state);
    auto *surface = addSurface(body, {base::Point(), kOutputSize});
    root->addChild(std::move(visual));
    m_scene.root()->addChild(std::move(animation));
    EXPECT_FALSE(propose().hasDirectScanout());
    EXPECT_TRUE(propose().overlays.empty());
    auto *other = addSurface(m_scene.root(), {40, 40, 200, 100});
    ASSERT_EQ(propose().overlays.size(), 1u);
    EXPECT_EQ(propose().overlays.front().node, other);
    other->setVisible(false);
    state.clipContent = state.drawOutline = false;
    state.shape.radius = 0;
    body->setState(state);
    EXPECT_EQ(propose().primary.node, surface);
}

} // namespace Lingmo

namespace Lingmo {
TEST_F(OverlayCandidateTest, AnimationBelowFullscreenMustAdvanceThroughComposition)
{
    auto *animation = m_scene.root()->addChild(std::make_unique<scene::WindowAnimation>());
    animation->setGeometry({0, 0, 100, 100});
    addSurface(m_scene.root(), base::Rect(base::Point(), kOutputSize));
    EXPECT_TRUE(propose().isEmpty());
    static_cast<scene::WindowAnimation *>(animation)->finish();
    EXPECT_TRUE(propose().hasDirectScanout());
}

TEST_F(OverlayCandidateTest, ATranslucentWindowIsComposited)
{
    auto *animation = static_cast<scene::WindowAnimation *>(
            m_scene.root()->addChild(std::make_unique<scene::WindowAnimation>()));
    animation->setGeometry({base::Point(), kOutputSize});
    animation->finish();
    scene::SurfaceNode *surface = addSurface(animation, {base::Point(), kOutputSize});
    ASSERT_EQ(propose().primary.node, surface);
    animation->setOpacity(0);
    EXPECT_TRUE(propose().isEmpty());
    animation->setOpacity(1);
    EXPECT_EQ(propose().primary.node, surface);
}
} // namespace Lingmo

namespace Lingmo {
TEST_F(OverlayCandidateTest, AMinimizeOfAHiddenWindowStillNeedsComposition)
{
    addSurface(m_scene.root(), base::Rect(base::Point(), kOutputSize));
    auto *animation = static_cast<scene::WindowAnimation *>(
            m_scene.root()->addChild(std::make_unique<scene::WindowAnimation>()));
    const base::Rect bounds(100, 100, 400, 300);
    animation->setGeometry(bounds);
    animation->finish();
    animation->setVisible(false);
    ASSERT_TRUE(propose().hasDirectScanout());
    animation->start(scene::WindowAnimation::Type::Minimize, bounds, {900, 1040, 48, 40});
    EXPECT_TRUE(propose().isEmpty());
    animation->finish();
    EXPECT_TRUE(propose().hasDirectScanout());
}
} // namespace Lingmo

namespace Lingmo {
TEST_F(OverlayCandidateTest, InvertedDmaBufRequiresComposition)
{
    auto *surface = addSurface(m_scene.root(), base::Rect(base::Point(), kOutputSize));
    surface->setScanoutBuffer(std::make_shared<TestScanoutBuffer>(kOutputSize, true));
    EXPECT_TRUE(propose().isEmpty());
}

TEST_F(OverlayCandidateTest, OpacityAboveOneCannotBeReproducedByScanout)
{
    auto *surface = addSurface(m_scene.root(), base::Rect(base::Point(), kOutputSize));
    surface->setAlpha(1.5f);
    EXPECT_TRUE(propose().isEmpty());
}
} // namespace Lingmo
