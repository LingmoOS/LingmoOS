/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <ktexteditor/range.h>
#include <vimode/range.h>

using namespace KateVi;

Range::Range()
    : Range(-1, -1, -1, -1, InclusiveMotion)
{
}

Range::Range(int slin, int scol, int elin, int ecol, MotionType inc)
    : startLine(slin)
    , startColumn(scol)
    , endLine(elin)
    , endColumn(ecol)
    , motionType(inc)
    , valid(true)
    , jump(false)
{
}

Range::Range(int elin, int ecol, MotionType inc)
    : Range(-1, -1, elin, ecol, inc)
{
}

Range::Range(const KTextEditor::Cursor c, MotionType mt)
    : Range(-1, -1, c.line(), c.column(), mt)
{
}

Range::Range(const KTextEditor::Cursor c1, const KTextEditor::Cursor c2, MotionType mt)
    : Range(c1.line(), c1.column(), c2.line(), c2.column(), mt)
{
}

void Range::normalize()
{
    int sl = startLine;
    int el = endLine;
    int sc = startColumn;
    int ec = endColumn;

    if (sl < el) {
        startLine = sl;
        startColumn = sc;
        endLine = el;
        endColumn = ec;
    } else {
        startLine = el;
        endLine = sl;
        if (sl != el) {
            startColumn = ec;
            endColumn = sc;
        } else {
            startColumn = qMin(sc, ec);
            endColumn = qMax(sc, ec);
        }
    }
}

KTextEditor::Range Range::toEditorRange() const
{
    return KTextEditor::Range(startLine, startColumn, endLine, endColumn);
}

Range Range::invalid()
{
    Range r;
    r.valid = false;
    return r;
}

QDebug operator<<(QDebug s, const Range &range)
{
    s << "["
      << " (" << range.startLine << ", " << range.startColumn << ")"
      << " -> "
      << " (" << range.endLine << ", " << range.endColumn << ")"
      << "]"
      << " (" << (range.motionType == InclusiveMotion ? "Inclusive" : "Exclusive") << ") (jump: " << (range.jump ? "true" : "false") << ")";
    return s;
}
