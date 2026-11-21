// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/gridview.h"
#include "core/preferences.h"
#include "model/foldermodel.h"
#include "views/style.h"

#include "ui/image_view.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

namespace fm {

using ui::rounding::centeredStart;

static constexpr double kSmallSpacing = 6;
static constexpr double kLargeSpacing = 12;
static constexpr double kMinimumIconSize = 22;

// The selection's ColorOverlay over a thumbnail: its rounded shape in the colour.
class ThumbnailTint : public ui::View
{
public:
    void setRadius(double radius)
    {
        m_radius = radius;
        requestPaint();
    }

protected:
    void paint(ui::Painter& painter) override
    {
        painter.drawSmoothRectangle(localRect(), m_radius, ui::Theme::current().highlight);
    }

private:
    double m_radius = 0;
};

// One item's cell in the grid.
class GridItem : public IconItem
{
public:
    explicit GridItem(FolderGridView* view)
        : IconItem(view, kSmallSpacing / 2, 0.5)
        , m_grid(view)
    {
        addIconViews();
        m_thumbnailTint = addChild<ThumbnailTint>();
        m_thumbnailTint->setVisible(false);
        addLabelView();
    }

protected:
    void bindItem(const FileItem&, int row) override
    {
        m_label->setText(m_grid->model().displayName(row));
        m_label->setMaximumLineCount(m_selected ? 3 : 2);
        m_label->setColor(m_selected ? style::selectedTextColor() : ui::Theme::current().text);
    }

    void layout() override
    {
        const double width = geometry().width;
        const double iconSize = m_grid->iconSize();
        const double layoutWidth = m_grid->minimumCellWidth();
        const double iconItemWidth = layoutWidth - kLargeSpacing * 2;
        layoutIcon({centeredStart(width, iconItemWidth), kSmallSpacing, iconItemWidth, iconSize}, 16);

        const bool thumbnailShown = m_thumbnail->isVisible();
        m_thumbnailTint->setVisible(m_selected && thumbnailShown);
        if (thumbnailShown) {
            m_thumbnailTint->setGeometry(m_thumbnail->geometry());
            m_thumbnailTint->setRadius(m_thumbnail->geometry().height * 0.1);
            m_thumbnailTint->setOpacity(0.5 * (m_hidden ? 0.5 : 1.0));
        }

        const double labelWidth = layoutWidth - kLargeSpacing * 2 - kSmallSpacing;
        const double labelHeight = m_label->heightForWidth(labelWidth);
        m_label->setGeometry({centeredStart(width, labelWidth), kSmallSpacing + iconSize + kSmallSpacing, labelWidth,
                              labelHeight});
    }

private:
    FolderGridView* m_grid;
    ThumbnailTint* m_thumbnailTint;
};

FolderGridView::FolderGridView(FolderModel& model)
    : IconGridView(model)
{
    setName("FolderGridView");
    m_iconSize = Preferences::instance().gridIconSize();
    updateMetrics();
}

FolderGridView::~FolderGridView() = default;

std::unique_ptr<IconItem> FolderGridView::makeItem()
{
    return std::make_unique<GridItem>(this);
}

void FolderGridView::setIconSize(double size)
{
    if (m_iconSize == size)
        return;
    m_iconSize = size;
    Preferences::instance().setGridIconSize(size);
    requestLayout();
}

void FolderGridView::increaseIconSize()
{
    const double maximum = Preferences::instance().maximumIconSize();
    setIconSize(m_iconSize >= maximum ? maximum : std::min(maximum, m_iconSize + m_iconSize * 0.1));
}

void FolderGridView::decreaseIconSize()
{
    setIconSize(m_iconSize <= kMinimumIconSize ? kMinimumIconSize
                                               : std::max(kMinimumIconSize, m_iconSize - m_iconSize * 0.1));
}

void FolderGridView::updateMetrics()
{
    const int fromColumns = m_columns;
    const double fromRowHeight = m_cellHeight;

    const double available = geometry().width - leftMargin() - rightMargin();
    // The columns come from the width alone and share it: counting them from
    // the cell width, which follows the width too, dropped a column for an
    // instant on every pixel of a resize.
    m_columns = std::max(1, int(std::floor(available / minimumCellWidth())));
    // Rounded down a 64th of a pixel, so the cells never add up to more than
    // the width; each item snaps its own drawing to the pixel grid.
    m_cellWidth = std::max(minimumCellWidth(), std::floor(available / m_columns * 64) / 64);
    m_cellHeight = m_iconSize + style::fontHeight() * 2 + kLargeSpacing * 2;

    if (fromRowHeight > 0)
        reflow(fromColumns, fromRowHeight);
}

void FolderGridView::reflow(int fromColumns, double fromRowHeight)
{
    if (fromColumns <= 0 || fromRowHeight <= 0 || (fromColumns == m_columns && fromRowHeight == m_cellHeight))
        return;

    // The offset of the view's top below the first row, in content
    // coordinates, which start at the first row.
    const double top = scrollPosition().y - topMargin() + headerHeight();
    if (top <= 0)
        return;
    const int row = int(std::floor(top / fromRowHeight));
    const double within = (top - row * fromRowHeight) * m_cellHeight / fromRowHeight;
    const double y = std::floor(double(row) * fromColumns / m_columns) * m_cellHeight + within - headerHeight();
    // The content height changes with the new metrics: lay it out first.
    ScrollView::layout();
    setScrollPosition({0, std::max(0.0, y + topMargin())});
}

void FolderGridView::layout()
{
    updateMetrics();
    FolderView::layout();
}

double FolderGridView::minimumContentHeight() const
{
    const int count = model().count();
    const int rows = (count + m_columns - 1) / std::max(1, m_columns);
    return std::max(geometry().height, topMargin() + rows * m_cellHeight + bottomMargin());
}

ui::RectF FolderGridView::cellRect(int index) const
{
    const int column = index % std::max(1, m_columns);
    const int row = index / std::max(1, m_columns);
    const ui::Window* window = this->window();
    const double scale = window ? window->scale() : 1;
    // Cells are a fraction of a pixel wide; each sits on whole device pixels.
    const double x = std::round(column * m_cellWidth * scale) / scale;
    return {leftMargin() + x, topMargin() + row * m_cellHeight, m_cellWidth, m_cellHeight};
}

int FolderGridView::indexAt(ui::PointF content) const
{
    const double x = content.x - leftMargin();
    const double y = content.y - topMargin();
    if (x < 0 || y < 0)
        return -1;
    const int column = int(std::floor(x / m_cellWidth));
    const int row = int(std::floor(y / m_cellHeight));
    if (column >= m_columns)
        return -1;
    const int index = row * m_columns + column;
    return index < model().count() ? index : -1;
}

void FolderGridView::rectangleSelect(const ui::RectF& band)
{
    const int count = model().count();
    const double cellWidth = m_cellWidth - 2 * kSmallSpacing;
    const double cellHeight = m_cellHeight - 2 * kSmallSpacing;
    // In content coordinates, which start at the first cell.
    const double x = band.x - leftMargin();
    const double y = band.y - topMargin();
    const int perStripe = std::max(1, int(std::floor(geometry().width / m_cellWidth)));
    const int stripes = (count + perStripe - 1) / perStripe;
    const int firstColumn = std::max(0, int(std::floor(x / m_cellWidth)) - 1);
    const int lastColumn = int(std::floor((x + band.width) / m_cellWidth)) + 1;
    const int firstRow = std::max(0, int(std::floor(y / m_cellHeight)) - 1);
    const int lastRow = std::min(stripes - 1, int(std::floor((y + band.height) / m_cellHeight)) + 1);
    const int lastCell = std::min(perStripe - 1, lastColumn);

    std::vector<int> indices;
    for (int stripe = firstRow; stripe <= lastRow; ++stripe) {
        for (int cell = firstColumn; cell <= lastCell; ++cell) {
            const int index = stripe * perStripe + cell;
            if (index >= count)
                break;
            if (model().isBlank(rowOf(index)))
                continue;

            const double cellX = cell * m_cellWidth + kSmallSpacing;
            const double cellY = stripe * m_cellHeight + kSmallSpacing;
            // The cell first, which is cheaper than the item.
            if (cellX >= x + band.width || x >= cellX + cellWidth || cellY >= y + band.height
                || y >= cellY + cellHeight)
                continue;

            // An item answers for its icon and name; one not made has its cell.
            IconItem* item = itemFor(index);
            const ui::RectF itemCell = cellRect(index);
            if (!item || item->intersectsRect({band.x - itemCell.x, band.y - itemCell.y, band.width, band.height}))
                indices.push_back(index);
        }
    }
    applyRectangleSelection(indices);
}

void FolderGridView::revealIndex(int index)
{
    if (index < 0)
        return;
    const ui::RectF cell = cellRect(index);
    const double top = scrollPosition().y + headerHeight();
    const double bottom = scrollPosition().y + geometry().height - bottomMargin();
    if (cell.y < top)
        setContentY(cell.y - headerHeight());
    else if (cell.y + cell.height > bottom)
        setContentY(cell.y + cell.height - geometry().height + bottomMargin());
}

void FolderGridView::updateItems()
{
    const int count = model().count();
    const double top = scrollPosition().y;
    const double bottom = top + geometry().height;
    int first = 0;
    int last = 0;
    if (count > 0 && m_cellHeight > 0) {
        const int firstRow = std::max(0, int(std::floor((top - topMargin()) / m_cellHeight)));
        const int lastRow = std::max(0, int(std::ceil((bottom - topMargin()) / m_cellHeight)));
        first = std::min(count, firstRow * m_columns);
        last = std::min(count, lastRow * m_columns);
    }

    recycleItems([first, last](int index) { return index >= first && index < last; });
    for (int index = first; index < last; ++index)
        showItem(index);
}

bool FolderGridView::navigate(const ui::KeyEvent& event)
{
    if ((event.key == '=' || event.key == '+') && (event.modifiers & ui::ControlModifier)) {
        increaseIconSize();
        return true;
    }
    if (event.key == '-' && (event.modifiers & ui::ControlModifier)) {
        decreaseIconSize();
        return true;
    }

    int step = 0;
    if (event.key == ui::key::Left)
        step = -1;
    else if (event.key == ui::key::Right)
        step = 1;
    else if (event.key == ui::key::Up)
        step = -m_columns;
    else if (event.key == ui::key::Down)
        step = m_columns;
    else
        return false;

    const int count = model().count();
    if (count <= 0 || m_columns <= 0)
        return true;
    // A packed grid: plain index arithmetic, nothing past either end.
    const int from = keyboardIndex(step > 0);
    const int next = from < 0 ? 0 : from + step;
    if (next < 0 || next >= count)
        return true;
    setCurrentIndex(next);
    updateSelection(event.modifiers);
    return true;
}

bool FolderGridView::scroll(const ui::ScrollEvent& event)
{
    if (event.modifiers & ui::ControlModifier) {
        if (event.deltaY < 0 || event.value120Y < 0)
            increaseIconSize();
        else
            decreaseIconSize();
        return true;
    }
    return FolderView::scroll(event);
}

} // namespace fm
