/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "buildservice.h"

using namespace Attica;

class Q_DECL_HIDDEN BuildService::Private : public QSharedData
{
public:
    QString id;
    QString name;
    QString url;
    // QStringList targets;
    QList<Target> targets;

    Private()
    {
    }
};

BuildService::BuildService()
    : d(new Private)
{
}

BuildService::BuildService(const BuildService &other)
    : d(other.d)
{
}

BuildService &BuildService::operator=(const Attica::BuildService &other)
{
    d = other.d;
    return *this;
}

BuildService::~BuildService()
{
}

void BuildService::setId(const QString &u)
{
    d->id = u;
}

QString BuildService::id() const
{
    return d->id;
}

void BuildService::setName(const QString &u)
{
    d->name = u;
}

QString BuildService::name() const
{
    return d->name;
}

void BuildService::addTarget(const Target &t)
{
    d->targets << t;
}

QList<Target> BuildService::targets() const
{
    return d->targets;
}

void BuildService::setUrl(const QString &u)
{
    d->url = u;
}

QString BuildService::url() const
{
    return d->url;
}

bool BuildService::isValid() const
{
    return !(d->id.isEmpty());
}
