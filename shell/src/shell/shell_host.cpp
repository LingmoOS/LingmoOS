// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/shell_host.h"

#include <cmath>

#include "base/event_loop.h"
#include "core/main_loop.h"
#include "shell/shell.h"
#include "shell/shell_data.h"
#include "shell/shell_drag.h"
#include "shell/shell_text_input.h"
#include "shell/shell_window.h"
#include "ui/embedded_window.h"
#include "ui/theme.h"

namespace Lingmo::Shell {

ShellHost::ShellHost(Shell *shell, wayland::InputMethodRelay *inputMethod)
    : m_shell(shell)
    , m_drag(std::make_unique<ShellDrag>(shell))
{
    if (inputMethod)
        m_textInput = std::make_unique<ShellTextInput>(inputMethod);
    ui::EmbeddedWindow::setHost(this);
}

ShellHost::~ShellHost()
{
    *m_alive = false;
    if (ui::EmbeddedWindow::host() == this)
        ui::EmbeddedWindow::setHost(nullptr);
    for (auto &[popup, window] : m_popups)
        window->release();
    m_popups.clear();
    m_released.clear();
    m_textInput.reset();
    m_drag.reset();
}

ShellWindow *ShellHost::windowFor(const ui::EmbeddedWindow *window) const
{
    if (!window)
        return nullptr;
    for (ShellWindow *candidate : m_shell->windows()) {
        if (candidate->window() == window)
            return candidate;
    }
    return nullptr;
}

void ShellHost::windowDestroying(ShellWindow *window)
{
    m_glassDark.erase(window->window());
    if (m_textInput)
        m_textInput->windowDestroying(window);
    if (m_drag)
        m_drag->windowDestroying(window);
}

void ShellHost::frameStarting(ShellWindow *window)
{
    const auto it = m_glassDark.find(window->window());
    if (it != m_glassDark.end() && it->second != ui::Theme::current().darkMode)
        blurChanged(window->window());
}

void ShellHost::popupShown(ui::EmbeddedWindow *popup)
{
    ShellWindow *parent = windowFor(popup->popupParent());
    if (!parent || m_popups.contains(popup))
        return;
    auto window = std::make_unique<ShellWindow>(parent, popup);
    ShellWindow *shown = window.get();
    m_popups[popup] = std::move(window);
    // A popup sets its blur before it is shown, when there was no window to
    // take it yet.
    if (popup->blurRect().width > 0 && popup->blurRect().height > 0)
        blurChanged(popup);
    shown->show();
}

void ShellHost::popupRepositioned(ui::EmbeddedWindow *popup)
{
    const auto it = m_popups.find(popup);
    if (it != m_popups.end())
        it->second->place();
}

void ShellHost::popupHidden(ui::EmbeddedWindow *popup)
{
    const auto it = m_popups.find(popup);
    if (it == m_popups.end())
        return;
    std::unique_ptr<ShellWindow> window = std::move(it->second);
    m_popups.erase(it);
    window->release();
    m_released.push_back(std::move(window));
    core::mainLoop()->post([this, alive = m_alive] {
        if (*alive)
            m_released.clear();
    });
}

void ShellHost::activationRequested(ui::EmbeddedWindow *window)
{
    if (ShellWindow *shellWindow = windowFor(window))
        shellWindow->activate();
}

void ShellHost::textInputChanged(ui::EmbeddedWindow *window, const ui::TextInputState *state,
                                 bool fromInputMethod)
{
    if (m_textInput)
        m_textInput->update(windowFor(window), state, fromInputMethod);
}

void ShellHost::blurChanged(ui::EmbeddedWindow *window)
{
    ShellWindow *shellWindow = windowFor(window);
    if (!shellWindow)
        return;
    const ui::RectF rect = window->blurRect();
    const int left = int(std::lround(rect.x));
    const int top = int(std::lround(rect.y));
    // FishUI's popups (menus, drop-downs) sit on PopupGlass, outlined and
    // masked by their bubble.
    const bool dark = ui::Theme::current().darkMode;
    m_glassDark[window] = dark;
    shellWindow->setBackgroundBlur(base::Rect(left, top, int(std::lround(rect.x + rect.width)) - left,
                                              int(std::lround(rect.y + rect.height)) - top),
                                   int(std::lround(window->blurRadius())),
                                   dark ? popupGlass(0.17, 0.17, 0.19, 0.6) : popupGlass(1, 1, 1, 0.6),
                                   window->blurShape());
}

bool ShellHost::startDrag(ui::EmbeddedWindow *window, ui::DragSource source)
{
    ShellWindow *shellWindow = windowFor(window);
    return shellWindow && m_drag->start(shellWindow, std::move(source));
}

} // namespace Lingmo::Shell
