// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <optional>
#include <string>

#include "ui/view.h"

namespace Lingmo::ui {

class Label;

// A control with an indicator and an optional label beside it that the user
// turns on and off: the base of Switch, CheckBox and RadioButton.
class ToggleControl : public View
{
public:
    ~ToggleControl() override;

    void setText(std::string text);
    const std::string& text() const;

    // Per side; the theme's unless set.
    void setPadding(Margins padding);

    bool isChecked() const { return m_checked; }
    // Changes the state without calling onToggled.
    void setChecked(bool checked);

    SizeF sizeHint() const override;
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    // After the user changed the state.
    std::function<void(bool checked)> onToggled;

protected:
    // A switch's padding is horizontal only without text, and its indicator
    // then centres; the others keep it on all sides and at the left.
    enum class Kind {
        Switch,
        Button,
    };

    ToggleControl(std::string text, SizeF indicator, double padding, Kind kind = Kind::Button);

    bool isPressed() const { return m_pressed; }
    bool isHovered() const { return m_hovered; }
    RectF indicatorRect() const;
    Margins padding() const;
    // The user's action; a radio button only turns on.
    virtual void toggle();
    virtual void checkedChanged(bool animated);
    virtual void paintIndicator(Painter& painter, const RectF& rect) = 0;

    void layout() override;
    void paint(Painter& painter) override;
    void enabledChanged() override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const PointerEvent& event) override;
    void pointerRelease(const PointerEvent& event) override;
    bool keyPress(const KeyEvent& event) override;
    bool keyRelease(const KeyEvent& event) override;

private:
    Label* m_label;
    SizeF m_indicator;
    double m_padding;
    Kind m_kind;
    std::optional<Margins> m_customPadding;
    bool m_checked = false;
    bool m_pressed = false;
    bool m_hovered = false;
};

} // namespace Lingmo::ui
