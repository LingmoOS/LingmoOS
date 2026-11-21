// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/drm_syncobj.h"

#include <sys/eventfd.h>
#include <xf86drm.h>

#include <utility>

#include "core/logging.h"

namespace Lingmo::Render {

namespace {

// A binary syncobj used to move one timeline point in or out as a sync_file.
// The kernel has no ioctl that exports a timeline point directly.
class ScopedSyncobj
{
public:
    explicit ScopedSyncobj(int drmFd)
        : m_drmFd(drmFd)
    {
        if (drmSyncobjCreate(m_drmFd, 0, &m_handle) != 0)
            m_handle = 0;
    }

    ScopedSyncobj(const ScopedSyncobj&) = delete;
    ScopedSyncobj& operator=(const ScopedSyncobj&) = delete;

    ~ScopedSyncobj()
    {
        if (m_handle)
            drmSyncobjDestroy(m_drmFd, m_handle);
    }

    bool isValid() const { return m_handle != 0; }
    uint32_t handle() const { return m_handle; }

private:
    int m_drmFd = -1;
    uint32_t m_handle = 0;
};

} // namespace

DrmSyncobj::DrmSyncobj(int drmFd, uint32_t handle)
    : m_drmFd(drmFd)
    , m_handle(handle)
{
}

DrmSyncobj::~DrmSyncobj()
{
    if (m_handle)
        drmSyncobjDestroy(m_drmFd, m_handle);
}

std::shared_ptr<DrmSyncobj> DrmSyncobj::import(int drmFd, ScopedFd syncobjFd)
{
    if (drmFd < 0 || !syncobjFd.isValid())
        return nullptr;

    uint32_t handle = 0;
    if (drmSyncobjFDToHandle(drmFd, syncobjFd.get(), &handle) != 0 || handle == 0)
        return nullptr;

    return std::shared_ptr<DrmSyncobj>(new DrmSyncobj(drmFd, handle));
}

std::shared_ptr<DrmSyncobj> DrmSyncobj::create(int drmFd)
{
    if (drmFd < 0)
        return nullptr;

    uint32_t handle = 0;
    if (drmSyncobjCreate(drmFd, 0, &handle) != 0 || handle == 0)
        return nullptr;

    return std::shared_ptr<DrmSyncobj>(new DrmSyncobj(drmFd, handle));
}

bool DrmSyncobj::isMaterialized(uint64_t point) const
{
    uint32_t handle = m_handle;
    uint64_t value = point;
    // WAIT_AVAILABLE asks only whether a fence exists, not whether it signalled.
    return drmSyncobjTimelineWait(m_drmFd, &handle, &value, 1, 0,
                                  DRM_SYNCOBJ_WAIT_FLAGS_WAIT_AVAILABLE, nullptr)
        == 0;
}

SyncFence DrmSyncobj::exportFence(uint64_t point) const
{
    if (!m_handle)
        return SyncFence();

    if (!isMaterialized(point))
        return SyncFence();

    ScopedSyncobj binary(m_drmFd);
    if (!binary.isValid())
        return SyncFence();

    if (drmSyncobjTransfer(m_drmFd, binary.handle(), 0, m_handle, point, 0) != 0)
        return SyncFence();

    int fenceFd = -1;
    if (drmSyncobjExportSyncFile(m_drmFd, binary.handle(), &fenceFd) != 0)
        return SyncFence();

    return SyncFence(ScopedFd(fenceFd));
}

ScopedFd DrmSyncobj::signalEventFd(uint64_t point) const
{
    if (!m_handle)
        return ScopedFd();

    ScopedFd eventFd(::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
    if (!eventFd.isValid())
        return ScopedFd();

    // Without WAIT_AVAILABLE the eventfd waits for the signal, not the submission.
    if (drmSyncobjEventfd(m_drmFd, m_handle, point, eventFd.get(), 0) != 0)
        return ScopedFd();

    return eventFd;
}

bool DrmSyncobj::signal(uint64_t point, const SyncFence& fence)
{
    if (!m_handle)
        return false;

    if (!fence.isValid()) {
        uint32_t handle = m_handle;
        uint64_t value = point;
        return drmSyncobjTimelineSignal(m_drmFd, &handle, &value, 1) == 0;
    }

    ScopedSyncobj binary(m_drmFd);
    if (!binary.isValid())
        return false;

    if (drmSyncobjImportSyncFile(m_drmFd, binary.handle(), fence.get()) != 0)
        return false;

    return drmSyncobjTransfer(m_drmFd, m_handle, point, binary.handle(), 0, 0) == 0;
}

} // namespace Lingmo::Render
