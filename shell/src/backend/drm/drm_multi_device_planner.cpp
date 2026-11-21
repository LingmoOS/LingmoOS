// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_gpu_display_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: device-local tests precede ordered commits, partial
// success rolls back, and unsupported cross-GPU/tile routes stay unavailable.

#include "backend/drm/drm_multi_device_planner.h"

#include <algorithm>
#include <map>
#include <set>

namespace Lingmo::Backend {

DrmMultiDeviceResult
DrmMultiDeviceCommitCoordinator::run(const std::vector<DrmDeviceTransaction> &devices,
                                     uint64_t expectedGeneration,
                                     std::function<uint64_t()> currentGeneration)
{
    if (devices.empty() || !currentGeneration)
        return {false, false, {}, "No DRM devices were supplied"};
    for (const auto &device : devices) {
        if (currentGeneration() != expectedGeneration)
            return {false, false, device.deviceKey,
                    "DRM topology changed during testing"};
        if (!device.test || !device.test())
            return {false, false, device.deviceKey, "A device test commit failed"};
    }
    std::vector<const DrmDeviceTransaction *> committed;
    const auto recover = [&](const std::string &failedDevice) {
        bool restored = !committed.empty();
        for (auto it = committed.crbegin(); it != committed.crend(); ++it) {
            const bool success = (*it)->rollback && (*it)->rollback();
            restored = restored && success;
        }
        return DrmMultiDeviceResult {
                false, restored, failedDevice,
                committed.empty() ? "A device commit was rejected before any changes"
                        : restored ? "A device commit failed and prior devices were restored"
                                   : "A device commit failed and rollback was incomplete"};
    };
    for (const auto &device : devices) {
        if (currentGeneration() != expectedGeneration || !device.commit || !device.commit())
            return recover(device.deviceKey);
        committed.push_back(&device);
        if (currentGeneration() != expectedGeneration)
            return recover(device.deviceKey);
    }
    return {true, false, {}, {}};
}

CrossGpuTransfer chooseCrossGpuTransfer(const CrossGpuCapabilities &capabilities)
{
    if (capabilities.sameDevice)
        return CrossGpuTransfer::Direct;
    if (capabilities.dmaBufImport)
        return CrossGpuTransfer::DmaBufImport;
    if (capabilities.gpuCopy)
        return CrossGpuTransfer::GpuCopy;
    if (capabilities.cpuMappable && capabilities.framePixels > 0
        && capabilities.framePixels <= kMaximumCpuCopyPixels)
        return CrossGpuTransfer::CpuCopy;
    return CrossGpuTransfer::Unavailable;
}

std::optional<std::vector<DrmTiledMonitor>>
groupDrmTiles(const std::vector<DrmTileSnapshot> &tiles)
{
    std::map<std::string, std::vector<DrmTileSnapshot>> groups;
    for (const auto &tile : tiles) {
        if (!tile.groupId.empty())
            groups[tile.groupId].push_back(tile);
    }
    std::vector<DrmTiledMonitor> result;
    for (auto &[key, group] : groups) {
        const auto first = group.front();
        if (first.columns < 1 || first.rows < 1 || first.tileSize.isEmpty()
            || int(group.size()) != first.columns * first.rows)
            return std::nullopt;
        std::set<int> positions;
        for (const auto &tile : group) {
            if (tile.columns != first.columns || tile.rows != first.rows
                || tile.tileSize != first.tileSize || tile.column < 0 || tile.column >= tile.columns
                || tile.row < 0 || tile.row >= tile.rows)
                return std::nullopt;
            positions.insert(tile.row * tile.columns + tile.column);
        }
        if (positions.size() != group.size())
            return std::nullopt;
        std::sort(group.begin(), group.end(), [](const auto &left, const auto &right) {
            return left.row != right.row ? left.row < right.row : left.column < right.column;
        });
        DrmTiledMonitor monitor {
                key,
                base::Size(first.tileSize.width() * first.columns, first.tileSize.height() * first.rows),
                {}};
        for (const auto &tile : group)
            monitor.connectors.push_back(tile.connectorId);
        result.push_back(monitor);
    }
    return result;
}

bool hardwareCloneEligible(const std::string &firstDevice, const std::string &secondDevice,
                           const std::string &firstMode, const std::string &secondMode,
                           uint32_t firstPossibleCrtcs, uint32_t secondPossibleCrtcs)
{
    return !firstDevice.empty() && firstDevice == secondDevice && !firstMode.empty()
            && firstMode == secondMode && (firstPossibleCrtcs & secondPossibleCrtcs) != 0;
}

} // namespace Lingmo::Backend
