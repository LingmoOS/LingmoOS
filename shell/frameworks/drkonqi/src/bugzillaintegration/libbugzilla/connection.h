/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QUrl>

#include "apijob.h"
#include "exceptions.h"
#include "query.h"

namespace Bugzilla
{
class APIJob;

/**
 * Base Connection. Has CRUD-y methods that need implementing in a specific
 * connection variant.
 */
class Connection : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual void setToken(const QString &authToken) = 0;

    virtual APIJob *get(const QString &path, const Query &query = Query()) const = 0;
    virtual APIJob *post(const QString &path, const QByteArray &data, const Query &query = Query()) const = 0;
    virtual APIJob *put(const QString &path, const QByteArray &data, const Query &query = Query()) const = 0;
};

/**
 * HTTP Connection.
 */
class HTTPConnection : public Connection
{
    Q_OBJECT
    friend class ConnectionTest;

public:
    explicit HTTPConnection(const QUrl &root = QUrl(QStringLiteral("http://bugstest.kde.org/rest")), QObject *parent = nullptr);
    ~HTTPConnection() override;

    void setToken(const QString &authToken) override;

    APIJob *get(const QString &path, const Query &query = Query()) const override;
    APIJob *post(const QString &path, const QByteArray &data, const Query &query = Query()) const override;
    APIJob *put(const QString &path, const QByteArray &data, const Query &query = Query()) const override;

    QUrl root() const;

private:
    QUrl url(const QString &appendix, Query query) const;

    QUrl m_root;
    QString m_token;

    Q_DISABLE_COPY_MOVE(HTTPConnection)
};

/**
 * @return the "default" global connection instance. This is the instance used
 *   by all clients unless another one is manually set on the client.
 */
Connection &connection();

/**
 * Changes the "default" global connection. This generally shouldn't be used
 * outside tests, where it is used to inject connection doubles.
 */
void setConnection(Connection *newConnection);

} // namespace Bugzilla

#endif // CONNECTION_H
