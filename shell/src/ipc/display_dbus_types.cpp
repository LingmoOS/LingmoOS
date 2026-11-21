// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/cros_display_config.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: serialize the native display contract through fixed
// session D-Bus structures without Ash or Mojo dependencies.

#include "ipc/display_dbus_types.h"

#include "display/recommended_scale.h"

namespace Lingmo::Display::ipc {

DisplayConfiguration fromWire(const std::string& instanceId, uint64_t topologyGeneration,
                              uint64_t appliedRevision, const std::vector<DisplayRequest>& displays,
                              const std::string& primary, const std::vector<Placement>& placements,
                              const MirrorGroups& mirrorGroups)
{
    DisplayConfiguration result;
    result.base = {instanceId, topologyGeneration, appliedRevision};
    result.primary = DisplayId(primary);
    for (const auto& value : displays) {
        DisplayMode mode;
        mode.pixelSize = {value.mode.width, value.mode.height};
        mode.refreshMilliHz = value.mode.refreshMilliHz;
        mode.pixelClockKHz = value.mode.pixelClockKHz;
        mode.horizontalTotal = value.mode.horizontalTotal;
        mode.verticalTotal = value.mode.verticalTotal;
        mode.flags = value.mode.flags;
        mode.preferred = value.mode.preferred;
        mode.name = value.mode.name;
        const auto scale = compositor::OutputScale::fromNumerator(value.scale120);
        result.displays.push_back({DisplayId(value.id), value.enabled, mode,
                                   scale.value_or(compositor::OutputScale()),
                                   DisplayRotation(value.rotation)});
    }
    for (const auto& value : placements) {
        result.placements.push_back({DisplayId(value.displayId),
                                     DisplayId(value.parentId),
                                     LayoutEdge(value.edge), value.offset});
    }
    for (const auto& group : mirrorGroups) {
        std::vector<DisplayId> ids;
        for (const auto& value : group)
            ids.push_back(DisplayId(value));
        result.mirrorGroups.push_back(ids);
    }
    return result;
}

std::vector<PhysicalDisplay> toWire(const std::vector<DisplaySnapshot>& displays)
{
    std::vector<PhysicalDisplay> result;
    for (const auto& value : displays) {
        PhysicalDisplay item;
        item.sessionId = value.sessionId.value();
        item.persistentKey = value.persistentKey;
        item.portKey = value.portKey;
        item.edidKey = value.edidKey;
        item.connectorName = value.connectorName;
        item.devicePath = value.devicePath;
        item.baseConnectorKey = value.baseConnectorKey;
        item.pathTopology = value.pathTopology;
        item.make = value.make;
        item.model = value.model;
        item.physicalWidthMm = value.physicalSizeMm.width();
        item.physicalHeightMm = value.physicalSizeMm.height();
        item.connectionType = uint32_t(value.connectionType);
        item.capabilities = value.capabilities.toInt();
        item.recommendedScale120 = recommendedScale(value).numerator();
        for (const auto& mode : value.modes) {
            const bool current =
                    value.currentMode && value.currentMode->stableKey() == mode.stableKey();
            item.modes.push_back({mode.pixelSize.width(), mode.pixelSize.height(),
                                  mode.refreshMilliHz, mode.pixelClockKHz, mode.horizontalTotal,
                                  mode.verticalTotal, mode.flags, mode.preferred, current,
                                  mode.name});
        }
        result.push_back(item);
    }
    return result;
}

std::vector<LogicalDisplay> toWire(const std::vector<Lingmo::Display::LogicalDisplay>& displays)
{
    std::vector<LogicalDisplay> result;
    for (const auto& value : displays) {
        std::vector<std::string> destinations;
        for (const auto& id : value.destinations)
            destinations.push_back(id.value());
        result.push_back({value.id.value(), value.waylandName, destinations, value.bounds.x(),
                          value.bounds.y(), value.bounds.width(), value.bounds.height(),
                          value.workArea.x(), value.workArea.y(), value.workArea.width(),
                          value.workArea.height(), value.scale.numerator(),
                          uint32_t(value.rotation), value.primary});
    }
    return result;
}

std::vector<Placement> toWire(const std::vector<DisplayPlacement>& placements)
{
    std::vector<Placement> result;
    for (const auto& value : placements)
        result.push_back(
                {value.display.value(), value.parent.value(), uint32_t(value.edge), value.offset});
    return result;
}

} // namespace Lingmo::Display::ipc
