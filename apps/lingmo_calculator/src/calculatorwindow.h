// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "calculator.h"
#include "globalmenu.h"
#include "base/signal.h"
#include "base/weak_ptr.h"
#include "ui/window.h"
#include <memory>
#include <array>
namespace Lingmo::ui { class AboutDialog; class Menu; }
namespace Calculator {
enum class Mode { Standard, Scientific, Graph, Programmer };
class CalculatorView;
class CalculatorWindow {
public:
    CalculatorWindow();
    ~CalculatorWindow();
    void show();
    Lingmo::ui::Window& window() { return m_window; }
    const Model& model() const { return m_model; }
    void input(const std::string& text);
    void calculate();
    void clear();
    void backspace();
    void negate();
    void copy();
    void paste();
    void toggleHistory();
    void clearHistory();
    void recallHistory(size_t index);
    void showAbout();
    void showMenu(Lingmo::ui::View* anchor);
    void showModeMenu(Lingmo::ui::View* anchor);
    void setMode(Mode mode);
    Mode mode() const { return m_mode; }
    bool historyVisible() const { return m_historyVisible; }
    void toggleAngleUnit();
    void refreshMenus();
    void recordInteger(const std::string& expression, const std::string& result, unsigned base);
private:
    void refresh();
    std::vector<GlobalMenu::Entry> menuEntries();
    Model m_model;
    Lingmo::ui::Window m_window;
    CalculatorView* m_view = nullptr;
    bool m_historyVisible = false;
    Mode m_mode = Mode::Standard;
    std::array<Lingmo::ui::SizeF, 4> m_modeSizes = {{{400, 620}, {460, 740}, {600, 620}, {520, 780}}};
    std::unique_ptr<Lingmo::ui::AboutDialog> m_about;
    std::unique_ptr<Lingmo::ui::Menu> m_localMenu;
    std::unique_ptr<GlobalMenu> m_globalMenu;
    Lingmo::base::Subscription m_language;
    Lingmo::base::Subscription m_clipboard;
    Lingmo::base::Subscription m_menuAvailability;
    Lingmo::base::WeakPtrFactory<CalculatorWindow> m_weak{this};
};
}
