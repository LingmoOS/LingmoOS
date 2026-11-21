// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/status_item.h"

#include <algorithm>
#include <cmath>

#include "ui/platform_window.h"
#include "ui/view.h"

namespace Lingmo::ui {

// The height the item has until the bar says.
static constexpr double kDefaultHeight = 24;

class StatusItem::Client final : public StatusItemClient
{
public:
    explicit Client(StatusItem* item)
        : m_item(item)
    {
    }

    void statusItemLimits(int maxWidth) override
    {
        if (maxWidth == m_item->m_maximumWidth)
            return;
        m_item->m_maximumWidth = maxWidth;
        if (m_item->onConfigured)
            m_item->onConfigured();
    }

    void statusItemAppearance(bool dark, uint32_t foreground) override
    {
        const Color color = Color::fromArgb(foreground);
        if (dark == m_item->m_darkAppearance && color == m_item->m_foreground)
            return;
        m_item->m_darkAppearance = dark;
        m_item->m_foreground = color;
        m_item->repaint();
        if (m_item->onAppearanceChanged)
            m_item->onAppearanceChanged();
    }

    void statusItemHighlight(bool highlighted) override
    {
        if (highlighted == m_item->m_highlighted)
            return;
        m_item->m_highlighted = highlighted;
        if (m_item->onHighlightChanged)
            m_item->onHighlightChanged();
    }

    void statusItemPlaced(const RectF& rect) override
    {
        if (rect == m_item->m_screenRect)
            return;
        m_item->m_screenRect = rect;
        if (m_item->onScreenRectChanged)
            m_item->onScreenRectChanged();
    }

private:
    StatusItem* m_item;
};

void StatusItem::repaint()
{
    if (m_window)
        m_window->damageAll();
}

bool StatusItem::isSupported()
{
    return PlatformWindow::statusItemsSupported();
}

StatusItem::StatusItem()
    : m_client(std::make_unique<Client>(this))
    , m_window(std::unique_ptr<Window>(new Window(m_client.get())))
{
    m_window->setBackgroundColor(ColorRecipe(Color {0, 0, 0, 0}));
    m_window->resize({kDefaultHeight, kDefaultHeight});
    // The bar takes the item away; there is no close button to press.
    m_window->onCloseRequested = [] {};
}

StatusItem::~StatusItem()
{
    m_window.reset();
}

void StatusItem::setContentView(std::unique_ptr<View> view)
{
    m_window->setContentView(std::move(view));
}

void StatusItem::setWidth(double width)
{
    m_window->resize({std::max(1.0, std::round(width)), m_window->size().height});
}

double StatusItem::width() const
{
    return m_window->size().width;
}

double StatusItem::height() const
{
    return m_window->size().height;
}

void StatusItem::setIdentifier(const std::string& identifier)
{
    m_window->m_platform->setStatusItemIdentifier(identifier);
}

void StatusItem::setHighlighted(bool highlighted)
{
    m_window->m_platform->setStatusItemHighlight(highlighted);
}

void StatusItem::show()
{
    m_window->show();
}

PopupPlacement StatusItem::popupPlacement(double gap) const
{
    PopupPlacement placement;
    placement.anchorRect = {0, 0, m_window->size().width, m_window->size().height};
    placement.anchor = PopupAnchor::Bottom;
    placement.gravity = PopupAnchor::Bottom;
    placement.offset = {0, gap};
    // Under the bar there is only down.
    placement.flipY = false;
    return placement;
}

} // namespace Lingmo::ui
