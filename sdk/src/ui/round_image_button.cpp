// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/color_provider.h"
#include "ui/round_image_button.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "include/core/SkImage.h"

#include "ui/icon.h"
#include "ui/icon_loader.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace Lingmo::ui {

RoundImageButton::RoundImageButton(std::string iconName, double size)
    : m_iconName(std::move(iconName))
    , m_size(size)
    , m_image(addChild<ThemeIcon>())
{
    m_image->setVisible(false);
}

void RoundImageButton::setButtonSize(double size)
{
    if (size == m_size)
        return;
    m_size = size;
    requestLayout();
    requestPaint();
}

void RoundImageButton::setIconMargins(double margins)
{
    if (margins == m_iconMargins)
        return;
    m_iconMargins = margins;
    requestLayout();
    requestPaint();
}

void RoundImageButton::setIconSize(double size)
{
    if (size == m_iconSize)
        return;
    m_iconSize = size;
    requestLayout();
    requestPaint();
}

double RoundImageButton::iconSize() const
{
    // An int property in FishUI: whole logical pixels.
    return m_iconSize > 0 ? std::round(m_iconSize) : std::round(m_size - m_iconMargins * 2);
}

void RoundImageButton::setIconOffset(PointF offset)
{
    if (offset == m_iconOffset)
        return;
    m_iconOffset = offset;
    requestLayout();
    requestPaint();
}

void RoundImageButton::setIconName(std::string name)
{
    m_iconName = std::move(name);
    updateIcon();
    requestPaint();
}

void RoundImageButton::setSource(std::string source)
{
    m_source = std::move(source);
    updateIcon();
}

void RoundImageButton::setIconColor(std::optional<Color> color)
{
    m_iconColor = color;
    requestPaint();
}

void RoundImageButton::setBackgroundColor(std::optional<Color> color)
{
    m_backgroundColor = color;
    requestPaint();
}

void RoundImageButton::setHoveredColor(std::optional<Color> color)
{
    m_hoveredColor = color;
    requestPaint();
}

void RoundImageButton::setPressedColor(std::optional<Color> color)
{
    m_pressedColor = color;
    requestPaint();
}

void RoundImageButton::setHighlighted(bool highlighted)
{
    if (m_highlighted == highlighted)
        return;
    m_highlighted = highlighted;
    requestPaint();
}

void RoundImageButton::setChecked(bool checked)
{
    if (m_checked == checked)
        return;
    m_checked = checked;
    requestPaint();
}

void RoundImageButton::setIconOpacity(std::optional<double> opacity)
{
    m_iconOpacity = opacity;
    updateIcon();
    requestPaint();
}

PointF RoundImageButton::iconPosition() const
{
    const RectF bounds = localRect();
    const double icon = iconSize();
    return {std::round((bounds.width - icon) / 2 + m_iconOffset.x),
            std::round((bounds.height - icon) / 2 + m_iconOffset.y)};
}

void RoundImageButton::updateIcon()
{
    m_image->setIconName(m_source);
    m_image->setVisible(m_iconName.empty() && !m_source.empty());
    m_image->setOpacity(m_iconOpacity.value_or(isEnabled() ? 1.0 : 0.35));
}

void RoundImageButton::layout()
{
    const double icon = iconSize();
    const PointF position = iconPosition();
    m_image->setSize(icon);
    m_image->setGeometry({position.x, position.y, icon, icon});
}

void RoundImageButton::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const RectF bounds = localRect();
    const double icon = iconSize();
    const PointF position = iconPosition();
    const double ratio = painter.scale();

    Color plate = m_backgroundColor.value_or(Color{0, 0, 0, 0});
    if (!isEnabled())
        plate = Color{0, 0, 0, 0};
    else if (m_pressed || m_checked)
        plate = m_pressedColor.value_or(color(ColorId::RoundButtonPressed));
    else if (m_hovered || m_highlighted)
        plate = m_hoveredColor.value_or(color(ColorId::RoundButtonHovered));

    if (plate.a > 0) {
        // Whole device pixels of the icon's parity, so the snapped plate and
        // the snapped icon share a centre wherever the button lands.
        const auto plateLength = [&](double length) {
            int pixels = int(std::lround(length * ratio));
            if ((pixels - int(std::lround(icon * ratio))) % 2 != 0)
                pixels -= 1;
            return pixels / ratio;
        };
        const double margin = std::round(m_size * 0.1);
        const double width = plateLength(bounds.width - margin * 2);
        const double height = plateLength(bounds.height - margin * 2);
        painter.fillRoundedRect({position.x + (icon - width) / 2, position.y + (icon - height) / 2, width, height},
                                std::min(width, height) / 2, plate);
    }

    if (m_iconName.empty())
        return;
    // A glyph as FishUI.Icon draws it: its colour in every state, faded
    // rather than greyed when disabled.
    const int side = std::max(1, int(std::lround(icon * ratio)));
    const double extent = side / ratio;
    const Rect rect = painter.snappedRect({position.x + (icon - extent) / 2, position.y + (icon - extent) / 2,
                                           extent, extent});
    sk_sp<SkImage> image = IconLoader::instance().glyph(m_iconName, rect.width, m_iconColor.value_or(theme.text),
                                                        ratio);
    if (!image)
        return;
    painter.save();
    painter.multiplyOpacity(m_iconOpacity.value_or(isEnabled() ? 1.0 : 0.35));
    painter.drawImage(image.get(), rect.x, rect.y);
    painter.restore();
}

void RoundImageButton::enabledChanged()
{
    updateIcon();
    requestPaint();
}

void RoundImageButton::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void RoundImageButton::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void RoundImageButton::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left)
        return;
    m_pressed = true;
    requestPaint();
}

void RoundImageButton::pointerRelease(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (localRect().contains(event.position) && onClicked)
        onClicked();
}

} // namespace Lingmo::ui
