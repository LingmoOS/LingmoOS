// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_RENDER_DRM_SYNCOBJ_H_
#define LINGMO_RENDER_DRM_SYNCOBJ_H_

#include <cstdint>
#include <memory>

#include "core/scoped_fd.h"
#include "render/sync_fence.h"

namespace Lingmo::Render {

// A DRM synchronization object timeline, imported from a client through
// wp_linux_drm_syncobj_manager_v1.import_timeline.
//
// Chromium has no equivalent - Exo predates this protocol and synchronizes
// through gpu::SyncToken - so only the ioctl sequences are new here; the fence
// plumbing around them is the Ozone one.
class DrmSyncobj
{
public:
    DrmSyncobj(const DrmSyncobj&) = delete;
    DrmSyncobj& operator=(const DrmSyncobj&) = delete;
    ~DrmSyncobj();

    // Imports a client's syncobj fd onto `drmFd`, which must stay open for the
    // lifetime of the returned object. Returns nullptr when the fd is not a
    // syncobj, which the protocol reports as invalid_timeline.
    static std::shared_ptr<DrmSyncobj> import(int drmFd, ScopedFd syncobjFd);

    // Creates a fresh timeline the compositor owns. Used for tests.
    static std::shared_ptr<DrmSyncobj> create(int drmFd);

    // Whether `point` already has a fence attached, so exportFence() can
    // export it without waiting for the client to submit its work.
    bool isMaterialized(uint64_t point) const;

    // A sync_file for `point`. Returns an invalid fence when the point has no
    // fence yet, which the caller must treat as "synchronize some other way"
    // rather than "already signalled".
    SyncFence exportFence(uint64_t point) const;

    // An eventfd that turns readable once `point` signals, also when the
    // client submits its fence only later. Invalid when the kernel cannot.
    ScopedFd signalEventFd(uint64_t point) const;

    // Signals `point`, immediately when `fence` is invalid, otherwise when
    // `fence` signals. Returns false if the kernel rejected it.
    bool signal(uint64_t point, const SyncFence& fence);

private:
    DrmSyncobj(int drmFd, uint32_t handle);

    int m_drmFd = -1;
    uint32_t m_handle = 0;
};

} // namespace Lingmo::Render

#endif // LINGMO_RENDER_DRM_SYNCOBJ_H_
