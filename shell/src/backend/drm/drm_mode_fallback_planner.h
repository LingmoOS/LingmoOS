// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_gpu_display_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: bounded combined-mode retries downgrade only affected
// external base-connector groups and return structured link diagnostics.

#pragma once

#include <cstdint>
#include <functional>
#include <optional>

#include <string>
#include <vector>

namespace Lingmo::Backend {

struct DrmModeCandidate {
    std::string key;
    int width = 0;
    int height = 0;
    int refreshMilliHz = 0;
    int pixelClockKHz = 0;
    bool preferred = false;
};

struct DrmFallbackHead {
    uint32_t connectorId = 0;
    std::string connectorName;
    std::string baseConnectorKey;
    bool internal = false;
    std::vector<DrmModeCandidate> modes;
};

struct DrmFallbackTestResult {
    bool succeeded = false;
    std::vector<std::string> failedBaseConnectorKeys;
    std::string message;
    bool retryable = true;
};

struct DrmFallbackPlan {
    std::vector<DrmModeCandidate> modes;
    std::vector<std::string> downgradedConnectors;
    std::string diagnostic;
    int attempts = 0;
};

class DrmModeFallbackPlanner
{
public:
    using Tester = std::function<DrmFallbackTestResult(const std::vector<DrmModeCandidate> &)>;
    static std::optional<DrmFallbackPlan> choose(const std::vector<DrmFallbackHead> &heads,
                                                 Tester tester, int maximumAttempts = 64);
};

} // namespace Lingmo::Backend
