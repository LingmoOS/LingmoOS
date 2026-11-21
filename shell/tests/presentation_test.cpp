// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/surface.cc (Commit, RequestPresentationCallback)
//           chromium/src/ui/ozone/platform/drm/gpu/page_flip_request.cc (Signal)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include <algorithm>

#include <gtest/gtest.h>

#include "backend/drm/presentation_tracker.h"
#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "shm_test_util.h"

using namespace Lingmo;

namespace {
class PresentationTest : public testing::Test
{
protected:
    std::vector<render::PresentationResult> results;
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    testsupport::FakeRenderer renderer;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(4096);
    std::shared_ptr<compositor::ShmBuffer> buffer =
        pool->createBuffer({16, 16}, 0, 64, render::PixelFormat::Bgra8888, nullptr);
    std::unique_ptr<compositor::Surface> surface = compositor.createSurface();
    backend::PresentationTracker tracker;
    std::shared_ptr<render::PresentationOutput> output =
        std::make_shared<render::PresentationOutput>();

    void SetUp() override
    {
        scene.setSize(base::Size(1920, 1080));
        surface->setPosition({0, 0});
        surface->attach(buffer, {});
        surface->commit();
        compositor.updateTextures(renderer);
    }

    void request(compositor::Surface* target = nullptr)
    {
        (target ? target : surface.get())->requestPresentationCallback([&](const auto& result) {
            results.push_back(result);
        });
    }

    render::PresentationSubmission draw()
    {
        render::PresentationSubmission submission;
        renderer.presentationSubmission = &submission;
        compositor.updateTextures(renderer);
        scene.render(renderer);
        renderer.presentationSubmission = nullptr;
        return submission;
    }

    void submit(uint64_t token) { tracker.submitted(token, draw()); }

    render::PresentationSubmission drawOn(const base::Rect& viewport)
    {
        render::OutputRenderContext context;
        context.outputId = std::to_string(viewport.x());
        context.logicalViewport = viewport;
        context.nativeTargetSize = viewport.size();
        renderer.setOutputContext(context);
        return draw();
    }

    void sideBySide(const base::Rect& left, const base::Rect& right)
    {
        scene.setSize(left.united(right).size());
        compositor.setOutputViews({{"left", left, {}, "A"}, {"right", right, {}, "B"}});
    }

    void complete(uint64_t token, uint32_t seq = 42)
    {
        ASSERT_TRUE(tracker.complete(token, 123, 456789, seq, true, 75, output));
    }
};

TEST_F(PresentationTest, PendingMultipleRequestsAndExactKernelResult)
{
    request();
    request();
    submit(1);
    complete(1);
    EXPECT_TRUE(results.empty());
    surface->commit();
    submit(2);
    EXPECT_TRUE(results.empty());
    complete(2);
    ASSERT_EQ(results.size(), 2u);
    for (const auto& r : results) {
        EXPECT_TRUE(r.valid());
        EXPECT_EQ(r.seconds, 123u);
        EXPECT_EQ(r.nanoseconds, 456789000u);
        EXPECT_EQ(r.sequence, 42u);
        EXPECT_EQ(r.refresh, 13333333u);
        EXPECT_EQ(r.flags, 7u);
        EXPECT_EQ(r.output, output);
        EXPECT_EQ(r.frameTimeMs(), 123456u);
    }
    submit(3);
    complete(3, 43);
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(PresentationTest, SubmittedASurvivesBAndCachedBIsNotReportedWithC)
{
    request();
    surface->commit();
    submit(1);
    request();
    surface->commit();
    request();
    surface->setViewport(base::Size(8, 8));
    surface->commit();
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].presented);
    complete(1);
    ASSERT_EQ(results.size(), 2u);
    EXPECT_TRUE(results[1].presented);
    submit(2);
    complete(2, 43);
    ASSERT_EQ(results.size(), 3u);
    EXPECT_EQ(results[2].sequence, 43u);
}

TEST_F(PresentationTest, SynchronizedReplacementAndParentApplication)
{
    auto child = compositor.createSurface();
    auto role = compositor::SubSurface::create(child.get(), surface.get());
    child->attach(buffer, {});
    request(child.get());
    child->commit();
    submit(1);
    complete(1);
    EXPECT_TRUE(results.empty());
    request(child.get());
    child->commit();
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].presented);
    surface->commit();
    submit(2);
    complete(2);
    ASSERT_EQ(results.size(), 2u);
    EXPECT_TRUE(results[1].presented);
}

TEST_F(PresentationTest, HiddenAndUnmappedDiscardWithoutAnUnrelatedFrame)
{
    surface->setHidden(true);
    request();
    surface->commit();
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].presented);
    surface->setHidden(false);
    request();
    surface->attach(nullptr, {});
    surface->commit();
    ASSERT_EQ(results.size(), 2u);
    EXPECT_FALSE(results[1].presented);
    submit(1);
    complete(1);
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(PresentationTest, FailedAttemptRetainsContentForCompositionRetry)
{
    request();
    surface->commit();
    {
        auto rejected = draw();
        EXPECT_TRUE(results.empty());
    }
    EXPECT_TRUE(results.empty());
    submit(2);
    complete(2);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].flags, 7u);
}

TEST_F(PresentationTest, SurfaceDestructionCancelsLateAndDuplicateCompletion)
{
    request();
    surface->commit();
    submit(1);
    surface.reset();
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].presented);
    complete(1);
    EXPECT_FALSE(tracker.complete(1, 124, 1, 43, true, 60, output));
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PresentationTest, OffOutputAndAbandonedAttempts)
{
    request();
    surface->commit();
    surface->setPosition({10000, 10000});
    auto submission = draw();
    surface->abandonUnsubmittedPresentation(submission);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].presented);
    tracker.submitted(1, std::move(submission));
    complete(1);
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PresentationTest, AnotherOutputsFrameLeavesContentToItsOwnOutput)
{
    const base::Rect left(0, 0, 1920, 1080), right(1920, 0, 1920, 1080);
    sideBySide(left, right);
    surface->setPosition({2000, 100});
    request();
    surface->commit();
    // The left output renders first and shows nothing of the surface.
    const auto first = drawOn(left);
    compositor.abandonUnsubmittedPresentation(left, first);
    EXPECT_TRUE(results.empty());
    auto second = drawOn(right);
    compositor.abandonUnsubmittedPresentation(right, second);
    tracker.submitted(1, std::move(second));
    complete(1);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_TRUE(results[0].valid());
}

TEST_F(PresentationTest, ContentOnNoOutputIsDiscardedByAnyOutputsFrame)
{
    const base::Rect left(0, 0, 1920, 1080), right(1920, 0, 1920, 1080);
    sideBySide(left, right);
    request();
    surface->commit();
    surface->setPosition({10000, 10000});
    const auto submission = drawOn(left);
    compositor.abandonUnsubmittedPresentation(left, submission);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].presented);
}

TEST_F(PresentationTest, PerContentZeroCopyAndComposition)
{
    request();
    surface->commit();
    auto composed = surface->node()->presentationContent();
    auto promoted = std::make_shared<render::PresentationContent>();
    promoted->add([&](const auto& r) {
        results.push_back(r);
    });
    render::PresentationSubmission submission;
    submission.add(composed, false);
    submission.add(promoted, true);
    tracker.submitted(1, std::move(submission));
    complete(1);
    ASSERT_EQ(results.size(), 2u);
    EXPECT_EQ(results[0].flags, 7u);
    EXPECT_EQ(results[1].flags, 15u);
}

TEST(PresentationNumeric, RefreshTimestampAndSequence)
{
    EXPECT_EQ(render::refreshInterval(60), 16666667u);
    EXPECT_EQ(render::refreshInterval(75), 13333333u);
    EXPECT_EQ(render::refreshInterval(60000.0 / 1001), 16683333u);
    EXPECT_EQ(render::refreshInterval(0), 0u);
    render::PresentationResult r;
    r.presented = true;
    EXPECT_TRUE(r.valid());
    r.seconds = (uint64_t(5) << 32) | 123;
    r.nanoseconds = 999999999;
    EXPECT_TRUE(r.valid());
    EXPECT_EQ(r.frameTimeMs(), 123999u);
    r.nanoseconds = 1000000000;
    EXPECT_FALSE(r.valid());
    render::PresentationSequence sequence;
    EXPECT_EQ(sequence.extend(0xfffffffe), 0xfffffffeu);
    EXPECT_EQ(sequence.extend(1), 0x100000001ull);
    EXPECT_EQ(sequence.extend(1), 0x100000001ull);
    EXPECT_EQ(sequence.extend(0), 0x100000000ull);
    sequence.reset();
    EXPECT_EQ(sequence.extend(4), 0x100000004ull);
}

TEST_F(PresentationTest, DesynchronizedChildAppliesImmediately)
{
    auto child = compositor.createSurface();
    auto role = compositor::SubSurface::create(child.get(), surface.get());
    role->setCommitBehavior(false);
    child->attach(buffer, {});
    request(child.get());
    child->commit();
    submit(1);
    EXPECT_TRUE(results.empty());
    complete(1);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_TRUE(results[0].valid());
}

TEST_F(PresentationTest, OpaqueOcclusionDoesNotPresentCoveredContent)
{
    request();
    surface->commit();
    auto cover = std::make_unique<scene::ColorNode>(render::Color {0, 0, 0, 1});
    cover->setGeometry({0, 0, 16, 16});
    scene.root()->addChild(std::move(cover));
    auto submission = draw();
    surface->abandonUnsubmittedPresentation(submission);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].valid());
    tracker.submitted(1, std::move(submission));
    complete(1);
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PresentationTest, ContentLeftOutOfAPartialRepaintIsStillPresented)
{
    request();
    surface->commit();
    renderer.paintClip = base::Rect(100, 100, 10, 10);
    renderer.calls.clear();
    submit(1);
    renderer.paintClip.reset();
    EXPECT_TRUE(std::none_of(renderer.calls.begin(), renderer.calls.end(),
                             [](const auto& call) { return call.kind == "texture"; }));
    complete(1);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_TRUE(results[0].valid());
}

TEST_F(PresentationTest, ClockValidityAndExplicitTeardown)
{
    request();
    surface->commit();
    submit(1);
    EXPECT_TRUE(tracker.complete(1, 123, 1, 42, false, 60, output));
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].valid());
    request();
    surface->commit();
    submit(2);
    tracker.abandon();
    ASSERT_EQ(results.size(), 2u);
    EXPECT_FALSE(results[1].valid());
    EXPECT_FALSE(tracker.complete(2, 123, 1, 43, true, 60, output));
}

TEST_F(PresentationTest, DiscontinuityAndDelayedOldEpochCompletion)
{
    request();
    surface->commit();
    submit(1);
    complete(1, 500);
    request();
    surface->commit();
    submit(2);
    tracker.discontinuity();
    request();
    surface->commit();
    submit(3);
    complete(2, 501);
    complete(3, 2);
    ASSERT_EQ(results.size(), 3u);
    EXPECT_EQ(results[1].sequence, 501u);
    EXPECT_EQ(results[2].sequence, 502u);
    request();
    surface->commit();
    submit(4);
    complete(4, 3);
    EXPECT_EQ(results.back().sequence, 503u);
}

TEST_F(PresentationTest, SupersessionCallbackCanDestroyItsSurface)
{
    surface->requestPresentationCallback([&](const auto& result) {
        results.push_back(result);
        surface.reset();
    });
    surface->commit();
    surface->commit();
    EXPECT_FALSE(surface);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].valid());
}

TEST_F(PresentationTest, BufferReleaseDoesNotWaitForFeedback)
{
    surface->attach(buffer, {});
    request();
    surface->commit();
    EXPECT_FALSE(buffer->isReleased());
    submit(1);
    EXPECT_TRUE(buffer->isReleased());
    EXPECT_TRUE(results.empty());
    complete(1);
    ASSERT_EQ(results.size(), 1u);
}

TEST_F(PresentationTest, RetainedResumeSubmissionHasItsOwnMeasuredCompletion)
{
    request();
    surface->commit();
    auto retained = draw();
    tracker.submitted(1, retained);
    tracker.discontinuity();
    tracker.submitted(2, retained);
    EXPECT_TRUE(results.empty());
    complete(2);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_TRUE(results[0].valid());
    complete(1);
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PresentationTest, HidingParentDiscardsUnsubmittedChild)
{
    auto child = compositor.createSurface();
    auto role = compositor::SubSurface::create(child.get(), surface.get());
    child->attach(buffer, {});
    request(child.get());
    child->commit();
    surface->commit();
    surface->setHidden(true);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].valid());
}
} // namespace
