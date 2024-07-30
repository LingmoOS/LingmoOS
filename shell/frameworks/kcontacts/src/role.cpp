/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "role.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN Role::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        role = other.role;
    }

    ParameterMap mParamMap;
    QString role;
};

Role::Role()
    : d(new Private)
{
}

Role::Role(const Role &other)
    : d(other.d)
{
}

Role::Role(const QString &role)
    : d(new Private)
{
    d->role = role;
}

Role::~Role()
{
}

void Role::setRole(const QString &role)
{
    d->role = role;
}

QString Role::role() const
{
    return d->role;
}

bool Role::isValid() const
{
    return !d->role.isEmpty();
}

void Role::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap Role::params() const
{
    return d->mParamMap;
}

bool Role::operator==(const Role &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->role == other.role());
}

bool Role::operator!=(const Role &other) const
{
    return !(other == *this);
}

Role &Role::operator=(const Role &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString Role::toString() const
{
    QString str = QLatin1String("Role {\n");
    str += QStringLiteral("    role: %1\n").arg(d->role);
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const Role &role)
{
    return s << role.d->mParamMap << role.d->role;
}

QDataStream &KContacts::operator>>(QDataStream &s, Role &role)
{
    s >> role.d->mParamMap >> role.d->role;
    return s;
}
