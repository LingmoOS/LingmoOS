// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/types.h"

namespace Lingmo::WM {

const char* toString(WindowStateType type)
{
    switch (type) {
    case WindowStateType::Normal:
        return "normal";
    case WindowStateType::PrimarySnapped:
        return "primary-snapped";
    case WindowStateType::SecondarySnapped:
        return "secondary-snapped";
    case WindowStateType::Maximized:
        return "maximized";
    case WindowStateType::Fullscreen:
        return "fullscreen";
    case WindowStateType::Minimized:
        return "minimized";
    }
    return "unknown";
}

const char* toString(WindowRole role)
{
    switch (role) {
    case WindowRole::Normal:
        return "normal";
    case WindowRole::Desktop:
        return "desktop";
    case WindowRole::Panel:
        return "panel";
    case WindowRole::Dock:
        return "dock";
    case WindowRole::Launcher:
        return "launcher";
    case WindowRole::Notification:
        return "notification";
    case WindowRole::Capture:
        return "capture";
    case WindowRole::Osd:
        return "osd";
    case WindowRole::Sheet:
        return "sheet";
    case WindowRole::Picker:
        return "picker";
    }
    return "unknown";
}

const char* toString(ResizeEdge edge)
{
    switch (edge) {
    case ResizeEdge::None:
        return "none";
    case ResizeEdge::Top:
        return "top";
    case ResizeEdge::Bottom:
        return "bottom";
    case ResizeEdge::Left:
        return "left";
    case ResizeEdge::Right:
        return "right";
    case ResizeEdge::TopLeft:
        return "top-left";
    case ResizeEdge::TopRight:
        return "top-right";
    case ResizeEdge::BottomLeft:
        return "bottom-left";
    case ResizeEdge::BottomRight:
        return "bottom-right";
    }
    return "unknown";
}

} // namespace Lingmo::WM
