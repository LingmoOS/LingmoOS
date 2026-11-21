// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/hardware_display_plane_manager_atomic.cc
//           (CreateMergedGpuFenceFromFDs)
//           chromium/src/components/exo/buffer.cc (release callbacks)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Explicit synchronization: fence handling and the buffer-release rules that
// keep a client from reusing storage the compositor is still reading.
//
// Adapted for Lingmo: exercises the corresponding behavior with native
// compositor test doubles and APIs.

#include <gtest/gtest.h>

#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <iterator>
#include <memory>
#include <vector>

#include "compositor/buffer.h"
#include "compositor/shm_pool.h"
#include "fake_fence.h"
#include "render/sync_fence.h"

namespace Lingmo {

namespace {

using testsupport::FakeFence;

// Records what a client's release timeline was told, standing in for a DRM
// syncobj point.
class RecordingRelease final : public compositor::BufferRelease
{
public:
    void signal(render::SyncFence fence) override
    {
        ++signalCount;
        hadFence = fence.isValid();
        received = std::move(fence);
    }

    int signalCount = 0;
    bool hadFence = false;
    render::SyncFence received;
};

// A DMA-BUF-like buffer: the compositor samples client storage in place, so
// release waits for detach rather than for an upload.
class RetainingBuffer final : public compositor::Buffer
{
public:
    Lingmo::base::Size size() const override { return {64, 64}; }
    render::PixelFormat format() const override { return render::PixelFormat::Bgrx8888; }
    const uint8_t* pixels() const override { return nullptr; }
    int stride() const override { return 256; }
};

int openFdCount()
{
    return int(std::distance(std::filesystem::directory_iterator("/proc/self/fd"),
                             std::filesystem::directory_iterator()));
}

} // namespace

TEST(SyncFenceTest, AnInvalidFenceIsAlreadySignalled)
{
    const render::SyncFence fence;

    EXPECT_FALSE(fence.isValid());
    // "Nothing to wait for" is what makes the implicit-sync fallback safe.
    EXPECT_TRUE(fence.isSignaled());
    EXPECT_TRUE(fence.wait(0));
}

TEST(SyncFenceTest, APendingFenceIsNotSignalledUntilItFires)
{
    FakeFence source;
    const render::SyncFence fence = source.take();
    ASSERT_TRUE(fence.isValid());

    EXPECT_FALSE(fence.isSignaled());
    source.signal();
    EXPECT_TRUE(fence.isSignaled());
}

TEST(SyncFenceTest, OnlyASignalledSyncFileHasASignalTime)
{
    EXPECT_FALSE(render::SyncFence().signalTimeNs());
    FakeFence pipe;
    auto fence = pipe.take();
    EXPECT_FALSE(fence.signalTimeNs());
}

TEST(SyncFenceTest, DuplicatingAFenceSharesTheSameSignal)
{
    FakeFence source;
    const render::SyncFence fence = source.take();
    const render::SyncFence copy = fence.duplicate();

    ASSERT_TRUE(copy.isValid());
    EXPECT_NE(copy.get(), fence.get());
    EXPECT_FALSE(copy.isSignaled());
    source.signal();
    EXPECT_TRUE(copy.isSignaled());
}

TEST(SyncFenceTest, DuplicatingAnInvalidFenceStaysInvalid)
{
    EXPECT_FALSE(render::SyncFence().duplicate().isValid());
}

TEST(SyncFenceTest, MergingNothingYieldsAnInvalidFence)
{
    std::vector<render::SyncFence> fences;
    fences.push_back(render::SyncFence());
    fences.push_back(render::SyncFence());

    EXPECT_FALSE(render::SyncFence::merge(std::move(fences)).isValid());
}

TEST(SyncFenceTest, MergingOneFenceKeepsIt)
{
    FakeFence source;
    std::vector<render::SyncFence> fences;
    fences.push_back(render::SyncFence());
    fences.push_back(source.take());

    const render::SyncFence merged = render::SyncFence::merge(std::move(fences));

    ASSERT_TRUE(merged.isValid());
    EXPECT_FALSE(merged.isSignaled());
    source.signal();
    EXPECT_TRUE(merged.isSignaled());
}

TEST(BufferSyncTest, TheAcquireFenceTravelsWithTheBuffer)
{
    FakeFence source;
    RetainingBuffer buffer;

    EXPECT_FALSE(buffer.acquireFence().isValid());
    buffer.setAcquireFence(source.take());
    EXPECT_TRUE(buffer.acquireFence().isValid());
    EXPECT_FALSE(buffer.acquireFence().isSignaled());

    source.signal();
    EXPECT_TRUE(buffer.acquireFence().isSignaled());
}

TEST(BufferSyncTest, ReleasePointsAreSignalledWhenTheBufferIsReleased)
{
    RetainingBuffer buffer;
    auto release = std::make_shared<RecordingRelease>();
    buffer.addRelease(release);

    buffer.onAttach();
    EXPECT_EQ(release->signalCount, 0);

    buffer.onDetach();
    EXPECT_EQ(release->signalCount, 1);
    EXPECT_EQ(buffer.pendingReleaseCount(), 0u);
}

TEST(BufferSyncTest, AReleaseIsNeverDroppedWhenTheBufferGoesAway)
{
    auto release = std::make_shared<RecordingRelease>();
    {
        RetainingBuffer buffer;
        buffer.addRelease(release);
        buffer.onAttach();
    }

    // A client waiting on the release point must not wait forever just because
    // the compositor tore the buffer down first.
    EXPECT_EQ(release->signalCount, 1);
}

TEST(BufferSyncTest, ABufferOnAPlaneIsNotReleasedWhenTheClientReattaches)
{
    RetainingBuffer buffer;
    auto release = std::make_shared<RecordingRelease>();
    buffer.addRelease(release);

    buffer.onAttach();
    buffer.onScanoutAcquired();

    // The client committed something else, so nothing references the buffer any
    // more - except the display, which is still showing it.
    buffer.onDetach();
    EXPECT_EQ(release->signalCount, 0);
    EXPECT_FALSE(buffer.isReleased());

    buffer.onScanoutReleased(render::SyncFence());
    EXPECT_EQ(release->signalCount, 1);
    EXPECT_TRUE(buffer.isReleased());
}

TEST(BufferSyncTest, TheScanoutCompletionFenceReachesTheReleasePoint)
{
    FakeFence completion;
    RetainingBuffer buffer;
    auto release = std::make_shared<RecordingRelease>();
    buffer.addRelease(release);

    buffer.onAttach();
    buffer.onScanoutAcquired();
    buffer.onDetach();
    buffer.onScanoutReleased(completion.take());

    ASSERT_EQ(release->signalCount, 1);
    // The client is told when the display actually stops reading, not merely
    // that the compositor stopped caring.
    EXPECT_TRUE(release->hadFence);
}

TEST(BufferSyncTest, ABufferKeptOnAPlaneDoesNotCollectAnOutFencePerFlip)
{
    RetainingBuffer buffer;
    buffer.onAttach();
    buffer.onScanoutAcquired();

    const int before = openFdCount();
    // Each frame acquires the buffer before the flip retires the previous
    // frame, so the scanout count never reaches zero while it stays on screen.
    for (int flip = 0; flip < 64; ++flip) {
        buffer.onScanoutAcquired();
        FakeFence outFence;
        outFence.signal();
        buffer.onScanoutReleased(outFence.take());
    }

    EXPECT_EQ(openFdCount(), before);
}

TEST(BufferSyncTest, APendingOutFenceOutlivesTheSignalledOnesAfterIt)
{
    FakeFence pending;
    RetainingBuffer buffer;
    auto release = std::make_shared<RecordingRelease>();
    buffer.addRelease(release);

    buffer.onAttach();
    buffer.onScanoutAcquired();
    buffer.onScanoutAcquired();
    buffer.onScanoutReleased(pending.take());
    buffer.onScanoutAcquired();
    FakeFence landed;
    landed.signal();
    buffer.onScanoutReleased(landed.take());
    buffer.onDetach();
    buffer.onScanoutReleased(render::SyncFence());

    ASSERT_EQ(release->signalCount, 1);
    ASSERT_TRUE(release->received.isValid());
    EXPECT_FALSE(release->received.isSignaled());
    pending.signal();
    EXPECT_TRUE(release->received.isSignaled());
}

TEST(BufferSyncTest, ABufferOnSeveralPlanesWaitsForAllOfThem)
{
    RetainingBuffer buffer;
    auto release = std::make_shared<RecordingRelease>();
    buffer.addRelease(release);

    buffer.onAttach();
    buffer.onScanoutAcquired();
    buffer.onScanoutAcquired();
    buffer.onDetach();

    buffer.onScanoutReleased(render::SyncFence());
    EXPECT_EQ(release->signalCount, 0);
    buffer.onScanoutReleased(render::SyncFence());
    EXPECT_EQ(release->signalCount, 1);
}

TEST(BufferSyncTest, TheSameBufferOnTwoSurfacesOwesTwoReleases)
{
    RetainingBuffer buffer;
    auto first = std::make_shared<RecordingRelease>();
    auto second = std::make_shared<RecordingRelease>();
    buffer.addRelease(first);
    buffer.addRelease(second);

    buffer.onAttach();
    buffer.onAttach();
    buffer.onDetach();
    EXPECT_EQ(first->signalCount, 0);

    buffer.onDetach();
    EXPECT_EQ(first->signalCount, 1);
    EXPECT_EQ(second->signalCount, 1);
}

} // namespace Lingmo
