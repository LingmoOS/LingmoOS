namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <chrono>
#include <cmath>

#include "base/event_loop.h"
#include "base/paths.h"
#include "base/settings.h"
#include "ui/application.h"
#include "ui/platform_window.h"
#include "ui/window.h"

namespace Lingmo::ui {

// The end of a resize comes before the configure carrying its last size.
static constexpr auto kSaveDelay = std::chrono::milliseconds(300);

static std::string storePath()
{
    std::string process = Application::instance()->applicationId();
    if (process.rfind("lingmo-", 0) == 0)
        process.erase(0, 9);
    return process.empty() ? std::string() : base::paths::lingmoConfig(process);
}

void Window::setRestoresSize(std::string windowId)
{
    m_restoreId = std::move(windowId);
}

void Window::restoreSize()
{
    if (m_restoreId.empty() || isPopup())
        return;
    const std::string path = storePath();
    if (path.empty())
        return;

    const base::Settings settings(path);
    if (const auto saved = settings.sizeValue("WindowSize/" + m_restoreId); saved && saved->width > 0 && saved->height > 0) {
        m_size = {std::clamp(double(saved->width), m_minimumSize.width, std::max(m_minimumSize.width, m_maximumSize.width)),
                  std::clamp(double(saved->height), m_minimumSize.height,
                             std::max(m_minimumSize.height, m_maximumSize.height))};
    }
    // Where a window opened maximized returns to when the user restores it.
    m_platform->setRestoreSize(int(std::lround(m_size.width)), int(std::lround(m_size.height)));

    // Maximized before it is shown, so its very first frame already is.
    const bool resizable = !(m_minimumSize == m_maximumSize);
    if (resizable && settings.boolValue("WindowMaximized/" + m_restoreId))
        m_platform->setMaximized(true);
}

void Window::handleConfigureBounds(int width, int height)
{
    m_bounds = {double(width), double(height)};
}

void Window::handleInteractiveResize(bool active)
{
    const bool changed = active != m_interactiveResize;
    m_interactiveResize = active;
    if (active)
        m_resized = false;
    if (changed && onInteractiveResizeChanged)
        onInteractiveResizeChanged();
    if (active || !m_resized || m_restoreId.empty())
        return;

    base::EventLoop& loop = Application::instance()->loop();
    if (m_saveTimer)
        loop.stopTimer(m_saveTimer);
    m_saveTimer = loop.startTimer(kSaveDelay, false, [self = std::weak_ptr<Window*>(m_self)] {
        if (const auto window = self.lock()) {
            (*window)->m_saveTimer = 0;
            (*window)->saveSize();
        }
    });
}

void Window::saveSize()
{
    if (m_restoreId.empty() || m_states.maximized || m_states.fullScreen)
        return;
    const std::string path = storePath();
    if (path.empty())
        return;
    base::Settings settings(path);
    settings.setValue("WindowSize/" + m_restoreId,
                      base::Settings::Size{int(std::lround(m_size.width)), int(std::lround(m_size.height))});
}

// Full screen says nothing about how the window wants to reopen.
void Window::saveMaximized()
{
    if (m_restoreId.empty() || m_states.fullScreen)
        return;
    const std::string path = storePath();
    if (path.empty())
        return;
    base::Settings settings(path);
    const std::string key = "WindowMaximized/" + m_restoreId;
    if (settings.boolValue(key) == m_states.maximized)
        return;
    if (m_states.maximized)
        settings.setValue(key, true);
    else
        settings.remove(key);
}

} // namespace Lingmo::ui
