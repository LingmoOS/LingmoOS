// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <functional>
#include <vector>
#include "desktop/widgets/widgetconfig.h"
#include "ui/view.h"
namespace Lingmo::ui { class Button; class Label; class TextField; class ScrollView; class Column; }
namespace fm {
// Searchable desktop widget catalogue. Previews are static examples: opening
// the catalogue does not subscribe to services or read private widget data.
class WidgetGallery : public ui::View {
public:
    WidgetGallery();
    ~WidgetGallery() override;
    std::function<void(WidgetKind)> onAdd; // Compatibility with older hosts.
    std::function<void(WidgetKind, WidgetSize)> onAddSized;
    std::function<void(WidgetKind, WidgetSize, ui::PointF)> onAddAt;
    std::function<void()> onDone;
    std::function<void(ui::RectF)> onSidebarGeometry;
    std::function<void(bool)> onTargetChanged;
    void setNotificationTarget(bool notifications);
    void retranslate();
    void selectKind(WidgetKind kind);
    void focusSearch();
    bool acceptsPointer() const override { return false; }
    bool acceptsFocus() const override { return true; }
    void paint(ui::Painter&) override;
    void layout() override;
protected:
    bool keyPress(const ui::KeyEvent&) override;
private:
    class PanelView;
    class Category;
    class PreviewCard;
    class DragPreview;
    void filter();
    void add(WidgetKind kind, WidgetSize size);
    PanelView* m_panel = nullptr;
    ui::TextField* m_search = nullptr;
    ui::ScrollView* m_categories = nullptr;
    ui::Column* m_list = nullptr;
    ui::Label* m_title = nullptr;
    ui::Label* m_description = nullptr;
    ui::Label* m_hint = nullptr;
    ui::Label* m_empty = nullptr;
    ui::Button* m_done = nullptr;
    ui::Button* m_desktopTarget = nullptr;
    ui::Button* m_notificationTargetButton = nullptr;
    bool m_notificationTarget = false;
    DragPreview* m_drag = nullptr;
    WidgetKind m_selected = WidgetKind::Calendar;
    std::vector<Category*> m_rows;
    std::vector<PreviewCard*> m_previews;
    std::vector<ui::Button*> m_captions;
};
} // namespace fm
