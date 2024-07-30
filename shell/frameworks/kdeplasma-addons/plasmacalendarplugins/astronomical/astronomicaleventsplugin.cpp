/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "astronomicaleventsplugin.h"

// KF
#include <KConfigGroup>
#include <KHolidays/AstroSeasons>
#include <KHolidays/LunarPhase>
#include <KLocalizedString>
#include <KSharedConfig>
// Qt
#include <QDebug>

AstronomicalEventsPlugin::AstronomicalEventsPlugin()
    : CalendarEvents::CalendarEventsPlugin()
{
    auto config = KSharedConfig::openConfig(QStringLiteral("plasma_calendar_astronomicalevents"));
    const KConfigGroup generalConfig = config->group("General");

    m_lunarPhaseShown = generalConfig.readEntry("showLunarPhase", true);
    m_seasonShown = generalConfig.readEntry("showSeason", true);
}

AstronomicalEventsPlugin::~AstronomicalEventsPlugin()
{
}

void AstronomicalEventsPlugin::loadEventsForDateRange(const QDate &startDate, const QDate &endDate)
{
    QMultiHash<QDate, CalendarEvents::EventData> data;

    if (!endDate.isValid()) {
        Q_EMIT dataReady(data);
        return;
    }

    for (QDate date = startDate; date <= endDate && date.isValid(); date = date.addDays(1)) {
        if (m_lunarPhaseShown) {
            const auto phase = KHolidays::LunarPhase::phaseAtDate(date);
            if (phase == KHolidays::LunarPhase::NewMoon || phase == KHolidays::LunarPhase::FirstQuarter || phase == KHolidays::LunarPhase::LastQuarter
                || phase == KHolidays::LunarPhase::FullMoon) {
                CalendarEvents::EventData lunarPhaseData;
                lunarPhaseData.setIsAllDay(true);
                lunarPhaseData.setTitle(KHolidays::LunarPhase::phaseName(phase));
                lunarPhaseData.setEventType(CalendarEvents::EventData::Event);
                lunarPhaseData.setIsMinor(false);

                data.insert(date, lunarPhaseData);
            }
        }

        if (m_seasonShown) {
            const auto season = KHolidays::AstroSeasons::seasonAtDate(date);
            if (season != KHolidays::AstroSeasons::None) {
                CalendarEvents::EventData seasonData;
                seasonData.setIsAllDay(true);
                seasonData.setTitle(KHolidays::AstroSeasons::seasonName(season));
                seasonData.setEventType(CalendarEvents::EventData::Event);
                seasonData.setIsMinor(false);

                data.insert(date, seasonData);
            }
        }
    }

    Q_EMIT dataReady(data);
}
