/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "category.h"

#include <QDebug>
#include <QString>

using namespace Attica;

class Q_DECL_HIDDEN Category::Private : public QSharedData
{
public:
    QString m_id;
    QString m_name;
    QString m_displayName;
};

Category::Category()
    : d(new Private)
{
}

Category::Category(const Attica::Category &other)
    : d(other.d)
{
}

Category &Category::operator=(const Attica::Category &other)
{
    d = other.d;
    return *this;
}

Category::~Category()
{
}

void Category::setId(const QString &u)
{
    d->m_id = u;
}

QString Category::id() const
{
    return d->m_id;
}

void Category::setName(const QString &name)
{
    d->m_name = name;
}

QString Category::name() const
{
    return d->m_name;
}

void Category::setDisplayName(const QString &name)
{
    d->m_displayName = name;
}

QString Category::displayName() const
{
    return d->m_displayName;
}

bool Category::isValid() const
{
    return !(d->m_id.isEmpty());
}
