// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/surface_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

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

class ViewporterTest : public testing::Test
{
protected:
    scene::Scene scene;
    Compositor compositor {scene.root()};
    std::shared_ptr<ShmPool> pool = testsupport::makePool(400 * 200 * 4);
    std::unique_ptr<Surface> surface = compositor.createSurface();
    testsupport::FakeRenderer renderer;

    std::shared_ptr<ShmBuffer> buffer(base::Size size)
    {
        return pool->createBuffer(size, 0, size.width() * 4,
                                  render::PixelFormat::Rgba8888,
                                  nullptr);
    }
};

TEST_F(ViewporterTest, SetViewport)
{
    auto b = buffer({1, 1});
    surface->attach(b, {});
    surface->setViewport(base::Size(256, 256));
    EXPECT_TRUE(surface->contentSize().isEmpty());
    surface->commit();
    EXPECT_EQ(surface->contentSize(), base::Size(256, 256));
    surface->setViewport(base::Size(512, 512));
    EXPECT_EQ(surface->contentSize(), base::Size(256, 256));
    surface->commit();
    EXPECT_EQ(surface->contentSize(), base::Size(512, 512));
    EXPECT_EQ(surface->node()->geometry().size(), base::Size(512, 512));
    EXPECT_TRUE(surface->damageRegion().intersects(base::Rect(0, 0, 512, 512)));
    surface->attach(nullptr, {});
    surface->commit();
    EXPECT_TRUE(surface->contentSize().isEmpty());
    EXPECT_FALSE(surface->isMapped());
    surface->attach(b, {});
    surface->commit();
    EXPECT_EQ(surface->contentSize(), base::Size(512, 512));
}

TEST_F(ViewporterTest, SetCrop)
{
    auto b = buffer({16, 16});
    surface->attach(b, {});
    surface->setCrop(base::RectF(2, 2, 12, 12));
    surface->commit();
    EXPECT_EQ(surface->contentSize(), base::Size(12, 12));
    EXPECT_EQ(surface->node()->geometry().size(), base::Size(12, 12));
    EXPECT_TRUE(surface->damageRegion().intersects(base::Rect(0, 0, 12, 12)));
    surface->attach(nullptr, {});
    surface->commit();
    EXPECT_TRUE(surface->contentSize().isEmpty());
}

TEST_F(ViewporterTest, SetCropAndBufferTransform)
{
    // Exo's asymmetric crop and exact normalized bounding rectangles.
    const base::RectF expected[] = {
        {0.03125, 0.1875, 0.40625, 0.0625}, {0.875, 0.0625, 0.03125, 0.8125},
        {0.5625, 0.75, 0.40625, 0.0625},    {0.09375, 0.125, 0.03125, 0.8125},
        {0.5625, 0.1875, 0.40625, 0.0625},  {0.09375, 0.0625, 0.03125, 0.8125},
        {0.03125, 0.75, 0.40625, 0.0625},   {0.875, 0.125, 0.03125, 0.8125}};
    auto b = buffer({128, 64});
    surface->attach(b, {});
    for (bool viewport : {false, true}) {
        for (int scale : {1, 2}) {
            surface->setCrop(base::RectF(4.0 / scale, 12.0 / scale, 52.0 / scale, 4.0 / scale));
            surface->setBufferScale(scale);
            surface->setViewport(viewport ? std::optional<base::Size>({128, 64}) : std::nullopt);
            for (int i = 0; i < 8; ++i) {
                SCOPED_TRACE(i);
                surface->setBufferTransform(static_cast<Transform>(i));
                surface->commit();
                EXPECT_EQ(surface->textureMapping().mapRect(base::RectF(0, 0, 1, 1)), expected[i]);
                EXPECT_EQ(surface->contentSize(),
                          viewport ? base::Size(128, 64) : base::Size(52 / scale, 4 / scale));
                EXPECT_EQ(surface->node()->textureMapping(), surface->textureMapping());
            }
        }
    }
}

TEST_F(ViewporterTest, HandVerifiableMappingDamageAndInput)
{
    auto b = buffer({400, 200});
    surface->attach(b, {});
    surface->setBufferScale(2);
    surface->setCrop(base::RectF(25, 10, 100, 50));
    surface->setViewport(base::Size(200, 100));
    surface->commit();
    EXPECT_EQ(surface->contentSize(), base::Size(200, 100));
    EXPECT_EQ(surface->surfaceToBuffer().mapRect(base::RectF(0, 0, 200, 100)),
              base::RectF(50, 20, 200, 100));
    EXPECT_EQ(surface->surfaceToBuffer().map(base::PointF(100, 50)), base::PointF(150, 70));
    EXPECT_TRUE(surface->hitTest(base::Point(100, 50)));
    EXPECT_FALSE(surface->hitTest(base::Point(200, 50)));
    surface->updateTexture(renderer);
    surface->damageBuffer(base::Rect(45, 15, 15, 15));
    surface->commit();
    EXPECT_EQ(surface->damageRegion(), base::Region(base::Rect(0, 0, 10, 10)));
    surface->setCrop(base::RectF(25.25, 10.5, 100, 50));
    surface->commit();
    EXPECT_EQ(surface->surfaceToBuffer().map(base::PointF()), base::PointF(50.5, 21));
    EXPECT_TRUE(surface->damageRegion().intersects(base::Rect(0, 0, 200, 100)));
    EXPECT_FALSE(surface->updateTexture(renderer));
    EXPECT_EQ(renderer.createdTextures, 1);
    surface->setCrop(std::nullopt);
    surface->setViewport(std::nullopt);
    surface->commit();
    EXPECT_EQ(surface->contentSize(), base::Size(200, 100));
    EXPECT_TRUE(surface->textureMapping().isIdentity());
    surface->commit();
    EXPECT_FALSE(surface->crop());
    EXPECT_FALSE(surface->viewport());
}

TEST_F(ViewporterTest, CachedStateAndChildCoordinates)
{
    auto b = buffer({128, 64});
    auto child = compositor.createSurface();
    auto role = SubSurface::create(child.get(), surface.get());
    role->setPosition({80, 30});
    surface->attach(b, {});
    surface->setViewport(base::Size(20, 10));
    child->attach(b, {});
    child->setCrop(base::RectF(4, 12, 52, 4));
    child->commit();
    child->setViewport(base::Size(104, 8));
    child->commit();
    EXPECT_TRUE(child->contentSize().isEmpty());
    surface->commit();
    EXPECT_EQ(child->contentSize(), base::Size(104, 8));
    EXPECT_EQ(child->position(), base::Point(80, 30));
    EXPECT_TRUE(surface->hitTestRegion().contains(base::Point(180, 34)));
    EXPECT_FALSE(surface->hitTest(base::Point(80, 30)));
    EXPECT_EQ(child->surfaceToBuffer().map(base::PointF()), base::PointF(4, 12));
    role->setCommitBehavior(false);
    child->setViewport(base::Size(52, 4));
    child->commit();
    EXPECT_EQ(child->contentSize(), base::Size(52, 4));
    EXPECT_EQ(surface->contentSize(), base::Size(20, 10));
}

TEST_F(ViewporterTest, ConservativeFractionalDamageForAllTransforms)
{
    auto b = buffer({128, 64});
    surface->attach(b, {});
    surface->setBufferScale(2);
    surface->setCrop(base::RectF(2.25, 6.25, 26.5, 2.5));
    surface->setViewport(base::Size(79, 11));
    for (int i = 0; i < 8; ++i) {
        surface->setBufferTransform(static_cast<Transform>(i));
        surface->commit();
        surface->attach(b, {});
        surface->commit();
        surface->updateTexture(renderer);
        surface->damageBuffer(base::Rect(0, 0, 128, 64));
        surface->commit();
        EXPECT_EQ(surface->damageRegion(), base::Region(base::Rect(0, 0, 79, 11)));
    }
}

TEST_F(ViewporterTest, ExtremeValidScalingDoesNotLoseBufferDamage)
{
    auto b = buffer({400, 200});
    surface->attach(b, {});
    surface->setCrop(base::RectF(25, 10, 1, 1));
    surface->setViewport(base::Size(10000000, 10000000));
    surface->commit();
    surface->updateTexture(renderer);
    surface->damageBuffer(base::Rect(25, 10, 1, 1));
    surface->commit();
    EXPECT_EQ(surface->damageRegion(), base::Region(base::Rect(0, 0, 10000000, 10000000)));
}

} // namespace
