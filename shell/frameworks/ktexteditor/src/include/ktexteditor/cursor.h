/*
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2001-2005 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2002 Christian Couder <christian@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_CURSOR_H
#define KTEXTEDITOR_CURSOR_H

#include <ktexteditor_export.h>

#include <QtGlobal>

class QDebug;
class QString;
class QStringView;

namespace KTextEditor
{
class Document;
class Range;

/**
 * \class Cursor cursor.h <KTextEditor/Cursor>
 *
 * \short The Cursor represents a position in a Document.
 *
 * \section kte_cursor_intro Introduction
 * A Cursor represents a position in a Document through a tuple
 * of two int%s, namely the line() and column(). A Cursor maintains
 * no affiliation with a particular Document, meaning that it remains
 * constant if not changed through the Cursor API.
 *
 * \section kte_cursor_notes Important Notes
 *
 * Working with a cursor, one should be aware of the following notes:
 * - Lines and columns start a 0.
 * - The Cursor class is designed to be passed by value (only 8 Bytes).
 * - Think of cursors as having their position at the start of a character,
 *   not in the middle of one.
 * - invalid() Cursor%s are located at (-1, -1). In addition, a Cursor
 *   is invalid(), if either its line() and/or its column() is arbitrarily
 *   negative, i.e. < 0.
 * - All Cursor%s with line() >= 0 and column() >= 0 are valid. In this case
 *   isValid() returns \e true.
 * - A Cursor has a non-virtual destructor. Hence, you cannot derive from Cursor.
 *
 * \section kte_cursor_properties Cursor Efficiency
 *
 * The Cursor consists of just two int%s, the line() and the column().
 * Therefore, a Cursor instance takes 8 Bytes of memory. Further, a Cursor
 * is a non-virtual class, turning it into a primitive old data type (POD).
 * Thus, it can be moved and copied very efficiently.
 *
 * \section kte_cursor_more Additional Concepts
 *
 * In addition to the Cursor, the KTextEditor API provides advanced concepts:
 * - The DocumentCursor is a Cursor bound to a specific Document. In addition
 *   to the Cursor API, it provides convenience functions like
 *   DocumentCursor::isValidTextPosition() or DocumentCursor::move().
 *   The DocumentCursor does not maintain its position, though.
 * - The MovingCursor is also bound to a specific Document. In addition to the
 *   DocumentCursor, the MovingCursor maintains its position, meaning that
 *   whenever the Document changes, the MovingCursor moves, too.
 * - The Cursor forms the basis for the Range.
 *
 * \sa DocumentCursor, MovingCursor, Range
 */
class KTEXTEDITOR_EXPORT Cursor
{
public:
    /**
     * The default constructor creates a cursor at position (0, 0).
     */
    constexpr Cursor() noexcept = default;

    /**
     * This constructor creates a cursor initialized with \p line
     * and \p column.
     * \param line line for cursor
     * \param column column for cursor
     */
    constexpr Cursor(int line, int column) noexcept
        : m_line(line)
        , m_column(column)
    {
    }

    /**
     * Returns whether the current position of this cursor is a valid position
     * (line + column must both be >= 0).
     *
     * @note If you want to check, whether a cursor position is a valid
     *       \e text-position, use DocumentCursor::isValidTextPosition(),
     *       or Document::isValidTextPosition().
     */
    constexpr bool isValid() const noexcept
    {
        return m_line >= 0 && m_column >= 0;
    }

    /**
     * Returns an invalid cursor.
     * The returned cursor position is set to (-1, -1).
     * \see isValid()
     */
    constexpr static Cursor invalid() noexcept
    {
        return Cursor(-1, -1);
    }

    /**
     * Returns a cursor representing the start of any document - i.e., line 0, column 0.
     */
    constexpr static Cursor start() noexcept
    {
        return Cursor();
    }

    /**
     * Returns the cursor position as string in the format "(line, column)".
     * \see fromString()
     */
    QString toString() const;

    /**
     * Returns a Cursor created from the string \p str containing the format
     * "(line, column)". In case the string cannot be parsed, Cursor::invalid()
     * is returned.
     * \see toString()
     */
    static Cursor fromString(QStringView str) noexcept;

    /**
     * \name Position
     *
     * The following functions provide access to, and manipulation of, the cursor's position.
     * \{
     */
    /**
     * Set the current cursor position to \e position.
     *
     * \param position new cursor position
     */
    void setPosition(Cursor position) noexcept
    {
        m_line = position.m_line;
        m_column = position.m_column;
    }

    /**
     * \overload
     *
     * Set the cursor position to \e line and \e column.
     *
     * \param line new cursor line
     * \param column new cursor column
     */
    void setPosition(int line, int column) noexcept
    {
        m_line = line;
        m_column = column;
    }

    /**
     * Retrieve the line on which this cursor is situated.
     * \return line number, where 0 is the first line.
     */
    constexpr int line() const noexcept
    {
        return m_line;
    }

    /**
     * Set the cursor line to \e line.
     * \param line new cursor line
     */
    void setLine(int line) noexcept
    {
        m_line = line;
    }

    /**
     * Retrieve the column on which this cursor is situated.
     * \return column number, where 0 is the first column.
     */
    constexpr int column() const noexcept
    {
        return m_column;
    }

    /**
     * Set the cursor column to \e column.
     * \param column new cursor column
     */
    void setColumn(int column) noexcept
    {
        m_column = column;
    }

    /**
     * Determine if this cursor is located at the start of a line (= at column 0).
     * \return \e true if the cursor is situated at the start of the line, \e false if it isn't.
     */
    constexpr bool atStartOfLine() const noexcept
    {
        return m_column == 0;
    }

    /**
     * Determine if this cursor is located at the start of a document (= at position (0, 0)).
     * \return \e true if the cursor is situated at the start of the document, \e false if it isn't.
     */
    constexpr bool atStartOfDocument() const noexcept
    {
        return m_line == 0 && m_column == 0;
    }

    /**
     * Get both the line and column of the cursor position.
     * \param line will be filled with current cursor line
     * \param column will be filled with current cursor column
     */
    void position(int &line, int &column) const noexcept
    {
        line = m_line;
        column = m_column;
    }
    //!\}

    /**
     * Addition operator. Takes two cursors and returns their summation.
     * \param c1 the first position
     * \param c2 the second position
     * \return a the summation of the two input cursors
     */
    constexpr friend Cursor operator+(Cursor c1, Cursor c2) noexcept
    {
        return Cursor(c1.line() + c2.line(), c1.column() + c2.column());
    }

    /**
     * Addition assignment operator. Adds \p c2 to this cursor.
     * \param c1 the cursor being added to
     * \param c2 the position to add
     * \return a reference to the cursor which has just been added to
     */
    friend Cursor &operator+=(Cursor &c1, Cursor c2) noexcept
    {
        c1.setPosition(c1.line() + c2.line(), c1.column() + c2.column());
        return c1;
    }

    /**
     * Subtraction operator. Takes two cursors and returns the subtraction
     * of \p c2 from \p c1.
     *
     * \param c1 the first position
     * \param c2 the second position
     * \return a cursor representing the subtraction of \p c2 from \p c1
     */
    constexpr friend Cursor operator-(Cursor c1, Cursor c2) noexcept
    {
        return Cursor(c1.line() - c2.line(), c1.column() - c2.column());
    }

    /**
     * Subtraction assignment operator. Subtracts \p c2 from \p c1.
     * \param c1 the cursor being subtracted from
     * \param c2 the position to subtract
     * \return a reference to the cursor which has just been subtracted from
     */
    friend Cursor &operator-=(Cursor &c1, Cursor c2) noexcept
    {
        c1.setPosition(c1.line() - c2.line(), c1.column() - c2.column());
        return c1;
    }

    /**
     * Equality operator.
     *
     * \note comparison between two invalid cursors is undefined.
     *       comparison between and invalid and a valid cursor will always be \e false.
     *
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's and c2's line and column are \e equal.
     */
    constexpr friend bool operator==(Cursor c1, Cursor c2) noexcept
    {
        return c1.line() == c2.line() && c1.column() == c2.column();
    }

    /**
     * Inequality operator.
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's and c2's line and column are \e not equal.
     */
    constexpr friend bool operator!=(Cursor c1, Cursor c2) noexcept
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
    constexpr friend bool operator>(Cursor c1, Cursor c2) noexcept
    {
        return c1.line() > c2.line() || (c1.line() == c2.line() && c1.m_column > c2.m_column);
    }

    /**
     * Greater than or equal to operator.
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's position is greater than or equal to c2's
     *         position, otherwise \e false.
     */
    constexpr friend bool operator>=(Cursor c1, Cursor c2) noexcept
    {
        return c1.line() > c2.line() || (c1.line() == c2.line() && c1.m_column >= c2.m_column);
    }

    /**
     * Less than operator.
     * \param c1 first cursor to compare
     * \param c2 second cursor to compare
     * \return \e true, if c1's position is greater than or equal to c2's
     *         position, otherwise \e false.
     */
    constexpr friend bool operator<(Cursor c1, Cursor c2) noexcept
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
    constexpr friend bool operator<=(Cursor c1, Cursor c2) noexcept
    {
        return !(c1 > c2);
    }

private:
    /**
     * \internal
     *
     * Cursor line
     */
    int m_line = 0;

    /**
     * \internal
     *
     * Cursor column
     */
    int m_column = 0;
};

/**
 * QHash function for KTextEditor::Cursor.
 * Returns the hash value for @p cursor.
 */
KTEXTEDITOR_EXPORT size_t qHash(KTextEditor::Cursor cursor, size_t seed = 0) noexcept;

} // namespace KTextEditor

Q_DECLARE_TYPEINFO(KTextEditor::Cursor, Q_PRIMITIVE_TYPE);

/**
 * qDebug() stream operator.  Writes this cursor to the debug output in a nicely formatted way.
 */
KTEXTEDITOR_EXPORT QDebug operator<<(QDebug s, KTextEditor::Cursor cursor);

namespace QTest
{
// forward declaration of template in qtestcase.h
template<typename T>
char *toString(const T &);

/**
 * QTestLib integration to have nice output in e.g. QCOMPARE failures.
 */
template<>
KTEXTEDITOR_EXPORT char *toString(const KTextEditor::Cursor &cursor);
}

#endif
