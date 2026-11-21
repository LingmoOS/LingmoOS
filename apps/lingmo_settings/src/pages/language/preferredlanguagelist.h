// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Menu;
class ScrollView;
class ColumnLayout;
}

namespace settings {

class Language;
class PreferredLanguageRow;
class ReorderHandle;

// The preferred languages, most preferred first, reordered by dragging a
// row's handle: a line shows where it lands, and the list scrolls while the
// pointer rests near its ends. Seven rows show; more scroll.
class PreferredLanguageList : public Lingmo::ui::View
{
public:
    static constexpr double kRowHeight = 52;
    static constexpr int kVisibleRows = 7;

    explicit PreferredLanguageList(Language& language);
    ~PreferredLanguageList() override;

    bool isDragging() const { return m_dragging; }
    int pressedIndex() const { return m_pressedIndex; }
    int targetIndex() const { return m_targetIndex; }
    double scrollY() const;
    int count() const;

    Lingmo::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

    // A drag or a key moved a language; unset, the language moves.
    std::function<void(int from, int to)> onMoveRequested;

protected:
    void layout() override;

private:
    friend class PreferredLanguageRow;
    friend class ReorderHandle;
    class Indicator;

    // Pointer positions are in the list's coordinates.
    void press(int index, Lingmo::ui::PointF point);
    void updatePointer(Lingmo::ui::PointF point);
    void release();
    void cancel();
    void updateTarget();
    bool isInside() const;
    void scrollAtEdge();
    void rebuild();
    void updateStates();
    void showActions(int index, Lingmo::ui::PointF windowPosition);
    void move(int from, int to);

    Language& m_language;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    Lingmo::ui::ScrollView* m_scroll;
    Lingmo::ui::ColumnLayout* m_rows;
    Indicator* m_indicator;
    std::unique_ptr<Lingmo::ui::Menu> m_actions;
    Lingmo::base::Timer m_edgeTimer;
    int m_pressedIndex = -1;
    int m_targetIndex = -1;
    bool m_dragging = false;
    bool m_pointerHeld = false;
    Lingmo::ui::PointF m_pointer;
    double m_pressY = 0;
    double m_grabOffset = 0;
};

} // namespace settings
