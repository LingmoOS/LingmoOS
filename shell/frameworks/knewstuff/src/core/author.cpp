/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "author.h"

#include <QHash>

namespace KNSCore
{
class AuthorPrivate : public QSharedData
{
public:
    QString id;
    QString profilepage;
    QUrl avatarUrl;
    QString description;

    QString name;
    QString email;
    QString jabber;
    QString homepage;
};
}

using namespace KNSCore;

Author::Author()
    : d(new AuthorPrivate())
{
}

KNSCore::Author::Author(const KNSCore::Author &other)
    : d(other.d)
{
}

Author &Author::operator=(const Author &rhs)
{
    if (&rhs != this) {
        d = rhs.d;
    }

    return *this;
}

Author::~Author() = default;

void KNSCore::Author::setId(const QString &id)
{
    d->id = id;
}

QString KNSCore::Author::id() const
{
    return d->id;
}

void Author::setName(const QString &name)
{
    d->name = name;
}

QString Author::name() const
{
    return d->name;
}

void Author::setEmail(const QString &email)
{
    d->email = email;
}

QString Author::email() const
{
    return d->email;
}

void Author::setJabber(const QString &jabber)
{
    d->jabber = jabber;
}

QString Author::jabber() const
{
    return d->jabber;
}

void Author::setHomepage(const QString &homepage)
{
    d->homepage = homepage;
}

QString Author::homepage() const
{
    return d->homepage;
}

void Author::setProfilepage(const QString &profilepage)
{
    d->profilepage = profilepage;
}

QString Author::profilepage() const
{
    return d->profilepage;
}

void Author::setAvatarUrl(const QUrl &avatarUrl)
{
    d->avatarUrl = avatarUrl;
}

QUrl Author::avatarUrl() const
{
    return d->avatarUrl;
}

void Author::setDescription(const QString &description)
{
    d->description = description;
}

QString Author::description() const
{
    return d->description;
}
