// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/color_provider.h"
#include "ui/combo_box.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/text_field.h"
#include "ui/theme.h"

namespace Lingmo::ui {

// Three medium icons and a large spacing wide, as tall as a text field.
static constexpr double kMinimumWidth = 3 * 32 + 12;
static constexpr double kHeight = 35;
static constexpr double kChevron = 16;
static constexpr double kIcon = 22;
// Half the small spacing between the box and its list.
static constexpr double kMenuGap = 3;

// An editable box's field, as FishUI's contentItem: a bare text input.
class ComboBox::EditField : public TextField
{
public:
    explicit EditField(ComboBox* box)
        : m_box(box)
    {
    }

protected:
    bool keyPress(const KeyEvent& event) override
    {
        // As a single-line TextInput, the field leaves these to the box,
        // which opens its list on them.
        if (event.modifiers == NoModifier
            && (event.key == key::Up || event.key == key::Down || event.key == key::F4)) {
            m_box->openMenu(true);
            return true;
        }
        m_box->m_allowComplete = event.key != key::BackSpace && event.key != key::Delete;
        return TextField::keyPress(event);
    }

    void focusIn(FocusReason reason) override
    {
        TextField::focusIn(reason);
        m_box->requestPaint();
    }

    void focusOut() override
    {
        TextField::focusOut();
        m_box->requestPaint();
        if (m_box->onFocusLost)
            m_box->onFocusLost();
    }

private:
    ComboBox* m_box;
};

// The part of an editable box over its arrow, above the field, that opens
// the list.
class ComboBox::ArrowArea : public View
{
public:
    explicit ArrowArea(ComboBox* box)
        : m_box(box)
    {
    }

    bool acceptsPointer() const override { return true; }

protected:
    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        m_box->m_field->setFocus(FocusReason::Mouse);
        m_box->m_pressed = true;
        m_box->requestPaint();
        m_box->openMenu(false);
    }

    void pointerRelease(const PointerEvent&) override
    {
        m_box->m_pressed = false;
        m_box->requestPaint();
    }

private:
    ComboBox* m_box;
};

ComboBox::ComboBox(std::vector<std::string> items)
    : m_label(addChild<Label>())
    , m_chevron(addChild<Icon>("chevron-down", kChevron))
    , m_icon(addChild<ThemeIcon>(std::string(), kIcon))
{
    m_label->setVerticallyCentered(true);
    m_icon->setVisible(false);
    setItems(std::move(items));
}

ComboBox::~ComboBox() = default;

void ComboBox::setItems(std::vector<std::string> items)
{
    // An open list shows the old items, and its rows choose by their index.
    if (isMenuOpen() && items != m_items)
        m_menu->close();
    m_items = std::move(items);
    m_current = m_items.empty() ? -1 : std::clamp(m_current, 0, int(m_items.size()) - 1);
    m_label->setText(currentText());
    updateIcon();
    requestLayout();
}

void ComboBox::setCurrentIndex(int index)
{
    if (index < (m_field ? -1 : 0) || index >= int(m_items.size()) || index == m_current)
        return;
    if (m_field)
        setEditText(index >= 0 ? m_items[size_t(index)] : std::string());
    m_current = index;
    m_label->setText(currentText());
    updateIcon();
}

void ComboBox::setEditable(bool editable)
{
    if (editable == isEditable())
        return;
    if (editable) {
        m_field = addChild<EditField>(this);
        m_field->setFrameVisible(false);
        m_field->setFadesWhenDisabled(false);
        m_field->setText(currentText());
        m_field->onTextEdited = [this] { edited(); };
        m_field->onTextChanged = [this] {
            if (onEditTextChanged)
                onEditTextChanged();
        };
        m_field->onAccepted = [this] { accept(); };
        // Above the field, as FishUI's MouseArea at z 1.
        m_arrow = addChild<ArrowArea>(this);
    } else {
        takeChild(m_arrow);
        takeChild(m_field);
        m_arrow = nullptr;
        m_field = nullptr;
    }
    m_label->setVisible(!editable);
    requestLayout();
    requestPaint();
}

std::string ComboBox::editText() const
{
    return m_field ? m_field->text() : std::string();
}

void ComboBox::setEditText(std::string text)
{
    if (m_field)
        m_field->setText(std::move(text));
}

std::string ComboBox::completed(const std::string& text) const
{
    const icu::UnicodeString typed = icu::UnicodeString::fromUTF8(text);
    icu::UnicodeString folded = typed;
    folded.foldCase();
    icu::UnicodeString match;
    bool found = false;
    for (const std::string& item : m_items) {
        const icu::UnicodeString candidate = icu::UnicodeString::fromUTF8(item);
        icu::UnicodeString start = candidate.tempSubString(0, typed.length());
        if (candidate.length() < typed.length() || start.foldCase() != folded)
            continue;
        // The first or the shortest match.
        if (!found || candidate.length() < match.length()) {
            match = candidate;
            found = true;
        }
    }
    if (!found)
        return text;
    std::string result;
    (typed + match.tempSubString(typed.length())).toUTF8String(result);
    return result;
}

int ComboBox::find(const std::string& text) const
{
    const icu::UnicodeString wanted = icu::UnicodeString::fromUTF8(text);
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (icu::UnicodeString::fromUTF8(m_items[i]).caseCompare(wanted, U_FOLD_CASE_DEFAULT) == 0)
            return int(i);
    }
    return -1;
}

void ComboBox::edited()
{
    const std::string text = m_field->text();
    if (!m_allowComplete || text.empty())
        return;
    const std::string completion = completed(text);
    if (completion.size() <= text.size())
        return;
    // The completed part selected, the cursor after what was typed.
    m_field->setText(completion);
    m_field->select(completion.size(), text.size());
}

void ComboBox::accept()
{
    const int index = find(m_field->text());
    if (index >= 0) {
        setCurrentIndex(index);
        // The text now that of the item, nothing selected.
        m_field->setCursorPosition(m_field->text().size());
    }
    if (onAccepted)
        onAccepted();
}

void ComboBox::setItemIcons(std::vector<std::string> icons)
{
    m_icons = std::move(icons);
    updateIcon();
}

void ComboBox::updateIcon()
{
    const std::string icon = m_current >= 0 && size_t(m_current) < m_icons.size() ? m_icons[size_t(m_current)]
                                                                                   : std::string();
    if (icon.empty() != !m_icon->isVisible())
        requestLayout();
    m_icon->setIconName(icon);
    m_icon->setVisible(!icon.empty());
}

bool ComboBox::isMenuOpen() const
{
    return m_menu && m_menu->isOpen();
}

std::string ComboBox::currentText() const
{
    return m_current >= 0 ? m_items[size_t(m_current)] : std::string();
}

SizeF ComboBox::sizeHint() const
{
    const Theme& theme = Theme::current();
    double widest = 0;
    for (const std::string& item : m_items) {
        Label measure(item);
        measure.setTextLayoutMode(textLayoutMode());
        widest = std::max(widest, measure.sizeHint().width);
    }
    const double padding = theme.largeSpacing;
    return {std::max(kMinimumWidth, padding + widest + theme.mediumSpacing + kChevron + padding),
            kHeight};
}

void ComboBox::layout()
{
    const Theme& theme = Theme::current();
    const double padding = theme.largeSpacing;
    const RectF bounds = localRect();
    const bool enabled = isEnabled();
    // FishUI's indicator: kChevron high at a whole logical y.
    m_chevron->setGeometry({bounds.width - padding - kChevron, std::round((bounds.height - kChevron) / 2), kChevron,
                            kChevron});
    // The current item's icon, beside the text.
    const double iconSpace = m_icon->isVisible() ? kIcon + theme.mediumSpacing : 0;
    m_icon->setGeometry({padding, (bounds.height - kIcon) / 2, kIcon, kIcon});
    m_label->setGeometry({padding + iconSpace, 0,
                          std::max(0.0, bounds.width - 2 * padding - kChevron - theme.mediumSpacing - iconSpace),
                          bounds.height});
    if (m_field) {
        // The contentItem fills the box inside its paddings, running under
        // the arrow; its own padding is 0 on the left and the small spacing
        // on the right.
        m_field->setGeometry({padding + iconSpace, 0, std::max(0.0, bounds.width - 2 * padding - iconSpace),
                              bounds.height});
        m_field->setTextMargins(0, theme.smallSpacing);
        m_field->setTextColor(enabled ? theme.text : theme.disabledText);
        m_field->setOpacity(enabled ? 1 : 0.6);
        const double arrowWidth = kChevron + 2 * padding;
        m_arrow->setGeometry({bounds.width - arrowWidth, 0, arrowWidth, bounds.height});
    }
    // A disabled combo box greys its text and arrow; the label
    // fades on its own, the arrow as far.
    m_label->setColor(enabled ? theme.text : theme.disabledText);
    m_chevron->setColor(enabled ? theme.text : theme.disabledText);
    m_chevron->setOpacity(enabled ? 1 : 0.6);
    m_icon->setOpacity(enabled ? 1 : 0.6);
}

void ComboBox::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const bool enabled = isEnabled();
    // Pressed only, as FishUI's (no hover colour); an editable box keeps
    // its colour, the field taking the presses.
    const Color background = color(enabled && !m_field && m_pressed ? ColorId::ComboBoxBackgroundPressed
                                                                     : ColorId::ComboBoxBackground);
    const bool open = m_menu && m_menu->isOpen();
    const bool focused = m_field ? m_field->hasFocus() : hasFocus();
    painter.drawRoundedRect(localRect(), {theme.smallRadius, 0, themeCorners()}, background, 1,
                            enabled && (focused || open) ? color(ColorId::ComboBoxFocusBorder) : background);
}

void ComboBox::enabledChanged()
{
    requestLayout();
    requestPaint();
}

void ComboBox::openMenu(bool fromKeyboard)
{
    if (isMenuOpen()) {
        m_menu->close();
        return;
    }
    if (m_items.empty() || !window())
        return;
    m_menu = std::make_unique<Menu>();
    m_menu->setDropDown(true);
    for (size_t i = 0; i < m_items.size(); ++i) {
        const size_t index = m_menu->addItem(m_items[i], [this, i] {
            setCurrentIndex(int(i));
            if (onActivated)
                onActivated(int(i));
        });
        m_menu->setItemChecked(index, int(i) == m_current);
        if (i < m_icons.size())
            m_menu->setItemIcon(index, m_icons[i]);
        if (m_fontPreview)
            m_menu->setItemFontFamily(index, m_items[i]);
    }
    m_menu->setMinimumWidth(geometry().width);
    m_menu->onClosed = [this] { requestPaint(); };
    m_menu->popupFor(this, kMenuGap, std::nullopt, fromKeyboard);
    requestPaint();
}

void ComboBox::pointerEnter()
{
    m_hovered = true;
}

void ComboBox::pointerLeave()
{
    m_hovered = false;
}

void ComboBox::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left || m_field)
        return;
    m_pressed = true;
    requestPaint();
    openMenu(false);
}

void ComboBox::pointerRelease(const PointerEvent&)
{
    m_pressed = false;
    requestPaint();
}

bool ComboBox::keyPress(const KeyEvent& event)
{
    // An editable box's field types these; Return accepts its text.
    if (event.modifiers != NoModifier || m_field)
        return false;
    switch (event.key) {
    // These open the menu rather than stepping the current item.
    case key::Up:
    case key::Down:
    case key::F4:
    case key::Space:
    case key::Return:
    case key::KeypadEnter:
        openMenu(true);
        return true;
    default:
        return false;
    }
}

} // namespace Lingmo::ui
