// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <unordered_map>
#include <vector>

#include "compositor/surface_observer.h"
#include "shell/common/task_windows.h"
#include "wm/window_observer.h"

namespace Lingmo::WM {
class Window;
class WindowManager;
} // namespace Lingmo::WM

namespace Lingmo::Shell {

// The window manager's application windows, read directly. Shell windows and
// popups are left out.
class WmTaskWindows : public TaskWindows, private wm::WindowObserver, private compositor::SurfaceObserver
{
public:
    explicit WmTaskWindows(wm::WindowManager &manager);
    ~WmTaskWindows() override;

    WmTaskWindows(const WmTaskWindows &) = delete;
    WmTaskWindows &operator=(const WmTaskWindows &) = delete;

    std::vector<uint64_t> windows() const override { return m_order; }
    const TaskWindow *window(uint64_t id) const override;
    uint64_t activeWindow() const override { return m_activeWindow; }
    const ui::Bitmap *minimizedSnapshot(uint64_t id) const override;

    void activate(uint64_t id) override;
    void minimize(uint64_t id) override;
    void setHidden(uint64_t id, bool hidden) override;
    void close(uint64_t id) override;
    void quitApplication(uint64_t id) override;
    void hideApplication(uint64_t id) override;
    void hideOtherApplications(uint64_t keep) override;

private:
    void observe(wm::Window *window);
    void forget(wm::Window *window);
    // Announces, updates or withdraws the window as the rules say now.
    void refresh(wm::Window *window);
    void updateActiveWindow();
    wm::Window *resolve(uint64_t id) const;
    TaskWindow describe(wm::Window *window) const;

    // wm::WindowObserver
    void onWindowBoundsChanged(wm::Window *window, const base::Rect &oldBounds) override;
    void onWindowStateTypeChanged(wm::Window *window, wm::WindowStateType oldType) override;
    void onWindowActivationChanged(wm::Window *window, bool active) override;
    void onWindowTransientParentChanged(wm::Window *window, wm::Window *oldParent) override;
    void onWindowResponsiveChanged(wm::Window *window) override;
    void onWindowDemandsAttentionChanged(wm::Window *window) override;
    void onWindowApplicationIdChanged(wm::Window *window) override;
    void onWindowTitleChanged(wm::Window *window) override;
    void onWindowHiddenChanged(wm::Window *window) override;
    void onWindowMinimizedSnapshotChanged(wm::Window *window) override;
    void onWindowMinimizeLanded(wm::Window *window) override;
    void onWindowDestroying(wm::Window *window) override;

    // compositor::SurfaceObserver
    void onSurfaceDestroying(compositor::Surface *surface) override;
    void onSurfaceMenuChanged(compositor::Surface *surface) override;
    void onSurfaceWindowHintsChanged(compositor::Surface *surface) override;

    wm::WindowManager &m_manager;
    // Every application window, announced or not: a change of app id can
    // announce one that was the shell's own.
    std::vector<wm::Window *> m_observed;
    std::unordered_map<wm::Window *, compositor::Surface *> m_surfaces;
    std::vector<uint64_t> m_order;
    std::unordered_map<uint64_t, TaskWindow> m_windows;
    // Announced windows, most recently active first.
    std::vector<uint64_t> m_history;
    uint64_t m_activeWindow = 0;
    base::Subscription m_windowAdded;
    base::Subscription m_windowRemoved;
    base::Subscription m_activeChanged;
};

} // namespace Lingmo::Shell

