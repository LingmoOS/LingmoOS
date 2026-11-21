// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/window_cycle/window_cycle_controller.cc
//           window_cycle_list.cc and window_cycle_event_filter.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "wm/window_cycle_controller.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include <xkbcommon/xkbcommon-keysyms.h>

#include "core/logging.h"
#include "core/main_loop.h"
#include "display/display_manager.h"
#include "input/seat.h"
#include "wm/activation.h"
#include "wm/mru_window_tracker.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::WM {

WindowCycleController::WindowCycleController(WindowManager *manager, MruWindowTracker *mruTracker)
    : m_manager(manager)
    , m_mruTracker(mruTracker)
    , m_showTimer(core::mainLoop())
{
    m_windowRemoved = m_manager->windowRemoved.connect([this](Window *window) {
        onWindowRemoved(window);
    });
    m_showTimer.setSingleShot(true);
    m_showTimer.setInterval(std::chrono::milliseconds(kShowDelayMs));
    m_showTimer.onTimeout = [this] {
        showView();
    };
}

WindowCycleController::~WindowCycleController()
{
    m_windowRemoved.reset();
}

bool WindowCycleController::canCycle() const
{
    if (!m_manager->allowsActivation())
        return false;
    // As upstream: the window the pointer holds stays the one in use.
    if (m_manager->eventHandler()->draggedWindow())
        return false;
    for (Window *window : m_manager->windows()) {
        if (window->isSystemDialog() && window->isVisible())
            return false;
    }
    return true;
}

void WindowCycleController::handleCycleWindow(Direction direction)
{
    if (!canCycle())
        return;
    const bool starting = !m_cycling;
    if (starting)
        startCycling();
    step(direction, starting);
}

void WindowCycleController::completeCycling()
{
    if (m_cycling)
        stopCycling(/*accept=*/true);
}

void WindowCycleController::cancelCycling()
{
    if (m_cycling)
        stopCycling(/*accept=*/false);
}

void WindowCycleController::setView(WindowCycleView *view)
{
    if (view == m_view)
        return;
    hideView();
    m_view = view;
    if (m_cycling && !m_showTimer.isActive())
        showView();
}

std::vector<Window *> WindowCycleController::entries() const
{
    std::vector<Window *> entries;
    entries.reserve(m_applications.size());
    for (const auto &windows : m_applications)
        entries.push_back(windows.front());
    return entries;
}

void WindowCycleController::showViewNowForTesting()
{
    m_showTimer.stop();
    showView();
}

bool WindowCycleController::onKey(uint32_t keysym, bool pressed)
{
    if (!m_cycling)
        return false;
    if (!pressed)
        return true;
    switch (keysym) {
    case XKB_KEY_Escape:
        cancelCycling();
        break;
    case XKB_KEY_grave:
    case XKB_KEY_Left:
        step(Direction::Backward, false);
        break;
    case XKB_KEY_Right:
        step(Direction::Forward, false);
        break;
    case XKB_KEY_q:
        // The list follows as its windows close; one asking about unsaved
        // work stays.
        if (!m_applications.empty())
            m_manager->quitApplication(m_applications[m_currentIndex].front());
        break;
    case XKB_KEY_h:
        if (!m_applications.empty()) {
            Window *window = m_applications[m_currentIndex].front();
            if (window->isHidden())
                m_manager->unhideApplication(window);
            else
                m_manager->hideApplication(window);
        }
        break;
    default:
        break;
    }
    // Until the cycle completes every other key belongs to it.
    return true;
}

void WindowCycleController::onPointerMotion(const base::PointF &location)
{
    if (!m_cycling || !isViewShown())
        return;
    if (m_pointerOrigin) {
        const base::PointF moved = location - *m_pointerOrigin;
        if (std::hypot(moved.x(), moved.y()) <= kMouseMovementThreshold)
            return;
        m_pointerOrigin.reset();
    }
    const int index = m_view->windowCycleEntryAt(location);
    if (index < 0 || index >= static_cast<int>(m_applications.size()) || index == m_currentIndex)
        return;
    m_currentIndex = index;
    m_view->selectWindowCycleEntry(m_currentIndex);
}

bool WindowCycleController::onPointerButtonPressed(const base::PointF &location)
{
    if (!m_cycling)
        return false;
    if (!isViewShown()) {
        completeCycling();
        return false;
    }
    const int index = m_view->windowCycleEntryAt(location);
    if (index >= 0 && index < static_cast<int>(m_applications.size())) {
        m_currentIndex = index;
        completeCycling();
    }
    return true;
}

void WindowCycleController::startCycling()
{
    m_applications.clear();
    for (Window *window : m_mruTracker->buildWindowListForCycling()) {
        const auto application =
                std::find_if(m_applications.begin(), m_applications.end(),
                             [this, window](const std::vector<Window *> &windows) {
                                 return m_manager->belongsToApplication(window, windows.front());
                             });
        if (application == m_applications.end())
            m_applications.push_back({window});
        else
            application->push_back(window);
    }
    m_currentIndex = 0;
    m_cycling = true;
    m_pointerOrigin.reset();
    if (const input::Seat *seat = m_manager->seat())
        m_pointerOrigin = seat->pointerLocation();
    m_showTimer.start();
    LM_DEBUG(lingmoInputTransitions) << "application cycle started over" << m_applications.size()
                                << "applications";
}

void WindowCycleController::step(Direction direction, bool startingCycle)
{
    if (m_applications.empty())
        return;

    int offset = direction == Direction::Forward ? 1 : -1;
    // When the first application is not the active one - every window is
    // minimized, or the desktop has the keyboard - the first forward step
    // goes to it rather than past it.
    if (startingCycle && direction == Direction::Forward) {
        Window *active = m_manager->activation()->activeWindow();
        const auto &first = m_applications.front();
        if (std::find(first.begin(), first.end(), active) == first.end()) {
            offset = 0;
            m_currentIndex = 0;
        }
    }

    const int count = static_cast<int>(m_applications.size());
    const int index = ((m_currentIndex + offset) % count + count) % count;
    if (index == m_currentIndex && !startingCycle)
        return;
    m_currentIndex = index;
    if (isViewShown())
        m_view->selectWindowCycleEntry(m_currentIndex);
}

void WindowCycleController::stopCycling(bool accept)
{
    m_cycling = false;
    m_showTimer.stop();
    hideView();
    const auto applications = std::exchange(m_applications, {});
    if (accept && !applications.empty())
        switchToApplication(applications[m_currentIndex]);
    LM_DEBUG(lingmoInputTransitions) << "application cycle" << (accept ? "completed" : "cancelled");
}

void WindowCycleController::switchToApplication(const std::vector<Window *> &windows)
{
    // Switching to a hidden application brings all of its windows back.
    m_manager->unhideApplication(windows.front());
    // Its windows come forward together, keeping their order among themselves.
    const std::vector<Window *> stacking = m_manager->windows();
    for (Window *window : stacking) {
        if (std::find(windows.begin(), windows.end(), window) != windows.end()
            && !window->state()->isMinimized()) {
            m_manager->raise(window);
        }
    }
    const auto shown = std::find_if(windows.begin(), windows.end(), [](Window *window) {
        return !window->state()->isMinimized();
    });
    // An application whose windows are all minimized gets its latest back.
    Window *target = shown != windows.end() ? *shown : windows.front();
    if (target->state()->isMinimized())
        target->restore();
    m_manager->activation()->activate(target);
}

void WindowCycleController::showView()
{
    if (!m_cycling || !m_view || m_applications.empty())
        return;
    m_viewShown = true;
    m_view->showWindowCycle(entries(), m_currentIndex, outputForView());
}

void WindowCycleController::hideView()
{
    if (!std::exchange(m_viewShown, false))
        return;
    if (m_view)
        m_view->hideWindowCycle();
}

std::string WindowCycleController::outputForView() const
{
    const display::DisplayManager *displays = m_manager->displayManager();
    if (!displays || !displays->applied())
        return {};
    const base::Rect bounds = m_manager->outputForNewWindow();
    for (const auto &logical : displays->applied()->logicalDisplays) {
        if (logical.bounds == bounds)
            return logical.waylandName;
    }
    return {};
}

void WindowCycleController::onWindowRemoved(Window *window)
{
    if (!m_cycling)
        return;
    const auto application = std::find_if(m_applications.begin(), m_applications.end(),
                                          [window](const std::vector<Window *> &windows) {
                                              return std::find(windows.begin(), windows.end(),
                                                               window)
                                                      != windows.end();
                                          });
    if (application == m_applications.end())
        return;
    std::erase(*application, window);
    if (!application->empty()) {
        // Its entry now shows another window.
        if (isViewShown())
            showView();
        return;
    }
    const int index = static_cast<int>(application - m_applications.begin());
    m_applications.erase(application);
    if (m_applications.empty()) {
        stopCycling(/*accept=*/false);
        return;
    }
    if (index < m_currentIndex || m_currentIndex >= static_cast<int>(m_applications.size()))
        --m_currentIndex;
    if (isViewShown())
        showView();
}

} // namespace Lingmo::WM
