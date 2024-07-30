/*
    SPDX-FileCopyrightText: 2012-2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LINGMOENTRY_H
#define LINGMOENTRY_H

#include "Entry.h"

class LingmoEntry : public Entry
{
public:
    LingmoEntry();
    static QString lingmoVersion();
};

#endif // LINGMOENTRY_H
