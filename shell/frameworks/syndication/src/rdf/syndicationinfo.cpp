/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "syndicationinfo.h"
#include "property.h"
#include "statement.h"
#include "syndicationvocab.h"

#include <tools.h>

#include <QString>

namespace Syndication
{
namespace RDF
{
SyndicationInfo::SyndicationInfo(ResourcePtr resource)
    : ResourceWrapper(resource)
{
}

SyndicationInfo::~SyndicationInfo()
{
}

SyndicationInfo::Period SyndicationInfo::updatePeriod() const
{
    return stringToPeriod(resource()->property(SyndicationVocab::self()->updatePeriod())->asString());
}

int SyndicationInfo::updateFrequency() const
{
    QString freqStr = resource()->property(SyndicationVocab::self()->updateFrequency())->asString();

    if (freqStr.isEmpty()) {
        return 1; // 1 is default
    }

    bool ok = false;
    int freq = freqStr.toInt(&ok);

    if (ok) {
        return freq;
    } else {
        return 1; // 1 is default
    }
}

time_t SyndicationInfo::updateBase() const
{
    QString str = resource()->property(SyndicationVocab::self()->updateBase())->asString();

    return parseDate(str, ISODate);
}

QString SyndicationInfo::debugInfo() const
{
    QString info;
    if (updatePeriod() != Daily) {
        info += QStringLiteral("syn:updatePeriod: #%1#\n").arg(periodToString(updatePeriod()));
    }
    info += QStringLiteral("syn:updateFrequency: #%1#\n").arg(QString::number(updateFrequency()));

    const QString dbase = dateTimeToString(updateBase());
    if (!dbase.isNull()) {
        info += QStringLiteral("syn:updateBase: #%1#\n").arg(dbase);
    }

    return info;
}

QString SyndicationInfo::periodToString(Period period)
{
    switch (period) {
    case Daily:
        return QStringLiteral("daily");
    case Hourly:
        return QStringLiteral("hourly");
    case Monthly:
        return QStringLiteral("monthly");
    case Weekly:
        return QStringLiteral("weekly");
    case Yearly:
        return QStringLiteral("yearly");
    default: // should never happen
        return QString();
    }
}

SyndicationInfo::Period SyndicationInfo::stringToPeriod(const QString &str)
{
    if (str.isEmpty()) {
        return Daily; // default is "daily"
    }

    if (str == QLatin1String("hourly")) {
        return Hourly;
    }
    if (str == QLatin1String("monthly")) {
        return Monthly;
    }
    if (str == QLatin1String("weekly")) {
        return Weekly;
    }
    if (str == QLatin1String("yearly")) {
        return Yearly;
    }

    return Daily; // default is "daily"
}

} // namespace RDF
} // namespace Syndication
