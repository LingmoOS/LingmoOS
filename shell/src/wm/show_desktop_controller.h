// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_SHOW_DESKTOP_CONTROLLER_H_
#define LINGMO_WM_SHOW_DESKTOP_CONTROLLER_H_

#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "wm/activation.h"
#include "wm/desks/desks_controller.h"
#include "wm/window_observer.h"

namespace Lingmo::WM {

class Window;
class WindowManager;

// Slides the application windows of the active desk aside, toward the left or
// right edge of their output, until only a sliver of each is left on screen,
// and back. Only what is drawn moves (Window::setPresentationOffset): bounds,
// clients and popups stay where they are, and input follows the drawn window,
// so a sliver can be clicked. Activating any application window brings them
// all back; the desktop takes the keyboard meanwhile.
class ShowDesktopController : public ActivationObserver,
                              public DesksObserver,
                              public WindowObserver
{
public:
    static constexpr double kSlideDurationMs = 250;
    // What stays on screen of a window moved aside, in logical pixels.
    static constexpr int kSliverWidth = 12;

    explicit ShowDesktopController(WindowManager *manager);
    ShowDesktopController(const ShowDesktopController &) = delete;
    ShowDesktopController &operator=(const ShowDesktopController &) = delete;
    ~ShowDesktopController() override;

    bool isActive() const { return m_active; }
    bool isMovedAside(const Window *window) const;
    // Between 0, every window in place, and 1, all of them aside.
    double progress() const { return m_progress; }

    bool beginGesture();
    void updateGesture(double progress);
    void endGesture(bool cancelled);
    void toggle();
    void enter();
    // Slides the windows back. `activate`, one of them, becomes active.
    void exit(Window *activate = nullptr);
    // Puts every window back at once, for the overview and desk switches.
    void exitImmediately();

    // Steps the slide; driven by the compositor's frame clock.
    void advance(double frameTimeMs);

    // --- ActivationObserver ---
    void onWindowActivated(Window *gainedActive, Window *lostActive) override;

    // --- DesksObserver ---
    void onDeskActivationChanged(const Desk *activated, const Desk *deactivated) override;
    void onDeskSwitchAnimationLaunching() override;

    // --- WindowObserver ---
    void onWindowBoundsChanged(Window *window, const base::Rect &oldBounds) override;
    void onWindowVisibilityChanged(Window *window, bool visible) override;
    void onWindowDestroying(Window *window) override;

private:
    // Where `window` is drawn from its bounds once fully aside.
    base::Point awayOffset(const Window *window) const;
    void slideTo(double target);
    void present();
    void release(Window *window);
    void releaseAll();
    void requestFrame();

    WindowManager *m_manager = nullptr;
    bool m_active = false;
    bool m_tracking = false;
    double m_gestureStart = 0;
    uint64_t m_gestureActiveWindow = 0;
    // The windows moved aside, bottom to top.
    std::vector<Window *> m_windows;
    double m_progress = 0;
    double m_slideFrom = 0;
    double m_slideTo = 0;
    double m_slideStarted = -1;
    bool m_sliding = false;
    base::Subscription m_frameStarted;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_SHOW_DESKTOP_CONTROLLER_H_
