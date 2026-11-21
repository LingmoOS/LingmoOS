// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "model/positioner.h"
#include "views/icongridview.h"

#include <vector>

namespace fm {

// The desktop's folder view: icons on fixed cells of a grid
// anchored to the top right of the work area, in columns filled from the
// top, white names over the wallpaper. The view covers the whole output and
// takes presses outside the grid too.
class DesktopGridView : public IconGridView
{
public:
    explicit DesktopGridView(FolderModel& model);
    ~DesktopGridView() override;

    Positioner& positioner() { return m_positioner; }

    // Where new icons appear on the desktop grid.
    enum class IconCorner { TopRight = 0, TopLeft = 1, BottomLeft = 2, BottomRight = 3 };
    void setIconCorner(IconCorner corner);
    IconCorner iconCorner() const { return m_iconCorner; }

    void setIconSize(double size);
    void setMaximumIconSize(double size) { m_maximumIconSize = size; }
    void setIconsVisible(bool visible);
    bool iconsVisible() const { return m_iconsVisible; }
    // The work area in the view's coordinates; the grid keeps 12 px inside it.
    void setWorkArea(const ui::RectF& area);
    // The grid as laid out, in the view's coordinates.
    const ui::RectF& gridRect() const { return m_grid; }

    double cellWidth() const { return m_cellWidth; }
    double cellHeight() const { return m_cellHeight; }
    int columns() const { return m_columns; }
    int rows() const { return m_rows; }
    ui::RectF cellRect(int cell) const override;

    // Desktop label metrics, shared with the items.
    double labelPixelSize() const override;
    int labelWeight() const override { return 600; }
    double labelHeight() const;
    // A name on at most two lines, the middle of the second elided, so a long
    // name keeps how it starts and its extension.
    std::string labelText(const std::string& name, double width) const;

    void revealIndex(int) override {}

    std::function<void()> onIconSizeChanged;

protected:
    std::unique_ptr<IconItem> makeItem() override;
    int indexAt(ui::PointF content) const override;
    int rowOf(int index) const override;
    int indexOfRow(int row) const override { return m_positioner.cellOfRow(row); }
    void selectRange(int from, int to) override;
    int cellAt(ui::PointF content) const override;
    void rectangleSelect(const ui::RectF& band) override;
    void updateItems() override;
    bool navigate(const ui::KeyEvent& event) override;
    double minimumContentHeight() const override { return geometry().height; }
    bool autoScrolls() const override { return false; }
    void layout() override;
    bool scroll(const ui::ScrollEvent& event) override;

private:
    void updateMetrics();
    // A drop of the files at a point: the icons go to the cell under it,
    // keeping their arrangement when the drag started on one of them.
    void moveToCell(int x, int y, const std::vector<std::string>& urls);
    ui::PointF cellNear(ui::PointF point) const;

    Positioner m_positioner;
    double m_maximumIconSize = 256;
    ui::RectF m_workArea;
    ui::RectF m_grid;
    double m_cellWidth = 108;
    double m_cellHeight = 0;
    int m_columns = 1;
    int m_rows = 1;
    IconCorner m_iconCorner = IconCorner::TopRight;
    bool m_iconsVisible = true;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace fm
