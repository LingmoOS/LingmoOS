// Copyright 2019 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/wm_shadow_controller_delegate.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "wm/wm_shadow_controller_delegate.h"

#include "compositor/surface.h"
#include "wm/window.h"

namespace Lingmo::WM {

bool WmShadowControllerDelegate::shouldShowShadowForWindow(const Window *window)
{
    if (!window || !window->isVisible() || window->bounds().isEmpty())
        return false;

    // The client draws its own shadow, or none.
    if (const auto *surface = window->rootSurface(); surface && !surface->castsShadow())
        return false;

    // Popups and shell surfaces retain their client-owned exterior policy.
    if ((window->isPopup() || window->role() != WindowRole::Normal) && !window->isDecorated()
        && window->hasClientExteriorDecoration())
        return false;

    // A popup whose geometry is its whole surface but whose input region leaves
    // a band on every edge draws its own shadow in that band.
    if (window->isPopup() && !window->isDecorated() && window->hasClientInputMargin())
        return false;

    if (window->isPopup())
        return true;

    switch (window->role()) {
    case WindowRole::Normal:
        break;
    case WindowRole::Dock:
    case WindowRole::Notification:
    case WindowRole::Osd:
    case WindowRole::Sheet:
    case WindowRole::Picker:
        return true;
    case WindowRole::Desktop:
    case WindowRole::Panel:
    case WindowRole::Launcher:
    case WindowRole::Capture:
        return false;
    }

    // Maximized and fullscreen fill the work area or the output and own no
    // exterior space to cast into. A window being dragged out of one of those
    // owns its shadow again as soon as its content has caught up; a shadow
    // around the full-size content it still has would be a band across the
    // bottom of the work area, over the dock.
    return isRoundedStateType(window->decoratedStateType());
}

ShadowElevation WmShadowControllerDelegate::elevationForWindow(const Window *window)
{
    if (window->isPopup())
        return ShadowElevation::MenuOrTooltip;
    if (window->role() == WindowRole::Dock)
        return ShadowElevation::Dock;
    if (window->role() == WindowRole::Notification || window->role() == WindowRole::Osd
        || window->role() == WindowRole::Picker)
        return ShadowElevation::Popup;
    // A window being moved or resized reads as lifted off the desktop, so it
    // keeps the active elevation even if something else holds the keyboard.
    if (window->isDragged())
        return ShadowElevation::Active;
    return window->isActive() ? ShadowElevation::Active : ShadowElevation::Inactive;
}

} // namespace Lingmo::WM
