/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "folder.h"

using namespace Attica;

class Q_DECL_HIDDEN Folder::Private : public QSharedData
{
public:
    QString m_id;
    QString m_name;
    int m_messageCount;
    QString m_type;

    Private()
        : m_messageCount(0)
    {
    }
};

Folder::Folder()
    : d(new Private)
{
}

Folder::Folder(const Folder &other)
    : d(other.d)
{
}

Folder &Folder::operator=(const Folder &other)
{
    d = other.d;
    return *this;
}

Folder::~Folder()
{
}

void Folder::setId(const QString &u)
{
    d->m_id = u;
}

QString Folder::id() const
{
    return d->m_id;
}

void Folder::setName(const QString &name)
{
    d->m_name = name;
}

QString Folder::name() const
{
    return d->m_name;
}

void Folder::setMessageCount(int c)
{
    d->m_messageCount = c;
}

int Folder::messageCount() const
{
    return d->m_messageCount;
}

void Folder::setType(const QString &v)
{
    d->m_type = v;
}

QString Folder::type() const
{
    return d->m_type;
}

bool Folder::isValid() const
{
    return !(d->m_id.isEmpty());
}
