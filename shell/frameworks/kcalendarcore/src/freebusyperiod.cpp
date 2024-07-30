/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 David Jarvie <djarvie@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the FreeBusyPeriod class.

  @brief
  Represents a period of time.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "freebusyperiod.h"

using namespace KCalendarCore;

//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::FreeBusyPeriod::Private
{
public:
    Private()
        : mType(Unknown)
    {
    }

    QString mSummary;
    QString mLocation;
    FreeBusyType mType;
};
//@endcond

FreeBusyPeriod::FreeBusyPeriod()
    : Period()
    , d(new KCalendarCore::FreeBusyPeriod::Private())
{
}

FreeBusyPeriod::FreeBusyPeriod(const QDateTime &start, const QDateTime &end)
    : Period(start, end)
    , d(new KCalendarCore::FreeBusyPeriod::Private())
{
}

FreeBusyPeriod::FreeBusyPeriod(const QDateTime &start, const Duration &duration)
    : Period(start, duration)
    , d(new KCalendarCore::FreeBusyPeriod::Private())
{
}

FreeBusyPeriod::FreeBusyPeriod(const FreeBusyPeriod &period)
    : Period(period)
    , d(new KCalendarCore::FreeBusyPeriod::Private(*period.d))
{
}

FreeBusyPeriod::FreeBusyPeriod(const Period &period)
    : Period(period)
    , d(new KCalendarCore::FreeBusyPeriod::Private())
{
}

FreeBusyPeriod::~FreeBusyPeriod()
{
    delete d;
}

FreeBusyPeriod &FreeBusyPeriod::operator=(const FreeBusyPeriod &other)
{
    // check for self assignment
    if (&other == this) {
        return *this;
    }

    Period::operator=(other);
    *d = *other.d;
    return *this;
}

QString FreeBusyPeriod::summary() const
{
    return d->mSummary;
}

void FreeBusyPeriod::setSummary(const QString &summary)
{
    d->mSummary = summary;
}

QString FreeBusyPeriod::location() const
{
    return d->mLocation;
}

void FreeBusyPeriod::setLocation(const QString &location)
{
    d->mLocation = location;
}

FreeBusyPeriod::FreeBusyType FreeBusyPeriod::type() const
{
    return d->mType;
}

void FreeBusyPeriod::setType(FreeBusyPeriod::FreeBusyType type)
{
    d->mType = type;
}

QDataStream &KCalendarCore::operator<<(QDataStream &stream, const KCalendarCore::FreeBusyPeriod &period)
{
    KCalendarCore::Period periodParent = static_cast<KCalendarCore::Period>(period);
    stream << periodParent;
    stream << period.summary() << period.location() << static_cast<int>(period.type());
    return stream;
}

QDataStream &KCalendarCore::operator>>(QDataStream &stream, FreeBusyPeriod &period)
{
    KCalendarCore::Period periodParent;
    QString summary;
    QString location;
    int type;

    stream >> periodParent >> summary >> location >> type;

    period = periodParent;
    period.setLocation(location);
    period.setSummary(summary);
    period.setType(static_cast<FreeBusyPeriod::FreeBusyType>(type));
    return stream;
}
