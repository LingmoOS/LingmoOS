/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  SPDX-FileContributor: Alvaro Manera <alvaro.manera@nokia.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "sorting.h"

// PENDING(kdab) Review
// The QString::compare() need to be replace by a DUI string comparisons.
// See http://qt.gitorious.org/maemo-6-ui-framework/libdui
// If not compiled in "meego-mode" should we be using locale compares?

using namespace KCalendarCore;

/**
 * How one QDateTime compares with another.
 *
 * If any all-day events are involved, comparison of QDateTime values
 * requires them to be considered as representing time periods. An all-day
 * instance represents a time period from 00:00:00 to 23:59:59.999 on a given
 * date, while a date/time instance can be considered to represent a time
 * period whose start and end times are the same. They may therefore be
 * earlier or later, or may overlap or be contained one within the other.
 *
 * Values may be OR'ed with each other in any combination of 'consecutive'
 * intervals to represent different types of relationship.
 *
 * In the descriptions of the values below,
 * - s1 = start time of first instance
 * - e1 = end time of first instance
 * - s2 = start time of second instance
 * - e2 = end time of second instance.
 */
enum DateTimeComparison {
    Before = 0x01, /**< The first QDateTime is strictly earlier than the second,
                    *   i.e. e1 < s2.
                    */
    AtStart = 0x02, /**< The first QDateTime starts at the same time as the second,
                     *   and ends before the end of the second,
                     *   i.e. s1 = s2, e1 < e2.
                     */
    Inside = 0x04, /**< The first QDateTime starts after the start of the second,
                    *   and ends before the end of the second,
                    *   i.e. s1 > s2, e1 < e2.
                    */
    AtEnd = 0x08, /**< The first QDateTime starts after the start of the second,
                   *   and ends at the same time as the second,
                   *   i.e. s1 > s2, e1 = e2.
                   */
    After = 0x10, /**< The first QDateTime is strictly later than the second,
                   *   i.e. s1 > e2.
                   */
    Equal = AtStart | Inside | AtEnd,
    /**< Simultaneous, i.e. s1 = s2 && e1 = e2.
     */
    Outside = Before | AtStart | Inside | AtEnd | After,
    /**< The first QDateTime starts before the start of the other,
     *   and ends after the end of the other,
     *   i.e. s1 < s2, e1 > e2.
     */
    StartsAt = AtStart | Inside | AtEnd | After,
    /**< The first QDateTime starts at the same time as the other,
     *   and ends after the end of the other,
     *   i.e. s1 = s2, e1 > e2.
     */
    EndsAt = Before | AtStart | Inside | AtEnd,
    /**< The first QDateTime starts before the start of the other,
     *   and ends at the same time as the other,
     *   i.e. s1 < s2, e1 = e2.
     */
};

/**
    * Compare two QDateTime instances to determine whether they are
    * simultaneous, earlier or later.

    * The comparison takes time zones into account: if the two instances have
    * different time zones, they are first converted to UTC before comparing.
    *
    * If both instances are not all-day values, the first instance is considered to
    * be either simultaneous, earlier or later, and does not overlap.
    *
    * If one instance is all-day and the other is a not all-day, the first instance
    * is either strictly earlier, strictly later, or overlaps.
    *
    * If both instance are all-day, they are considered simultaneous if both
    * their start of day and end of day times are simultaneous with each
    * other. (Both start and end of day times need to be considered in case a
    * daylight savings change occurs during that day.) Otherwise, the first instance
    * can be strictly earlier, earlier but overlapping, later but overlapping,
    * or strictly later.
    *
    * Note that if either instance is a local  time (Qt::TimeSpec of Qt::LocalTime),
    * the result cannot be guaranteed to be correct, since by definition they
    * contain no information about time zones or daylight savings changes.
    *
    * @return DateTimeComparison indicating the relationship of dt1 to dt2
    * @see operator==(), operator!=(), operator<(), operator<=(), operator>=(), operator>()
    */

DateTimeComparison compare(const QDateTime &dt1, bool isAllDay1, const QDateTime &dt2, bool isAllDay2)
{
    QDateTime start1, start2;
    // FIXME When secondOccurrence is available in QDateTime
    // const bool conv = (!d->equalSpec(*other.d) || d->secondOccurrence() != other.d->secondOccurrence());
    const bool conv = dt1.timeSpec() != dt2.timeSpec() || (dt1.timeSpec() == Qt::OffsetFromUTC && dt1.offsetFromUtc() != dt2.offsetFromUtc())
                      || (dt1.timeSpec() == Qt::TimeZone && dt1.timeZone() != dt2.timeZone());
    if (conv) {
        // Different time specs or one is a time which occurs twice,
        // so convert to UTC before comparing
        start1 = dt1.toUTC();
        start2 = dt2.toUTC();
    } else {
        // Same time specs, so no need to convert to UTC
        start1 = dt1;
        start2 = dt2;
    }
    if (isAllDay1 || isAllDay2) {
        // At least one of the instances is date-only, so we need to compare
        // time periods rather than just times.
        QDateTime end1, end2;
        if (conv) {
            if (isAllDay1) {
                QDateTime dt(dt1);
                dt.setTime(QTime(23, 59, 59, 999));
                end1 = dt.toUTC();
            } else {
                end1 = start1;
            }
            if (isAllDay2) {
                QDateTime dt(dt2);
                dt.setTime(QTime(23, 59, 59, 999));
                end2 = dt.toUTC();
            } else {
                end2 = start2;
            }
        } else {
            if (isAllDay1) {
                end1 = QDateTime(dt1.date(), QTime(23, 59, 59, 999), Qt::LocalTime);
            } else {
                end1 = dt1;
            }
            if (isAllDay2) {
                end2 = QDateTime(dt2.date(), QTime(23, 59, 59, 999), Qt::LocalTime);
            } else {
                end2 = dt2;
            }
        }

        if (start1 == start2) {
            return !isAllDay1 ? AtStart
                              : (end1 == end2) ? Equal
                                               : (end1 < end2) ? static_cast<DateTimeComparison>(AtStart | Inside)
                                                               : static_cast<DateTimeComparison>(AtStart | Inside | AtEnd | After);
        }

        if (start1 < start2) {
            return (end1 < start2) ? Before
                                   : (end1 == end2) ? static_cast<DateTimeComparison>(Before | AtStart | Inside | AtEnd)
                                                    : (end1 == start2) ? static_cast<DateTimeComparison>(Before | AtStart)
                                                                       : (end1 < end2) ? static_cast<DateTimeComparison>(Before | AtStart | Inside)
                                                                                       : Outside;
        } else {
            return (start1 > end2) ? After
                                   : (start1 == end2) ? (end1 == end2 ? AtEnd : static_cast<DateTimeComparison>(AtEnd | After))
                                                      : (end1 == end2) ? static_cast<DateTimeComparison>(Inside | AtEnd)
                                                                       : (end1 < end2) ? Inside
                                                                                       : static_cast<DateTimeComparison>(Inside | AtEnd | After);
        }
    }
    return (start1 == start2) ? Equal : (start1 < start2) ? Before : After;
}

bool KCalendarCore::Events::startDateLessThan(const Event::Ptr &e1, const Event::Ptr &e2)
{
    DateTimeComparison res = compare(e1->dtStart(), e1->allDay(), e2->dtStart(), e2->allDay());
    if (res == Equal) {
        return Events::summaryLessThan(e1, e2);
    } else {
        return (res & Before || res & AtStart);
    }
}

bool KCalendarCore::Events::startDateMoreThan(const Event::Ptr &e1, const Event::Ptr &e2)
{
    DateTimeComparison res = compare(e1->dtStart(), e1->allDay(), e2->dtStart(), e2->allDay());
    if (res == Equal) {
        return Events::summaryMoreThan(e1, e2);
    } else {
        return (res & After || res & AtEnd);
    }
}

bool KCalendarCore::Events::summaryLessThan(const Event::Ptr &e1, const Event::Ptr &e2)
{
    return QString::compare(e1->summary(), e2->summary(), Qt::CaseInsensitive) < 0;
}

bool KCalendarCore::Events::summaryMoreThan(const Event::Ptr &e1, const Event::Ptr &e2)
{
    return QString::compare(e1->summary(), e2->summary(), Qt::CaseInsensitive) > 0;
}

bool KCalendarCore::Events::endDateLessThan(const Event::Ptr &e1, const Event::Ptr &e2)
{
    DateTimeComparison res = compare(e1->dtEnd(), e1->allDay(), e2->dtEnd(), e2->allDay());
    if (res == Equal) {
        return Events::summaryLessThan(e1, e2);
    } else {
        return (res & Before || res & AtStart);
    }
}

bool KCalendarCore::Events::endDateMoreThan(const Event::Ptr &e1, const Event::Ptr &e2)
{
    DateTimeComparison res = compare(e1->dtEnd(), e1->allDay(), e2->dtEnd(), e2->allDay());
    if (res == Equal) {
        return Events::summaryMoreThan(e1, e2);
    } else {
        return (res & After || res & AtEnd);
    }
}

bool KCalendarCore::Journals::dateLessThan(const Journal::Ptr &j1, const Journal::Ptr &j2)
{
    DateTimeComparison res = compare(j1->dtStart(), j1->allDay(), j2->dtStart(), j2->allDay());
    return (res & Before || res & AtStart);
}

bool KCalendarCore::Journals::dateMoreThan(const Journal::Ptr &j1, const Journal::Ptr &j2)
{
    DateTimeComparison res = compare(j1->dtStart(), j1->allDay(), j2->dtStart(), j2->allDay());
    return (res & After || res & AtEnd);
}

bool KCalendarCore::Journals::summaryLessThan(const Journal::Ptr &j1, const Journal::Ptr &j2)
{
    return QString::compare(j1->summary(), j2->summary(), Qt::CaseInsensitive) < 0;
}

bool KCalendarCore::Journals::summaryMoreThan(const Journal::Ptr &j1, const Journal::Ptr &j2)
{
    return QString::compare(j1->summary(), j2->summary(), Qt::CaseInsensitive) > 0;
}

bool KCalendarCore::Todos::startDateLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    DateTimeComparison res = compare(t1->dtStart(), t1->allDay(), t2->dtStart(), t2->allDay());
    if (res == Equal) {
        return Todos::summaryLessThan(t1, t2);
    } else {
        return (res & Before || res & AtStart);
    }
}

bool KCalendarCore::Todos::startDateMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    DateTimeComparison res = compare(t1->dtStart(), t1->allDay(), t2->dtStart(), t2->allDay());
    if (res == Equal) {
        return Todos::summaryMoreThan(t1, t2);
    } else {
        return (res & After || res & AtEnd);
    }
}

bool KCalendarCore::Todos::dueDateLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    if (!t1->hasDueDate()) {
        return false;
    }
    if (!t2->hasDueDate()) {
        return true;
    }
    DateTimeComparison res = compare(t1->dtDue(), t1->allDay(), t2->dtDue(), t2->allDay());
    if (res == Equal) {
        return Todos::summaryLessThan(t1, t2);
    } else {
        return (res & Before || res & AtStart);
    }
}

bool KCalendarCore::Todos::dueDateMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    if (!t2->hasDueDate()) {
        return false;
    }
    if (!t1->hasDueDate()) {
        return true;
    }
    DateTimeComparison res = compare(t1->dtDue(), t1->allDay(), t2->dtDue(), t2->allDay());
    if (res == Equal) {
        return Todos::summaryMoreThan(t1, t2);
    } else {
        return (res & After || res & AtEnd);
    }
}

bool KCalendarCore::Todos::priorityLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    if (t1->priority() < t2->priority()) {
        return true;
    } else if (t1->priority() == t2->priority()) {
        return Todos::summaryLessThan(t1, t2);
    } else {
        return false;
    }
}

bool KCalendarCore::Todos::priorityMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    if (t1->priority() > t2->priority()) {
        return true;
    } else if (t1->priority() == t2->priority()) {
        return Todos::summaryMoreThan(t1, t2);
    } else {
        return false;
    }
}

bool KCalendarCore::Todos::percentLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    if (t1->percentComplete() < t2->percentComplete()) {
        return true;
    } else if (t1->percentComplete() == t2->percentComplete()) {
        return Todos::summaryLessThan(t1, t2);
    } else {
        return false;
    }
}

bool KCalendarCore::Todos::percentMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    if (t1->percentComplete() > t2->percentComplete()) {
        return true;
    } else if (t1->percentComplete() == t2->percentComplete()) {
        return Todos::summaryMoreThan(t1, t2);
    } else {
        return false;
    }
}

bool KCalendarCore::Todos::summaryLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    return QString::compare(t1->summary(), t2->summary(), Qt::CaseInsensitive) < 0;
}

bool KCalendarCore::Todos::summaryMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    return QString::compare(t1->summary(), t2->summary(), Qt::CaseInsensitive) > 0;
}

bool KCalendarCore::Todos::createdLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    DateTimeComparison res = compare(t1->created(), t1->allDay(), t2->created(), t2->allDay());
    if (res == Equal) {
        return Todos::summaryLessThan(t1, t2);
    } else {
        return (res & Before || res & AtStart);
    }
}

bool KCalendarCore::Todos::createdMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2)
{
    DateTimeComparison res = compare(t1->created(), t1->allDay(), t2->created(), t2->allDay());
    if (res == Equal) {
        return Todos::summaryMoreThan(t1, t2);
    } else {
        return (res & After || res & AtEnd);
    }
}

bool KCalendarCore::Incidences::dateLessThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2)
{
    DateTimeComparison res = compare(i1->dateTime(Incidence::RoleSort), i1->allDay(), i2->dateTime(Incidence::RoleSort), i2->allDay());
    if (res == Equal) {
        return Incidences::summaryLessThan(i1, i2);
    } else {
        return (res & Before || res & AtStart);
    }
}

bool KCalendarCore::Incidences::dateMoreThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2)
{
    DateTimeComparison res = compare(i1->dateTime(Incidence::RoleSort), i1->allDay(), i2->dateTime(Incidence::RoleSort), i2->allDay());
    if (res == Equal) {
        return Incidences::summaryMoreThan(i1, i2);
    } else {
        return (res & After || res & AtEnd);
    }
}

bool KCalendarCore::Incidences::createdLessThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2)
{
    DateTimeComparison res = compare(i1->created(), i1->allDay(), i2->created(), i2->allDay());
    if (res == Equal) {
        return Incidences::summaryLessThan(i1, i2);
    } else {
        return (res & Before || res & AtStart);
    }
}

bool KCalendarCore::Incidences::createdMoreThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2)
{
    DateTimeComparison res = compare(i1->created(), i1->allDay(), i2->created(), i2->allDay());
    if (res == Equal) {
        return Incidences::summaryMoreThan(i1, i2);
    } else {
        return (res & After || res & AtEnd);
    }
}

bool KCalendarCore::Incidences::summaryLessThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2)
{
    return QString::compare(i1->summary(), i2->summary(), Qt::CaseInsensitive) < 0;
}

bool KCalendarCore::Incidences::summaryMoreThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2)
{
    return QString::compare(i1->summary(), i2->summary(), Qt::CaseInsensitive) > 0;
}

bool KCalendarCore::Incidences::categoriesLessThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2)
{
    const auto res = QString::compare(i1->categoriesStr(), i2->categoriesStr(), Qt::CaseSensitive);
    if (res == 0) {
        return Incidences::summaryLessThan(i1, i2);
    } else {
        return res < 0;
    }
}

bool KCalendarCore::Incidences::categoriesMoreThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2)
{
    const auto res = QString::compare(i1->categoriesStr(), i2->categoriesStr(), Qt::CaseSensitive);
    if (res == 0) {
        return Incidences::summaryMoreThan(i1, i2);
    } else {
        return res > 0;
    }
}
