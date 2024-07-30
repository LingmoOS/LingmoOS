/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2012 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentcursor.h"

namespace KTextEditor
{
DocumentCursor::DocumentCursor(KTextEditor::Document *document)
    : m_document(document)
    , m_cursor(KTextEditor::Cursor::invalid())
{
    // we require a valid document
    Q_ASSERT(m_document);
}

DocumentCursor::DocumentCursor(KTextEditor::Document *document, KTextEditor::Cursor position)
    : m_document(document)
    , m_cursor(position)
{
    // we require a valid document
    Q_ASSERT(m_document);
}

DocumentCursor::DocumentCursor(KTextEditor::Document *document, int line, int column)
    : m_document(document)
    , m_cursor(line, column)
{
    // we require a valid document
    Q_ASSERT(m_document);
}

DocumentCursor::DocumentCursor(const DocumentCursor &other)
    : m_document(other.m_document)
    , m_cursor(other.m_cursor)
{
}

void DocumentCursor::makeValid()
{
    const int line = m_cursor.line();
    const int col = m_cursor.line();

    if (line < 0) {
        m_cursor.setPosition(0, 0);
    } else if (line >= m_document->lines()) {
        m_cursor = m_document->documentEnd();
    } else if (col > m_document->lineLength(line)) {
        m_cursor.setColumn(m_document->lineLength(line));
    } else if (col < 0) {
        m_cursor.setColumn(0);
    } else if (!isValidTextPosition()) {
        // inside a unicode surrogate (utf-32 character)
        // -> move half one char left to the start of the utf-32 char
        m_cursor.setColumn(col - 1);
    }

    Q_ASSERT(isValidTextPosition());
}

void DocumentCursor::setPosition(int line, int column)
{
    m_cursor.setPosition(line, column);
}

void DocumentCursor::setLine(int line)
{
    setPosition(line, column());
}

void DocumentCursor::setColumn(int column)
{
    setPosition(line(), column);
}

bool DocumentCursor::atStartOfLine() const
{
    return isValidTextPosition() && column() == 0;
}

bool DocumentCursor::atEndOfLine() const
{
    return isValidTextPosition() && column() == document()->lineLength(line());
}

bool DocumentCursor::atStartOfDocument() const
{
    return line() == 0 && column() == 0;
}

bool DocumentCursor::atEndOfDocument() const
{
    // avoid costly lineLength computation if we are not in the last line
    // this is called often e.g. during search & replace, >> 2% of the total costs
    const auto lastLine = document()->lines() - 1;
    return line() == lastLine && column() == document()->lineLength(lastLine);
}

bool DocumentCursor::gotoNextLine()
{
    // only allow valid cursors
    const bool ok = isValid() && (line() + 1 < document()->lines());

    if (ok) {
        setPosition(Cursor(line() + 1, 0));
    }

    return ok;
}

bool DocumentCursor::gotoPreviousLine()
{
    // only allow valid cursors
    bool ok = (line() > 0) && (column() >= 0);

    if (ok) {
        setPosition(Cursor(line() - 1, 0));
    }

    return ok;
}

bool DocumentCursor::move(int chars, WrapBehavior wrapBehavior)
{
    if (!isValid()) {
        return false;
    }

    // create temporary cursor to modify
    Cursor c(m_cursor);

    // forwards?
    if (chars > 0) {
        // cache lineLength to minimize calls of KTextEditor::DocumentPrivate::lineLength(), as
        // results in locating the correct block in the text buffer every time,
        // which is relatively slow
        int lineLength = document()->lineLength(c.line());

        // special case: cursor position is not in valid text, then the algo does
        // not work for Wrap mode. Hence, catch this special case by setting
        // c.column() to the lineLength()
        if (wrapBehavior == Wrap && c.column() > lineLength) {
            c.setColumn(lineLength);
        }

        while (chars != 0) {
            if (wrapBehavior == Wrap) {
                const int advance = qMin(lineLength - c.column(), chars);

                if (chars > advance) {
                    if (c.line() + 1 >= document()->lines()) {
                        return false;
                    }

                    c.setPosition(c.line() + 1, 0);
                    chars -= advance + 1; // +1 because of end-of-line wrap

                    // advanced one line, so cache correct line length again
                    lineLength = document()->lineLength(c.line());
                } else {
                    c.setColumn(c.column() + chars);
                    chars = 0;
                }
            } else { // NoWrap
                c.setColumn(c.column() + chars);
                chars = 0;
            }
        }
    }

    // backwards?
    else {
        while (chars != 0) {
            const int back = qMin(c.column(), -chars);
            if (-chars > back) {
                if (c.line() == 0) {
                    return false;
                }

                c.setPosition(c.line() - 1, document()->lineLength(c.line() - 1));
                chars += back + 1; // +1 because of wrap-around at start-of-line
            } else {
                c.setColumn(c.column() + chars);
                chars = 0;
            }
        }
    }

    if (c != m_cursor) {
        setPosition(c);
    }
    return true;
}

}
