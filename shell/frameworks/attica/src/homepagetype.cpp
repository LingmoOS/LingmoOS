/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "homepagetype.h"

using namespace Attica;

class Q_DECL_HIDDEN HomePageType::Private : public QSharedData
{
public:
    int id;
    QString name;

    Private()
        : id(-1)
    {
    }
};

HomePageType::HomePageType()
    : d(new Private)
{
}

HomePageType::HomePageType(const Attica::HomePageType &other)
    : d(other.d)
{
}

HomePageType &HomePageType::operator=(const Attica::HomePageType &other)
{
    d = other.d;
    return *this;
}

HomePageType::~HomePageType()
{
}

uint HomePageType::id() const
{
    return d->id;
}

void HomePageType::setId(uint id)
{
    d->id = id;
}

QString HomePageType::name() const
{
    return d->name;
}

void HomePageType::setName(const QString &name)
{
    d->name = name;
}
