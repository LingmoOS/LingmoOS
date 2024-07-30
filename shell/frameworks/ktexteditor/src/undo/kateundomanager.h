/*
    SPDX-FileCopyrightText: 2009-2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEUNDOMANAGER_H
#define KATEUNDOMANAGER_H

#include "kateundo.h"
#include <QObject>

#include <ktexteditor_export.h>

#include <QList>

#include <optional>

namespace KTextEditor
{
class DocumentPrivate;
}
class KateUndo;
class KateUndoGroup;

namespace KTextEditor
{
class Document;
class View;
class ViewPrivate;
class Cursor;
}

/**
 * KateUndoManager implements a document's history. It is in either of the two states:
 * @li the default state, which allows rolling back and forth the history of a document, and
 * @li a state in which a new element is being added to the history.
 *
 * The state of the KateUndomanager can be switched using editStart() and editEnd().
 */
class KateUndoManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a clean undo history.
     *
     * @param doc the document the KateUndoManager will belong to
     */
    explicit KateUndoManager(KTextEditor::DocumentPrivate *doc);

    ~KateUndoManager() override;

    KTextEditor::DocumentPrivate *document()
    {
        return m_document;
    }

    /**
     * Returns how many undo() actions can be performed.
     *
     * @return the number of undo groups which can be undone
     */
    uint undoCount() const;

    /**
     * Returns how many redo() actions can be performed.
     *
     * @return the number of undo groups which can be redone
     */
    uint redoCount() const;

    /**
     * Prevent latest KateUndoGroup from being merged with the next one.
     */
    KTEXTEDITOR_EXPORT void undoSafePoint();

    /**
     * Allows or disallows merging of "complex" undo groups.
     *
     * When an undo group contains different types of undo items, it is considered
     * a "complex" group.
     *
     * @param allow whether complex merging is allowed
     */
    void setAllowComplexMerge(bool allow);

    bool isActive() const
    {
        return m_isActive;
    }

    void setModified(bool modified);
    void updateConfig();
    KTEXTEDITOR_EXPORT void updateLineModifications();

    /**
     * Used by the swap file recovery, this function afterwards manipulates
     * the undo/redo cursors of the last KateUndoGroup.
     * This function should not be used other than by Kate::SwapFile.
     * @param undoCursor the undo cursor
     * @param redoCursor the redo cursor
     */
    void setUndoRedoCursorsOfLastGroup(const KTextEditor::Cursor undoCursor, const KTextEditor::Cursor redoCursor);

    /**
     * Returns the redo cursor of the last undo group.
     * Needed for the swap file recovery.
     */
    KTextEditor::Cursor lastRedoCursor() const;

public Q_SLOTS:
    /**
     * Undo the latest undo group.
     *
     * Make sure isDefaultState() is true when calling this method.
     */
    void undo();

    /**
     * Redo the latest undo group.
     *
     * Make sure isDefaultState() is true when calling this method.
     */
    void redo();

    KTEXTEDITOR_EXPORT void clearUndo();
    KTEXTEDITOR_EXPORT void clearRedo();

    /**
     * Notify KateUndoManager about the beginning of an edit.
     */
    void editStart();

    /**
     * Notify KateUndoManager about the end of an edit.
     */
    void editEnd();

    void startUndo();
    void endUndo();

    void inputMethodStart();
    void inputMethodEnd();

    /**
     * Notify KateUndoManager that text was inserted.
     */
    void slotTextInserted(int line, int col, const QString &s, const Kate::TextLine &tl);

    /**
     * Notify KateUndoManager that text was removed.
     */
    void slotTextRemoved(int line, int col, const QString &s, const Kate::TextLine &tl);

    /**
     * Notify KateUndoManager that a line was marked as autowrapped.
     */
    void slotMarkLineAutoWrapped(int line, bool autowrapped);

    /**
     * Notify KateUndoManager that a line was wrapped.
     */
    void slotLineWrapped(int line, int col, int length, bool newLine, const Kate::TextLine &tl);

    /**
     * Notify KateUndoManager that a line was un-wrapped.
     */
    void slotLineUnWrapped(int line, int col, int length, bool lineRemoved, const Kate::TextLine &tl, const Kate::TextLine &nextLine);

    /**
     * Notify KateUndoManager that a line was inserted.
     */
    void slotLineInserted(int line, const QString &s);

    /**
     * Notify KateUndoManager that a line was removed.
     */
    void slotLineRemoved(int line, const QString &s, const Kate::TextLine &tl);

Q_SIGNALS:
    void undoChanged();
    void undoStart(KTextEditor::Document *);
    void undoEnd(KTextEditor::Document *);
    void redoStart(KTextEditor::Document *);
    void redoEnd(KTextEditor::Document *);
    void isActiveChanged(bool enabled);

private Q_SLOTS:
    /**
     * @short Add an undo item to the current undo group.
     *
     * @param undo undo item to be added, must be non-null
     */
    void addUndoItem(UndoItem undo);

    void setActive(bool active);

    void updateModified();

    void undoCancel();
    void viewCreated(KTextEditor::Document *, KTextEditor::View *newView) const;

private:
    KTEXTEDITOR_NO_EXPORT
    KTextEditor::ViewPrivate *activeView();

private:
    KTextEditor::DocumentPrivate *m_document = nullptr;
    bool m_undoComplexMerge = false;
    bool m_isActive = true;
    std::optional<KateUndoGroup> m_editCurrentUndo;
    std::vector<KateUndoGroup> undoItems;
    std::vector<KateUndoGroup> redoItems;
    // these two variables are for resetting the document to
    // non-modified if all changes have been undone...
    KateUndoGroup *lastUndoGroupWhenSaved = nullptr;
    KateUndoGroup *lastRedoGroupWhenSaved = nullptr;
    bool docWasSavedWhenUndoWasEmpty = true;
    bool docWasSavedWhenRedoWasEmpty = true;

    // saved undo items that are used to restore state on doc reload
    std::vector<KateUndoGroup> savedUndoItems;
    std::vector<KateUndoGroup> savedRedoItems;
    QByteArray docChecksumBeforeReload;
};

#endif
