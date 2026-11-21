// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/screen_manager.cc
//           chromium/src/ui/ozone/platform/drm/gpu/drm_gpu_display_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: solve connector, CRTC, and primary-plane ownership as
// one deterministic device-wide assignment without Chromium's GPU process.

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace Lingmo::Backend {

struct DrmRouteCandidate {
    uint32_t connectorId = 0;
    uint32_t possibleCrtcs = 0;
    uint32_t currentCrtcId = 0;
};

struct DrmCrtcCandidate {
    uint32_t crtcId = 0;
    int index = -1;
    std::vector<uint32_t> primaryPlanes;
};

struct DrmRouteAssignment {
    uint32_t connectorId = 0;
    uint32_t crtcId = 0;
    int crtcIndex = -1;
    uint32_t primaryPlaneId = 0;
};

class DrmConfigurationPlanner
{
public:
    static std::optional<std::vector<DrmRouteAssignment>>
    assign(const std::vector<DrmRouteCandidate> &routes, const std::vector<DrmCrtcCandidate> &crtcs,
           int alternative = 0);
};

} // namespace Lingmo::Backend
