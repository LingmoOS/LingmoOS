// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/window_positioner.{h,cc}
//           (GetBoundsAndShowStateForNewWindow, GetReferenceWindow)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the reference window is the application's own most
// recent window, and a new window cascades from it rather than taking its
// bounds, since a Wayland client has chosen its size before it maps. Ash's
// automatic side-by-side rearrangement is not ported.

#ifndef LINGMO_WM_WINDOW_POSITIONER_H_
#define LINGMO_WM_WINDOW_POSITIONER_H_

#include <string>

#include "base/geometry.h"
#include "wm/types.h"

namespace Lingmo::WM {

class Window;
class WindowManager;

namespace WindowPositioner {

// Where a new `window` at `bounds`, centered in its work area, should go:
// offset down and right from its application's most recent window on the same
// output, or from that window's restore bounds while it is maximized. A cascade
// that would run past the work area restarts from its top-left corner.
base::Rect boundsForNewWindow(const Window *window, base::Rect bounds);

// The state a new window of `applicationId` on `output` should be shown in,
// decided before it exists. A window that asked for nothing is maximized when
// its application's most recent window is.
WindowStateType showStateForNewWindow(WindowManager *manager, const base::Rect &output,
                                      const std::string &applicationId, WindowStateType requested);

} // namespace WindowPositioner

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_POSITIONER_H_
