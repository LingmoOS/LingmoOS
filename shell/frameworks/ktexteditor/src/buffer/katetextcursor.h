/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    Based on code of the SmartCursor/Range by:
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_TEXTCURSOR_H
#define KATE_TEXTCURSOR_H

#include <ktexteditor/movingcursor.h>

#include "katetextblock.h"

namespace Kate
{
class TextBuffer;
class TextBlock;
class TextRange;

/**
 * Class representing a 'clever' text cursor.
 * It will automagically move if the text inside the buffer it belongs to is modified.
 * By intention no subclass of KTextEditor::Cursor, must be converted manually.
 */
class TextCursor final : public KTextEditor::MovingCursor
{
    // range wants direct access to some internals
    friend class TextRange;

    // this is a friend, because this is needed to efficiently transfer cursors from on to an other block
    friend class TextBlock;

    Q_DISABLE_COPY(TextCursor)

private:
    /**
     * Construct a text cursor with given range as parent, private, used by TextRange constructor only.
     * @param buffer text buffer this cursor belongs to
     * @param range text range this cursor is part of
     * @param position wanted cursor position, if not valid for given buffer, will lead to invalid cursor
     * @param insertBehavior behavior of this cursor on insert of text at its position
     */
    TextCursor(TextBuffer &buffer, TextRange *range, const KTextEditor::Cursor position, InsertBehavior insertBehavior);

public:
    /**
     * Construct a text cursor.
     * @param buffer text buffer this cursor belongs to
     * @param position wanted cursor position, if not valid for given buffer, will lead to invalid cursor
     * @param insertBehavior behavior of this cursor on insert of text at its position
     */
    TextCursor(TextBuffer &buffer, const KTextEditor::Cursor position, InsertBehavior insertBehavior);

    /**
     * Destruct the text cursor
     */
    ~TextCursor() override;

    /**
     * Set insert behavior.
     * @param insertBehavior new insert behavior
     */
    void setInsertBehavior(InsertBehavior insertBehavior) override
    {
        m_moveOnInsert = insertBehavior == MoveOnInsert;
    }

    /**
     * Get current insert behavior.
     * @return current insert behavior
     */
    InsertBehavior insertBehavior() const override
    {
        return m_moveOnInsert ? MoveOnInsert : StayOnInsert;
    }

    /**
     * Gets the document to which this cursor is bound.
     * \return a pointer to the document
     */
    KTextEditor::Document *document() const override;

    /**
     * Fast way to set the current cursor position to \e position.
     *
     * \param position new cursor position
     */
    void setPosition(const TextCursor &position);

    /**
     * Set the current cursor position to \e position.
     *
     * \param position new cursor position
     */
    void setPosition(KTextEditor::Cursor position) override;

    /**
     * \overload
     *
     * Set the cursor position to \e line and \e column.
     *
     * \param line new cursor line
     * \param column new cursor column
     */
    void setPosition(int line, int column);

    /**
     * Retrieve the line on which this cursor is situated.
     * \return line number, where 0 is the first line.
     */
    int line() const override
    {
        return lineInternal();
    }

    /**
     * Non-virtual version of line(), which is faster.
     * Inlined for fast access (especially in KateTextBuffer::rangesForLine
     * \return line number, where 0 is the first line or -1 if invalid.
     */
    int lineInternal() const
    {
        if (m_block) {
            return m_block->startLine() + m_line;
        }
        return -1;
    }

    /**
     * Retrieve the column on which this cursor is situated.
     * \return column number, where 0 is the first column.
     */
    int column() const override
    {
        return m_column;
    }

    /**
     * Non-virtual version of column(), which is faster.
     * \return column number, where 0 is the first column.
     * */
    int columnInternal() const
    {
        return m_column;
    }

    /**
     * Get range this cursor belongs to, if any
     * @return range this pointer is part of, else 0
     */
    KTextEditor::MovingRange *range() const override;

    /**
     * Get range this cursor belongs to, if any
     * @return range this pointer is part of, else 0
     */
    Kate::TextRange *kateRange() const
    {
        return m_range;
    }

    /**
     * Get offset into block this cursor belongs to, if any
     * @return offset into block this pointer is part of, else -1
     */
    int lineInBlock() const
    {
        if (m_block) {
            return m_line;
        }
        return -1;
    }

private:
    /**
     * Set the current cursor position to \e position.
     * Internal helper to allow the same code be used for constructor and
     * setPosition.
     *
     * @param position new cursor position
     * @param init is this the initial setup of the position in the constructor?
     */
    void setPosition(KTextEditor::Cursor position, bool init);

private:
    /**
     * parent text buffer
     * is a reference, and no pointer, as this must always exist and can't change
     */
    TextBuffer &m_buffer;

    /**
     * range this cursor belongs to
     * may be null, then no range owns this cursor
     * can not change after initial assignment
     */
    TextRange *const m_range = nullptr;

    /**
     * parent text block, valid cursors always belong to a block, else they are invalid.
     */
    TextBlock *m_block = nullptr;

    /**
     * line, offset in block, or -1
     */
    int m_line = -1;

    /**
     * column
     */
    int m_column = -1;

    /**
     * should this cursor move on insert
     */
    bool m_moveOnInsert;
};

}

#endif
