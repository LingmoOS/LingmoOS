/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2012-2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_DOCUMENT_CURSOR_H
#define KTEXTEDITOR_DOCUMENT_CURSOR_H

#include <ktexteditor/cursor.h>
#include <ktexteditor/document.h>
#include <ktexteditor_export.h>

#include <QDebug>

namespace KTextEditor
{
/**
 * \class DocumentCursor documentcursor.h <KTextEditor/DocumentCursor>
 *
 * \short A Cursor which is bound to a specific Document.
 *
 * \section documentcursor_intro Introduction
 * A DocumentCursor is an extension of the basic Cursor class.
 * The DocumentCursor is bound to a specific Document instance.
 * This way, the cursor provides additional functions like gotoNextLine(),
 * gotoPreviousLine() and move() according to the WrapBehavior.
 *
 * The only difference to a MovingCursor is that the DocumentCursor's
 * position does not automatically move on text manipulation.
 *
 * \section documentcursor_position Validity
 *
 * When constructing a DocumentCursor, a valid document pointer is required
 * in the constructor. A null pointer will assert in debug builds.
 * Further, a DocumentCursor should only be used as long as the Document
 * exists, otherwise the DocumentCursor contains a dangling pointer to the
 * previously assigned Document.
 *
 * \section documentcursor_example Example
 *
 * A DocumentCursor is created and used like this:
 * \code
 * KTextEditor::DocumentCursor docCursor(document);
 * docCursor.setPosition(0, 0);
 * docCursor.gotoNextLine();
 * docCursor.move(5); // move 5 characters to the right
 * \endcode
 *
 * \see KTextEditor::Cursor, KTextEditor::MovingCursor
 *
 * \author Dominik Haumann \<dhaumann@kde.org\>
 */
class KTEXTEDITOR_EXPORT DocumentCursor
{
    //
    // sub types
    //
public:
    /**
     * Wrap behavior for end of line treatement used in move().
     */
    enum WrapBehavior {
        Wrap = 0x0, ///< wrap at end of line
        NoWrap = 0x1 ///< do not wrap at end of line
    };

    //
    // Constructor
    //
public:
    /**
     * Constructor that creates a DocumentCursor at the \e invalid position
     * (-1, -1).
     * \see isValid()
     */
    DocumentCursor(KTextEditor::Document *document);

    /**
     * Constructor that creates a DocumentCursor located at \p position.
     */
    DocumentCursor(KTextEditor::Document *document, KTextEditor::Cursor position);

    /**
     * Constructor that creates a DocumentCursor located at \p line and \p column.
     */
    DocumentCursor(KTextEditor::Document *document, int line, int column);

    /**
     * Copy constructor. Make sure the Document of the DocumentCursor is
     * valid.
     */
    DocumentCursor(const DocumentCursor &other);

    //
    // stuff that needs to be implemented by editor part cursors
    //
public:
    /**
     * Gets the document to which this cursor is bound.
     * \return a pointer to the document
     */
    inline Document *document() const
    {
        return m_document;
    }

    /**
     * Set the current cursor position to \e position.
     * If \e position is not valid, meaning that either its line < 0 or its
     * column < 0, then the document cursor will also be invalid
     *
     * \param position new cursor position
     */
    void setPosition(KTextEditor::Cursor position)
    {
        m_cursor = position;
    }

    /**
     * Retrieve the line on which this cursor is situated.
     * \return line number, where 0 is the first line.
     */
    inline int line() const
    {
        return m_cursor.line();
    }

    /**
     * Retrieve the column on which this cursor is situated.
     * \return column number, where 0 is the first column.
     */
    inline int column() const
    {
        return m_cursor.column();
    }

    /**
     * Destruct the moving cursor.
     */
    ~DocumentCursor()
    {
    }

    //
    // forbidden stuff
    //
public:
    /**
     * no default constructor, as we need a document.
     */
    DocumentCursor() = delete;

    //
    // convenience API
    //
public:
    /**
     * Check if the current position of this cursor is a valid position,
     * i.e. whether line() >= 0 and column() >= 0.
     * \return \e true, if the cursor position is valid, otherwise \e false
     * \see KTextEditor::Cursor::isValid(), isValidTextPosition()
     */
    inline bool isValid() const
    {
        return m_cursor.isValid();
    }

    /**
     * Check if this cursor is currently at a valid text position.
     * A cursor position at (line, column) is valid, if
     * - line >= 0 and line < lines() holds, and
     * - column >= 0 and column <= lineLength(column).
     *
     * The text position is also invalid if it is inside a Unicode surrogate.
     * Therefore, use this function when iterating over the characters of a line.
     *
     * \see KTextEditor::Document::isValidTextPosition(), isValid()
     */
    inline bool isValidTextPosition() const
    {
        return document()->isValidTextPosition(m_cursor);
    }

    /**
     * Make sure the cursor position is at a valid text position according to
     * the following rules.
     * - If the cursor is invalid(), i.e. either line < 0 or column < 0, it is
     *   set to (0, 0).
     * - If the cursor's line is past the number of lines in the document, the
     *   cursor is set to Document::documentEnd().
     * - If the cursor's column is past the line length, the cursor column is
     *   set to the line length.
     * - If the cursor is inside a Unicode surrogate, the cursor is moved to the
     *   beginning of the Unicode surrogate.
     *
     * After calling makeValid(), the cursor is guaranteed to be located at
     * a valid text position.
     * \see isValidTextPosition(), isValid()
     */
    void makeValid();

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
     * Set the cursor line to \e line.
     * \param line new cursor line
     */
    void setLine(int line);

    /**
     * Set the cursor column to \e column.
     * \param column new cursor column
     */
    void setColumn(int column);

    /**
     * Determine if this cursor is located at column 0 of a valid text line.
     *
     * \return \e true if cursor is a valid text position and column()=0, otherwise \e false.
     */
    bool atStartOfLine() const;

    /**
     * Determine if this cursor is located at the end of the current line.
     *
     * \return \e true if the cursor is situated at the end of the line, otherwise \e false.
     */
    bool atEndOfLine() const;

    /**
     * Determine if this cursor is located at line 0 and column 0.
     *
     * \return \e true if the cursor is at start of the document, otherwise \e false.
     */
    bool atStartOfDocument() const;

    /**
     * Determine if this cursor is located at the end of the last line in the
     * document.
     *
     * \return \e true if the cursor is at the end of the document, otherwise \e false.
     */
    bool atEndOfDocument() const;

    /**
     * Moves the cursor to the next line and sets the column to 0. If the cursor
     * position is already in the last line of the document, the cursor position
     * remains unchanged and the return value is \e false.
     *
     * \return \e true on success, otherwise \e false
     */
    bool gotoNextLine();

    /**
     * Moves the cursor to the previous line and sets the column to 0. If the
     * cursor position is already in line 0, the cursor position remains
     * unchanged and the return value is \e false.
     *
     * \return \e true on success, otherwise \e false
     */
    bool gotoPreviousLine();

    /**
     * Moves the cursor \p chars character forward or backwards. If \e wrapBehavior
     * equals WrapBehavior::Wrap, the cursor is automatically wrapped to the
     * next line at the end of a line.
     *
     * When moving backwards, the WrapBehavior does not have any effect.
     * \note If the cursor could not be moved the amount of chars requested,
     *       the cursor is not moved at all!
     *
     * \return \e true on success, otherwise \e false
     */
    bool move(int chars, WrapBehavior wrapBehavior = Wrap);

    /**
     * Convert this clever cursor into a dumb one.
     * @return normal cursor
     */
    inline Cursor toCursor() const
    {
        return m_cursor;
    }

    /**
     * Convert this clever cursor into a dumb one. Equal to toCursor, allowing to use implicit conversion.
     * @return normal cursor
     */
    inline operator Cursor() const
    {
        return m_cursor;
    }

    //
    // operators for: DocumentCursor <-> DocumentCursor
    //
    /**
     * Assignment operator. Same as the copy constructor. Make sure that
     * the assigned Document is a valid document pointer.
     */
    DocumentCursor &operator=(const DocumentCursor &other)
    {
        m_document = other.m_document;
        m_cursor = other.m_cursor;
        return *this;
    }

    /**
     * Equality operator.
     *
     * \note comparison between two invalid cursors is undefined.
     *       comparison between an invalid and a valid cursor will always be \e false.
     *
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's and c2's assigned document, line and column are \e equal.
     */
    inline friend bool operator==(DocumentCursor c1, DocumentCursor c2)
    {
        return c1.document() == c2.document() && c1.line() == c2.line() && c1.column() == c2.column();
    }

    /**
     * Inequality operator.
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's and c2's assigned document, line and column are \e not equal.
     */
    inline friend bool operator!=(DocumentCursor c1, DocumentCursor c2)
    {
        return !(c1 == c2);
    }

    /**
     * Greater than operator.
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's position is greater than c2's position,
     *         otherwise \e false.
     */
    inline friend bool operator>(DocumentCursor c1, DocumentCursor c2)
    {
        return c1.line() > c2.line() || (c1.line() == c2.line() && c1.column() > c2.column());
    }

    /**
     * Greater than or equal to operator.
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's position is greater than or equal to c2's
     *         position, otherwise \e false.
     */
    inline friend bool operator>=(DocumentCursor c1, DocumentCursor c2)
    {
        return c1.line() > c2.line() || (c1.line() == c2.line() && c1.column() >= c2.column());
    }

    /**
     * Less than operator.
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's position is greater than or equal to c2's
     *         position, otherwise \e false.
     */
    inline friend bool operator<(DocumentCursor c1, DocumentCursor c2)
    {
        return !(c1 >= c2);
    }

    /**
     * Less than or equal to operator.
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's position is lesser than or equal to c2's
     *         position, otherwise \e false.
     */
    inline friend bool operator<=(DocumentCursor c1, DocumentCursor c2)
    {
        return !(c1 > c2);
    }

    /**
     * qDebug() stream operator. Writes this cursor to the debug output in a nicely formatted way.
     * @param s debug stream
     * @param cursor cursor to print
     * @return debug stream
     */
    inline friend QDebug operator<<(QDebug s, const DocumentCursor *cursor)
    {
        if (cursor) {
            s.nospace() << "(" << cursor->document() << ": " << cursor->line() << ", " << cursor->column() << ")";
        } else {
            s.nospace() << "(null document cursor)";
        }
        return s.space();
    }

    /**
     * qDebug() stream operator. Writes this cursor to the debug output in a nicely formatted way.
     * @param s debug stream
     * @param cursor cursor to print
     * @return debug stream
     */
    inline friend QDebug operator<<(QDebug s, const DocumentCursor &cursor)
    {
        return s << &cursor;
    }

private:
    KTextEditor::Document *m_document;
    KTextEditor::Cursor m_cursor;
};

}

Q_DECLARE_TYPEINFO(KTextEditor::DocumentCursor, Q_RELOCATABLE_TYPE);

#endif
