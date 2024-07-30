/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "message.h"

using namespace Attica;

class Q_DECL_HIDDEN Message::Private : public QSharedData
{
public:
    QString m_id;
    QString m_from;
    QString m_to;
    QDateTime m_sent;
    Status m_status;
    QString m_subject;
    QString m_body;

    Private()
        : m_status(Unread)
    {
    }
};

Message::Message()
    : d(new Private)
{
}

Message::Message(const Message &other)
    : d(other.d)
{
}

Message &Message::operator=(const Attica::Message &other)
{
    d = other.d;
    return *this;
}

Message::~Message()
{
}

void Message::setId(const QString &u)
{
    d->m_id = u;
}

QString Message::id() const
{
    return d->m_id;
}

void Message::setFrom(const QString &n)
{
    d->m_from = n;
}

QString Message::from() const
{
    return d->m_from;
}

void Message::setTo(const QString &n)
{
    d->m_to = n;
}

QString Message::to() const
{
    return d->m_to;
}

void Message::setSent(const QDateTime &date)
{
    d->m_sent = date;
}

QDateTime Message::sent() const
{
    return d->m_sent;
}

void Message::setStatus(Message::Status s)
{
    d->m_status = s;
}

Message::Status Message::status() const
{
    return d->m_status;
}

void Message::setSubject(const QString &subject)
{
    d->m_subject = subject;
}

QString Message::subject() const
{
    return d->m_subject;
}

void Message::setBody(const QString &body)
{
    d->m_body = body;
}

QString Message::body() const
{
    return d->m_body;
}

bool Message::isValid() const
{
    return !(d->m_id.isEmpty());
}
