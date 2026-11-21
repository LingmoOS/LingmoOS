// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/window_cycle/window_cycle_controller.h
//           window_cycle_list.h and window_cycle_event_filter.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: the three classes fold into one, and what is cycled
// through is applications, not windows: one entry per application, in the
// order its windows were last used. As in Ash, stepping only moves the
// selection; the chosen application comes forward, all of its windows
// together, once the modifier is released. The list is a WindowCycleView.

#ifndef LINGMO_WM_WINDOW_CYCLE_CONTROLLER_H_
#define LINGMO_WM_WINDOW_CYCLE_CONTROLLER_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>


#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"

namespace Lingmo::WM {

class MruWindowTracker;
class Window;
class WindowManager;

// What shows the list while cycling. Upstream: WindowCycleView.
class WindowCycleView
{
public:
    virtual ~WindowCycleView() = default;

    // `entries` holds each application's most recently used window, centred
    // on the output named `output`. A list that changes while shown is shown
    // again.
    virtual void showWindowCycle(const std::vector<Window *> &entries, int selected,
                                 const std::string &output) = 0;
    virtual void selectWindowCycleEntry(int index) = 0;
    virtual void hideWindowCycle() = 0;

    // Where the list is on screen, in global logical coordinates, and which
    // entry is at `location`, or -1.
    virtual bool windowCycleViewContains(const base::PointF &location) const = 0;
    virtual int windowCycleEntryAt(const base::PointF &location) const = 0;
};

class WindowCycleController
{
public:
    enum class Direction {
        Forward,
        Backward,
    };

    // Upstream: kShowDelayDuration. A quick press switches without the list.
    static constexpr int kShowDelayMs = 150;
    // Upstream: kMouseMovementThreshold. The list may appear under a pointer
    // resting there; it selects only once the pointer moves.
    static constexpr double kMouseMovementThreshold = 5;

    WindowCycleController(WindowManager *manager, MruWindowTracker *mruTracker);
    WindowCycleController(const WindowCycleController &) = delete;
    WindowCycleController &operator=(const WindowCycleController &) = delete;
    ~WindowCycleController();

    // Whether cycling may start or continue: the session is not blocked and
    // no system-modal dialog holds the desktop.
    bool canCycle() const;

    // Starts cycling if needed, then steps once.
    void handleCycleWindow(Direction direction);

    bool isCycling() const { return m_cycling; }

    // Switches to the selected application.
    void completeCycling();

    // Ends cycling where it started.
    void cancelCycling();

    void setView(WindowCycleView *view);

    // Each application's most recently used window, in list order.
    std::vector<Window *> entries() const;
    int selectedIndex() const { return m_currentIndex; }
    bool isViewShown() const { return m_viewShown; }

    // Shows the list now rather than after kShowDelayMs.
    void showViewNowForTesting();

    // Key and pointer events seen while cycling. Ash's WindowCycleEventFilter:
    // Escape cancels, and every other key is held back from the focused window
    // until cycling ends; the arrow keys and ` step, Q quits and H hides the
    // selected application. Releasing the modifiers that started the cycle
    // completes it; the AcceleratorFilter knows which those are. Returns true
    // when the event was consumed.
    bool onKey(uint32_t keysym, bool pressed);

    // Hovering an entry selects it.
    void onPointerMotion(const base::PointF &location);
    // While the list is shown a press on an entry switches to it, and one
    // anywhere else does nothing; both are consumed. Before it shows, a press
    // means the user moved on: the selection is kept and the press lands.
    bool onPointerButtonPressed(const base::PointF &location);

private:
    void startCycling();
    void step(Direction direction, bool startingCycle);
    void stopCycling(bool accept);
    void switchToApplication(const std::vector<Window *> &windows);
    void showView();
    void hideView();
    std::string outputForView() const;
    void onWindowRemoved(Window *window);

    WindowManager *m_manager = nullptr;
    MruWindowTracker *m_mruTracker = nullptr;
    WindowCycleView *m_view = nullptr;
    base::Subscription m_windowRemoved;
    base::Timer m_showTimer;
    bool m_cycling = false;
    // Each application's cyclable windows, most recently used first.
    std::vector<std::vector<Window *>> m_applications;
    int m_currentIndex = 0;
    bool m_viewShown = false;
    // Where the pointer was when cycling started, until it moves away.
    std::optional<base::PointF> m_pointerOrigin;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_CYCLE_CONTROLLER_H_
