// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "undostack.h"
#include "unicode.h"

// A pause this long ends a typing run even within a word.
static constexpr int64_t mergeInterval = 5000;

static bool startsWithSpace(const std::string &text)
{
    return Unicode::isSpace(Unicode::at(text, 0));
}

static bool endsWithSpace(const std::string &text)
{
    return Unicode::isSpace(Unicode::before(text, text.size()));
}

void UndoStack::clear()
{
    m_groups.clear();
    m_index = 0;
    m_cleanIndex = 0;
    m_depth = 0;
    m_groupOpen = false;
    m_sealed = false;
}

void UndoStack::begin(const Selection &before)
{
    if (m_depth++ > 0)
        return;
    m_groupBefore = before;
    m_groupOpen = false;
}

void UndoStack::end(const Selection &after)
{
    if (m_depth == 0 || --m_depth > 0)
        return;
    if (m_groupOpen)
        m_groups.back().after = after;
    m_groupOpen = false;
    m_sealed = true;
}

bool UndoStack::canMerge(const UndoGroup &group, const EditStep &step, UndoGroup::Kind kind, int64_t time) const
{
    if (m_sealed || group.kind != kind || kind == UndoGroup::Other || group.steps.size() != 1)
        return false;
    if (m_cleanIndex == m_index || time - group.time > mergeInterval)
        return false;

    const EditStep &last = group.steps.back();
    switch (kind) {
    case UndoGroup::Typing:
        if (!step.removed.empty() || step.inserted.find('\n') != std::string::npos || last.inserted.empty())
            return false;
        if (TextBuffer::endOf(last.at, last.inserted) != step.at)
            return false;
        // A new word starts a new group; the spaces before it stay with the last one.
        return !(endsWithSpace(last.inserted) && !startsWithSpace(step.inserted));
    case UndoGroup::Backspace:
        if (!step.inserted.empty() || !last.inserted.empty() || step.removed.find('\n') != std::string::npos)
            return false;
        if (TextBuffer::endOf(step.at, step.removed) != last.at)
            return false;
        return !(endsWithSpace(step.removed) && !startsWithSpace(last.removed));
    case UndoGroup::Delete:
        if (!step.inserted.empty() || !last.inserted.empty() || step.removed.find('\n') != std::string::npos)
            return false;
        if (step.at != last.at)
            return false;
        return !(!endsWithSpace(last.removed) && startsWithSpace(step.removed));
    case UndoGroup::Other:
        break;
    }
    return false;
}

void UndoStack::record(const EditStep &step, UndoGroup::Kind kind, const Selection &before,
                       const Selection &after, int64_t time)
{
    // A group that changes nothing leaves the redo entries alone.
    if (m_depth > 0 && m_groupOpen) {
        m_groups.back().steps.push_back(step);
        return;
    }

    m_groups.resize(size_t(m_index));
    if (m_cleanIndex > m_index)
        m_cleanIndex = -1;

    if (m_depth > 0) {
        UndoGroup group;
        group.steps.push_back(step);
        group.before = m_groupBefore;
        group.after = m_groupBefore;
        m_groups.push_back(std::move(group));
        ++m_index;
        m_groupOpen = true;
        return;
    }

    if (!m_groups.empty() && canMerge(m_groups.back(), step, kind, time)) {
        UndoGroup &group = m_groups.back();
        EditStep &last = group.steps.back();
        if (kind == UndoGroup::Typing) {
            last.inserted += step.inserted;
        } else if (kind == UndoGroup::Backspace) {
            last.at = step.at;
            last.removed.insert(0, step.removed);
        } else {
            last.removed += step.removed;
        }
        group.after = after;
        group.time = time;
        return;
    }

    UndoGroup group;
    group.steps.push_back(step);
    group.before = before;
    group.after = after;
    group.kind = kind;
    group.time = time;
    m_groups.push_back(std::move(group));
    ++m_index;
    m_sealed = false;
}

const UndoGroup &UndoStack::undo()
{
    m_sealed = true;
    return m_groups[size_t(--m_index)];
}

const UndoGroup &UndoStack::redo()
{
    m_sealed = true;
    return m_groups[size_t(m_index++)];
}
