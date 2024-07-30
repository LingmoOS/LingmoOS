/*
    SPDX-FileCopyrightText: 2012-2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "FancyString.h"

namespace FancyString
{
QString fromUgly(const QString &string)
{
    QString ugly(string);
    ugly.replace(QStringLiteral("(TM)"), QChar(8482));
    ugly.replace(QStringLiteral("(R)"), QChar(174));
    return ugly.simplified();
}

} // namespace FancyString
