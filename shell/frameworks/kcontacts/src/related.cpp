/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "related.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN Related::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        relatedTo = other.relatedTo;
    }

    ParameterMap mParamMap;
    QString relatedTo;
};

Related::Related()
    : d(new Private)
{
}

Related::Related(const Related &other)
    : d(other.d)
{
}

Related::Related(const QString &relatedTo)
    : d(new Private)
{
    d->relatedTo = relatedTo;
}

Related::~Related()
{
}

void Related::setRelated(const QString &relatedTo)
{
    d->relatedTo = relatedTo;
}

QString Related::related() const
{
    return d->relatedTo;
}

bool Related::isValid() const
{
    return !d->relatedTo.isEmpty();
}

void Related::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap Related::params() const
{
    return d->mParamMap;
}

bool Related::operator==(const Related &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->relatedTo == other.related());
}

bool Related::operator!=(const Related &other) const
{
    return !(other == *this);
}

Related &Related::operator=(const Related &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString Related::toString() const
{
    QString str = QLatin1String("Related {\n");
    str += QStringLiteral("    relatedTo: %1\n").arg(d->relatedTo);
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const Related &related)
{
    return s << related.d->mParamMap << related.d->relatedTo;
}

QDataStream &KContacts::operator>>(QDataStream &s, Related &related)
{
    s >> related.d->mParamMap >> related.d->relatedTo;
    return s;
}
