// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ui/view.h"

namespace Lingmo::ui {

class Icon;
class Label;
class Menu;
class TextField;
class ThemeIcon;

// A combo box: the current item in a field-like box with a chevron,
// opening a menu of the items with the current one checked.
class ComboBox : public View
{
public:
    explicit ComboBox(std::vector<std::string> items = {});
    ~ComboBox() override;

    void setItems(std::vector<std::string> items);
    const std::vector<std::string>& items() const { return m_items; }
    int currentIndex() const { return m_current; }
    // Does not call onActivated. An editable box also takes -1, no item,
    // and shows the current item's text in its field.
    void setCurrentIndex(int index);
    std::string currentText() const;
    // Theme icon names (or paths) beside the items, in the box and the list,
    // as an iconRole; one for each item.
    void setItemIcons(std::vector<std::string> icons);
    // Each row of the list in the font family its text names, as fontPreview.
    void setFontPreview(bool preview) { m_fontPreview = preview; }
    bool isMenuOpen() const;

    // A field to type into in place of the current item: the arrow opens the
    // list, typing completes the text from the items, Return takes an item
    // the text names (ignoring case) as the current one.
    void setEditable(bool editable);
    bool isEditable() const { return m_field != nullptr; }
    // The field's text; empty unless editable.
    std::string editText() const;
    void setEditText(std::string text);
    // The field while editable, as the contentItem.
    TextField* field() const { return m_field; }

    SizeF sizeHint() const override;
    bool acceptsPointer() const override { return true; }
    // An editable box's field takes the focus.
    bool acceptsFocus() const override { return !m_field; }

    // After the user picked an item.
    std::function<void(int index)> onActivated;
    // Return or Enter in an editable box's field, as accepted.
    std::function<void()> onAccepted;
    // The text of an editable box's field changed, from the user or code.
    std::function<void()> onEditTextChanged;
    // An editable box's field lost the focus.
    std::function<void()> onFocusLost;

protected:
    void layout() override;
    void paint(Painter& painter) override;
    void enabledChanged() override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const PointerEvent& event) override;
    void pointerRelease(const PointerEvent& event) override;
    bool keyPress(const KeyEvent& event) override;

private:
    class EditField;
    class ArrowArea;

    void openMenu(bool fromKeyboard);
    void updateIcon();
    // The completion of a typed text, as QQuickComboBox's: the shortest item
    // it starts, ignoring case, with the typed part kept as typed.
    std::string completed(const std::string& text) const;
    int find(const std::string& text) const;
    void edited();
    void accept();

    std::vector<std::string> m_items;
    int m_current = -1;
    Label* m_label;
    Icon* m_chevron;
    ThemeIcon* m_icon;
    std::vector<std::string> m_icons;
    bool m_fontPreview = false;
    std::unique_ptr<Menu> m_menu;
    TextField* m_field = nullptr;
    ArrowArea* m_arrow = nullptr;
    bool m_allowComplete = true;
    bool m_hovered = false;
    bool m_pressed = false;
};

} // namespace Lingmo::ui
