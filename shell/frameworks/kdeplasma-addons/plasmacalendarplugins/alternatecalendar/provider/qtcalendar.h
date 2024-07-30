/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "abstractcalendarprovider.h"

/**
 * @short An alternate calendar provider for the calendar system available in QCalendar.
 *
 * This class presents an alternate calendar provider for the calendar system available
 * in QCalendar.
 * The available calendar systems are listed in QCalendar::availableCalendars().
 */
class QtCalendarProvider : public AbstractCalendarProvider
{
    Q_OBJECT

public:
    explicit QtCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate, int dateOffset = 0);
    ~QtCalendarProvider() override;

    QCalendar::YearMonthDay fromGregorian(const QDate &date) const override;
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date) const override;

private:
    const QCalendar m_calendar;
};
