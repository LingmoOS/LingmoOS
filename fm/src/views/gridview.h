// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "views/icongridview.h"

namespace fm {

// The browser's icon grid. The columns follow the width
// (at least icon + 48 wide each, sharing the width evenly in 1/64 px), the
// rows the icon size and two lines of name; items are made for the rows in
// sight only.
class FolderGridView : public IconGridView
{
public:
    explicit FolderGridView(FolderModel& model);
    ~FolderGridView() override;

    void setIconSize(double size);
    void increaseIconSize();
    void decreaseIconSize();

    int columns() const { return m_columns; }
    double cellWidth() const { return m_cellWidth; }
    double cellHeight() const { return m_cellHeight; }
    double minimumCellWidth() const { return m_iconSize + 48; }
    // The left and right margins inside the view, and the top one under the bars.
    double leftMargin() const { return 6; }
    double rightMargin() const { return 12; }
    double topMargin() const { return topInset() + 6; }
    double bottomMargin() const { return 6 + bottomInset(); }

    // The cell sits on whole device pixels.
    ui::RectF cellRect(int index) const override;
    void revealIndex(int index) override;

protected:
    std::unique_ptr<IconItem> makeItem() override;
    int indexAt(ui::PointF content) const override;
    void rectangleSelect(const ui::RectF& band) override;
    void updateItems() override;
    bool navigate(const ui::KeyEvent& event) override;
    double minimumContentHeight() const override;
    void layout() override;
    bool scroll(const ui::ScrollEvent& event) override;

private:
    void updateMetrics();
    // A new column count or icon size moves every row; the one at the top
    // of the view stays there instead of whatever lands under it.
    void reflow(int fromColumns, double fromRowHeight);

    int m_columns = 1;
    double m_cellWidth = 112;
    double m_cellHeight = 0;
    base::Subscription m_preferences;
};

} // namespace fm
