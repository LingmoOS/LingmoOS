// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "base/signal.h"
#include "clockstore.h"
#include "ui/window.h"
#include <memory>
namespace Lingmo::ui {
class View;
class AboutDialog;
}
namespace Clock {
class ClockWindow {
  public:
    explicit ClockWindow(std::string storePath = {});
    ~ClockWindow();
    void show();
    void showAbout();
    Lingmo::ui::Window& window() {
        return m_window;
    }

  private:
    void showPage(const std::string& id);
    Clock::Store m_store;
    std::unique_ptr<class GlobalMenu> m_menu;
    std::unique_ptr<Lingmo::ui::AboutDialog> m_about;
    Lingmo::ui::Window m_window;
    Lingmo::ui::View* m_root = nullptr;
    Lingmo::ui::View* m_pages = nullptr;
    std::string m_current = "world";
};
} // namespace Clock
