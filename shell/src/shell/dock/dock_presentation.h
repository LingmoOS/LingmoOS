// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "ui/frame_client.h"

namespace Lingmo::Shell {

// Where each of a row of dock items is drawn while the row changes. One clock
// for every transition, 260 ms on OutQuint, stepped on the window's frames;
// positions are counted in slots and section gaps so a new icon size rescales
// the motion in flight. With opening for new icons, what moves as a new icon
// arrives takes 260 ms on OutQuad instead, and from 75 ms on the icon grows out
// of the middle of its slot and fades in, in 200 ms on OutQuint; a leaving icon
// fades out while its slot closes.
class DockPresentation : public ui::FrameClient
{
public:
    static constexpr double kDurationMs = 260;
    // The frame interval a new transition is backdated by, so its first
    // frame already shows one step of motion.
    static constexpr double kFrameIntervalMs = 1000.0 / 60;
    static constexpr double kOpeningDurationMs = 260;
    static constexpr double kRevealDelayMs = 75;
    static constexpr double kRevealDurationMs = 200;

    struct Tween {
        double value = 0;
        double from = 0;
        double to = 0;
        double startMs = 0;
        bool active = false;
        bool pending = false;
        // An appearing icon, which grows as it fades in: kRevealDurationMs
        // after kRevealDelayMs.
        bool revealing = false;
        // Started as a new icon arrived.
        bool opening = false;

        void snap(double target);
        // Whether it has somewhere new to go.
        bool retarget(double target);
    };

    struct Entry {
        std::string key;
        // In the source; -1 for one leaving.
        int row = -1;
        Tween cells;
        Tween gaps;
        Tween appearance;
        bool leaving = false;
        // Inserted into the slot a drop held open; shown there as the drag ends.
        bool dropped = false;
    };

    explicit DockPresentation(ui::Window *window);
    ~DockPresentation() override;

    // The source's rows, by a key that stays with a row while it moves.
    void setKeys(const std::vector<std::string> &keys);
    // Rows from `row` on follow a divider gap of `extent`; -1 for none.
    void setSection(int row, double extent, bool animate);
    void setPitch(double pitch, bool animate);
    void setItemSize(double size) { m_itemSize = size; }
    // What is there when the dock first appears is shown at once.
    void setAnimationsEnabled(bool enabled) { m_animationsEnabled = enabled; }
    void setOpensForNewIcons(bool enabled) { m_opensForNewIcons = enabled; }
    bool animationsEnabled() const { return m_animationsEnabled; }

    // A drag of row `row` over the dock (-1 for one from outside): with
    // `removeSource` its slot closes while it is away, else the row stays.
    void beginDrag(int row, bool removeSource = true);
    void endDrag();
    // The gap a drop would land in, before row `row` of the pinned or the
    // running section; -1 closes it.
    void setDropSlot(int row, bool pinned);
    // The source row a drop into `slot` lands at.
    int sourceRowForSlot(int slot) const;
    bool isDragging() const { return m_dragging; }
    int draggedRow() const;
    int removedRow() const { return m_removeDragSource ? draggedRow() : -1; }
    int placeholder() const { return m_placeholder; }
    bool placeholderPinned() const { return m_placeholderPinned; }

    const std::vector<Entry> &entries() const { return m_entries; }
    // Along the dock, as drawn and where it comes to rest.
    double position(const Entry &entry) const;
    double targetPosition(const Entry &entry) const;
    double appearance(const Entry &entry) const;
    double iconScale(const Entry &entry) const;

    double length() const;
    double targetLength() const;
    double sectionPosition() const { return m_sectionCells.value * pitch(); }
    double targetSectionPosition() const { return m_sectionCells.to * m_pitch; }
    double trailingExtent() const { return m_trailing.value * sectionExtent(); }
    double targetTrailingExtent() const { return m_trailing.to * m_sectionExtent; }
    double resizeProgress() const { return m_resize.value; }
    bool isRunning() const { return isTicking(); }

    // Something drawn moved.
    std::function<void()> onChanged;

protected:
    void tick(uint64_t frameTimeNs) override;

private:
    double pitch() const { return m_fromPitch + (m_pitch - m_fromPitch) * m_resize.value; }
    double sectionExtent() const
    {
        return m_fromSectionExtent + (m_sectionExtent - m_fromSectionExtent) * m_resize.value;
    }
    double slotGaps(int row) const { return m_sectionRow >= 0 && row >= m_sectionRow ? 1 : 0; }
    double slotIndex(int row) const;
    void snapToSlot(Entry &entry);
    bool settledForDrop(const std::string &landing) const;
    void retarget();
    void apply(Tween &tween, double target, bool animate);
    void begin(Tween &tween);
    void beginResize(bool animate);
    bool advance(Tween &tween, double nowMs);
    void changed();

    std::vector<Entry> m_entries;
    double m_pitch = 70;
    double m_fromPitch = 70;
    double m_itemSize = 54;
    double m_sectionExtent = 0;
    double m_fromSectionExtent = 0;
    int m_sectionRow = -1;
    int m_rowCount = 0;
    Tween m_lengthCells;
    Tween m_lengthGaps;
    Tween m_sectionCells;
    Tween m_trailing;
    Tween m_resize;
    bool m_animationsEnabled = false;
    bool m_opensForNewIcons = false;
    // A new icon arrived since the last frame: what starts by then opens.
    bool m_opening = false;
    // The dragged row by its key, so it stays the same row while rows move.
    std::string m_dragKey;
    bool m_dragging = false;
    bool m_removeDragSource = false;
    int m_placeholder = -1;
    bool m_placeholderPinned = true;
};

} // namespace Lingmo::Shell

