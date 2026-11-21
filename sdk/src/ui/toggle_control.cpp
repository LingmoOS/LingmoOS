// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/toggle_control.h"

#include <algorithm>
#include <utility>

#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace Lingmo::ui {

ToggleControl::ToggleControl(std::string text, SizeF indicator, double padding, Kind kind)
    : m_label(addChild<Label>(std::move(text)))
    , m_indicator(indicator)
    , m_padding(padding)
    , m_kind(kind)
{
    m_label->setVerticallyCentered(true);
    // A check box's and a radio button's label is a plain Text.
    m_label->setFadesWhenDisabled(kind == Kind::Switch);
}

ToggleControl::~ToggleControl() = default;

void ToggleControl::setText(std::string text)
{
    m_label->setText(std::move(text));
}

const std::string& ToggleControl::text() const
{
    return m_label->text();
}

void ToggleControl::setChecked(bool checked)
{
    if (checked == m_checked)
        return;
    m_checked = checked;
    checkedChanged(false);
    requestPaint();
}

void ToggleControl::setPadding(Margins padding)
{
    if (padding == m_customPadding)
        return;
    m_customPadding = padding;
    requestLayout();
}

Margins ToggleControl::padding() const
{
    if (m_customPadding)
        return *m_customPadding;
    const double vertical = m_kind == Kind::Switch && text().empty() ? 0 : m_padding;
    return {m_padding, vertical, m_padding, vertical};
}

SizeF ToggleControl::sizeHint() const
{
    const Margins pad = padding();
    if (text().empty())
        return {m_indicator.width + pad.left + pad.right, m_indicator.height + pad.top + pad.bottom};
    const SizeF label = m_label->sizeHint();
    const double spacing = Theme::current().mediumSpacing;
    return {pad.left + m_indicator.width + spacing + label.width + pad.right,
            std::max(m_indicator.height, label.height) + pad.top + pad.bottom};
}

// Where the indicator goes in the padded control.
RectF ToggleControl::indicatorRect() const
{
    const RectF bounds = localRect();
    const Margins pad = padding();
    const double available = bounds.width - pad.left - pad.right;
    const double x = m_kind == Kind::Switch && text().empty() ? pad.left + (available - m_indicator.width) / 2
                                                              : pad.left;
    const double y = pad.top + (bounds.height - pad.top - pad.bottom - m_indicator.height) / 2;
    return {x, y, m_indicator.width, m_indicator.height};
}

void ToggleControl::enabledChanged()
{
    requestLayout();
    requestPaint();
}

void ToggleControl::layout()
{
    // Disabled, the label takes the disabled text colour; a switch's also
    // fades with the whole switch.
    const bool enabled = isEnabled();
    m_label->setColor(enabled ? std::nullopt : std::optional<Color>(Theme::current().disabledText));
    m_label->setOpacity(enabled || m_kind != Kind::Switch ? 1 : 0.5);
    const Margins pad = padding();
    const double left = pad.left + m_indicator.width + Theme::current().mediumSpacing;
    m_label->setVisible(!text().empty());
    m_label->setGeometry({left, pad.top, std::max(0.0, geometry().width - left - pad.right),
                          std::max(0.0, geometry().height - pad.top - pad.bottom)});
}

void ToggleControl::paint(Painter& painter)
{
    paintIndicator(painter, indicatorRect());
}

void ToggleControl::toggle()
{
    m_checked = !m_checked;
    checkedChanged(true);
    requestPaint();
    if (onToggled)
        onToggled(m_checked);
}

void ToggleControl::checkedChanged(bool)
{
}

void ToggleControl::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void ToggleControl::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void ToggleControl::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left)
        return;
    m_pressed = true;
    requestPaint();
}

void ToggleControl::pointerRelease(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (localRect().contains(event.position))
        toggle();
}

bool ToggleControl::keyPress(const KeyEvent& event)
{
    if (event.key != key::Space || event.modifiers != NoModifier)
        return false;
    m_pressed = true;
    requestPaint();
    return true;
}

bool ToggleControl::keyRelease(const KeyEvent& event)
{
    if (event.key != key::Space || !m_pressed)
        return false;
    m_pressed = false;
    toggle();
    return true;
}

} // namespace Lingmo::ui
