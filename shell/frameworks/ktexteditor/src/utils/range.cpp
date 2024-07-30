/*
    SPDX-FileCopyrightText: 2016 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2002 Christian Couder <christian@kdevelop.org>
    SPDX-FileCopyrightText: 2001, 2003 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "range.h"

#include <QByteArray>
#include <QDebug>
#include <QHash>
#include <QString>

using namespace KTextEditor;

Range Range::fromString(QStringView str) noexcept
{
    const int startIndex = str.indexOf(QLatin1Char('['));
    const int endIndex = str.indexOf(QLatin1Char(']'));
    const int closeIndex = str.indexOf(QLatin1Char(')')); // end of first cursor

    if (startIndex < 0 || endIndex < 0 || closeIndex < 0 || closeIndex < startIndex || endIndex < closeIndex || endIndex < startIndex) {
        return invalid();
    }

    return Range(Cursor::fromString(str.mid(startIndex + 1, closeIndex - startIndex)), Cursor::fromString(str.mid(closeIndex + 2, endIndex - closeIndex - 2)));
}

void Range::setRange(Range range) noexcept
{
    m_start = range.start();
    m_end = range.end();
}

void Range::setRange(Cursor start, Cursor end) noexcept
{
    if (start > end) {
        setRange(Range(end, start));
    } else {
        setRange(Range(start, end));
    }
}

bool Range::confineToRange(Range range) noexcept
{
    if (start() < range.start()) {
        if (end() > range.end()) {
            setRange(range);
        } else {
            setStart(range.start());
        }
    } else if (end() > range.end()) {
        setEnd(range.end());
    } else {
        return false;
    }

    return true;
}

bool Range::expandToRange(Range range) noexcept
{
    if (start() > range.start()) {
        if (end() < range.end()) {
            setRange(range);
        } else {
            setStart(range.start());
        }
    } else if (end() < range.end()) {
        setEnd(range.end());
    } else {
        return false;
    }

    return true;
}

void Range::setBothLines(int line) noexcept
{
    setRange(Range(line, start().column(), line, end().column()));
}

void KTextEditor::Range::setBothColumns(int column) noexcept
{
    setRange(Range(start().line(), column, end().line(), column));
}

LineRange LineRange::fromString(QStringView str) noexcept
{
    // parse format "[start, end]"
    const int startIndex = str.indexOf(QLatin1Char('['));
    const int endIndex = str.indexOf(QLatin1Char(']'));
    const int commaIndex = str.indexOf(QLatin1Char(','));

    if (startIndex < 0 || endIndex < 0 || commaIndex < 0 || commaIndex < startIndex || endIndex < commaIndex || endIndex < startIndex) {
        return invalid();
    }

    bool ok1 = false;
    bool ok2 = false;

    const int start = str.mid(startIndex + 1, commaIndex - startIndex - 1).toInt(&ok1);
    const int end = str.mid(commaIndex + 1, endIndex - commaIndex - 1).toInt(&ok2);

    if (!ok1 || !ok2) {
        return invalid();
    }

    return {start, end};
}

QDebug operator<<(QDebug s, KTextEditor::Range range)
{
    s << "[" << range.start() << " -> " << range.end() << "]";
    return s;
}

QDebug operator<<(QDebug s, KTextEditor::LineRange range)
{
    s << "[" << range.start() << " -> " << range.end() << "]";
    return s;
}

size_t KTextEditor::qHash(KTextEditor::LineRange range, size_t seed) noexcept
{
    return qHashMulti(seed, range.start(), range.end());
}

size_t KTextEditor::qHash(KTextEditor::Range range, size_t seed) noexcept
{
    return qHashMulti(seed, range.start(), range.end());
}

QString Range::toString() const
{
    return QStringLiteral("[%1, %2]").arg(m_start.toString()).arg(m_end.toString());
}

QString LineRange::toString() const
{
    return QStringLiteral("[%1, %2]").arg(m_start).arg(m_end);
}

namespace QTest
{
// Cursor: template specialization for QTest::toString()
template<>
char *toString(const KTextEditor::Cursor &cursor)
{
    QByteArray ba = "Cursor[" + QByteArray::number(cursor.line()) + ", " + QByteArray::number(cursor.column()) + ']';
    return qstrdup(ba.data());
}

// Range: template specialization for QTest::toString()
template<>
char *toString(const KTextEditor::Range &range)
{
    QByteArray ba = "Range[";
    ba += QByteArray::number(range.start().line()) + ", " + QByteArray::number(range.start().column()) + " - ";
    ba += QByteArray::number(range.end().line()) + ", " + QByteArray::number(range.end().column());
    ba += ']';
    return qstrdup(ba.data());
}

// LineRange: template specialization for QTest::toString()
template<>
char *toString(const KTextEditor::LineRange &range)
{
    QByteArray ba = "LineRange[";
    ba += QByteArray::number(range.start()) + ", " + QByteArray::number(range.end());
    ba += ']';
    return qstrdup(ba.data());
}
}
