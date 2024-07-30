/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "clientpidmap.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN ClientPidMap::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        clientpidmap = other.clientpidmap;
    }

    ParameterMap mParamMap;
    QString clientpidmap;
};

ClientPidMap::ClientPidMap()
    : d(new Private)
{
}

ClientPidMap::ClientPidMap(const ClientPidMap &other)
    : d(other.d)
{
}

ClientPidMap::ClientPidMap(const QString &clientpidmap)
    : d(new Private)
{
    d->clientpidmap = clientpidmap;
}

ClientPidMap::~ClientPidMap()
{
}

void ClientPidMap::setClientPidMap(const QString &clientpidmap)
{
    d->clientpidmap = clientpidmap;
}

QString ClientPidMap::clientPidMap() const
{
    return d->clientpidmap;
}

bool ClientPidMap::isValid() const
{
    return !d->clientpidmap.isEmpty();
}

void ClientPidMap::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap ClientPidMap::params() const
{
    return d->mParamMap;
}

bool ClientPidMap::operator==(const ClientPidMap &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->clientpidmap == other.clientPidMap());
}

bool ClientPidMap::operator!=(const ClientPidMap &other) const
{
    return !(other == *this);
}

ClientPidMap &ClientPidMap::operator=(const ClientPidMap &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString ClientPidMap::toString() const
{
    QString str = QLatin1String("ClientPidMap {\n");
    str += QStringLiteral("    clientpidmap: %1\n").arg(d->clientpidmap);
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const ClientPidMap &clientpidmap)
{
    return s << clientpidmap.d->mParamMap << clientpidmap.d->clientpidmap;
}

QDataStream &KContacts::operator>>(QDataStream &s, ClientPidMap &clientpidmap)
{
    s >> clientpidmap.d->mParamMap >> clientpidmap.d->clientpidmap;
    return s;
}
