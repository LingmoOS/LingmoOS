/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "knowledgebaseentry.h"

using namespace Attica;

class Q_DECL_HIDDEN KnowledgeBaseEntry::Private : public QSharedData
{
public:
    QString m_id;
    int m_contentId;
    QString m_user;
    QString m_status;
    QDateTime m_changed;
    QString m_name;
    QString m_description;
    QString m_answer;
    int m_comments;
    QUrl m_detailPage;

    QMap<QString, QString> m_extendedAttributes;

    Private()
        : m_contentId(0)
        , m_comments(0)
    {
    }
};

KnowledgeBaseEntry::KnowledgeBaseEntry()
    : d(new Private)
{
}

KnowledgeBaseEntry::KnowledgeBaseEntry(const KnowledgeBaseEntry &other)
    : d(other.d)
{
}

KnowledgeBaseEntry &KnowledgeBaseEntry::operator=(const Attica::KnowledgeBaseEntry &other)
{
    d = other.d;
    return *this;
}

KnowledgeBaseEntry::~KnowledgeBaseEntry()
{
}

void KnowledgeBaseEntry::setId(QString id)
{
    d->m_id = std::move(id); // TODO KF6 Make QString const & and remove the std::move
}

QString KnowledgeBaseEntry::id() const
{
    return d->m_id;
}

void KnowledgeBaseEntry::setContentId(int id)
{
    d->m_contentId = id;
}

int KnowledgeBaseEntry::contentId() const
{
    return d->m_contentId;
}

void KnowledgeBaseEntry::setUser(const QString &user)
{
    d->m_user = user;
}

QString KnowledgeBaseEntry::user() const
{
    return d->m_user;
}

void KnowledgeBaseEntry::setStatus(const QString &status)
{
    d->m_status = status;
}

QString KnowledgeBaseEntry::status() const
{
    return d->m_status;
}

void KnowledgeBaseEntry::setChanged(const QDateTime &changed)
{
    d->m_changed = changed;
}

QDateTime KnowledgeBaseEntry::changed() const
{
    return d->m_changed;
}

void KnowledgeBaseEntry::setName(const QString &name)
{
    d->m_name = name;
}

QString KnowledgeBaseEntry::name() const
{
    return d->m_name;
}

void KnowledgeBaseEntry::setDescription(const QString &description)
{
    d->m_description = description;
}

QString KnowledgeBaseEntry::description() const
{
    return d->m_description;
}

void KnowledgeBaseEntry::setAnswer(const QString &answer)
{
    d->m_answer = answer;
}

QString KnowledgeBaseEntry::answer() const
{
    return d->m_answer;
}

void KnowledgeBaseEntry::setComments(int comments)
{
    d->m_comments = comments;
}

int KnowledgeBaseEntry::comments() const
{
    return d->m_comments;
}

void KnowledgeBaseEntry::setDetailPage(const QUrl &detailPage)
{
    d->m_detailPage = detailPage;
}

QUrl KnowledgeBaseEntry::detailPage() const
{
    return d->m_detailPage;
}

void KnowledgeBaseEntry::addExtendedAttribute(const QString &key, const QString &value)
{
    d->m_extendedAttributes.insert(key, value);
}

QString KnowledgeBaseEntry::extendedAttribute(const QString &key) const
{
    return d->m_extendedAttributes.value(key);
}

QMap<QString, QString> KnowledgeBaseEntry::extendedAttributes() const
{
    return d->m_extendedAttributes;
}

bool KnowledgeBaseEntry::isValid() const
{
    return !(d->m_id.isEmpty());
}
