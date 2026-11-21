// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/types/display_snapshot.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: validate stable snapshots and exact timing-based
// mode identities without Chromium display or gfx dependencies.

#include "display/display_types.h"

#include <format>
#include <set>
#include <unordered_set>

namespace Lingmo::Display {

bool DisplayMode::isValid() const
{
    return pixelSize.width() > 0 && pixelSize.height() > 0 && refreshMilliHz > 0 &&
        horizontalTotal >= pixelSize.width() && verticalTotal >= pixelSize.height();
}

std::string DisplayMode::stableKey() const
{
    return std::format("{}x{}@{}:{}:{}:{}:{}", pixelSize.width(), pixelSize.height(),
                       refreshMilliHz, pixelClockKHz, horizontalTotal, verticalTotal, flags);
}

bool DisplaySnapshot::isValid() const
{
    if (!sessionId.isValid() || persistentKey.empty() || portKey.empty() ||
        connectorName.empty() || devicePath.empty() || physicalSizeMm.width() < 0 ||
        physicalSizeMm.height() < 0 || modes.empty()) {
        return false;
    }

    std::set<std::string> modeKeys;
    for (const DisplayMode& mode : modes) {
        if (!mode.isValid() || !modeKeys.insert(mode.stableKey()).second)
            return false;
    }
    return (!currentMode || modeKeys.contains(currentMode->stableKey())) &&
        (!nativeMode || modeKeys.contains(nativeMode->stableKey()));
}

bool HardwareSnapshot::isValid() const
{
    if (compositorInstanceId.empty() || seatId.empty() || topologyGeneration == 0)
        return false;

    std::unordered_set<DisplayId> sessionIds;
    std::set<std::string> portKeys;
    for (const DisplaySnapshot& display : displays) {
        if (!display.isValid() || sessionIds.contains(display.sessionId) ||
            portKeys.contains(display.portKey)) {
            return false;
        }
        sessionIds.insert(display.sessionId);
        portKeys.insert(display.portKey);
    }
    return true;
}

} // namespace Lingmo::Display
