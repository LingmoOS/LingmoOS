/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "publisher.h"

using namespace Attica;

class Q_DECL_HIDDEN Publisher::Private : public QSharedData
{
public:
    QString id;
    QString name;
    QString url;
    QList<Field> fields;
    QList<Target> targets;

    Private()
    {
    }
};

Publisher::Publisher()
    : d(new Private)
{
}

Publisher::Publisher(const Publisher &other)
    : d(other.d)
{
}

Publisher &Publisher::operator=(const Attica::Publisher &other)
{
    d = other.d;
    return *this;
}

Publisher::~Publisher()
{
}

void Publisher::setId(const QString &u)
{
    d->id = u;
}

QString Publisher::id() const
{
    return d->id;
}

void Publisher::setName(const QString &u)
{
    d->name = u;
}

QString Publisher::name() const
{
    return d->name;
}

void Publisher::addField(const Field &t)
{
    d->fields << t;
}

QList<Field> Publisher::fields() const
{
    return d->fields;
}

void Publisher::setUrl(const QString &u)
{
    d->url = u;
}

QString Publisher::url() const
{
    return d->url;
}

void Publisher::addTarget(const Attica::Target &t)
{
    d->targets << t;
}

QList<Target> Publisher::targets() const
{
    return d->targets;
}

bool Publisher::isValid() const
{
    return !(d->id.isEmpty());
}
