// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/styled_label.h"

#include <utility>

namespace Lingmo::ui {

StyledLabel::StyledLabel(std::string text)
    : Label(std::move(text))
{
}

void StyledLabel::setStyle(Style style, Color color)
{
    m_style = style;
    m_styleColor = color;
    requestPaint();
}

RectF StyledLabel::visualRect() const
{
    const RectF rect = Label::visualRect();
    switch (m_style) {
    case Style::Raised:
        return rect.adjusted(0, 0, 0, 1);
    case Style::Sunken:
        return rect.adjusted(0, -1, 0, 0);
    case Style::Normal:
        break;
    }
    return rect;
}

void StyledLabel::paint(Painter& painter)
{
    if (m_style != Style::Normal)
        paintText(painter, {0, m_style == Style::Raised ? 1.0 : -1.0}, m_styleColor);
    paintText(painter, {}, std::nullopt);
}

} // namespace Lingmo::ui
