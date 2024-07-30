/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <vimode/motion.h>

using namespace KateVi;

Motion::Motion(const QString &pattern, Range (NormalViMode::*commandMethod)(), unsigned int flags)
    : Command(pattern, nullptr, flags)
{
    m_ptr2commandMethod = commandMethod;
}

Range Motion::execute(NormalViMode *mode) const
{
    return (mode->*m_ptr2commandMethod)();
}
