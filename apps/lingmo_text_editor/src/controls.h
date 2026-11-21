// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTROLS_H
#define CONTROLS_H

#include "ui/view.h"

#include <functional>
#include <optional>
#include <string>

namespace Lingmo::ui {
class Icon;
class Label;
class Painter;
}

// A round plate inset a tenth of its size, tinted
// with the text colour when hovered or pressed, around a glyph or a label.
class RoundButton : public Lingmo::ui::View
{
public:
    explicit RoundButton(double size = 32);

    void setButtonSize(double size);
    double buttonSize() const { return m_size; }
    // A glyph of the icon set, drawn at iconSize in the middle.
    void setGlyph(std::string name, double iconSize = 16);
    void setIconColor(std::optional<Lingmo::ui::Color> color);
    // A label in the middle instead of a glyph, which the button is as wide
    // as, plus `padding` on each side, unless laid out otherwise.
    Lingmo::ui::Label *setLabel(std::string text, double padding = 8);
    Lingmo::ui::Label *label() const { return m_label; }
    void setBackgroundColor(Lingmo::ui::Color color);
    // The pressed plate while set, as while the button's menu is open.
    void setChecked(bool checked);

    Lingmo::ui::SizeF sizeHint() const override;
    bool acceptsPointer() const override { return true; }

    // The click and where it was, in window coordinates.
    std::function<void()> onClicked;
    std::function<void(Lingmo::ui::PointF windowPosition)> onClickedAt;

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter &painter) override;
    void enabledChanged() override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const Lingmo::ui::PointerEvent &event) override;
    void pointerRelease(const Lingmo::ui::PointerEvent &event) override;

private:
    double iconSize() const;

    double m_size;
    double m_iconSize = -1;
    double m_padding = 8;
    Lingmo::ui::Icon *m_icon = nullptr;
    Lingmo::ui::Label *m_label = nullptr;
    std::optional<Lingmo::ui::Color> m_iconColor;
    Lingmo::ui::Color m_background{0, 0, 0, 0};
    bool m_checked = false;
    bool m_hovered = false;
    bool m_pressed = false;
};

// A text button, plain or flat (the accent's tint, its text in the accent).
class TextButton : public Lingmo::ui::View
{
public:
    explicit TextButton(std::string text = {}, bool flat = false);

    void setText(std::string text);
    void setFlat(bool flat);

    Lingmo::ui::SizeF sizeHint() const override;
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter &painter) override;
    void enabledChanged() override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const Lingmo::ui::PointerEvent &event) override;
    void pointerRelease(const Lingmo::ui::PointerEvent &event) override;
    bool keyPress(const Lingmo::ui::KeyEvent &event) override;

private:
    Lingmo::ui::Label *m_label;
    bool m_flat;
    bool m_hovered = false;
    bool m_pressed = false;
};

// The menus' surface colour, rounded, a hairline edge
// and a soft shadow under the plate; its content inset by `padding`.
class FloatingCard : public Lingmo::ui::View
{
public:
    static constexpr double padding = 6;

    void setRadius(double radius) { m_radius = radius; }
    Lingmo::ui::RectF visualRect() const override;
    bool acceptsPointer() const override { return true; }

protected:
    void paint(Lingmo::ui::Painter &painter) override;

private:
    double m_radius = 20;
};

// One device pixel tall, snapped as a RoundedRectangle of height 1 / dpr.
void drawHairline(Lingmo::ui::Painter &painter, double x, double y, double width, Lingmo::ui::Color color);

#endif
