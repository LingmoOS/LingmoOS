/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtcalendar.h"

namespace
{
constexpr QCalendar::System toQCalendarSystem(CalendarSystem::System calendarSystem)
{
    switch (calendarSystem) {
    case CalendarSystem::System::Jalali:
        return QCalendar::System::Jalali;
    case CalendarSystem::System::IslamicCivil:
        return QCalendar::System::IslamicCivil;
    default:
        return static_cast<QCalendar::System>(calendarSystem);
    }
}
}

QtCalendarProvider::QtCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate, int dateOffset)
    : AbstractCalendarProvider(parent, calendarSystem, startDate, endDate, dateOffset)
    , m_calendar(QCalendar(toQCalendarSystem(calendarSystem)))
{
    Q_ASSERT(toQCalendarSystem(calendarSystem) <= QCalendar::System::Last);
}

QtCalendarProvider::~QtCalendarProvider()
{
}

QCalendar::YearMonthDay QtCalendarProvider::fromGregorian(const QDate &date) const
{
    if (!date.isValid()) {
        return {};
    }

    return m_calendar.partsFromDate(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel QtCalendarProvider::subLabel(const QDate &date) const
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (!date.isValid()) {
        return sublabel;
    }

    const QCalendar::YearMonthDay altDate = fromGregorian(date);
    sublabel.label = i18ndc("plasma_calendar_alternatecalendar",
                            "@label %1 day %2 month name %3 year",
                            "%1 %2, %3",
                            QString::number(altDate.day),
                            m_calendar.standaloneMonthName(QLocale::system(), altDate.month, altDate.year),
                            QString::number(altDate.year));

    return sublabel;
}
