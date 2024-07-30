/*
    SPDX-FileCopyrightText: 2001-2005 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2021 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_LINERANGE_H
#define KTEXTEDITOR_LINERANGE_H

#include <ktexteditor_export.h>

#include <QtGlobal>

class QDebug;
class QString;
class QStringView;

namespace KTextEditor
{
/**
 * @class LineRange linerange.h <KTextEditor/LineRange>
 *
 * @short An object representing lines from a start line to an end line.
 *
 * A LineRange is a basic class which represents a range of lines, from a start()
 * line to an end() line.
 *
 * For simplicity and convenience, ranges always maintain their start() line to
 * be before or equal to their end() line. Attempting to set either the
 * start or end of the range beyond the respective end or start will result in
 * both values being set to the specified line. In the constructor, the
 * start and end will be swapped if necessary.
 *
 * @sa Range
 *
 * @since 5.79
 * @author Dominik Haumann \<dhaumann@kde.org\>
 */
class KTEXTEDITOR_EXPORT LineRange
{
public:
    /**
     * Default constructor. Creates a valid line range with both start and end
     * line set to 0.
     */
    constexpr LineRange() noexcept = default;

    /**
     * Constructor which creates a range from \e start to \e end.
     * If start is after end, they will be swapped.
     *
     * @param start start line
     * @param end end line
     */
    constexpr LineRange(int start, int end) noexcept
        : m_start(qMin(start, end))
        , m_end(qMax(start, end))
    {
    }

    /**
     * Validity check. In the base class, returns true unless the line range starts before (0,0).
     */
    constexpr bool isValid() const noexcept
    {
        return m_start >= 0 && m_end >= 0;
    }

    /**
     * Returns an invalid line range.
     */
    constexpr static LineRange invalid() noexcept
    {
        return LineRange(-1, -1);
    }

    /**
     * Returns the line range as string in the format
     * "[start line, end line]".
     * @see fromString()
     */
    QString toString() const;

    /**
     * Returns a LineRange created from the string \p str containing the format
     * "[start line, end line]".
     * In case the string cannot be parsed, an LineRange::invalid() is returned.
     * @see toString()
     */
    static LineRange fromString(QStringView str) noexcept;

    /**
     * @name Position
     *
     * The following functions provide access to, and manipulation of, the range's position.
     * @{
     */

    /**
     * Get the start line of this line range. This will always be <= end().
     *
     * @returns the start line of this line range.
     */
    constexpr int start() const noexcept
    {
        return m_start;
    }

    /**
     * Get the end line of this line range. This will always be >= start().
     *
     * @returns the end line of this line range.
     */
    constexpr int end() const noexcept
    {
        return m_end;
    }

    /**
     * Set the start and end lines to \e start and \e end respectively.
     *
     * \note If \e start is after \e end, they will be reversed.
     *
     * \param start start line
     * \param end end line
     */
    void setRange(LineRange range) noexcept
    {
        setRange(range.start(), range.end());
    }

    /**
     * Set the start and end lines to \e start and \e end respectively.
     *
     * \note If \e start is after \e end, they will be reversed.
     *
     * \param start start line
     * \param end end line
     */
    void setRange(int start, int end) noexcept
    {
        m_start = qMin(start, end);
        m_end = qMax(start, end);
    }

    /**
     * Convenience function.  Set the start and end lines to \p line.
     *
     * @param line the line number to assign to start() and end()
     */
    void setBothLines(int line) noexcept
    {
        m_start = line;
        m_end = line;
    }

    /**
     * Set the start line to \e start.
     *
     * @note If \e start is after current end, start and end will be set to new start value.
     *
     * @param start new start line
     */
    void setStart(int start) noexcept
    {
        if (start > end()) {
            setRange(start, start);
        } else {
            setRange(start, end());
        }
    }

    /**
     * Set the end line to \e end.
     *
     * @note If \e end is in front of current start, start and end will be set to new end value.
     *
     * @param end new end line
     */
    void setEnd(int end) noexcept
    {
        if (end < start()) {
            setRange(end, end);
        } else {
            setRange(start(), end);
        }
    }

    /**
     * Expand this line range if necessary to contain \p range.
     *
     * @param range range which this range should contain
     *
     * @return \e true if expansion occurred, \e false otherwise
     */
    bool expandToRange(LineRange range) noexcept
    {
        if (start() > range.start())
            if (end() < range.end()) {
                setRange(range);
            } else {
                setStart(range.start());
            }
        else if (end() < range.end()) {
            setEnd(range.end());
        } else {
            return false;
        }

        return true;
    }

    /**
     * Confine this range if necessary to fit within \p range.
     *
     * @param range range which should contain this range
     *
     * @return \e true if confinement occurred, \e false otherwise
     */
    bool confineToRange(LineRange range) noexcept
    {
        if (start() < range.start())
            if (end() > range.end()) {
                setRange(range);
            } else {
                setStart(range.start());
            }
        else if (end() > range.end()) {
            setEnd(range.end());
        } else {
            return false;
        }

        return true;
    }

    /**
     * Check whether this line range is on one line.
     *
     * @return \e true if both the start and end line are equal, otherwise \e false
     */
    constexpr bool onSingleLine() const noexcept
    {
        return start() == end();
    }

    /**
     * Returns the number of lines separating the start() and end() line.
     *
     * @return the number of lines separating the start() and end() line;
     *         0 if the start and end lines are the same.
     */
    constexpr int numberOfLines() const noexcept
    {
        return end() - start();
    }

    // BEGIN comparison functions
    /**
     * @}
     *
     * @name Comparison
     *
     * The following functions perform checks against this range in comparison
     * to other lines and ranges.
     * @{
     */
    /**
     * Check whether the this range wholly encompasses \e range.
     *
     * @param range range to check
     *
     * @return \e true, if this range contains \e range, otherwise \e false
     */
    constexpr bool contains(LineRange range) const noexcept
    {
        return range.start() >= start() && range.end() <= end();
    }

    /**
     * Returns true if this range wholly encompasses \p line.
     *
     * @param line line to check
     *
     * @return \e true if the line is wholly encompassed by this range, otherwise \e false.
     */
    constexpr bool containsLine(int line) const noexcept
    {
        return line >= start() && line < end();
    }

    /**
     * Check whether the this range overlaps with \e range.
     *
     * @param range range to check against
     *
     * @return \e true, if this range overlaps with \e range, otherwise \e false
     */
    constexpr bool overlaps(LineRange range) const noexcept
    {
        return (range.start() <= start()) ? (range.end() > start()) : (range.end() >= end()) ? (range.start() < end()) : contains(range);
    }

    /**
     * Check whether the range overlaps at least part of \e line.
     *
     * @param line line to check
     *
     * @return \e true, if the range overlaps at least part of \e line, otherwise \e false
     */
    constexpr bool overlapsLine(int line) const noexcept
    {
        return line >= start() && line <= end();
    }
    //!\}
    // END

    /**
     * Intersects this line range with another, returning the shared lines of
     * the two line ranges.
     *
     * @param range other line range to intersect with this
     *
     * @return the intersection of this range and the supplied \a range.
     */
    constexpr LineRange intersect(LineRange range) const noexcept
    {
        return ((!isValid() || !range.isValid() || *this > range || *this < range)) ? invalid()
                                                                                    : LineRange(qMax(start(), range.start()), qMin(end(), range.end()));
    }

    /**
     * Returns the smallest range which encompasses this line range and the
     * supplied \a range.
     *
     * @param range other range to encompass
     *
     * @return the smallest range which contains this range and the supplied \a range.
     */
    constexpr LineRange encompass(LineRange range) const noexcept
    {
        return (!isValid())      ? (range.isValid() ? range : invalid())
            : (!range.isValid()) ? (*this)
                                 : LineRange(qMin(start(), range.start()), qMax(end(), range.end()));
    }

    /**
     * Addition operator. Takes two ranges and returns their summation.
     *
     * @param r1 the first range
     * @param r2 the second range
     *
     * @return a the summation of the two input ranges
     */
    constexpr friend LineRange operator+(LineRange r1, LineRange r2) noexcept
    {
        return LineRange(r1.start() + r2.start(), r1.end() + r2.end());
    }

    /**
     * Addition assignment operator. Adds \p r2 to this range.
     *
     * @param r1 the first range
     * @param r2 the second range
     *
     * @return a reference to the line range which has just been added to
     */
    friend LineRange &operator+=(LineRange &r1, LineRange r2) noexcept
    {
        r1.setRange(r1.start() + r2.start(), r1.end() + r2.end());
        return r1;
    }

    /**
     * Subtraction operator. Takes two ranges and returns the subtraction
     * of \p r2 from \p r1.
     *
     * @param r1 the first range
     * @param r2 the second range
     *
     * @return a range representing the subtraction of \p r2 from \p r1
     */
    constexpr friend LineRange operator-(LineRange r1, LineRange r2) noexcept
    {
        return LineRange(r1.start() - r2.start(), r1.end() - r2.end());
    }

    /**
     * Subtraction assignment operator. Subtracts \p r2 from \p r1.
     *
     * @param r1 the first range
     * @param r2 the second range
     *
     * @return a reference to the range which has just been subtracted from
     */
    friend LineRange &operator-=(LineRange &r1, LineRange r2) noexcept
    {
        r1.setRange(r1.start() - r2.start(), r1.end() - r2.end());
        return r1;
    }

    /**
     * Intersects \a r1 and \a r2.
     *
     * @param r1 the first range
     * @param r2 the second range
     *
     * @return the intersected range, invalid() if there is no overlap
     */
    constexpr friend LineRange operator&(LineRange r1, LineRange r2) noexcept
    {
        return r1.intersect(r2);
    }

    /**
     * Intersects \a r1 with \a r2 and assigns the result to \a r1.
     *
     * @param r1 the range to assign the intersection to
     * @param r2 the range to intersect \a r1 with
     *
     * @return a reference to this range, after the intersection has taken place
     */
    friend LineRange &operator&=(LineRange &r1, LineRange r2) noexcept
    {
        r1.setRange(r1.intersect(r2));
        return r1;
    }

    /**
     * Equality operator.
     *
     * @param r1 first range to compare
     * @param r2 second range to compare
     *
     * @return \e true if \e r1 and \e r2 equal, otherwise \e false
     */
    constexpr friend bool operator==(LineRange r1, LineRange r2) noexcept
    {
        return r1.start() == r2.start() && r1.end() == r2.end();
    }

    /**
     * Inequality operator.
     *
     * @param r1 first range to compare
     * @param r2 second range to compare
     *
     * @return \e true if \e r1 and \e r2 do \e not equal, otherwise \e false
     */
    constexpr friend bool operator!=(LineRange r1, LineRange r2) noexcept
    {
        return r1.start() != r2.start() || r1.end() != r2.end();
    }

    /**
     * Greater than operator.  Looks only at the lines of the two ranges,
     * does not consider their size.
     *
     * @param r1 first range to compare
     * @param r2 second range to compare
     *
     * @return \e true if \e r1 starts after where \e r2 ends, otherwise \e false
     */
    constexpr friend bool operator>(LineRange r1, LineRange r2) noexcept
    {
        return r1.start() > r2.end();
    }

    /**
     * Less than operator.  Looks only at the lines of the two ranges,
     * does not consider their size.
     *
     * @param r1 first range to compare
     * @param r2 second range to compare
     *
     * @return \e true if \e r1 ends before \e r2 begins, otherwise \e false
     */
    constexpr friend bool operator<(LineRange r1, LineRange r2) noexcept
    {
        return r1.end() < r2.start();
    }

private:
    /**
     * This range's start line.
     *
     * @internal
     */
    int m_start = 0;

    /**
     * This range's end line.
     *
     * @internal
     */
    int m_end = 0;
};

/**
 * QHash function for KTextEditor::LineRange.
 * Returns the hash value for @p range.
 */
KTEXTEDITOR_EXPORT size_t qHash(KTextEditor::LineRange range, size_t seed = 0) noexcept;
}

Q_DECLARE_TYPEINFO(KTextEditor::LineRange, Q_PRIMITIVE_TYPE);

/**
 * qDebug() stream operator.  Writes this range to the debug output in a nicely formatted way.
 */
KTEXTEDITOR_EXPORT QDebug operator<<(QDebug s, KTextEditor::LineRange range);

namespace QTest
{
// forward declaration of template in qtestcase.h
template<typename T>
char *toString(const T &);

/**
 * QTestLib integration to have nice output in e.g. QCOMPARE failures.
 */
template<>
KTEXTEDITOR_EXPORT char *toString(const KTextEditor::LineRange &range);
}

#endif
