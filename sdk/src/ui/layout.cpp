// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/layout.h"

#include <algorithm>

namespace Lingmo::ui {

// Where a child of the given length starts in the given space.
static double aligned(Alignment alignment, double space, double length)
{
    switch (alignment) {
    case Alignment::Center:
        return (space - length) / 2;
    case Alignment::End:
        return space - length;
    default:
        return 0;
    }
}

BoxLayout::BoxLayout(Direction direction)
    : m_direction(direction)
{
}

void BoxLayout::setSpacing(double spacing)
{
    if (spacing == m_spacing)
        return;
    m_spacing = spacing;
    requestLayout();
}

void BoxLayout::setMargins(const Margins& margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    requestLayout();
}

View* BoxLayout::addStretch(double stretch)
{
    View* spacer = addChild<View>();
    LayoutHints hints;
    hints.fillWidth = m_direction == Direction::Horizontal;
    hints.fillHeight = m_direction == Direction::Vertical;
    hints.stretch = stretch;
    spacer->setLayoutHints(hints);
    return spacer;
}

Alignment BoxLayout::crossAlignment(const View* view) const
{
    const Alignment alignment = view->layoutHints().alignment;
    if (alignment != Alignment::Default)
        return alignment;
    return m_direction == Direction::Vertical ? Alignment::Start : Alignment::Center;
}

// Lengths along the layout before the leftover space is shared. A negative
// width measures without a bound.
std::vector<BoxLayout::Item> BoxLayout::measure(double width) const
{
    const double contentWidth = width < 0 ? -1 : std::max(0.0, width - m_margins.horizontal());
    std::vector<Item> items;
    for (const auto& child : children()) {
        if (!child->isVisible())
            continue;
        const LayoutHints& hints = child->layoutHints();
        const SizeF preferred = child->preferredSize();
        Item item{child.get(), 0, 0};
        if (m_direction == Direction::Vertical) {
            double childWidth = preferred.width;
            if (contentWidth >= 0)
                childWidth = hints.fillWidth ? contentWidth : std::min(childWidth, contentWidth);
            item.crossLength = childWidth;
            item.length = hints.preferredHeight >= 0 ? hints.preferredHeight
                                                     : child->heightForWidth(childWidth);
        } else {
            item.length = preferred.width;
        }
        items.push_back(item);
    }
    return items;
}

void BoxLayout::distribute(std::vector<Item>& items, double available) const
{
    if (items.empty())
        return;
    const bool vertical = m_direction == Direction::Vertical;
    double used = m_spacing * double(items.size() - 1);
    double stretch = 0;
    for (const Item& item : items) {
        used += item.length;
        const LayoutHints& hints = item.view->layoutHints();
        if (vertical ? hints.fillHeight : hints.fillWidth)
            stretch += hints.stretch;
    }
    const double leftover = available - used;
    if (stretch <= 0 || leftover == 0)
        return;
    for (Item& item : items) {
        const LayoutHints& hints = item.view->layoutHints();
        if (vertical ? hints.fillHeight : hints.fillWidth)
            item.length = std::max(0.0, item.length + leftover * hints.stretch / stretch);
    }
}

SizeF BoxLayout::sizeHint() const
{
    const std::vector<Item> items = measure(-1);
    double length = items.empty() ? 0 : m_spacing * double(items.size() - 1);
    double cross = 0;
    for (const Item& item : items) {
        length += item.length;
        const double itemCross = m_direction == Direction::Vertical
                ? item.crossLength
                : item.view->heightForWidth(item.length);
        cross = std::max(cross, itemCross);
    }
    if (m_direction == Direction::Vertical)
        return {cross + m_margins.horizontal(), length + m_margins.vertical()};
    return {length + m_margins.horizontal(), cross + m_margins.vertical()};
}

double BoxLayout::heightForWidth(double width) const
{
    std::vector<Item> items = measure(width);
    if (m_direction == Direction::Vertical) {
        double height = items.empty() ? 0 : m_spacing * double(items.size() - 1);
        for (const Item& item : items)
            height += item.length;
        return height + m_margins.vertical();
    }

    distribute(items, width - m_margins.horizontal());
    double height = 0;
    for (const Item& item : items) {
        const LayoutHints& hints = item.view->layoutHints();
        height = std::max(height, hints.preferredHeight >= 0
                                          ? hints.preferredHeight
                                          : item.view->heightForWidth(item.length));
    }
    return height + m_margins.vertical();
}

void BoxLayout::layout()
{
    const RectF content = localRect().shrunk(m_margins);
    std::vector<Item> items = measure(geometry().width);
    const bool vertical = m_direction == Direction::Vertical;
    distribute(items, vertical ? content.height : content.width);

    double position = vertical ? content.y : content.x;
    for (Item& item : items) {
        View* view = item.view;
        const LayoutHints& hints = view->layoutHints();
        if (vertical) {
            const double width = item.crossLength;
            const double x = content.x + aligned(crossAlignment(view), content.width, width);
            view->setGeometry({x, position, width, item.length});
        } else {
            double height = content.height;
            if (!hints.fillHeight) {
                height = hints.preferredHeight >= 0 ? hints.preferredHeight
                                                    : view->heightForWidth(item.length);
                height = std::min(height, content.height);
            }
            const double y = content.y + aligned(crossAlignment(view), content.height, height);
            view->setGeometry({position, y, item.length, height});
        }
        position += item.length + m_spacing;
    }
}

Grid::Grid(int columns)
    : m_columns(std::max(1, columns))
{
}

void Grid::setColumns(int columns)
{
    if (std::max(1, columns) == m_columns)
        return;
    m_columns = std::max(1, columns);
    requestLayout();
}

void Grid::setColumnSpacing(double spacing)
{
    if (spacing == m_columnSpacing)
        return;
    m_columnSpacing = spacing;
    requestLayout();
}

void Grid::setRowSpacing(double spacing)
{
    if (spacing == m_rowSpacing)
        return;
    m_rowSpacing = spacing;
    requestLayout();
}

void Grid::setMargins(const Margins& margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    requestLayout();
}

std::vector<View*> Grid::visibleChildren() const
{
    std::vector<View*> views;
    for (const auto& child : children()) {
        if (child->isVisible())
            views.push_back(child.get());
    }
    return views;
}

// Column widths and row heights. A negative width measures without a bound.
Grid::Tracks Grid::measure(double width) const
{
    const std::vector<View*> views = visibleChildren();
    const int rows = int((views.size() + m_columns - 1) / m_columns);
    Tracks tracks;
    tracks.widths.assign(m_columns, 0);
    tracks.heights.assign(rows, 0);
    std::vector<bool> fills(m_columns, false);

    for (size_t i = 0; i < views.size(); ++i) {
        const int column = int(i % m_columns);
        tracks.widths[column] = std::max(tracks.widths[column], views[i]->preferredSize().width);
        if (views[i]->layoutHints().fillWidth)
            fills[column] = true;
    }

    const int filling = int(std::count(fills.begin(), fills.end(), true));
    if (width >= 0 && filling > 0) {
        double used = m_columnSpacing * (m_columns - 1);
        for (double columnWidth : tracks.widths)
            used += columnWidth;
        const double share = (width - m_margins.horizontal() - used) / filling;
        for (int column = 0; column < m_columns; ++column) {
            if (fills[column])
                tracks.widths[column] = std::max(0.0, tracks.widths[column] + share);
        }
    }

    for (size_t i = 0; i < views.size(); ++i) {
        const int row = int(i / m_columns);
        const double columnWidth = tracks.widths[i % m_columns];
        const LayoutHints& hints = views[i]->layoutHints();
        const double childWidth = hints.fillWidth
                ? columnWidth
                : std::min(views[i]->preferredSize().width, columnWidth);
        const double height = hints.preferredHeight >= 0 ? hints.preferredHeight
                                                         : views[i]->heightForWidth(childWidth);
        tracks.heights[row] = std::max(tracks.heights[row], height);
    }
    return tracks;
}

SizeF Grid::sizeHint() const
{
    const Tracks tracks = measure(-1);
    double width = m_margins.horizontal() + m_columnSpacing * (m_columns - 1);
    for (double columnWidth : tracks.widths)
        width += columnWidth;
    return {width, heightForWidth(width)};
}

double Grid::heightForWidth(double width) const
{
    const Tracks tracks = measure(width);
    double height = m_margins.vertical();
    if (!tracks.heights.empty())
        height += m_rowSpacing * double(tracks.heights.size() - 1);
    for (double rowHeight : tracks.heights)
        height += rowHeight;
    return height;
}

void Grid::layout()
{
    const RectF content = localRect().shrunk(m_margins);
    const std::vector<View*> views = visibleChildren();
    Tracks tracks = measure(geometry().width);

    // Rows holding a child that fills its height share the height left over.
    std::vector<bool> fills(tracks.heights.size(), false);
    for (size_t i = 0; i < views.size(); ++i) {
        if (views[i]->layoutHints().fillHeight)
            fills[i / m_columns] = true;
    }
    const int filling = int(std::count(fills.begin(), fills.end(), true));
    if (filling > 0) {
        const double used = heightForWidth(geometry().width) - m_margins.vertical();
        const double share = (content.height - used) / filling;
        for (size_t row = 0; row < tracks.heights.size(); ++row) {
            if (fills[row])
                tracks.heights[row] = std::max(0.0, tracks.heights[row] + share);
        }
    }

    double y = content.y;
    for (size_t row = 0; row < tracks.heights.size(); ++row) {
        double x = content.x;
        for (int column = 0; column < m_columns; ++column) {
            const size_t index = row * m_columns + column;
            if (index >= views.size())
                break;
            View* view = views[index];
            const LayoutHints& hints = view->layoutHints();
            const double cellWidth = tracks.widths[column];
            const double cellHeight = tracks.heights[row];
            const double width = hints.fillWidth
                    ? cellWidth
                    : std::min(view->preferredSize().width, cellWidth);
            double height = cellHeight;
            if (!hints.fillHeight) {
                height = hints.preferredHeight >= 0 ? hints.preferredHeight
                                                    : view->heightForWidth(width);
                height = std::min(height, cellHeight);
            }
            const Alignment horizontal = hints.alignment == Alignment::Default
                    ? Alignment::Start
                    : hints.alignment;
            view->setGeometry({x + aligned(horizontal, cellWidth, width),
                               y + aligned(Alignment::Center, cellHeight, height), width, height});
            x += cellWidth + m_columnSpacing;
        }
        y += tracks.heights[row] + m_rowSpacing;
    }
}

} // namespace Lingmo::ui
