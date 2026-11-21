// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_ANCHORED_PLACEMENT_H_
#define LINGMO_WM_ANCHORED_PLACEMENT_H_

#include "base/geometry.h"
#include "compositor/surface.h"

namespace Lingmo::WM {

// Bounds of `size` next to the anchor's rectangle, as an xdg_positioner
// with flipping and sliding on both axes places a popup: flipped to the
// other side when that fits and this side does not, then moved, and shrunk
// when it is larger, into `workArea`.
base::Rect placeAtAnchor(const base::Size &size, const compositor::PlacementAnchor &anchor,
                         const base::Rect &workArea);

} // namespace Lingmo::WM

#endif // LINGMO_WM_ANCHORED_PLACEMENT_H_
