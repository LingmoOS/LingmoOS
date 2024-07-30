/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "abstractcalendarprovider.h"

AbstractCalendarProvider::AbstractCalendarProvider(QObject *parent,
                                                   CalendarSystem::System calendarSystem,
                                                   const QDate &startDate,
                                                   const QDate &endDate,
                                                   int dateOffset)
    : QObject(parent)
    , m_calendarSystem(calendarSystem)
    , m_startDate(startDate)
    , m_endDate(endDate)
    , m_dateOffset(dateOffset)
{
}

AbstractCalendarProvider::~AbstractCalendarProvider()
{
}

QCalendar::YearMonthDay AbstractCalendarProvider::fromGregorian(const QDate &date) const
{
    if (!date.isValid()) {
        return {};
    }

    return QCalendar::YearMonthDay(date.year(), date.month(), date.day());
}

CalendarEvents::CalendarEventsPlugin::SubLabel AbstractCalendarProvider::subLabel([[maybe_unused]] const QDate &date) const
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}

void AbstractCalendarProvider::run()
{
    static_assert(std::is_trivially_copy_assignable_v<QDate>);
    static_assert(std::is_trivially_move_assignable_v<QCalendar::YearMonthDay>);

    QHash<QDate, QCalendar::YearMonthDay> alternateDatesData;
    QHash<QDate, CalendarEvents::CalendarEventsPlugin::SubLabel> sublabelData;
    for (QDate date = m_startDate; date <= m_endDate && date.isValid(); date = date.addDays(1)) {
        const QDate offsetDate = date.addDays(m_dateOffset);
        const QCalendar::YearMonthDay alt = fromGregorian(offsetDate);
        alternateDatesData.emplace(date, alt.year, alt.month, alt.day);
        sublabelData.emplace(date, subLabel(offsetDate));
    }

    Q_EMIT dataReady(alternateDatesData, sublabelData);
}

#include "moc_abstractcalendarprovider.cpp"