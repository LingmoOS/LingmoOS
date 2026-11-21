// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "input/cursor_shape.h"
#include "wm/types.h"

namespace Lingmo::WM {

inline input::CursorShape resizeCursor(ResizeEdge edge)
{
    using input::CursorShape;
    switch (edge) {
    case ResizeEdge::Left:
    case ResizeEdge::Right:
        return CursorShape::EwResize;
    case ResizeEdge::Top:
    case ResizeEdge::Bottom:
        return CursorShape::NsResize;
    case ResizeEdge::TopLeft:
    case ResizeEdge::BottomRight:
        return CursorShape::NwseResize;
    case ResizeEdge::TopRight:
    case ResizeEdge::BottomLeft:
        return CursorShape::NeswResize;
    default:
        return CursorShape::Default;
    }
}

} // namespace Lingmo::WM
