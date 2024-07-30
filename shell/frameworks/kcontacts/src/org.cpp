/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "org.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN Org::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        organization = other.organization;
    }

    ParameterMap mParamMap;
    QString organization;
};

Org::Org()
    : d(new Private)
{
}

Org::Org(const Org &other)
    : d(other.d)
{
}

Org::Org(const QString &org)
    : d(new Private)
{
    d->organization = org;
}

Org::~Org()
{
}

void Org::setOrganization(const QString &org)
{
    d->organization = org;
}

QString Org::organization() const
{
    return d->organization;
}

bool Org::isValid() const
{
    return !d->organization.isEmpty();
}

void Org::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap Org::params() const
{
    return d->mParamMap;
}

bool Org::operator==(const Org &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->organization == other.organization());
}

bool Org::operator!=(const Org &other) const
{
    return !(other == *this);
}

Org &Org::operator=(const Org &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString Org::toString() const
{
    QString str = QLatin1String("Org {\n");
    str += QStringLiteral("    organization: %1\n").arg(d->organization);
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const Org &org)
{
    return s << org.d->mParamMap << org.d->organization;
}

QDataStream &KContacts::operator>>(QDataStream &s, Org &org)
{
    s >> org.d->mParamMap >> org.d->organization;
    return s;
}
