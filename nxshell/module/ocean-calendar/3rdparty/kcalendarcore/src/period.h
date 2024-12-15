/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCALCORE_PERIOD_H
#define KCALCORE_PERIOD_H

#include "duration.h"

#include <QDataStream>
#include <QDateTime>
#include <QMetaType>
#include <QVector>

class QTimeZone;

namespace KCalendarCore {
/**
  The period can be defined by either a start time and an end time or
  by a start time and a duration.
*/
class Q_CORE_EXPORT Period
{
public:
    /**
       List of periods.
     */
    typedef QVector<Period> List;

    /**
      Constructs a period without a duration.
    */
    Period();

    /**
      Constructs a period from @p start to @p end.

      @param start the time the period begins.
      @param end the time the period ends.
    */
    Period(const QDateTime &start, const QDateTime &end);

    /**
      Constructs a period from @p start and lasting @p duration.

      @param start the time when the period starts.
      @param duration how long the period lasts.
    */
    Period(const QDateTime &start, const Duration &duration);

    /**
      Constructs a period by copying another period object

      @param period the period to copy
     */

    Period(const Period &period);

    /**
      Destroys a period.
    */
    ~Period();

    /**
      Returns true if the start of this period is earlier than the start of
      the @p other one.

      @param other is the other period to compare.
    */
    bool operator<(const Period &other) const;

    /**
      Returns true if the start of this period is later than the start of
      the @p other one.

      @param other the other period to compare
    */
    bool operator>(const Period &other) const
    {
        return other.operator<(*this);
    }

    /**
      Returns true if this period is equal to the @p other one.
      Even if their start and end times are the same, two periods are
      considered not equal if one is defined in terms of a duration and the
      other in terms of a start and end time.

      @param other the other period to compare
    */
    bool operator==(const Period &other) const;

    /**
      Returns true if this period is not equal to the @p other one.

      @param other the other period to compare
      @see operator==()
    */
    bool operator!=(const Period &other) const
    {
        return !operator==(other);
    }

    /**
      Sets this period equal to the @p other one.

      @param other is the other period to compare.
    */
    Period &operator=(const Period &other);

    /**
      Returns when this period starts.
    */
    Q_REQUIRED_RESULT QDateTime start() const;

    /**
      Returns when this period ends.
    */
    Q_REQUIRED_RESULT QDateTime end() const;

    /**
      Returns the duration of the period.

      If the period is defined in terms of a start and end time, the duration
      is computed from these. In this case, if the time of day in @p start and
      @p end is equal, and their time specifications (i.e. time zone etc.) are
      the same, the duration will be set in terms of days. Otherwise, the
      duration will be set in terms of seconds.

      If the period is defined in terms of a duration, that duration is
      returned unchanged.
    */
    Q_REQUIRED_RESULT Duration duration() const;

    /**
      Returns the duration of the period.

      If the period is defined in terms of a start and end time, the duration
      is first computed from these.

      If @p type is Days, and the duration is not an exact number of days,
      the duration will be rounded down to the nearest whole number of days.

      @param type the unit of time to use (seconds or days)
    */
    Q_REQUIRED_RESULT Duration duration(Duration::Type type) const;

    /**
      Returns true if this period has a set duration, false
      if it just has a start and an end.
    */
    Q_REQUIRED_RESULT bool hasDuration() const;

    /**
      Shift the times of the period so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual period time zone.

      For example, shifting a period whose start time is 09:00 America/New York,
      using an old viewing time zone (@p oldSpec) of Europe/London, to a new
      time zone (@p newSpec) of Europe/Paris, will result in the time being
      shifted from 14:00 (which is the London time of the period start) to
      14:00 Paris time.

      @param oldZone the time zone which provides the clock times
      @param newZone the new time zone
    */
    void shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone);

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KCalendarCore::Period &period);

    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, KCalendarCore::Period &period);
};

/** Write @p period to the datastream @p stream, in binary format. */
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KCalendarCore::Period &period);

/** Read a Period object into @p period from @p stream, in binary format. */
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, KCalendarCore::Period &period);

/**
  Return a hash value for a Period argument.
  @param key is a Period.
*/
Q_CORE_EXPORT uint qHash(const KCalendarCore::Period &key);
} // namespace KCalendarCore

//@cond PRIVATE
Q_DECLARE_METATYPE(KCalendarCore::Period)
Q_DECLARE_TYPEINFO(KCalendarCore::Period, Q_MOVABLE_TYPE);
//@endcond

#endif
