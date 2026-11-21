// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on chromium/src/ash/wm/wm_event.h (revision
// 7acd40ddefff49c9d36144999aafdbb674dbd07a). The shape - a request object
// handed to WindowState, which decides what actually happens - is Ash's; the
// event set is reduced to the states Lingmo v0.1 has.

#ifndef LINGMO_WM_WM_EVENT_H_
#define LINGMO_WM_WM_EVENT_H_

#include "base/geometry.h"

namespace Lingmo::WM {

enum class WMEventType {
    // Requests to enter the corresponding state. The request may not be
    // granted: a non-resizable window will not maximize, for instance.
    Normal,
    Maximize,
    Minimize,
    Fullscreen,
    // Half the work area, on its left or its right edge. Refused for a window
    // whose minimum size does not fit there.
    SnapPrimary,
    SnapSecondary,

    // Return to the state the window was in before the current one.
    Restore,

    // Request a new bounds. Honoured only in the normal state; a maximized or
    // fullscreen window's bounds are the work area's.
    SetBounds,

    // Compound: the resulting state depends on the current one.
    ToggleMaximize,
    ToggleFullscreen,

    // The work area or output geometry changed. The state type does not change;
    // the bounds are recomputed for it.
    WorkAreaChanged,
};

class WMEvent
{
public:
    explicit WMEvent(WMEventType type);
    WMEvent(const WMEvent&) = delete;
    WMEvent& operator=(const WMEvent&) = delete;
    virtual ~WMEvent();

    WMEventType type() const { return m_type; }

private:
    WMEventType m_type;
};

class SetBoundsWMEvent : public WMEvent
{
public:
    explicit SetBoundsWMEvent(const base::Rect& bounds);
    ~SetBoundsWMEvent() override;

    const base::Rect& requestedBounds() const { return m_requestedBounds; }

private:
    base::Rect m_requestedBounds;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WM_EVENT_H_
