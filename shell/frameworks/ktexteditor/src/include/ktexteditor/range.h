/*
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2001-2005 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002 Christian Couder <christian@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_RANGE_H
#define KTEXTEDITOR_RANGE_H

#include <ktexteditor_export.h>

#include <ktexteditor/cursor.h>
#include <ktexteditor/linerange.h>

#include <QtGlobal>

class QDebug;
class QString;
class QStringView;

namespace KTextEditor
{
/**
 * \class Range range.h <KTextEditor/Range>
 *
 * \short An object representing a section of text, from one Cursor to another.
 *
 * A Range is a basic class which represents a range of text with two Cursors,
 * from a start() position to an end() position.
 *
 * For simplicity and convenience, ranges always maintain their start position to
 * be before or equal to their end position.  Attempting to set either the
 * start or end of the range beyond the respective end or start will result in
 * both values being set to the specified position.  In the constructor, the
 * start and end will be swapped if necessary.
 *
 * If you want additional functionality such as the ability to maintain position
 * in a document, see MovingRange.
 *
 * \sa MovingRange
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
class KTEXTEDITOR_EXPORT Range
{
public:
    /**
     * Default constructor. Creates a valid range from position (0, 0) to
     * position (0, 0).
     */
    constexpr Range() noexcept = default;

    /**
     * Constructor which creates a range from \e start to \e end.
     * If start is after end, they will be swapped.
     *
     * \param start start position
     * \param end end position
     */
    constexpr Range(Cursor start, Cursor end) noexcept
        : m_start(qMin(start, end))
        , m_end(qMax(start, end))
    {
    }

    /**
     * Constructor which creates a single-line range from \p start,
     * extending \p width characters along the same line.
     *
     * \param start start position
     * \param width width of this range in columns along the same line
     */
    constexpr Range(Cursor start, int width) noexcept
        : m_start(qMin(start, Cursor(start.line(), start.column() + width)))
        , m_end(qMax(start, Cursor(start.line(), start.column() + width)))
    {
    }

    /**
     * Constructor which creates a range from \p start, to \p endLine, \p endColumn.
     *
     * \param start start position
     * \param endLine end line
     * \param endColumn end column
     */
    constexpr Range(Cursor start, int endLine, int endColumn) noexcept
        : m_start(qMin(start, Cursor(endLine, endColumn)))
        , m_end(qMax(start, Cursor(endLine, endColumn)))
    {
    }

    /**
     * Constructor which creates a range from \e startLine, \e startColumn to \e endLine, \e endColumn.
     *
     * \param startLine start line
     * \param startColumn start column
     * \param endLine end line
     * \param endColumn end column
     */
    constexpr Range(int startLine, int startColumn, int endLine, int endColumn) noexcept
        : m_start(qMin(Cursor(startLine, startColumn), Cursor(endLine, endColumn)))
        , m_end(qMax(Cursor(startLine, startColumn), Cursor(endLine, endColumn)))
    {
    }

    /**
     * Validity check.  In the base class, returns true unless the range starts before (0,0).
     */
    constexpr bool isValid() const noexcept
    {
        return start().isValid() && end().isValid();
    }

    /**
     * Returns an invalid range.
     */
    constexpr static Range invalid() noexcept
    {
        return Range(Cursor::invalid(), Cursor::invalid());
    }

    /**
     * Returns the cursor position as string in the format
     * "start-line:start-column,endl-line:end-column".
     * \see fromString()
     */
    QString toString() const;

    /**
     * Returns a Range created from the string \p str containing the format
     * "[(start-line, start-column), (endl-line:end-column)]".
     * In case the string cannot be parsed, an Range::invalid() is returned.
     * \see toString()
     */
    static Range fromString(QStringView str) noexcept;

    /**
     * \name Position
     *
     * The following functions provide access to, and manipulation of, the range's position.
     * \{
     */

    /**
     * Get the start position of this range. This will always be <= end().
     *
     * \returns const reference to the start position of this range.
     */
    constexpr Cursor start() const noexcept
    {
        return m_start;
    }

    /**
     * Get the end position of this range. This will always be >= start().
     *
     * \returns const reference to the end position of this range.
     */
    constexpr Cursor end() const noexcept
    {
        return m_end;
    }

    /**
     * Convert this Range to a LineRange
     *
     * @return LineRange from the start line to the end line of this range.
     */
    constexpr LineRange toLineRange() const noexcept
    {
        return {start().line(), end().line()};
    }

    /**
     * Convenience function.  Set the start and end lines to \p line.
     *
     * \param line the line number to assign to start() and end()
     */
    void setBothLines(int line) noexcept;

    /**
     * Convenience function.  Set the start and end columns to \p column.
     *
     * \param column the column number to assign to start() and end()
     */
    void setBothColumns(int column) noexcept;

    /**
     * Set the start and end cursors to \e range.start() and \e range.end() respectively.
     *
     * \param range range to assign to this range
     */
    void setRange(Range range) noexcept;

    /**
     * \overload
     * \n \n
     * Set the start and end cursors to \e start and \e end respectively.
     *
     * \note If \e start is after \e end, they will be reversed.
     *
     * \param start start cursor
     * \param end end cursor
     */
    void setRange(Cursor start, Cursor end) noexcept;

    /**
     * Set the start cursor to \e start.
     *
     * \note If \e start is after current end, start and end will be set to new start value.
     *
     * \param start new start cursor
     */
    void setStart(Cursor start) noexcept
    {
        if (start > end()) {
            setRange(start, start);
        } else {
            setRange(start, end());
        }
    }

    /**
     * Set the end cursor to \e end.
     *
     * \note If \e end is in front of current start, start and end will be set to new end value.
     *
     * \param end new end cursor
     */
    void setEnd(Cursor end) noexcept
    {
        if (end < start()) {
            setRange(end, end);
        } else {
            setRange(start(), end);
        }
    }

    /**
     * Expand this range if necessary to contain \p range.
     *
     * \param range range which this range should contain
     *
     * \return \e true if expansion occurred, \e false otherwise
     */
    bool expandToRange(Range range) noexcept;

    /**
     * Confine this range if necessary to fit within \p range.
     *
     * \param range range which should contain this range
     *
     * \return \e true if confinement occurred, \e false otherwise
     */
    bool confineToRange(Range range) noexcept;

    /**
     * Check whether this range is wholly contained within one line, ie. if
     * the start() and end() positions are on the same line.
     *
     * \return \e true if both the start and end positions are on the same
     *         line, otherwise \e false
     */
    constexpr bool onSingleLine() const noexcept
    {
        return start().line() == end().line();
    }

    /**
     * Returns the number of lines separating the start() and end() positions.
     *
     * \return the number of lines separating the start() and end() positions;
     *         0 if the start and end lines are the same.
     */
    constexpr int numberOfLines() const noexcept
    {
        return end().line() - start().line();
    }

    /**
     * Returns the number of columns separating the start() and end() positions.
     *
     * \return the number of columns separating the start() and end() positions;
     *         0 if the start and end columns are the same.
     */
    constexpr int columnWidth() const noexcept
    {
        return end().column() - start().column();
    }

    /**
     * Returns true if this range contains no characters, ie. the start() and
     * end() positions are the same.
     *
     * \returns \e true if the range contains no characters, otherwise \e false
     */
    constexpr bool isEmpty() const noexcept
    {
        return start() == end();
    }

    // BEGIN comparison functions
    /**
     * \}
     *
     * \name Comparison
     *
     * The following functions perform checks against this range in comparison
     * to other lines, columns, cursors, and ranges.
     * \{
     */
    /**
     * Check whether the this range wholly encompasses \e range.
     *
     * \param range range to check
     *
     * \return \e true, if this range contains \e range, otherwise \e false
     */
    constexpr bool contains(Range range) const noexcept
    {
        return range.start() >= start() && range.end() <= end();
    }

    /**
     * Check to see if \p cursor is contained within this range, ie >= start() and \< end().
     *
     * \param cursor the position to test for containment
     *
     * \return \e true if the cursor is contained within this range, otherwise \e false.
     */
    constexpr bool contains(Cursor cursor) const noexcept
    {
        return cursor >= start() && cursor < end();
    }

    /**
     * Returns true if this range wholly encompasses \p line.
     *
     * \param line line to check
     *
     * \return \e true if the line is wholly encompassed by this range, otherwise \e false.
     */
    constexpr bool containsLine(int line) const noexcept
    {
        return (line > start().line() || (line == start().line() && !start().column())) && line < end().line();
    }

    /**
     * Check whether the range contains \e column.
     *
     * \param column column to check
     *
     * \return \e true if the range contains \e column, otherwise \e false
     */
    constexpr bool containsColumn(int column) const noexcept
    {
        return column >= start().column() && column < end().column();
    }

    /**
     * Check whether the this range overlaps with \e range.
     *
     * \param range range to check against
     *
     * \return \e true, if this range overlaps with \e range, otherwise \e false
     */
    constexpr bool overlaps(Range range) const noexcept
    {
        return (range.start() <= start()) ? (range.end() > start()) : (range.end() >= end()) ? (range.start() < end()) : contains(range);
    }

    /**
     * Check whether the range overlaps at least part of \e line.
     *
     * \param line line to check
     *
     * \return \e true, if the range overlaps at least part of \e line, otherwise \e false
     */
    constexpr bool overlapsLine(int line) const noexcept
    {
        return line >= start().line() && line <= end().line();
    }

    /**
     * Check to see if this range overlaps \p column; that is, if \p column is
     * between start().column() and end().column().  This function is most likely
     * to be useful in relation to block text editing.
     *
     * \param column the column to test
     *
     * \return \e true if the column is between the range's starting and ending
     *         columns, otherwise \e false.
     */
    constexpr bool overlapsColumn(int column) const noexcept
    {
        return start().column() <= column && end().column() > column;
    }

    /**
     * Check whether \p cursor is located at either of the start() or end()
     * boundaries.
     *
     * \param cursor cursor to check
     *
     * \return \e true if the cursor is equal to \p start() or \p end(),
     *         otherwise \e false.
     */
    constexpr bool boundaryAtCursor(Cursor cursor) const noexcept
    {
        return cursor == start() || cursor == end();
    }
    //!\}
    // END

    /**
     * Intersects this range with another, returning the shared area of
     * the two ranges.
     *
     * \param range other range to intersect with this
     *
     * \return the intersection of this range and the supplied \a range.
     */
    constexpr Range intersect(Range range) const noexcept
    {
        return ((!isValid() || !range.isValid() || *this > range || *this < range)) ? invalid() : Range(qMax(start(), range.start()), qMin(end(), range.end()));
    }

    /**
     * Returns the smallest range which encompasses this range and the
     * supplied \a range.
     *
     * \param range other range to encompass
     *
     * \return the smallest range which contains this range and the supplied \a range.
     */
    constexpr Range encompass(Range range) const noexcept
    {
        return (!isValid())      ? (range.isValid() ? range : invalid())
            : (!range.isValid()) ? (*this)
                                 : Range(qMin(start(), range.start()), qMax(end(), range.end()));
    }

    /**
     * Addition operator. Takes two ranges and returns their summation.
     *
     * \param r1 the first range
     * \param r2 the second range
     *
     * \return a the summation of the two input ranges
     */
    constexpr friend Range operator+(Range r1, Range r2) noexcept
    {
        return Range(r1.start() + r2.start(), r1.end() + r2.end());
    }

    /**
     * Addition assignment operator. Adds \p r2 to this range.
     *
     * \param r1 the first range
     * \param r2 the second range
     *
     * \return a reference to the cursor which has just been added to
     */
    friend Range &operator+=(Range &r1, Range r2) noexcept
    {
        r1.setRange(r1.start() + r2.start(), r1.end() + r2.end());
        return r1;
    }

    /**
     * Subtraction operator. Takes two ranges and returns the subtraction
     * of \p r2 from \p r1.
     *
     * \param r1 the first range
     * \param r2 the second range
     *
     * \return a range representing the subtraction of \p r2 from \p r1
     */
    constexpr friend Range operator-(Range r1, Range r2) noexcept
    {
        return Range(r1.start() - r2.start(), r1.end() - r2.end());
    }

    /**
     * Subtraction assignment operator. Subtracts \p r2 from \p r1.
     *
     * \param r1 the first range
     * \param r2 the second range
     *
     * \return a reference to the range which has just been subtracted from
     */
    friend Range &operator-=(Range &r1, Range r2) noexcept
    {
        r1.setRange(r1.start() - r2.start(), r1.end() - r2.end());
        return r1;
    }

    /**
     * Intersects \a r1 and \a r2.
     *
     * \param r1 the first range
     * \param r2 the second range
     *
     * \return the intersected range, invalid() if there is no overlap
     */
    constexpr friend Range operator&(Range r1, Range r2) noexcept
    {
        return r1.intersect(r2);
    }

    /**
     * Intersects \a r1 with \a r2 and assigns the result to \a r1.
     *
     * \param r1 the range to assign the intersection to
     * \param r2 the range to intersect \a r1 with
     *
     * \return a reference to this range, after the intersection has taken place
     */
    friend Range &operator&=(Range &r1, Range r2) noexcept
    {
        r1.setRange(r1.intersect(r2));
        return r1;
    }

    /**
     * Equality operator.
     *
     * \param r1 first range to compare
     * \param r2 second range to compare
     *
     * \return \e true if \e r1 and \e r2 equal, otherwise \e false
     */
    constexpr friend bool operator==(Range r1, Range r2) noexcept
    {
        return r1.start() == r2.start() && r1.end() == r2.end();
    }

    /**
     * Inequality operator.
     *
     * \param r1 first range to compare
     * \param r2 second range to compare
     *
     * \return \e true if \e r1 and \e r2 do \e not equal, otherwise \e false
     */
    constexpr friend bool operator!=(Range r1, Range r2) noexcept
    {
        return r1.start() != r2.start() || r1.end() != r2.end();
    }

    /**
     * Greater than operator.  Looks only at the position of the two ranges,
     * does not consider their size.
     *
     * \param r1 first range to compare
     * \param r2 second range to compare
     *
     * \return \e true if \e r1 starts after where \e r2 ends, otherwise \e false
     */
    constexpr friend bool operator>(Range r1, Range r2) noexcept
    {
        return r1.start() > r2.end();
    }

    /**
     * Less than operator.  Looks only at the position of the two ranges,
     * does not consider their size.
     *
     * \param r1 first range to compare
     * \param r2 second range to compare
     *
     * \return \e true if \e r1 ends before \e r2 begins, otherwise \e false
     */
    constexpr friend bool operator<(Range r1, Range r2) noexcept
    {
        return r1.end() < r2.start();
    }

private:
    /**
     * This range's start cursor pointer.
     *
     * \internal
     */
    Cursor m_start;

    /**
     * This range's end cursor pointer.
     *
     * \internal
     */
    Cursor m_end;
};

/**
 * QHash function for KTextEditor::Range.
 * Returns the hash value for @p range.
 */
KTEXTEDITOR_EXPORT size_t qHash(KTextEditor::Range range, size_t seed = 0) noexcept;
}

Q_DECLARE_TYPEINFO(KTextEditor::Range, Q_RELOCATABLE_TYPE);

/**
 * qDebug() stream operator.  Writes this range to the debug output in a nicely formatted way.
 */
KTEXTEDITOR_EXPORT QDebug operator<<(QDebug s, KTextEditor::Range range);

namespace QTest
{
// forward declaration of template in qtestcase.h
template<typename T>
char *toString(const T &);

/**
 * QTestLib integration to have nice output in e.g. QCOMPARE failures.
 */
template<>
KTEXTEDITOR_EXPORT char *toString(const KTextEditor::Range &range);
}

#endif
