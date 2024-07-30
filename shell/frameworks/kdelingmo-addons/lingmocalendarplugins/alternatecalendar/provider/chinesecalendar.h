/*
    SPDX-FileCopyrightText: 2021 Gary Wang <wzc782970009@gmail.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "abstractcalendarprovider.h"

/**
 * @short An alternate calendar provider for Chinese calendar system.
 *
 * This class presents an alternate calendar provider for Chinese calendar system.
 */
class ChineseCalendarProvider : public AbstractCalendarProvider
{
    Q_OBJECT

public:
    explicit ChineseCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate);
    ~ChineseCalendarProvider() override;

    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date) const override;

private:
    const std::unique_ptr<class ChineseCalendarProviderPrivate> d;
};
