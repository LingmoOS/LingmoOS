/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bugsearch.h"

namespace Bugzilla
{
Query BugSearch::toQuery() const
{
    Query query;
    QSet<QString> seen;

    for (const QString &product : products) {
        query.addQueryItem(QStringLiteral("product"), product);
    }
    seen << QStringLiteral("products");

    if (!order.isEmpty()) {
        query.addQueryItem(QStringLiteral("order"), order.join(QLatin1Char(',')));
    }
    seen << QStringLiteral("order");

    expandQuery(query, seen);

    return query;
}

} // namespace Bugzilla

#include "moc_bugsearch.cpp"
