// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/fileitem.h"
#include "views/folderview.h"

#include "ui/bitmap.h"
#include "ui/view.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class ImageView;
class Label;
class ThemeIcon;
}

namespace fm {

class IconItem;

// What the browser's grid and the desktop share: icon items on cells, made
// for the cells that show and reused for others.
class IconGridView : public FolderView
{
public:
    using FolderView::FolderView;

    double iconSize() const { return m_iconSize; }
    // An item's cell in content coordinates.
    virtual ui::RectF cellRect(int index) const = 0;

    // For the items.
    bool isDropTarget(int index) const { return dropTargetIndex() == index; }
    bool dropTargetAllowedFor(int index) const { return isDropTarget(index) && dropTargetAllowed(); }
    int renamingIndex() const { return isRenaming() ? currentIndex() : -1; }

protected:
    virtual std::unique_ptr<IconItem> makeItem() = 0;

    IconItem* itemFor(int index) const;
    // Sets aside the items whose index fails keep.
    void recycleItems(const std::function<bool(int index)>& keep);
    // Binds the index's item, made or reused when it has none.
    void showItem(int index);

    bool hitsItem(int index, ui::PointF content) const override;
    int dropRowAt(ui::PointF content) const override;
    std::optional<ui::RectF> labelRect(int index) const override;
    bool centeredRename() const override { return true; }
    std::optional<ui::RectF> dragRect(int index) const override;
    void forEachItemView(const std::function<void(int index, ui::View* item)>& visit) const override;
    void paintDragItem(int index, ui::Painter& painter, const ui::RectF& rect) override;

    double m_iconSize = 64;
    std::map<int, IconItem*> m_items;

private:
    std::vector<IconItem*> m_spare;
};

// One item's cell: its icon or thumbnail with the link emblem, tinted when
// selected, and its name under it, on a highlight when selected.
// Subclasses add their own layers between the icon's and the name's.
class IconItem : public ui::View
{
public:
    // The selection behind the name reaches the padding past each line.
    IconItem(IconGridView* view, double selectionHPadding, double selectionVPadding);

    void bind(int index, int row);

    // The icon, or the thumbnail as drawn, in item coordinates.
    ui::RectF iconRect() const;
    // The name as drawn, narrower than its box when it is short.
    ui::RectF labelRect() const;
    ui::RectF labelBox() const;
    bool containsPoint(ui::PointF point) const;
    bool intersectsRect(const ui::RectF& rect) const;

protected:
    // The icon, thumbnail, emblem and the icon's tint, then the name: each
    // subclass calls both in its constructor, its own layers before, between
    // or after them.
    void addIconViews();
    void addLabelView();

    // The rest of bind for the subclass: the name's text and colour.
    virtual void bindItem(const FileItem& item, int row) = 0;

    // Places the icon or thumbnail in the icon box, the emblem at its corner.
    void layoutIcon(const ui::RectF& iconItem, double emblemSize);
    void paint(ui::Painter& painter) override;
    double paintedWidth() const;

    IconGridView* m_view;
    int m_index = -1;
    bool m_selected = false;
    bool m_hidden = false;
    ui::RectF m_iconItem;
    ui::ThemeIcon* m_icon = nullptr;
    ui::ImageView* m_thumbnail = nullptr;
    ui::ThemeIcon* m_overlay = nullptr;
    ui::Label* m_label = nullptr;

private:
    void layoutThumbnail();
    void loadThumbnail(double scale);
    void applyImage(double scale);

    double m_selectionHPadding;
    double m_selectionVPadding;
    int m_row = -1;
    bool m_dropTarget = false;
    bool m_dropAllowed = false;
    bool m_wantsThumbnail = false;
    std::string m_path;
    int64_t m_modified = 0;
    ui::SizeF m_thumbnailBox;
    ui::SizeF m_thumbnailRequest;
    ui::Bitmap m_image;
    ui::ThemeIcon* m_emblem = nullptr;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
