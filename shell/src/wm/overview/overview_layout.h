// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_OVERVIEW_OVERVIEW_LAYOUT_H_
#define LINGMO_WM_OVERVIEW_OVERVIEW_LAYOUT_H_

#include "base/geometry.h"

#include <vector>

namespace Lingmo::WM::Overview {

// The overview's arrangement of one output's windows: in rows, as few as give
// the largest windows, filled top to bottom by the windows' vertical centres
// and each ordered left to right by their horizontal ones, so the windows keep
// roughly to where they are. A smaller window takes more room in its row than
// its size, by the fourth root of how much smaller its area is than the
// largest one's. The
// rows are scaled together to fit `area`, never enlarging a window, and
// centred; each window then grows on its own into the room left around it,
// keeping its shape and never past its own size. At least `gap` logical pixels
// stay between any two windows. Rectangles land on whole pixels of an output
// at `deviceScale`.
//
// The same windows in the same order always give the same arrangement.
std::vector<base::RectF> spreadLayout(const std::vector<base::Rect> &windows, const base::Rect &area, int gap,
                                 double deviceScale);

} // namespace Lingmo::WM::Overview

#endif // LINGMO_WM_OVERVIEW_OVERVIEW_LAYOUT_H_
