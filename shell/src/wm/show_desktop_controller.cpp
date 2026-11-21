// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/show_desktop_controller.h"

#include <algorithm>
#include <cmath>

#include "compositor/compositor.h"
#include "wm/overview/overview_controller.h"
#include "wm/window.h"
#include "wm/window_manager.h"

namespace Lingmo::WM {

namespace {

double easeOut(double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return 1 - std::pow(1 - t, 3);
}

// Moved aside: the application windows on screen, dialogs and sheets
// included, but not the shell's own or popups, which belong to the window they
// open from.
bool isMovable(const Window *window)
{
    return window->isVisible() && !window->isPopup()
            && (!window->isShellWindow() || window->role() == WindowRole::Sheet);
}

const Window *transientRoot(const Window *window)
{
    while (window->transientParent())
        window = window->transientParent();
    return window;
}

} // namespace

ShowDesktopController::ShowDesktopController(WindowManager *manager)
    : m_manager(manager)
{
    m_manager->activation()->addObserver(this);
    m_manager->desks()->addObserver(this);
    if (compositor::Compositor *compositor = m_manager->compositor())
        m_frameStarted = compositor->frameStarted.connect([this](double frameTimeMs) {
            advance(frameTimeMs);
        });
}

ShowDesktopController::~ShowDesktopController()
{
    releaseAll();
    m_manager->desks()->removeObserver(this);
    m_manager->activation()->removeObserver(this);
}

bool ShowDesktopController::isMovedAside(const Window *window) const
{
    return std::find(m_windows.begin(), m_windows.end(), window) != m_windows.end();
}

bool ShowDesktopController::beginGesture()
{
    if (m_active || m_tracking || !m_manager->allowsActivation() || m_manager->overview()->isActive()) return false;
    m_gestureStart = m_progress;
    m_gestureActiveWindow = m_manager->activation()->activeWindow() ? m_manager->activation()->activeWindow()->id() : 0;
    enter();
    if (!m_active) return false;
    m_tracking = true; m_sliding = false;
    m_progress = m_gestureStart; present();
    return true;
}

void ShowDesktopController::updateGesture(double progress)
{
    if (!m_tracking || !std::isfinite(progress)) return;
    m_progress = std::clamp(progress, 0.0, 1.0);
    present(); requestFrame();
}

void ShowDesktopController::endGesture(bool cancelled)
{
    if (!m_tracking) return;
    m_tracking = false;
    m_active = !cancelled && m_progress >= 0.5;
    slideTo(m_active ? 1 : 0);
    if (!m_active && m_gestureActiveWindow) {
        for (auto *window : m_manager->windows()) {
            if (window->id() == m_gestureActiveWindow && window->isVisible()) {
                m_manager->activation()->activate(window); break;
            }
        }
    }
    m_gestureActiveWindow = 0;
}

void ShowDesktopController::toggle()
{
    if (m_active)
        exit();
    else
        enter();
}

void ShowDesktopController::enter()
{
    // The overview has the screen and the windows meanwhile.
    if (m_active || !m_manager->allowsActivation() || m_manager->overview()->isActive())
        return;
    // Windows still sliding back from the last time are taken along.
    for (Window *window : m_manager->windows()) {
        if (isMovable(window) && !isMovedAside(window)) {
            window->addObserver(this);
            m_windows.push_back(window);
        }
    }
    if (m_windows.empty())
        return;
    // The desktop takes the keyboard. Activating it before the mode starts
    // keeps that activation from ending the mode.
    for (Window *window : m_manager->windows()) {
        if (window->role() == WindowRole::Desktop && window->isVisible()) {
            m_manager->activation()->activate(window);
            break;
        }
    }
    m_active = true;
    slideTo(1);
}

void ShowDesktopController::exit(Window *activate)
{
    if (!m_active)
        return;
    m_tracking = false;
    m_active = false;
    slideTo(0);
    if (activate)
        m_manager->activation()->activate(activate);
}

void ShowDesktopController::exitImmediately()
{
    m_tracking = false;
    m_active = false;
    m_sliding = false;
    m_progress = 0;
    releaseAll();
}

void ShowDesktopController::advance(double frameTimeMs)
{
    if (!m_sliding)
        return;
    if (m_slideStarted < 0)
        m_slideStarted = frameTimeMs;
    const double t = (frameTimeMs - m_slideStarted) / kSlideDurationMs;
    m_progress = m_slideFrom + (m_slideTo - m_slideFrom) * easeOut(t);
    if (t >= 1) {
        m_progress = m_slideTo;
        m_sliding = false;
    }
    present();
    if (m_sliding)
        requestFrame();
    else if (m_progress == 0)
        releaseAll();
}

void ShowDesktopController::onWindowActivated(Window *gainedActive, Window *)
{
    if (m_active && gainedActive && !gainedActive->isShellWindow())
        exit();
}

void ShowDesktopController::onDeskActivationChanged(const Desk *, const Desk *)
{
    exitImmediately();
}

void ShowDesktopController::onDeskSwitchAnimationLaunching()
{
    // Before the switch takes its frames of the desk being left.
    exitImmediately();
}

void ShowDesktopController::onWindowBoundsChanged(Window *, const base::Rect &)
{
    present();
}

void ShowDesktopController::onWindowVisibilityChanged(Window *window, bool visible)
{
    if (!visible)
        release(window);
}

void ShowDesktopController::onWindowDestroying(Window *window)
{
    release(window);
}

base::Point ShowDesktopController::awayOffset(const Window *window) const
{
    // A dialog goes the way of the window it belongs to, and keeps its place
    // over it.
    const Window *root = transientRoot(window);
    const base::Rect output = m_manager->outputBoundsFor(root);
    const base::Rect bounds = root->bounds();

    // An edge another output continues past would show the window there.
    const auto continues = [this, &output](int x) {
        for (const WindowManager::OutputArea &area : m_manager->outputAreas()) {
            if (area.bounds != output && area.bounds.contains(x, centerPixel(output).y()))
                return true;
        }
        return false;
    };
    bool left = centerPixel(bounds).x() < centerPixel(output).x();
    if (left && continues(output.left() - 1) && !continues(output.right()))
        left = false;
    else if (!left && continues(output.right()) && !continues(output.left() - 1))
        left = true;

    const int dx = left ? std::min(0, output.left() + kSliverWidth - (bounds.left() + bounds.width()))
                        : std::max(0, output.left() + output.width() - kSliverWidth - bounds.left());
    return base::Point(dx, 0);
}

void ShowDesktopController::slideTo(double target)
{
    // A reversal mid-slide turns around from where the windows are.
    m_slideFrom = m_progress;
    m_slideTo = target;
    m_slideStarted = -1;
    m_sliding = m_progress != target;
    if (m_sliding)
        requestFrame();
    else if (target == 0)
        releaseAll();
}

void ShowDesktopController::present()
{
    for (Window *window : m_windows)
        window->setPresentationOffset((base::PointF(awayOffset(window)) * m_progress).toPoint());
}

void ShowDesktopController::release(Window *window)
{
    const auto it = std::find(m_windows.begin(), m_windows.end(), window);
    if (it == m_windows.end())
        return;
    m_windows.erase(it);
    window->removeObserver(this);
    window->setPresentationOffset(base::Point());
}

void ShowDesktopController::releaseAll()
{
    while (!m_windows.empty())
        release(m_windows.back());
}

void ShowDesktopController::requestFrame()
{
    if (compositor::Compositor *compositor = m_manager->compositor())
        compositor->scheduleRepaint();
}

} // namespace Lingmo::WM
