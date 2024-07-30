/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "homepageentry.h"

using namespace Attica;

class Q_DECL_HIDDEN HomePageEntry::Private : public QSharedData
{
public:
    QString type;
    QUrl url;

    Private()
    {
    }
};

HomePageEntry::HomePageEntry()
    : d(new Private)
{
}

HomePageEntry::HomePageEntry(const Attica::HomePageEntry &other)
    : d(other.d)
{
}

HomePageEntry &HomePageEntry::operator=(const Attica::HomePageEntry &other)
{
    d = other.d;
    return *this;
}

HomePageEntry::~HomePageEntry()
{
}

QString HomePageEntry::type() const
{
    return d->type;
}

void HomePageEntry::setType(const QString &type)
{
    d->type = type;
}

QUrl HomePageEntry::url() const
{
    return d->url;
}

void HomePageEntry::setUrl(const QUrl &url)
{
    d->url = url;
}
