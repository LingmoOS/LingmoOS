/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the OccurrenceIterator class.

  @author Christian Mollekopf \<mollekopf@kolabsys.com\>
 */

#ifndef KCALCORE_OCCURRENCEITERATOR_H
#define KCALCORE_OCCURRENCEITERATOR_H

#include "incidence.h"

namespace KCalendarCore {
class Calendar;
/**
 * Iterate over calendar items in a calendar.
 *
 * The iterator takes recurrences and exceptions to recurrences into account
 *
 * The iterator does not iterate the occurrences of all incidences chronologically.
 * @since 4.11
 */
class Q_CORE_EXPORT OccurrenceIterator
{
public:
    /**
     * Creates iterator that iterates over all occurrences of all incidences
     * between @param start and @param end (inclusive)
     */
    explicit OccurrenceIterator(const Calendar &calendar, const QDateTime &start = QDateTime(), const QDateTime &end = QDateTime());

    /**
     * Creates iterator that iterates over all occurrences
     * of @param incidence between @param start and @param end (inclusive)
     */
    OccurrenceIterator(const Calendar &calendar,
                       const KCalendarCore::Incidence::Ptr &incidence,
                       const QDateTime &start = QDateTime(),
                       const QDateTime &end = QDateTime());
    ~OccurrenceIterator();
    bool hasNext() const;

    /**
     * Advance iterator to the next occurrence.
     */
    void next();

    /**
     * Returns either main incidence or exception, depending on occurrence.
     */
    Incidence::Ptr incidence() const;

    /**
     * Returns the start date of the occurrence
     *
     * This is either the occurrence date, or the start date of an exception
     * which overrides that occurrence.
     */
    QDateTime occurrenceStartDate() const;

    /**
     * Returns the recurrence Id.
     *
     * This is the date where the occurrence starts without exceptions,
     * this id is used to identify one exact occurrence.
     */
    QDateTime recurrenceId() const;

private:
    Q_DISABLE_COPY(OccurrenceIterator)
    //@cond PRIVATE
    class Private;
    QScopedPointer<Private> d;
    //@endcond
};

} // namespace KCalendarCore

#endif
