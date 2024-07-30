/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Lars Knoll <knoll@kde.org>
    SPDX-FileCopyrightText: 2001, 2003, 2004, 2005, 2006 Nicolas GOUTTE <goutte@kde.org>
    SPDX-FileCopyrightText: 2007 Nick Shaforostoff <shafff@ukr.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCHARSETS_P_H
#define KCHARSETS_P_H

#include <QStringList>

class KCharsetsPrivate
{
public:
    // Cache list so QStrings can be implicitly shared
    QList<QStringList> encodingsByScript;
};

#endif
