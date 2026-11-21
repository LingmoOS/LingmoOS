// Copyright 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "keyboard_view.h"
#include "base/i18n.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"
#include <algorithm>
using namespace Lingmo::ui;
using Lingmo::I18n::tr;
namespace Lingmo::Shell {
namespace {
class KeyButton : public Button {
public:
    bool selected=false;
    bool primary=false;
    explicit KeyButton(const std::string& text):Button(text) {
        for(const auto& child:children())if(auto* label=dynamic_cast<Label*>(child.get()))label->setFontSize(18);
    }
    bool acceptsFocus() const override { return false; }
protected:
    void layout() override {
        for(const auto& child:children())if(auto* label=dynamic_cast<Label*>(child.get()))
            label->setFontSize(geometry().width<48?14:18);
        Button::layout();
    }
    void paint(Painter& painter) override {
        Button::paint(painter);
        if(primary && !selected)painter.fillRoundedRect(localRect(),Theme::current().mediumRadius,Theme::current().highlight.withAlpha(0.12));
        if(selected)painter.fillRoundedRect(localRect(),Theme::current().mediumRadius,Theme::current().highlight.withAlpha(0.2));
    }
};
std::string labelFor(const ScreenKey& key) {
    if(key.id=="space")return tr("Space");
    if(key.id=="enter")return tr("Enter");
    return key.label;
}
}
ScreenKeyboardView::ScreenKeyboardView() {
    setName("screen-keyboard");
    m_title=addChild<Label>(tr("Screen Keyboard"));m_title->setWeight(600);m_title->setFontSize(15);
    m_notice=addChild<Label>();m_notice->setFontSize(12);m_notice->setElide(Label::Elide::Right);
    m_modifiers=addChild<Label>();m_modifiers->setName("keyboard-modifiers");m_modifiers->setFontSize(12);m_modifiers->setWeight(600);m_modifiers->setTextAlignment(Label::TextAlignment::Right);
    m_hide=addChild<KeyButton>("⌄");m_hide->setName("keyboard-hide");m_hide->setToolTip(tr("Hide keyboard"));
    m_hide->onClicked=[this]{if(onHide)onHide();};
    const auto rows=m_model.rows();
    for(size_t row=0;row<rows.size();++row) {
        m_buttons.emplace_back();
        for(size_t col=0;col<rows[row].size();++col) {
            const auto& key=rows[row][col];auto* button=addChild<KeyButton>(labelFor(key));
            button->setName("keyboard:"+key.id);
            button->primary=key.id=="enter";
            button->onClicked=[this,row,col]{
                const auto key=m_model.rows()[row][col];
                if(!m_model.toggle(key.id)) {
                    if(onKey && onKey(key.symbol,m_model.control(),m_model.alt())) {
                        m_model.consumed();m_notice->setText(tr("Uses the current input method."));
                    } else m_notice->setText(tr("Select an input field, or check the keyboard layout."));
                }
                refresh();
            };
            m_buttons.back().push_back(button);
        }
    }
    reset();
}
void ScreenKeyboardView::reset(){m_model.reset();m_notice->setText(tr("Uses the current input method."));refresh();}
void ScreenKeyboardView::setNotice(std::string text){m_notice->setText(std::move(text));}
void ScreenKeyboardView::refresh(){
    const auto rows=m_model.rows();
    for(size_t r=0;r<rows.size();++r)for(size_t c=0;c<rows[r].size();++c){
        auto* button=static_cast<KeyButton*>(m_buttons[r][c]);button->setText(labelFor(rows[r][c]));
        const auto& id=rows[r][c].id;
        button->selected=m_model.selected(id);
        if(id=="shift" || id=="ctrl" || id=="alt")button->setToolTip(tr("Applies to the next key only"));
        else if(id=="caps")button->setToolTip(tr("Keep letters uppercase"));
        else if(id=="symbols")button->setToolTip(tr("Switch letters and symbols"));
        else if(id=="backspace")button->setToolTip(tr("Delete previous character"));
        else if(id=="delete")button->setToolTip(tr("Delete next character"));
        button->requestPaint();
    }
    std::string modifiers;
    for(const auto* id:{"shift","caps","ctrl","alt"})if(m_model.selected(id)){
        if(!modifiers.empty())modifiers+=" + ";
        modifiers+=id==std::string("shift")?"Shift":id==std::string("caps")?"Caps":id==std::string("ctrl")?"Ctrl":"Alt";
    }
    m_modifiers->setText(modifiers);
    m_title->setText(tr("Screen Keyboard"));m_hide->setToolTip(tr("Hide keyboard"));requestLayout();requestPaint();
}
void ScreenKeyboardView::layout(){
    const auto rect=localRect();
    m_title->setGeometry({18,10,std::max(0.0,rect.width-290),22});
    m_modifiers->setGeometry({rect.width-270,12,190,22});m_modifiers->setColor(Theme::current().highlight);m_notice->setGeometry({18,34,rect.width-90,20});
    m_notice->setColor(Theme::current().disabledText);m_hide->setGeometry({rect.width-64,10,48,36});
    const double gap=6,margin=12,top=66;
    const double height=std::max(1.0,(rect.height-top-margin-4*gap)/5);
    const auto rows=m_model.rows();
    for(size_t r=0;r<rows.size();++r){
        double units=0;for(const auto& key:rows[r])units+=key.width;
        const double unit=std::max(1.0,(rect.width-2*margin-gap*(rows[r].size()-1))/units);
        double x=margin;
        for(size_t c=0;c<rows[r].size();++c){const double w=unit*rows[r][c].width;m_buttons[r][c]->setGeometry({x,top+r*(height+gap),w,height});x+=w+gap;}
    }
}
void ScreenKeyboardView::paint(Painter& painter){
    const auto rect=localRect();const auto& theme=Theme::current();
    // ShellWindow supplies glass behind the transparent header; keys stay opaque.
    if(!m_glassEnabled)painter.fillRect({0,0,rect.width,60},theme.secondBackground);
    painter.fillRect({0,60,rect.width,std::max(0.0,rect.height-60)},theme.background);
    painter.fillRect({0,60,rect.width,1/painter.scale()},theme.text.withAlpha(0.08));
}
}
