// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Adapted from chromium/src/components/exo/surface_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include <gtest/gtest.h>

#include "base/region.h"

#include "compositor/buffer.h"
#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "fake_renderer.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "shm_test_util.h"

using namespace Lingmo;
using namespace Lingmo::Compositor;
using Lingmo::testsupport::makePool;

namespace {

class SurfaceTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_scene.setSize(base::Size(1920, 1080));
        m_surface = m_compositor.createSurface();
    }

    std::shared_ptr<ShmBuffer>
    makeBuffer(const base::Size &size, render::PixelFormat format = render::PixelFormat::Bgra8888)
    {
        return m_pool->createBuffer(size, m_nextOffset(size), size.width() * 4,
                                    format, nullptr);
    }

    int32_t m_nextOffset(const base::Size &size)
    {
        const int32_t offset = m_offset;
        m_offset += size.width() * size.height() * 4;
        return offset;
    }

    // Outlives m_surface, whose destructor cancels callbacks that write here.
    std::vector<std::optional<uint32_t>> m_fired;

    scene::Scene m_scene;
    Compositor m_compositor {m_scene.root()};
    std::unique_ptr<Surface> m_surface;
    std::shared_ptr<ShmPool> m_pool = makePool(4 * 1024 * 1024);
    Lingmo::testsupport::FakeRenderer m_renderer;
    int32_t m_offset = 0;
};

TEST_F(SurfaceTest, AttachIsPendingUntilCommit)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(64, 32));
    ASSERT_TRUE(buffer);

    m_surface->attach(buffer, base::Point());
    EXPECT_TRUE(m_surface->hasPendingAttachedBuffer());
    EXPECT_EQ(nullptr, m_surface->buffer());
    EXPECT_FALSE(m_surface->hasContents());
    EXPECT_EQ(0u, buffer->attachCount());

    m_surface->commit();
    EXPECT_FALSE(m_surface->hasPendingAttachedBuffer());
    EXPECT_EQ(buffer, m_surface->buffer());
    EXPECT_EQ(base::Size(64, 32), m_surface->bufferSize());
    EXPECT_EQ(1u, buffer->attachCount());
}

TEST_F(SurfaceTest, AttachOffsetTakesEffectAtCommit)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(16, 16));
    m_surface->attach(buffer, base::Point(4, -8));
    EXPECT_EQ(base::Point(), m_surface->offset());

    m_surface->commit();
    EXPECT_EQ(base::Point(4, -8), m_surface->offset());
}

TEST_F(SurfaceTest, DamageIsPendingUntilCommit)
{
    m_surface->damage(base::Rect(0, 0, 10, 10));
    EXPECT_EQ(base::Region(base::Rect(0, 0, 10, 10)), m_surface->pendingDamage());
    EXPECT_TRUE(m_surface->damageRegion().isEmpty());

    m_surface->damage(base::Rect(20, 20, 10, 10));
    EXPECT_EQ(2u, m_surface->pendingDamage().rectCount());

    m_surface->commit();
    EXPECT_TRUE(m_surface->pendingDamage().isEmpty());
    EXPECT_EQ(base::Region(base::Rect(0, 0, 10, 10)) + base::Region(base::Rect(20, 20, 10, 10)),
              m_surface->damageRegion());
}

TEST_F(SurfaceTest, BufferDamageIsMappedThroughScale)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(64, 64));
    m_surface->attach(buffer, base::Point());
    m_surface->setBufferScale(2);
    m_surface->damageBuffer(base::Rect(8, 16, 16, 32));
    m_surface->commit();

    EXPECT_EQ(base::Size(32, 32), m_surface->contentSize());
    EXPECT_EQ(base::Region(base::Rect(4, 8, 8, 16)), m_surface->damageRegion());
}

TEST_F(SurfaceTest, AClientScaleActsAsAnImplicitViewport)
{
    // Xwayland at scale 1.5: a 300x150 buffer is 200x100 logical units, and
    // buffer damage maps through that.
    m_surface->attach(makeBuffer(base::Size(300, 150)), base::Point());
    m_surface->setClientScale(1.5);
    m_surface->damageBuffer(base::Rect(30, 15, 60, 30));
    m_surface->commit();
    EXPECT_EQ(base::Size(200, 100), m_surface->contentSize());
    EXPECT_EQ(base::Region(base::Rect(20, 10, 40, 20)), m_surface->damageRegion());

    // A crop stays in buffer units and is scaled like the whole buffer.
    m_surface->setCrop(base::RectF(0, 0, 150, 75));
    m_surface->commit();
    EXPECT_EQ(base::Size(100, 50), m_surface->contentSize());

    // A destination, already in logical units, is taken as it is.
    m_surface->setViewport(base::Size(120, 60));
    m_surface->commit();
    EXPECT_EQ(base::Size(120, 60), m_surface->contentSize());
}

TEST_F(SurfaceTest, BufferDamageIsMappedThroughTransform)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(40, 20));
    m_surface->attach(buffer, base::Point());
    m_surface->setBufferTransform(Transform::Rotate90);
    // A 90 degree rotation swaps the axes: a 40x20 buffer is a 20x40 surface.
    m_surface->damageBuffer(base::Rect(0, 0, 10, 5));
    m_surface->commit();

    EXPECT_EQ(base::Size(20, 40), m_surface->contentSize());
    EXPECT_EQ(base::Region(base::Rect(0, 30, 5, 10)), m_surface->damageRegion());
}

TEST_F(SurfaceTest, DamageIsClearedOnceTheContentsAreUploaded)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(16, 16));
    m_surface->attach(buffer, base::Point());
    m_surface->damage(base::Rect(0, 0, 16, 16));
    m_surface->commit();
    ASSERT_FALSE(m_surface->damageRegion().isEmpty());

    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_TRUE(m_surface->damageRegion().isEmpty());
}

TEST_F(SurfaceTest, DamageWithAnUnboundedRectDoesNotCollapse)
{
    m_surface->damage(base::Rect(0, 0, 0x7FFFFFFF, 0x7FFFFFFF));
    m_surface->commit();
    EXPECT_FALSE(m_surface->damageRegion().isEmpty());
}

TEST_F(SurfaceTest, BufferReplacement)
{
    std::shared_ptr<ShmBuffer> first = makeBuffer(base::Size(16, 16));
    std::shared_ptr<ShmBuffer> second = makeBuffer(base::Size(32, 8));

    m_surface->attach(first, base::Point());
    m_surface->commit();
    ASSERT_EQ(first, m_surface->buffer());

    m_surface->attach(second, base::Point());
    EXPECT_EQ(first, m_surface->buffer()) << "still pending";
    m_surface->commit();
    EXPECT_EQ(second, m_surface->buffer());
    EXPECT_EQ(base::Size(32, 8), m_surface->contentSize());
}

TEST_F(SurfaceTest, CommitWithoutAttachKeepsTheBuffer)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(16, 16));
    m_surface->attach(buffer, base::Point());
    m_surface->commit();

    // An unrelated commit must not drop the attached buffer.
    m_surface->setBufferScale(1);
    m_surface->commit();
    EXPECT_EQ(buffer, m_surface->buffer());
}

TEST_F(SurfaceTest, NullAttachClearsTheContents)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(16, 16));
    m_surface->attach(buffer, base::Point());
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    ASSERT_NE(nullptr, m_surface->node()->texture());

    m_surface->attach(nullptr, base::Point());
    EXPECT_FALSE(m_surface->hasPendingAttachedBuffer());
    EXPECT_TRUE(m_surface->hasContents()) << "still the old contents";

    m_surface->commit();
    EXPECT_EQ(nullptr, m_surface->buffer());
    EXPECT_FALSE(m_surface->hasContents());
    EXPECT_EQ(0u, buffer->attachCount());

    m_surface->updateTexture(m_renderer);
    EXPECT_EQ(nullptr, m_surface->node()->texture());
}

TEST_F(SurfaceTest, DestroyedResizeBufferUploadsNewContents)
{
    auto buffer = makeBuffer(base::Size(16, 16));
    m_surface->attach(buffer, {});
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));

    const int fd = testsupport::makeMemfd(32 * 24 * 4);
    ASSERT_GE(fd, 0);
    auto pool = ShmPool::create(fd, 32 * 24 * 4);
    ASSERT_NE(pool, nullptr);
    auto resized = pool->createBuffer({32, 24}, 0, 32 * 4, render::PixelFormat::Bgra8888, nullptr);
    const std::vector<uint8_t> pixels(32 * 24 * 4, 127);
    ASSERT_EQ(pwrite(fd, pixels.data(), pixels.size(), 0), ssize_t(pixels.size()));
    std::weak_ptr<Buffer> lifetime = resized;
    m_surface->attach(resized, {});
    m_surface->commit();
    resized.reset();
    ASSERT_FALSE(lifetime.expired());
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_TRUE(lifetime.expired());
    auto *texture = dynamic_cast<testsupport::FakeTexture *>(m_surface->node()->texture());
    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(texture->size(), base::Size(32, 24));
    EXPECT_EQ(texture->pixels().front(), 127);
    EXPECT_TRUE(m_surface->isMapped());

    m_surface->attach(nullptr, {});
    m_surface->commit();
    m_surface->updateTexture(m_renderer);
    EXPECT_FALSE(m_surface->isMapped());
    EXPECT_EQ(m_surface->node()->texture(), nullptr);
}

TEST_F(SurfaceTest, DamageDoesNotReadReleasedShmStorage)
{
    const int fd = testsupport::makeMemfd(16 * 16 * 4);
    ASSERT_GE(fd, 0);
    auto pool = ShmPool::create(fd, 16 * 16 * 4);
    ASSERT_NE(pool, nullptr);
    auto buffer = pool->createBuffer({16, 16}, 0, 16 * 4, render::PixelFormat::Bgra8888, nullptr);
    std::vector<uint8_t> pixels(16 * 16 * 4, 64);
    ASSERT_EQ(pwrite(fd, pixels.data(), pixels.size(), 0), ssize_t(pixels.size()));
    auto released = std::make_shared<bool>(false);
    buffer->setReleaseCallback([released] {
        *released = true;
    });
    m_surface->attach(buffer, {});
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    ASSERT_TRUE(*released);

    // Released storage may contain an unfinished frame that has not been attached.
    pixels.assign(pixels.size(), 192);
    ASSERT_EQ(pwrite(fd, pixels.data(), pixels.size(), 0), ssize_t(pixels.size()));
    for (bool bufferDamage : {false, true}) {
        if (bufferDamage)
            m_surface->damageBuffer({0, 0, 16, 16});
        else
            m_surface->damage({0, 0, 16, 16});
        m_surface->commit();
        EXPECT_FALSE(m_surface->updateTexture(m_renderer));
        auto *texture = dynamic_cast<testsupport::FakeTexture *>(m_surface->node()->texture());
        ASSERT_NE(texture, nullptr);
        EXPECT_EQ(texture->pixels().front(), 64);
    }

    m_surface->attach(buffer, {});
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    auto *texture = dynamic_cast<testsupport::FakeTexture *>(m_surface->node()->texture());
    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(texture->pixels().front(), 192);
    buffer->setReleaseCallback({});
}

TEST_F(SurfaceTest, RegionsAreCopiedAndPendingUntilCommit)
{
    base::Region opaque(base::Rect(0, 0, 10, 10));
    m_surface->setOpaqueRegion(opaque);
    m_surface->setInputRegion(base::Region(base::Rect(2, 2, 4, 4)));

    EXPECT_TRUE(m_surface->opaqueRegion().isEmpty());
    EXPECT_FALSE(m_surface->inputRegion().has_value());

    // Mutating the caller's region after the request must not be visible.
    opaque += base::Rect(100, 100, 10, 10);

    m_surface->commit();
    EXPECT_EQ(base::Region(base::Rect(0, 0, 10, 10)), m_surface->opaqueRegion());
    ASSERT_TRUE(m_surface->inputRegion().has_value());
    EXPECT_EQ(base::Region(base::Rect(2, 2, 4, 4)), *m_surface->inputRegion());
}

TEST_F(SurfaceTest, FullyOpaqueNeedsTheWholeContentCovered)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(64, 32));
    m_surface->attach(buffer, {});
    m_surface->commit();
    EXPECT_FALSE(m_surface->isFullyOpaque());

    // A partial rectangle overlaps the content without covering it.
    m_surface->setOpaqueRegion(base::Region(base::Rect(0, 0, 10, 10)));
    m_surface->commit();
    EXPECT_FALSE(m_surface->isFullyOpaque());

    // Several rectangles that tile the content do cover it.
    m_surface->setOpaqueRegion(base::Region(base::Rect(0, 0, 64, 16))
                               + base::Region(base::Rect(0, 16, 64, 16)));
    m_surface->commit();
    EXPECT_TRUE(m_surface->isFullyOpaque());

    // A hole anywhere breaks it again; so does leaving a corner out.
    m_surface->setOpaqueRegion(base::Region(base::Rect(0, 0, 64, 32))
                               - base::Region(base::Rect(30, 14, 4, 4)));
    m_surface->commit();
    EXPECT_FALSE(m_surface->isFullyOpaque());
    m_surface->setOpaqueRegion(base::Region(base::Rect(0, 0, 64, 32))
                               - base::Region(base::Rect(0, 0, 1, 1)));
    m_surface->commit();
    EXPECT_FALSE(m_surface->isFullyOpaque());

    // A region larger than the content is fine.
    m_surface->setOpaqueRegion(base::Region(base::Rect(-10, -10, 200, 200)));
    m_surface->commit();
    EXPECT_TRUE(m_surface->isFullyOpaque());
    buffer->setReleaseCallback({});
}

TEST_F(SurfaceTest, AFormatWithoutAlphaIsOpaqueRegardlessOfTheRegion)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(16, 16), render::PixelFormat::Bgrx8888);
    m_surface->attach(buffer, {});
    m_surface->commit();
    EXPECT_TRUE(m_surface->isFullyOpaque());
    buffer->setReleaseCallback({});
}

TEST_F(SurfaceTest, AnEmptyInputRegionIsNotTheSameAsNone)
{
    m_surface->setInputRegion(base::Region());
    m_surface->commit();
    ASSERT_TRUE(m_surface->inputRegion().has_value());
    EXPECT_TRUE(m_surface->inputRegion()->isEmpty());

    m_surface->resetInputRegion();
    m_surface->commit();
    EXPECT_FALSE(m_surface->inputRegion().has_value());
}

TEST_F(SurfaceTest, ScaleAndTransformArePendingUntilCommit)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(64, 32));
    m_surface->attach(buffer, base::Point());
    m_surface->commit();
    ASSERT_EQ(base::Size(64, 32), m_surface->contentSize());

    m_surface->setBufferScale(2);
    m_surface->setBufferTransform(Transform::Rotate270);
    EXPECT_EQ(1, m_surface->bufferScale());
    EXPECT_EQ(Transform::Normal, m_surface->bufferTransform());
    EXPECT_EQ(base::Size(64, 32), m_surface->contentSize());

    m_surface->commit();
    EXPECT_EQ(2, m_surface->bufferScale());
    EXPECT_EQ(Transform::Rotate270, m_surface->bufferTransform());
    EXPECT_EQ(base::Size(16, 32), m_surface->contentSize());
}

TEST_F(SurfaceTest, FrameCallbacksBelongToTheCommitThatFollowsThem)
{
    const auto record = [this](std::optional<uint32_t> time) {
        m_fired.push_back(time);
    };

    m_surface->requestFrameCallback(record);
    EXPECT_EQ(1u, m_surface->pendingFrameCallbackCount());
    EXPECT_EQ(0u, m_surface->committedFrameCallbackCount());

    m_surface->commit();
    EXPECT_EQ(0u, m_surface->pendingFrameCallbackCount());
    EXPECT_EQ(1u, m_surface->committedFrameCallbackCount());
    EXPECT_TRUE(m_fired.empty()) << "commit must not fire frame callbacks";

    // A callback requested after the commit waits for the next one.
    m_surface->requestFrameCallback(record);
    EXPECT_EQ(1u, m_surface->pendingFrameCallbackCount());
    EXPECT_EQ(1u, m_surface->committedFrameCallbackCount());
}

TEST_F(SurfaceTest, FrameCallbacksFireOnPresentation)
{
    m_surface->requestFrameCallback([this](std::optional<uint32_t> time) {
        m_fired.push_back(time);
    });
    m_surface->commit();

    m_surface->frameSubmitted();
    EXPECT_EQ(0u, m_surface->committedFrameCallbackCount());
    EXPECT_EQ(1u, m_surface->inFlightFrameCallbackCount());
    EXPECT_TRUE(m_fired.empty()) << "the frame has not reached the display yet";

    m_surface->frameDrawn(4242);
    ASSERT_EQ(1u, m_fired.size());
    ASSERT_TRUE(m_fired[0].has_value());
    EXPECT_EQ(4242u, *m_fired[0]);
    EXPECT_EQ(0u, m_surface->inFlightFrameCallbackCount());

    // Presenting again fires nothing.
    m_surface->frameDrawn(5000);
    EXPECT_EQ(1u, m_fired.size());
}

TEST_F(SurfaceTest, OutputViewportDrivesOnlyIntersectingSurfaceCallbacks)
{
    auto second = m_compositor.createSurface();
    m_surface->attach(makeBuffer(base::Size(40, 40)), {});
    second->attach(makeBuffer(base::Size(40, 40)), {});
    m_surface->setPosition({10, 10});
    second->setPosition({120, 10});
    m_surface->requestFrameCallback([this](std::optional<uint32_t> time) {
        m_fired.push_back(time);
    });
    second->requestFrameCallback([this](std::optional<uint32_t> time) {
        m_fired.push_back(time);
    });
    m_surface->commit();
    second->commit();

    m_compositor.frameSubmittedForOutput(base::Rect(100, 0, 100, 100), 9);
    EXPECT_EQ(m_surface->committedFrameCallbackCount(), 1u);
    EXPECT_EQ(second->committedFrameCallbackCount(), 0u);
    m_compositor.frameDrawn(55, 9);
    ASSERT_EQ(m_fired.size(), 1u);
    EXPECT_EQ(m_fired.front(), std::optional<uint32_t>(55));
}

TEST_F(SurfaceTest, PreferredScaleUsesHighestIntersectedOutput)
{
    m_surface->attach(makeBuffer(base::Size(80, 40)), {});
    m_surface->setPosition({70, 10});
    m_surface->commit();
    m_compositor.setOutputViews(
            {{"left", base::Rect(0, 0, 100, 100), OutputScale()},
             {"right", base::Rect(100, 0, 100, 100), *OutputScale::fromNumerator(180)}});
    EXPECT_EQ(m_surface->preferredScale(), 180u);

    m_surface->setPosition({10, 10});
    EXPECT_EQ(m_surface->preferredScale(), 120u);
}

TEST_F(SurfaceTest, PresentingBeforeSubmittingLeavesTheCallbackQueued)
{
    m_surface->requestFrameCallback([this](std::optional<uint32_t> time) {
        m_fired.push_back(time);
    });
    m_surface->commit();

    m_surface->frameDrawn(1);
    EXPECT_TRUE(m_fired.empty());
    EXPECT_EQ(1u, m_surface->committedFrameCallbackCount());
}

TEST_F(SurfaceTest, DestroyingTheSurfaceCancelsFrameCallbacks)
{
    const auto record = [this](std::optional<uint32_t> time) {
        m_fired.push_back(time);
    };

    m_surface->requestFrameCallback(record);
    m_surface->commit();
    m_surface->requestFrameCallback(record);
    m_surface->frameSubmitted();
    m_surface->requestFrameCallback(record);

    m_surface.reset();
    ASSERT_EQ(3u, m_fired.size());
    for (const std::optional<uint32_t> &time : m_fired)
        EXPECT_FALSE(time.has_value()) << "cancelled callbacks carry no presentation time";
}

TEST_F(SurfaceTest, TheNodeFollowsTheCommittedContentSize)
{
    ASSERT_NE(nullptr, m_surface->node());
    EXPECT_TRUE(m_surface->node()->geometry().size().isEmpty());

    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(48, 24));
    m_surface->attach(buffer, base::Point());
    m_surface->commit();
    EXPECT_EQ(base::Size(48, 24), m_surface->node()->geometry().size());
    EXPECT_EQ(m_surface->position(), m_surface->node()->geometry().topLeft());
}

TEST_F(SurfaceTest, OnlyWhatChangedInTheBufferIsUploaded)
{
    std::shared_ptr<ShmBuffer> a = makeBuffer(base::Size(64, 64));
    std::shared_ptr<ShmBuffer> b = makeBuffer(base::Size(64, 64));
    m_surface->attach(a, base::Point());
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    ASSERT_EQ(1, m_renderer.createdTextures);

    m_surface->attach(b, base::Point());
    m_surface->damageBuffer(base::Rect(1, 2, 3, 4));
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_EQ(m_renderer.lastUploadDamage, base::Region(base::Rect(1, 2, 3, 4)));

    // Commits between two uploads add up.
    m_surface->attach(a, base::Point());
    m_surface->damage(base::Rect(0, 0, 1, 1));
    m_surface->commit();
    m_surface->attach(b, base::Point());
    m_surface->damageBuffer(base::Rect(10, 10, 2, 2));
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_EQ(m_renderer.lastUploadDamage, base::Region(base::Rect(0, 0, 1, 1)) + base::Rect(10, 10, 2, 2));

    // A new mapping cannot be told from new content; after it, surface damage
    // is taken through the buffer scale.
    m_surface->attach(a, base::Point());
    m_surface->setBufferScale(2);
    m_surface->damage(base::Rect(0, 0, 1, 1));
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_EQ(m_renderer.lastUploadDamage, base::Region(base::Rect(0, 0, 64, 64)));
    m_surface->attach(b, base::Point());
    m_surface->damage(base::Rect(3, 3, 1, 1));
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_EQ(m_renderer.lastUploadDamage, base::Region(base::Rect(6, 6, 2, 2)));

    // Undamaged, the new buffer holds what the texture has.
    m_surface->attach(a, base::Point());
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_TRUE(m_renderer.lastUploadDamage->isEmpty());
    EXPECT_EQ(1, m_renderer.createdTextures);
}

TEST_F(SurfaceTest, TheTextureIsUploadedOnlyWhenTheContentsChanged)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(16, 16));
    m_surface->attach(buffer, base::Point());
    m_surface->commit();

    EXPECT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_EQ(1, m_renderer.createdTextures);
    EXPECT_FALSE(m_surface->updateTexture(m_renderer));
    EXPECT_EQ(1, m_renderer.createdTextures);

    // The same geometry and format re-uses the texture rather than making one.
    m_surface->attach(buffer, base::Point());
    m_surface->commit();
    EXPECT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_EQ(1, m_renderer.createdTextures);

    // A different size needs a new texture.
    std::shared_ptr<ShmBuffer> smaller = makeBuffer(base::Size(8, 8));
    m_surface->attach(smaller, base::Point());
    m_surface->commit();
    EXPECT_TRUE(m_surface->updateTexture(m_renderer));
    EXPECT_EQ(2, m_renderer.createdTextures);
}

TEST_F(SurfaceTest, TheUploadedPixelsCarryTheBufferFormat)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(2, 2), render::PixelFormat::Bgrx8888);
    m_surface->attach(buffer, base::Point());
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));

    ASSERT_NE(nullptr, m_surface->node()->texture());
    EXPECT_EQ(render::PixelFormat::Bgrx8888, m_surface->node()->texture()->format());
}

TEST_F(SurfaceTest, SurfacesRegisterWithTheCompositor)
{
    EXPECT_EQ(1u, m_compositor.surfaces().size());
    std::unique_ptr<Surface> second = m_compositor.createSurface();
    EXPECT_EQ(2u, m_compositor.surfaces().size());
    // Without a window manager, surfaces are cascaded rather than stacked.
    EXPECT_NE(m_surface->position(), second->position());

    second.reset();
    EXPECT_EQ(1u, m_compositor.surfaces().size());
}

TEST_F(SurfaceTest, DestroyAttachedBuffer)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(1, 1));
    m_surface->attach(buffer, base::Point());
    m_surface->commit();
    ASSERT_TRUE(m_surface->updateTexture(m_renderer));

    // The content keeps its size after the client destroys the buffer.
    buffer.reset();
    m_surface->commit();
    m_surface->updateTexture(m_renderer);
    EXPECT_EQ(base::Size(1, 1), m_surface->contentSize());
    EXPECT_TRUE(m_surface->isMapped());
}

TEST_F(SurfaceTest, CommitWithoutAnythingPending)
{
    m_surface->commit();
    m_surface->commit();
    EXPECT_FALSE(m_surface->hasContents());
    EXPECT_FALSE(m_surface->isMapped());
}

class RecordingObserver : public SurfaceObserver
{
public:
    explicit RecordingObserver(Surface *surface)
        : m_surface(surface)
    {
        m_surface->addSurfaceObserver(this);
    }

    ~RecordingObserver() override
    {
        if (m_surface)
            m_surface->removeSurfaceObserver(this);
    }

    void onSurfaceDestroying(Surface *surface) override
    {
        EXPECT_EQ(m_surface, surface);
        ++destroying;
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

    void onSurfaceCommit(Surface *) override { ++others; }

    void onScaleFactorChanged(Surface *) override { ++others; }

    void onSurfaceMenuChanged(Surface *) override { ++others; }

    void onSurfaceWindowHintsChanged(Surface *) override { ++others; }

    bool onSurfaceStateApplying(Surface *) override
    {
        ++others;
        return true;
    }

    int destroying = 0;
    int others = 0;

private:
    Surface *m_surface;
};

// Exo's ObserversNotCalledWhenDestroying and NoOcclusionUpdateOnDestroyingSurface:
// tearing down a parent detaches its children, and that must not reach the
// parent's observers as anything but its destruction.
TEST_F(SurfaceTest, ObserversNotCalledWhenDestroying)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(16, 16));
    std::unique_ptr<Surface> child = m_compositor.createSurface();
    std::unique_ptr<SubSurface> sub = SubSurface::create(child.get(), m_surface.get());
    child->attach(buffer, base::Point());
    child->commit();
    m_surface->attach(buffer, base::Point());
    m_surface->commit();
    m_surface->updateTexture(m_renderer);

    RecordingObserver parentObserver(m_surface.get());
    RecordingObserver childObserver(child.get());
    m_surface.reset();
    EXPECT_EQ(1, parentObserver.destroying);
    EXPECT_EQ(0, parentObserver.others);
    EXPECT_EQ(0, childObserver.destroying);

    // The orphaned child is unmapped but stays a usable surface.
    EXPECT_FALSE(child->isMapped());
    child->commit();
    sub.reset();
    child.reset();
    EXPECT_EQ(1, childObserver.destroying);
}

// Exo's BufferTest.ProduceTransferableResourceFailure: a texture that cannot
// be created leaves the surface showing nothing, lets the client have its
// buffer back, and the next commit tries again.
TEST_F(SurfaceTest, TextureCreationFailureShowsNothingAndRecovers)
{
    std::shared_ptr<ShmBuffer> buffer = makeBuffer(base::Size(32, 16));
    int releases = 0;
    buffer->setReleaseCallback([&releases] {
        ++releases;
    });

    m_renderer.failTextureCreation = true;
    m_surface->attach(buffer, base::Point());
    m_surface->commit();
    EXPECT_FALSE(m_surface->updateTexture(m_renderer));
    EXPECT_EQ(nullptr, m_surface->node()->texture());
    EXPECT_EQ(1, releases);
    m_renderer.calls.clear();
    m_scene.root()->render(m_renderer);
    for (const auto &call : m_renderer.calls)
        EXPECT_EQ(nullptr, call.texture);

    m_renderer.failTextureCreation = false;
    m_surface->attach(buffer, base::Point());
    m_surface->damage(base::Rect(0, 0, 4, 4));
    m_surface->commit();
    EXPECT_TRUE(m_surface->updateTexture(m_renderer));
    ASSERT_NE(nullptr, m_surface->node()->texture());
    EXPECT_EQ(base::Size(32, 16), m_surface->node()->texture()->size());
    EXPECT_EQ(2, releases);
}

} // namespace

TEST_F(SurfaceTest, BackgroundBlurIsCommittedPersistentSurfaceState)
{
    const base::Rect region(2, 3, 100, 80);
    m_surface->setBackgroundBlur(region, 12);
    EXPECT_TRUE(m_surface->backgroundBlurRect().isEmpty());
    EXPECT_FALSE(m_surface->node()->hasBackgroundBlur());
    m_surface->commit();
    EXPECT_EQ(m_surface->backgroundBlurRect(), region);
    EXPECT_EQ(m_surface->backgroundBlurRadius(), 12);
    EXPECT_TRUE(m_surface->node()->hasBackgroundBlur());
    m_surface->setBufferScale(2);
    m_surface->commit();
    EXPECT_EQ(m_surface->backgroundBlurRect(), region);
    m_surface->setBackgroundBlur({}, 0);
    EXPECT_TRUE(m_surface->node()->hasBackgroundBlur());
    m_surface->commit();
    EXPECT_FALSE(m_surface->node()->hasBackgroundBlur());
}
