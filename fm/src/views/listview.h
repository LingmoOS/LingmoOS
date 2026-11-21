// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "views/folderview.h"

#include <map>
#include <vector>

namespace fm {

class ListHeader;
class ListRow;

// The list's columns and their widths, which narrow
// windows drop before they squeeze the name.
struct ListColumns
{
    double availableWidth = 0;
    bool trash = false;
    double originWidth = 200;
    double dateWidth = 180;
    double sizeWidth = 96;
    double typeWidth = 150;

    static constexpr double minimumNameWidth = 180;
    static constexpr double minimumWidth = 64;
    static constexpr double cellPadding = 10;
    static constexpr int iconSize = 22;

    static double rowHeight();
    static double headerHeight();

    bool showOrigin() const;
    bool showDate() const;
    bool showSize() const;
    bool showType() const;
    double nameWidth() const;
    double originX() const { return nameWidth(); }
    double dateX() const { return originX() + (showOrigin() ? originWidth : 0); }
    double sizeX() const { return dateX() + (showDate() ? dateWidth : 0); }
    double typeX() const { return sizeX() + (showSize() ? sizeWidth : 0); }

    int dateSortMode() const { return trash ? 8 : 2; }
};

// Rows of name, dates, size and kind under a column
// header that sorts and resizes; rows scroll beneath the header.
class FolderListView : public FolderView
{
public:
    explicit FolderListView(FolderModel& model);
    ~FolderListView() override;

    double headerHeight() const override;
    const ListColumns& columns() const { return m_columns; }
    void sortBy(int mode);
    void setColumnWidth(int column, double width, bool save);

    void revealIndex(int index) override;
    void retranslate() override;
    double leftMargin() const { return 6; }
    double rightMargin() const { return 12; }
    // The content's top above the first row: under the bars and the header.
    double topMargin() const { return headerHeight() + 2; }
    double bottomMargin() const { return 6 + bottomInset(); }
    ui::RectF rowRect(int index) const;

    // For the rows.
    bool isDropTarget(int index) const { return dropTargetIndex() == index; }
    bool dropAllowed() const { return dropTargetAllowed(); }
    int renamingIndex() const { return isRenaming() ? currentIndex() : -1; }

protected:
    int indexAt(ui::PointF content) const override;
    bool hitsItem(int index, ui::PointF content) const override;
    int dropRowAt(ui::PointF content) const override;
    void rectangleSelect(const ui::RectF& band) override;
    std::optional<ui::RectF> labelRect(int index) const override;
    void updateItems() override;
    bool navigate(const ui::KeyEvent& event) override;
    double minimumContentHeight() const override;
    std::optional<ui::RectF> dragRect(int index) const override;
    void forEachItemView(const std::function<void(int index, ui::View* item)>& visit) const override;
    void paintDragItem(int index, ui::Painter& painter, const ui::RectF& rect) override;
    void layout() override;

private:
    void moveCurrentIndex(int index, uint32_t modifiers);
    void loadColumns();

    ListColumns m_columns;
    ListHeader* m_header;
    std::map<int, ListRow*> m_rows;
    std::vector<ListRow*> m_spare;
    base::Subscription m_preferences;
};

} // namespace fm
