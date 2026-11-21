// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/gfx/gpu_fence_handle.cc
//           chromium/src/ui/ozone/platform/drm/gpu/
//               hardware_display_plane_manager_atomic.cc
//               (CreateMergedGpuFenceFromFDs)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "render/sync_fence.h"

#include <fcntl.h>
#include <linux/sync_file.h>
#include <poll.h>
#include <sys/ioctl.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

namespace Lingmo::Render {

namespace {

// A fence that will not merge is waited out rather than dropped. Bounded so a
// wedged client cannot stall the compositor indefinitely.
constexpr int kMergeFallbackWaitMs = 100;

} // namespace

SyncFence SyncFence::duplicate() const
{
    if (!m_fd.isValid())
        return SyncFence();
    return SyncFence(ScopedFd(::fcntl(m_fd.get(), F_DUPFD_CLOEXEC, 0)));
}

bool SyncFence::isSignaled() const
{
    if (!m_fd.isValid())
        return true;
    return wait(0);
}

std::optional<int64_t> SyncFence::signalTimeNs() const
{
    if (!m_fd.isValid())
        return std::nullopt;
    sync_file_info info = {};
    if (::ioctl(m_fd.get(), SYNC_IOC_FILE_INFO, &info) != 0 || info.status != 1
        || info.num_fences == 0)
        return std::nullopt;
    std::vector<sync_fence_info> fences(info.num_fences);
    info.sync_fence_info = reinterpret_cast<uint64_t>(fences.data());
    if (::ioctl(m_fd.get(), SYNC_IOC_FILE_INFO, &info) != 0 || info.status != 1)
        return std::nullopt;
    int64_t latest = 0;
    for (const sync_fence_info &fence : fences)
        latest = std::max<int64_t>(latest, int64_t(fence.timestamp_ns));
    return latest;
}

bool SyncFence::wait(int timeoutMs) const
{
    if (!m_fd.isValid())
        return true;

    pollfd entry = {};
    entry.fd = m_fd.get();
    entry.events = POLLIN;

    int result = 0;
    do {
        result = ::poll(&entry, 1, timeoutMs);
    } while (result < 0 && errno == EINTR);

    // A closed or invalid fd polls back as POLLNVAL, which counts as ready
    // here: an unwaitable fence must not stall the frame forever.
    return result > 0;
}

SyncFence SyncFence::merge(std::vector<SyncFence> fences)
{
    SyncFence merged;
    for (SyncFence& fence : fences) {
        if (!fence.isValid())
            continue;
        if (!merged.isValid()) {
            merged = std::move(fence);
            continue;
        }

        sync_merge_data data = {};
        std::strncpy(data.name, "lingmo", sizeof(data.name) - 1);
        data.fd2 = fence.get();
        data.fence = -1;
        if (::ioctl(merged.get(), SYNC_IOC_MERGE, &data) == 0) {
            merged = SyncFence(ScopedFd(data.fence));
        } else {
            // Waiting on a subset of the fences would be unsafe, so a fence
            // that will not merge is waited out here instead.
            fence.wait(kMergeFallbackWaitMs);
        }
    }
    return merged;
}

} // namespace Lingmo::Render
