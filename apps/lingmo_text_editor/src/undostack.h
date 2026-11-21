// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include "textbuffer.h"

#include <cstdint>
#include <string>
#include <vector>

// One replacement: `removed` was at `at` and `inserted` took its place.
struct EditStep
{
    TextPosition at;
    std::string removed;
    std::string inserted;
};

struct Selection
{
    TextPosition cursor;
    TextPosition anchor;
};

struct UndoGroup
{
    enum Kind {
        Other,
        Typing,
        Backspace,
        Delete,
    };

    std::vector<EditStep> steps;
    Selection before;
    Selection after;
    Kind kind = Other;
    int64_t time = 0;
};

// Edits grouped the way they are undone: typing and deleting by the word,
// everything else one action at a time. Remembers which entry matches the
// file on disk, so undoing back to it makes the document unmodified again.
class UndoStack
{
public:
    void clear();

    // Starts a group that later records join until end(), e.g. Replace All.
    void begin(const Selection &before);
    void end(const Selection &after);
    bool inGroup() const { return m_depth > 0; }

    void record(const EditStep &step, UndoGroup::Kind kind, const Selection &before,
                const Selection &after, int64_t time);
    // Stops the next record from merging into the last group.
    void seal() { m_sealed = true; }

    bool canUndo() const { return m_index > 0; }
    bool canRedo() const { return m_index < int(m_groups.size()); }
    // The group to revert or reapply; the stack moves past it.
    const UndoGroup &undo();
    const UndoGroup &redo();

    void setClean() { m_cleanIndex = m_index; }
    // The disk copy can no longer be reached by undo or redo.
    void setCleanUnreachable() { m_cleanIndex = -1; }
    bool isClean() const { return m_cleanIndex == m_index; }

    int index() const { return m_index; }
    int count() const { return int(m_groups.size()); }

private:
    bool canMerge(const UndoGroup &group, const EditStep &step, UndoGroup::Kind kind, int64_t time) const;

    std::vector<UndoGroup> m_groups;
    int m_index = 0;
    int m_cleanIndex = 0;
    int m_depth = 0;
    Selection m_groupBefore;
    bool m_groupOpen = false;
    bool m_sealed = false;
};

#endif
