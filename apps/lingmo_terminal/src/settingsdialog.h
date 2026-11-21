// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"

namespace Lingmo::ui {
class ComboBox;
class DialogWindow;
class Slider;
class Switch;
class Label;
}

class SettingsContent;
class TerminalWindow;

// The terminal's settings, shown over a window.
class SettingsDialog
{
public:
    explicit SettingsDialog(TerminalWindow* window);
    ~SettingsDialog();

    void show();
    void close();
    // An open dialog takes the new language in place, keeping its window.
    void retranslate();

private:
    void update();

    TerminalWindow* m_window;
    std::unique_ptr<Lingmo::ui::DialogWindow> m_dialog;
    SettingsContent* m_content = nullptr;
    bool m_shown = false;
    Lingmo::base::Subscription m_settingsChanged;
};
