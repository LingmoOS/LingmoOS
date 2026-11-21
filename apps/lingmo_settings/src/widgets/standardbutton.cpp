// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/colors.h"
#include "widgets/standardbutton.h"

#include <algorithm>

#include "ui/color_provider.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;

namespace settings {

StandardButton::StandardButton(std::string text)
{
    m_label = addChild<Label>(std::move(text));
    m_label->setTextAlignment(Label::TextAlignment::Center);
    m_label->setVerticallyCentered(true);
    setLayoutHints({.fillWidth = true});
}

void StandardButton::setText(std::string text)
{
    m_label->setText(std::move(text));
    requestLayout();
}

void StandardButton::setBackgroundColor(std::optional<Color> color)
{
    m_background = color;
    requestPaint();
}

Color StandardButton::backgroundColor() const
{
    return m_background.value_or(color(SettingsColor::StandardButtonBackground));
}

SizeF StandardButton::sizeHint() const
{
    const SizeF label = m_label->sizeHint();
    return {label.width + 2 * metrics::cardHorizontalPadding,
            std::max(metrics::buttonHeight, label.height + 2 * Theme::current().smallSpacing)};
}

void StandardButton::layout()
{
    m_label->setGeometry(localRect().shrunk({metrics::cardHorizontalPadding, 0, metrics::cardHorizontalPadding, 0}));
}

void StandardButton::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const Color background = backgroundColor();
    Color color = background;
    if (isEnabled() && m_pressed)
        color = theme.darkMode ? background.lighter(1.1) : background.darker(1.2);
    painter.drawRoundedRect(localRect(), {theme.mediumRadius, 0, themeCorners()}, color);
}

void StandardButton::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left)
        return;
    m_pressed = true;
    requestPaint();
}

void StandardButton::pointerRelease(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (localRect().contains(event.position)) {
        setFocus(FocusReason::Mouse);
        if (onClicked)
            onClicked();
    }
}

bool StandardButton::keyPress(const KeyEvent& event)
{
    if (event.key == key::Space || event.key == key::Return || event.key == key::KeypadEnter) {
        if (onClicked)
            onClicked();
        return true;
    }
    return false;
}

} // namespace settings
