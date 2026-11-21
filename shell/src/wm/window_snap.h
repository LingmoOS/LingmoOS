// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on chromium/src/ash/wm/workspace/workspace_window_resizer.cc
// (revision 7acd40ddefff49c9d36144999aafdbb674dbd07a): GetNonSnapWorkArea,
// GetActiveDragAreaForSnapAndMaximize and GetSnapType, plus the snapped bounds
// ash::GetSnappedWindowBounds computes.
//
// Only Ash's landscape layout is ported: left and right snap to half the work
// area, the top maximizes. Ash's portrait orientation, where the top and the
// bottom snap instead, has no Lingmo equivalent and is left out. The snap
// ratio is always a half; Ash carries a per-window ratio for its resizable
// split view.

#ifndef LINGMO_WM_WINDOW_SNAP_H_
#define LINGMO_WM_WINDOW_SNAP_H_

#include "base/geometry.h"
#include "wm/types.h"

namespace Lingmo::WM {

// What the edge the pointer is dragging against asks for.
enum class SnapType {
    None,
    Primary, // Left half.
    Secondary, // Right half.
    Maximize,
};

// The area a drag can move through without asking for anything. Outside it, on
// one of the screen edges, is where a snap or a maximize is triggered. The
// tolerance is only added on the edges where the work area reaches the output:
// past a dock the pointer has to cross the dock first.
base::Rect nonSnapArea(const base::Rect &workArea, const base::Rect &outputBounds);

// `location` is in output coordinates. Returns what a drag ending there would
// ask for, before any window constraint is taken into account.
SnapType snapTypeAt(const base::PointF &location, const base::Rect &workArea, const base::Rect &outputBounds);

// Empty for SnapType::None.
base::Rect snapBounds(SnapType type, const base::Rect &workArea);

WindowStateType snapStateType(SnapType type);

// A window narrower than half the work area cannot be snapped into it.
bool canSnapSize(const base::Size &minimumOuterSize, const base::Rect &workArea);

// Whether `bounds` still look snapped: full work-area height, against one of
// its vertical edges. Ash's WorkspaceWindowResizer::AreBoundsValidSnappedBounds.
bool areBoundsValidSnappedBounds(WindowStateType type, const base::Rect &bounds, const base::Rect &workArea);

const char *toString(SnapType type);

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_SNAP_H_
