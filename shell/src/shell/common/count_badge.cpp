// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/count_badge.h"

#include <algorithm>
#include <cmath>

#include "ui/label.h"
#include "ui/painter.h"

namespace Lingmo::Shell {

static constexpr int64_t kMaximum = 99;

CountBadge::CountBadge()
{
    setName("count-badge");
    m_label = addChild<ui::Label>();
    m_label->setColor(ui::Color {255, 255, 255, 255});
    m_label->setWeight(600);
    m_label->setFontFeatures({{"tnum", 1}});
    m_label->setElide(ui::Label::Elide::None);
    setVisible(false);
}

CountBadge::~CountBadge() = default;

std::string CountBadge::textFor(int64_t count)
{
    if (count <= 0)
        return {};
    return count > kMaximum ? std::to_string(kMaximum) + "+" : std::to_string(count);
}

const std::string &CountBadge::text() const
{
    return m_label->text();
}

void CountBadge::setCount(int64_t count)
{
    count = std::max<int64_t>(0, count);
    if (count == m_count)
        return;
    m_count = count;
    m_label->setText(textFor(count));
    setVisible(count > 0);
    place();
}

double CountBadge::heightFor(double iconSize, double ratio)
{
    return std::max(14.0, std::round(iconSize * ratio));
}

void CountBadge::setHeightRatio(double ratio)
{
    if (ratio == m_heightRatio)
        return;
    m_heightRatio = ratio;
    place();
}

void CountBadge::setIconRect(const ui::RectF &icon)
{
    if (icon == m_iconRect)
        return;
    m_iconRect = icon;
    place();
}

void CountBadge::place()
{
    if (m_count <= 0 || m_iconRect.width <= 0)
        return;
    const double height = heightFor(m_iconRect.width, m_heightRatio);
    m_label->setFontSize(std::round(height * 0.6));
    // The text keeps the gap above and below it at its sides too, so one
    // digit sits in a circle and more widen it into a pill.
    const ui::RectF ink = m_label->inkBoundingRect();
    const double scale = textLayoutMode().scale;
    const double width = std::max(height, std::round((ink.width + height - ink.height) * scale) / scale);
    // Over the icon's corner, a little past its top and right edges.
    const double outset = std::round(height * 0.2);
    setGeometry({m_iconRect.x + m_iconRect.width + outset - width, m_iconRect.y - outset, width, height});
    requestLayout();
    requestPaint();
}

void CountBadge::layout()
{
    const ui::RectF rect = localRect();
    const ui::SizeF size = m_label->sizeHint();
    // Centred on the glyphs' ink.
    const ui::RectF ink = m_label->inkBoundingRect();
    const double x = (rect.width - ink.width) / 2 - ink.x;
    const double baseline = (rect.height - ink.height) / 2 - ink.y;
    m_label->setGeometry({x, baseline - m_label->baselineOffset(), size.width, size.height});
}

void CountBadge::paint(ui::Painter &painter)
{
    const ui::RectF rect = localRect();
    painter.fillRoundedRect(rect, rect.height / 2, kColor);
}

} // namespace Lingmo::Shell
