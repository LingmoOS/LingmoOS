/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "title.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN Title::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        title = other.title;
    }

    ParameterMap mParamMap;
    QString title;
};

Title::Title()
    : d(new Private)
{
}

Title::Title(const Title &other)
    : d(other.d)
{
}

Title::Title(const QString &title)
    : d(new Private)
{
    d->title = title;
}

Title::~Title()
{
}

void Title::setTitle(const QString &title)
{
    d->title = title;
}

QString Title::title() const
{
    return d->title;
}

bool Title::isValid() const
{
    return !d->title.isEmpty();
}

void Title::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap Title::params() const
{
    return d->mParamMap;
}

bool Title::operator==(const Title &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->title == other.title());
}

bool Title::operator!=(const Title &other) const
{
    return !(other == *this);
}

Title &Title::operator=(const Title &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString Title::toString() const
{
    QString str = QLatin1String("Title {\n");
    str += QStringLiteral("    title: %1\n").arg(d->title);
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const Title &title)
{
    return s << title.d->mParamMap << title.d->title;
}

QDataStream &KContacts::operator>>(QDataStream &s, Title &title)
{
    s >> title.d->mParamMap >> title.d->title;
    return s;
}
