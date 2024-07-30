/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDate>
#include <QHash>
#include <QObject>
#include <QRunnable>

#include <CalendarEvents/CalendarEventsPlugin>

#include "calendarsystem.h"

/**
 * @short An abstract base class for alternate calendar providers.
 *
 * This class serves as abstract base class for alternate calendar
 * provider implementations.
 */
class AbstractCalendarProvider : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit AbstractCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate, int dateOffset = 0);
    ~AbstractCalendarProvider() override;

    /**
     * Converts Gregorian date to alternate calendar date
     *
     * @param date Gregorian date
     * @return Alternate calendar date
     */
    virtual QCalendar::YearMonthDay fromGregorian(const QDate &date) const;

    /**
     * Gets the sub-labels for the specific Gregorian date
     *
     * @param date Gregorian date
     * @return A sublabel for the specific Gregorian date
     */
    virtual CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date) const;

    /**
     * Loads calendar data in date list @p dates with given offset @p dateOffset
     */
    void run() override;

Q_SIGNALS:
    void dataReady(const QHash<QDate, QCalendar::YearMonthDay> &alternateDatesData,
                   const QHash<QDate, CalendarEvents::CalendarEventsPlugin::SubLabel> &sublabelData);

protected:
    const CalendarSystem::System m_calendarSystem;
    const QDate m_startDate;
    const QDate m_endDate;
    const int m_dateOffset;
};
