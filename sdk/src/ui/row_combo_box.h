// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "ui/view.h"

namespace Lingmo::ui {

class Icon;
class Label;
class Menu;
class ThemeIcon;

// A list row's choice: a flat combo box with no background until hovered or
// open.  The chevron is smaller than a framed box's and its drawing ends
// where a switch does.  Fits a plain row without changing its height.
class RowComboBox : public View
{
public:
    struct Item
    {
        std::string text;
        // A theme icon name; empty for none.
        std::string icon;
    };

    explicit RowComboBox(std::vector<std::string> items = {});
    ~RowComboBox() override;

    void setItems(std::vector<std::string> items);
    void setItems(std::vector<Item> items);
    int count() const { return int(m_items.size()); }
    int currentIndex() const { return m_current; }
    // Does not call onActivated.
    void setCurrentIndex(int index);
    std::string currentText() const;
    // Shown in place of the current item's text while set.
    void setDisplayText(std::optional<std::string> text);
    // Each menu row drawn in the font family its text names.
    void setFontPreview(bool preview) { m_fontPreview = preview; }

    SizeF sizeHint() const override;
    // How far the box reaches past its row's content edge so the chevron's
    // drawing, not its border, ends where a switch does.
    double trailingOverhang() const;
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    // After the user picks an item.
    std::function<void(int index)> onActivated;

protected:
    void layout() override;
    void paint(Painter& painter) override;
    void enabledChanged() override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const PointerEvent& event) override;
    bool keyPress(const KeyEvent& event) override;

private:
    void update();
    void toggleMenu(bool fromKeyboard);

    std::vector<Item> m_items;
    int m_current = -1;
    std::optional<std::string> m_displayText;
    ThemeIcon* m_icon = nullptr;
    Label* m_label = nullptr;
    Icon* m_chevron = nullptr;
    std::unique_ptr<Menu> m_menu;
    bool m_hovered = false;
    bool m_fontPreview = false;
};

} // namespace Lingmo::ui
