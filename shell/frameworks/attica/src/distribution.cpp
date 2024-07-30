/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "distribution.h"

using namespace Attica;

class Q_DECL_HIDDEN Distribution::Private : public QSharedData
{
public:
    int id;
    QString name;

    Private()
        : id(-1)
    {
    }
};

Distribution::Distribution()
    : d(new Private)
{
}

Distribution::Distribution(const Attica::Distribution &other)
    : d(other.d)
{
}

Distribution &Distribution::operator=(const Attica::Distribution &other)
{
    d = other.d;
    return *this;
}

Distribution::~Distribution()
{
}

uint Distribution::id() const
{
    return d->id;
}

void Distribution::setId(uint id)
{
    d->id = id;
}

QString Distribution::name() const
{
    return d->name;
}

void Distribution::setName(const QString &name)
{
    d->name = name;
}
