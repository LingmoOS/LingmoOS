// Copyright 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "keyboard_model.h"
#include "ui/view.h"
#include <functional>
namespace Lingmo::ui { class Button; class Label; }
namespace Lingmo::Shell {
class ScreenKeyboardView : public ui::View {
public:
    ScreenKeyboardView();
    std::function<bool(uint32_t,bool,bool)> onKey;
    std::function<void()> onHide;
    void reset();
    void setGlassEnabled(bool enabled) { m_glassEnabled=enabled; requestPaint(); }
    void setNotice(std::string text);
protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
private:
    void refresh();
    ScreenKeyboardModel m_model;
    bool m_glassEnabled=false;
    ui::Label *m_title, *m_notice, *m_modifiers;
    ui::Button* m_hide;
    std::vector<std::vector<ui::Button*>> m_buttons;
};
}
