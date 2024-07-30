/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_RANGE_H
#define KATEVI_RANGE_H

#include <QDebug>
#include <vimode/definitions.h>

namespace KTextEditor
{
class Cursor;
class Range;
}

namespace KateVi
{
enum MotionType { ExclusiveMotion = 0, InclusiveMotion };

class Range
{
public:
    Range();

    /**
     * For motions which only return a position, in contrast to
     * "text objects" which returns a full blown range.
     */
    explicit Range(int elin, int ecol, MotionType inc);

    explicit Range(int slin, int scol, int elin, int ecol, MotionType mt);
    explicit Range(const KTextEditor::Cursor c, MotionType mt);
    explicit Range(const KTextEditor::Cursor c1, const KTextEditor::Cursor c2, MotionType mt);

    /**
     * Modifies this range so the start attributes are lesser than
     * the end attributes.
     */
    void normalize();

    /**
     * @returns an equivalent KTextEditor::Range for this Range.
     */
    KTextEditor::Range toEditorRange() const;

    /**
     * Writes this KateViRange to the debug output in a nicely formatted way.
     */
    friend QDebug operator<<(QDebug s, const Range &range);

    /**
     * @returns an invalid KateViRange allocated on stack.
     */
    static Range invalid();

public:
    int startLine, startColumn;
    int endLine, endColumn;
    MotionType motionType;
    bool valid, jump;
};

}

#endif /* KATEVI_RANGE_H */
