/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include "query.h"

namespace Bugzilla
{
bool Query::hasQueryItem(const QString &key)
{
    return contains(key);
}

void Query::addQueryItem(const QString &key, const QString &value)
{
    insert(key, value);
}

// Don't use this to do anything other than streaming to qDebug.
// The output is not encoded and thus not necessarily a valid URL query.
QString Query::toString() const
{
    QString output;
    for (auto it = cbegin(); it != cend(); ++it) {
        if (!output.isEmpty()) {
            output += QStringLiteral("&");
        }
        output += it.key() + QStringLiteral("=") + it.value();
    }
    return output;
}

} // namespace Bugzilla
