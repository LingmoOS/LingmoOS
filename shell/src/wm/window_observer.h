// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_WINDOW_OBSERVER_H_
#define LINGMO_WM_WINDOW_OBSERVER_H_

#include "base/geometry.h"
#include "wm/types.h"

namespace Lingmo::WM {

class Window;

// What ShellSurface listens to in order to emit xdg configure events. Nothing here knows about the protocol.
class WindowObserver
{
public:
    virtual ~WindowObserver() = default;

    virtual void onWindowBoundsChanged(Window* window, const base::Rect& oldBounds) { }
    virtual void onWindowStateTypeChanged(Window* window, WindowStateType oldType) { }
    virtual void onWindowActivationChanged(Window* window, bool active) { }
    virtual void onWindowResizingChanged(Window* window, bool resizing) { }
    // The window is being moved or resized interactively. Wider than
    // onWindowResizingChanged, which is only the resize half of it because it
    // is what the xdg_toplevel resizing state means.
    virtual void onWindowDragStateChanged(Window* window, bool dragged) { }
    virtual void onWindowTransientParentChanged(Window* window, Window* oldParent) { }
    virtual void onWindowModalChanged(Window* window) { }
    // Window::isResponsive flipped.
    virtual void onWindowResponsiveChanged(Window* window) { }

    // Window::demandsAttention flipped: the application asked for the user's
    // attention, the way an X11 client's urgency hint does.
    virtual void onWindowDemandsAttentionChanged(Window* window) { }

    virtual void onWindowApplicationIdChanged(Window* window) { }

    virtual void onWindowTitleChanged(Window* window) { }

    // The client committed, or its pixels moved inside the window: whatever
    // the window actually has on screen - Window::presentedBounds - changed.
    virtual void onWindowContentChanged(Window* window) { }

    // Minimize, restore, hide and unhide; not a change to the scene node's
    // own visibility.
    virtual void onWindowVisibilityChanged(Window* window, bool visible) { }

    // Window::isHidden flipped.
    virtual void onWindowHiddenChanged(Window* window) { }
    // Window::isDeskHidden flipped: the window's desk was activated or left,
    // or the window moved to another desk.
    virtual void onWindowDeskHiddenChanged(Window* window) { }

    // Window::minimizedSnapshot was taken or dropped.
    virtual void onWindowMinimizedSnapshotChanged(Window* window) { }

    // The minimize animation is holding the window on its dock slot, waiting
    // for Window::endMinimizeHandoff.
    virtual void onWindowMinimizeLanded(Window* window) { }

    // How far the client's buffer reaches outside its declared window geometry
    // changed, so whether it owns its exterior decoration may have changed too.
    virtual void onWindowClientDecorationChanged(Window* window) { }

    // The window gained or lost its server-side frame, so the client area
    // inside the same outer bounds changed size.
    virtual void onWindowFrameChanged(Window* window) { }

    // Called at the top of the window's destructor.
    virtual void onWindowDestroying(Window* window) { }
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_OBSERVER_H_
