/*
    SPDX-FileCopyrightText: 2011 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2023 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kate_undo_h
#define kate_undo_h

#include <QList>

#include <QBitArray>
#include <kateview.h>
#include <ktexteditor/range.h>

class KateUndoManager;
namespace KTextEditor
{
class DocumentPrivate;
}

class UndoItem
{
public:
    enum UndoType { editInsertText, editRemoveText, editWrapLine, editUnWrapLine, editInsertLine, editRemoveLine, editMarkLineAutoWrapped, editInvalid };

    enum ModificationFlag {
        UndoLine1Modified = 1,
        UndoLine2Modified = 2,
        UndoLine1Saved = 4,
        UndoLine2Saved = 8,
        RedoLine1Modified = 16,
        RedoLine2Modified = 32,
        RedoLine1Saved = 64,
        RedoLine2Saved = 128
    };
    Q_DECLARE_FLAGS(ModificationFlags, ModificationFlag)

    UndoType type = editInvalid;
    ModificationFlags lineModFlags;
    int line = 0;
    int col = 0;
    QString text;
    bool autowrapped = false;
    bool newLine = false;
    bool removeLine = false;
    int len = 0;
};

/**
 * Class to manage a group of undo items
 */
class KateUndoGroup
{
public:
    /**
     * Constructor
     * @param manager KateUndoManager this undo group will belong to
     */
    explicit KateUndoGroup(const KTextEditor::Cursor cursorPosition,
                           KTextEditor::Range selection,
                           const QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> &);

    KateUndoGroup(const KateUndoGroup &) = delete;
    KateUndoGroup &operator=(const KateUndoGroup &) = delete;

    KateUndoGroup(KateUndoGroup &&o) = default;
    KateUndoGroup &operator=(KateUndoGroup &&o) = default;

public:
    /**
     * Undo the contained undo items
     */
    void undo(KateUndoManager *manager, KTextEditor::ViewPrivate *view);

    /**
     * Redo the contained undo items
     */
    void redo(KateUndoManager *manager, KTextEditor::ViewPrivate *view);

    void editEnd(const KTextEditor::Cursor cursorPosition,
                 KTextEditor::Range selectionRange,
                 const QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> &secondaryCursors);

    /**
     * merge this group with an other
     * @param newGroup group to merge into this one
     * @param complex set if a complex undo
     * @return success
     */
    bool merge(KateUndoGroup *newGroup, bool complex);

    /**
     * set group as as savepoint. the next group will not merge with this one
     */
    void safePoint(bool safePoint = true);

    /**
     * is this undogroup empty?
     */
    bool isEmpty() const
    {
        return m_items.empty();
    }

    /**
     * Change all LineSaved flags to LineModified of the line modification system.
     */
    void flagSavedAsModified();

    void markUndoAsSaved(QBitArray &lines);
    void markRedoAsSaved(QBitArray &lines);

    /**
     * Set the undo cursor to @p cursor.
     */
    inline void setUndoCursor(const KTextEditor::Cursor cursor)
    {
        m_undoCursor = cursor;
    }

    /**
     * Set the redo cursor to @p cursor.
     */
    inline void setRedoCursor(const KTextEditor::Cursor cursor)
    {
        m_redoCursor = cursor;
    }

    inline KTextEditor::Cursor redoCursor() const
    {
        return m_redoCursor;
    }

private:
    /**
     * singleType
     * @return the type if it's only one type, or editInvalid if it contains multiple types.
     */
    UndoItem::UndoType singleType() const;

    /**
     * are we only of this type ?
     * @param type type to query
     * @return we contain only the given type
     */
    bool isOnlyType(UndoItem::UndoType type) const;

public:
    /**
     * add an undo item
     * @param u item to add
     */
    void addItem(UndoItem u);

private:
    /**
     * list of items contained
     */
    std::vector<UndoItem> m_items;

    /**
     * prohibit merging with the next group
     */
    bool m_safePoint = false;
    /*
     * Selection Range of primary cursor
     */
    KTextEditor::Range m_undoSelection;
    /*
     * Selection Range of primary cursor
     */
    KTextEditor::Range m_redoSelection;

    /**
     * the cursor position of the active view before the edit step
     */
    KTextEditor::Cursor m_undoCursor;
    /**
     * the cursor positions of the active view before the edit step
     */
    QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> m_undoSecondaryCursors;

    /**
     * the cursor position of the active view after the edit step
     */
    KTextEditor::Cursor m_redoCursor;
    /**
     * the cursor positions of the active view before the edit step
     */
    QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> m_redoSecondaryCursors;
};

#endif
