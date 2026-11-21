// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "desktop/widgets/widgetconfig.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Menu;
class Window;
}

namespace fm {

class DesktopWidget;
class WidgetLayer;

// The desktop's widget layer: it holds the cards, keeps them inside the work
// area and on an 8 px grid, edits and sizes them, and asks the desktop window
// to blur their silhouettes. The layer itself takes no pointer, so a press
// between cards still goes to the desktop's icons; only the cards do.
class WidgetLayer : public ui::View
{
public:
    WidgetLayer();
    ~WidgetLayer() override;

    void setDesktopWindow(ui::Window* window);
    ui::Window* desktopWindow() const { return m_window; }

    // The work area in the layer's coordinates; the cards stay inside it.
    void setWorkArea(const ui::RectF& area);
    const ui::RectF& workArea() const { return m_workArea; }

    bool editing() const { return m_editing; }
    void setEditing(bool editing);

    void setAppearance(bool visible, int colorMode, bool desktopActive);
    bool monochrome() const { return m_monochrome && !m_editing; }
    void setFlowLayout(bool flow) { m_flow = flow; }
    double flowHeight() const;
    void addWidget(WidgetKind kind, WidgetSize size = WidgetSize::Medium);
    // Drop position is the requested card centre, in window coordinates.
    void addWidgetAt(WidgetKind kind, WidgetSize size, ui::PointF windowPosition);
    void setGalleryBlurRect(ui::RectF rect);
    void removeWidget(DesktopWidget* widget);
    // Above the other cards, and last in the saved order.
    void raiseWidget(DesktopWidget* widget);
    // As removeWidget, after the event that asked for it is over.
    void removeWidgetDeferred(DesktopWidget* widget);
    // Replaces every card: what the desktop's settings held for this output.
    void setPlacements(const std::vector<WidgetPlacement>& placements);
    std::vector<WidgetPlacement> placements() const;
    const std::vector<DesktopWidget*>& widgets() const { return m_widgets; }

    void retranslate();

    bool acceptsPointer() const override { return false; }
    void layout() override;

    // The window's backdrop blur behind the cards, as one shape of rounded
    // rectangles; empty when there is no blur to ask for.
    void updateBlur();

    // Called by the cards.
    void moveWidget(DesktopWidget* widget, ui::PointF position, bool finished);
    void setWidgetSize(DesktopWidget* widget, WidgetSize size);
    void showWidgetMenu(DesktopWidget* widget, ui::PointF windowPosition);

    // Bumped when a card moves or is added, removed or resized.
    std::function<void()> onChanged;
    // The card's menu asked for the gallery.
    std::function<void()> onEditWidgets;

private:
    DesktopWidget* create(const WidgetPlacement& placement);
    ui::PointF clamped(ui::PointF position, ui::SizeF size) const;
    ui::PointF snapped(ui::PointF position) const;
    ui::PointF freePosition(ui::SizeF size) const;
    bool overlaps(const ui::RectF& rect) const;

    ui::Window* m_window = nullptr;
    ui::RectF m_workArea;
    ui::RectF m_galleryBlur;
    std::vector<DesktopWidget*> m_widgets;
    std::unique_ptr<ui::Menu> m_menu;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    bool m_editing = false;
    bool m_monochrome = false;
    bool m_flow = false;
};

} // namespace fm
