// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/theme.h"
namespace settings {
// A responsive label/visual-choice row. At narrow widths the choices move
// below the label, preserving their hit targets and readable captions.
class ChoiceRow : public Lingmo::ui::View {
public:
    explicit ChoiceRow(std::string text) {
        setLayoutHints({.fillWidth=true});
        m_label=addChild<Lingmo::ui::Label>(std::move(text));m_label->setFontSize(14);m_label->setWeight(500);
        m_options=addChild<Lingmo::ui::Row>();m_options->setSpacing(12);
    }
    Lingmo::ui::Row* options()const{return m_options;}
    Lingmo::ui::SizeF sizeHint()const override{return {0,heightForWidth(600)};}
    double heightForWidth(double width)const override {
        const auto size=m_options->sizeHint();return width<size.width+184?size.height+68:std::max(76.0,size.height+32);
    }
    void layout()override {
        const auto size=m_options->sizeHint();const double w=geometry().width;
        const bool stacked=w<size.width+184;
        m_label->setColor(Lingmo::ui::Theme::current().text);
        m_label->setGeometry({18,stacked?14.0:24.0,stacked?w-36:std::max(0.0,w-size.width-54),24});
        m_options->setGeometry({stacked?18.0:w-size.width-18,stacked?48.0:16.0,std::min(size.width,std::max(0.0,w-36)),size.height});
    }
private: Lingmo::ui::Label* m_label;Lingmo::ui::Row* m_options;
};
// Small, deliberately schematic previews. The three widget modes share a
// recognizable weather card; automatic shows both states, monochrome removes
// all chroma, and full colour retains the sun and the system accent.
class ShellPreview : public Lingmo::ui::View {
public:
    enum class Style { WidgetAutomatic, WidgetMonochrome, WidgetColor, MenuClear, MenuTinted };
    explicit ShellPreview(Style style) : m_style(style) {}
    Lingmo::ui::SizeF sizeHint() const override {
        return isWidget() ? Lingmo::ui::SizeF{84,56} : Lingmo::ui::SizeF{104,64};
    }
    void paint(Lingmo::ui::Painter& p) override {
        using namespace Lingmo::ui;
        const auto r=localRect();
        p.save();
        // The selection host owns the only rounded clip, matching its ring.
        p.clip(r);
        if(isWidget()) {
            if(m_style==Style::WidgetAutomatic) {
                p.save();p.clip({0,0,r.width/2,r.height});paintWidget(p,true);p.restore();
                p.save();p.clip({r.width/2,0,r.width/2,r.height});paintWidget(p,false);p.restore();
                p.fillRect({r.width/2-.5,0,1,r.height},Theme::current().text.withAlpha(.18));
            } else {
                paintWidget(p,m_style==Style::WidgetColor);
            }
        } else {
            paintMenu(p);
        }
        p.restore();
    }
private:
    bool isWidget() const { return m_style<=Style::WidgetColor; }
    void paintWidget(Lingmo::ui::Painter& p,bool color) const {
        using namespace Lingmo::ui;
        const auto& theme=Theme::current();const auto r=localRect();
        const bool dark=theme.darkMode;
        const Color background=color?theme.highlight:(dark?Color::rgb(0x50545B):Color::rgb(0xDCE0E4));
        const Color ink=color?theme.highlightedText:(dark?Color::rgb(0xF1F2F4):Color::rgb(0x59616B));
        p.fillRect(r,background);
        // Sun, with four short rays, partly behind a compact cloud.
        const Color sun=color?Color::rgb(0xFFD66C):ink.withAlpha(.7);
        const double x=r.width*.24,y=r.height*.31;
        p.fillEllipse({x-7,y-7,14,14},sun);
        p.drawLine({x,y-11},{x,y-9},sun,1.5);
        p.drawLine({x-11,y},{x-9,y},sun,1.5);
        p.drawLine({x+7,y-7},{x+9,y-9},sun,1.5);
        p.drawLine({x-7,y-7},{x-9,y-9},sun,1.5);
        p.fillEllipse({x+1,y+1,14,12},ink);
        p.fillEllipse({x+10,y-3,17,18},ink);
        p.fillRoundedRect({x-1,y+8,32,9},4,ink);
        // A small thermometer column, separate from the weather symbol.
        p.fillRoundedRect({r.width-21,11,5,19},2.5,ink.withAlpha(.42));
        p.fillRoundedRect({r.width-21,19,5,11},2.5,ink);
        p.fillEllipse({r.width-23,27,9,9},ink);
        // Location and forecast rhythm; no tiny invented text or gloss.
        p.fillRoundedRect({12,r.height-13,25,3},1.5,ink.withAlpha(.9));
        p.fillRoundedRect({43,r.height-13,9,3},1.5,ink.withAlpha(.45));
        p.fillRoundedRect({56,r.height-13,9,3},1.5,ink.withAlpha(.45));
    }
    void paintMenu(Lingmo::ui::Painter& p) const {
        using namespace Lingmo::ui;
        const auto r=localRect();const bool dark=Theme::current().darkMode;
        // Identical geometric wallpaper in both thumbnails. Its edges cross
        // the bar, making the actual opacity difference easy to compare.
        p.fillRect(r,dark?Color::rgb(0x244D58):Color::rgb(0xB8DCD8));
        p.fillRect({r.width*.36,0,r.width*.32,r.height},dark?Color::rgb(0x405B88):Color::rgb(0xAFBFDF));
        p.fillRect({r.width*.68,0,r.width*.32,r.height},dark?Color::rgb(0x78564C):Color::rgb(0xE4BDA5));
        p.fillRoundedRect({18,36,r.width-36,20},4,dark?Color::rgb(0x30353C):Color::rgb(0xF4F5F6));
        p.fillRoundedRect({25,42,20,2},1,Color::rgb(0x959CA5));
        p.fillRoundedRect({25,47,37,2},1,Color::rgb(0xBAC0C8));
        const bool tinted=m_style==Style::MenuTinted;
        const Color surface=dark?Color::rgb(0x22262D):Color::rgb(0xFFFFFF);
        p.fillRect({0,0,r.width,24},surface.withAlpha(tinted?.90:.22));
        const Color ink=dark?Color::rgb(0xF3F4F6):Color::rgb(0x363C47);
        p.fillRect({0,23,r.width,1},ink.withAlpha(tinted?.16:.07));
        // App mark, two menus, connection and battery glyphs.
        p.fillRoundedRect({7,8,8,8},2,ink);
        p.fillRoundedRect({21,10,14,3},1,ink.withAlpha(.88));
        p.fillRoundedRect({40,10,11,3},1,ink.withAlpha(.66));
        for(int i=0;i<3;++i)p.fillRoundedRect({r.width-29+i*3.0,14-i*2.0,2,3+i*2.0},.5,ink);
        p.drawRoundedRect({r.width-16,9,10,6},{1.5,0,Corners::Circular},{0,0,0,0},1,ink);
        p.fillRect({r.width-14,11,5,2},ink);
        p.fillRect({r.width-5,11,1,2},ink);
    }
    Style m_style;
};
} // namespace settings
