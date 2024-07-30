/*
    SPDX-FileCopyrightText: 2019-2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "connection.h"

#include <QNetworkAccessManager>
#include <QUrlQuery>

#include "bugzilla_debug.h"

namespace Bugzilla
{
// Static container for global default connection.
// We need a container here because the connection may be anything derived from
// Connection and its effective type may change (e.g. in autotests).
class GlobalConnection
{
public:
    std::unique_ptr<Connection> m_connection = std::make_unique<HTTPConnection>();
};

Q_GLOBAL_STATIC(GlobalConnection, s_connection)

Connection &connection()
{
    return *(s_connection->m_connection);
}

void setConnection(Connection *newConnection)
{
    s_connection->m_connection.reset(newConnection);
}

HTTPConnection::HTTPConnection(const QUrl &root, QObject *parent)
    : Connection(parent)
    , m_root(root)
{
}

HTTPConnection::~HTTPConnection() = default;

void HTTPConnection::setToken(const QString &authToken)
{
    m_token = authToken;
}

APIJob *HTTPConnection::get(const QString &path, const Query &query) const
{
    qCDebug(BUGZILLA_LOG) << path << query.toString();
    auto job = new NetworkAPIJob(url(path, query), [](QNetworkAccessManager &manager, QNetworkRequest &request) -> QNetworkReply * {
        return manager.get(request);
    });
    return job;
}

APIJob *HTTPConnection::post(const QString &path, const QByteArray &data, const Query &query) const
{
    qCDebug(BUGZILLA_LOG) << path << query.toString();
    auto job = new NetworkAPIJob(url(path, query), [data](QNetworkAccessManager &manager, QNetworkRequest &request) -> QNetworkReply * {
        return manager.post(request, data);
    });
    return job;
}

APIJob *HTTPConnection::put(const QString &path, const QByteArray &data, const Query &query) const
{
    qCDebug(BUGZILLA_LOG) << path << query.toString();
    auto job = new NetworkAPIJob(url(path, query), [data](QNetworkAccessManager &manager, QNetworkRequest &request) -> QNetworkReply * {
        return manager.put(request, data);
    });
    return job;
}

QUrl HTTPConnection::root() const
{
    return m_root;
}

QUrl HTTPConnection::url(const QString &appendix, Query query) const
{
    QUrl url(m_root);
    url.setPath(m_root.path() + appendix);

    if (!m_token.isEmpty()) {
        query.addQueryItem(QStringLiteral("token"), m_token);
    }

    // https://bugs.kde.org/show_bug.cgi?id=413920
    // Force encoding. Query by default wouldn't encode '+' and bugzilla doesn't like that...
    // For any query argument. Tested with username, password, and products (for bug search)
    // on bugzilla 5.0.6. As a result let's force full encoding on every argument.
    QUrlQuery escapedQuery;
    for (auto it = query.cbegin(); it != query.cend(); ++it) {
        escapedQuery.addQueryItem(it.key(), QString::fromUtf8(it.value().toUtf8().toPercentEncoding()));
    }

    url.setQuery(escapedQuery);
    return url;
}

} // namespace Bugzilla

#include "moc_connection.cpp"
