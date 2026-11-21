// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "terminalwindow.h"

class TerminalSettings;

// Owns the terminal windows of this process. A tab dragged out of a window
// keeps its shell, so the window it lands in has to live in the same process.
class Windows
{
public:
    explicit Windows(TerminalSettings& settings);
    ~Windows();

    // A new window, shown.
    TerminalWindow* create(TerminalWindow::Options options);

    // Destroys the window once the event it was released in is over; the
    // application ends with its last window.
    void release(TerminalWindow* window);

    const std::vector<std::unique_ptr<TerminalWindow>>& windows() const { return m_windows; }

private:
    TerminalSettings& m_settings;
    std::vector<std::unique_ptr<TerminalWindow>> m_windows;
};
