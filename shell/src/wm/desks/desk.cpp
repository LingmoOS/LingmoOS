// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/desk.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65

#include "wm/desks/desk.h"

#include <algorithm>

#include "wm/desks/desks_controller.h"
#include "wm/mru_window_tracker.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::WM {

namespace {

uint32_t nextDeskId()
{
    static uint32_t next = 0;
    return ++next;
}

} // namespace

Desk::Desk(DesksController *controller)
    : m_controller(controller)
    , m_id(nextDeskId())
{
}

Desk::~Desk() = default;

bool Desk::contains(const Window *window) const
{
    return std::find(m_windows.begin(), m_windows.end(), window) != m_windows.end();
}

void Desk::setName(const std::string &name, bool setByUser)
{
    if (name == m_name && setByUser == m_nameSetByUser)
        return;
    m_name = name;
    m_nameSetByUser = setByUser && !name.empty();
    m_controller->notify([this](DesksObserver *observer) {
        observer->onDeskNameChanged(this);
    });
}

void Desk::addWindow(Window *window)
{
    if (!contains(window))
        m_windows.push_back(window);
}

void Desk::removeWindow(Window *window)
{
    std::erase(m_windows, window);
}

void Desk::activate(bool updateWindowActivation)
{
    m_active = true;
    m_controller->updateDeskVisibility();

    if (!updateWindowActivation || m_windows.empty())
        return;

    // The window on this desk that was most recently used right before the
    // user left it, so as not to break the user's workflow.
    WindowManager *manager = m_controller->windowManager();
    for (Window *window : manager->mruWindowTracker()->buildMruWindowList()) {
        if (m_controller->deskOf(window) != this)
            continue;
        // Activating a minimized or hidden window would bring it back.
        if (window->state()->isMinimized() || window->isHidden())
            continue;
        manager->activation()->activate(window);
        return;
    }
}

void Desk::deactivate(bool updateWindowActivation)
{
    Window *active = m_controller->windowManager()->activation()->activeWindow();
    m_active = false;
    // Hiding the active window moves activation on by itself; upstream hides the
    // containers first for the same reason, so that it lands off this desk.
    m_controller->updateDeskVisibility();
    if (!updateWindowActivation)
        return;
    Activation *activation = m_controller->windowManager()->activation();
    if (active && activation->activeWindow() == active && m_controller->deskOf(active) == this)
        activation->deactivate(active);
}

void Desk::moveWindowsToDesk(Desk *target)
{
    WindowManager *manager = m_controller->windowManager();
    // Top-most first: lowering each to the bottom in turn keeps their order.
    std::vector<Window *> windows;
    const auto &stacking = manager->windows();
    for (auto it = stacking.rbegin(); it != stacking.rend(); ++it) {
        if (contains(*it))
            windows.push_back(*it);
    }
    for (Window *window : windows) {
        if (!manager->hasWindow(window) || !contains(window))
            continue;
        m_controller->assignWindow(window, target);
        manager->stackAtBottomOfLayer(window);
        manager->mruWindowTracker()->onWindowMovedOutFromRemovingDesk(window);
    }
}

void Desk::moveWindowToDesk(Window *window, Desk *target)
{
    // Always the root of the transient tree: a transient child is never left
    // behind by its parent, nor moved without it.
    Window *root = m_controller->transientRoot(window);
    if (!contains(root) || target == this)
        return;
    m_controller->assignWindow(root, target);
    // Upstream FixWindowStackingAccordingToGlobalMru: the window was the one in
    // use, so it goes on top of the target desk's windows.
    m_controller->windowManager()->raise(root);
    if (root->state()->isMinimized())
        root->restore();
}

} // namespace Lingmo::WM
