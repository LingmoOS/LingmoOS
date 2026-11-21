// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <vector>

#include "base/signal.h"
#include "shell/statusbar/bar_item.h"
#include "ui/view.h"

namespace Lingmo::WM {
class StatusItem;
class StatusItems;
} // namespace Lingmo::WM

namespace Lingmo::Shell {

class StatusItemSlot;

// The items applications draw themselves (lingmo_status_item_v1), in the
// order the window manager keeps, the newest leftmost. Each has a slot as
// wide as the item plus the gap the bar keeps between plates; the item's
// surface lies over the slot's plate, on whole logical pixels, and the slot
// draws the bar's highlight plate under it.
class StatusItemsView : public ui::View
{
public:
    StatusItemsView(BarContext *bar, wm::StatusItems *items);
    ~StatusItemsView() override;

    // Visible slots, left to right.
    int count() const { return int(m_slots.size()); }
    StatusItemSlot *slot(int index) const { return m_slots[size_t(index)]; }
    double preferredWidth() const;

    // The bar's look changed.
    void barChanged();
    // Lays the slots out again where the row put this view and puts each
    // item over its slot; the row calls it whenever it moved its views.
    void placeItems();

    // The width the bar should give it changed.
    std::function<void()> onWidthChanged;

protected:
    void layout() override;

private:
    void rebuild();

    BarContext *m_bar;
    wm::StatusItems *m_items;
    std::vector<StatusItemSlot *> m_slots;
    base::Subscription m_changed;
};

// One application item's place in the row.
class StatusItemSlot : public BarItem
{
public:
    StatusItemSlot(BarContext *bar, wm::StatusItem *item);

    wm::StatusItem *item() const { return m_item; }
    void setItem(wm::StatusItem *item) { m_item = item; }
    // The item's surface in the slot, in slot coordinates: whole logical
    // pixels in the window.
    ui::RectF itemRect() const;

private:
    wm::StatusItem *m_item;
};

} // namespace Lingmo::Shell

