/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "activity.h"

#include <QDateTime>

using namespace Attica;

class Q_DECL_HIDDEN Activity::Private : public QSharedData
{
public:
    QString m_id;
    Person m_associatedPerson;
    QDateTime m_timestamp;
    QString m_message;
    QUrl m_link;
};

Activity::Activity()
    : d(new Private)
{
}

Activity::Activity(const Attica::Activity &other)
    : d(other.d)
{
}

Activity &Activity::operator=(const Attica::Activity &other)
{
    d = other.d;
    return *this;
}

Activity::~Activity()
{
}

void Activity::setId(const QString &id)
{
    d->m_id = id;
}

QString Activity::id() const
{
    return d->m_id;
}

void Activity::setAssociatedPerson(const Person &associatedPerson)
{
    d->m_associatedPerson = associatedPerson;
}

Person Activity::associatedPerson() const
{
    return d->m_associatedPerson;
}

void Activity::setTimestamp(const QDateTime &date)
{
    d->m_timestamp = date;
}

QDateTime Activity::timestamp() const
{
    return d->m_timestamp;
}

void Activity::setMessage(const QString &c)
{
    d->m_message = c;
}

QString Activity::message() const
{
    return d->m_message;
}

void Activity::setLink(const QUrl &v)
{
    d->m_link = v;
}

QUrl Activity::link() const
{
    return d->m_link;
}

bool Activity::isValid() const
{
    return !(d->m_id.isEmpty());
}
