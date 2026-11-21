// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A commit is held until the client's GPU has finished writing its buffer, and
// every later commit in the same surface tree waits behind it.

#include <gtest/gtest.h>

#include <fcntl.h>
#include <unistd.h>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "compositor/buffer.h"
#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "fake_fence.h"
#include "loop_test_util.h"
#include "render/drm_syncobj.h"
#include "scene/scene.h"

using namespace Lingmo;
using namespace Lingmo::Compositor;
using Lingmo::testsupport::FakeFence;

namespace {

// A DMA-BUF-like buffer whose implicit fences the test controls.
class FencedBuffer final : public Buffer
{
public:
    Lingmo::base::Size size() const override { return {64, 64}; }
    render::PixelFormat format() const override { return render::PixelFormat::Bgrx8888; }
    const uint8_t *pixels() const override { return nullptr; }
    int stride() const override { return 256; }
    bool supportsExplicitSync() const override { return true; }

    std::vector<render::SyncFence> implicitFences() const override
    {
        ++implicitFenceQueries;
        return std::move(fences);
    }

    mutable std::vector<render::SyncFence> fences;
    mutable int implicitFenceQueries = 0;
};

class RecordingRelease final : public BufferRelease
{
public:
    void signal(render::SyncFence) override { ++signalCount; }

    int signalCount = 0;
};

// Takes a value as double-buffered role state, and records the value each
// applied commit installed.
class RoleObserver final : public SurfaceObserver
{
public:
    explicit RoleObserver(Surface *surface)
        : m_surface(surface)
    {
        m_surface->addSurfaceObserver(this);
    }

    ~RoleObserver() override
    {
        if (m_surface)
            m_surface->removeSurfaceObserver(this);
    }

    void onSurfaceDestroying(Surface *surface) override
    {
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

    std::function<void()> onSurfaceCommitRequested(Surface *) override
    {
        return [this, value = pending] {
            installed = value;
        };
    }

    void onSurfaceCommit(Surface *) override { applied.push_back(installed); }

    int pending = 0;
    int installed = -1;
    std::vector<int> applied;

private:
    Surface *m_surface;
};

bool waitFor(const std::function<bool()> &condition)
{
    return Lingmo::test::runUntil(condition, std::chrono::seconds(2));
}

class CommitReadinessTest : public testing::Test
{
protected:
    void SetUp() override { m_surface = m_compositor.createSurface(); }

    // Attaches a buffer whose acquire fence is `fence`, and commits.
    std::shared_ptr<FencedBuffer> commitFenced(Surface *surface, FakeFence &fence)
    {
        auto buffer = std::make_shared<FencedBuffer>();
        surface->attach(buffer, base::Point());
        surface->setAcquireFence(fence.take());
        surface->commit();
        return buffer;
    }

    // Outlives the surfaces, whose destructors cancel callbacks that write here.
    std::vector<std::optional<uint32_t>> m_fired;

    scene::Scene m_scene;
    Compositor m_compositor {m_scene.root()};
    std::unique_ptr<Surface> m_surface;
};

TEST_F(CommitReadinessTest, ACommitWaitsForItsAcquireFence)
{
    FakeFence fence;
    auto buffer = commitFenced(m_surface.get(), fence);

    EXPECT_EQ(nullptr, m_surface->buffer());
    EXPECT_EQ(1u, m_surface->heldCommitCount());
    // The client handed the buffer over at commit, so it is not released.
    EXPECT_FALSE(buffer->isReleased());
    EXPECT_FALSE(m_surface->hasPendingAttachedBuffer());

    fence.signal();
    ASSERT_TRUE(waitFor([&] {
        return m_surface->buffer() == buffer;
    }));
    EXPECT_EQ(0u, m_surface->heldCommitCount());
}

TEST_F(CommitReadinessTest, ASignalledFenceAppliesAtOnce)
{
    FakeFence fence;
    fence.signal();
    auto buffer = commitFenced(m_surface.get(), fence);

    EXPECT_EQ(buffer, m_surface->buffer());
    EXPECT_EQ(0u, m_surface->heldCommitCount());
}

TEST_F(CommitReadinessTest, ImplicitFencesHoldTheCommit)
{
    FakeFence fence;
    auto buffer = std::make_shared<FencedBuffer>();
    buffer->fences.push_back(fence.take());
    m_surface->attach(buffer, base::Point());
    m_surface->commit();
    EXPECT_EQ(nullptr, m_surface->buffer());

    fence.signal();
    ASSERT_TRUE(waitFor([&] {
        return m_surface->buffer() == buffer;
    }));
}

TEST_F(CommitReadinessTest, ExplicitSyncIgnoresImplicitFences)
{
    FakeFence implicit;
    FakeFence acquire;
    acquire.signal();
    auto buffer = std::make_shared<FencedBuffer>();
    buffer->fences.push_back(implicit.take());
    m_surface->attach(buffer, base::Point());
    m_surface->setAcquireFence(acquire.take());
    m_surface->commit();

    EXPECT_EQ(buffer, m_surface->buffer());
    EXPECT_EQ(0, buffer->implicitFenceQueries);
}

TEST_F(CommitReadinessTest, LaterCommitsWaitBehindAHeldOne)
{
    FakeFence fence;
    m_surface->requestFrameCallback([this](std::optional<uint32_t> time) {
        m_fired.push_back(time);
    });
    auto buffer = commitFenced(m_surface.get(), fence);

    // Nothing to wait for, but it must not overtake the first.
    m_surface->damage(base::Rect(0, 0, 8, 8));
    m_surface->requestFrameCallback([this](std::optional<uint32_t> time) {
        m_fired.push_back(time);
    });
    m_surface->commit();
    EXPECT_EQ(2u, m_surface->heldCommitCount());
    EXPECT_EQ(0u, m_surface->committedFrameCallbackCount());

    fence.signal();
    ASSERT_TRUE(waitFor([&] {
        return m_surface->heldCommitCount() == 0;
    }));
    EXPECT_EQ(buffer, m_surface->buffer());
    EXPECT_EQ(2u, m_surface->committedFrameCallbackCount());
    EXPECT_TRUE(m_fired.empty());
}

TEST_F(CommitReadinessTest, RoleStateIsTakenWhenTheClientCommits)
{
    RoleObserver role(m_surface.get());
    FakeFence fence;
    role.pending = 1;
    commitFenced(m_surface.get(), fence);
    role.pending = 2;
    m_surface->commit();
    role.pending = 3;
    EXPECT_TRUE(role.applied.empty());

    fence.signal();
    ASSERT_TRUE(waitFor([&] {
        return role.applied.size() == 2;
    }));
    EXPECT_EQ((std::vector<int> {1, 2}), role.applied);
}

TEST_F(CommitReadinessTest, ARemovedObserverDropsItsHeldRoleState)
{
    auto role = std::make_unique<RoleObserver>(m_surface.get());
    FakeFence fence;
    commitFenced(m_surface.get(), fence);
    role.reset();

    fence.signal();
    EXPECT_TRUE(waitFor([&] {
        return m_surface->heldCommitCount() == 0;
    }));
}

TEST_F(CommitReadinessTest, OtherSurfacesDoNotWait)
{
    FakeFence fence;
    commitFenced(m_surface.get(), fence);

    std::unique_ptr<Surface> other = m_compositor.createSurface();
    auto buffer = std::make_shared<FencedBuffer>();
    other->attach(buffer, base::Point());
    other->commit();
    EXPECT_EQ(buffer, other->buffer());
    EXPECT_EQ(1u, m_surface->heldCommitCount());
}

TEST_F(CommitReadinessTest, ADesynchronizedChildWaitsBehindItsParent)
{
    std::unique_ptr<Surface> child = m_compositor.createSurface();
    std::unique_ptr<SubSurface> sub = SubSurface::create(child.get(), m_surface.get());
    sub->setCommitBehavior(false);

    FakeFence fence;
    auto parentBuffer = commitFenced(m_surface.get(), fence);
    auto childBuffer = std::make_shared<FencedBuffer>();
    child->attach(childBuffer, base::Point());
    child->commit();
    EXPECT_EQ(nullptr, child->buffer());
    EXPECT_EQ(1u, child->heldCommitCount());

    fence.signal();
    ASSERT_TRUE(waitFor([&] {
        return child->buffer() == childBuffer;
    }));
    EXPECT_EQ(parentBuffer, m_surface->buffer());
}

TEST_F(CommitReadinessTest, ASynchronizedChildCommitWaitsForTheParentsHeldCommit)
{
    std::unique_ptr<Surface> child = m_compositor.createSurface();
    std::unique_ptr<SubSurface> sub = SubSurface::create(child.get(), m_surface.get());

    // The child's state is cached before the parent's commit, which applies it.
    FakeFence fence;
    auto childBuffer = std::make_shared<FencedBuffer>();
    child->attach(childBuffer, base::Point());
    child->commit();
    auto parentBuffer = commitFenced(m_surface.get(), fence);
    EXPECT_EQ(nullptr, child->buffer());

    fence.signal();
    ASSERT_TRUE(waitFor([&] {
        return m_surface->buffer() == parentBuffer;
    }));
    // Both appear in the same application.
    EXPECT_EQ(childBuffer, child->buffer());
}

TEST_F(CommitReadinessTest, SetDesyncAppliesTheCacheInOrder)
{
    std::unique_ptr<Surface> child = m_compositor.createSurface();
    std::unique_ptr<SubSurface> sub = SubSurface::create(child.get(), m_surface.get());

    FakeFence fence;
    commitFenced(m_surface.get(), fence);
    auto childBuffer = std::make_shared<FencedBuffer>();
    child->attach(childBuffer, base::Point());
    child->commit();
    sub->setCommitBehavior(false);
    EXPECT_EQ(nullptr, child->buffer());

    fence.signal();
    ASSERT_TRUE(waitFor([&] {
        return child->buffer() == childBuffer;
    }));
    EXPECT_EQ(0u, child->heldCommitCount());
}

TEST_F(CommitReadinessTest, AChildLeavingTheTreeStopsWaiting)
{
    std::unique_ptr<Surface> child = m_compositor.createSurface();
    std::unique_ptr<SubSurface> sub = SubSurface::create(child.get(), m_surface.get());
    sub->setCommitBehavior(false);

    FakeFence fence;
    commitFenced(m_surface.get(), fence);
    auto childBuffer = std::make_shared<FencedBuffer>();
    child->attach(childBuffer, base::Point());
    child->commit();
    EXPECT_EQ(nullptr, child->buffer());

    sub.reset();
    ASSERT_TRUE(waitFor([&] {
        return child->buffer() == childBuffer;
    }));
    EXPECT_EQ(1u, m_surface->heldCommitCount());
}

TEST_F(CommitReadinessTest, DestroyingTheSurfaceSettlesItsHeldCommits)
{
    FakeFence fence;
    auto release = std::make_shared<RecordingRelease>();
    m_surface->requestFrameCallback([this](std::optional<uint32_t> time) {
        m_fired.push_back(time);
    });
    m_surface->setBufferRelease(release);
    auto buffer = commitFenced(m_surface.get(), fence);
    ASSERT_EQ(1u, m_surface->heldCommitCount());

    m_surface.reset();
    ASSERT_EQ(1u, m_fired.size());
    EXPECT_EQ(std::nullopt, m_fired.front());
    EXPECT_EQ(1, release->signalCount);
    EXPECT_EQ(0u, buffer->attachCount());
    EXPECT_TRUE(buffer->isReleased());
}

TEST_F(CommitReadinessTest, AHeldBufferSurvivesItsWlBuffer)
{
    FakeFence fence;
    auto buffer = commitFenced(m_surface.get(), fence);
    const std::weak_ptr<FencedBuffer> weak = buffer;
    buffer.reset();
    EXPECT_FALSE(weak.expired());

    fence.signal();
    ASSERT_TRUE(waitFor([&] {
        return m_surface->heldCommitCount() == 0;
    }));
    EXPECT_TRUE(m_surface->hasContents());
}

TEST_F(CommitReadinessTest, AnAcquirePointIsWaitedForUntilItSignals)
{
    const ScopedFd node(::open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC));
    if (!node.isValid())
        GTEST_SKIP() << "No render node for DRM syncobj timelines";
    std::shared_ptr<render::DrmSyncobj> timeline = render::DrmSyncobj::create(node.get());
    if (!timeline || !timeline->signalEventFd(1).isValid())
        GTEST_SKIP() << "The render node cannot wait on timeline points";

    // Not even submitted yet: a fence for it only comes later.
    auto buffer = std::make_shared<FencedBuffer>();
    m_surface->attach(buffer, base::Point());
    m_surface->setAcquirePoint(timeline, 1);
    m_surface->commit();
    EXPECT_EQ(nullptr, m_surface->buffer());
    EXPECT_EQ(0, buffer->implicitFenceQueries);

    ASSERT_TRUE(timeline->signal(1, render::SyncFence()));
    ASSERT_TRUE(waitFor([&] {
        return m_surface->buffer() == buffer;
    }));
}

} // namespace
