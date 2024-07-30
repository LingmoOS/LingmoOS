/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2002, 2006, 2007 David Jarvie <djarvie@kde.org>
  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCALCORE_RECURRENCERULE_H
#define KCALCORE_RECURRENCERULE_H

#include "kcalendarcore_export.h"

#include <QDateTime>
#include <QTimeZone>

class QTimeZone;

namespace KCalendarCore
{
// These two are duplicates wrt. incidencebase.h
typedef QList<QDate> DateList;
/* List of times */
typedef QList<QTime> TimeList;

/**
  This class represents a recurrence rule for a calendar incidence.
*/
class KCALENDARCORE_EXPORT RecurrenceRule
{
public:
    class RuleObserver
    {
    public:
        virtual ~RuleObserver();
        /** This method is called on each change of the recurrence object */
        virtual void recurrenceChanged(RecurrenceRule *) = 0;
    };
    typedef QList<RecurrenceRule *> List;

    /** enum for describing the frequency how an event recurs, if at all. */
    enum PeriodType {
        rNone = 0,
        rSecondly,
        rMinutely,
        rHourly,
        rDaily,
        rWeekly,
        rMonthly,
        rYearly,
    };

    /** structure for describing the n-th weekday of the month/year. */
    class KCALENDARCORE_EXPORT WDayPos // krazy:exclude=dpointer
    {
    public:
        explicit WDayPos(int ps = 0, short dy = 0);
        void setDay(short dy);
        short day() const;
        void setPos(int ps);
        int pos() const;

        bool operator==(const RecurrenceRule::WDayPos &pos2) const;
        bool operator!=(const RecurrenceRule::WDayPos &pos2) const;

    protected:
        short mDay; // Weekday, 1=monday, 7=sunday
        int mPos; // week of the day (-1 for last, 1 for first, 0 for all weeks)
        // Bounded by -366 and +366, 0 means all weeks in that period

        friend KCALENDARCORE_EXPORT QDataStream &operator<<(QDataStream &out, const KCalendarCore::RecurrenceRule::WDayPos &);
        friend KCALENDARCORE_EXPORT QDataStream &operator>>(QDataStream &in, KCalendarCore::RecurrenceRule::WDayPos &);
    };

    RecurrenceRule();
    RecurrenceRule(const RecurrenceRule &r);
    ~RecurrenceRule();

    bool operator==(const RecurrenceRule &r) const;
    bool operator!=(const RecurrenceRule &r) const
    {
        return !operator==(r);
    }

    RecurrenceRule &operator=(const RecurrenceRule &r);

    /** Set if recurrence is read-only or can be changed. */
    void setReadOnly(bool readOnly);

    /**
      Returns true if the recurrence is read-only; false if it can be changed.
    */
    Q_REQUIRED_RESULT bool isReadOnly() const;

    /**
      Returns the event's recurrence status.  See the enumeration at the top
      of this file for possible values.
    */
    Q_REQUIRED_RESULT bool recurs() const;
    void setRecurrenceType(PeriodType period);
    Q_REQUIRED_RESULT PeriodType recurrenceType() const;

    /** Turns off recurrence for the event. */
    void clear();

    /**
      Returns the recurrence frequency, in terms of the recurrence time period type.
    */
    Q_REQUIRED_RESULT uint frequency() const;

    /**
      Sets the recurrence frequency, in terms of the recurrence time period type.
    */
    void setFrequency(int freq);

    /**
      Returns the recurrence start date/time.
      Note that the recurrence does not necessarily occur on the start date/time.
      For this to happen, it must actually match the rule.
    */
    Q_REQUIRED_RESULT QDateTime startDt() const;

    /**
      Sets the recurrence start date/time.
      Note that setting the start date/time does not make the recurrence occur
      on that date/time, it simply sets a lower limit to when the recurrences
      take place (this applies only for the by- rules, not for i.e. an hourly
      rule where the startDt is the first occurrence).

      Note that setting @p start to a date-only value does not make an all-day
      recurrence: to do this, call setAllDay(true).

      @param start the recurrence's start date and time
    */
    void setStartDt(const QDateTime &start);

    /** Returns whether the start date has no time associated. All-Day
        means -- according to rfc2445 -- that the event has no time associate. */
    Q_REQUIRED_RESULT bool allDay() const;

    /** Sets whether the dtstart is all-day (i.e. has no time attached)
     *
     * @param allDay Whether start datetime is all-day
     */
    void setAllDay(bool allDay);

    /** Returns the date and time of the last recurrence.
     * An invalid date is returned if the recurrence has no end.
     * @param result if non-null, *result is updated to true if successful,
     * or false if there is no recurrence or its end date cannot be determined.
     */
    Q_REQUIRED_RESULT QDateTime endDt(bool *result = nullptr) const;

    /** Sets the date and time of the last recurrence.
     * @param endDateTime the ending date/time after which to stop recurring. */
    void setEndDt(const QDateTime &endDateTime);

    /**
     * Returns -1 if the event recurs infinitely, 0 if the end date is set,
     * otherwise the total number of recurrences, including the initial occurrence.
     */
    Q_REQUIRED_RESULT int duration() const;

    /** Sets the total number of times the event is to occur, including both the
     * first and last. */
    void setDuration(int duration);

    /** Returns the number of recurrences up to and including the date/time specified. */
    Q_REQUIRED_RESULT int durationTo(const QDateTime &dt) const;

    /** Returns the number of recurrences up to and including the date specified. */
    Q_REQUIRED_RESULT int durationTo(const QDate &date) const;

    /**
      Shift the times of the rule so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual rule time zone.

      For example, shifting a rule whose start time is 09:00 America/New York,
      using an old viewing time zone (@p oldTz) of Europe/London, to a new time
      zone (@p newTz) of Europe/Paris, will result in the time being shifted
      from 14:00 (which is the London time of the rule start) to 14:00 Paris
      time.

      @param oldTz the time specification which provides the clock times
      @param newTz the new time specification
    */
    void shiftTimes(const QTimeZone &oldTz, const QTimeZone &newTz);

    /** Returns true if the date specified is one on which the event will
     * recur. The start date returns true only if it actually matches the rule.
     *
     * @param date date to check
     * @param timeZone time specification for @p date
     */
    Q_REQUIRED_RESULT bool recursOn(const QDate &date, const QTimeZone &timeZone) const;

    /** Returns true if the date/time specified is one at which the event will
     * recur. Times are rounded down to the nearest minute to determine the result.
     * The start date/time returns true only if it actually matches the rule.
     *
     * @param dt the date+time to check for recurrency
     */
    Q_REQUIRED_RESULT bool recursAt(const QDateTime &dt) const;

    /** Returns true if the date matches the rules. It does not necessarily
        mean that this is an actual occurrence. In particular, the method does
        not check if the date is after the end date, or if the frequency interval
        matches.

        @param dt the date+time to check for matching the rules
     */
    Q_REQUIRED_RESULT bool dateMatchesRules(const QDateTime &dt) const;

    /** Returns a list of the times on the specified date at which the
     * recurrence will occur. The returned times should be interpreted in the
     * context of @p timeZone.
     * @param date the date for which to find the recurrence times
     * @param timeZone time specification for @p date
     */
    Q_REQUIRED_RESULT TimeList recurTimesOn(const QDate &date, const QTimeZone &timeZone) const;

    /** Returns a list of all the times at which the recurrence will occur
     * between two specified times.
     *
     * There is a (large) maximum limit to the number of times returned. If due to
     * this limit the list is incomplete, this is indicated by the last entry being
     * set to an invalid QDateTime value. If you need further values, call the
     * method again with a start time set to just after the last valid time returned.
     * @param start inclusive start of interval
     * @param end inclusive end of interval
     * @return list of date/time values
     */
    Q_REQUIRED_RESULT QList<QDateTime> timesInInterval(const QDateTime &start, const QDateTime &end) const;

    /** Returns the date and time of the next recurrence, after the specified date/time.
     * If the recurrence has no time, the next date after the specified date is returned.
     * @param preDateTime the date/time after which to find the recurrence.
     * @return date/time of next recurrence, or invalid date if none.
     */
    Q_REQUIRED_RESULT QDateTime getNextDate(const QDateTime &preDateTime) const;

    /** Returns the date and time of the last previous recurrence, before the specified date/time.
     * If a time later than 00:00:00 is specified and the recurrence has no time, 00:00:00 on
     * the specified date is returned if that date recurs.
     * @param afterDateTime the date/time before which to find the recurrence.
     * @return date/time of previous recurrence, or invalid date if none.
     */
    Q_REQUIRED_RESULT QDateTime getPreviousDate(const QDateTime &afterDateTime) const;

    void setBySeconds(const QList<int> &bySeconds);
    void setByMinutes(const QList<int> &byMinutes);
    void setByHours(const QList<int> &byHours);

    void setByDays(const QList<WDayPos> &byDays);
    void setByMonthDays(const QList<int> &byMonthDays);
    void setByYearDays(const QList<int> &byYearDays);
    void setByWeekNumbers(const QList<int> &byWeekNumbers);
    void setByMonths(const QList<int> &byMonths);
    void setBySetPos(const QList<int> &bySetPos);
    void setWeekStart(short weekStart);

    const QList<int> &bySeconds() const;
    const QList<int> &byMinutes() const;
    const QList<int> &byHours() const;

    const QList<WDayPos> &byDays() const;
    const QList<int> &byMonthDays() const;
    const QList<int> &byYearDays() const;
    const QList<int> &byWeekNumbers() const;
    const QList<int> &byMonths() const;
    const QList<int> &bySetPos() const;
    short weekStart() const;

    /**
      Set the RRULE string for the rule.
      This is merely stored for future reference. The string is not used in any way
      by the RecurrenceRule.

      @param rrule the RRULE string
     */
    void setRRule(const QString &rrule);
    Q_REQUIRED_RESULT QString rrule() const;

    void setDirty();
    /**
      Installs an observer. Whenever some setting of this recurrence
      object is changed, the recurrenceUpdated( Recurrence* ) method
      of each observer will be called to inform it of changes.
      @param observer the Recurrence::Observer-derived object, which
      will be installed as an observer of this object.
    */
    void addObserver(RuleObserver *observer);

    /**
      Removes an observer that was added with addObserver. If the
      given object was not an observer, it does nothing.
      @param observer the Recurrence::Observer-derived object to
      be removed from the list of observers of this object.
    */
    void removeObserver(RuleObserver *observer);

    /**
      Debug output.
    */
    void dump() const;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    friend KCALENDARCORE_EXPORT QDataStream &operator<<(QDataStream &out, const KCalendarCore::RecurrenceRule *);
    friend KCALENDARCORE_EXPORT QDataStream &operator>>(QDataStream &in, const KCalendarCore::RecurrenceRule *);
};

/**
 * RecurrenceRule serializer and deserializer.
 * @since 4.12
 */
KCALENDARCORE_EXPORT QDataStream &operator<<(QDataStream &out, const KCalendarCore::RecurrenceRule *);
KCALENDARCORE_EXPORT QDataStream &operator>>(QDataStream &in, const KCalendarCore::RecurrenceRule *);

/**
 * RecurrenceRule::WDayPos serializer and deserializer.
 * @since 4.12
 */
KCALENDARCORE_EXPORT QDataStream &operator<<(QDataStream &out, const KCalendarCore::RecurrenceRule::WDayPos &);
KCALENDARCORE_EXPORT QDataStream &operator>>(QDataStream &in, KCalendarCore::RecurrenceRule::WDayPos &);
}

Q_DECLARE_TYPEINFO(KCalendarCore::RecurrenceRule::WDayPos, Q_RELOCATABLE_TYPE);

#endif
