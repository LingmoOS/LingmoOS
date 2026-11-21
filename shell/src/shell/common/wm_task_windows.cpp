// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/wm_task_windows.h"

#include <algorithm>

#include "compositor/surface.h"
#include "ui/bitmap.h"
#include "wm/activation.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::Shell {

WmTaskWindows::WmTaskWindows(wm::WindowManager &manager)
    : m_manager(manager)
{
    m_windowAdded = m_manager.windowAdded.connect([this](wm::Window *window) {
        if (window && !window->isShellWindow() && !window->isPopup())
            observe(window);
    });
    m_windowRemoved = m_manager.windowRemoved.connect([this](wm::Window *window) { forget(window); });
    m_activeChanged = m_manager.activeWindowChanged.connect([this](wm::Window *) { updateActiveWindow(); });

    // Identifiers grow with each window, so they give the order windows came.
    std::vector<wm::Window *> existing;
    for (wm::Window *window : m_manager.windows()) {
        if (!window->isShellWindow() && !window->isPopup())
            existing.push_back(window);
    }
    std::sort(existing.begin(), existing.end(),
              [](const wm::Window *a, const wm::Window *b) { return a->id() < b->id(); });
    for (wm::Window *window : existing)
        observe(window);
    updateActiveWindow();
}

WmTaskWindows::~WmTaskWindows()
{
    for (wm::Window *window : m_observed) {
        window->removeObserver(this);
        const auto surface = m_surfaces.find(window);
        if (surface != m_surfaces.end() && surface->second)
            surface->second->removeSurfaceObserver(this);
    }
}

const TaskWindow *WmTaskWindows::window(uint64_t id) const
{
    const auto it = m_windows.find(id);
    return it == m_windows.end() ? nullptr : &it->second;
}

const ui::Bitmap *WmTaskWindows::minimizedSnapshot(uint64_t id) const
{
    wm::Window *window = resolve(id);
    if (!window || window->minimizedSnapshot().isNull() || !window->state()->isMinimized())
        return nullptr;
    return &window->minimizedSnapshot();
}

wm::Window *WmTaskWindows::resolve(uint64_t id) const
{
    if (!m_windows.count(id))
        return nullptr;
    return m_manager.windowById(id);
}

void WmTaskWindows::activate(uint64_t id)
{
    // As the protocol's activate: a hidden application comes back whole, and
    // a minimized window is restored before it takes the keyboard. Each step
    // may close the window, so it is looked up again after each one.
    wm::Window *window = resolve(id);
    if (!window)
        return;
    m_manager.unhideApplication(window);
    if ((window = resolve(id)))
        window->setHidden(false);
    if ((window = resolve(id)) && window->state()->isMinimized())
        window->restore();
    if ((window = resolve(id)))
        m_manager.activation()->activate(window);
}

void WmTaskWindows::minimize(uint64_t id)
{
    if (wm::Window *window = resolve(id))
        window->minimize();
}

void WmTaskWindows::setHidden(uint64_t id, bool hidden)
{
    if (wm::Window *window = resolve(id))
        window->setHidden(hidden);
}

void WmTaskWindows::close(uint64_t id)
{
    if (wm::Window *window = resolve(id))
        window->requestClose();
}

void WmTaskWindows::quitApplication(uint64_t id)
{
    if (wm::Window *window = resolve(id))
        m_manager.quitApplication(window);
}

void WmTaskWindows::hideApplication(uint64_t id)
{
    if (wm::Window *window = resolve(id))
        m_manager.hideApplication(window);
}

void WmTaskWindows::hideOtherApplications(uint64_t keep)
{
    m_manager.hideOtherApplications(resolve(keep));
}

void WmTaskWindows::observe(wm::Window *window)
{
    if (std::find(m_observed.begin(), m_observed.end(), window) != m_observed.end())
        return;
    m_observed.push_back(window);
    window->addObserver(this);
    if (compositor::Surface *surface = window->rootSurface()) {
        surface->addSurfaceObserver(this);
        m_surfaces[window] = surface;
    }
    refresh(window);
}

void WmTaskWindows::forget(wm::Window *window)
{
    const auto it = std::find(m_observed.begin(), m_observed.end(), window);
    if (it == m_observed.end())
        return;
    m_observed.erase(it);
    window->removeObserver(this);
    const auto surface = m_surfaces.find(window);
    if (surface != m_surfaces.end()) {
        if (surface->second)
            surface->second->removeSurfaceObserver(this);
        m_surfaces.erase(surface);
    }

    const uint64_t id = window->id();
    if (m_windows.erase(id)) {
        std::erase(m_order, id);
        std::erase(m_history, id);
        windowRemoved(id);
    }
    updateActiveWindow();
}

TaskWindow WmTaskWindows::describe(wm::Window *window) const
{
    TaskWindow info;
    info.id = window->id();
    info.appId = window->applicationId();
    info.title = window->title();
    info.pid = window->processId();
    info.parent = window->transientParent() ? window->transientParent()->id() : 0;
    info.geometry = window->bounds();
    info.skipTaskbar = window->skipTaskbar();
    const wm::WindowStateType state = window->stateType();
    info.minimized = state == wm::WindowStateType::Minimized;
    info.maximized = state == wm::WindowStateType::Maximized;
    info.fullscreen = state == wm::WindowStateType::Fullscreen;
    info.hidden = window->isHidden();
    info.responsive = window->isResponsive();
    info.demandsAttention = window->demandsAttention();
    if (compositor::Surface *surface = window->rootSurface()) {
        info.menuService = surface->menuAddress().service;
        info.menuPath = surface->menuAddress().path;
        info.gtkMenuBusName = surface->menuAddress().gtkBusName;
        info.gtkAppMenuPath = surface->menuAddress().gtkAppMenuPath;
        info.gtkMenuPath = surface->menuAddress().gtkMenuPath;
        info.gtkWindowPath = surface->menuAddress().gtkWindowPath;
        info.gtkApplicationPath = surface->menuAddress().gtkApplicationPath;
    }
    return info;
}

void WmTaskWindows::refresh(wm::Window *window)
{
    const uint64_t id = window->id();
    const auto it = m_windows.find(id);

    TaskWindow info = describe(window);
    if (it == m_windows.end()) {
        m_windows.emplace(id, std::move(info));
        m_order.push_back(id);
        windowAdded(id);
        updateActiveWindow();
        return;
    }

    const TaskWindow &old = it->second;
    uint32_t changes = 0;
    if (old.title != info.title)
        changes |= kTitleChange;
    if (old.appId != info.appId)
        changes |= kAppIdChange;
    if (old.pid != info.pid)
        changes |= kPidChange;
    if (old.minimized != info.minimized || old.maximized != info.maximized || old.fullscreen != info.fullscreen
        || old.hidden != info.hidden)
        changes |= kStateChange;
    if (old.skipTaskbar != info.skipTaskbar)
        changes |= kSkipTaskbarChange;
    if (old.geometry != info.geometry)
        changes |= kGeometryChange;
    if (old.menuService != info.menuService || old.menuPath != info.menuPath
        || old.gtkMenuBusName != info.gtkMenuBusName || old.gtkAppMenuPath != info.gtkAppMenuPath
        || old.gtkMenuPath != info.gtkMenuPath
        || old.gtkWindowPath != info.gtkWindowPath || old.gtkApplicationPath != info.gtkApplicationPath)
        changes |= kApplicationMenuChange;
    if (old.parent != info.parent)
        changes |= kParentChange;
    if (old.responsive != info.responsive)
        changes |= kResponsiveChange;
    if (old.demandsAttention != info.demandsAttention)
        changes |= kAttentionChange;
    if (!changes)
        return;
    it->second = std::move(info);
    windowChanged(id, changes);
}

void WmTaskWindows::updateActiveWindow()
{
    wm::Window *window = m_manager.activation()->activeWindow();
    // A sheet speaks for the window it hangs from.
    if (window && window->role() == wm::WindowRole::Sheet && window->transientParent())
        window = window->transientParent();
    std::erase_if(m_history, [this](uint64_t id) {
        const auto it = m_windows.find(id);
        return it == m_windows.end() || !it->second.isTaskListWindow();
    });
    if (!window || window->isShellWindow() || window->isPopup()) {
        m_history.clear();
    } else if (!window->skipTaskbar() && m_windows.count(window->id())) {
        // A window off the task list, such as a dialog of the shell's, leaves
        // the bar and the dock with the application it is in front of.
        std::erase(m_history, window->id());
        m_history.insert(m_history.begin(), window->id());
    }
    const uint64_t id = m_history.empty() ? 0 : m_history.front();
    if (id == m_activeWindow)
        return;
    m_activeWindow = id;
    activeWindowChanged(id);
}

void WmTaskWindows::onWindowBoundsChanged(wm::Window *window, const base::Rect &)
{
    refresh(window);
}

void WmTaskWindows::onWindowStateTypeChanged(wm::Window *window, wm::WindowStateType)
{
    refresh(window);
}

void WmTaskWindows::onWindowActivationChanged(wm::Window *window, bool active)
{
    // The user has the application in front of them: it no longer asks for
    // attention, and the dock stops bouncing its icon.
    if (active && window->demandsAttention())
        window->setDemandsAttention(false);
    refresh(window);
}

void WmTaskWindows::onWindowTransientParentChanged(wm::Window *window, wm::Window *)
{
    refresh(window);
}

void WmTaskWindows::onWindowResponsiveChanged(wm::Window *window)
{
    refresh(window);
}

void WmTaskWindows::onWindowDemandsAttentionChanged(wm::Window *window)
{
    refresh(window);
}

void WmTaskWindows::onWindowApplicationIdChanged(wm::Window *window)
{
    refresh(window);
    updateActiveWindow();
}

void WmTaskWindows::onWindowTitleChanged(wm::Window *window)
{
    refresh(window);
}

void WmTaskWindows::onWindowHiddenChanged(wm::Window *window)
{
    refresh(window);
}

void WmTaskWindows::onWindowMinimizedSnapshotChanged(wm::Window *window)
{
    if (m_windows.count(window->id()))
        minimizedSnapshotChanged(window->id());
}

void WmTaskWindows::onWindowMinimizeLanded(wm::Window *window)
{
    if (m_windows.count(window->id()))
        minimizeLanded(window->id());
}

void WmTaskWindows::onWindowDestroying(wm::Window *window)
{
    // Normally windowRemoved came first; this catches a window torn down
    // without the window manager.
    forget(window);
}

void WmTaskWindows::onSurfaceDestroying(compositor::Surface *surface)
{
    surface->removeSurfaceObserver(this);
    for (auto &[window, observed] : m_surfaces) {
        if (observed == surface)
            observed = nullptr;
    }
}

void WmTaskWindows::onSurfaceMenuChanged(compositor::Surface *surface)
{
    wm::Window *window = m_manager.windowForSurface(surface);
    if (window && window->rootSurface() == surface && m_surfaces.count(window))
        refresh(window);
}

void WmTaskWindows::onSurfaceWindowHintsChanged(compositor::Surface *surface)
{
    wm::Window *window = m_manager.windowForSurface(surface);
    if (window && window->rootSurface() == surface && m_surfaces.count(window)) {
        refresh(window);
        updateActiveWindow();
    }
}

} // namespace Lingmo::Shell
