// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/window_positioner.{h,cc}
//           (GetBoundsAndShowStateForNewWindow, GetReferenceWindow)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "wm/window_positioner.h"

#include "wm/activation.h"
#include "wm/placement.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::WM::WindowPositioner {

namespace {

constexpr int kWindowOffset = 32;

// The most recent shown top-level window of `applicationId` on `output`.
// Activation raises, so topmost first is most recently used first - but only
// within one stacking layer: an always-on-top window stays topmost regardless.
const Window *referenceWindow(WindowManager *manager, const base::Rect &output,
                              const std::string &applicationId, bool alwaysOnTop,
                              const Window *exclude)
{
    if (applicationId.empty())
        return nullptr;
    const std::vector<Window *> &windows = manager->windows();
    for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
        const Window *window = *it;
        if (window != exclude && window->applicationId() == applicationId && !window->isPopup()
            && !window->isShellWindow() && !window->transientParent() && window->isVisible()
            && !window->state()->isMinimized() && window->isAlwaysOnTop() == alwaysOnTop
            && manager->outputBoundsFor(window) == output) {
            return window;
        }
    }
    return nullptr;
}

} // namespace

base::Rect boundsForNewWindow(const Window *window, base::Rect bounds)
{
    WindowManager *manager = window->manager();
    if (!window->maximumSize().isEmpty() && window->minimumSize() == window->maximumSize()) {
        const Window *reference = manager->activation()->activeWindow();
        if (reference && reference != window && !window->applicationId().empty()
            && reference->applicationId() == window->applicationId() && !reference->isPopup()
            && !reference->isShellWindow() && !reference->transientParent()
            && reference->isVisible() && !reference->state()->isMinimized()) {
            return Placement().placeTransient(bounds.size(), reference->bounds(),
                                              reference->frameInsets().top,
                                              manager->workAreaFor(reference));
        }
        return bounds;
    }
    const Window *top = referenceWindow(manager, manager->outputBoundsFor(window),
                                        window->applicationId(), window->isAlwaysOnTop(), window);
    if (!top)
        return bounds;

    const WindowState *topState = top->state();
    const base::Rect topBounds = !topState->isNormal() && topState->hasRestoreBounds()
            ? topState->restoreBounds()
            : top->bounds();
    bounds.moveTopLeft(topBounds.topLeft() + base::Point(kWindowOffset, kWindowOffset));

    const base::Rect workArea = manager->workAreaFor(window);
    if (bounds.x() + bounds.width() > workArea.x() + workArea.width()
        || bounds.y() + bounds.height() > workArea.y() + workArea.height()) {
        bounds.moveTopLeft(workArea.topLeft());
    }
    adjustBoundsToFit(workArea, &bounds);
    return bounds;
}

WindowStateType showStateForNewWindow(WindowManager *manager, const base::Rect &output,
                                      const std::string &applicationId, WindowStateType requested)
{
    if (requested != WindowStateType::Normal)
        return requested;
    const Window *top = referenceWindow(manager, output, applicationId, false, nullptr);
    return top && top->state()->isMaximized() ? WindowStateType::Maximized
                                              : WindowStateType::Normal;
}

} // namespace Lingmo::WM::WindowPositioner
