/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ALTERNATECALENDARPLUGIN_H
#define ALTERNATECALENDARPLUGIN_H

#include <QDate>

#include <KConfigGroup>
#include <KConfigWatcher>

#include <CalendarEvents/CalendarEventsPlugin>

#include "calendarsystem.h"

class AbstractCalendarProvider;

class AlternateCalendarPlugin : public CalendarEvents::CalendarEventsPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.CalendarEventsPlugin" FILE "alternatecalendarplugin.json")
    Q_INTERFACES(CalendarEvents::CalendarEventsPlugin)

public:
    AlternateCalendarPlugin(QObject *parent = nullptr);
    ~AlternateCalendarPlugin() override;

    void loadEventsForDateRange(const QDate &startDate, const QDate &endDate) override;

public Q_SLOTS:
    void updateSettings(const KConfigGroup &configGroup);

private:
    void init();
    void emitDataChangedSignal();

    QDate m_lastStartDate;
    QDate m_lastEndDate;
    AbstractCalendarProvider *m_provider = nullptr;

    // Cache lookup data
    QHash<QDate, QCalendar::YearMonthDay> m_alternateDateCache;
    QHash<QDate, SubLabel> m_sublabelCache;

    // For updating config
    KConfigGroup m_generalConfigGroup;
    KConfigWatcher::Ptr m_configWatcher;

    CalendarSystem::System m_calendarSystem = CalendarSystem::Gregorian;
    int m_dateOffset = 0; // For the (tabular) Islamic Civil calendar
};

#endif
