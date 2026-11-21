// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/stacking.h"

#include "wm/window.h"

namespace Lingmo::WM {

StackingLayer layerFor(WindowRole role, PanelBehavior behavior)
{
    switch (role) {
    case WindowRole::Desktop:
        return StackingLayer::Desktop;
    case WindowRole::Normal:
        return StackingLayer::Normal;
    case WindowRole::Launcher:
        return StackingLayer::Launcher;
    case WindowRole::Panel:
        return behavior == PanelBehavior::WindowsCanCover ? StackingLayer::Normal
                                                          : StackingLayer::Panel;
    case WindowRole::Dock:
        return behavior == PanelBehavior::WindowsCanCover ? StackingLayer::Normal
                                                          : StackingLayer::Dock;
    case WindowRole::Notification:
        return StackingLayer::Notification;
    case WindowRole::Osd:
        return StackingLayer::Osd;
    case WindowRole::Picker:
        return StackingLayer::Picker;
    case WindowRole::Capture:
        return StackingLayer::Capture;
    case WindowRole::Sheet:
        return StackingLayer::Normal;
    }
    return StackingLayer::Normal;
}

StackingLayer layerFor(const Window *window)
{
    if (!window)
        return StackingLayer::Normal;
    if (window->role() == WindowRole::Sheet && window->transientParent())
        return layerFor(window->transientParent());
    if (window->isPopup() && window->transientParent()) {
        const StackingLayer parentLayer = layerFor(window->transientParent());
        return parentLayer == StackingLayer::Desktop ? StackingLayer::DesktopPopup : parentLayer;
    }
    if (window->isSystemDialog() && window->role() == WindowRole::Normal)
        return StackingLayer::SystemDialog;
    if (window->isAlwaysOnTop() && window->role() == WindowRole::Normal)
        return StackingLayer::AlwaysOnTop;
    return layerFor(window->role(), window->panelBehavior());
}

} // namespace Lingmo::WM
