// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/gfx/gpu_fence_handle.h
//           chromium/src/ui/ozone/platform/drm/gpu/
//               hardware_display_plane_manager_atomic.cc
//               (CreateMergedGpuFenceFromFDs)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: gfx::GpuFenceHandle becomes a plain owned sync_file fd
// and sync_merge() is issued directly, since libsync is an Android library.

#ifndef LINGMO_RENDER_SYNC_FENCE_H_
#define LINGMO_RENDER_SYNC_FENCE_H_

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "core/scoped_fd.h"

namespace Lingmo::Render {

// One kernel sync_file: a fence that becomes signalled when the GPU or display
// work behind it has completed.
//
// An invalid fence means "nothing to wait for", which is always safe to treat
// as already signalled. That is what makes the implicit-sync fallback work:
// every path here accepts an empty fence and does nothing.
class SyncFence
{
public:
    SyncFence() = default;

    explicit SyncFence(ScopedFd fd)
        : m_fd(std::move(fd))
    {
    }

    SyncFence(SyncFence&&) noexcept = default;
    SyncFence& operator=(SyncFence&&) noexcept = default;

    bool isValid() const { return m_fd.isValid(); }

    int get() const { return m_fd.get(); }

    int release() { return m_fd.release(); }

    void reset() { m_fd.reset(); }

    // A second reference to the same fence. Invalid fences duplicate as invalid.
    SyncFence duplicate() const;

    // Whether the fence has already been signalled. Never blocks; an invalid
    // fence counts as signalled.
    bool isSignaled() const;

    // When the fence was signalled, in nanoseconds of CLOCK_MONOTONIC: the
    // latest of the fences it holds, as gfx::GpuFence::GetMaxTimestamp reads
    // it. Empty while unsignalled, and for anything but a sync_file.
    std::optional<int64_t> signalTimeNs() const;

    // Blocks until signalled or `timeoutMs` elapses (-1 waits forever). Only
    // the fallback paths call this: with KMS IN_FENCE_FD or a GPU-side wait
    // available the steady state never blocks the compositor thread.
    bool wait(int timeoutMs = -1) const;

    // A fence signalled when every input fence is. Invalid inputs are skipped,
    // so merging nothing yields an invalid, already-signalled fence.
    static SyncFence merge(std::vector<SyncFence> fences);

private:
    ScopedFd m_fd;
};

} // namespace Lingmo::Render

#endif // LINGMO_RENDER_SYNC_FENCE_H_
