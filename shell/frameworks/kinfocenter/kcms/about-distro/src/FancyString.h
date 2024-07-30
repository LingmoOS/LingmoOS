/*
    SPDX-FileCopyrightText: 2012-2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FANCYSTRING_H
#define FANCYSTRING_H

#include <QString>

namespace FancyString
{
// Turns ugly '(R)' '(TM)' ascii into unicode.
QString fromUgly(const QString &string);
}

#endif // FANCYSTRING_H
