// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_gpu_display_manager.cc
//           chromium/src/ui/ozone/platform/drm/gpu/screen_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: atomic TEST_ONLY and real commit share the exact same
// property request and publish state only after both succeed.

#include "backend/drm/drm_commit_coordinator.h"

#include <map>
#include <xf86drmMode.h>

namespace Lingmo::Backend {

DrmCommitCoordinator::DrmCommitCoordinator(DrmDeviceApi *device)
    : m_device(device)
{
}

std::string drmCommitMessage(DrmCommitStatus status)
{
    switch (status) {
    case DrmCommitStatus::Success:
        return "DRM configuration succeeded";
    case DrmCommitStatus::InvalidRequest:
        return "Invalid or conflicting DRM resources";
    case DrmCommitStatus::TransferUnavailable:
        return "No cross-device scanout transfer supports this mode";
    case DrmCommitStatus::PreparationFailed:
        return "DRM target or mode allocation failed";
    case DrmCommitStatus::TestFailed:
        return "Combined DRM TEST_ONLY failed";
    case DrmCommitStatus::Invalidated:
        return "DRM routes or session changed before commit";
    case DrmCommitStatus::RollbackFailed:
        return "DRM compensation failed; scanout state requires reprobe";
    case DrmCommitStatus::CommitFailed:
        return "DRM commit failed after TEST_ONLY succeeded";
    }
    return {};
}

DrmCommitStatus DrmCommitCoordinator::execute(const AtomicRequest &request, bool testOnly,
                                              uint32_t flags, void *eventData,
                                              std::function<bool()> valid)
{
    if (!m_device || request.properties.empty()
        || (flags & (DRM_MODE_ATOMIC_TEST_ONLY | DRM_MODE_ATOMIC_NONBLOCK)))
        return DrmCommitStatus::InvalidRequest;
    std::map<std::pair<uint32_t, uint32_t>, uint64_t> assignments;
    for (const auto &property : request.properties) {
        const auto [it, inserted] = assignments.emplace(
                std::make_pair(property.objectId, property.propertyId), property.value);
        if (!property.objectId || !property.propertyId
            || (!inserted && it->second != property.value))
            return DrmCommitStatus::InvalidRequest;
    }
    if (valid && !valid())
        return DrmCommitStatus::Invalidated;
    const uint32_t testFlags = (flags & ~DRM_MODE_PAGE_FLIP_EVENT) | DRM_MODE_ATOMIC_TEST_ONLY;
    if (!m_device->atomicCommit(request, testFlags, nullptr))
        return DrmCommitStatus::TestFailed;
    if (valid && !valid())
        return DrmCommitStatus::Invalidated;
    if (testOnly)
        return DrmCommitStatus::Success;
    if (!m_device->atomicCommit(request, flags, eventData))
        return DrmCommitStatus::CommitFailed;
    m_appliedRequest = request;
    ++m_revision;
    return DrmCommitStatus::Success;
}

bool DrmCommitCoordinator::testAndCommit(const AtomicRequest &request, uint32_t flags,
                                         void *eventData)
{
    return execute(request, false, flags, eventData) == DrmCommitStatus::Success;
}

} // namespace Lingmo::Backend
