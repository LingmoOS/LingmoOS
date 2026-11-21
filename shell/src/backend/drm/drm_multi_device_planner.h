// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_gpu_display_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: cross-device commits, framebuffer transfer fallback,
// tiled monitor grouping, and clone eligibility are explicit testable plans.

#pragma once

#include <functional>
#include <optional>

#include <cstdint>
#include <string>
#include <vector>

#include "base/geometry.h"

namespace Lingmo::Backend {

struct DrmDeviceTransaction {
    std::string deviceKey;
    std::function<bool()> test;
    std::function<bool()> commit;
    std::function<bool()> rollback;
};

struct DrmMultiDeviceResult {
    bool succeeded = false;
    bool rolledBack = false;
    std::string failedDevice;
    std::string message;
};

class DrmMultiDeviceCommitCoordinator
{
public:
    static DrmMultiDeviceResult run(const std::vector<DrmDeviceTransaction>& devices,
                                    uint64_t expectedGeneration,
                                    std::function<uint64_t()> currentGeneration);
};

// The largest frame copied through the CPU when the scanout device cannot
// import it: a 4K screen still gets a picture, at the cost of a copy a frame.
inline constexpr int64_t kMaximumCpuCopyPixels = 3840 * 2160;

enum class CrossGpuTransfer {
    Direct,
    DmaBufImport,
    GpuCopy,
    CpuCopy,
    Unavailable
};

struct CrossGpuCapabilities {
    bool sameDevice = false;
    bool dmaBufImport = false;
    bool gpuCopy = false;
    bool cpuMappable = false;
    int64_t framePixels = 0;
};

CrossGpuTransfer chooseCrossGpuTransfer(const CrossGpuCapabilities& capabilities);

struct DrmTileSnapshot {
    uint32_t connectorId = 0;
    std::string groupId;
    int column = 0;
    int row = 0;
    int columns = 1;
    int rows = 1;
    base::Size tileSize;
};

struct DrmTiledMonitor {
    std::string groupId;
    base::Size combinedSize;
    std::vector<uint32_t> connectors;
};

std::optional<std::vector<DrmTiledMonitor>>
groupDrmTiles(const std::vector<DrmTileSnapshot>& tiles);

bool hardwareCloneEligible(const std::string& firstDevice, const std::string& secondDevice,
                           const std::string& firstMode, const std::string& secondMode,
                           uint32_t firstPossibleCrtcs, uint32_t secondPossibleCrtcs);

} // namespace Lingmo::Backend
