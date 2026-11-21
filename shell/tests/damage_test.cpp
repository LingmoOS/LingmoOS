// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "render/damage.h"
#include "scene/damage_tracker.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "base/region.h"

using namespace Lingmo;

TEST(Damage, OldNewRemovedAndRestoredCoverage)
{
    scene::Scene scene;
    const base::Rect viewport(0, 0, 400, 300);
    scene.setSize(viewport.size());
    auto *node = scene.root()->addChild(std::make_unique<scene::Node>());
    node->setGeometry({10, 20, 30, 40});
    scene::DamageTracker tracker;
    EXPECT_EQ(tracker.update(*scene.root(), viewport), base::Region(viewport));
    EXPECT_TRUE(tracker.update(*scene.root(), viewport).isEmpty());
    node->setGeometry({20, 30, 50, 60});
    EXPECT_EQ(tracker.update(*scene.root(), viewport),
              base::Region(base::Rect(10, 20, 30, 40)) + base::Rect(20, 30, 50, 60));
    node->setVisible(false);
    EXPECT_EQ(tracker.update(*scene.root(), viewport), base::Region(base::Rect(20, 30, 50, 60)));
    node->setVisible(true);
    EXPECT_EQ(tracker.update(*scene.root(), viewport), base::Region(base::Rect(20, 30, 50, 60)));
    auto owned = scene.root()->removeChild(node);
    EXPECT_EQ(tracker.update(*scene.root(), viewport), base::Region(base::Rect(20, 30, 50, 60)));
}

TEST(Damage, OutputsRetainIndependentContentHistory)
{
    scene::Node root;
    auto *node = root.addChild(std::make_unique<scene::Node>());
    node->setGeometry({90, 0, 30, 40});
    scene::DamageTracker left, right;
    const base::Rect a(0, 0, 100, 100), b(100, 0, 100, 100);
    left.update(root, a);
    right.update(root, b);
    node->notifyDamage(base::Rect(5, 2, 20, 3));
    EXPECT_EQ(left.update(root, a), base::Region(base::Rect(95, 2, 5, 3)));
    EXPECT_TRUE(left.update(root, a).isEmpty());
    EXPECT_EQ(right.update(root, b), base::Region(base::Rect(100, 2, 15, 3)));
    for (int i = 0; i < 40; ++i)
        node->notifyDamage(base::Rect(0, i, 1, 1));
    EXPECT_EQ(right.update(root, b), base::Region(base::Rect(100, 0, 20, 40)));
}

TEST(Damage, ParentMovementAndStackingDamageDescendants)
{
    scene::Node root;
    auto *parent = root.addChild(std::make_unique<scene::Node>());
    auto *child = parent->addChild(std::make_unique<scene::Node>());
    child->setGeometry({10, 20, 30, 40});
    scene::DamageTracker tracker;
    const base::Rect viewport(0, 0, 400, 300);
    tracker.update(root, viewport);
    parent->setPosition({5, 6});
    EXPECT_EQ(tracker.update(root, viewport),
              base::Region(base::Rect(10, 20, 30, 40)) + base::Rect(15, 26, 30, 40));
    root.addChild(std::make_unique<scene::Node>());
    tracker.update(root, viewport);
    root.raise(parent);
    EXPECT_EQ(tracker.update(root, viewport), base::Region(base::Rect(15, 26, 30, 40)));
}

TEST(Damage, BlurUsesCompleteSourcePixels)
{
    scene::Node root;
    auto *node = static_cast<scene::SurfaceNode *>(
            root.addChild(std::make_unique<scene::SurfaceNode>()));
    node->setGeometry({20, 20, 50, 50});
    node->setBackgroundBlur({0, 0, 50, 50}, 12);
    scene::DamageTracker tracker;
    const base::Rect viewport(0, 0, 100, 100);
    tracker.update(root, viewport);
    node->notifyDamage(base::Rect(2, 2, 1, 1));
    // Its own content lies above its blur, which reads what it did.
    const base::Region damage = tracker.update(root, viewport);
    EXPECT_EQ(damage, base::Region(base::Rect(22, 22, 1, 1)));
    const int halo = render::kBackdropBlurSupport + 2;
    const base::Rect sampled = base::Rect(20, 20, 50, 50).adjusted(-halo, -halo, halo, halo);
    EXPECT_EQ(render::coverSamples(damage.boundingRect(), tracker.samples()), sampled);
}

TEST(Damage, ABlurReadsOnlyWhatIsDrawnBeforeIt)
{
    scene::Node root;
    auto *below = root.addChild(std::make_unique<scene::Node>());
    below->setGeometry({500, 400, 10, 10});
    const auto addBlur = [&](const base::Rect &geometry) {
        auto *node = static_cast<scene::SurfaceNode *>(
                root.addChild(std::make_unique<scene::SurfaceNode>()));
        node->setGeometry(geometry);
        node->setBackgroundBlur({base::Point(), geometry.size()}, 12);
        return node;
    };
    const auto *first = addBlur({0, 0, 100, 40});
    const auto *second = addBlur({150, 0, 100, 40});
    auto *above = root.addChild(std::make_unique<scene::Node>());
    above->setGeometry({500, 400, 10, 10});
    scene::DamageTracker tracker;
    const base::Rect viewport(0, 0, 800, 600);
    tracker.update(root, viewport);
    const int halo = render::kBackdropBlurSupport + 2;
    const base::Rect firstSampled = base::Rect(0, 0, 100, 40).adjusted(-halo, -halo, halo, halo);
    const base::Rect secondSampled = base::Rect(150, 0, 100, 40).adjusted(-halo, -halo, halo, halo);
    EXPECT_EQ(tracker.samples(), (std::vector<base::Rect> {firstSampled, secondSampled}));
    const auto kept = [](uint64_t) {
        return true;
    };

    above->setPosition({10, 50});
    EXPECT_EQ(tracker.update(root, viewport),
              base::Region(base::Rect(500, 400, 10, 10)) + base::Rect(10, 50, 10, 10));
    EXPECT_TRUE(tracker.samples(kept).empty());
    EXPECT_EQ(tracker.freshBackdrops(kept),
              (std::vector<uint64_t> {first->damageId(), second->damageId()}));
    // Without a kept result, a repaint reaching a blur still takes in all it reads.
    EXPECT_EQ(tracker.samples(), (std::vector<base::Rect> {firstSampled, secondSampled}));

    // The first blur changes, and the second, drawn over it, reads it.
    below->setPosition({10, 50});
    EXPECT_EQ(tracker.update(root, viewport),
              (base::Region(base::Rect(500, 400, 10, 10)) + base::Rect(10, 50, 10, 10)
               + base::Rect(-1, -1, 102, 42) + base::Rect(149, -1, 102, 42))
                      .intersected(viewport));
    EXPECT_EQ(tracker.samples(kept), (std::vector<base::Rect> {firstSampled, secondSampled}));
    EXPECT_TRUE(tracker.freshBackdrops(kept).empty());

    // A kept result is only drawn again where nothing under it changed.
    below->setPosition({600, 400});
    tracker.update(root, viewport);
    below->setPosition({300, 100});
    tracker.update(root, viewport);
    EXPECT_EQ(tracker.samples(kept), (std::vector<base::Rect> {secondSampled}));
    EXPECT_EQ(tracker.freshBackdrops(kept), (std::vector<uint64_t> {first->damageId()}));
}

TEST(Damage, ABlurRepaintsWhenWhatItReadsChangesOutOfSight)
{
    scene::Node root;
    auto *node = static_cast<scene::SurfaceNode *>(
            root.addChild(std::make_unique<scene::SurfaceNode>()));
    node->setGeometry({100, 100, 50, 50});
    node->setBackgroundBlur({0, 0, 50, 50}, 12);
    bool changed = false;
    const auto backdropChanged = [&](const base::Rect &) {
        return changed;
    };
    const auto kept = [](uint64_t) {
        return true;
    };
    scene::DamageTracker tracker;
    const base::Rect viewport(0, 0, 400, 400);
    tracker.update(root, viewport, 1, 0, backdropChanged);
    EXPECT_TRUE(tracker.update(root, viewport, 1, 0, backdropChanged).isEmpty());
    EXPECT_TRUE(tracker.samples(kept).empty());
    changed = true;
    const int halo = render::kBackdropBlurSupport + 2;
    EXPECT_EQ(tracker.update(root, viewport, 1, 0, backdropChanged),
              base::Region(base::Rect(99, 99, 52, 52)));
    EXPECT_EQ(tracker.samples(kept),
              (std::vector<base::Rect> {base::Rect(100, 100, 50, 50).adjusted(-halo, -halo, halo, halo)}));
}

TEST(Damage, ABlurDrawnAgainWholeReadsEverythingAgain)
{
    class Retained : public scene::Node
    {
    public:
        bool redraws = false;

        bool redrawsSubtree(const base::Rect &, double) const override { return redraws; }
    };

    scene::Node root;
    auto *retained = static_cast<Retained *>(root.addChild(std::make_unique<Retained>()));
    auto *node = static_cast<scene::SurfaceNode *>(
            retained->addChild(std::make_unique<scene::SurfaceNode>()));
    node->setGeometry({100, 100, 50, 50});
    node->setBackgroundBlur({0, 0, 50, 50}, 12);
    const auto kept = [](uint64_t) {
        return true;
    };
    scene::DamageTracker tracker, inner;
    const base::Rect viewport(0, 0, 400, 400);
    tracker.update(root, viewport);
    inner.update(*retained, viewport);
    EXPECT_TRUE(tracker.update(root, viewport).isEmpty());
    retained->redraws = true;
    const int halo = render::kBackdropBlurSupport + 2;
    EXPECT_EQ(tracker.update(root, viewport), base::Region(base::Rect(99, 99, 52, 52)));
    EXPECT_EQ(tracker.samples(kept),
              (std::vector<base::Rect> {base::Rect(100, 100, 50, 50).adjusted(-halo, -halo, halo, halo)}));
    // A tracker of the retained subtree itself is what draws it again.
    EXPECT_TRUE(inner.update(*retained, viewport).isEmpty());
}

TEST(Damage, AFullRepaintStaysOnTheOutputsItDrawsOn)
{
    class Filter : public scene::Node
    {
    public:
        bool requiresFullRepaint() const override { return true; }
    };

    scene::Node root;
    auto *node = root.addChild(std::make_unique<Filter>());
    node->setGeometry({10, 10, 50, 50});
    scene::DamageTracker left, right;
    const base::Rect a(0, 0, 100, 100), b(100, 0, 100, 100);
    left.update(root, a);
    right.update(root, b);
    node->notifyDamage(base::Rect(0, 0, 1, 1));
    EXPECT_EQ(left.update(root, a), base::Region(a));
    EXPECT_TRUE(right.update(root, b).isEmpty());
    node->setPosition({80, 10});
    EXPECT_EQ(left.update(root, a), base::Region(a));
    EXPECT_EQ(right.update(root, b), base::Region(b));
}

TEST(Damage, RepaintCoversEveryBlurItReaches)
{
    const render::OutputRenderContext context {"test", base::Rect(100, 0, 200, 100), base::Size(400, 200),
                                               2};
    const base::Size size(400, 200);
    const std::vector<base::Rect> samples {base::Rect(110, 10, 20, 20), base::Rect(125, 25, 20, 20),
                                      base::Rect(250, 60, 10, 10)};
    EXPECT_EQ(render::coverSamples(base::Rect(300, 0, 10, 10), samples, context, size),
              base::Rect(300, 0, 10, 10));
    EXPECT_TRUE(render::coverSamples(base::Rect(), samples, context, size).isEmpty());
    // The first, once taken in, reaches the second; the third stays out of it.
    EXPECT_EQ(render::coverSamples(base::Rect(0, 0, 30, 30), samples, context, size),
              base::Rect(0, 0, 90, 90));
    EXPECT_EQ(render::coverSamples(base::Rect(0, 0, 100, 100), samples, context, size),
              base::Rect(0, 0, 100, 100));
}

TEST(Damage, CheckingForDamageLeavesTheHistoryAlone)
{
    scene::Scene scene;
    scene.setSize({400, 300});
    auto *left = scene.root()->addChild(std::make_unique<scene::Node>());
    left->setGeometry({10, 20, 30, 40});
    auto *right = scene.root()->addChild(std::make_unique<scene::Node>());
    right->setGeometry({300, 20, 30, 40});
    testsupport::FakeRenderer renderer;
    const render::OutputRenderContext context {"left", base::Rect(0, 0, 200, 300), {200, 300}, 1};
    renderer.setOutputContext(context);
    EXPECT_TRUE(scene.hasDamage(context));
    scene.render(renderer);
    EXPECT_FALSE(scene.hasDamage(context));
    right->setPosition({310, 20});
    EXPECT_FALSE(scene.hasDamage(context));
    left->setPosition({20, 20});
    EXPECT_TRUE(scene.hasDamage(context));
    EXPECT_TRUE(scene.hasDamage(context));
    scene.render(renderer);
    EXPECT_FALSE(scene.hasDamage(context));
}

TEST(Damage, FractionalTranslatedAndRotatedPhysicalCoverage)
{
    // Each rect also takes the pixel a snapped texture can reach past its far edges.
    render::OutputRenderContext context {"test", base::Rect(-100, 50, 80, 60), base::Size(100, 75), 1.25};
    EXPECT_EQ(render::physicalDamage(base::Rect(-99, 51, 2, 2), context, {100, 75}), base::Rect(1, 1, 4, 4));
    context.transform = 1;
    EXPECT_EQ(render::physicalDamage(base::Rect(-99, 51, 2, 2), context, {75, 100}), base::Rect(1, 95, 4, 4));
    context.transform = 2;
    EXPECT_EQ(render::physicalDamage(base::Rect(-99, 51, 2, 2), context, {100, 75}),
              base::Rect(95, 70, 4, 4));
    context.transform = 3;
    EXPECT_EQ(render::physicalDamage(base::Rect(-99, 51, 2, 2), context, {75, 100}), base::Rect(70, 1, 4, 4));
    context.transform = 0;
    context.nativeContentRect = {10, 20, 100, 75};
    EXPECT_EQ(render::physicalDamage(base::Rect(-99, 51, 2, 2), context, {120, 120}),
              base::Rect(11, 21, 4, 4));
    EXPECT_TRUE(render::physicalDamage(base::Rect(500, 500, 1, 1), context, {120, 120}).isEmpty());
}

TEST(Damage, LogicalRepaintTakesInEverythingThatReachesTheRepaint)
{
    // Culling by the logical repaint must never skip a rect whose physical
    // coverage, overhang included, reaches the physical repaint.
    for (const double scale : {1.0, 1.25, 1.5, 1.75, 2.0}) {
        for (uint32_t transform = 0; transform < 4; ++transform) {
            for (const bool letterboxed : {false, true}) {
                render::OutputRenderContext context {"test", base::Rect(-40, 30, 24, 20), {}, scale,
                                                     transform};
                base::Size content(int(std::ceil(24 * scale)), int(std::ceil(20 * scale)));
                if (transform % 2)
                    content = content.transposed();
                base::Size target = content;
                if (letterboxed) {
                    context.nativeContentRect = base::Rect(base::Point(3, 5), content);
                    target += base::Size(7, 9);
                }
                for (const base::Rect &damage : {base::Rect(-37, 33, 1, 1), base::Rect(-30, 40, 5, 3)}) {
                    const base::Rect repaint = render::physicalDamage(damage, context, target);
                    const auto clip = render::logicalRepaint(repaint, context, target);
                    ASSERT_TRUE(clip);
                    for (int y = 30; y < 50; ++y) {
                        for (int x = -40; x < -16; ++x) {
                            const base::Rect rect(x, y, 1, 1);
                            if (render::physicalDamage(rect, context, target).intersects(repaint))
                                EXPECT_TRUE(clip->intersects(rect))
                                        << scale << " " << transform << " " << letterboxed
                                        << " " << x << "," << y;
                        }
                    }
                }
            }
        }
    }
    render::OutputRenderContext flipped {"test", base::Rect(0, 0, 10, 10), {}, 1, 4};
    EXPECT_FALSE(render::logicalRepaint(base::Rect(0, 0, 2, 2), flipped, {10, 10}));
}

TEST(Damage, BufferAgeRepairsMissingFramesAndResets)
{
    render::BufferDamageHistory history;
    const base::Rect full(0, 0, 100, 100), a(1, 1, 2, 2), b(20, 20, 3, 3), c(40, 40, 4, 4);
    EXPECT_EQ(history.repair(a, 0, full), full);
    EXPECT_EQ(history.repair(a, 2, full), full);
    history.presented(a);
    history.presented(b);
    EXPECT_EQ(history.repair(c, 1, full), c);
    EXPECT_EQ(history.repair(c, 2, full), b.united(c));
    EXPECT_EQ(history.repair(c, 3, full), a.united(b).united(c));
    EXPECT_EQ(history.repair(c, 4, full), full);
    history.reset();
    EXPECT_EQ(history.repair(c, 2, full), full);
}

TEST(Damage, SynchronizedSurfaceDamageWaitsForParentAndSurvivesUpload)
{
    scene::Scene scene;
    compositor::Compositor compositor(scene.root());
    auto parent = compositor.createSurface(), child = compositor.createSurface();
    auto sub = compositor::SubSurface::create(child.get(), parent.get());
    auto pool = testsupport::makePool(65536);
    auto buffer = pool->createBuffer({64, 64}, 0, 256, render::PixelFormat::Bgrx8888, nullptr);
    child->attach(buffer, {});
    child->setBufferScale(2);
    child->commit();
    parent->attach(buffer, {});
    parent->commit();
    testsupport::FakeRenderer renderer;
    compositor.updateTextures(renderer);
    scene::DamageTracker tracker;
    const base::Rect viewport(0, 0, 100, 100);
    tracker.update(*scene.root(), viewport);
    child->damageBuffer({4, 6, 8, 10});
    child->attach(buffer, {});
    child->commit();
    EXPECT_TRUE(tracker.update(*scene.root(), viewport).isEmpty());
    parent->commit();
    compositor.updateTextures(renderer);
    EXPECT_EQ(tracker.update(*scene.root(), viewport), base::Region(base::Rect(1, 2, 6, 7)));
    child->setBufferScale(1);
    child->attach(buffer, {});
    child->damageBuffer({40, 40, 2, 2});
    child->commit();
    child->setBufferScale(2);
    child->attach(buffer, {});
    child->commit();
    EXPECT_TRUE(tracker.update(*scene.root(), viewport).isEmpty());
    parent->commit();
    compositor.updateTextures(renderer);
    EXPECT_EQ(tracker.update(*scene.root(), viewport), base::Region(base::Rect(0, 0, 32, 32)));
}

namespace {

// A parent with one subsurface, both mapped and uploaded, and a tracker that
// has seen them once. Modelled on exo's SurfaceTest subsurface damage cases.
struct SubsurfaceDamageFixture {
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    std::unique_ptr<compositor::Surface> parent = compositor.createSurface();
    std::unique_ptr<compositor::Surface> child = compositor.createSurface();
    std::unique_ptr<compositor::SubSurface> sub =
            compositor::SubSurface::create(child.get(), parent.get());
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1024 * 1024);
    std::shared_ptr<compositor::ShmBuffer> parentBuffer =
            pool->createBuffer({256, 512}, 0, 1024, render::PixelFormat::Bgrx8888, nullptr);
    std::shared_ptr<compositor::ShmBuffer> childBuffer = pool->createBuffer(
            {64, 128}, 256 * 512 * 4, 256, render::PixelFormat::Bgrx8888, nullptr);
    testsupport::FakeRenderer renderer;
    scene::DamageTracker tracker;
    const base::Rect viewport {0, 0, 400, 600};

    explicit SubsurfaceDamageFixture(bool synchronized)
    {
        sub->setCommitBehavior(synchronized);
        child->attach(childBuffer, {});
        child->commit();
        parent->attach(parentBuffer, {});
        parent->commit();
        compositor.updateTextures(renderer);
    }

    base::Region update()
    {
        compositor.updateTextures(renderer);
        return tracker.update(*scene.root(), viewport);
    }
};

// Linear sampling reaches one texel past damage at scale 1, as exo's margin.
bool approximately(const base::Region &damage, const base::Rect &expected)
{
    return damage.boundingRect().contains(expected)
            && expected.adjusted(-1, -1, 1, 1).contains(damage.boundingRect());
}

} // namespace

TEST(Damage, SubsurfaceDamageAggregation)
{
    SubsurfaceDamageFixture f(true);
    EXPECT_EQ(f.update(), base::Region(f.viewport));

    f.child->damage({32, 32, 16, 16});
    f.child->commit();
    f.parent->commit();
    EXPECT_TRUE(approximately(f.update(), base::Rect(32, 32, 16, 16)));

    // A root commit without a child commit must not repeat the child's damage.
    f.parent->damage({0, 0, 16, 16});
    f.parent->commit();
    const base::Region damage = f.update();
    EXPECT_TRUE(approximately(damage, base::Rect(0, 0, 16, 16)));
    EXPECT_FALSE(damage.intersects(base::Rect(32, 32, 16, 16)));
}

TEST(Damage, SubsurfaceDamageSynchronizedCommitBehavior)
{
    SubsurfaceDamageFixture f(true);
    f.update();

    f.child->damage({32, 32, 16, 16});
    EXPECT_EQ(f.child->pendingDamage(), base::Region(base::Rect(32, 32, 16, 16)));
    f.child->commit();
    EXPECT_TRUE(f.child->pendingDamage().isEmpty());
    EXPECT_TRUE(f.update().isEmpty());

    // Pending damage stays behind; the parent applies only what was cached.
    f.child->damage({0, 0, 16, 16});
    f.parent->commit();
    const base::Region damage = f.update();
    EXPECT_TRUE(approximately(damage, base::Rect(32, 32, 16, 16)));
    EXPECT_FALSE(damage.intersects(base::Rect(0, 0, 16, 16)));
    EXPECT_EQ(f.child->pendingDamage(), base::Region(base::Rect(0, 0, 16, 16)));
}

TEST(Damage, SubsurfaceDamageDesynchronizedCommitBehavior)
{
    SubsurfaceDamageFixture f(false);
    f.update();

    f.child->damage({32, 32, 16, 16});
    f.child->commit();
    EXPECT_TRUE(f.child->pendingDamage().isEmpty());
    EXPECT_TRUE(approximately(f.update(), base::Rect(32, 32, 16, 16)));
}

TEST(Damage, SubsurfaceReorderDamagesOnlyWhenTheOrderChanges)
{
    SubsurfaceDamageFixture f(true);
    auto sibling = f.compositor.createSurface();
    auto siblingSub = compositor::SubSurface::create(sibling.get(), f.parent.get());
    siblingSub->setPosition({100, 0});
    sibling->attach(f.childBuffer, {});
    sibling->commit();
    f.parent->commit();
    f.update();

    // Already in this order: nothing to repaint.
    siblingSub->placeAbove(f.child.get());
    f.sub->placeBelow(sibling.get());
    f.parent->commit();
    EXPECT_TRUE(f.update().isEmpty());

    siblingSub->placeBelow(f.child.get());
    f.parent->commit();
    EXPECT_FALSE(f.update().isEmpty());
}

TEST(Damage, RemovingASubsurfaceDamagesOnlyWhereItWas)
{
    SubsurfaceDamageFixture f(true);
    f.sub->setPosition({20, 10});
    f.parent->commit();
    f.update();

    f.sub.reset();
    EXPECT_TRUE(f.parent->pendingDamage().isEmpty());
    EXPECT_EQ(f.update(), base::Region(base::Rect(20, 10, 64, 128)));
    EXPECT_TRUE(f.update().isEmpty());
}
