// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/header_button_group.h"

#include "ui/header_button.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace Lingmo::ui {

// The vertical divider between neighbouring buttons is inset from the plate's
// top and bottom by this much so it doesn't reach the rounded corners.
static constexpr double kDividerInset = 6;

HeaderButtonGroup::HeaderButtonGroup()
{
    setMovesWindow(true);
}

HeaderButton* HeaderButtonGroup::addButton(std::string glyph, double glyphSize)
{
    HeaderButton* button = addChild<HeaderButton>(std::string(), kSegment);
    button->setGlyph(std::move(glyph), glyphSize);
    m_buttons.push_back(button);
    return button;
}

SizeF HeaderButtonGroup::sizeHint() const
{
    int visible = 0;
    for (const HeaderButton* button : m_buttons) {
        if (button->isVisible())
            ++visible;
    }
    return {visible * kSegment, kSegment};
}

void HeaderButtonGroup::layout()
{
    const RectF bounds = localRect();
    const SizeF hint = sizeHint();
    // Distribute the actual width proportionally so buttons stay square if the
    // group is sized differently from its hint.
    const double width = hint.width > 0 ? bounds.width * kSegment / hint.width : 0;
    double x = 0;
    for (HeaderButton* button : m_buttons) {
        if (!button->isVisible())
            continue;
        button->setGeometry({x, 0, width, bounds.height});
        x += width;
    }
}

void HeaderButtonGroup::paint(Painter& painter)
{
    const Window* window = this->window();
    const bool dark = window ? window->headerDarkMode() : Theme::current().darkMode;
    const RectF bounds = localRect();

    // Plate background: same frosted appearance as a single HeaderButton plate
    // but covering the whole group.
    const double alpha = 0.11;
    if (dark) {
        painter.fillRoundedRect(bounds, kSegment / 2,
                                Color{255, 255, 255, uint8_t(alpha * 255)});
        // Top-edge rim for the glass look.
        painter.strokeRoundedRect(bounds.adjusted(0.5, 0.5, -0.5, -0.5), kSegment / 2 - 0.5,
                                  Color{255, 255, 255, uint8_t(0.22 * 255)}, 1);
    } else {
        painter.fillRoundedRect(bounds, kSegment / 2,
                                Color{0, 0, 0, uint8_t(0.07 * 255)});
        painter.strokeRoundedRect(bounds.adjusted(0.5, 0.5, -0.5, -0.5), kSegment / 2 - 0.5,
                                  Color{255, 255, 255, uint8_t(0.35 * 255)}, 1);
    }

    // Hairline dividers between visible buttons.
    const Color edge = (dark ? Color{255, 255, 255, 36} : Color{0, 0, 0, 30});
    bool first = true;
    for (const HeaderButton* button : m_buttons) {
        if (!button->isVisible())
            continue;
        if (!first) {
            painter.drawVerticalLine(button->geometry().x,
                                     bounds.y + kDividerInset,
                                     bounds.height - 2 * kDividerInset,
                                     edge);
        }
        first = false;
    }
}

} // namespace Lingmo::ui
