/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "comment.h"

#include <QMap>

using namespace Attica;

QString Comment::commentTypeToString(const Comment::Type type)
{
    switch (type) {
    case ContentComment:
        return QStringLiteral("1");
    case ForumComment:
        return QStringLiteral("4");
    case KnowledgeBaseComment:
        return QStringLiteral("7");
    case EventComment:
        return QStringLiteral("8");
    }

    Q_ASSERT(false);
    return QString();
}

class Q_DECL_HIDDEN Comment::Private : public QSharedData
{
public:
    QString m_id;
    QString m_subject;
    QString m_text;
    int m_childCount;
    QString m_user;
    QDateTime m_date;
    int m_score;
    QList<Comment> m_children;

    Private()
        : m_childCount(0)
        , m_score(0)
    {
    }
};

Comment::Comment()
    : d(new Private)
{
}

Comment::Comment(const Comment &other)
    : d(other.d)
{
}

Comment &Comment::operator=(const Attica::Comment &other)
{
    d = other.d;
    return *this;
}

Comment::~Comment()
{
}

void Comment::setId(const QString &id)
{
    d->m_id = id;
}

QString Comment::id() const
{
    return d->m_id;
}

void Comment::setSubject(const QString &subject)
{
    d->m_subject = subject;
}

QString Comment::subject() const
{
    return d->m_subject;
}

void Comment::setText(const QString &text)
{
    d->m_text = text;
}

QString Comment::text() const
{
    return d->m_text;
}

void Comment::setChildCount(const int childCount)
{
    d->m_childCount = childCount;
}

int Comment::childCount() const
{
    return d->m_childCount;
}

void Comment::setUser(const QString &user)
{
    d->m_user = user;
}

QString Comment::user() const
{
    return d->m_user;
}

void Comment::setDate(const QDateTime &date)
{
    d->m_date = date;
}

QDateTime Comment::date() const
{
    return d->m_date;
}

void Comment::setScore(const int score)
{
    d->m_score = score;
}

int Comment::score() const
{
    return d->m_score;
}

void Comment::setChildren(QList<Comment> children)
{
    d->m_children = std::move(children); // TODO KF6 Make QList const & and remove the std::move
}

QList<Comment> Comment::children() const
{
    return d->m_children;
}

bool Comment::isValid() const
{
    return !(d->m_id.isEmpty());
}
