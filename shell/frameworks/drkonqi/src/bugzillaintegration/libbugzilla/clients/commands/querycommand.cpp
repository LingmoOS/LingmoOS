/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "querycommand.h"
#include <QMetaProperty>
#include <QVariant>

namespace Bugzilla
{
Query QueryCommand::toQuery() const
{
    Query query;
    return expandQuery(query, QSet<QString>());
}

Query QueryCommand::expandQuery(Query &query, const QSet<QString> &seen) const
{
    const auto propertyCount = metaObject()->propertyCount();
    for (int i = 0; i < propertyCount; ++i) {
        const auto property = metaObject()->property(i);
        const auto name = QString::fromLatin1(property.name());
        const auto value = property.read(this);

        if (query.hasQueryItem(name) || seen.contains(name) || name == QLatin1String("objectName")) {
            // The element was manually set or builtin property.
            continue;
        }

        if (value.toLongLong() < 0) {
            // Invalid value => member was not set!
            // This does generally also work for all integers, ulonglong of
            // course being the only one that can cause trouble.
            continue;
        }

        // Lists must be serialized manually. They could have a number of representations.
        Q_ASSERT_X(value.type() != QVariant::StringList, Q_FUNC_INFO, qPrintable(QStringLiteral("Trying to auto serialize string list %1").arg(name)));

        // Either can't serialize or not set.
        if (value.toString().isEmpty()) {
            continue;
        }

        query.addQueryItem(name, property.read(this).toString());
    }

    return query;
}

} // namespace Bugzilla
