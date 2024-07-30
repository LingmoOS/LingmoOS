/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "publisherfield.h"

using namespace Attica;

class Q_DECL_HIDDEN PublisherField::Private : public QSharedData
{
public:
    QString name;
    QString type;
    QString data;

    Private()
    {
    }
};

PublisherField::PublisherField()
    : d(new Private)
{
}

PublisherField::PublisherField(const PublisherField &other)
    : d(other.d)
{
}

PublisherField &PublisherField::operator=(const Attica::PublisherField &other)
{
    d = other.d;
    return *this;
}

PublisherField::~PublisherField()
{
}

void PublisherField::setName(const QString &value)
{
    d->name = value;
}

QString PublisherField::name() const
{
    return d->name;
}

void PublisherField::setType(const QString &value)
{
    d->type = value;
}

QString PublisherField::type() const
{
    return d->type;
}

void PublisherField::setData(const QString &value)
{
    d->data = value;
}

QString PublisherField::data() const
{
    return d->data;
}

bool PublisherField::isValid() const
{
    return true;
}
