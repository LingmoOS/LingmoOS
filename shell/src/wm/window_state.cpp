// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/window_state.h"

#include <algorithm>

#include "scene/window_animation.h"
#include "wm/activation.h"
#include "wm/placement.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_observer.h"
#include "wm/window_snap.h"
#include "wm/wm_event.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::WM {

namespace {

// Upper bound for previews and high-density dock thumbnails.
constexpr int kMinimizedSnapshotSize = 384;

base::WeakPtr<WindowManager> weakManager(Window *window)
{
    return window->manager() ? window->manager()->weakPtr() : base::WeakPtr<WindowManager>();
}

// A window may only restore back to a state at a strictly lower layer.
// Upstream: ash/wm/window_state.cc kWindowStateRestoreHistoryLayerMap.
int restoreLayer(WindowStateType type)
{
    switch (type) {
    case WindowStateType::Normal:
        return 0;
    case WindowStateType::PrimarySnapped:
    case WindowStateType::SecondarySnapped:
        return 1;
    case WindowStateType::Maximized:
        return 2;
    case WindowStateType::Fullscreen:
        return 3;
    case WindowStateType::Minimized:
        return 4;
    }
    return 0;
}

bool canRestoreState(WindowStateType current, WindowStateType previous)
{
    return restoreLayer(current) > restoreLayer(previous);
}

// Unmaximizing to bounds that are still the size of the work area leaves no
// visible difference and nothing to grab, so inset them slightly.
// Upstream: ash/wm/default_state.cc kMaximizedWindowInset.
constexpr int kMaximizedWindowInset = 10;

} // namespace

WindowState::WindowState(Window *window)
    : m_window(window)
{
}

WindowState::~WindowState() = default;

void WindowState::saveCurrentBoundsForRestore()
{
    m_restoreBounds = m_window->bounds();
}

bool WindowState::canSnap() const
{
    return m_window->isResizable()
            && canSnapSize(m_window->minimumOuterSize(),
                           m_window->manager()->workAreaFor(m_window));
}

WindowStateType WindowState::restoreStateType() const
{
    return m_restoreHistory.empty() ? WindowStateType::Normal : m_restoreHistory.back();
}

void WindowState::onEvent(const WMEvent &event)
{
    if (m_window->isPopup())
        return;
    switch (event.type()) {
    case WMEventType::Normal:
        enterState(WindowStateType::Normal);
        break;
    case WMEventType::Maximize:
        enterState(WindowStateType::Maximized);
        break;
    case WMEventType::Minimize:
        enterState(WindowStateType::Minimized);
        break;
    case WMEventType::Fullscreen:
        enterState(WindowStateType::Fullscreen);
        break;
    case WMEventType::SnapPrimary:
        enterState(WindowStateType::PrimarySnapped);
        break;
    case WMEventType::SnapSecondary:
        enterState(WindowStateType::SecondarySnapped);
        break;
    case WMEventType::Restore:
        enterState(restoreStateType());
        break;
    case WMEventType::ToggleMaximize:
        enterState(isMaximized() ? restoreStateType() : WindowStateType::Maximized);
        break;
    case WMEventType::ToggleFullscreen:
        enterState(isFullscreen() ? restoreStateType() : WindowStateType::Fullscreen);
        break;
    case WMEventType::SetBounds: {
        const auto &setBounds = static_cast<const SetBoundsWMEvent &>(event);
        if (isNormal()) {
            base::Rect bounds = setBounds.requestedBounds();
            adjustBoundsToEnsureMinimumVisibility(
                    m_window->manager()->visibleAreaFor(m_window, bounds), &bounds,
                    m_window->manager()->workAreaFor(m_window, bounds));
            setBoundsDirect(bounds);
        } else if (isMinimized()) {
            // Nothing to show, but the window still needs somewhere to come
            // back to.
            m_restoreBounds = setBounds.requestedBounds();
        }
        break;
    }
    case WMEventType::WorkAreaChanged:
        if (!m_window->manager()->outputBounds().isEmpty())
            updateBoundsFromState(m_type);
        break;
    }
}

void WindowState::enterState(WindowStateType next)
{
    if (next == WindowStateType::Maximized && !m_window->isResizable())
        return;
    if (isSnappedStateType(next) && !canSnap())
        return;
    if (next == m_type)
        return;

    const WindowStateType previous = m_type;
    const base::Rect previousBounds = m_window->bounds();

    // Only the normal state's bounds are worth coming back to.
    if (previous == WindowStateType::Normal)
        saveCurrentBoundsForRestore();

    m_type = next;
    updateRestoreHistory(previous);
    updateBoundsFromState(previous);
    updateVisibility(previous);

    // A hidden window, or one on an inactive desk, has nothing on screen to
    // animate, and a minimize snapshot taken now would be blank.
    auto *animation = dynamic_cast<scene::WindowAnimation *>(m_window->node());
    if (animation && !m_window->isHidden() && !m_window->isDeskHidden()) {
        using Type = scene::WindowAnimation::Type;
        // Read the dock bounds again on restore: its icon may have moved.
        base::Rect target = m_window->minimizedGeometry();
        if (target.isEmpty()) {
            const base::Rect output = m_window->manager()->outputBoundsFor(m_window);
            target = base::Rect(centerPixel(output).x(), output.bottom() - 1, 1, 1);
        }
        const bool dock = next == WindowStateType::Minimized || previous == WindowStateType::Minimized;
        const base::WeakPtr<WindowManager> manager = weakManager(m_window);
        const uint64_t id = m_window->id();
        animation->setHoldAtEnd(next == WindowStateType::Minimized);
        if (next == WindowStateType::Minimized) {
            // Landed on the dock slot: hand over to the shell's thumbnail.
            animation->setFinishedCallback([manager, id] {
                if (!manager)
                    return;
                manager->runLater(std::chrono::milliseconds(0), [manager, id] {
                    if (Window *window = manager->windowById(id))
                        window->beginMinimizeHandoff();
                });
            });
        } else if (dock) {
            // Back into the window's own layer once it has left the panels' way.
            animation->setFinishedCallback([manager] {
                if (!manager)
                    return;
                manager->runLater(std::chrono::milliseconds(0), [manager] {
                    manager->restackScene();
                });
            });
        } else {
            animation->setFinishedCallback({});
        }
        if (next == WindowStateType::Minimized) {
            animation->start(Type::Minimize, previousBounds, target);
            requestMinimizedSnapshot(animation);
        } else if (previous == WindowStateType::Minimized) {
            animation->start(Type::Restore, target, m_window->bounds());
        } else if (previousBounds != m_window->bounds()) {
            animation->start(Type::Bounds, previousBounds, m_window->bounds());
        }
        if (dock)
            m_window->manager()->restackScene();
    } else if (animation && previous == WindowStateType::Minimized) {
        // A leftover Minimize keeps drawing the old snapshot in place of the
        // live window once it shows.
        animation->setHoldAtEnd(false);
        animation->setFinishedCallback({});
        animation->skip(scene::WindowAnimation::Type::Restore);
    }

    // Restore bounds and restore history are two views of the same thing;
    // an empty history means there is nothing left to restore to.
    if (m_restoreHistory.empty())
        clearRestoreBounds();

    for (WindowObserver *observer : std::vector<WindowObserver *>(m_window->m_observers)) {
        if (m_window->hasObserver(observer))
            observer->onWindowStateTypeChanged(m_window, previous);
    }
}

void WindowState::updateMinimizeTarget()
{
    const base::Rect target = m_window->minimizedGeometry();
    auto *animation = dynamic_cast<scene::WindowAnimation *>(m_window->node());
    if (animation && isMinimized() && !target.isEmpty())
        animation->setMinimizeTarget(target);
}

void WindowState::requestMinimizedSnapshot(scene::WindowAnimation *animation)
{
    const base::WeakPtr<WindowManager> manager = weakManager(m_window);
    const uint64_t id = m_window->id();
    animation->requestSnapshot(
            [manager, id](const ui::Bitmap &bitmap) {
                if (!manager)
                    return;
                // Rendering is not the place to notify observers.
                manager->runLater(std::chrono::milliseconds(0), [manager, id, bitmap] {
                    Window *window = manager->windowById(id);
                    if (!window || !window->state()->isMinimized())
                        return;
                    window->setMinimizedSnapshot(bitmap);
                });
            },
            kMinimizedSnapshotSize);
}

void WindowState::updateRestoreHistory(WindowStateType previous)
{
    while (!m_restoreHistory.empty() && !canRestoreState(m_type, m_restoreHistory.back()))
        m_restoreHistory.pop_back();

    if (canRestoreState(m_type, previous))
        m_restoreHistory.push_back(previous);
}

void WindowState::updateBoundsFromState(WindowStateType previous)
{
    WindowManager *manager = m_window->manager();
    const base::Rect workArea = manager->workAreaFor(m_window);

    switch (m_type) {
    case WindowStateType::Normal: {
        base::Rect bounds = m_window->bounds();
        base::Rect normalWorkArea = workArea;
        if (hasRestoreBounds()) {
            bounds = restoreBounds();
            // A window fullscreen on another output comes back to the one it
            // left, so that output's areas are the ones that apply.
            normalWorkArea = manager->workAreaFor(m_window, bounds);
            // The restore bounds can be as large as the work area if the window
            // was resized to it, or if the output shrank while it was
            // maximized. Inset so the result is visibly not maximized. Only
            // when leaving Maximized: a minimise or fullscreen round trip
            // brings a work-area-sized window back exactly as it was.
            if (previous == WindowStateType::Maximized && !normalWorkArea.isEmpty()
                && bounds.width() >= normalWorkArea.width()
                && bounds.height() >= normalWorkArea.height()) {
                bounds = normalWorkArea.adjusted(kMaximizedWindowInset, kMaximizedWindowInset,
                                                 -kMaximizedWindowInset, -kMaximizedWindowInset);
            }
        }
        adjustBoundsToEnsureMinimumVisibility(manager->visibleAreaFor(m_window, bounds), &bounds,
                                              normalWorkArea);
        setBoundsDirect(bounds);
        break;
    }
    case WindowStateType::PrimarySnapped:
    case WindowStateType::SecondarySnapped:
        setBoundsDirect(snapBounds(m_type == WindowStateType::PrimarySnapped ? SnapType::Primary
                                                                             : SnapType::Secondary,
                                   workArea));
        break;
    case WindowStateType::Maximized:
        setBoundsDirect(workArea);
        break;
    case WindowStateType::Fullscreen:
        setBoundsDirect(manager->outputBoundsFor(m_window));
        break;
    case WindowStateType::Minimized:
        // A minimized window keeps the bounds it will come back to.
        break;
    }
}

void WindowState::updateVisibility(WindowStateType previous)
{
    if (isMinimized()) {
        // Window::setVisible drops activation; a window nobody can see must
        // not hold the keyboard.
        m_window->setVisible(false);
    } else if (previous == WindowStateType::Minimized) {
        m_window->setMinimizedSnapshot(ui::Bitmap());
        m_window->setVisible(!m_window->isHidden());
        m_window->manager()->restackScene();
    }
}

void WindowState::setBoundsDirect(const base::Rect &bounds)
{
    base::Rect adjusted = bounds;
    // Below the minimum size nothing useful is left to interact with, whatever
    // the state machine computed.
    if (isNormal()) {
        const base::Size minimum = m_window->minimumOuterSize();
        adjusted.setWidth(std::max(adjusted.width(), minimum.width()));
        adjusted.setHeight(std::max(adjusted.height(), minimum.height()));
    }
    m_window->setBounds(adjusted);
}

} // namespace Lingmo::WM
