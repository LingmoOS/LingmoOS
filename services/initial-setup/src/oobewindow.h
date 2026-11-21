// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include "ui/window.h"

namespace Lingmo::ui {
class Application;
class View;
}

class OobeWindow
{
public:
    OobeWindow();
    ~OobeWindow();

    void show();

private:
    void complete();
    void refreshTheme();

    Lingmo::ui::Window m_window;
    std::unique_ptr<Lingmo::ui::View> m_view;
};
