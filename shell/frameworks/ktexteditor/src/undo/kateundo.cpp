/*
    SPDX-FileCopyrightText: 2011 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2023 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateundo.h"

#include "katebuffer.h"
#include "katedocument.h"
#include "kateundomanager.h"
#include "kateview.h"

#include <ktexteditor/cursor.h>
#include <ktexteditor/view.h>

KateUndoGroup::KateUndoGroup(const KTextEditor::Cursor cursorPosition,
                             KTextEditor::Range selection,
                             const QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> &secondary)
    : m_undoSelection(selection)
    , m_redoSelection(-1, -1, -1, -1)
    , m_undoCursor(cursorPosition)
    , m_undoSecondaryCursors(secondary)
    , m_redoCursor(-1, -1)
{
}

void KateUndoGroup::undo(KateUndoManager *manager, KTextEditor::ViewPrivate *view)
{
    if (m_items.empty()) {
        return;
    }

    manager->startUndo();

    auto doc = manager->document();
    auto updateDocLine = [doc](const UndoItem &item) {
        Kate::TextLine tl = doc->plainKateTextLine(item.line);
        tl.markAsModified(item.lineModFlags.testFlag(UndoItem::UndoLine1Modified));
        tl.markAsSavedOnDisk(item.lineModFlags.testFlag(UndoItem::UndoLine1Saved));
        doc->buffer().setLineMetaData(item.line, tl);
    };

    for (auto rit = m_items.rbegin(); rit != m_items.rend(); ++rit) {
        auto &item = *rit;
        switch (item.type) {
        case UndoItem::editInsertText:
            doc->editRemoveText(item.line, item.col, item.text.size());
            updateDocLine(item);
            break;
        case UndoItem::editRemoveText:
            doc->editInsertText(item.line, item.col, item.text);
            updateDocLine(item);
            break;
        case UndoItem::editWrapLine:
            doc->editUnWrapLine(item.line, item.newLine, item.len);
            updateDocLine(item);
            break;
        case UndoItem::editUnWrapLine: {
            doc->editWrapLine(item.line, item.col, item.removeLine);
            updateDocLine(item);

            auto next = doc->plainKateTextLine(item.line + 1);
            next.markAsModified(item.lineModFlags.testFlag(UndoItem::UndoLine2Modified));
            next.markAsSavedOnDisk(item.lineModFlags.testFlag(UndoItem::UndoLine2Saved));
            doc->buffer().setLineMetaData(item.line + 1, next);
        } break;
        case UndoItem::editInsertLine:
            doc->editRemoveLine(item.line);
            break;
        case UndoItem::editRemoveLine:
            doc->editInsertLine(item.line, item.text);
            updateDocLine(item);
            break;
        case UndoItem::editMarkLineAutoWrapped:
            doc->editMarkLineAutoWrapped(item.line, item.autowrapped);
            break;
        case UndoItem::editInvalid:
            break;
        }
    }

    if (view != nullptr) {
        if (m_undoSelection.isValid()) {
            view->setSelection(m_undoSelection);
        } else {
            view->removeSelection();
        }
        view->clearSecondaryCursors();
        view->addSecondaryCursorsWithSelection(m_undoSecondaryCursors);

        if (m_undoCursor.isValid()) {
            view->setCursorPosition(m_undoCursor);
        }
    }

    manager->endUndo();
}

void KateUndoGroup::redo(KateUndoManager *manager, KTextEditor::ViewPrivate *view)
{
    if (m_items.empty()) {
        return;
    }

    manager->startUndo();

    auto doc = manager->document();
    auto updateDocLine = [doc](const UndoItem &item) {
        Kate::TextLine tl = doc->plainKateTextLine(item.line);
        tl.markAsModified(item.lineModFlags.testFlag(UndoItem::RedoLine1Modified));
        tl.markAsSavedOnDisk(item.lineModFlags.testFlag(UndoItem::RedoLine1Saved));
        doc->buffer().setLineMetaData(item.line, tl);
    };

    for (auto &item : m_items) {
        switch (item.type) {
        case UndoItem::editInsertText:
            doc->editInsertText(item.line, item.col, item.text);
            updateDocLine(item);
            break;
        case UndoItem::editRemoveText:
            doc->editRemoveText(item.line, item.col, item.text.size());
            updateDocLine(item);
            break;
        case UndoItem::editWrapLine: {
            doc->editWrapLine(item.line, item.col, item.newLine);
            updateDocLine(item);

            Kate::TextLine next = doc->plainKateTextLine(item.line + 1);
            next.markAsModified(item.lineModFlags.testFlag(UndoItem::RedoLine2Modified));
            next.markAsSavedOnDisk(item.lineModFlags.testFlag(UndoItem::RedoLine2Saved));
            doc->buffer().setLineMetaData(item.line + 1, next);
        } break;
        case UndoItem::editUnWrapLine:
            doc->editUnWrapLine(item.line, item.removeLine, item.len);
            updateDocLine(item);
            break;
        case UndoItem::editInsertLine:
            doc->editInsertLine(item.line, item.text);
            updateDocLine(item);
            break;
        case UndoItem::editRemoveLine:
            doc->editRemoveLine(item.line);
            break;
        case UndoItem::editMarkLineAutoWrapped:
            doc->editMarkLineAutoWrapped(item.line, item.autowrapped);
            break;
        case UndoItem::editInvalid:
            break;
        }
    }

    if (view != nullptr) {
        if (m_redoSelection.isValid()) {
            view->setSelection(m_redoSelection);
        } else {
            view->removeSelection();
        }
        view->clearSecondaryCursors();
        view->addSecondaryCursorsWithSelection(m_redoSecondaryCursors);

        if (m_redoCursor.isValid()) {
            view->setCursorPosition(m_redoCursor);
        }
    }

    manager->endUndo();
}

void KateUndoGroup::editEnd(const KTextEditor::Cursor cursorPosition,
                            KTextEditor::Range selectionRange,
                            const QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> &secondaryCursors)
{
    m_redoCursor = cursorPosition;
    m_redoSecondaryCursors = secondaryCursors;
    m_redoSelection = selectionRange;
}

static bool mergeUndoItems(UndoItem &base, const UndoItem &u)
{
    if (base.type == UndoItem::editInsertText && u.type == UndoItem::editWrapLine) {
        // merge insert text full line + wrap line
        if (base.col == 0 && base.line == u.line && base.col + base.text.length() == u.col && u.newLine) {
            base.type = UndoItem::editInsertLine;
            base.lineModFlags.setFlag(UndoItem::RedoLine1Modified);
            return true;
        }
    }

    if (base.type == UndoItem::editRemoveText && base.type == u.type) {
        if (base.line == u.line && base.col == (u.col + u.text.size())) {
            base.text.prepend(u.text);
            base.col = u.col;
            return true;
        }
    }

    if (base.type == UndoItem::editInsertText && base.type == u.type) {
        if (base.line == u.line && (base.col + base.text.size()) == u.col) {
            base.text += u.text;
            return true;
        }
    }

    return false;
}

void KateUndoGroup::addItem(UndoItem u)
{
    // try to merge, do that only for equal types, inside mergeWith we do hard casts
    if (!m_items.empty() && mergeUndoItems(m_items.back(), u)) {
        return;
    }

    // default: just add new item unchanged
    m_items.push_back(std::move(u));
}

bool KateUndoGroup::merge(KateUndoGroup *newGroup, bool complex)
{
    if (m_safePoint) {
        return false;
    }

    if (newGroup->isOnlyType(singleType()) || complex) {
        // Take all of its items first -> last
        for (auto &item : newGroup->m_items) {
            addItem(item);
        }
        newGroup->m_items.clear();

        if (newGroup->m_safePoint) {
            safePoint();
        }

        m_redoCursor = newGroup->m_redoCursor;
        m_redoSecondaryCursors = newGroup->m_redoSecondaryCursors;
        m_redoSelection = newGroup->m_redoSelection;
        //         m_redoSelections = newGroup->m_redoSelections;

        return true;
    }

    return false;
}

void KateUndoGroup::safePoint(bool safePoint)
{
    m_safePoint = safePoint;
}

void KateUndoGroup::flagSavedAsModified()
{
    for (UndoItem &item : m_items) {
        if (item.lineModFlags.testFlag(UndoItem::UndoLine1Saved)) {
            item.lineModFlags.setFlag(UndoItem::UndoLine1Saved, false);
            item.lineModFlags.setFlag(UndoItem::UndoLine1Modified, true);
        }

        if (item.lineModFlags.testFlag(UndoItem::UndoLine2Saved)) {
            item.lineModFlags.setFlag(UndoItem::UndoLine2Saved, false);
            item.lineModFlags.setFlag(UndoItem::UndoLine2Modified, true);
        }

        if (item.lineModFlags.testFlag(UndoItem::RedoLine1Saved)) {
            item.lineModFlags.setFlag(UndoItem::RedoLine1Saved, false);
            item.lineModFlags.setFlag(UndoItem::RedoLine1Modified, true);
        }

        if (item.lineModFlags.testFlag(UndoItem::RedoLine2Saved)) {
            item.lineModFlags.setFlag(UndoItem::RedoLine2Saved, false);
            item.lineModFlags.setFlag(UndoItem::RedoLine2Modified, true);
        }
    }
}

static void updateUndoSavedOnDiskFlag(UndoItem &item, QBitArray &lines)
{
    const int line = item.line;
    if (line >= lines.size()) {
        lines.resize(line + 1);
    }

    const bool wasBitSet = lines.testBit(line);
    if (!wasBitSet) {
        lines.setBit(line);
    }

    auto &lineFlags = item.lineModFlags;

    switch (item.type) {
    case UndoItem::editInsertText:
    case UndoItem::editRemoveText:
    case UndoItem::editRemoveLine:
        if (!wasBitSet) {
            lineFlags.setFlag(UndoItem::UndoLine1Modified, false);
            lineFlags.setFlag(UndoItem::UndoLine1Saved, true);
        }
        break;
    case UndoItem::editWrapLine:
        if (lineFlags.testFlag(UndoItem::UndoLine1Modified) && !wasBitSet) {
            lineFlags.setFlag(UndoItem::UndoLine1Modified, false);
            lineFlags.setFlag(UndoItem::UndoLine1Saved, true);
        }
        break;
    case UndoItem::editUnWrapLine:
        if (line + 1 >= lines.size()) {
            lines.resize(line + 2);
        }
        if (lineFlags.testFlag(UndoItem::UndoLine1Modified) && !wasBitSet) {
            lineFlags.setFlag(UndoItem::UndoLine1Modified, false);
            lineFlags.setFlag(UndoItem::UndoLine1Saved, true);
        }

        if (lineFlags.testFlag(UndoItem::UndoLine2Modified) && !lines.testBit(line + 1)) {
            lines.setBit(line + 1);

            lineFlags.setFlag(UndoItem::UndoLine2Modified, false);
            lineFlags.setFlag(UndoItem::UndoLine2Saved, true);
        }
        break;
    case UndoItem::editInsertLine:
    case UndoItem::editMarkLineAutoWrapped:
    case UndoItem::editInvalid:
        break;
    }
}

void KateUndoGroup::markUndoAsSaved(QBitArray &lines)
{
    for (auto rit = m_items.rbegin(); rit != m_items.rend(); ++rit) {
        updateUndoSavedOnDiskFlag(*rit, lines);
    }
}

static void updateRedoSavedOnDiskFlag(UndoItem &item, QBitArray &lines)
{
    const int line = item.line;
    if (line >= lines.size()) {
        lines.resize(line + 1);
    }

    const bool wasBitSet = lines.testBit(line);
    if (!wasBitSet) {
        lines.setBit(line);
    }
    auto &lineFlags = item.lineModFlags;

    switch (item.type) {
    case UndoItem::editInsertText:
    case UndoItem::editRemoveText:
    case UndoItem::editInsertLine:
        lineFlags.setFlag(UndoItem::RedoLine1Modified, false);
        lineFlags.setFlag(UndoItem::RedoLine1Saved, true);
        break;
    case UndoItem::editUnWrapLine:
        if (lineFlags.testFlag(UndoItem::RedoLine1Modified) && !wasBitSet) {
            lineFlags.setFlag(UndoItem::RedoLine1Modified, false);
            lineFlags.setFlag(UndoItem::RedoLine1Saved, true);
        }
        break;
    case UndoItem::editWrapLine:
        if (line + 1 >= lines.size()) {
            lines.resize(line + 2);
        }

        if (lineFlags.testFlag(UndoItem::RedoLine1Modified) && !wasBitSet) {
            lineFlags.setFlag(UndoItem::RedoLine1Modified, false);
            lineFlags.setFlag(UndoItem::RedoLine1Saved, true);
        }

        if (lineFlags.testFlag(UndoItem::RedoLine2Modified) && !lines.testBit(line + 1)) {
            lineFlags.setFlag(UndoItem::RedoLine2Modified, false);
            lineFlags.setFlag(UndoItem::RedoLine2Saved, true);
        }
        break;
    case UndoItem::editRemoveLine:
    case UndoItem::editMarkLineAutoWrapped:
    case UndoItem::editInvalid:
        break;
    }
}

void KateUndoGroup::markRedoAsSaved(QBitArray &lines)
{
    for (auto rit = m_items.rbegin(); rit != m_items.rend(); ++rit) {
        updateRedoSavedOnDiskFlag(*rit, lines);
    }
}

UndoItem::UndoType KateUndoGroup::singleType() const
{
    UndoItem::UndoType ret = UndoItem::editInvalid;

    for (const auto &item : m_items) {
        if (ret == UndoItem::editInvalid) {
            ret = item.type;
        } else if (ret != item.type) {
            return UndoItem::editInvalid;
        }
    }

    return ret;
}

bool KateUndoGroup::isOnlyType(UndoItem::UndoType type) const
{
    if (type == UndoItem::editInvalid) {
        return false;
    }
    for (const auto &item : m_items) {
        if (item.type != type)
            return false;
    }
    return true;
}
