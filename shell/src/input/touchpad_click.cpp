// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/touchpad_click.h"

#include <algorithm>
#include <cmath>

namespace Lingmo::Input {

namespace {

// libgestures' "Two Finger Horizontal/Vertical Close Distance Thresh", cut from
// 50 x 45 mm; the recorded diagonal presses need about 45 mm on both axes.
constexpr double kCloseHorizontal = 45.0;
constexpr double kCloseVertical = 45.0;
// "Right Click Start Time Diff Thresh": fingers this close in time are one hand.
constexpr auto kTogether = std::chrono::milliseconds(100);
// "Right Click Second Finger Age Thresh" and "Button Move Distance": a finger
// older than this that has not moved this far is resting.
constexpr auto kRestingAge = std::chrono::milliseconds(500);
constexpr double kRestingTravel = 10.0;
// "Bottom Zone Size", where a resting finger is a thumb.
constexpr double kThumbZone = 10.0;
// libinput's side palm edges: 8% of the width, at most 8 mm, on pads at least
// 70 mm wide. It ignores a finger that came down there, and so does this.
constexpr double kPalmEdgeMinWidth = 70.0;
constexpr double kPalmEdge = 8.0;

bool inPalmEdge(const TouchpadContact &contact, double width)
{
    if (width < kPalmEdgeMinWidth)
        return false;
    const double edge = std::min(kPalmEdge, width * 0.08);
    return contact.origin.x() < edge || contact.origin.x() > width - edge;
}

} // namespace

bool isTwoFingerClick(const TouchpadContact &first, const TouchpadContact &second,
                      Timestamp pressTime, const base::SizeF &padSize)
{
    if (inPalmEdge(first, padSize.width()) || inPalmEdge(second, padSize.width()))
        return false;

    const double dx = std::abs(first.position.x() - second.position.x());
    const double dy = std::abs(first.position.y() - second.position.y());
    if (std::pow(dx / kCloseHorizontal, 2) + std::pow(dy / kCloseVertical, 2) >= 1.0)
        return false;

    const bool firstOlder = first.downTime <= second.downTime;
    const TouchpadContact &older = firstOlder ? first : second;
    const TouchpadContact &newer = firstOlder ? second : first;
    if (newer.downTime - older.downTime < kTogether)
        return true;

    const auto inThumbZone = [&padSize](const TouchpadContact &contact) {
        return contact.position.y() > padSize.height() - kThumbZone;
    };
    const base::PointF travel = older.position - older.origin;
    const bool olderResting = pressTime - older.downTime >= kRestingAge
            && std::hypot(travel.x(), travel.y()) < kRestingTravel;
    if (olderResting && inThumbZone(older))
        return false;
    // Stacked vertically with one at the bottom edge: a thumb under a finger.
    if (dx < dy && (inThumbZone(first) || inThumbZone(second)))
        return false;
    return true;
}

} // namespace Lingmo::Input
