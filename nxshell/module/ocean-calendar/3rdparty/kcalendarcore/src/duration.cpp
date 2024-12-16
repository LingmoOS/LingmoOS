/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 David Jarvie <djarvie@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Duration class.

  @brief
  Represents a span of time measured in seconds.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author David Jarvie \<djarvie@kde.org\>
*/
#include "duration.h"

#include <QDateTime>
#include <QTimeZone>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::Duration::Private
{
public:
    int seconds() const
    {
        return mDaily ? mDuration * 86400 : mDuration;
    }
    int mDuration; // number of seconds or days in the duration
    bool mDaily = false; // specified in terms of days rather than seconds
};
//@endcond

Duration::Duration()
    : d(new KCalendarCore::Duration::Private())
{
}

Duration::Duration(const QDateTime &start, const QDateTime &end)
    : d(new KCalendarCore::Duration::Private())
{
    if (start.time() == end.time() && start.timeZone() == end.timeZone()) {
        d->mDuration = start.daysTo(end);
        d->mDaily = true;
    } else {
        d->mDuration = start.secsTo(end);
        d->mDaily = false;
    }
}

Duration::Duration(const QDateTime &start, const QDateTime &end, Type type)
    : d(new KCalendarCore::Duration::Private())
{
    if (type == Days) {
        QDateTime endSt(end.toTimeZone(start.timeZone()));
        d->mDuration = start.daysTo(endSt);
        if (d->mDuration) {
            // Round down to whole number of days if necessary
            if (start < endSt) {
                if (endSt.time() < start.time()) {
                    --d->mDuration;
                }
            } else {
                if (endSt.time() > start.time()) {
                    ++d->mDuration;
                }
            }
        }
        d->mDaily = true;
    } else {
        d->mDuration = start.secsTo(end);
        d->mDaily = false;
    }
}

Duration::Duration(int duration, Type type)
    : d(new KCalendarCore::Duration::Private())
{
    d->mDuration = duration;
    d->mDaily = (type == Days);
}

Duration::Duration(const Duration &duration)
    : d(new KCalendarCore::Duration::Private(*duration.d))
{
}

Duration::~Duration()
{
    delete d;
}

Duration &Duration::operator=(const Duration &duration)
{
    // check for self assignment
    if (&duration == this) {
        return *this;
    }

    *d = *duration.d;
    return *this;
}

Duration::operator bool() const
{
    return d->mDuration;
}

bool Duration::operator<(const Duration &other) const
{
    if (d->mDaily == other.d->mDaily) {
        // guard against integer overflow for two daily durations
        return d->mDuration < other.d->mDuration;
    }
    return d->seconds() < other.d->seconds();
}

bool Duration::operator==(const Duration &other) const
{
    // Note: daily and non-daily durations are always unequal, since a day's
    // duration may differ from 24 hours if it happens to span a daylight saving
    // time change.
    return d->mDuration == other.d->mDuration && d->mDaily == other.d->mDaily;
}

Duration &Duration::operator+=(const Duration &other)
{
    if (d->mDaily == other.d->mDaily) {
        d->mDuration += other.d->mDuration;
    } else if (d->mDaily) {
        d->mDuration = d->mDuration * 86400 + other.d->mDuration;
        d->mDaily = false;
    } else {
        d->mDuration += other.d->mDuration + 86400;
    }
    return *this;
}

Duration Duration::operator-() const
{
    return Duration(-d->mDuration, (d->mDaily ? Days : Seconds));
}

Duration &Duration::operator-=(const Duration &duration)
{
    return operator+=(-duration);
}

Duration &Duration::operator*=(int value)
{
    d->mDuration *= value;
    return *this;
}

Duration &Duration::operator/=(int value)
{
    d->mDuration /= value;
    return *this;
}

QDateTime Duration::end(const QDateTime &start) const
{
    return d->mDaily ? start.addDays(d->mDuration) : start.addSecs(d->mDuration);
}

Duration::Type Duration::type() const
{
    return d->mDaily ? Days : Seconds;
}

bool Duration::isDaily() const
{
    return d->mDaily;
}

int Duration::asSeconds() const
{
    return d->seconds();
}

int Duration::asDays() const
{
    return d->mDaily ? d->mDuration : d->mDuration / 86400;
}

int Duration::value() const
{
    return d->mDuration;
}

bool Duration::isNull() const
{
    return d->mDuration == 0;
}

QDataStream &KCalendarCore::operator<<(QDataStream &out, const KCalendarCore::Duration &duration)
{
    out << duration.d->mDuration << duration.d->mDaily;
    return out;
}

QDataStream &KCalendarCore::operator>>(QDataStream &in, KCalendarCore::Duration &duration)
{
    in >> duration.d->mDuration >> duration.d->mDaily;
    return in;
}
