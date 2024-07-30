/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <map>

#include <QCalendar>
#include <QMetaType>

#include <KLocalizedString>

namespace CalendarSystem
{
Q_NAMESPACE

/**
 * @note When adding a new calendar system, \s_calendarMap should also be updated.
 */
enum System : int {
    Gregorian = static_cast<int>(QCalendar::System::Gregorian),
#ifndef QT_BOOTSTRAPPED
    Julian = static_cast<int>(QCalendar::System::Julian),
    Milankovic = static_cast<int>(QCalendar::System::Milankovic),
#endif
    Chinese = static_cast<int>(QCalendar::System::Last) + 1, // QTBUG-89824: QCalendar doesn't support Chinese calendar system
    Indian, // QCalendar doesn't support India calendar system
    Hebrew,
    Jalali, // Persian
    Islamic, // Astronomical
    IslamicCivil,
    IslamicUmalqura,
};
Q_ENUM_NS(System)
};

struct CalendarSystemItem {
    const CalendarSystem::System system;
    const QString id;
    const QString text;
};

// clang-format off
static const std::map<QString /* id */, CalendarSystemItem> s_calendarMap{
    {
        QStringLiteral("Julian"),
        {CalendarSystem::Julian, QStringLiteral("Julian"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Julian")}
    },
    {
        QStringLiteral("Milankovic"),
        {CalendarSystem::Milankovic, QStringLiteral("Milankovic"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Milankovic")}
    },
    {
        QStringLiteral("Jalali"),
        {CalendarSystem::Jalali, QStringLiteral("Jalali"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "The Solar Hijri Calendar (Persian)")}
    },
    {
        QStringLiteral("Islamic"),
        {CalendarSystem::Islamic, QStringLiteral("Islamic"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist See https://cldr.unicode.org/development/development-process/design-proposals/islamic-calendar-types for more details", "Islamic Calendar (Astronomical)")}
    },
    {
        QStringLiteral("IslamicCivil"),
        {CalendarSystem::IslamicCivil, QStringLiteral("IslamicCivil"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist See https://cldr.unicode.org/development/development-process/design-proposals/islamic-calendar-types for more details", "The Islamic Civil Calendar (Tabular)")}
    },
    {
        QStringLiteral("IslamicUmalqura"),
        {CalendarSystem::IslamicUmalqura, QStringLiteral("IslamicUmalqura"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist See https://cldr.unicode.org/development/development-process/design-proposals/islamic-calendar-types for more details", "Islamic Calendar (Umm al-Qura)")}
    },
    {
        QStringLiteral("Chinese"),
        {CalendarSystem::Chinese, QStringLiteral("Chinese"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Chinese Lunar Calendar")}
    },
    {
        QStringLiteral("Indian"),
        {CalendarSystem::Indian, QStringLiteral("Indian"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Indian National Calendar")}
    },
    {
        QStringLiteral("Hebrew"),
        {CalendarSystem::Hebrew, QStringLiteral("Hebrew"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Hebrew Calendar")}
    },
};
// clang-format on
