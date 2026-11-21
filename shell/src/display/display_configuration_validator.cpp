// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/configure_displays_task.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: exact-mode and capability validation precede the
// backend-independent layout resolver without running a DRM transaction.

#include "display/display_configuration_validator.h"

#include <algorithm>
#include <set>



namespace Lingmo::Display {

namespace {

DisplayConfigurationValidator::Result failure(DisplayErrorCode code, std::string message,
                                               std::vector<DisplayId> displays = {})
{
    DisplayConfigurationValidator::Result result;
    result.error = {code, std::move(message), std::move(displays)};
    return result;
}

const DisplaySnapshot* findDisplay(const HardwareSnapshot& hardware, const DisplayId& id)
{
    for (const DisplaySnapshot& display : hardware.displays) {
        if (display.sessionId == id)
            return &display;
    }
    return nullptr;
}

bool containsMode(const DisplaySnapshot& display, const DisplayMode& requested)
{
    for (const DisplayMode& mode : display.modes) {
        if (mode.stableKey() == requested.stableKey())
            return true;
    }
    return false;
}

} // namespace

DisplayConfigurationValidator::Result DisplayConfigurationValidator::validateExtended(
    const HardwareSnapshot& hardware, uint64_t appliedRevision,
    const DisplayConfiguration& requested)
{
    if (!hardware.isValid())
        return failure(DisplayErrorCode::Unavailable,
                       "The hardware snapshot is unavailable");
    if (requested.base.compositorInstanceId != hardware.compositorInstanceId ||
        requested.base.topologyGeneration != hardware.topologyGeneration ||
        requested.base.appliedRevision != appliedRevision) {
        return failure(DisplayErrorCode::StaleConfiguration,
                       "The configuration base is stale");
    }
    if (requested.displays.size() > DisplayLayout::maximumDisplayCount)
        return failure(DisplayErrorCode::InvalidLayout,
                       "The configuration contains too many displays");

    std::set<DisplayId> seen;
    std::set<DisplayId> enabled;
    std::map<DisplayId, base::Size> logicalSizes;
    for (const DisplayRequest& displayRequest : requested.displays) {
        const DisplaySnapshot* display = findDisplay(hardware, displayRequest.display);
        if (!display || seen.contains(displayRequest.display))
            return failure(DisplayErrorCode::UnknownOutput,
                           "The configuration contains an unknown or duplicate display",
                           {displayRequest.display});
        seen.insert(displayRequest.display);
        if (!displayRequest.enabled)
            continue;
        enabled.insert(displayRequest.display);

        if (!displayRequest.mode.isValid() || !containsMode(*display, displayRequest.mode))
            return failure(DisplayErrorCode::UnsupportedMode,
                           "The requested mode is unavailable",
                           {displayRequest.display});
        if (displayRequest.scale.numerator() != compositor::OutputScale::denominator &&
            !display->capabilities.testFlag(DisplayCapability::Scale)) {
            return failure(DisplayErrorCode::UnsupportedProperty,
                           "The display does not support the requested scale",
                           {displayRequest.display});
        }
        if (!isValid(displayRequest.rotation))
            return failure(DisplayErrorCode::UnsupportedProperty,
                           "The requested rotation is unknown",
                           {displayRequest.display});
        if (displayRequest.rotation != DisplayRotation::Normal &&
            !display->capabilities.testFlag(DisplayCapability::Rotation)) {
            return failure(DisplayErrorCode::UnsupportedProperty,
                           "The display does not support rotation",
                           {displayRequest.display});
        }

        base::Size rotated = displayRequest.mode.pixelSize;
        if (displayRequest.rotation == DisplayRotation::Rotate90 ||
            displayRequest.rotation == DisplayRotation::Rotate270) {
            rotated = rotated.transposed();
        }
        logicalSizes.insert_or_assign(
                displayRequest.display,
                displayRequest.scale.logicalSize(rotated));
    }

    if (enabled.empty())
        return failure(DisplayErrorCode::NoEnabledOutput,
                       "At least one display must be enabled");
    if (!enabled.contains(requested.primary))
        return failure(DisplayErrorCode::InvalidLayout,
                       "The primary display must be enabled", {requested.primary});

    std::set<DisplayId> mirrorDestinations;
    std::set<DisplayId> mirrorMembers;
    for (const auto& group : requested.mirrorGroups) {
        if (group.size() < 2 || !enabled.contains(group.front()))
            return failure(DisplayErrorCode::InvalidLayout,
                           "A mirror group needs an enabled source and destination",
                           group);
        for (size_t index = 0; index < group.size(); ++index) {
            const DisplayId id = group[index];
            const DisplaySnapshot* display = findDisplay(hardware, id);
            if (!display || !enabled.contains(id) || mirrorMembers.contains(id) ||
                !display->capabilities.testFlag(DisplayCapability::SoftwareMirror)) {
                return failure(DisplayErrorCode::UnsupportedProperty,
                               "A mirror destination is unavailable or unsupported",
                               {id});
            }
            mirrorMembers.insert(id);
            if (index > 0) {
                mirrorDestinations.insert(id);
                logicalSizes.erase(id);
            }
        }
    }
    if (mirrorDestinations.contains(requested.primary))
        return failure(DisplayErrorCode::InvalidLayout,
                       "The primary display must be a mirror source",
                       {requested.primary});

    for (const DisplayPlacement& placement : requested.placements) {
        if (!isValid(placement.edge))
            return failure(DisplayErrorCode::InvalidLayout,
                           "A placement names an unknown edge",
                           {placement.display});
    }

    const DisplayLayout::Result layout =
        DisplayLayout::resolve(requested.primary, logicalSizes, requested.placements);
    if (!layout.succeeded) {
        Result result;
        result.error = layout.error;
        return result;
    }

    Result result;
    result.succeeded = true;
    result.normalized = requested;
    result.normalized.placements = layout.placements;
    result.logicalBounds = layout.bounds;
    return result;
}

} // namespace Lingmo::Display
