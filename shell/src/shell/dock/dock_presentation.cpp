// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/dock/dock_presentation.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace Lingmo::Shell {

namespace {

bool fuzzyEqual(double a, double b)
{
    // qFuzzyCompare(1 + a, 1 + b)
    return std::abs((1 + a) - (1 + b)) * 1e12 <= std::min(std::abs(1 + a), std::abs(1 + b));
}

double outQuint(double t)
{
    t -= 1;
    return t * t * t * t * t + 1;
}

double outQuad(double t)
{
    return t * (2 - t);
}

} // namespace

void DockPresentation::Tween::snap(double target)
{
    value = from = to = target;
    active = false;
    pending = false;
    revealing = false;
    opening = false;
}

bool DockPresentation::Tween::retarget(double target)
{
    if (fuzzyEqual(to, target))
        return false;
    from = value;
    to = target;
    active = true;
    revealing = false;
    return true;
}

DockPresentation::DockPresentation(ui::Window *window)
    : FrameClient(window)
{
    m_resize.snap(1);
}

DockPresentation::~DockPresentation() = default;

void DockPresentation::setKeys(const std::vector<std::string> &keys)
{
    std::map<std::string, int> rows;
    for (int row = 0; row < int(keys.size()); ++row)
        rows[keys[size_t(row)]] = row;
    // Rows gone from the source leave.
    for (Entry &entry : m_entries) {
        const auto it = rows.find(entry.key);
        entry.row = it != rows.end() && !entry.leaving ? it->second : -1;
    }
    m_rowCount = int(keys.size());
    // A change of rows outside a drag takes the gap a drop left open.
    if (!m_dragging)
        m_placeholder = -1;
    std::vector<bool> known(keys.size(), false);
    for (const Entry &entry : m_entries) {
        if (entry.row >= 0)
            known[size_t(entry.row)] = true;
    }
    // New rows, each run spreading from where it starts.
    for (int row = 0; row < m_rowCount; ++row) {
        if (known[size_t(row)])
            continue;
        int first = row;
        while (first > 0 && !known[size_t(first - 1)])
            --first;
        Entry entry;
        entry.key = keys[size_t(row)];
        entry.row = row;
        entry.dropped = m_dragging && m_placeholder >= 0 && removedRow() < 0 && row == m_placeholder;
        entry.cells.snap(slotIndex(first));
        entry.gaps.snap(slotGaps(row));
        entry.appearance.snap(m_animationsEnabled ? 0 : 1);
        m_opening = m_opening || (m_opensForNewIcons && m_animationsEnabled && !entry.dropped);
        m_entries.push_back(entry);
    }
    retarget();
}

void DockPresentation::setSection(int row, double extent, bool animate)
{
    const bool rowChanged = m_sectionRow != row;
    const bool extentChanged = !fuzzyEqual(m_sectionExtent, extent);
    if (!rowChanged && !extentChanged)
        return;
    m_sectionRow = row;
    if (extentChanged) {
        beginResize(animate);
        m_sectionExtent = extent;
    }
    if (rowChanged)
        retarget();
    changed();
}

void DockPresentation::setPitch(double pitch, bool animate)
{
    if (fuzzyEqual(m_pitch, pitch))
        return;
    beginResize(animate);
    m_pitch = pitch;
    changed();
}

int DockPresentation::draggedRow() const
{
    if (!m_dragging || m_dragKey.empty())
        return -1;
    for (const Entry &entry : m_entries) {
        if (entry.key == m_dragKey && entry.row >= 0)
            return entry.row;
    }
    return -1;
}

void DockPresentation::beginDrag(int row, bool removeSource)
{
    if (row < -1 || row >= m_rowCount || (m_dragging && row == draggedRow() && removeSource == m_removeDragSource))
        return;
    m_dragging = true;
    m_removeDragSource = removeSource;
    m_dragKey.clear();
    for (const Entry &entry : m_entries) {
        if (entry.row == row && row >= 0)
            m_dragKey = entry.key;
    }
    m_placeholder = removeSource ? row : -1;
    m_placeholderPinned = m_sectionRow < 0 || row < m_sectionRow;
    retarget();
}

void DockPresentation::endDrag()
{
    if (!m_dragging && m_placeholder < 0)
        return;
    const std::string source = m_dragKey;
    const bool removed = m_removeDragSource;
    // The gap the drop held open, which the dropped icon may now fill.
    const double gapSlot = m_placeholder;
    const double gapGaps = !m_placeholderPinned && m_sectionRow >= 0 ? 1 : 0;
    m_dragging = false;
    m_dragKey.clear();
    m_removeDragSource = false;
    m_placeholder = -1;
    for (Entry &entry : m_entries) {
        if (entry.row < 0)
            continue;
        if (entry.dropped || (removed && entry.key == source)) {
            // Whole in a gap already open, taking over from the drag image; into a gap
            // still opening, or its own slot again, it is revealed as a new icon is.
            snapToSlot(entry);
            const bool landsInGap = gapSlot >= 0 && slotIndex(entry.row) == gapSlot
                    && slotGaps(entry.row) == gapGaps;
            entry.appearance.snap(landsInGap && settledForDrop(entry.key) ? 1 : 0);
            entry.dropped = false;
        } else if (entry.appearance.value == 0) {
            snapToSlot(entry);
        }
    }
    retarget();
}

void DockPresentation::setDropSlot(int row, bool pinned)
{
    row = std::clamp(row, -1, m_rowCount - (removedRow() >= 0 ? 1 : 0));
    if (row == m_placeholder && pinned == m_placeholderPinned)
        return;
    m_placeholder = row;
    m_placeholderPinned = pinned;
    retarget();
}

int DockPresentation::sourceRowForSlot(int slot) const
{
    const int removed = removedRow();
    return slot + (removed >= 0 && slot >= removed ? 1 : 0);
}

double DockPresentation::slotIndex(int row) const
{
    const int removed = removedRow();
    if (removed >= 0 && row > removed)
        --row;
    if (m_placeholder >= 0 && row >= m_placeholder)
        ++row;
    return row;
}

void DockPresentation::snapToSlot(Entry &entry)
{
    entry.cells.snap(slotIndex(entry.row));
    entry.gaps.snap(slotGaps(entry.row));
}

bool DockPresentation::settledForDrop(const std::string &landing) const
{
    // Everything drawn is within half a gap of where the drop leaves it, so the
    // dropped icon fits its slot whole. After endDrag's reset: the drop's targets.
    const double clear = (pitch() - m_itemSize) / 2;
    const auto near = [clear](double value, double target) { return std::abs(value - target) <= clear; };
    if (m_sectionRow >= 0 && !near(m_sectionCells.value * pitch(), m_sectionRow * pitch()))
        return false;
    if (!near(length(), m_rowCount * pitch() + (m_sectionRow >= 0 ? sectionExtent() : 0)))
        return false;
    return std::all_of(m_entries.begin(), m_entries.end(), [&](const Entry &entry) {
        // What is not drawn cannot be in the way: leaving, hidden, or the landing icon.
        if (entry.row < 0 || entry.leaving || entry.appearance.value == 0 || entry.dropped
            || entry.key == landing)
            return true;
        return near(position(entry), slotIndex(entry.row) * pitch() + slotGaps(entry.row) * sectionExtent());
    });
}

double DockPresentation::position(const Entry &entry) const
{
    return entry.cells.value * pitch() + entry.gaps.value * sectionExtent();
}

double DockPresentation::targetPosition(const Entry &entry) const
{
    return entry.cells.to * m_pitch + entry.gaps.to * m_sectionExtent;
}

double DockPresentation::appearance(const Entry &entry) const
{
    if (m_removeDragSource && entry.row >= 0 && entry.key == m_dragKey)
        return 0;
    return entry.appearance.value;
}

double DockPresentation::iconScale(const Entry &entry) const
{
    return entry.appearance.revealing ? entry.appearance.value : 1;
}

double DockPresentation::length() const
{
    return m_lengthCells.value * pitch() + m_lengthGaps.value * sectionExtent();
}

double DockPresentation::targetLength() const
{
    return m_lengthCells.to * m_pitch + m_lengthGaps.to * m_sectionExtent;
}

void DockPresentation::apply(Tween &tween, double target, bool animate)
{
    if (!animate)
        tween.snap(target);
    else if (tween.retarget(target))
        begin(tween);
}

void DockPresentation::retarget()
{
    const bool animate = m_animationsEnabled;
    const int removed = removedRow() >= 0 ? 1 : 0;
    const int removedPinned = removedRow() >= 0 && removedRow() < m_sectionRow ? 1 : 0;
    const int placeholder = m_placeholder >= 0 ? 1 : 0;
    apply(m_sectionCells, m_sectionRow - removedPinned + (placeholder && m_placeholderPinned ? 1 : 0), animate);
    apply(m_lengthCells, m_rowCount - removed + placeholder, animate);
    apply(m_lengthGaps, m_sectionRow >= 0 ? 1 : 0, animate);
    apply(m_trailing, m_sectionRow >= 0 && m_rowCount > m_sectionRow ? 1 : 0, animate);

    for (Entry &entry : m_entries) {
        if (entry.row < 0) {
            if (!entry.leaving) {
                entry.cells.snap(entry.cells.value);
                entry.gaps.snap(entry.gaps.value);
            }
            entry.leaving = true;
            entry.dropped = false;
            apply(entry.appearance, 0, animate);
            continue;
        }
        apply(entry.cells, slotIndex(entry.row), animate);
        apply(entry.gaps, slotGaps(entry.row), animate);
        if (!animate) {
            entry.appearance.snap(1);
        } else if (entry.appearance.retarget(1)) {
            entry.appearance.revealing = entry.appearance.from == 0;
            begin(entry.appearance);
        } else if (entry.appearance.revealing && entry.appearance.value == 0) {
            begin(entry.appearance);
        }
    }
    if (!animate) {
        m_resize.snap(1);
        std::erase_if(m_entries, [](const Entry &entry) { return entry.leaving; });
    }
    changed();
}

void DockPresentation::beginResize(bool animate)
{
    m_fromPitch = pitch();
    m_fromSectionExtent = sectionExtent();
    if (!animate || !m_animationsEnabled) {
        m_resize.snap(1);
        return;
    }
    m_resize.snap(0);
    m_resize.retarget(1);
    begin(m_resize);
}

void DockPresentation::begin(Tween &tween)
{
    tween.pending = true;
    if (!isTicking())
        startTicking();
}

bool DockPresentation::advance(Tween &tween, double nowMs)
{
    if (!tween.active || tween.pending)
        return false;
    const double duration = tween.revealing ? kRevealDurationMs
            : tween.opening                 ? kOpeningDurationMs
                                            : kDurationMs;
    const double delay = tween.revealing ? kRevealDelayMs : 0;
    const double progress = std::clamp((nowMs - tween.startMs - delay) / duration, 0.0, 1.0);
    const double eased = tween.opening && !tween.revealing ? outQuad(progress) : outQuint(progress);
    const double next = progress >= 1 ? tween.to : tween.from + (tween.to - tween.from) * eased;
    tween.active = progress < 1;
    if (next == tween.value)
        return false;
    tween.value = next;
    return true;
}

void DockPresentation::tick(uint64_t frameTimeNs)
{
    const double nowMs = double(frameTimeNs) / 1e6;
    std::vector<Tween *> tweens {&m_resize, &m_lengthCells, &m_lengthGaps, &m_sectionCells,
                                 &m_trailing};
    for (Entry &entry : m_entries) {
        tweens.push_back(&entry.cells);
        tweens.push_back(&entry.gaps);
        tweens.push_back(&entry.appearance);
    }
    // Drawn as one frame in, wherever within the frame the change arrived.
    for (Tween *tween : tweens) {
        if (tween->pending) {
            tween->pending = false;
            tween->startMs = nowMs - kFrameIntervalMs;
            tween->opening = m_opening;
        }
    }
    m_opening = false;
    bool moved = false;
    bool active = false;
    for (Tween *tween : tweens) {
        moved |= advance(*tween, nowMs);
        active |= tween->active;
    }
    const size_t left = std::erase_if(
            m_entries, [](const Entry &entry) { return entry.leaving && !entry.appearance.active; });
    if (!active)
        stopTicking();
    if (moved || left > 0)
        changed();
}

void DockPresentation::changed()
{
    if (onChanged)
        onChanged();
}

} // namespace Lingmo::Shell
