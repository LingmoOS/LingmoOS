/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_COMMAND_H
#define KATEVI_COMMAND_H

#include <QRegularExpression>
#include <QString>

namespace KateVi
{
class KeyParser;
class NormalViMode;

enum CommandFlags {
    REGEX_PATTERN = 0x1, // the pattern is a regex
    NEEDS_MOTION = 0x2, // the command needs a motion before it can be executed
    SHOULD_NOT_RESET = 0x4, // the command should not cause the current mode to be left
    IS_CHANGE = 0x8, // the command changes the buffer
    IS_NOT_LINEWISE = 0x10, // the motion is not line wise
    CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION = 0x20, // the motion is a text object that can set the
                                                   // whole Visual Mode selection to the text object
    CAN_LAND_INSIDE_FOLDING_RANGE = 0x40 // the motion can end up inside a folding range
};

class Command
{
public:
    Command(const QString &pattern, bool (NormalViMode::*pt2Func)(), unsigned int flags = 0);
    virtual ~Command();

    bool matches(const QString &pattern) const;
    bool matchesExact(const QString &pattern) const;
    bool execute(NormalViMode *mode) const;
    const QString pattern() const
    {
        return m_pattern;
    }
    bool isRegexPattern() const
    {
        return m_flags & REGEX_PATTERN;
    }
    bool needsMotion() const
    {
        return m_flags & NEEDS_MOTION;
    }
    bool shouldReset() const
    {
        return !(m_flags & SHOULD_NOT_RESET);
    }
    bool isChange() const
    {
        return m_flags & IS_CHANGE;
    }
    bool isLineWise() const
    {
        return !(m_flags & IS_NOT_LINEWISE);
    }
    bool canChangeWholeVisualModeSelection() const
    {
        return m_flags & CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION;
    }
    bool canLandInsideFoldingRange() const
    {
        return m_flags & CAN_LAND_INSIDE_FOLDING_RANGE;
    }

protected:
    // constant stuff, we create each command just once globally
    const QString m_pattern;
    const unsigned int m_flags;
    bool (NormalViMode::*m_ptr2commandMethod)();

    // we create commands only once globally
    // regex compile is costly, we do this at first match of this command
    mutable QRegularExpression m_patternRegex;
    mutable QRegularExpression m_patternAnchoredRegex;
};

}

#endif /* KATEVI_COMMAND_H */
