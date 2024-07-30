/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "abstractcalendarprovider.h"

/**
 * @brief An alternate calendar provider for Hebrew/Jewish calendar system.
 */
class HebrewCalendarProvider : public AbstractCalendarProvider
{
    Q_OBJECT

public:
    explicit HebrewCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate);
    ~HebrewCalendarProvider() override;

    QCalendar::YearMonthDay fromGregorian(const QDate &date) const override;
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date) const override;

private:
    const std::unique_ptr<class HebrewCalendarProviderPrivate> d;
};
