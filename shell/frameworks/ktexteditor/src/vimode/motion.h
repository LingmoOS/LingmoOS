/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_MOTION_H
#define KATEVI_MOTION_H

#include <vimode/command.h>
#include <vimode/range.h>

namespace KateVi
{
class NormalViMode;
/**
 * combined class for motions and text objects. execute() returns a KateViRange.
 * For motions the returned range is only a position (start pos is (-1, -1) to
 * indicate this) for text objects a range (startx, starty), (endx, endy) is
 * returned
 */
class Motion : public Command
{
public:
    Motion(const QString &pattern, Range (NormalViMode::*commandMethod)(), unsigned int flags = 0);
    Range execute(NormalViMode *mode) const;

protected:
    Range (NormalViMode::*m_ptr2commandMethod)();
};

}

#endif /* KATEVI_MOTION_H */
