/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lang.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN Lang::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        language = other.language;
    }

    ParameterMap mParamMap;
    QString language;
};

Lang::Lang()
    : d(new Private)
{
}

Lang::Lang(const Lang &other)
    : d(other.d)
{
}

Lang::Lang(const QString &language)
    : d(new Private)
{
    d->language = language;
}

Lang::~Lang()
{
}

void Lang::setLanguage(const QString &lang)
{
    d->language = lang;
}

QString Lang::language() const
{
    return d->language;
}

bool Lang::isValid() const
{
    return !d->language.isEmpty();
}

void Lang::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap Lang::params() const
{
    return d->mParamMap;
}

bool Lang::operator==(const Lang &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->language == other.language());
}

bool Lang::operator!=(const Lang &other) const
{
    return !(other == *this);
}

Lang &Lang::operator=(const Lang &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString Lang::toString() const
{
    QString str = QLatin1String("Lang {\n");
    str += QStringLiteral("    language: %1\n").arg(d->language);
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const Lang &lang)
{
    return s << lang.d->mParamMap << lang.d->language;
}

QDataStream &KContacts::operator>>(QDataStream &s, Lang &lang)
{
    s >> lang.d->mParamMap >> lang.d->language;
    return s;
}
