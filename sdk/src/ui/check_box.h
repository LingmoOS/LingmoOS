// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ui/toggle_control.h"

namespace Lingmo::ui {

class Icon;
class RadioButton;

// A check box: a rounded square, filled with the accent colour and
// checked when on.
class CheckBox : public ToggleControl
{
public:
    explicit CheckBox(std::string text = {});

protected:
    void layout() override;
    void paintIndicator(Painter& painter, const RectF& rect) override;
    void checkedChanged(bool animated) override;

private:
    Icon* m_check;
};

// Radio buttons that turn each other off: at most one is on.
class ButtonGroup
{
public:
    ~ButtonGroup();

    void add(RadioButton* button);
    void remove(RadioButton* button);
    RadioButton* checkedButton() const;
    int checkedIndex() const;

    // After the user picked a button.
    std::function<void(int index)> onChanged;

private:
    friend class RadioButton;

    void checked(RadioButton* button, bool byUser);

    std::vector<RadioButton*> m_buttons;
};

// A radio button: a round indicator with a dot when on. Picking one
// turns the others of its group off.
class RadioButton : public ToggleControl
{
public:
    explicit RadioButton(std::string text = {});
    ~RadioButton() override;

    void setGroup(ButtonGroup* group);

protected:
    void toggle() override;
    void checkedChanged(bool animated) override;
    void paintIndicator(Painter& painter, const RectF& rect) override;

private:
    friend class ButtonGroup;

    ButtonGroup* m_group = nullptr;
};

} // namespace Lingmo::ui
