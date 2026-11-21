// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/drop_down_popup.h"

#include <cmath>

#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/embedded_window.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/popup_glass.h"
#include "ui/theme.h"
#include "ui/view.h"
#include "ui/window.h"

namespace Lingmo::ui {

// The bubble behind the content, as a menu's, and Escape.
class DropDownPopup::Surface : public View
{
public:
    explicit Surface(DropDownPopup* popup)
        : m_popup(popup)
    {
    }

    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

protected:
    void layout() override
    {
        for (const auto& child : children())
            child->setGeometry(localRect());
    }

    void paint(Painter& painter) override
    {
        // An embedded host draws the glass, tint included.
        if (Application::instance()->platform() == Application::Platform::Embedded && EmbeddedWindow::host())
            return;
        if (m_glass.update(*window()))
            return;
        // Without the compositor's blur: opaque, in the window radius.
        const Theme& theme = Theme::current();
        painter.fillRoundedRect(localRect(), theme.windowRadius, theme.secondBackground);
    }

    bool keyPress(const KeyEvent& event) override
    {
        if (event.key != key::Escape || event.modifiers != NoModifier)
            return false;
        m_popup->close();
        return true;
    }

private:
    DropDownPopup* m_popup;
    PopupGlass m_glass;
};

DropDownPopup::DropDownPopup() = default;

DropDownPopup::~DropDownPopup()
{
    // The content goes with the window; nothing is told.
    m_window.reset();
}

void DropDownPopup::setContentView(std::unique_ptr<View> view)
{
    m_content = view.get();
    if (m_surface) {
        while (!m_surface->children().empty())
            m_surface->takeChild(m_surface->children().front().get());
        m_surface->addChild(std::move(view));
        updateSize();
        return;
    }
    m_ownedContent = std::move(view);
}

SizeF DropDownPopup::contentSize() const
{
    if (!m_content)
        return {};
    // MenuPopupWindow's size: the content's implicit size, rounded up.
    const double width = std::ceil(m_content->sizeHint().width);
    return {width, std::ceil(m_content->heightForWidth(width))};
}

void DropDownPopup::applySize(SizeF size)
{
    m_window->resize(size);
    m_window->setBackgroundBlur({0, 0, size.width, size.height}, Theme::current().windowRadius);
}

void DropDownPopup::popupFor(View* view, double gap, std::optional<RectF> rect)
{
    close();
    Window* parent = view ? view->window() : nullptr;
    if (!parent)
        return;
    const RectF local = rect.value_or(view->localRect());
    const PointF topLeft = view->mapToWindow({local.x, local.y});
    // MenuPopupWindow::showDropDown's anchor: the item's rect on whole
    // pixels, gap above and below it; the compositor flips it above when it
    // does not fit, and slides it along the screen.
    const double x = std::round(topLeft.x);
    const double y = std::round(topLeft.y);
    PopupPlacement placement;
    placement.anchorRect = {x, y - gap, std::ceil(local.width), std::ceil(local.height) + 2 * gap};
    placement.anchor = PopupAnchor::BottomLeft;
    placement.gravity = PopupAnchor::BottomRight;
    placement.flipX = false;

    m_window = Window::createPopup(parent, placement, true);
    auto surface = std::make_unique<Surface>(this);
    m_surface = surface.get();
    if (m_ownedContent)
        m_surface->addChild(std::move(m_ownedContent));
    // Measured in the popup, at its scale.
    m_window->setContentView(std::move(surface));
    PopupGlass::prepare(*m_window);
    applySize(contentSize());
    m_window->onCloseRequested = [this] { close(); };
    // The content sees keys first; Escape reaches the surface unless it takes it.
    if (m_content && m_content->acceptsFocus())
        m_content->setFocus(FocusReason::Other);
    else
        m_surface->setFocus(FocusReason::Other);
    m_window->show();
}

void DropDownPopup::updateSize()
{
    if (m_window)
        applySize(contentSize());
}

void DropDownPopup::close()
{
    if (!m_window)
        return;
    // The content may be closing it from its own event handling: it stays
    // alive here, and the window goes once that is over, off screen now.
    if (m_content && m_content->parent() == m_surface)
        m_ownedContent = m_surface->takeChild(m_content);
    m_surface = nullptr;
    std::shared_ptr<Window> window(std::move(m_window));
    window->hide();
    Application::instance()->loop().post([window] {});
    if (onClosed)
        onClosed();
}

} // namespace Lingmo::ui
