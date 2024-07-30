/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "topic.h"

using namespace Attica;

class Q_DECL_HIDDEN Topic::Private : public QSharedData
{
public:
    QString m_id;
    QString m_forumId;
    QString m_user;
    QDateTime m_date;
    QString m_subject;
    QString m_content;
    int m_comments;

    Private()
        : m_comments(0)
    {
    }
};

Topic::Topic()
    : d(new Private)
{
}

Topic::Topic(const Topic &other)
    : d(other.d)
{
}

Topic &Topic::operator=(const Topic &other)
{
    d = other.d;
    return *this;
}

Topic::~Topic()
{
}

void Topic::setId(const QString &id)
{
    d->m_id = id;
}

QString Topic::id() const
{
    return d->m_id;
}

void Topic::setForumId(const QString &forumId)
{
    d->m_forumId = forumId;
}

QString Topic::forumId() const
{
    return d->m_forumId;
}

void Topic::setUser(const QString &user)
{
    d->m_user = user;
}

QString Topic::user() const
{
    return d->m_user;
}

void Topic::setDate(const QDateTime &date)
{
    d->m_date = date;
}

QDateTime Topic::date() const
{
    return d->m_date;
}

void Topic::setSubject(const QString &subject)
{
    d->m_subject = subject;
}

QString Topic::subject() const
{
    return d->m_subject;
}

void Topic::setContent(const QString &content)
{
    d->m_content = content;
}

QString Topic::content() const
{
    return d->m_content;
}

void Topic::setComments(const int comments)
{
    d->m_comments = comments;
}

int Topic::comments() const
{
    return d->m_comments;
}

bool Topic::isValid() const
{
    return !(d->m_id.isEmpty());
}
