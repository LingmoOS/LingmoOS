// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/configure_displays_task.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: validate revisioned display requests before DRM
// planning and return structured errors plus normalized logical bounds.

#pragma once

#include <map>

#include "display/display_layout.h"
#include "display/display_types.h"

namespace Lingmo::Display {

class DisplayConfigurationValidator
{
public:
    struct Result {
        bool succeeded = false;
        DisplayError error;
        DisplayConfiguration normalized;
        std::map<DisplayId, base::Rect> logicalBounds;
    };

    static Result validateExtended(const HardwareSnapshot& hardware, uint64_t appliedRevision,
                                   const DisplayConfiguration& requested);
};

} // namespace Lingmo::Display
