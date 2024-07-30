/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bugzilla.h"

namespace Bugzilla
{
QString version(KJob *kjob)
{
    const APIJob *job = qobject_cast<APIJob *>(kjob);
    const QString version = job->object().value(QLatin1String("version")).toString();
    return version;
}

APIJob *version(const Connection &connection)
{
    return connection.get(QStringLiteral("/version"));
}

LoginDetails login(KJob *kjob)
{
    const APIJob *job = qobject_cast<APIJob *>(kjob);
    const auto obj = job->object();
    const QString token = obj.value(QLatin1String("token")).toString();
    const int id = obj.value(QLatin1String("id")).toInt(-1);
    return LoginDetails{id, token};
}

APIJob *login(const QString &username, const QString &password, const Connection &connection)
{
    Query query;
    query.addQueryItem(QStringLiteral("login"), username);
    query.addQueryItem(QStringLiteral("password"), password);
    query.addQueryItem(QStringLiteral("restrict_login"), QStringLiteral("true"));
    return connection.get(QStringLiteral("/login"), query);
}

} // namespace Bugzilla
