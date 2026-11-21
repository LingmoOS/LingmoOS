// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/desktopview.h"
#include "model/foldermodel.h"
#include "views/style.h"

#include "base/url.h"
#include "ui/blurred_copy.h"
#include "ui/fast_blur.h"
#include "ui/glyph_run.h"
#include "ui/icon.h"
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

static constexpr double kIconTopMargin = 6;
static constexpr double kLabelSpacing = 5;
static constexpr double kLabelInset = 6;
static constexpr double kCellBottomMargin = 8;
static constexpr double kMinimumIconSize = 22;

// A silhouette of the icon in black, as MultiEffect's brightness -1 makes it.
class IconSilhouette : public ui::View
{
public:
    IconSilhouette()
    {
        icon = addChild<ui::ThemeIcon>();
        icon->setColor(ui::Color {0, 0, 0, 255});
    }

    ui::ThemeIcon* icon;

protected:
    void layout() override
    {
        const ui::RectF rect = localRect();
        const double size = icon->sizeHint().width;
        icon->setGeometry({centeredStart(rect.width, size), centeredStart(rect.height, size), size, size});
    }
};

// The name in black at (8, 8), for the soft halo around it.
class LabelHalo : public ui::View
{
public:
    LabelHalo()
    {
        label = addChild<ui::Label>();
        label->setWrap(true);
        label->setTextAlignment(ui::Label::TextAlignment::Center);
        label->setColor(ui::Color {0, 0, 0, 255});
        label->setMaximumLineCount(2);
    }

    ui::Label* label;

protected:
    void layout() override
    {
        label->setGeometry({8, 8, std::max(0.0, geometry().width - 16), std::max(0.0, geometry().height - 16)});
    }
};

// One item's cell on the desktop.
class DesktopItem : public IconItem
{
public:
    explicit DesktopItem(DesktopGridView* view)
        : IconItem(view, 4, 1)
        , m_desktop(view)
    {
        m_shadow = addChild<ui::BlurredCopy>(16, 0.5);
        m_silhouette = m_shadow->setContent<IconSilhouette>();
        addIconViews();
        m_halo = addChild<ui::FastBlurredCopy>(6);
        m_haloContent = m_halo->setContent<LabelHalo>();
        m_raised = addChild<ui::Label>();
        addLabelView();
        for (ui::Label* label : {m_raised, m_label, m_haloContent->label}) {
            label->setWrap(true);
            label->setTextAlignment(ui::Label::TextAlignment::Center);
            label->setMaximumLineCount(2);
            label->setElide(ui::Label::Elide::Right);
        }
        m_raised->setColor(ui::Color {0, 0, 0, 51});
        m_raised->setSubpixelAntialiasing(false);
        m_haloContent->label->setSubpixelAntialiasing(false);
    }

protected:
    void bindItem(const FileItem&, int row) override
    {
        FolderModel& model = m_desktop->model();
        setVisible(m_desktop->iconsVisible() && !model.isBlank(row));
        m_silhouette->icon->setIconName(m_icon->iconName());
        m_silhouette->icon->setSize(m_desktop->iconSize());

        const double pixelSize = m_desktop->labelPixelSize();
        const double labelWidth = geometry().width - kLabelInset * 2;
        const std::string text = m_desktop->labelText(model.displayName(row), labelWidth);
        for (ui::Label* label : {m_raised, m_label, m_haloContent->label}) {
            label->setText(text);
            label->setFontSize(pixelSize);
            label->setWeight(600);
        }
        m_haloContent->requestLayout();
        m_halo->contentChanged();
        m_label->setColor(m_selected ? style::selectedTextColor() : ui::Color::rgb(0xffffff));
        // The raised style draws the name and its copy in greyscale; only the plain
        // text on the selection takes subpixel glyphs.
        m_label->setSubpixelAntialiasing(m_selected);
        const bool labelShown = m_label->isVisible();
        // Text.Raised: the name in the style colour a pixel lower, unless selected.
        m_raised->setVisible(labelShown && !m_selected);
        m_raised->setOpacity(m_hidden ? 0.8 : 1.0);
        m_halo->setVisible(labelShown && !m_selected);
        m_halo->setOpacity(m_hidden ? 0.35 : 0.65);
    }

    void layout() override
    {
        const double width = geometry().width;
        const double iconSize = m_desktop->iconSize();
        const double iconItemWidth = width - 24;
        layoutIcon({centeredStart(width, iconItemWidth), kIconTopMargin, iconItemWidth, iconSize},
                   std::max(16.0, std::round(iconSize / 4)));
        m_shadow->setGeometry({m_iconItem.x, m_iconItem.y + 1, m_iconItem.width, m_iconItem.height});
        m_shadow->setOpacity(0.15 * (m_hidden ? 0.5 : 1.0));
        m_silhouette->requestLayout();
        m_silhouette->icon->setVisible(!m_thumbnail->isVisible());
        m_shadow->contentChanged();

        const double labelWidth = width - kLabelInset * 2;
        const double labelHeight = m_label->heightForWidth(labelWidth);
        const ui::RectF label {centeredStart(width, labelWidth), m_iconItem.y + iconSize + kLabelSpacing, labelWidth,
                               labelHeight};
        m_label->setGeometry(label);
        m_raised->setGeometry({label.x, label.y + 1, label.width, label.height});
        m_halo->setGeometry({label.x - 8, label.y - 8 + 1, label.width + 16, label.height + 16});
    }

private:
    DesktopGridView* m_desktop;
    ui::BlurredCopy* m_shadow;
    IconSilhouette* m_silhouette;
    ui::FastBlurredCopy* m_halo;
    LabelHalo* m_haloContent;
    ui::Label* m_raised;
};

DesktopGridView::DesktopGridView(FolderModel& model)
    : IconGridView(model)
    , m_positioner(model)
{
    setName("DesktopGridView");
    setDesktopStyle(true);
    setScrollDirections(false, false);
    m_subscriptions.push_back(m_positioner.changed.connect([this] {
        contentView()->requestLayout();
        updateItems();
    }));
    m_subscriptions.push_back(model.move.connect([this](int x, int y, const std::vector<std::string>& urls) {
        moveToCell(x, y, urls);
    }));
}

DesktopGridView::~DesktopGridView() = default;

void DesktopGridView::setIconsVisible(bool visible)
{
    if (m_iconsVisible == visible)
        return;
    m_iconsVisible = visible;
    updateItems();
}

std::unique_ptr<IconItem> DesktopGridView::makeItem()
{
    return std::make_unique<DesktopItem>(this);
}

double DesktopGridView::labelPixelSize() const
{
    // Text steps up once per 32 px of icon, about a fifth of the icon.
    return std::min(14.0, 12 + std::floor(std::max(0.0, m_iconSize - 64) / 32));
}

double DesktopGridView::labelHeight() const
{
    const ui::Theme& theme = ui::Theme::current();
    const std::string family = theme.fontFamily.empty() ? std::string("sans-serif") : theme.fontFamily;
    // Unrounded.
    const ui::FontMetrics metrics(ui::FontSpec {family, labelPixelSize() * 3 / 4, true}, 1, false);
    return metrics.heightF();
}

std::string DesktopGridView::labelText(const std::string& name, double width) const
{
    ui::Label measure(name);
    measure.setTextLayoutMode(textLayoutMode());
    measure.setFontSize(labelPixelSize());
    measure.setWeight(600);
    const auto advance = [&measure](const std::string& text) {
        measure.setText(text);
        return measure.sizeHint().width;
    };
    // Code point boundaries of the UTF-8 name.
    std::vector<size_t> bounds;
    for (size_t i = 0; i < name.size(); ++i) {
        if ((static_cast<unsigned char>(name[i]) & 0xc0) != 0x80)
            bounds.push_back(i);
    }
    bounds.push_back(name.size());
    const auto prefix = [&](size_t count) { return name.substr(0, bounds[count]); };

    const auto elideMiddle = [&](const std::string& text, double room) {
        if (advance(text) <= room)
            return text;
        std::vector<size_t> points;
        for (size_t i = 0; i < text.size(); ++i) {
            if ((static_cast<unsigned char>(text[i]) & 0xc0) != 0x80)
                points.push_back(i);
        }
        points.push_back(text.size());
        const size_t count = points.size() - 1;
        std::string best = "…";
        for (size_t keep = count; keep > 0; --keep) {
            const size_t head = (keep + 1) / 2;
            const size_t tail = keep - head;
            const std::string candidate = text.substr(0, points[head]) + "…" + text.substr(points[count - tail]);
            if (advance(candidate) <= room) {
                best = candidate;
                break;
            }
        }
        return best;
    };

    // Measured advances can round below what the label lays out.
    const double room = width - 2;
    if (room <= 0 || advance(name) <= room)
        return name;

    const size_t count = bounds.size() - 1;
    size_t fit = 0;
    size_t low = 1;
    size_t high = count;
    while (low <= high) {
        const size_t middle = (low + high) / 2;
        if (advance(prefix(middle)) <= room) {
            fit = middle;
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    if (fit == 0)
        return elideMiddle(name, room);

    // Break after a word, as wrapping would, unless one word fills the line.
    size_t breakAt = fit;
    if (name[bounds[fit]] != ' ') {
        for (size_t i = fit; i > 0; --i) {
            const char c = name[bounds[i - 1]];
            if (c == ' ' || c == '-' || c == '_') {
                breakAt = i;
                break;
            }
        }
    }
    std::string first = prefix(breakAt);
    while (!first.empty() && (first.back() == ' ' || first.back() == '\t'))
        first.pop_back();
    std::string rest = name.substr(bounds[breakAt]);
    while (!rest.empty() && (rest.front() == ' ' || rest.front() == '\t'))
        rest.erase(rest.begin());
    if (advance(rest) > room)
        rest = elideMiddle(rest, room);
    return first + "\n" + rest;
}

void DesktopGridView::setIconSize(double size)
{
    size = std::clamp(size, kMinimumIconSize, m_maximumIconSize);
    if (m_iconSize == size)
        return;
    m_iconSize = size;
    requestLayout();
    if (onIconSizeChanged)
        onIconSizeChanged();
}

void DesktopGridView::setWorkArea(const ui::RectF& area)
{
    if (m_workArea == area)
        return;
    m_workArea = area;
    requestLayout();
}

void DesktopGridView::setIconCorner(IconCorner corner)
{
    if (m_iconCorner == corner)
        return;
    m_iconCorner = corner;
    m_positioner.setCorner(static_cast<Positioner::Corner>(corner));
    requestLayout();
}

void DesktopGridView::updateMetrics()
{
    m_cellWidth = std::ceil(std::max(88.0, m_iconSize + 44));
    m_cellHeight = std::ceil(kIconTopMargin + m_iconSize + kLabelSpacing + labelHeight() * 2 + kCellBottomMargin);

    // The work area, or the whole view before the compositor sends one.
    const ui::RectF area = m_workArea.width > 0 ? m_workArea : localRect();
    const double availableWidth = std::max(0.0, area.width - 24);
    const double availableHeight = std::max(0.0, area.height - 24);
    const double width = std::floor(availableWidth / m_cellWidth) * m_cellWidth;
    const double height = std::floor(availableHeight / m_cellHeight) * m_cellHeight;
    // Anchored to the chosen corner of the work area, 12 px in.
    double gridX, gridY;
    switch (m_iconCorner) {
    case IconCorner::TopLeft:
        gridX = area.x + 12;
        gridY = area.y + 12;
        break;
    case IconCorner::BottomLeft:
        gridX = area.x + 12;
        gridY = area.y + area.height - 12 - height;
        break;
    case IconCorner::BottomRight:
        gridX = area.x + area.width - 12 - width;
        gridY = area.y + area.height - 12 - height;
        break;
    case IconCorner::TopRight:
    default:
        gridX = area.x + area.width - 12 - width;
        gridY = area.y + 12;
        break;
    }
    m_grid = {gridX, gridY, width, height};
    m_columns = std::max(1, int(std::floor(width / m_cellWidth)));
    m_rows = std::max(1, int(std::floor(height / m_cellHeight)));
    m_positioner.setGrid(m_columns, m_rows);
}

void DesktopGridView::layout()
{
    updateMetrics();
    FolderView::layout();
}

ui::RectF DesktopGridView::cellRect(int cell) const
{
    // Flowing top to bottom: a stripe is a column.
    const int column = cell / std::max(1, m_rows);
    const int row = cell % std::max(1, m_rows);
    return {m_grid.x + column * m_cellWidth, m_grid.y + row * m_cellHeight, m_cellWidth, m_cellHeight};
}

int DesktopGridView::indexAt(ui::PointF content) const
{
    const double x = content.x - m_grid.x;
    const double y = content.y - m_grid.y;
    if (x < 0 || y < 0 || x >= m_grid.width || y >= m_grid.height)
        return -1;
    const int column = int(std::floor(x / m_cellWidth));
    const int row = int(std::floor(y / m_cellHeight));
    const int cell = column * m_rows + row;
    return m_positioner.isBlank(cell) ? -1 : cell;
}

int DesktopGridView::rowOf(int index) const
{
    return index < 0 ? -1 : m_positioner.rowAt(index);
}

void DesktopGridView::selectRange(int from, int to)
{
    std::vector<int> rows;
    for (int cell = std::min(from, to); cell <= std::max(from, to); ++cell) {
        const int row = m_positioner.rowAt(cell);
        if (row >= 0)
            rows.push_back(row);
    }
    if (!rows.empty())
        model().updateSelection(rows, false, false);
}

int DesktopGridView::cellAt(ui::PointF content) const
{
    if (m_cellWidth <= 0 || m_cellHeight <= 0)
        return -1;
    // Clamped into the grid: a press in the margin takes the nearest edge cell.
    const int column = int(std::clamp(std::floor((content.x - m_grid.x) / m_cellWidth), 0.0, double(m_columns - 1)));
    const int row = int(std::clamp(std::floor((content.y - m_grid.y) / m_cellHeight), 0.0, double(m_rows - 1)));
    return column * m_rows + row;
}

void DesktopGridView::rectangleSelect(const ui::RectF& band)
{
    std::vector<int> indices;
    for (const auto& [cell, item] : m_items) {
        if (m_positioner.isBlank(cell) || model().isBlank(rowOf(cell)))
            continue;
        const ui::RectF rect = cellRect(cell);
        if (rect.x >= band.x + band.width || band.x >= rect.x + rect.width || rect.y >= band.y + band.height
            || band.y >= rect.y + rect.height)
            continue;
        if (item->intersectsRect({band.x - rect.x, band.y - rect.y, band.width, band.height}))
            indices.push_back(cell);
    }
    std::sort(indices.begin(), indices.end());
    applyRectangleSelection(indices);
}

void DesktopGridView::updateItems()
{
    const int cells = std::min(m_positioner.cellCount(), m_columns * m_rows);
    recycleItems([this, cells](int cell) { return cell < cells && !m_positioner.isBlank(cell); });
    for (int cell = 0; cell < cells; ++cell) {
        if (!m_positioner.isBlank(cell))
            showItem(cell);
    }
}

bool DesktopGridView::navigate(const ui::KeyEvent& event)
{
    if ((event.key == '=' || event.key == '+') && (event.modifiers & ui::ControlModifier)) {
        setIconSize(m_iconSize + 8);
        return true;
    }
    if (event.key == '-' && (event.modifiers & ui::ControlModifier)) {
        setIconSize(m_iconSize - 8);
        return true;
    }
    // Flowing top to bottom, the positioner's rows are the view's columns.
    Positioner::Direction direction;
    switch (event.key) {
    case ui::key::Up:
        direction = Positioner::Direction::Left;
        break;
    case ui::key::Down:
        direction = Positioner::Direction::Right;
        break;
    case ui::key::Left:
        direction = Positioner::Direction::Up;
        break;
    case ui::key::Right:
        direction = Positioner::Direction::Down;
        break;
    default:
        return false;
    }
    if (isRenaming())
        return true;
    const bool forward = direction == Positioner::Direction::Right || direction == Positioner::Direction::Down;
    const int next = m_positioner.nearestItem(keyboardIndex(forward), direction);
    if (next != -1) {
        setCurrentIndex(next);
        updateSelection(event.modifiers);
    }
    return true;
}

bool DesktopGridView::scroll(const ui::ScrollEvent& event)
{
    if (event.modifiers & ui::ControlModifier) {
        setIconSize(m_iconSize + ((event.deltaY < 0 || event.value120Y < 0) ? 8 : -8));
        return true;
    }
    return true;
}

ui::PointF DesktopGridView::cellNear(ui::PointF point) const
{
    // The nearest cell to an icon's top left: half a cell of travel moves it.
    const double column = std::clamp(std::round((point.x - m_grid.x) / m_cellWidth), 0.0, double(m_columns - 1));
    const double row = std::clamp(std::round((point.y - m_grid.y) / m_cellHeight), 0.0, double(m_rows - 1));
    return {column, row};
}

void DesktopGridView::moveToCell(int x, int y, const std::vector<std::string>& urls)
{
    // A drag that started on an icon keeps the selection's shape: every icon
    // moves as many cells as the grabbed one. A drop from elsewhere lands on
    // the cell under the pointer.
    const int anchor = model().dragging() ? dragIndex() : -1;
    const auto cellOf = [this](int cell) {
        return ui::PointF {double(cell / std::max(1, m_rows)), double(cell % std::max(1, m_rows))};
    };
    ui::PointF target;
    ui::PointF anchorCell;
    if (anchor >= 0) {
        anchorCell = cellOf(anchor);
        const ui::RectF anchorRect = cellRect(anchor);
        const ui::PointF grab {dragGrab().x - anchorRect.x, dragGrab().y - anchorRect.y};
        target = cellNear({x - grab.x, y - grab.y});
    } else {
        target = {std::clamp(std::floor((x - m_grid.x) / m_cellWidth), 0.0, double(m_columns - 1)),
                  std::clamp(std::floor((y - m_grid.y) / m_cellHeight), 0.0, double(m_rows - 1))};
    }
    std::vector<std::pair<int, int>> moves;
    for (const std::string& url : urls) {
        const int from = m_positioner.cellOfUrl(url);
        if (from < 0)
            continue;
        const ui::PointF cell = cellOf(from);
        int column = int(anchor >= 0 ? target.x + (cell.x - anchorCell.x) : target.x);
        int row = int(anchor >= 0 ? target.y + (cell.y - anchorCell.y) : target.y);
        column = std::clamp(column, 0, m_columns - 1);
        row = std::clamp(row, 0, m_rows - 1);
        moves.emplace_back(from, column * m_rows + row);
    }
    if (!moves.empty())
        m_positioner.move(moves);
}

} // namespace fm
