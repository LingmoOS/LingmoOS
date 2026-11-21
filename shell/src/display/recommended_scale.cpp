// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/display_change_observer.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: external monitors get a density table of their own,
// where Chromium leaves them at 1, and results are Settings' scale steps.

#include "display/recommended_scale.h"

#include <array>

namespace Lingmo::Display {
namespace {

struct Threshold {
    double dpi;
    uint32_t scale120;
};

// Chromium's kThresholdTableForLcdInternal, each factor rounded to the
// nearest step: 2.666 and 2.4 to 2.5, 1.777 to 1.75, 1.6 to 1.5.
constexpr std::array<Threshold, 6> kInternal {{
        {270, 300},
        {230, 240},
        {220, 210},
        {180, 180},
        {150, 150},
        {0, 120},
}};

// A desktop monitor sits farther away than a laptop panel: about 110 dpi per
// unit of scale, so a 27" 4K runs at 150% and a 27" 5K at 200%.
constexpr std::array<Threshold, 5> kExternal {{
        {200, 240},
        {175, 210},
        {150, 180},
        {125, 150},
        {0, 120},
}};

// ui/display/util/display_util.cc: IsDisplaySizeValid. Projectors and some
// TVs report an aspect ratio or nothing in place of a size.
bool isPhysicalSizeValid(const base::Size &mm)
{
    if (mm.width() <= 40 || mm.height() <= 30)
        return false;
    return mm != base::Size(50, 40) && mm != base::Size(160, 90) && mm != base::Size(160, 100);
}

} // namespace

compositor::OutputScale recommendedScale(const DisplaySnapshot &display)
{
    const auto mode = display.nativeMode ? display.nativeMode : display.currentMode;
    if (!mode || display.connectionType == DisplayConnectionType::Virtual
        || !isPhysicalSizeValid(display.physicalSizeMm))
        return {};
    constexpr double kInchInMm = 25.4;
    const double dpi = kInchInMm * mode->pixelSize.width() / display.physicalSizeMm.width();
    const auto pick = [dpi](const auto &table) {
        for (const auto &threshold : table) {
            if (dpi >= threshold.dpi)
                return *compositor::OutputScale::fromNumerator(threshold.scale120);
        }
        return compositor::OutputScale();
    };
    return display.connectionType == DisplayConnectionType::Internal ? pick(kInternal)
                                                                      : pick(kExternal);
}

} // namespace Lingmo::Display
