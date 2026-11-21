// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "ui/view.h"

namespace Lingmo::ui {

// Lines up its visible children along one direction. Each gets its preferred
// length; the space left over goes to the children that fill that direction,
// by stretch. Across, a child fills or keeps its preferred size, aligned.
class BoxLayout : public View
{
public:
    enum class Direction {
        Horizontal,
        Vertical,
    };

    explicit BoxLayout(Direction direction);

    Direction direction() const { return m_direction; }

    void setSpacing(double spacing);
    double spacing() const { return m_spacing; }
    void setMargins(const Margins& margins);
    const Margins& margins() const { return m_margins; }

    // An empty child that fills the remaining space.
    View* addStretch(double stretch = 1);

    SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    void layout() override;

private:
    struct Item
    {
        View* view;
        double length;
        double crossLength;
    };

    std::vector<Item> measure(double width) const;
    void distribute(std::vector<Item>& items, double available) const;
    Alignment crossAlignment(const View* view) const;

    Direction m_direction;
    double m_spacing = 0;
    Margins m_margins;
};

class Column : public BoxLayout
{
public:
    Column()
        : BoxLayout(Direction::Vertical)
    {
    }
};

class Row : public BoxLayout
{
public:
    Row()
        : BoxLayout(Direction::Horizontal)
    {
    }
};

// Places its visible children row by row in a fixed number of columns. A
// column is as wide as its widest child and a row as tall as its tallest;
// columns or rows with a filling child share the space left over.
class Grid : public View
{
public:
    explicit Grid(int columns = 2);

    void setColumns(int columns);
    int columns() const { return m_columns; }
    void setColumnSpacing(double spacing);
    void setRowSpacing(double spacing);
    void setMargins(const Margins& margins);

    SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    void layout() override;

private:
    struct Tracks
    {
        std::vector<double> widths;
        std::vector<double> heights;
    };

    std::vector<View*> visibleChildren() const;
    Tracks measure(double width) const;

    int m_columns;
    double m_columnSpacing = 0;
    double m_rowSpacing = 0;
    Margins m_margins;
};

} // namespace Lingmo::ui
