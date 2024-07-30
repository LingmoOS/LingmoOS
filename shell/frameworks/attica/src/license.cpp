/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "license.h"

using namespace Attica;

class Q_DECL_HIDDEN License::Private : public QSharedData
{
public:
    int id;
    QString name;
    QUrl url;

    Private()
        : id(-1)
    {
    }
};

License::License()
    : d(new Private)
{
}

License::License(const Attica::License &other)
    : d(other.d)
{
}

License &License::operator=(const Attica::License &other)
{
    d = other.d;
    return *this;
}

License::~License()
{
}

uint License::id() const
{
    return d->id;
}

void License::setId(uint id)
{
    d->id = id;
}

QString License::name() const
{
    return d->name;
}

void License::setName(const QString &name)
{
    d->name = name;
}

void License::setUrl(const QUrl &url)
{
    d->url = url;
}

QUrl License::url() const
{
    return d->url;
}
