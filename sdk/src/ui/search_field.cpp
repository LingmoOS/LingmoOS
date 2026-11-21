// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/color_provider.h"
#include "ui/search_field.h"

#include "ui/icon.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace Lingmo::ui {

static constexpr double kGlyphSize = 14;
static constexpr double kFocusRing = 2;

// A press area over the glyph: it takes the press, and the click clears.
class SearchField::ClearButton : public Icon
{
public:
    explicit ClearButton(SearchField* field)
        : Icon("clear", kGlyphSize)
        , m_field(field)
    {
    }

    bool acceptsPointer() const override { return true; }

protected:
    void pointerPress(const PointerEvent&) override { m_pressed = true; }

    void pointerRelease(const PointerEvent& event) override
    {
        const bool clicked = m_pressed && localRect().contains(event.position);
        m_pressed = false;
        if (clicked)
            m_field->setText({});
    }

private:
    SearchField* m_field;
    bool m_pressed = false;
};

SearchField::SearchField(std::string placeholder)
    : m_search(addChild<Icon>("search", kGlyphSize))
    , m_clear(addChild<ClearButton>(this))
{
    setPlaceholder(std::move(placeholder));
    setFrameVisible(false);
    m_clear->setVisible(false);
    updateMargins();
}

void SearchField::setFill(std::optional<ColorRecipe> fill)
{
    m_fill = std::move(fill);
    requestPaint();
}

void SearchField::setGlyphColor(std::optional<ColorRecipe> color)
{
    m_search->setColor(color);
    m_clear->setColor(color);
}

void SearchField::setIconMetrics(double inset, double gap, double hitMargin)
{
    if (inset == m_inset && gap == m_gap && hitMargin == m_hitMargin)
        return;
    m_inset = inset;
    m_gap = gap;
    m_hitMargin = hitMargin;
    updateMargins();
    requestLayout();
}

void SearchField::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const RectF bounds = localRect();
    const Color fill = m_fill ? m_fill->resolve() : theme.secondBackground;
    const double radius = bounds.height / 2;
    const bool focused = hasFocus();
    painter.drawRoundedRect(bounds, {radius, 0, Corners::Circular}, fill, 1, focused ? theme.highlight : fill);
    if (focused) {
        painter.drawRoundedRect(bounds.adjusted(-kFocusRing, -kFocusRing, kFocusRing, kFocusRing),
                                {radius, -kFocusRing, Corners::Circular}, Color{0, 0, 0, 0}, 2,
                                color(ColorId::TextFieldFocusRing));
    }
    TextField::paint(painter);
}

void SearchField::layout()
{
    const RectF bounds = localRect();
    const double top = (bounds.height - kGlyphSize) / 2;
    m_search->setGeometry({m_inset, top, kGlyphSize, kGlyphSize});
    m_clear->setGeometry({bounds.width - m_inset - kGlyphSize - m_hitMargin, top - m_hitMargin,
                          kGlyphSize + 2 * m_hitMargin, kGlyphSize + 2 * m_hitMargin});
}

void SearchField::textChanged()
{
    m_clear->setVisible(!text().empty());
    updateMargins();
}

void SearchField::updateMargins()
{
    const double glyphEnd = m_inset + kGlyphSize + m_gap;
    setTextMargins(glyphEnd, text().empty() ? m_inset : glyphEnd);
}

} // namespace Lingmo::ui
