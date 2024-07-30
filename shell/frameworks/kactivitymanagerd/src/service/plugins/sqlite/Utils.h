/*
 *   SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <KCrash>
#include <QSqlQuery>
#include <common/database/schema/ResourcesDatabaseSchema.h>
#include <memory>

#include "DebugResources.h"

namespace Utils
{
static unsigned int errorCount = 0;

inline bool prepare(Common::Database &database, QSqlQuery &query, const QString &queryString)
{
    Q_UNUSED(database);

    return query.prepare(queryString);
}

inline bool prepare(Common::Database &database, std::unique_ptr<QSqlQuery> &query, const QString &queryString)
{
    if (query) {
        return true;
    }

    query.reset(new QSqlQuery(database.createQuery()));

    return prepare(database, *query, queryString);
}

enum ErrorHandling {
    IgnoreError,
    FailOnError,
};

inline bool exec(Common::Database &database, ErrorHandling eh, QSqlQuery &query)
{
    bool success = query.exec();

    if (eh == FailOnError) {
        if ((!success) && (errorCount++ < 2)) {
            qCWarning(KAMD_LOG_RESOURCES) << query.lastQuery();
            qCWarning(KAMD_LOG_RESOURCES) << query.lastError();
            KCrash::setErrorMessage(query.lastError().text());
        }
        Q_ASSERT_X(success, "Uils::exec", qPrintable(QStringLiteral("Query failed:") + query.lastError().text()));

        if (!success) {
            database.reportError(query.lastError());
        }
    }

    return success;
}

template<typename T1, typename T2, typename... Ts>
inline bool exec(Common::Database &database, ErrorHandling eh, QSqlQuery &query, const T1 &variable, const T2 &value, Ts... ts)
{
    query.bindValue(variable, value);

    return exec(database, eh, query, ts...);
}

} // namespace Utils
