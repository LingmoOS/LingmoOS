// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "shell/shell.h"

namespace Lingmo::ui {
class Window;
}

namespace Lingmo::Shell {

class Dock;
class Launcher;
class LauncherView;
class LauncherWallpaper;
class Wallpaper;
class ShellWindow;

// The launcher window: a window of
// the Launcher role covering the primary output, status bar included, with
// room left for the dock; shown, activated and hidden by the Launcher it is
// given, and hidden again when it loses the activation to anything but its
// own menu, a drag or the capture tools. The compositor animates its
// opening and closing by its role.
class LauncherWindow : public Component
{
public:
    // `dock` gives the room to leave; it may be null.
    // `wallpaper` is the desktop's, whose decode the launcher shrinks.
    LauncherWindow(Shell *shell, std::unique_ptr<Launcher> launcher, Dock *dock, Wallpaper *wallpaper);
    ~LauncherWindow() override;

    bool beginGesture(bool opening = true);
    void updateGesture(double progress);
    void endGesture(bool cancelled);
    double gestureProgress() const { return m_gestureProgress; }
    Launcher &launcher() { return *m_launcher; }
    ShellWindow *window() const { return m_window.get(); }
    LauncherView *view() const { return m_view; }
    LauncherWallpaper &wallpaper() { return *m_wallpaper; }

private:
    void showWindow();
    void hideWindow();
    void updateGeometry();
    void activeChanged();
    bool keepsShownWhileInactive() const;

    void presentGesture();
    void settleGesture();
    bool m_gestureActive = false;
    bool m_gestureOpening = true;
    double m_gestureProgress = 0;
    double m_gestureFrom = 0;
    double m_gestureTarget = 0;
    std::chrono::steady_clock::time_point m_gestureTime;
    base::Timer m_gestureTimer;
    Shell *m_shell;
    std::unique_ptr<Launcher> m_launcher;
    Dock *m_dock;
    std::unique_ptr<LauncherWallpaper> m_wallpaper;
    std::unique_ptr<ShellWindow> m_window;
    LauncherView *m_view = nullptr;
    std::vector<base::Subscription> m_subscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Lingmo::Shell

