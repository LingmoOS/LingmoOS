// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/mru_window_tracker.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: one desk, one switchable container. The tracked list
// is ordered by activation; windows never activated fall back to stacking
// order, topmost first, as Ash's container walk does.

#ifndef LINGMO_WM_MRU_WINDOW_TRACKER_H_
#define LINGMO_WM_MRU_WINDOW_TRACKER_H_

#include <vector>

#include "base/signal.h"
#include "wm/activation.h"

namespace Lingmo::WM {

class Window;
class WindowManager;

// Whether `window` is the kind of window that appears in the cycle list: an
// application toplevel, not a popup, not a shell panel, not a hidden window
// and not a transient child, which cycles with its parent.
bool canIncludeWindowInMruList(const Window *window);

class MruWindowTracker : public ActivationObserver
{
public:
    using WindowList = std::vector<Window *>;

    explicit MruWindowTracker(WindowManager *manager);
    MruWindowTracker(const MruWindowTracker &) = delete;
    MruWindowTracker &operator=(const MruWindowTracker &) = delete;
    ~MruWindowTracker() override;

    // Most recently used first, minimized windows included.
    WindowList buildMruWindowList() const;

    // The list alt-tab steps through: as above, minus the windows that cannot
    // be cycled to.
    WindowList buildWindowListForCycling() const;

    // ActivationObserver:
    void onWindowActivated(Window *gainedActive, Window *lostActive) override;

    void onWindowRemoved(Window *window);

    // The window's desk is being removed: it becomes the least recently used,
    // as the user is done with that desk.
    void onWindowMovedOutFromRemovingDesk(Window *window);

private:
    void setActiveWindow(Window *window);

    WindowManager *m_manager = nullptr;
    base::Subscription m_windowRemoved;
    // Least recently used first, so the active window is at the back.
    WindowList m_mruWindows;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_MRU_WINDOW_TRACKER_H_
