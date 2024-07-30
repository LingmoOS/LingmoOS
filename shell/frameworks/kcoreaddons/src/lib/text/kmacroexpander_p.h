/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2002-2003, 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KMACROEXPANDER_P_H
#define KMACROEXPANDER_P_H

#include "kmacroexpander.h"

class KMacroExpanderBasePrivate
{
public:
    KMacroExpanderBasePrivate(QChar c)
        : escapechar(c)
    {
    }
    QChar escapechar;
};

#endif
