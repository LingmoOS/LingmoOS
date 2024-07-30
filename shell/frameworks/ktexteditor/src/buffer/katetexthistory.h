/*
    SPDX-FileCopyrightText: 2013 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_TEXTHISTORY_H
#define KATE_TEXTHISTORY_H

#include <vector>

#include <ktexteditor/movingcursor.h>
#include <ktexteditor/movingrange.h>
#include <ktexteditor/range.h>

namespace Kate
{
class TextBuffer;

/**
 * Class representing the editing history of a TextBuffer
 */
class TextHistory
{
    friend class TextBuffer;
    friend class TextBlock;

public:
    /**
     * Current revision, just relay the revision of the buffer
     * @return current revision
     */
    qint64 revision() const;

    /**
     * Last revision the buffer got successful saved
     * @return last revision buffer got saved, -1 if none
     */
    qint64 lastSavedRevision() const
    {
        return m_lastSavedRevision;
    }

    /**
     * Lock a revision, this will keep it around until released again.
     * But all revisions will always be cleared on buffer clear() (and therefor load())
     * @param revision revision to lock
     */
    void lockRevision(qint64 revision);

    /**
     * Release a revision.
     * @param revision revision to release
     */
    void unlockRevision(qint64 revision);

    /**
     * Transform a cursor from one revision to an other.
     * @param line line number of the cursor to transform
     * @param column column number of the cursor to transform
     * @param insertBehavior behavior of this cursor on insert of text at its position
     * @param fromRevision from this revision we want to transform
     * @param toRevision to this revision we want to transform, default of -1 is current revision
     */
    void transformCursor(int &line, int &column, KTextEditor::MovingCursor::InsertBehavior insertBehavior, qint64 fromRevision, qint64 toRevision = -1);

    /**
     * Transform a range from one revision to an other.
     * @param range range to transform
     * @param insertBehaviors behavior of this range on insert of text at its position
     * @param emptyBehavior behavior on becoming empty
     * @param fromRevision from this revision we want to transform
     * @param toRevision to this revision we want to transform, default of -1 is current revision
     */
    void transformRange(KTextEditor::Range &range,
                        KTextEditor::MovingRange::InsertBehaviors insertBehaviors,
                        KTextEditor::MovingRange::EmptyBehavior emptyBehavior,
                        qint64 fromRevision,
                        qint64 toRevision = -1);

private:
    /**
     * Class representing one entry in the editing history.
     */
    class Entry
    {
    public:
        /**
         * transform cursor for this history entry
         * @param line line number of the cursor to transform
         * @param column column number of the cursor to transform
         * @param moveOnInsert behavior of this cursor on insert of text at its position
         */
        void transformCursor(int &line, int &column, bool moveOnInsert) const;

        /**
         * reverse transform cursor for this history entry
         * @param line line number of the cursor to transform
         * @param column column number of the cursor to transform
         * @param moveOnInsert behavior of this cursor on insert of text at its position
         */
        void reverseTransformCursor(int &line, int &column, bool moveOnInsert) const;

        /**
         * Types of entries, matching editing primitives of buffer and placeholder
         */
        enum Type { NoChange, WrapLine, UnwrapLine, InsertText, RemoveText };

        /**
         * Default Constructor, invalidates all fields
         */
        Entry()
        {
        }

        /**
         * Reference counter, how often ist this entry referenced from the outside?
         */
        unsigned int referenceCounter = 0;

        /**
         * Type of change
         */
        Type type = NoChange;

        /**
         * line the change occurred
         */
        int line = -1;

        /**
         * column the change occurred
         */
        int column = -1;

        /**
         * length of change (length of insert or removed text)
         */
        int length = -1;

        /**
         * old line length (needed for unwrap and insert)
         */
        int oldLineLength = -1;
    };

    /**
     * Construct an empty text history.
     * @param buffer buffer this text history belongs to
     */

    explicit TextHistory(TextBuffer &buffer);

    /**
     * Destruct the text history
     */

    ~TextHistory();

    /**
     * Clear the edit history, this is done on clear() in buffer.
     */

    void clear();

    /**
     * Set current revision as last saved revision
     */

    void setLastSavedRevision();

    /**
     * Notify about wrap line at given cursor position.
     * @param position line/column as cursor where to wrap
     */

    void wrapLine(const KTextEditor::Cursor position);

    /**
     * Notify about unwrap given line.
     * @param line line to unwrap
     * @param oldLineLength text length of the line in front of this one before this unwrap
     */

    void unwrapLine(int line, int oldLineLength);

    /**
     * Notify about insert text at given cursor position.
     * @param position position where to insert text
     * @param length text length to be inserted
     * @param oldLineLength text length of the line before this insert
     */

    void insertText(const KTextEditor::Cursor position, int length, int oldLineLength);

    /**
     * Notify about remove text at given range.
     * @param range range of text to remove, must be on one line only.
     * @param oldLineLength text length of the line before this remove
     */

    void removeText(KTextEditor::Range range, int oldLineLength);

    /**
     * Generic function to add a entry to the history. Is used by the above functions for the different editing primitives.
     * @param entry new entry to add
     */

    void addEntry(const Entry &entry);

private:
    /**
     * TextBuffer this history belongs to
     */
    TextBuffer &m_buffer;

    /**
     * Last revision the buffer got saved
     */
    qint64 m_lastSavedRevision;

    /**
     * history of edits
     * needs no sharing, small entries
     */
    std::vector<Entry> m_historyEntries;

    /**
     * offset for the first entry in m_history, to which revision it really belongs?
     */
    qint64 m_firstHistoryEntryRevision;
};

}

#endif
