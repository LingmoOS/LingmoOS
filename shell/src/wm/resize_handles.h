// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Resize hit regions along a window's rounded outline, shared by the server
// frame and by the handles the compositor keeps for client-decorated windows
// that ask for them (exo's server_side_resize, ash's EasyResizeWindowTargeter).

#ifndef LINGMO_WM_RESIZE_HANDLES_H_
#define LINGMO_WM_RESIZE_HANDLES_H_

#include "base/geometry.h"
#include "render/rounded_shape.h"
#include "wm/types.h"

namespace Lingmo::Decoration {
struct WindowVisualStyle;
}

namespace Lingmo::WM {

class Window;

// chromeos_ui_constants.h: how far the handles reach outside the window and
// into it, and how far along an edge a corner reaches.
constexpr int kResizeOutsideBoundsSize = 6;
constexpr int kResizeOutsideBoundsScaleForTouch = 5;
constexpr int kResizeInsideBoundsSize = 1;
constexpr int kResizeAreaCornerSize = 16;

// The edge `point` grabs on a window outlined by `shape` within `frame`: any
// point outside the outline or less than `inside` into it. A corner reaches
// `corner` along each edge, and past the arc. The caller bounds the outside.
ResizeEdge resizeEdgeFor(const render::RoundedShape& shape, const base::Rect& frame,
                         const base::Point& point, int inside, int corner);

// The edge of a client-decorated window that asked for server resize under
// `location`, in output coordinates, or None.
ResizeEdge serverResizeEdgeAt(const Window& window, const decoration::WindowVisualStyle& style,
                              const base::PointF& location, bool touch);

} // namespace Lingmo::WM

#endif // LINGMO_WM_RESIZE_HANDLES_H_
