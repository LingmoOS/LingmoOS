// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "desktop/widgets/widgetconfig.h"
#include "ui/animation.h"
#include "ui/view.h"
#include "ui/theme.h"

namespace Lingmo::ui {
class Painter;
class DialogWindow;
}

namespace fm {

class WidgetLayer;
// Palette shared by desktop and notification hosts, including nested controls.
bool widgetMonochrome(const ui::View* view);
ui::Theme widgetTheme(const ui::View* view);

// A card on the desktop: the rounded, blurred body, the hover and editing
// chrome, moving it by hand, and the widget's own content. The layer owns the
// card and places it; the card only reports what the pointer does.
class DesktopWidget : public ui::View
{
public:
    DesktopWidget(WidgetLayer& layer, WidgetKind kind, WidgetSize size);
    ~DesktopWidget() override;
    bool configurable() const;
    void configure();

    WidgetKind kind() const { return m_kind; }
    WidgetSize size() const { return m_size; }
    // Stable identity in the saved layout, for content that belongs to one card.
    const std::string& instanceId() const { return m_id; }
    void setInstanceId(std::string id);
    void setSize(WidgetSize size);
    WidgetPlacement placement() const;

    bool editing() const { return m_editing; }
    void setEditing(bool editing);

    // The card's outline in window coordinates, for the window's backdrop blur.
    ui::RectF blurRect() const;

    // Text that came from a translation; called after the language changes.
    void retranslate();

    bool acceptsPointer() const override { return true; }
    ui::CursorShape cursorShape() const override;
    void paint(ui::Painter& painter) override;
    void layout() override;

    // The widget's content is clickable: a press without a move calls this.
    std::function<void()> onClicked;

protected:
    // Places and draws what the widget itself shows, inside the card.
    virtual void layoutContent() = 0;
    virtual void paintContent(ui::Painter&) {}
    virtual void retranslateContent() {}

    // The card minus its padding, in the card's coordinates.
    ui::RectF contentRect() const;
    // Called when the card's saved identity arrives.
    virtual void instanceIdChanged() {}
    // False when the compositor has no backdrop blur: the card is then opaque.
    bool frosted() const;

    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const ui::PointerEvent& event) override;
    void pointerMove(const ui::PointerEvent& event) override;
    void pointerRelease(const ui::PointerEvent& event) override;
    void hoverChanged() override;

private:
    std::unique_ptr<ui::DialogWindow> m_configDialog;
    WidgetLayer& m_layer;
    WidgetKind m_kind;
    WidgetSize m_size;
    std::string m_id;
    bool m_editing = false;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_dragging = false;
    bool m_resizing = false;
    ui::PointF m_pressWindow;
    ui::PointF m_dragOrigin;
    std::unique_ptr<ui::Animation> m_sizeAnimation;
    std::unique_ptr<ui::Animation> m_wiggleAnimation;
    bool m_readyToDelete = false;
};

// The widget of a kind, as the desktop's settings name it.
std::unique_ptr<DesktopWidget> createDesktopWidget(WidgetLayer& layer, WidgetKind kind, WidgetSize size);

} // namespace fm
