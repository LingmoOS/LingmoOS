/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarurl.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>
#include <QUrl>

using namespace KContacts;

class Q_DECL_HIDDEN CalendarUrl::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        type = other.type;
        url = other.url;
    }

    static QString typeToString(CalendarUrl::CalendarType type);

    ParameterMap mParamMap;
    QUrl url;
    CalendarUrl::CalendarType type;
};

QString CalendarUrl::Private::typeToString(CalendarUrl::CalendarType type)
{
    QString ret;
    switch (type) {
    case Unknown:
    case EndCalendarType:
        ret = QStringLiteral("Unknown");
        break;
    case FBUrl:
        ret = QStringLiteral("FreeBusy");
        break;
    case CALUri:
        ret = QStringLiteral("CalUri");
        break;
    case CALADRUri:
        ret = QStringLiteral("Caladruri");
        break;
    }
    return ret;
}

CalendarUrl::CalendarUrl()
    : d(new Private)
{
    d->type = Unknown;
}

CalendarUrl::CalendarUrl(CalendarUrl::CalendarType type)
    : d(new Private)
{
    d->type = type;
}

CalendarUrl::CalendarUrl(const CalendarUrl &other)
    : d(other.d)
{
}

CalendarUrl::~CalendarUrl()
{
}

bool CalendarUrl::operator==(const CalendarUrl &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->type == other.type()) && (d->url == other.url());
}

bool CalendarUrl::operator!=(const CalendarUrl &other) const
{
    return !(other == *this);
}

CalendarUrl &CalendarUrl::operator=(const CalendarUrl &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString CalendarUrl::toString() const
{
    QString str = QLatin1String("CalendarUrl {\n");
    str += QStringLiteral("    url: %1\n").arg(d->url.toString());
    str += QStringLiteral("    type: %1\n").arg(CalendarUrl::Private::typeToString(d->type));
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

void CalendarUrl::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap CalendarUrl::params() const
{
    return d->mParamMap;
}

bool CalendarUrl::isValid() const
{
    return (d->type != Unknown) && (d->url.isValid());
}

void CalendarUrl::setType(CalendarUrl::CalendarType type)
{
    d->type = type;
}

CalendarUrl::CalendarType CalendarUrl::type() const
{
    return d->type;
}

void CalendarUrl::setUrl(const QUrl &url)
{
    d->url = url;
}

QUrl CalendarUrl::url() const
{
    return d->url;
}

QDataStream &KContacts::operator<<(QDataStream &s, const CalendarUrl &calUrl)
{
    return s << calUrl.d->mParamMap << static_cast<uint>(calUrl.d->type) << calUrl.d->url;
}

QDataStream &KContacts::operator>>(QDataStream &s, CalendarUrl &calUrl)
{
    uint type;
    s >> calUrl.d->mParamMap >> type >> calUrl.d->url;
    calUrl.d->type = static_cast<CalendarUrl::CalendarType>(type);
    return s;
}
