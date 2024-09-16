/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 David Jarvie <djarvie@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Period class.

  @brief
  Represents a period of time.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "period.h"
#include "utils_p.h"

#include <QHash>
#include <QTimeZone>

using namespace KCalendarCore;

//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::Period::Private
{
public:
    Private()
        : mHasDuration(false)
        , mDailyDuration(false)
    {
    }
    Private(const QDateTime &start, const QDateTime &end, bool hasDuration)
        : mStart(start)
        , mEnd(end)
        , mHasDuration(hasDuration)
        , mDailyDuration(false)
    {
    }
    QDateTime mStart; // period starting date/time
    QDateTime mEnd; // period ending date/time
    bool mHasDuration = false; // does period have a duration?
    bool mDailyDuration = false; // duration is defined as number of days, not seconds
};
//@endcond

Period::Period()
    : d(new KCalendarCore::Period::Private())
{
}

Period::Period(const QDateTime &start, const QDateTime &end)
    : d(new KCalendarCore::Period::Private(start, end, false))
{
}

Period::Period(const QDateTime &start, const Duration &duration)
    : d(new KCalendarCore::Period::Private(start, duration.end(start), true))
{
    d->mDailyDuration = duration.isDaily();
}

Period::Period(const Period &period)
    : d(new KCalendarCore::Period::Private(*period.d))
{
}

Period::~Period()
{
    delete d;
}

bool Period::operator<(const Period &other) const
{
    return d->mStart < other.d->mStart;
}

bool Period::operator==(const Period &other) const
{
    return ((d->mStart == other.d->mStart) || (!d->mStart.isValid() && !other.d->mStart.isValid()))
           && ((d->mEnd == other.d->mEnd) || (!d->mEnd.isValid() && !other.d->mEnd.isValid())) && d->mHasDuration == other.d->mHasDuration;
}

Period &Period::operator=(const Period &other)
{
    // check for self assignment
    if (&other == this) {
        return *this;
    }

    *d = *other.d;
    return *this;
}

QDateTime Period::start() const
{
    return d->mStart;
}

QDateTime Period::end() const
{
    return d->mEnd;
}

Duration Period::duration() const
{
    if (d->mHasDuration) {
        return Duration(d->mStart, d->mEnd, d->mDailyDuration ? Duration::Days : Duration::Seconds);
    } else {
        return Duration(d->mStart, d->mEnd);
    }
}

Duration Period::duration(Duration::Type type) const
{
    return Duration(d->mStart, d->mEnd, type);
}

bool Period::hasDuration() const
{
    return d->mHasDuration;
}

void Period::shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone)
{
    if (oldZone.isValid() && newZone.isValid() && oldZone != newZone) {
        d->mStart = d->mStart.toTimeZone(oldZone);
        d->mStart.setTimeZone(newZone);
        d->mEnd = d->mEnd.toTimeZone(oldZone);
        d->mEnd.setTimeZone(newZone);
    }
}

QDataStream &KCalendarCore::operator<<(QDataStream &stream, const KCalendarCore::Period &period)
{
    serializeQDateTimeAsKDateTime(stream, period.d->mStart);
    serializeQDateTimeAsKDateTime(stream, period.d->mEnd);
    return stream << period.d->mDailyDuration << period.d->mHasDuration;
}

QDataStream &KCalendarCore::operator>>(QDataStream &stream, KCalendarCore::Period &period)
{
    deserializeKDateTimeAsQDateTime(stream, period.d->mStart);
    deserializeKDateTimeAsQDateTime(stream, period.d->mEnd);
    stream >> period.d->mDailyDuration >> period.d->mHasDuration;
    return stream;
}

uint KCalendarCore::qHash(const KCalendarCore::Period &key)
{
    if (key.hasDuration()) {
        return qHash(key.duration());
    } else {
        return qHash(key.start().toString() + key.end().toString());
    }
}
