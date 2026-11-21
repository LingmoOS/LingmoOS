// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/display_layout.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: IDs and geometry are plain value types, and validation
// returns a normalized immutable result rather than mutating display objects.

#pragma once

#include <map>
#include <vector>

#include "base/geometry.h"

#include "display/display_types.h"

namespace Lingmo::Display {

class DisplayLayout
{
public:
    static constexpr int maximumCoordinate = 100000;
    static constexpr int preferredMinimumSharedEdge = 32;
    static constexpr int maximumDisplayCount = 64;

    struct Result {
        bool succeeded = false;
        DisplayError error;
        std::map<DisplayId, base::Rect> bounds;
        std::vector<DisplayPlacement> placements;
    };

    static Result resolve(const DisplayId& primary,
                          const std::map<DisplayId, base::Size>& displaySizes,
                          const std::vector<DisplayPlacement>& placements,
                          int minimumSharedEdge = preferredMinimumSharedEdge);
};

} // namespace Lingmo::Display
