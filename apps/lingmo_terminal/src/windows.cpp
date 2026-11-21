// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windows.h"

#include <algorithm>

#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/window.h"

Windows::Windows(TerminalSettings& settings)
    : m_settings(settings)
{
}

Windows::~Windows()
{
    // Before the settings go: the windows read them while closing.
    m_windows.clear();
}

TerminalWindow* Windows::create(TerminalWindow::Options options)
{
    m_windows.push_back(std::make_unique<TerminalWindow>(*this, m_settings, std::move(options)));
    TerminalWindow* window = m_windows.back().get();
    window->window().show();
    return window;
}

void Windows::release(TerminalWindow* window)
{
    auto found = std::find_if(m_windows.begin(), m_windows.end(),
                              [window](const auto& owned) { return owned.get() == window; });
    if (found == m_windows.end())
        return;

    std::shared_ptr<TerminalWindow> released(std::move(*found));
    m_windows.erase(found);
    Lingmo::base::EventLoop::current()->post([released, this] {
        // The last window takes the application with it.
        if (m_windows.empty())
            Lingmo::ui::Application::instance()->quit();
    });
}
