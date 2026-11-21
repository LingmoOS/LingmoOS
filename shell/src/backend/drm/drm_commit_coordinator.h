// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_gpu_display_manager.cc
//           chromium/src/ui/ozone/platform/drm/gpu/screen_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: test and commit one complete device request while the
// last successful request remains authoritative after either phase fails.

#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "backend/drm/drm_device_api.h"

namespace Lingmo::Backend {

enum class DrmCommitStatus {
    Success,
    InvalidRequest,
    PreparationFailed,
    TransferUnavailable,
    TestFailed,
    Invalidated,
    CommitFailed,
    RollbackFailed
};

std::string drmCommitMessage(DrmCommitStatus status);

class DrmCommitCoordinator
{
public:
    explicit DrmCommitCoordinator(DrmDeviceApi *device);

    DrmCommitStatus execute(const AtomicRequest &request, bool testOnly, uint32_t flags = 0,
                            void *eventData = nullptr, std::function<bool()> valid = {});

    bool testAndCommit(const AtomicRequest &request, uint32_t flags = 0, void *eventData = nullptr);

    const std::optional<AtomicRequest> &appliedRequest() const { return m_appliedRequest; }

    uint64_t revision() const { return m_revision; }

private:
    DrmDeviceApi *m_device = nullptr;
    std::optional<AtomicRequest> m_appliedRequest;
    uint64_t m_revision = 0;
};

} // namespace Lingmo::Backend
