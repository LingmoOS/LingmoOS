// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/status_items_view.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include "base/event_loop.h"
#include "core/main_loop.h"
#include "wm/status_items.h"

namespace Lingmo::Shell {

// Half the gap between two plates on each side of an item, as the tray's.
static constexpr double kInset = BarContext::kHighlightGap / 2;

StatusItemsView::StatusItemsView(BarContext *bar, wm::StatusItems *items)
    : m_bar(bar)
    , m_items(items)
{
    if (m_items)
        m_changed = m_items->changed.connect([this] { rebuild(); });
    rebuild();
}

StatusItemsView::~StatusItemsView()
{
    m_changed.reset();
    if (!m_items)
        return;
    for (wm::StatusItem *item : m_items->items())
        item->unplace();
}

double StatusItemsView::preferredWidth() const
{
    double width = 0;
    for (StatusItemSlot *slot : m_slots)
        width += slot->item()->width() + 2 * kInset;
    return width;
}

void StatusItemsView::rebuild()
{
    std::vector<StatusItemSlot *> slots;
    if (m_items) {
        for (wm::StatusItem *item : m_items->items()) {
            if (!item->isShown()) {
                item->unplace();
                continue;
            }
            auto it = std::find_if(m_slots.begin(), m_slots.end(),
                                   [item](StatusItemSlot *slot) { return slot->item() == item; });
            StatusItemSlot *slot;
            if (it != m_slots.end()) {
                slot = *it;
                m_slots.erase(it);
            } else {
                slot = addChild<StatusItemSlot>(m_bar, item);
            }
            slot->setChecked(item->isHighlighted());
            slots.push_back(slot);
        }
    }
    // Slots of items that went or hid; their item, if any, is out of the bar.
    for (StatusItemSlot *gone : m_slots) {
        gone->setItem(nullptr);
        gone->setVisible(false);
        std::shared_ptr<ui::View> retired(takeChild(gone));
        core::mainLoop()->post([retired] {});
    }
    m_slots = std::move(slots);
    requestLayout();
    if (onWidthChanged)
        onWidthChanged();
}

void StatusItemsView::barChanged()
{
    for (StatusItemSlot *slot : m_slots)
        slot->barChanged();
}

void StatusItemsView::layout()
{
    placeItems();
}

void StatusItemsView::placeItems()
{
    // Each item starts on a whole logical pixel of the window, wherever the
    // row puts this view.
    const ui::PointF origin = mapToWindow({0, 0});
    double cursor = origin.x;
    const double height = geometry().height;
    for (StatusItemSlot *slot : m_slots) {
        const double itemX = std::round(cursor + kInset);
        const double width = slot->item()->width() + 2 * kInset;
        slot->setGeometry({itemX - kInset - origin.x, 0, width, height});
        slot->item()->place({int(itemX), int(std::lround(origin.y))});
        cursor = itemX - kInset + width;
    }
}

StatusItemSlot::StatusItemSlot(BarContext *bar, wm::StatusItem *item)
    : BarItem(bar)
    , m_item(item)
{
    setName("status-item");
    setInsets(kInset, kInset);
}

ui::RectF StatusItemSlot::itemRect() const
{
    if (!m_item)
        return {};
    return {kInset, 0, double(m_item->width()), geometry().height};
}

} // namespace Lingmo::Shell
