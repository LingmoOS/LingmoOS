// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/sub_surface_unittest.cc (SetPosition
//           PlaceAbove, PlaceBelow, SetCommitBehavior), surface_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: exercises the corresponding behavior with native
// compositor test doubles and APIs.

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "shm_test_util.h"

using namespace Lingmo;
using namespace Lingmo::Compositor;

namespace {

class SubSurfaceTest : public testing::Test
{
protected:
    scene::Scene scene;
    Compositor compositor {scene.root()};
    testsupport::FakeRenderer renderer;
    std::shared_ptr<ShmPool> pool = testsupport::makePool(16384);
    std::shared_ptr<ShmBuffer> a =
        pool->createBuffer({16, 16}, 0, 64, render::PixelFormat::Bgrx8888, nullptr);
    std::shared_ptr<ShmBuffer> b =
        pool->createBuffer({32, 32}, 1024, 128, render::PixelFormat::Bgrx8888, nullptr);
    std::unique_ptr<Surface> parent = compositor.createSurface();
    std::unique_ptr<Surface> child = compositor.createSurface();
    std::unique_ptr<Surface> sibling = compositor.createSurface();
    std::unique_ptr<SubSurface> sub = SubSurface::create(child.get(), parent.get());
    std::unique_ptr<SubSurface> other = SubSurface::create(sibling.get(), parent.get());

    void map()
    {
        for (auto* s : {child.get(), sibling.get(), parent.get()}) {
            s->attach(a, {});
            s->commit();
        }
        compositor.updateTextures(renderer);
    }

    std::vector<render::Texture*> drawOrder()
    {
        renderer.calls.clear();
        scene.root()->render(renderer);
        std::vector<render::Texture*> result;
        for (const auto& call : renderer.calls)
            result.push_back(call.texture);
        return result;
    }
};

TEST_F(SubSurfaceTest, SynchronizedCommitRemainsCachedAndParentAppliesIt)
{
    map();
    child->attach(b, {});
    child->setBufferScale(2);
    child->damage({1, 2, 3, 4});
    child->commit();
    EXPECT_EQ(a, child->buffer());
    EXPECT_EQ(1, child->bufferScale());
    EXPECT_TRUE(child->damageRegion().isEmpty());
    EXPECT_FALSE(child->updateTexture(renderer));
    parent->commit();
    EXPECT_EQ(b, child->buffer());
    EXPECT_EQ(2, child->bufferScale());
    EXPECT_TRUE(child->damageRegion().intersects(base::Rect(a->size())));
    EXPECT_TRUE(child->damageRegion().intersects(base::Rect(child->contentSize())));
    EXPECT_TRUE(child->updateTexture(renderer));
    child->damage({1, 2, 3, 4});
    child->commit();
    EXPECT_TRUE(child->damageRegion().isEmpty());
    parent->commit();
    EXPECT_EQ(base::Region(base::Rect(1, 2, 3, 4)), child->damageRegion());
}

TEST_F(SubSurfaceTest, ParentDoesNotApplyUncommittedPendingState)
{
    child->attach(b, {});
    child->setBufferScale(2);
    parent->commit();
    EXPECT_FALSE(child->hasContents());
    EXPECT_EQ(1, child->bufferScale());
}

TEST_F(SubSurfaceTest, DesynchronizedCommitsIndependently)
{
    sub->setCommitBehavior(false);
    child->attach(a, {});
    child->commit();
    EXPECT_EQ(a, child->buffer());
}

TEST_F(SubSurfaceTest, DesynchronizedGrowthReachesTheParentHitRegion)
{
    map();
    sub->setCommitBehavior(false);
    sub->setPosition({8, 8});
    parent->commit();
    ASSERT_FALSE(parent->hitTestRegion().contains(base::Point(30, 30)));
    // The parent's region gates hit testing of its children. Exo recommits
    // the tree from the root after a desynchronized commit, so it grows too.
    child->attach(b, {});
    child->commit();
    EXPECT_TRUE(parent->hitTestRegion().contains(base::Point(30, 30)));
}

TEST_F(SubSurfaceTest, InheritedSynchronizationAndSyncToDesync)
{
    auto grandchild = compositor.createSurface();
    auto grand = SubSurface::create(grandchild.get(), child.get());
    grand->setCommitBehavior(false);
    EXPECT_TRUE(grandchild->isSynchronized());
    grandchild->attach(b, {});
    grandchild->commit();
    child->commit();
    EXPECT_FALSE(grandchild->hasContents());
    grand->setCommitBehavior(false);
    EXPECT_FALSE(grandchild->hasContents());
    // Removing the final synchronizing ancestor applies the existing caches.
    sub->setCommitBehavior(false);
    EXPECT_FALSE(grandchild->isSynchronized());
    EXPECT_EQ(b, grandchild->buffer());
    sub->setCommitBehavior(true);
    grandchild->attach(a, {});
    grandchild->commit();
    EXPECT_EQ(b, grandchild->buffer());
    parent->commit();
    EXPECT_EQ(a, grandchild->buffer());
}

TEST_F(SubSurfaceTest, DesyncFlushesCacheButNotPending)
{
    child->attach(a, {});
    child->commit();
    child->attach(b, {});
    sub->setCommitBehavior(false);
    EXPECT_EQ(a, child->buffer());
    child->commit();
    EXPECT_EQ(b, child->buffer());
}

TEST_F(SubSurfaceTest, SetPosition)
{
    EXPECT_EQ(base::Point(), child->position());
    sub->setPosition({10, 10});
    child->commit();
    EXPECT_EQ(base::Point(), child->position());
    parent->commit();
    EXPECT_EQ(base::Point(10, 10), child->position());
    sub->setCommitBehavior(false);
    sub->setPosition({20, 20});
    child->commit();
    EXPECT_EQ(base::Point(10, 10), child->position());
    parent->commit();
    EXPECT_EQ(base::Point(20, 20), child->position());
    sub.reset();
    sub = SubSurface::create(child.get(), parent.get());
    parent->commit();
    EXPECT_EQ(base::Point(), child->position());
}

TEST_F(SubSurfaceTest, CachedParentLayoutDoesNotReadLaterPendingPosition)
{
    auto root = compositor.createSurface();
    auto parentSub = SubSurface::create(parent.get(), root.get());
    sub->setPosition({10, 10});
    parent->commit();
    sub->setPosition({20, 20});
    root->commit();
    EXPECT_EQ(base::Point(10, 10), child->position());
    parent->commit();
    root->commit();
    EXPECT_EQ(base::Point(20, 20), child->position());
}

TEST_F(SubSurfaceTest, PlaceAboveParent)
{
    map();
    EXPECT_TRUE(other->placeAbove(parent.get()));
    EXPECT_EQ((std::vector<render::Texture*> {parent->node()->texture(), child->node()->texture(),
                                              sibling->node()->texture()}),
              drawOrder());
    parent->commit();
    EXPECT_EQ((std::vector<render::Texture*> {parent->node()->texture(), sibling->node()->texture(),
                                              child->node()->texture()}),
              drawOrder());
}

TEST_F(SubSurfaceTest, PlaceBelowParent)
{
    map();
    EXPECT_TRUE(sub->placeBelow(parent.get()));
    parent->commit();
    EXPECT_EQ((std::vector<render::Texture*> {child->node()->texture(), parent->node()->texture(),
                                              sibling->node()->texture()}),
              drawOrder());
    EXPECT_TRUE(sub->placeAbove(parent.get()));
    parent->commit();
    EXPECT_EQ((std::vector<render::Texture*> {parent->node()->texture(), child->node()->texture(),
                                              sibling->node()->texture()}),
              drawOrder());
}

TEST_F(SubSurfaceTest, PlaceAboveSibling)
{
    map();
    EXPECT_TRUE(sub->placeAbove(sibling.get()));
    parent->commit();
    EXPECT_EQ((std::vector<render::Texture*> {parent->node()->texture(), sibling->node()->texture(),
                                              child->node()->texture()}),
              drawOrder());
    EXPECT_TRUE(sub->placeAbove(sibling.get()));
    parent->commit();
    EXPECT_EQ(3u, drawOrder().size());
}

TEST_F(SubSurfaceTest, PlaceBelowSibling)
{
    map();
    EXPECT_TRUE(other->placeBelow(child.get()));
    parent->commit();
    EXPECT_EQ((std::vector<render::Texture*> {parent->node()->texture(), sibling->node()->texture(),
                                              child->node()->texture()}),
              drawOrder());
    sub->placeBelow(parent.get());
    other->placeBelow(child.get());
    parent->commit();
    EXPECT_EQ((std::vector<render::Texture*> {sibling->node()->texture(), child->node()->texture(),
                                              parent->node()->texture()}),
              drawOrder());
}

TEST_F(SubSurfaceTest, ParentDestruction)
{
    map();
    parent.reset();
    EXPECT_EQ(nullptr, sub->parent());
    EXPECT_FALSE(child->isMapped());
    EXPECT_FALSE(child->node()->isEffectivelyVisible());
    sub->setPosition({2, 3});
    sub->setCommitBehavior(false);
    child->commit();
    EXPECT_FALSE(child->node()->isEffectivelyVisible());
    sub.reset();
    child.reset();
}

TEST_F(SubSurfaceTest, ChildDestruction)
{
    map();
    child.reset();
    EXPECT_EQ(nullptr, sub->surface());
    sub->setCommitBehavior(false);
    sub->setPosition({2, 3});
    parent->commit();
    EXPECT_EQ(2u, drawOrder().size());
    parent.reset();
    sub.reset();
}

TEST_F(SubSurfaceTest, RoleDestructionImmediatelyUnmapsAndCanBeRecreated)
{
    map();
    sub.reset();
    EXPECT_FALSE(child->isMapped());
    EXPECT_EQ(2u, drawOrder().size());
    sub = SubSurface::create(child.get(), parent.get());
    ASSERT_TRUE(sub);
    parent->commit();
    EXPECT_EQ(3u, drawOrder().size());
}

TEST_F(SubSurfaceTest, InvalidRelationships)
{
    EXPECT_FALSE(SubSurface::create(parent.get(), parent.get()));
    EXPECT_FALSE(SubSurface::create(parent.get(), child.get()));
    EXPECT_FALSE(SubSurface::create(child.get(), sibling.get()));
    auto unrelated = compositor.createSurface();
    EXPECT_FALSE(sub->placeAbove(unrelated.get()));
    EXPECT_FALSE(sub->placeBelow(unrelated.get()));
    EXPECT_FALSE(sub->placeAbove(child.get()));
    EXPECT_FALSE(sub->placeBelow(child.get()));
    auto grandchild = compositor.createSurface();
    auto grand = SubSurface::create(grandchild.get(), child.get());
    EXPECT_FALSE(SubSurface::create(parent.get(), grandchild.get()));
}

TEST_F(SubSurfaceTest, CachedCallbacksArePresentationDrivenAndCancelledOnDestroy)
{
    map();
    std::vector<std::optional<uint32_t>> fired;
    child->requestFrameCallback([&](auto time) {
        fired.push_back(time);
    });
    child->commit();
    compositor.frameSubmitted();
    compositor.frameDrawn(1);
    EXPECT_TRUE(fired.empty());
    parent->commit();
    EXPECT_TRUE(fired.empty());
    compositor.frameSubmitted();
    compositor.frameDrawn(2);
    ASSERT_EQ(1u, fired.size());
    EXPECT_EQ(2u, fired[0]);
    child->requestFrameCallback([&](auto time) {
        fired.push_back(time);
    });
    child->commit();
    child.reset();
    ASSERT_EQ(2u, fired.size());
    EXPECT_FALSE(fired[1]);
}

TEST_F(SubSurfaceTest, CachedBufferReplacementAndDestructionReleaseStorage)
{
    int releases = 0;
    a->setReleaseCallback([&] {
        ++releases;
    });
    child->attach(a, {});
    child->commit();
    EXPECT_EQ(1u, a->attachCount());
    EXPECT_EQ(0, releases);
    child->attach(b, {});
    child->commit();
    EXPECT_EQ(0u, a->attachCount());
    EXPECT_EQ(1, releases);
    EXPECT_EQ(1u, b->attachCount());
    child.reset();
    EXPECT_EQ(0u, b->attachCount());
    a->setReleaseCallback({});
}

TEST_F(SubSurfaceTest, AttachOffsetSynchronizedSubsurface)
{
    child->attach(a, {1, 2});
    child->commit();
    child->attach(a, {1, 2});
    child->commit();
    EXPECT_EQ(base::Point(), child->offset());
    parent->commit();
    EXPECT_EQ(base::Point(2, 4), child->offset());
    child->attach(a, {1, 2});
    child->commit();
    parent->commit();
    EXPECT_EQ(base::Point(3, 6), child->offset());
}

TEST_F(SubSurfaceTest, OtherSurfaceUploadCannotReleaseCachedStorage)
{
    int releases = 0;
    a->setReleaseCallback([&] {
        ++releases;
    });
    child->attach(a, {});
    child->commit();
    parent->attach(a, {});
    // Use an unrelated root so the cached child is not applied yet.
    auto root = compositor.createSurface();
    root->attach(a, {});
    root->commit();
    root->updateTexture(renderer);
    EXPECT_EQ(0, releases);
    parent->commit();
    compositor.updateTextures(renderer);
    EXPECT_EQ(1, releases);
    a->setReleaseCallback({});
}

TEST_F(SubSurfaceTest, NullParentUnmapsDescendantsAndRemapRestoresThem)
{
    map();
    parent->attach(nullptr, {});
    parent->commit();
    EXPECT_TRUE(drawOrder().empty());
    EXPECT_FALSE(child->isMapped());
    parent->attach(a, {});
    parent->commit();
    compositor.updateTextures(renderer);
    EXPECT_TRUE(child->isMapped());
    EXPECT_EQ(3u, drawOrder().size());
}

TEST_F(SubSurfaceTest, CacheMergesCallbacksDamageAndLastBuffer)
{
    std::vector<std::optional<uint32_t>> fired;
    child->attach(a, {});
    child->damage({0, 0, 2, 2});
    child->requestFrameCallback([&](auto t) {
        fired.push_back(t);
    });
    child->commit();
    child->attach(b, {});
    child->damage({10, 10, 2, 2});
    child->requestFrameCallback([&](auto t) {
        fired.push_back(t);
    });
    child->commit();
    parent->attach(a, {});
    parent->commit();
    EXPECT_EQ(b, child->buffer());
    EXPECT_EQ(base::Region(base::Rect(0, 0, 2, 2)) + base::Region(base::Rect(10, 10, 2, 2)),
              child->damageRegion());
    EXPECT_TRUE(fired.empty());
    compositor.updateTextures(renderer);
    compositor.frameSubmitted();
    compositor.frameDrawn(42);
    EXPECT_EQ(2u, fired.size());
}

TEST_F(SubSurfaceTest, AttachOffsetDesynchronizedSubsurface)
{
    child->attach(a, {});
    sub->setCommitBehavior(false);
    EXPECT_EQ(base::Point(), child->offset());

    child->attach(a, {1, 2});
    // The parent's commit does not apply a desynchronized child's pending state.
    parent->commit();
    EXPECT_EQ(base::Point(), child->offset());

    // An uncommitted attachment is replaced, offset included.
    child->attach(a, {10, 20});
    child->commit();
    EXPECT_EQ(base::Point(10, 20), child->offset());
}

} // namespace

TEST_F(SubSurfaceTest, BackgroundBlurWaitsForSynchronizedParentCommit)
{
    child->setBackgroundBlur({0, 0, 16, 16}, 4);
    child->commit();
    EXPECT_TRUE(child->backgroundBlurRect().isEmpty());
    parent->commit();
    EXPECT_EQ(child->backgroundBlurRect(), base::Rect(0, 0, 16, 16));
    child->setBackgroundBlur({}, 0);
    child->commit();
    EXPECT_FALSE(child->backgroundBlurRect().isEmpty());
    parent->commit();
    EXPECT_TRUE(child->backgroundBlurRect().isEmpty());
}
