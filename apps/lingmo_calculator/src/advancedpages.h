// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "calculator.h"
#include "programmer.h"
#include "ui/view.h"
#include <functional>
namespace Lingmo::ui { class Button; class Label; class TextField; }
namespace Calculator {
std::string errorText(Error error);
class GraphCanvas;
class GraphPage : public Lingmo::ui::View {
public:
    GraphPage();
    void plot();
    void clear();
    void copy();
    void paste();
    void retranslate();
    bool canCalculate() const;
    bool hasText() const;
    std::function<void()> onChanged;
protected:
    void layout() override;
private:
    void updateStatus();
    Lingmo::ui::TextField* m_field;
    Lingmo::ui::Button *m_plot, *m_angle, *m_plus, *m_minus, *m_reset;
    Lingmo::ui::Label *m_status, *m_range;
    GraphCanvas* m_canvas;
    AngleUnit m_angleUnit = AngleUnit::Radians;
};
class ProgrammerPage : public Lingmo::ui::View {
public:
    ProgrammerPage();
    static constexpr double KeypadTop = 304;
    void calculate();
    void clear();
    void copy();
    void paste();
    void retranslate();
    bool canCalculate() const;
    bool hasText() const;
    void setBase(unsigned base);
    void restore(const std::string& result, unsigned base);
    std::function<void(const std::string&, const std::string&, unsigned)> onCalculated;
    std::function<void()> onChanged;
protected:
    void layout() override;
private:
    void update();
    void append(const std::string& text);
    Lingmo::ui::TextField* m_field;
    Lingmo::ui::Label *m_result, *m_conversions, *m_status, *m_hint;
    Lingmo::ui::Button *m_clear, *m_calculate;
    std::vector<Lingmo::ui::Button*> m_bases;
    std::vector<Lingmo::ui::Button*> m_keys;
    std::vector<std::string> m_keyTokens;
    unsigned m_base = 10;
    IntegerResult m_value;
    bool m_completed = false;
};
}
