/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASTRONOMICALEVENTSPLUGIN_H
#define ASTRONOMICALEVENTSPLUGIN_H

// KF
#include <CalendarEvents/CalendarEventsPlugin>

class AstronomicalEventsPlugin : public CalendarEvents::CalendarEventsPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.CalendarEventsPlugin" FILE "astronomicaleventsplugin.json")
    Q_INTERFACES(CalendarEvents::CalendarEventsPlugin)

public:
    AstronomicalEventsPlugin();
    ~AstronomicalEventsPlugin() override;

    void loadEventsForDateRange(const QDate &startDate, const QDate &endDate) override;

private:
    bool m_lunarPhaseShown;
    bool m_seasonShown;
};

#endif
