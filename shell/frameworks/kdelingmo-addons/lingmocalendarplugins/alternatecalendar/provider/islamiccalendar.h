/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "abstractcalendarprovider.h"

/**
 * @brief An alternate calendar provider for Islamic/the Hijri calendar system.
 *
 * There are 3 kinds of Islamic calendars.
 * See https://cldr.unicode.org/development/development-process/design-proposals/islamic-calendar-types
 * for more details.
 */
class IslamicCalendarProvider : public AbstractCalendarProvider
{
    Q_OBJECT

public:
    explicit IslamicCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate, int dateOffset);
    ~IslamicCalendarProvider() override;

    QCalendar::YearMonthDay fromGregorian(const QDate &date) const override;
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date) const override;

private:
    const std::unique_ptr<class IslamicCalendarProviderPrivate> d;
};
