// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/iconbutton.h"

#include "ui/icon.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"

using namespace Lingmo::ui;

namespace settings {

static constexpr Color kWhite = Color::rgb(0xFFFFFF);

IconButton::IconButton(std::string glyph, double size)
    : m_glyph(std::move(glyph))
    , m_size(size)
{
    if (m_glyph.ends_with(".svg")) {
        m_picture = addChild<Picture>(m_glyph);
        m_pictureOverlay = addChild<Picture>(m_glyph);
        m_pictureOverlay->setTint(kWhite);
        m_pictureOverlay->setOpacity(0.5);
        m_pictureOverlay->setVisible(false);
    } else {
        m_icon = addChild<Icon>(m_glyph, size);
        m_overlay = addChild<Icon>(m_glyph, size);
        m_overlay->setColor(kWhite);
        m_overlay->setOpacity(0.5);
        m_overlay->setVisible(false);
    }
}

void IconButton::setColor(std::optional<ColorRecipe> color)
{
    if (m_icon)
        m_icon->setColor(std::move(color));
    else
        m_picture->setTint(std::move(color));
}

double IconButton::trailingOverhang() const
{
    return -metrics::trailingGlyphMargin(m_glyph, m_size);
}

void IconButton::layout()
{
    for (const auto& child : children())
        child->setGeometry(localRect());
}

void IconButton::setPressedInside(bool inside)
{
    if (m_overlay)
        m_overlay->setVisible(inside);
    if (m_pictureOverlay)
        m_pictureOverlay->setVisible(inside);
}

void IconButton::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left)
        return;
    m_pressed = true;
    setPressedInside(true);
}

void IconButton::pointerMove(const PointerEvent& event)
{
    if (m_pressed)
        setPressedInside(localRect().contains(event.position));
}

void IconButton::pointerRelease(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    setPressedInside(false);
    if (localRect().contains(event.position) && onClicked)
        onClicked();
}

} // namespace settings
