// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/mru_window_tracker.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "wm/mru_window_tracker.h"

#include <algorithm>

#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::WM {

bool canIncludeWindowInMruList(const Window *window)
{
    if (window->skipTaskbar())
        return false;
    if (window->focusPolicy() == FocusPolicy::NoFocus)
        return false;
    if (window->transientParent())
        return false;
    // A hidden application, or one on another desk, is switched to as a
    // minimized window is.
    return window->targetVisibility() || window->state()->isMinimized() || window->isHidden();
}

MruWindowTracker::MruWindowTracker(WindowManager *manager)
    : m_manager(manager)
{
    m_manager->activation()->addObserver(this);
    m_windowRemoved = m_manager->windowRemoved.connect([this](Window *window) {
        onWindowRemoved(window);
    });
}

MruWindowTracker::~MruWindowTracker()
{
    m_windowRemoved.reset();
    m_manager->activation()->removeObserver(this);
}

MruWindowTracker::WindowList MruWindowTracker::buildMruWindowList() const
{
    WindowList windows;
    // The tracked list keeps the most recent window last.
    for (auto it = m_mruWindows.rbegin(); it != m_mruWindows.rend(); ++it) {
        if (canIncludeWindowInMruList(*it))
            windows.push_back(*it);
    }
    // Windows that were never activated come after, topmost first.
    const std::vector<Window *> &stacking = m_manager->windows();
    for (auto it = stacking.rbegin(); it != stacking.rend(); ++it) {
        Window *window = *it;
        if (!canIncludeWindowInMruList(window))
            continue;
        if (std::find(windows.begin(), windows.end(), window) == windows.end())
            windows.push_back(window);
    }
    return windows;
}

MruWindowTracker::WindowList MruWindowTracker::buildWindowListForCycling() const
{
    WindowList windows = buildMruWindowList();
    std::erase_if(windows, [this](Window *window) {
        return !window->state()->isMinimized() && !window->isHidden()
                && !m_manager->activation()->canActivate(window);
    });
    return windows;
}

void MruWindowTracker::onWindowActivated(Window *gainedActive, Window *)
{
    setActiveWindow(gainedActive);
}

void MruWindowTracker::onWindowRemoved(Window *window)
{
    std::erase(m_mruWindows, window);
}

void MruWindowTracker::onWindowMovedOutFromRemovingDesk(Window *window)
{
    const auto it = std::find(m_mruWindows.begin(), m_mruWindows.end(), window);
    if (it == m_mruWindows.end())
        return;
    m_mruWindows.erase(it);
    m_mruWindows.insert(m_mruWindows.begin(), window);
}

void MruWindowTracker::setActiveWindow(Window *window)
{
    if (!window)
        return;
    std::erase(m_mruWindows, window);
    m_mruWindows.push_back(window);
}

} // namespace Lingmo::WM
