// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/colors.h"
#include "views/listview.h"
#include "core/preferences.h"
#include "core/thumbnails.h"
#include "model/foldermodel.h"
#include "views/style.h"

#include "base/i18n.h"
#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/icon_image.h"
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
using Lingmo::i18n::tr;

static constexpr double kSmallSpacing = 6;

double ListColumns::rowHeight()
{
    return std::max(28.0, std::round(style::fontHeight() + 12));
}

double ListColumns::headerHeight()
{
    return std::max(26.0, std::round(style::fontHeight() + 10));
}

static double originRequired(const ListColumns& columns)
{
    return ListColumns::minimumNameWidth + (columns.trash ? columns.originWidth : 0);
}

bool ListColumns::showOrigin() const
{
    return trash && availableWidth >= originRequired(*this);
}

bool ListColumns::showDate() const
{
    return (!trash || showOrigin()) && availableWidth >= originRequired(*this) + dateWidth;
}

bool ListColumns::showSize() const
{
    return showDate() && availableWidth >= originRequired(*this) + dateWidth + sizeWidth;
}

bool ListColumns::showType() const
{
    return !trash && showSize() && availableWidth >= minimumNameWidth + dateWidth + sizeWidth + typeWidth;
}

double ListColumns::nameWidth() const
{
    return availableWidth - (showOrigin() ? originWidth : 0) - (showDate() ? dateWidth : 0)
            - (showSize() ? sizeWidth : 0) - (showType() ? typeWidth : 0);
}

// The columns that can be resized, by their leading edge.
enum Column {
    OriginColumn,
    DateColumn,
    SizeColumn,
    TypeColumn,
};

// The column titles, which sort, and the handles
// between them, which resize; it blocks the rows beneath it.
class ListHeader : public ui::View
{
public:
    explicit ListHeader(FolderListView* list)
        : m_list(list)
    {
        setName("listHeader");
        for (int i = 0; i < 5; ++i) {
            m_titles[i] = addChild<ui::Label>();
            m_titles[i]->setElide(ui::Label::Elide::Right);
            m_chevrons[i] = addChild<ui::Icon>("chevron-up", 12);
        }
    }

    bool acceptsPointer() const override { return true; }
    ui::CursorShape cursorShape() const override
    {
        return handleAt(m_hover) >= 0 || m_dragging >= 0 ? ui::CursorShape::ResizeHorizontal
                                                         : ui::CursorShape::Default;
    }
    void setBackground(std::optional<ui::Color> color) { m_background = color; }

protected:
    struct Cell
    {
        double x;
        double width;
        bool visible;
        std::string title;
        int sortMode;
        bool right;
    };

    std::vector<Cell> cells() const
    {
        const ListColumns& c = m_list->columns();
        return {
            {0, c.nameWidth(), true, tr("Name"), 0, false},
            {c.originX(), c.originWidth, c.showOrigin(), tr("Original Location"), 7, false},
            {c.dateX(), c.dateWidth, c.showDate(), c.trash ? tr("Date Deleted") : tr("Date Modified"), c.dateSortMode(),
             false},
            {c.sizeX(), c.sizeWidth, c.showSize(), tr("Size"), 1, true},
            {c.typeX(), c.typeWidth, c.showType(), tr("Kind"), 3, false},
        };
    }

    // The resize handle under a point: 9 px around a column's leading edge.
    int handleAt(ui::PointF point) const
    {
        const ListColumns& c = m_list->columns();
        const double left = m_list->leftMargin();
        const std::pair<double, bool> edges[] = {
            {c.originX(), c.showOrigin()}, {c.dateX(), c.showDate()},
            {c.sizeX(), c.showSize()},     {c.typeX(), c.showType()},
        };
        for (int i = 0; i < 4; ++i) {
            if (edges[i].second && std::abs(point.x - (left + edges[i].first)) <= 4.5)
                return i;
        }
        return -1;
    }

    void layout() override
    {
        const ui::Theme& theme = ui::Theme::current();
        const ListColumns& c = m_list->columns();
        const int sortMode = m_list->model().sortMode();
        const bool descending = m_list->model().sortDesc();
        const double height = geometry().height;
        const auto all = cells();
        for (size_t i = 0; i < all.size(); ++i) {
            const Cell& cell = all[i];
            ui::Label* title = m_titles[i];
            ui::Icon* chevron = m_chevrons[i];
            const bool sorted = sortMode == cell.sortMode;
            title->setVisible(cell.visible);
            chevron->setVisible(cell.visible && sorted);
            if (!cell.visible)
                continue;
            title->setText(cell.title);
            title->setFontSize(std::round(theme.fontPointSize * 0.92 * 100) / 100 * 4 / 3);
            title->setWeight(sorted ? 600 : 400);
            const ui::Color color = sorted ? theme.text : theme.disabledText;
            title->setColor(color);
            chevron->setIconName(descending ? "chevron-down" : "chevron-up");
            chevron->setColor(color);

            const double spacing = 4;
            const double room = cell.width - c.cellPadding * 2;
            const double chevronWidth = sorted ? 12 + spacing : 0;
            const ui::SizeF hint = title->sizeHint();
            const double titleWidth = std::min(hint.width, room - chevronWidth);
            const double rowWidth = std::min(hint.width + chevronWidth, room);
            const double rowX = cell.right ? std::max(c.cellPadding, cell.width - c.cellPadding - rowWidth)
                                           : c.cellPadding;
            const double x = m_list->leftMargin() + cell.x + rowX;
            title->setGeometry({x, centeredStart(height, hint.height), std::max(0.0, titleWidth), hint.height});
            chevron->setGeometry({x + titleWidth + spacing, centeredStart(height, 12), 12, 12});
        }
    }

    void paint(ui::Painter& painter) override
    {
        if (m_background)
            painter.fillRect(localRect(), *m_background);
        const double left = m_list->leftMargin();
        const double right = m_list->rightMargin();
        painter.drawRoundedRect({left, geometry().height - 1, geometry().width - left - right, 1}, {},
                                ui::color(ui::ColorId::HeaderSeparator));

        // The handles' dividers, snapped so every one is as wide.
        const ListColumns& c = m_list->columns();
        const std::pair<double, bool> edges[] = {
            {c.originX(), c.showOrigin()}, {c.dateX(), c.showDate()},
            {c.sizeX(), c.showSize()},     {c.typeX(), c.showType()},
        };
        const double height = geometry().height;
        for (int i = 0; i < 4; ++i) {
            if (!edges[i].second)
                continue;
            const bool active = handleAt(m_hover) == i || m_dragging == i;
            const double x = left + edges[i].first - 4.5;
            const double lineHeight = height * 0.5;
            painter.drawRoundedRect({x + centeredStart(9, 1), centeredStart(height, lineHeight), 1, lineHeight}, {},
                                    style::textAlpha(active ? 0.3 : 0.12));
        }
    }

    void hoverMove(const ui::PointerEvent& event) override
    {
        m_hover = event.position;
        requestPaint();
    }

    void pointerLeave() override
    {
        m_hover = {-100, -100};
        requestPaint();
    }

    void pointerMove(const ui::PointerEvent& event) override
    {
        m_hover = event.position;
        if (m_dragging < 0) {
            requestPaint();
            return;
        }
        const ListColumns& c = m_list->columns();
        const double widths[] = {c.originWidth, c.dateWidth, c.sizeWidth, c.typeWidth};
        const double delta = m_startX - event.position.x;
        const double maximum = std::floor(widths[m_dragging] + c.nameWidth() - ListColumns::minimumNameWidth);
        m_list->setColumnWidth(m_dragging,
                               std::max(ListColumns::minimumWidth, std::min(maximum, std::round(m_startWidth + delta))),
                               false);
    }

    void pointerPress(const ui::PointerEvent& event) override
    {
        if (event.button != ui::MouseButton::Left)
            return;
        const int handle = handleAt(event.position);
        if (handle >= 0) {
            const ListColumns& c = m_list->columns();
            const double widths[] = {c.originWidth, c.dateWidth, c.sizeWidth, c.typeWidth};
            if (event.clickCount == 2) {
                const double defaults[] = {200, 180, 96, 150};
                m_list->setColumnWidth(handle, defaults[handle], true);
                return;
            }
            m_dragging = handle;
            m_startWidth = widths[handle];
            m_startX = event.position.x;
            return;
        }
        m_pressed = event.position;
    }

    void pointerRelease(const ui::PointerEvent& event) override
    {
        if (m_dragging >= 0) {
            const ListColumns& c = m_list->columns();
            const double widths[] = {c.originWidth, c.dateWidth, c.sizeWidth, c.typeWidth};
            m_list->setColumnWidth(m_dragging, widths[m_dragging], true);
            m_dragging = -1;
            requestPaint();
            return;
        }
        if (!m_pressed)
            return;
        const ui::PointF pressed = *m_pressed;
        m_pressed.reset();
        const double x = event.position.x - m_list->leftMargin();
        const double px = pressed.x - m_list->leftMargin();
        for (const Cell& cell : cells()) {
            if (cell.visible && x >= cell.x && x < cell.x + cell.width && px >= cell.x && px < cell.x + cell.width) {
                m_list->sortBy(cell.sortMode);
                return;
            }
        }
    }

    bool scroll(const ui::ScrollEvent&) override { return false; }

private:
    FolderListView* m_list;
    ui::Label* m_titles[5];
    ui::Icon* m_chevrons[5];
    std::optional<ui::Color> m_background;
    ui::PointF m_hover {-100, -100};
    std::optional<ui::PointF> m_pressed;
    int m_dragging = -1;
    double m_startWidth = 0;
    double m_startX = 0;
};

// Neither side of a thumbnail is drawn shorter than this share of the box.
static constexpr double kThumbnailMinimumShare = 1.0 / 3;

// The largest thumbnail the cache keeps.
static constexpr double kThumbnailMaximumRequest = 1024;

// One item's row in the list.
class ListRow : public ui::View
{
public:
    explicit ListRow(FolderListView* list)
        : m_list(list)
    {
        m_icon = addChild<ui::IconImage>();
        m_thumbnail = addChild<ui::ImageView>();
        m_thumbnail->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
        m_thumbnail->setVisible(false);
        m_emblem = addChild<ui::ThemeIcon>("emblem-symbolic-link", 12);
        m_emblem->setImageProviderLookup(true);
        m_name = addChild<ui::Label>();
        m_name->setElide(ui::Label::Elide::Middle);
        for (ui::Label*& label : m_columns) {
            label = addChild<ui::Label>();
            label->setElide(ui::Label::Elide::Right);
        }
        m_columns[1]->setTextAlignment(ui::Label::TextAlignment::Right);
        m_columns[0]->setElide(ui::Label::Elide::Middle);
    }

    int index() const { return m_index; }

    void bind(int index)
    {
        if (index != m_index) {
            m_index = index;
            m_imageKey.clear();
            m_image = {};
        }
        FolderModel& model = m_list->model();
        const FileItemPtr item = model.item(index);
        if (!item)
            return;
        m_item = item;
        m_selected = model.isSelected(index);
        m_previousSelected = m_selected && index > 0 && model.isSelected(index - 1);
        m_nextSelected = m_selected && index + 1 < model.count() && model.isSelected(index + 1);
        m_dropTarget = m_list->isDropTarget(index);

        const ui::Theme& theme = ui::Theme::current();
        const ui::Color primary = m_selected ? style::selectedTextColor() : theme.text;
        const ui::Color secondary = m_selected ? style::selectedTextColor() : theme.disabledText;
        const double textOpacity = item->isHidden ? 0.6 : 1.0;

        std::string icon = item->iconName;
        if (icon.empty() || ui::themeIconPath(icon, 96).empty())
            icon = item->isDir ? "folder" : "unknown";
        m_icon->setName(icon);
        m_emblem->setVisible(item->isLink);
        m_name->setText(item->name);
        m_name->setColor(primary);
        m_name->setOpacity(textOpacity);
        m_name->setVisible(m_list->renamingIndex() != index);

        const ListColumns& c = m_list->columns();
        const std::string texts[] = {
            c.showOrigin() ? model.originalPathText(index) : std::string(),
            model.fileSizeText(index),
            model.fileTypeText(index),
            c.trash ? model.deletedDateText(index) : model.modifiedDateText(index),
        };
        for (int i = 0; i < 4; ++i) {
            m_columns[i]->setText(texts[i]);
            m_columns[i]->setColor(secondary);
            m_columns[i]->setOpacity(textOpacity);
        }
        m_wantsThumbnail = model.showsThumbnail(index);
        requestLayout();
        requestPaint();
    }

    ui::RectF labelBox() const { return m_name->geometry(); }

protected:
    void layout() override
    {
        const ListColumns& c = m_list->columns();
        const double height = geometry().height;
        const double size = ListColumns::iconSize;
        const ui::RectF icon {c.cellPadding, centeredStart(height, size), size, size};
        m_icon->setGeometry(icon);
        // The theme's 96 px picture, drawn small, picks the full-colour variant.
        const ui::Window* window = this->window();
        const double scale = window ? window->scale() : 1;
        m_icon->setPixels(int(std::round(96 * std::max(1.0, std::ceil(scale)))));
        const double iconOpacity = m_item && m_item->isHidden ? 0.5 : 1.0;
        m_icon->setOpacity(iconOpacity);
        layoutThumbnail(icon, scale);
        m_icon->setVisible(!m_thumbnail->isVisible());
        m_emblem->setGeometry({icon.x + icon.width - 12 + 2, icon.y + icon.height - 12 + 2, 12, 12});

        const double nameX = icon.x + size + kSmallSpacing + 2;
        const double nameRight = c.nameWidth() - c.cellPadding;
        const ui::SizeF nameHint = m_name->sizeHint();
        m_name->setGeometry({nameX, centeredStart(height, nameHint.height), std::max(0.0, nameRight - nameX),
                             nameHint.height});

        const std::pair<double, double> cells[] = {
            {c.originX(), c.originWidth},
            {c.sizeX(), c.sizeWidth},
            {c.typeX(), c.typeWidth},
            {c.dateX(), c.dateWidth},
        };
        const bool shown[] = {c.showOrigin(), c.showSize(), c.showType(), c.showDate()};
        for (int i = 0; i < 4; ++i) {
            ui::Label* label = m_columns[i];
            label->setVisible(shown[i]);
            const ui::SizeF hint = label->sizeHint();
            label->setGeometry({cells[i].first + c.cellPadding, centeredStart(height, hint.height),
                                std::max(0.0, cells[i].second - c.cellPadding * 2), hint.height});
        }
    }

    void paint(ui::Painter& painter) override
    {
        const ui::Theme& theme = ui::Theme::current();
        const ui::RectF rect = localRect();
        const double radius = 6;
        if (m_dropTarget) {
            const ui::Color color = m_list->dropAllowed() ? theme.highlight : style::kDanger;
            painter.drawRoundedRect(rect, {radius, 0, ui::Corners::Circular}, style::withAlpha(color, 0.16), 2, color);
            return;
        }

        ui::Color fill {0, 0, 0, 0};
        if (m_selected)
            fill = theme.highlight;
        else if (m_index % 2 == 1)
            fill = ui::color(FmColor::AlternateRow);
        if (fill.a == 0)
            return;

        // Adjacent selected rows read as one block: only the outer corners round.
        ui::Path path;
        const double top = m_previousSelected ? 0 : radius;
        const double bottom = m_nextSelected ? 0 : radius;
        const double x = rect.x;
        const double y = rect.y;
        const double w = rect.width;
        const double h = rect.height;
        path.moveTo(x + top, y);
        path.lineTo(x + w - top, y);
        if (top > 0)
            path.arcTo({x + w - 2 * top, y, 2 * top, 2 * top}, 90, -90);
        path.lineTo(x + w, y + h - bottom);
        if (bottom > 0)
            path.arcTo({x + w - 2 * bottom, y + h - 2 * bottom, 2 * bottom, 2 * bottom}, 0, -90);
        path.lineTo(x + bottom, y + h);
        if (bottom > 0)
            path.arcTo({x, y + h - 2 * bottom, 2 * bottom, 2 * bottom}, 270, -90);
        path.lineTo(x, y + top);
        if (top > 0)
            path.arcTo({x, y, 2 * top, 2 * top}, 180, -90);
        path.close();
        painter.fillPath(path, fill);
    }

private:
    void layoutThumbnail(const ui::RectF& box, double scale)
    {
        if (!m_wantsThumbnail || !m_item) {
            m_thumbnail->setVisible(false);
            return;
        }
        // Twice the box, as the row's sourceSize asks.
        const int width = int(std::round(box.width * 2 * scale));
        const int height = int(std::round(box.height * 2 * scale));
        const std::string key = m_item->path + '\n' + std::to_string(m_item->modified);
        if (key != m_imageKey || m_thumbnailBox != ui::SizeF(width, height)) {
            m_imageKey = key;
            m_thumbnailBox = ui::SizeF(width, height);
            m_thumbnailRequest = m_thumbnailBox;
            m_image = {};
            loadThumbnail();
        }
        if (m_image.isNull()) {
            m_thumbnail->setVisible(false);
            return;
        }

        // PreserveAspectFit, centred in the box, then widened or heightened
        // to the minimum: a picture too narrow or too flat for it is cropped
        // about its centre.
        const double fit = std::min(box.width / m_image.width(), box.height / m_image.height());
        const double fittedWidth = m_image.width() * fit;
        const double fittedHeight = m_image.height() * fit;
        const double w = std::max(fittedWidth, box.width * kThumbnailMinimumShare);
        const double h = std::max(fittedHeight, box.height * kThumbnailMinimumShare);
        m_thumbnail->setGeometry({box.x + (box.width - w) / 2, box.y + (box.height - h) / 2, w, h});

        // A cropped picture is drawn larger than it fits the box: a larger
        // thumbnail keeps it sharp, up to the largest the cache has.
        const double crop = std::max(w / fittedWidth, h / fittedHeight);
        const double limit = kThumbnailMaximumRequest / std::max(m_thumbnailBox.width, m_thumbnailBox.height);
        const double grow = std::max(1.0, std::min(crop, limit));
        const ui::SizeF request(std::round(m_thumbnailBox.width * grow), std::round(m_thumbnailBox.height * grow));
        if (request != m_thumbnailRequest) {
            m_thumbnailRequest = request;
            loadThumbnail();
        }

        m_thumbnail->setCornerRadius(3);
        m_thumbnail->setOpacity(m_item->isHidden ? 0.5 : 1.0);
        m_thumbnail->setVisible(true);
    }

    // The picture shown stays until the requested size arrives.
    void loadThumbnail()
    {
        const int width = int(m_thumbnailRequest.width);
        const int height = int(m_thumbnailRequest.height);
        const ui::Bitmap cached = thumbnails::cached(m_item->path, m_item->modified, width, height);
        if (!cached.isNull()) {
            m_image = cached;
            applyImage();
            return;
        }

        const std::weak_ptr<bool> alive = m_alive;
        const std::string key = m_imageKey;
        thumbnails::load(m_item->path, m_item->modified, width, height,
                         [this, alive, key, width, height](const ui::Bitmap& image) {
                             if (alive.expired() || image.isNull() || key != m_imageKey
                                 || m_thumbnailRequest != ui::SizeF(width, height))
                                 return;
                             m_image = image;
                             applyImage();
                             requestLayout();
                         });
    }

    void applyImage()
    {
        if (m_image.isNull())
            return;
        const ui::Bitmap image = m_image.converted(ui::Bitmap::Format::Argb32Premultiplied);
        m_thumbnail->setPixels(image.width(), image.height(), reinterpret_cast<const uint32_t*>(image.bits()),
                               image.stride(), true, 1);
    }

    FolderListView* m_list;
    int m_index = -1;
    FileItemPtr m_item;
    bool m_selected = false;
    bool m_previousSelected = false;
    bool m_nextSelected = false;
    bool m_dropTarget = false;
    bool m_wantsThumbnail = false;
    std::string m_imageKey;
    ui::SizeF m_thumbnailBox;
    ui::SizeF m_thumbnailRequest;
    ui::Bitmap m_image;
    ui::IconImage* m_icon;
    ui::ImageView* m_thumbnail;
    ui::ThemeIcon* m_emblem;
    ui::Label* m_name;
    // Original location, size, kind, date.
    ui::Label* m_columns[4];
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

FolderListView::FolderListView(FolderModel& model)
    : FolderView(model)
{
    setName("FolderListView");
    loadColumns();
    m_header = addChild<ListHeader>(this);
    m_preferences = Preferences::instance().changed.connect([this] {
        loadColumns();
        requestLayout();
    });
}

FolderListView::~FolderListView() = default;

void FolderListView::loadColumns()
{
    const Preferences& preferences = Preferences::instance();
    m_columns.originWidth = preferences.listOriginColumnWidth();
    m_columns.dateWidth = preferences.listDateColumnWidth();
    m_columns.sizeWidth = preferences.listSizeColumnWidth();
    m_columns.typeWidth = preferences.listTypeColumnWidth();
}

double FolderListView::headerHeight() const
{
    return topInset() + ListColumns::headerHeight();
}

void FolderListView::sortBy(int mode)
{
    Preferences& preferences = Preferences::instance();
    const bool trash = m_columns.trash;
    const int current = trash ? preferences.trashSortMode() : preferences.sortMode();
    const bool currentDesc = trash ? preferences.trashSortDesc() : preferences.sortDesc();
    // Newest first is the useful default for dates.
    const bool descending = current == mode ? !currentDesc : mode == m_columns.dateSortMode();
    if (trash)
        preferences.setTrashSort(mode, descending);
    else
        preferences.setSort(mode, descending);
}

void FolderListView::setColumnWidth(int column, double width, bool save)
{
    switch (column) {
    case OriginColumn:
        m_columns.originWidth = width;
        break;
    case DateColumn:
        m_columns.dateWidth = width;
        break;
    case SizeColumn:
        m_columns.sizeWidth = width;
        break;
    case TypeColumn:
        m_columns.typeWidth = width;
        break;
    }
    if (save)
        Preferences::instance().setListColumnWidths(m_columns.originWidth, m_columns.dateWidth, m_columns.sizeWidth,
                                                    m_columns.typeWidth);
    requestLayout();
    m_header->requestLayout();
    m_header->requestPaint();
    updateItems();
}

void FolderListView::layout()
{
    m_columns.availableWidth = std::max(0.0, geometry().width - leftMargin() - rightMargin());
    m_columns.trash = model().isTrash();
    FolderView::layout();
    m_header->setBackground(topInset() > 0 ? std::nullopt : std::optional(ui::Theme::current().secondBackground));
    m_header->setGeometry({0, topInset(), geometry().width, ListColumns::headerHeight()});
    raiseChild(m_header);
    m_header->requestLayout();
}

double FolderListView::minimumContentHeight() const
{
    return std::max(geometry().height,
                    topMargin() + model().count() * ListColumns::rowHeight() + bottomMargin());
}

ui::RectF FolderListView::rowRect(int index) const
{
    return {leftMargin(), topMargin() + index * ListColumns::rowHeight(), m_columns.availableWidth,
            ListColumns::rowHeight()};
}

int FolderListView::indexAt(ui::PointF content) const
{
    const double y = content.y - topMargin();
    const double x = content.x - leftMargin();
    if (y < 0 || x < 0 || x > m_columns.availableWidth)
        return -1;
    const int index = int(std::floor(y / ListColumns::rowHeight()));
    return index < model().count() ? index : -1;
}

bool FolderListView::hitsItem(int index, ui::PointF content) const
{
    return indexAt(content) == index;
}

int FolderListView::dropRowAt(ui::PointF content) const
{
    // A file's row passes the drop on to the folder around it.
    const int index = indexAt(content);
    if (index < 0 || model().isBlank(index))
        return -1;
    const FileItemPtr item = model().item(index);
    return item && item->isDir ? index : -1;
}

void FolderListView::rectangleSelect(const ui::RectF& band)
{
    const double rowHeight = ListColumns::rowHeight();
    const double y = band.y - topMargin();
    std::vector<int> indices;
    const int first = std::max(0, int(std::floor(y / rowHeight)));
    const int last = std::min(model().count() - 1, int(std::floor((y + band.height) / rowHeight)));
    for (int i = first; i <= last; ++i)
        indices.push_back(i);
    applyRectangleSelection(indices);
}

std::optional<ui::RectF> FolderListView::labelRect(int index) const
{
    const auto found = m_rows.find(index);
    if (found == m_rows.end())
        return std::nullopt;
    const ui::RectF row = rowRect(index);
    const ui::RectF label = found->second->labelBox();
    const ui::PointF origin = fromContent({row.x + label.x, row.y + label.y});
    return ui::RectF {origin.x, origin.y, label.width, label.height};
}

void FolderListView::retranslate()
{
    FolderView::retranslate();
    m_header->requestLayout();
}

void FolderListView::revealIndex(int index)
{
    if (index < 0 || index >= model().count())
        return;
    const ui::RectF row = rowRect(index);
    const double visibleTop = scrollPosition().y + headerHeight();
    const double visibleBottom = scrollPosition().y + geometry().height - bottomMargin();
    if (row.y < visibleTop)
        setContentY(row.y - headerHeight());
    else if (row.y + row.height > visibleBottom)
        setContentY(row.y + row.height - geometry().height + bottomMargin());
}

void FolderListView::moveCurrentIndex(int index, uint32_t modifiers)
{
    if (model().count() == 0 || isRenaming())
        return;
    index = std::clamp(index, 0, model().count() - 1);
    if (!(modifiers & ui::ShiftModifier) || currentIndex() == -1)
        setAnchorIndex(index);
    setCurrentIndex(index);
    updateSelection(modifiers);
    revealIndex(index);
}

bool FolderListView::navigate(const ui::KeyEvent& event)
{
    const double rowHeight = ListColumns::rowHeight();
    const int page = std::max(1, int(std::floor((geometry().height - headerHeight()) / rowHeight)) - 1);
    switch (event.key) {
    case ui::key::Home:
        moveCurrentIndex(0, event.modifiers);
        return true;
    case ui::key::End:
        moveCurrentIndex(model().count() - 1, event.modifiers);
        return true;
    case ui::key::PageUp:
        moveCurrentIndex(std::max(0, keyboardIndex(false)) - page, event.modifiers);
        return true;
    case ui::key::PageDown:
        moveCurrentIndex(keyboardIndex(true) + page, event.modifiers);
        return true;
    case ui::key::Up:
        moveCurrentIndex(keyboardIndex(false) == -1 ? 0 : keyboardIndex(false) - 1, event.modifiers);
        return true;
    case ui::key::Down:
        moveCurrentIndex(keyboardIndex(true) + 1, event.modifiers);
        return true;
    default:
        return false;
    }
}

void FolderListView::updateItems()
{
    View* content = contentView();
    const double rowHeight = ListColumns::rowHeight();
    const int count = model().count();
    const double top = scrollPosition().y - topMargin();
    const double bottom = top + geometry().height;
    const int first = std::clamp(int(std::floor(top / rowHeight)), 0, count);
    const int last = std::clamp(int(std::ceil(bottom / rowHeight)), 0, count);

    for (auto it = m_rows.begin(); it != m_rows.end();) {
        if (it->first < first || it->first >= last) {
            it->second->setVisible(false);
            m_spare.push_back(it->second);
            it = m_rows.erase(it);
        } else {
            ++it;
        }
    }

    for (int index = first; index < last; ++index) {
        ListRow* row;
        if (const auto found = m_rows.find(index); found != m_rows.end()) {
            row = found->second;
        } else if (!m_spare.empty()) {
            row = m_spare.back();
            m_spare.pop_back();
            row->setVisible(true);
            m_rows.emplace(index, row);
        } else {
            row = content->addChild<ListRow>(this);
            m_rows.emplace(index, row);
        }
        row->setGeometry(rowRect(index));
        row->bind(index);
    }
}

void FolderListView::forEachItemView(const std::function<void(int index, ui::View* item)>& visit) const
{
    for (const auto& [index, row] : m_rows)
        visit(index, row);
}

std::optional<ui::RectF> FolderListView::dragRect(int index) const
{
    return rowRect(index);
}

void FolderListView::paintDragItem(int, ui::Painter&, const ui::RectF&)
{
}

} // namespace fm
