// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <utility>
#include <vector>

#include "ui/view.h"

namespace Lingmo::ui {

// Lays its children out in a row, a column or a grid: implicit sizes are
// rounded up to whole pixels, positions are rounded, and filling children
// share the space beyond their preferred sizes in proportion to them. A
// child's LayoutHints give its preferred, minimum and maximum sizes,
// fillWidth and fillHeight, the alignment per axis (left and vertically
// centred by default) and margins.
// A nested layout fills, between the minimum and maximum its children take
// unless its hints set them. Hidden children take no space. Row, Column and
// Grid are the toolkit's own layouts, which only place children in turn.
class CellLayout : public View
{
public:
    // A layout's minimum, preferred and maximum on one axis; its padding
    // included.
    struct Extent
    {
        double minimum = 0;
        double preferred = 0;
        double maximum = 0;
    };

    // 5 between children by default.
    void setSpacing(double spacing);
    void setRowSpacing(double spacing);
    void setColumnSpacing(double spacing);
    // Children lay out in the rect less the padding, as a layout anchored
    // with margins in a padded item: inside another layout a padded one is
    // that item, filling only as its hints say.
    void setPadding(const Margins& padding);
    const Margins& padding() const { return m_padding; }

    Extent horizontalExtent() const;
    // Heights at the width; a negative width measures at the preferred one.
    Extent verticalExtent(double width) const;

    SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    enum class Flow {
        Row,
        Column,
        Grid,
    };

    CellLayout(Flow flow, int columns);

    void setColumnCount(int columns);
    void layout() override;

private:
    struct Cell
    {
        View* view;
        int row;
        int column;
        // The view as a nested layout, if it is one.
        const CellLayout* nested;
    };
    struct Geometry
    {
        std::vector<double> x;
        std::vector<double> widths;
        std::vector<double> y;
        std::vector<double> heights;
        Extent horizontal;
        Extent vertical;
        // Per cell: the width and height its item takes, margins included.
        std::vector<double> itemWidths;
        std::vector<double> itemHeights;
    };

    std::vector<Cell> cells() const;
    // Columns and rows for the size inside the padding; negative lengths
    // measure preferred. Without `place`, only the extents and the column
    // widths are filled in.
    Geometry solve(const std::vector<Cell>& cells, double width, double height, bool place) const;
    // Forgets the extents of an earlier measurement generation.
    void checkMeasurements() const;

    Flow m_flow;
    int m_columns;
    double m_rowSpacing = 5;
    double m_columnSpacing = 5;
    Margins m_padding;
    // Nested layouts are measured many times in one pass, for every
    // ancestor's solve; these keep the answers for one generation.
    mutable uint64_t m_measuredGeneration = 0;
    mutable std::optional<Extent> m_horizontalExtent;
    mutable std::vector<std::pair<double, Extent>> m_verticalExtents;
};

class RowLayout : public CellLayout
{
public:
    RowLayout()
        : CellLayout(Flow::Row, -1)
    {
    }
};

class ColumnLayout : public CellLayout
{
public:
    ColumnLayout()
        : CellLayout(Flow::Column, 1)
    {
    }
};

// Children flow left to right, a new row after every `columns` of them;
// unlimited columns keep them in one row.
class GridLayout : public CellLayout
{
public:
    explicit GridLayout(int columns = -1)
        : CellLayout(Flow::Grid, columns)
    {
    }

    void setColumns(int columns) { setColumnCount(columns); }
};

namespace rounding {

// Rounds halves away from zero.
double halfAwayFromZero(double value);

// Where centring anchors put an item of the extent in a parent of the
// extent: half the parent rounded less half the item rounded, halves away
// from zero.
double centeredStart(double parentExtent, double itemExtent);

} // namespace rounding

} // namespace Lingmo::ui
