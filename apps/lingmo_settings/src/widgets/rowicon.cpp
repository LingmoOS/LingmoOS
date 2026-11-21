// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/rowicon.h"

#include "ui/icon.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "widgets/picture.h"

using namespace Lingmo::ui;

namespace settings {

static constexpr double kGlyph = 16;

RowIcon::RowIcon(std::string glyph, Color tint, Color accent)
    : m_tint(tint)
    , m_accent(accent)
{
    m_picture = addChild<Picture>();
    m_glyph = addChild<Icon>(std::string(), kGlyph);
    set(std::move(glyph), tint);
}

void RowIcon::set(std::string glyph, Color tint)
{
    m_tint = tint;
    m_pictureGlyph = glyph.ends_with(".svg");
    m_picture->setVisible(m_pictureGlyph);
    m_glyph->setVisible(!m_pictureGlyph);
    if (m_pictureGlyph)
        m_picture->setPath(dataPath("images/sidebar/dark/" + glyph));
    else
        m_glyph->setIconName(std::move(glyph));
    applyGlyphColor();
    requestPaint();
}

void RowIcon::setInverted(bool inverted)
{
    if (inverted == m_inverted)
        return;
    m_inverted = inverted;
    applyGlyphColor();
    requestPaint();
}

// The sidebar's own glyphs are white already; the toolkit's need tinting. The
// unselected tile is a quiet neutral and the current tile takes the accent,
// with a white glyph in both states.
void RowIcon::applyGlyphColor()
{
    const Color glyph = Color::rgb(0xFFFFFF);
    if (m_pictureGlyph)
        m_picture->setTint(glyph);
    else
        m_glyph->setColor(glyph);
}

void RowIcon::layout()
{
    const RectF glyph{(geometry().width - kGlyph) / 2, (geometry().height - kGlyph) / 2, kGlyph, kGlyph};
    m_picture->setGeometry(glyph);
    m_glyph->setGeometry(glyph);
}

void RowIcon::paint(Painter& painter)
{
    const Color accent = m_accent.a ? m_accent : m_tint;
    const Color fill = m_inverted ? accent : m_tint;
    painter.fillRoundedRect(localRect(), 6, LinearGradient::vertical(localRect(), fill.lighter(1.15), fill));
}

} // namespace settings
