/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    Based on code of the SmartCursor/Range by:
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "document.h"
#include "documentcursor.h"
#include "movingcursor.h"
#include "movingrange.h"
#include "movingrangefeedback.h"

using namespace KTextEditor;

// BEGIN MovingRange
MovingRange::MovingRange() = default;

MovingRange::~MovingRange() = default;

void MovingRange::setRange(Cursor start, Cursor end)
{
    // just use other function, KTextEditor::Range will handle some normalization
    setRange(Range(start, end));
}

bool MovingRange::overlaps(const Range &range) const
{
    if (range.start() <= start()) {
        return range.end() > start();
    }

    else if (range.end() >= end()) {
        return range.start() < end();
    }

    else {
        return contains(range);
    }
}

QDebug KTextEditor::operator<<(QDebug s, const MovingRange *range)
{
    if (range) {
        s << "[" << range->start() << " -> " << range->end() << "]";
    } else {
        s << "(null range)";
    }
    return s.space();
}

/**
 * qDebug() stream operator. Writes this range to the debug output in a nicely formatted way.
 * @param s debug stream
 * @param range range to print
 * @return debug stream
 */
QDebug KTextEditor::operator<<(QDebug s, const MovingRange &range)
{
    return s << &range;
}
// END MovingRange

// BEGIN MovingCursor

MovingCursor::MovingCursor()
{
}

MovingCursor::~MovingCursor()
{
}

void MovingCursor::setPosition(int line, int column)
{
    // just use setPosition
    setPosition(Cursor(line, column));
}

void MovingCursor::setLine(int line)
{
    // just use setPosition
    setPosition(line, column());
}

void MovingCursor::setColumn(int column)
{
    // just use setPosition
    setPosition(line(), column);
}

bool MovingCursor::atStartOfLine() const
{
    return isValidTextPosition() && column() == 0;
}

bool MovingCursor::atEndOfLine() const
{
    return isValidTextPosition() && column() == document()->lineLength(line());
}

bool MovingCursor::atEndOfDocument() const
{
    return *this == document()->documentEnd();
}

bool MovingCursor::atStartOfDocument() const
{
    return line() == 0 && column() == 0;
}

bool MovingCursor::gotoNextLine()
{
    // only touch valid cursors
    const bool ok = isValid() && (line() + 1 < document()->lines());

    if (ok) {
        setPosition(Cursor(line() + 1, 0));
    }

    return ok;
}

bool MovingCursor::gotoPreviousLine()
{
    // only touch valid cursors
    bool ok = (line() > 0) && (column() >= 0);

    if (ok) {
        setPosition(Cursor(line() - 1, 0));
    }

    return ok;
}

bool MovingCursor::move(int chars, WrapBehavior wrapBehavior)
{
    DocumentCursor dc(document(), toCursor());

    const bool success = dc.move(chars, static_cast<DocumentCursor::WrapBehavior>(wrapBehavior));

    if (success && dc.toCursor() != toCursor()) {
        setPosition(dc.toCursor());
    }

    return success;
}

bool MovingCursor::isValidTextPosition() const
{
    return document()->isValidTextPosition(toCursor());
}

QDebug KTextEditor::operator<<(QDebug s, const MovingCursor *cursor)
{
    if (cursor) {
        s.nospace() << "(" << cursor->line() << ", " << cursor->column() << ")";
    } else {
        s.nospace() << "(null cursor)";
    }
    return s.space();
}

QDebug KTextEditor::operator<<(QDebug s, const MovingCursor &cursor)
{
    return s << &cursor;
}
// END MovingCursor

// BEGIN MovingRangeFeedback
MovingRangeFeedback::MovingRangeFeedback() = default;

MovingRangeFeedback::~MovingRangeFeedback() = default;

void MovingRangeFeedback::rangeEmpty(MovingRange *)
{
}

void MovingRangeFeedback::rangeInvalid(MovingRange *)
{
}

void MovingRangeFeedback::mouseEnteredRange(MovingRange *, View *)
{
}

void MovingRangeFeedback::mouseExitedRange(MovingRange *, View *)
{
}

void MovingRangeFeedback::caretEnteredRange(MovingRange *, View *)
{
}

void MovingRangeFeedback::caretExitedRange(MovingRange *, View *)
{
}
// END MovingRangeFeedback
