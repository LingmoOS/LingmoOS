/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "remoteaccount.h"

using namespace Attica;

class Q_DECL_HIDDEN RemoteAccount::Private : public QSharedData
{
public:
    QString id;
    QString type;
    QString remoteServiceId;
    QString data;
    QString login;
    QString password;

    Private()
    {
    }
};

RemoteAccount::RemoteAccount()
    : d(new Private)
{
}

RemoteAccount::RemoteAccount(const RemoteAccount &other)
    : d(other.d)
{
}

RemoteAccount &RemoteAccount::operator=(const Attica::RemoteAccount &other)
{
    d = other.d;
    return *this;
}

RemoteAccount::~RemoteAccount()
{
}

void RemoteAccount::setId(const QString &u)
{
    d->id = u;
}

QString RemoteAccount::id() const
{
    return d->id;
}

void RemoteAccount::setType(const QString &arg)
{
    d->type = arg;
}

QString RemoteAccount::type() const
{
    return d->type;
}

void RemoteAccount::setRemoteServiceId(const QString &arg)
{
    d->remoteServiceId = arg;
}

QString RemoteAccount::remoteServiceId() const
{
    return d->remoteServiceId;
}

void RemoteAccount::setData(const QString &arg)
{
    d->data = arg;
}

QString RemoteAccount::data() const
{
    return d->data;
}

void RemoteAccount::setLogin(const QString &arg)
{
    d->login = arg;
}

QString RemoteAccount::login() const
{
    return d->login;
}

void RemoteAccount::setPassword(const QString &arg)
{
    d->password = arg;
}

QString RemoteAccount::password() const
{
    return d->password;
}

bool RemoteAccount::isValid() const
{
    return !(d->id.isEmpty());
}
