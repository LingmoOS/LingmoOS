/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    Based on code of the SmartCursor/Range by:
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katetextcursor.h"
#include "katedocument.h"
#include "katetextblock.h"
#include "katetextbuffer.h"
#include "katetextrange.h"

namespace Kate
{
TextCursor::TextCursor(TextBuffer &buffer, const KTextEditor::Cursor position, InsertBehavior insertBehavior)
    : m_buffer(buffer)
    , m_moveOnInsert(insertBehavior == MoveOnInsert)
{
    // init position
    setPosition(position, true);
}

TextCursor::TextCursor(TextBuffer &buffer, TextRange *range, KTextEditor::Cursor position, InsertBehavior insertBehavior)
    : m_buffer(buffer)
    , m_range(range)
    , m_moveOnInsert(insertBehavior == MoveOnInsert)
{
    // init position
    setPosition(position, true);
}

TextCursor::~TextCursor()
{
    // remove cursor from block or buffer
    if (m_block) {
        m_block->removeCursor(this);
    }

    // only cursors without range are here!
    else if (!m_range) {
        m_buffer.m_invalidCursors.remove(this);
    }
}

void TextCursor::setPosition(const TextCursor &position)
{
    if (m_block && m_block != position.m_block) {
        m_block->removeCursor(this);
    }

    m_line = position.m_line;
    m_column = position.m_column;

    m_block = position.m_block;
    if (m_block) {
        m_block->insertCursor(this);
    }
}

void TextCursor::setPosition(KTextEditor::Cursor position, bool init)
{
    // any change or init? else do nothing
    if (!init && position.line() == line()) {
        // simple case: 1:1 equal
        if (position.column() == m_column) {
            return;
        }

        // ok, too: both old and new column are valid, we can just adjust the column and be done
        if (position.column() >= 0 && m_column >= 0) {
            m_column = position.column();
            return;
        }

        // else: we need to handle the change in a more complex way, new or old column are not valid!
    }

    // first: validate the line and column, else invalid
    if (!position.isValid() || position.line() >= m_buffer.lines()) {
        if (!m_range) {
            m_buffer.m_invalidCursors.insert(this);
        }
        if (m_block) {
            m_block->removeCursor(this);
        }
        m_block = nullptr;
        m_line = m_column = -1;
        return;
    }

    // find new block if m_block doesn't contain the line or if the block is null
    TextBlock *oldBlock = m_block;
    int startLine = oldBlock ? oldBlock->startLine() : -1;
    if (!oldBlock || position.line() < startLine || position.line() >= startLine + oldBlock->lines()) {
        if (oldBlock) {
            oldBlock->removeCursor(this);
        }
        m_block = m_buffer.m_blocks[m_buffer.blockForLine(position.line())];
        Q_ASSERT(m_block);
        m_block->insertCursor(this);
        startLine = m_block->startLine();
    }

    // if cursor was invalid before, remove it from invalid cursor list
    if (!m_range && !oldBlock && !init) {
        Q_ASSERT(m_buffer.m_invalidCursors.contains(this));
        m_buffer.m_invalidCursors.remove(this);
    }

    // else: valid cursor
    m_line = position.line() - startLine;
    m_column = position.column();
}

KTextEditor::Document *Kate::TextCursor::document() const
{
    return m_buffer.document();
}

KTextEditor::MovingRange *Kate::TextCursor::range() const
{
    return m_range;
}

void Kate::TextCursor::setPosition(KTextEditor::Cursor position)
{
    setPosition(position, false);
}

void Kate::TextCursor::setPosition(int line, int column)
{
    setPosition(KTextEditor::Cursor(line, column), false);
}
}
