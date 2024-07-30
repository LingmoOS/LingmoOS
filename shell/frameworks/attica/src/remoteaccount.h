/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_REMOTEACCOUNT_H
#define ATTICA_REMOTEACCOUNT_H

#include <QDate>
#include <QList>
#include <QMap>
#include <QSharedDataPointer>
#include <QStringList>
#include <QUrl>

#include "attica_export.h"

namespace Attica
{

/**
 * @class RemoteAccount remoteaccount.h <Attica/RemoteAccount>
 *
 * Represents a remote account.
 */
class ATTICA_EXPORT RemoteAccount
{
public:
    typedef QList<RemoteAccount> List;
    class Parser;

    RemoteAccount();
    RemoteAccount(const RemoteAccount &other);
    RemoteAccount &operator=(const RemoteAccount &other);
    ~RemoteAccount();

    void setId(const QString &);
    QString id() const;

    void setType(const QString &);
    QString type() const;

    void setRemoteServiceId(const QString &);
    QString remoteServiceId() const;

    void setData(const QString &);
    QString data() const;

    void setLogin(const QString &);
    QString login() const;

    void setPassword(const QString &);
    QString password() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
