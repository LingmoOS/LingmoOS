// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/tilegrid.h"

#include <algorithm>
#include <cmath>

using namespace Lingmo::ui;

namespace settings {

TileGrid::TileGrid(SizeF cell, double bottomMargin)
    : m_cell(cell)
    , m_bottomMargin(bottomMargin)
{
    setLayoutHints({.fillWidth = true});
}

int TileGrid::columns(double width) const
{
    return std::max(1, int(std::floor(width / m_cell.width)));
}

int TileGrid::visibleCount() const
{
    return int(std::count_if(children().begin(), children().end(),
                             [](const auto& child) { return child->isVisible(); }));
}

SizeF TileGrid::sizeHint() const
{
    return {m_cell.width, heightForWidth(m_cell.width)};
}

double TileGrid::heightForWidth(double width) const
{
    const int count = visibleCount();
    if (count == 0)
        return 0;
    const int rows = (count + columns(width) - 1) / columns(width);
    return rows * m_cell.height + m_bottomMargin;
}

void TileGrid::layout()
{
    const int perRow = columns(geometry().width);
    int index = 0;
    for (const auto& child : children()) {
        if (!child->isVisible())
            continue;
        const SizeF size = child->sizeHint();
        const double x = (index % perRow) * m_cell.width;
        const double y = (index / perRow) * m_cell.height + (m_cell.height - size.height) / 2;
        child->setGeometry({x, y, size.width, size.height});
        ++index;
    }
}

} // namespace settings
