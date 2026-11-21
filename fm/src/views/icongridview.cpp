// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/icongridview.h"
#include "core/thumbnails.h"
#include "model/foldermodel.h"
#include "views/style.h"

#include "ui/cell_layout.h"
#include "ui/icon.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

namespace fm {

using ui::rounding::centeredStart;

// An icon name the theme has, else the fallback, as IconItem's fallback.
static std::string iconOrFallback(const std::string& name, const std::string& fallback, int size)
{
    if (!name.empty() && !ui::themeIconPath(name, size).empty())
        return name;
    return fallback;
}

static void addRoundedRect(ui::Path& path, const ui::RectF& rect, double radius)
{
    // Quadratic corners.
    const double r = std::min({radius, rect.width / 2, rect.height / 2});
    const double x = rect.x;
    const double y = rect.y;
    const double w = rect.width;
    const double h = rect.height;
    path.moveTo({x + r, y});
    path.lineTo({x + w - r, y});
    path.quadTo({x + w, y}, {x + w, y + r});
    path.lineTo({x + w, y + h - r});
    path.quadTo({x + w, y + h}, {x + w - r, y + h});
    path.lineTo({x + r, y + h});
    path.quadTo({x, y + h}, {x, y + h - r});
    path.lineTo({x, y + r});
    path.quadTo({x, y}, {x + r, y});
    path.close();
}

IconItem* IconGridView::itemFor(int index) const
{
    const auto found = m_items.find(index);
    return found != m_items.end() ? found->second : nullptr;
}

void IconGridView::recycleItems(const std::function<bool(int index)>& keep)
{
    for (auto it = m_items.begin(); it != m_items.end();) {
        if (!keep(it->first)) {
            it->second->setVisible(false);
            m_spare.push_back(it->second);
            it = m_items.erase(it);
        } else {
            ++it;
        }
    }
}

void IconGridView::showItem(int index)
{
    IconItem* item = itemFor(index);
    if (!item) {
        if (!m_spare.empty()) {
            item = m_spare.back();
            m_spare.pop_back();
            item->setVisible(true);
        } else {
            std::unique_ptr<IconItem> made = makeItem();
            item = made.get();
            contentView()->addChild(std::move(made));
        }
        m_items.emplace(index, item);
    }
    item->setGeometry(cellRect(index));
    item->bind(index, rowOf(index));
}

bool IconGridView::hitsItem(int index, ui::PointF content) const
{
    const ui::RectF cell = cellRect(index);
    if (IconItem* item = itemFor(index))
        return item->containsPoint({content.x - cell.x, content.y - cell.y});
    return false;
}

int IconGridView::dropRowAt(ui::PointF content) const
{
    // Only a folder's icon and name take a drop, so the rest of every cell
    // still reaches the folder shown when icons fill it.
    const int index = indexAt(content);
    if (index < 0)
        return -1;
    const int row = rowOf(index);
    if (model().isBlank(row))
        return -1;
    const FileItemPtr item = model().item(row);
    if (!item || !item->isDir)
        return -1;
    return hitsItem(index, content) ? row : -1;
}

std::optional<ui::RectF> IconGridView::labelRect(int index) const
{
    IconItem* item = itemFor(index);
    if (!item)
        return std::nullopt;
    const ui::RectF cell = cellRect(index);
    const ui::RectF label = item->labelBox();
    const ui::PointF origin = fromContent({cell.x + label.x, cell.y + label.y});
    return ui::RectF {origin.x, origin.y, label.width, label.height};
}

std::optional<ui::RectF> IconGridView::dragRect(int index) const
{
    return cellRect(index);
}

void IconGridView::forEachItemView(const std::function<void(int index, ui::View* item)>& visit) const
{
    for (const auto& [index, item] : m_items)
        visit(index, item);
}

void IconGridView::paintDragItem(int, ui::Painter&, const ui::RectF&)
{
}

IconItem::IconItem(IconGridView* view, double selectionHPadding, double selectionVPadding)
    : m_view(view)
    , m_selectionHPadding(selectionHPadding)
    , m_selectionVPadding(selectionVPadding)
{
}

void IconItem::addIconViews()
{
    m_icon = addChild<ui::ThemeIcon>();
    m_thumbnail = addChild<ui::ImageView>();
    m_thumbnail->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
    m_thumbnail->setVisible(false);
    m_emblem = addChild<ui::ThemeIcon>("emblem-symbolic-link", 16);
    m_emblem->setImageProviderLookup(true);
    m_emblem->setVisible(false);
    // Over the emblem as well: the overlay's source is the whole icon item.
    m_overlay = addChild<ui::ThemeIcon>();
    m_overlay->setVisible(false);
}

void IconItem::addLabelView()
{
    m_label = addChild<ui::Label>();
    m_label->setWrap(true);
    m_label->setTextAlignment(ui::Label::TextAlignment::Center);
    m_label->setElide(ui::Label::Elide::Right);
}

void IconItem::bind(int index, int row)
{
    const bool fresh = index != m_index || row != m_row;
    m_index = index;
    m_row = row;
    FolderModel& model = m_view->model();
    const FileItemPtr item = model.item(row);
    if (!item)
        return;

    const double iconSize = m_view->iconSize();
    m_selected = model.isSelected(row);
    m_hidden = item->isHidden;
    m_dropTarget = m_view->isDropTarget(index);
    m_dropAllowed = m_view->dropTargetAllowedFor(index);

    const std::string icon = iconOrFallback(item->iconName, item->isDir ? "folder" : "unknown", int(iconSize));
    m_icon->setIconName(icon);
    m_icon->setSize(iconSize);
    m_overlay->setIconName(icon);
    m_overlay->setSize(iconSize);
    m_overlay->setColor(ui::Theme::current().highlight);

    const bool thumbnail = model.showsThumbnail(row);
    if (fresh || thumbnail != m_wantsThumbnail || item->modified != m_modified || item->path != m_path) {
        m_wantsThumbnail = thumbnail;
        m_modified = item->modified;
        m_path = item->path;
        m_image = {};
        m_thumbnailBox = {};
    }

    m_emblem->setVisible(item->isLink);
    m_label->setOpacity(m_hidden ? 0.8 : 1.0);
    m_label->setVisible(m_view->renamingIndex() != index);
    bindItem(*item, row);
    requestLayout();
    requestPaint();
}

ui::RectF IconItem::iconRect() const
{
    if (m_thumbnail->isVisible())
        return m_thumbnail->geometry();
    return m_icon->geometry();
}

ui::RectF IconItem::labelRect() const
{
    const ui::RectF box = m_label->geometry();
    const double width = paintedWidth();
    return {box.x + (box.width - width) / 2, box.y, width, box.height};
}

ui::RectF IconItem::labelBox() const
{
    return m_label->geometry();
}

bool IconItem::containsPoint(ui::PointF point) const
{
    const auto inside = [&point](const ui::RectF& rect) {
        return point.x >= rect.x && point.y >= rect.y && point.x <= rect.x + rect.width
                && point.y <= rect.y + rect.height;
    };
    return inside(iconRect()) || inside(labelRect());
}

bool IconItem::intersectsRect(const ui::RectF& rect) const
{
    const auto intersects = [&rect](const ui::RectF& other) {
        return other.x < rect.x + rect.width && rect.x < other.x + other.width && other.y < rect.y + rect.height
                && rect.y < other.y + other.height;
    };
    return intersects(iconRect()) || intersects(labelRect());
}

void IconItem::layoutIcon(const ui::RectF& iconItem, double emblemSize)
{
    m_iconItem = iconItem;
    const double iconSize = m_view->iconSize();
    const ui::RectF iconRect {iconItem.x + centeredStart(iconItem.width, iconSize), iconItem.y, iconSize, iconSize};
    m_icon->setGeometry(iconRect);
    m_overlay->setGeometry(iconRect);
    m_icon->setOpacity(m_hidden ? 0.5 : 1.0);
    m_overlay->setOpacity(0.5 * (m_hidden ? 0.5 : 1.0));

    layoutThumbnail();
    const bool thumbnailShown = m_thumbnail->isVisible();
    m_icon->setVisible(!thumbnailShown);
    m_overlay->setVisible(m_selected && !thumbnailShown);

    const ui::RectF anchor = thumbnailShown ? m_thumbnail->geometry() : iconRect;
    m_emblem->setSize(emblemSize);
    m_emblem->setGeometry({anchor.x + anchor.width - emblemSize, anchor.y + anchor.height - emblemSize, emblemSize,
                           emblemSize});
    m_emblem->setOpacity(m_hidden ? 0.5 : 1.0);
}

void IconItem::paint(ui::Painter& painter)
{
    const ui::Theme& theme = ui::Theme::current();
    if (m_dropTarget) {
        const double width = std::max(m_iconItem.width, paintedWidth());
        const double height = m_iconItem.height + m_label->geometry().height + 12;
        const ui::Color color = m_dropAllowed ? theme.highlight : style::kDanger;
        painter.drawRoundedRect({(geometry().width - width) / 2, m_iconItem.y, width, height},
                                {theme.mediumRadius, 0, ui::Corners::Circular}, style::withAlpha(color, 0.16), 2,
                                color);
    }

    // The selection behind the name: a rounded box per line, bridged
    // where lines meet.
    if (m_selected && m_label->isVisible()) {
        const double hPadding = m_selectionHPadding;
        const double vPadding = m_selectionVPadding;
        const ui::RectF box = m_label->geometry();
        const ui::PointF origin {box.x - hPadding, box.y - vPadding};
        ui::Path path;
        std::optional<ui::RectF> previous;
        for (size_t i = 0;; ++i) {
            const auto line = m_label->lineGeometry(i);
            if (!line)
                break;
            const double lineWidth = std::max(0.0, std::min(box.width, line->width));
            const ui::RectF rect {(box.width - lineWidth) / 2, line->top, lineWidth, line->height};
            if (rect.width <= 0 || rect.height <= 0)
                continue;
            addRoundedRect(path, {origin.x + rect.x, origin.y + rect.y, rect.width + hPadding * 2,
                                  rect.height + vPadding * 2},
                           4);
            if (previous) {
                const double left = std::max(previous->x, rect.x);
                const double right = std::min(previous->x + previous->width, rect.x + rect.width) + hPadding * 2;
                const double boundary = rect.y + vPadding;
                if (right > left)
                    path.addRect({origin.x + left, origin.y + boundary - 2, right - left, 4});
            }
            previous = rect;
        }
        painter.fillPath(path, theme.highlight);
    }
}

double IconItem::paintedWidth() const
{
    double width = 0;
    for (size_t i = 0;; ++i) {
        const auto line = m_label->lineGeometry(i);
        if (!line)
            break;
        width = std::max(width, line->width);
    }
    return std::min(width, m_label->geometry().width);
}

// Neither side of a thumbnail is drawn shorter than this share of the box.
static constexpr double kThumbnailMinimumShare = 1.0 / 3;

// The largest thumbnail the cache keeps.
static constexpr double kThumbnailMaximumRequest = 1024;

// The thumbnail fills the icon box less 6 px at the top and sides, keeping
// its aspect, cut to a tenth of its height at the corners. A picture too
// narrow or too flat for the minimum is cropped about its centre.
void IconItem::layoutThumbnail()
{
    if (!m_wantsThumbnail) {
        m_thumbnail->setVisible(false);
        return;
    }

    const ui::RectF box {m_iconItem.x + 6, m_iconItem.y + 6, m_iconItem.width - 12, m_iconItem.height - 6};
    const ui::Window* window = this->window();
    const double scale = window ? window->scale() : 1;
    // The size an Image asks its provider for: its logical size, rounded, at the scale.
    const int width = int(std::round(std::round(box.width) * scale));
    const int height = int(std::round(std::round(box.height) * scale));
    if (width <= 0 || height <= 0) {
        m_thumbnail->setVisible(false);
        return;
    }

    if (m_thumbnailBox != ui::SizeF(width, height)) {
        m_thumbnailBox = ui::SizeF(width, height);
        m_thumbnailRequest = m_thumbnailBox;
        m_image = {};
        loadThumbnail(scale);
    }

    if (m_image.isNull()) {
        m_thumbnail->setVisible(false);
        return;
    }

    // PreserveAspectFit, centred in the box, then widened or heightened to
    // the minimum.
    const double imageWidth = m_image.width() / scale;
    const double imageHeight = m_image.height() / scale;
    const double fit = std::min(box.width / imageWidth, box.height / imageHeight);
    const double fittedWidth = std::min(box.width, imageWidth * fit);
    const double fittedHeight = std::min(box.height, imageHeight * fit);
    const double paintedWidth = std::max(fittedWidth, box.width * kThumbnailMinimumShare);
    const double paintedHeight = std::max(fittedHeight, box.height * kThumbnailMinimumShare);
    m_thumbnail->setGeometry({box.x + (box.width - paintedWidth) / 2, box.y + (box.height - paintedHeight) / 2,
                              paintedWidth, paintedHeight});

    // A cropped picture is drawn larger than it fits the box: a larger
    // thumbnail keeps it sharp, up to the largest the cache has.
    const double crop = std::max(paintedWidth / fittedWidth, paintedHeight / fittedHeight);
    const double limit = kThumbnailMaximumRequest / std::max(m_thumbnailBox.width, m_thumbnailBox.height);
    const double grow = std::max(1.0, std::min(crop, limit));
    const ui::SizeF request(std::round(m_thumbnailBox.width * grow), std::round(m_thumbnailBox.height * grow));
    if (request != m_thumbnailRequest) {
        m_thumbnailRequest = request;
        loadThumbnail(scale);
    }

    m_thumbnail->setCornerRadius(paintedHeight * 0.1);
    m_thumbnail->setOpacity(m_hidden ? 0.5 : 1.0);
    m_thumbnail->setVisible(true);
}

// The picture shown stays until the requested size arrives.
void IconItem::loadThumbnail(double scale)
{
    const int width = int(m_thumbnailRequest.width);
    const int height = int(m_thumbnailRequest.height);
    const ui::Bitmap cached = thumbnails::cached(m_path, m_modified, width, height);
    if (!cached.isNull()) {
        m_image = cached;
        applyImage(scale);
        return;
    }

    const std::weak_ptr<bool> alive = m_alive;
    const std::string path = m_path;
    const int64_t modified = m_modified;
    thumbnails::load(path, modified, width, height,
                     [this, alive, path, modified, width, height, scale](const ui::Bitmap& image) {
                         if (alive.expired() || image.isNull() || path != m_path || modified != m_modified
                             || m_thumbnailRequest != ui::SizeF(width, height))
                             return;
                         m_image = image;
                         applyImage(scale);
                         requestLayout();
                     });
}

void IconItem::applyImage(double scale)
{
    if (m_image.isNull())
        return;
    const ui::Bitmap image = m_image.converted(ui::Bitmap::Format::Argb32Premultiplied);
    m_thumbnail->setPixels(image.width(), image.height(), reinterpret_cast<const uint32_t*>(image.bits()),
                           image.stride(), true, scale);
}

} // namespace fm
