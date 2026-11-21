// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controls.h"
#include "colors.h"

#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <cmath>

using namespace Lingmo::ui;

RoundButton::RoundButton(double size)
    : m_size(size)
{
}

void RoundButton::setButtonSize(double size)
{
    m_size = size;
    requestLayout();
    requestPaint();
}

void RoundButton::setGlyph(std::string name, double iconSize)
{
    if (!m_icon)
        m_icon = addChild<Icon>(std::move(name), iconSize);
    else
        m_icon->setIconName(std::move(name));
    m_icon->setSize(iconSize);
    m_iconSize = iconSize;
    requestLayout();
}

void RoundButton::setIconColor(std::optional<Color> color)
{
    m_iconColor = color;
    requestLayout();
}

Label *RoundButton::setLabel(std::string text, double padding)
{
    if (!m_label)
        m_label = addChild<Label>();
    m_label->setText(std::move(text));
    m_padding = padding;
    requestLayout();
    return m_label;
}

void RoundButton::setBackgroundColor(Color color)
{
    m_background = color;
    requestPaint();
}

void RoundButton::setChecked(bool checked)
{
    if (m_checked == checked)
        return;
    m_checked = checked;
    requestPaint();
}

double RoundButton::iconSize() const
{
    return m_iconSize > 0 ? m_iconSize : std::round(m_size);
}

SizeF RoundButton::sizeHint() const
{
    if (m_label)
        return {std::ceil(m_label->sizeHint().width) + 2 * m_padding, m_size};
    return {m_size, m_size};
}

void RoundButton::layout()
{
    const double width = geometry().width;
    const double height = geometry().height;
    const double icon = iconSize();
    const double x = std::round((width - icon) / 2);
    const double y = std::round((height - icon) / 2);
    if (m_icon) {
        m_icon->setGeometry({x, y, icon, icon});
        m_icon->setColor(m_iconColor ? m_iconColor : std::optional<Color>(Theme::current().text));
        m_icon->setOpacity(isEnabled() ? 1 : 0.35);
    }
    if (m_label) {
        const SizeF size = m_label->sizeHint();
        m_label->setGeometry({std::round(width / 2) - std::round(size.width / 2), std::round(height / 2) - std::round(size.height / 2),
                              size.width, size.height});
    }
}

void RoundButton::paint(Painter &painter)
{
    Color fill = m_background;
    if (isEnabled()) {
        if (m_pressed || m_checked)
            fill = color(ColorId::RoundButtonPressed);
        else if (m_hovered)
            fill = color(ColorId::RoundButtonHovered);
    } else {
        fill = {0, 0, 0, 0};
    }
    if (fill.a == 0)
        return;
    // Whole device pixels of the icon's parity, so the snapped plate and the
    // snapped icon share a centre wherever the button lands.
    const double ratio = painter.scale();
    const double icon = iconSize();
    const auto plateLength = [&](double length) {
        double pixels = std::round(length * ratio);
        if (int64_t(pixels - std::round(icon * ratio)) % 2 != 0)
            pixels -= 1;
        return pixels / ratio;
    };
    const double margin = std::round(m_size * 0.1);
    const double width = plateLength(geometry().width - margin * 2);
    const double height = plateLength(geometry().height - margin * 2);
    const double iconX = std::round((geometry().width - icon) / 2);
    const double iconY = std::round((geometry().height - icon) / 2);
    const RectF plate{iconX + (icon - width) / 2, iconY + (icon - height) / 2, width, height};
    painter.drawRoundedRect(plate, {std::min(width, height) / 2, 0, Corners::Circular}, fill);
}

void RoundButton::enabledChanged()
{
    requestLayout();
    requestPaint();
}

void RoundButton::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void RoundButton::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void RoundButton::pointerPress(const PointerEvent &event)
{
    if (event.button != MouseButton::Left)
        return;
    m_pressed = true;
    requestPaint();
}

void RoundButton::pointerRelease(const PointerEvent &event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (!localRect().contains(event.position))
        return;
    if (onClickedAt)
        onClickedAt(mapToWindow(event.position));
    if (onClicked)
        onClicked();
}

static constexpr double buttonMinimumWidth = 64;
static constexpr double buttonMinimumHeight = 32;

TextButton::TextButton(std::string text, bool flat)
    : m_label(addChild<Label>(std::move(text)))
    , m_flat(flat)
{
    m_label->setTextAlignment(Label::TextAlignment::Center);
    m_label->setVerticallyCentered(true);
    // A disabled button colours its label instead of fading it.
    m_label->setFadesWhenDisabled(false);
}

void TextButton::setText(std::string text)
{
    m_label->setText(std::move(text));
    requestLayout();
}

void TextButton::setFlat(bool flat)
{
    m_flat = flat;
    requestLayout();
    requestPaint();
}

SizeF TextButton::sizeHint() const
{
    const Theme &theme = Theme::current();
    const SizeF text = m_label->sizeHint();
    return {std::max(buttonMinimumWidth, text.width + 2 * theme.largeSpacing),
            std::max(buttonMinimumHeight, text.height + 2 * theme.smallSpacing)};
}

void TextButton::layout()
{
    const Theme &theme = Theme::current();
    m_label->setGeometry(localRect().adjusted(theme.largeSpacing, 0, -theme.largeSpacing, 0));
    if (!isEnabled())
        m_label->setColor(theme.disabledText);
    else
        m_label->setColor(m_flat ? std::optional<Color>(theme.highlight) : std::nullopt);
}

void TextButton::paint(Painter &painter)
{
    const Theme &theme = Theme::current();
    const bool enabled = isEnabled();
    Color fill = color(ColorId::ButtonBackground);
    if (!enabled)
        fill = color(ColorId::ButtonBackground);
    else if (m_pressed)
        fill = m_flat ? theme.highlight.withAlpha(0.25) : color(ColorId::ButtonBackgroundPressed);
    else if (m_hovered)
        fill = m_flat ? theme.highlight.withAlpha(0.2) : color(ColorId::ButtonBackgroundHovered);
    else if (m_flat)
        fill = theme.highlight.withAlpha(0.1);
    painter.drawRoundedRect(localRect(), {theme.mediumRadius, 0, themeCorners()}, fill, 1 / painter.scale(),
                            enabled && hasVisibleFocus() ? color(ColorId::ButtonFocusRing) : Color{0, 0, 0, 0});
}

void TextButton::enabledChanged()
{
    requestLayout();
    requestPaint();
}

void TextButton::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void TextButton::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void TextButton::pointerPress(const PointerEvent &event)
{
    if (event.button != MouseButton::Left)
        return;
    m_pressed = true;
    requestPaint();
}

void TextButton::pointerRelease(const PointerEvent &event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (localRect().contains(event.position) && onClicked)
        onClicked();
}

bool TextButton::keyPress(const KeyEvent &event)
{
    if (event.modifiers != NoModifier)
        return false;
    if (event.key == key::Space || event.key == key::Return || event.key == key::KeypadEnter) {
        if (onClicked)
            onClicked();
        return true;
    }
    return false;
}

// A shadow radius of 16 is a Gaussian of deviation (16 + 1) / 3.3333.
static constexpr double shadowSigma = 17 / 3.3333;

RectF FloatingCard::visualRect() const
{
    const double reach = Painter::shadowReach(shadowSigma);
    return localRect().adjusted(-reach, -reach + 3, reach, reach + 3);
}

void FloatingCard::paint(Painter &painter)
{
    const Theme &theme = Theme::current();
    const RectF plate = localRect();
    painter.drawShadow(plate, m_radius, shadowSigma, color(EditorColor::FloatingCardShadow), {0, 3});
    painter.drawRoundedRect(plate, {m_radius, 0, Corners::Circular}, theme.secondBackground, 1 / painter.scale(),
                            color(EditorColor::FloatingCardBorder));
}

void drawHairline(Painter &painter, double x, double y, double width, Color color)
{
    painter.drawRoundedRect({x, y, width, 1 / painter.scale()}, {0, 0, Corners::Circular}, color);
}
