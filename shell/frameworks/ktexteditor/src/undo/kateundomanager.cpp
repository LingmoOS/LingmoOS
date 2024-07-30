/*
    SPDX-FileCopyrightText: 2009-2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateundomanager.h"

#include <ktexteditor/view.h>

#include "katedocument.h"
#include "katepartdebug.h"
#include "kateview.h"

#include <QBitArray>

KateUndoManager::KateUndoManager(KTextEditor::DocumentPrivate *doc)
    : QObject(doc)
    , m_document(doc)
{
    connect(this, &KateUndoManager::undoEnd, this, &KateUndoManager::undoChanged);
    connect(this, &KateUndoManager::redoEnd, this, &KateUndoManager::undoChanged);

    connect(doc, &KTextEditor::DocumentPrivate::viewCreated, this, &KateUndoManager::viewCreated);

    // Before reload save history
    connect(doc, &KTextEditor::DocumentPrivate::aboutToReload, this, [this] {
        savedUndoItems = std::move(undoItems);
        savedRedoItems = std::move(redoItems);
        docChecksumBeforeReload = m_document->checksum();
    });

    // After reload restore it only if checksum of the doc is same
    connect(doc, &KTextEditor::DocumentPrivate::loaded, this, [this](KTextEditor::Document *doc) {
        if (doc && !doc->checksum().isEmpty() && !docChecksumBeforeReload.isEmpty() && doc->checksum() == docChecksumBeforeReload) {
            undoItems = std::move(savedUndoItems);
            redoItems = std::move(savedRedoItems);
            Q_EMIT undoChanged();
        }
        docChecksumBeforeReload.clear();
        savedUndoItems.clear();
        savedRedoItems.clear();
    });
}

KateUndoManager::~KateUndoManager() = default;

void KateUndoManager::viewCreated(KTextEditor::Document *, KTextEditor::View *newView) const
{
    connect(newView, &KTextEditor::View::cursorPositionChanged, this, &KateUndoManager::undoCancel);
}

void KateUndoManager::editStart()
{
    if (!m_isActive) {
        return;
    }

    // editStart() and editEnd() must be called in alternating fashion
    Q_ASSERT(!m_editCurrentUndo.has_value()); // make sure to enter a clean state

    const KTextEditor::Cursor cursorPosition = activeView() ? activeView()->cursorPosition() : KTextEditor::Cursor::invalid();
    const KTextEditor::Range primarySelectionRange = activeView() ? activeView()->selectionRange() : KTextEditor::Range::invalid();
    QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> secondaryCursors;
    if (activeView()) {
        secondaryCursors = activeView()->plainSecondaryCursors();
    }

    // new current undo item
    m_editCurrentUndo = KateUndoGroup(cursorPosition, primarySelectionRange, secondaryCursors);

    Q_ASSERT(m_editCurrentUndo.has_value()); // a new undo group must be created by this method
}

void KateUndoManager::editEnd()
{
    if (!m_isActive) {
        return;
    }

    // editStart() and editEnd() must be called in alternating fashion
    Q_ASSERT(m_editCurrentUndo.has_value()); // an undo group must have been created by editStart()

    const KTextEditor::Cursor cursorPosition = activeView() ? activeView()->cursorPosition() : KTextEditor::Cursor::invalid();
    const KTextEditor::Range selectionRange = activeView() ? activeView()->selectionRange() : KTextEditor::Range::invalid();

    QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> secondaryCursors;
    if (activeView()) {
        secondaryCursors = activeView()->plainSecondaryCursors();
    }

    m_editCurrentUndo->editEnd(cursorPosition, selectionRange, secondaryCursors);

    bool changedUndo = false;

    if (m_editCurrentUndo->isEmpty()) {
        m_editCurrentUndo.reset();
    } else if (!undoItems.empty() && undoItems.back().merge(&*m_editCurrentUndo, m_undoComplexMerge)) {
        m_editCurrentUndo.reset();
    } else {
        undoItems.push_back(std::move(*m_editCurrentUndo));
        changedUndo = true;
    }

    m_editCurrentUndo.reset();

    if (changedUndo) {
        Q_EMIT undoChanged();
    }

    Q_ASSERT(!m_editCurrentUndo.has_value()); // must be 0 after calling this method
}

void KateUndoManager::inputMethodStart()
{
    setActive(false);
    m_document->editStart();
}

void KateUndoManager::inputMethodEnd()
{
    m_document->editEnd();
    setActive(true);
}

void KateUndoManager::startUndo()
{
    setActive(false);
    m_document->editStart();
}

void KateUndoManager::endUndo()
{
    m_document->editEnd();
    setActive(true);
}

void KateUndoManager::slotTextInserted(int line, int col, const QString &s, const Kate::TextLine &tl)
{
    if (!m_editCurrentUndo.has_value() || s.isEmpty()) { // do we care about notifications?
        return;
    }

    UndoItem item;
    item.type = UndoItem::editInsertText;
    item.line = line;
    item.col = col;
    item.text = s;
    item.lineModFlags.setFlag(UndoItem::RedoLine1Modified);

    if (tl.markedAsModified()) {
        item.lineModFlags.setFlag(UndoItem::UndoLine1Modified);
    } else {
        item.lineModFlags.setFlag(UndoItem::UndoLine1Saved);
    }
    addUndoItem(std::move(item));
}

void KateUndoManager::slotTextRemoved(int line, int col, const QString &s, const Kate::TextLine &tl)
{
    if (!m_editCurrentUndo.has_value() || s.isEmpty()) { // do we care about notifications?
        return;
    }

    UndoItem item;
    item.type = UndoItem::editRemoveText;
    item.line = line;
    item.col = col;
    item.text = s;
    item.lineModFlags.setFlag(UndoItem::RedoLine1Modified);

    if (tl.markedAsModified()) {
        item.lineModFlags.setFlag(UndoItem::UndoLine1Modified);
    } else {
        item.lineModFlags.setFlag(UndoItem::UndoLine1Saved);
    }
    addUndoItem(std::move(item));
}

void KateUndoManager::slotMarkLineAutoWrapped(int line, bool autowrapped)
{
    if (m_editCurrentUndo.has_value()) { // do we care about notifications?
        UndoItem item;
        item.type = UndoItem::editMarkLineAutoWrapped;
        item.line = line;
        item.autowrapped = autowrapped;
        addUndoItem(std::move(item));
    }
}

void KateUndoManager::slotLineWrapped(int line, int col, int length, bool newLine, const Kate::TextLine &tl)
{
    if (!m_editCurrentUndo.has_value()) { // do we care about notifications?
        return;
    }

    UndoItem item;
    item.type = UndoItem::editWrapLine;
    item.line = line;
    item.col = col;
    item.len = length;
    item.newLine = newLine;

    if (length > 0 || tl.markedAsModified()) {
        item.lineModFlags.setFlag(UndoItem::RedoLine1Modified);
    } else if (tl.markedAsSavedOnDisk()) {
        item.lineModFlags.setFlag(UndoItem::RedoLine1Saved);
    }

    if (col > 0 || length == 0 || tl.markedAsModified()) {
        item.lineModFlags.setFlag(UndoItem::RedoLine2Modified);
    } else if (tl.markedAsSavedOnDisk()) {
        item.lineModFlags.setFlag(UndoItem::RedoLine2Saved);
    }

    if (tl.markedAsModified()) {
        item.lineModFlags.setFlag(UndoItem::UndoLine1Modified);
    } else if ((length > 0 && col > 0) || tl.markedAsSavedOnDisk()) {
        item.lineModFlags.setFlag(UndoItem::UndoLine1Saved);
    }

    addUndoItem(std::move(item));
}

void KateUndoManager::slotLineUnWrapped(int line, int col, int length, bool lineRemoved, const Kate::TextLine &tl, const Kate::TextLine &nextLine)
{
    if (!m_editCurrentUndo.has_value()) { // do we care about notifications?
        return;
    }

    UndoItem item;
    item.type = UndoItem::editUnWrapLine;
    item.line = line;
    item.col = col;
    item.len = length;
    item.removeLine = lineRemoved;

    const int len1 = tl.length();
    const int len2 = nextLine.length();

    if (len1 > 0 && len2 > 0) {
        item.lineModFlags.setFlag(UndoItem::RedoLine1Modified);

        if (tl.markedAsModified()) {
            item.lineModFlags.setFlag(UndoItem::UndoLine1Modified);
        } else {
            item.lineModFlags.setFlag(UndoItem::UndoLine1Saved);
        }

        if (nextLine.markedAsModified()) {
            item.lineModFlags.setFlag(UndoItem::UndoLine2Modified);
        } else {
            item.lineModFlags.setFlag(UndoItem::UndoLine2Saved);
        }
    } else if (len1 == 0) {
        if (nextLine.markedAsModified()) {
            item.lineModFlags.setFlag(UndoItem::RedoLine1Modified);
        } else if (nextLine.markedAsSavedOnDisk()) {
            item.lineModFlags.setFlag(UndoItem::RedoLine1Saved);
        }

        if (tl.markedAsModified()) {
            item.lineModFlags.setFlag(UndoItem::UndoLine1Modified);
        } else {
            item.lineModFlags.setFlag(UndoItem::UndoLine1Saved);
        }

        if (nextLine.markedAsModified()) {
            item.lineModFlags.setFlag(UndoItem::UndoLine2Modified);
        } else if (nextLine.markedAsSavedOnDisk()) {
            item.lineModFlags.setFlag(UndoItem::UndoLine2Saved);
        }
    } else { // len2 == 0
        if (nextLine.markedAsModified()) {
            item.lineModFlags.setFlag(UndoItem::RedoLine1Modified);
        } else if (nextLine.markedAsSavedOnDisk()) {
            item.lineModFlags.setFlag(UndoItem::RedoLine1Saved);
        }

        if (tl.markedAsModified()) {
            item.lineModFlags.setFlag(UndoItem::UndoLine1Modified);
        } else if (tl.markedAsSavedOnDisk()) {
            item.lineModFlags.setFlag(UndoItem::UndoLine1Saved);
        }

        if (nextLine.markedAsModified()) {
            item.lineModFlags.setFlag(UndoItem::UndoLine2Modified);
        } else {
            item.lineModFlags.setFlag(UndoItem::UndoLine2Saved);
        }
    }

    addUndoItem(std::move(item));
}

void KateUndoManager::slotLineInserted(int line, const QString &s)
{
    if (m_editCurrentUndo.has_value()) { // do we care about notifications?
        UndoItem item;
        item.type = UndoItem::editInsertLine;
        item.line = line;
        item.text = s;
        item.lineModFlags.setFlag(UndoItem::RedoLine1Modified);
        addUndoItem(std::move(item));
    }
}

void KateUndoManager::slotLineRemoved(int line, const QString &s, const Kate::TextLine &tl)
{
    if (m_editCurrentUndo.has_value()) { // do we care about notifications?
        UndoItem item;
        item.type = UndoItem::editRemoveLine;
        item.line = line;
        item.text = s;
        item.lineModFlags.setFlag(UndoItem::RedoLine1Modified);

        if (tl.markedAsModified()) {
            item.lineModFlags.setFlag(UndoItem::UndoLine1Modified);
        } else {
            item.lineModFlags.setFlag(UndoItem::UndoLine1Saved);
        }
        addUndoItem(std::move(item));
    }
}

void KateUndoManager::undoCancel()
{
    // Don't worry about this when an edit is in progress
    if (m_document->isEditRunning()) {
        return;
    }

    undoSafePoint();
}

void KateUndoManager::undoSafePoint()
{
    if (!m_editCurrentUndo.has_value() && !undoItems.empty()) {
        undoItems.back().safePoint();
    } else if (m_editCurrentUndo.has_value()) {
        m_editCurrentUndo.value().safePoint();
    }
}

void KateUndoManager::addUndoItem(UndoItem undo)
{
    Q_ASSERT(m_editCurrentUndo.has_value()); // make sure there is an undo group for our item

    m_editCurrentUndo->addItem(std::move(undo));

    // Clear redo buffer
    redoItems.clear();
}

void KateUndoManager::setActive(bool enabled)
{
    Q_ASSERT(!m_editCurrentUndo.has_value()); // must not already be in edit mode
    Q_ASSERT(m_isActive != enabled);

    m_isActive = enabled;

    Q_EMIT isActiveChanged(enabled);
}

uint KateUndoManager::undoCount() const
{
    return undoItems.size();
}

uint KateUndoManager::redoCount() const
{
    return redoItems.size();
}

void KateUndoManager::undo()
{
    Q_ASSERT(!m_editCurrentUndo.has_value()); // undo is not supported while we care about notifications (call editEnd() first)

    if (!undoItems.empty()) {
        Q_EMIT undoStart(document());

        undoItems.back().undo(this, activeView());
        redoItems.push_back(std::move(undoItems.back()));
        undoItems.pop_back();
        updateModified();

        Q_EMIT undoEnd(document());
    }
}

void KateUndoManager::redo()
{
    Q_ASSERT(!m_editCurrentUndo.has_value()); // redo is not supported while we care about notifications (call editEnd() first)

    if (!redoItems.empty()) {
        Q_EMIT redoStart(document());

        redoItems.back().redo(this, activeView());
        undoItems.push_back(std::move(redoItems.back()));
        redoItems.pop_back();
        updateModified();

        Q_EMIT redoEnd(document());
    }
}

void KateUndoManager::updateModified()
{
    /*
    How this works:

      After noticing that there where to many scenarios to take into
      consideration when using 'if's to toggle the "Modified" flag
      I came up with this baby, flexible and repetitive calls are
      minimal.

      A numeric unique pattern is generated by toggling a set of bits,
      each bit symbolizes a different state in the Undo Redo structure.

        undoItems.isEmpty() != null          BIT 1
        redoItems.isEmpty() != null          BIT 2
        docWasSavedWhenUndoWasEmpty == true  BIT 3
        docWasSavedWhenRedoWasEmpty == true  BIT 4
        lastUndoGroupWhenSavedIsLastUndo     BIT 5
        lastUndoGroupWhenSavedIsLastRedo     BIT 6
        lastRedoGroupWhenSavedIsLastUndo     BIT 7
        lastRedoGroupWhenSavedIsLastRedo     BIT 8

      If you find a new pattern, please add it to the patterns array
    */

    unsigned char currentPattern = 0;
    const unsigned char patterns[] = {5, 16, 21, 24, 26, 88, 90, 93, 133, 144, 149, 154, 165};
    const unsigned char patternCount = sizeof(patterns);
    KateUndoGroup *undoLast = nullptr;
    KateUndoGroup *redoLast = nullptr;

    if (undoItems.empty()) {
        currentPattern |= 1;
    } else {
        undoLast = &undoItems.back();
    }

    if (redoItems.empty()) {
        currentPattern |= 2;
    } else {
        redoLast = &redoItems.back();
    }

    if (docWasSavedWhenUndoWasEmpty) {
        currentPattern |= 4;
    }
    if (docWasSavedWhenRedoWasEmpty) {
        currentPattern |= 8;
    }
    if (lastUndoGroupWhenSaved == undoLast) {
        currentPattern |= 16;
    }
    if (lastUndoGroupWhenSaved == redoLast) {
        currentPattern |= 32;
    }
    if (lastRedoGroupWhenSaved == undoLast) {
        currentPattern |= 64;
    }
    if (lastRedoGroupWhenSaved == redoLast) {
        currentPattern |= 128;
    }

    // This will print out the pattern information

    qCDebug(LOG_KTE) << "Pattern:" << static_cast<unsigned int>(currentPattern);

    for (uint patternIndex = 0; patternIndex < patternCount; ++patternIndex) {
        if (currentPattern == patterns[patternIndex]) {
            // Note: m_document->setModified() calls KateUndoManager::setModified!
            m_document->setModified(false);
            // (dominik) whenever the doc is not modified, succeeding edits
            // should not be merged
            undoSafePoint();
            qCDebug(LOG_KTE) << "setting modified to false!";
            break;
        }
    }
}

void KateUndoManager::clearUndo()
{
    undoItems.clear();

    lastUndoGroupWhenSaved = nullptr;
    docWasSavedWhenUndoWasEmpty = false;

    Q_EMIT undoChanged();
}

void KateUndoManager::clearRedo()
{
    redoItems.clear();

    lastRedoGroupWhenSaved = nullptr;
    docWasSavedWhenRedoWasEmpty = false;

    Q_EMIT undoChanged();
}

void KateUndoManager::setModified(bool modified)
{
    if (!modified) {
        if (!undoItems.empty()) {
            lastUndoGroupWhenSaved = &undoItems.back();
        }

        if (!redoItems.empty()) {
            lastRedoGroupWhenSaved = &redoItems.back();
        }

        docWasSavedWhenUndoWasEmpty = undoItems.empty();
        docWasSavedWhenRedoWasEmpty = redoItems.empty();
    }
}

void KateUndoManager::updateLineModifications()
{
    // change LineSaved flag of all undo & redo items to LineModified
    for (KateUndoGroup &undoGroup : undoItems) {
        undoGroup.flagSavedAsModified();
    }

    for (KateUndoGroup &undoGroup : redoItems) {
        undoGroup.flagSavedAsModified();
    }

    // iterate all undo/redo items to find out, which item sets the flag LineSaved
    QBitArray lines(document()->lines(), false);
    for (int i = undoItems.size() - 1; i >= 0; --i) {
        undoItems[i].markRedoAsSaved(lines);
    }

    lines.fill(false);
    for (int i = redoItems.size() - 1; i >= 0; --i) {
        redoItems[i].markUndoAsSaved(lines);
    }
}

void KateUndoManager::setUndoRedoCursorsOfLastGroup(const KTextEditor::Cursor undoCursor, const KTextEditor::Cursor redoCursor)
{
    Q_ASSERT(!m_editCurrentUndo.has_value());
    if (!undoItems.empty()) {
        KateUndoGroup &last = undoItems.back();
        last.setUndoCursor(undoCursor);
        last.setRedoCursor(redoCursor);
    }
}

KTextEditor::Cursor KateUndoManager::lastRedoCursor() const
{
    Q_ASSERT(!m_editCurrentUndo.has_value());
    if (!undoItems.empty()) {
        undoItems.back().redoCursor();
    }
    return KTextEditor::Cursor::invalid();
}

void KateUndoManager::updateConfig()
{
    Q_EMIT undoChanged();
}

void KateUndoManager::setAllowComplexMerge(bool allow)
{
    m_undoComplexMerge = allow;
}

KTextEditor::ViewPrivate *KateUndoManager::activeView()
{
    return static_cast<KTextEditor::ViewPrivate *>(m_document->activeView());
}

#include "moc_kateundomanager.cpp"
