/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config-ICU.h"

#include "alternatecalendarplugin.h"

#include <QThreadPool>

#include <KSharedConfig>

#include "provider/qtcalendar.h"
#if HAVE_ICU
#include "provider/chinesecalendar.h"
#include "provider/hebrewcalendar.h"
#include "provider/indiancalendar.h"
#include "provider/islamiccalendar.h"
#endif

using namespace Qt::StringLiterals;
using SubLabel = CalendarEvents::CalendarEventsPlugin::SubLabel;

AlternateCalendarPlugin::AlternateCalendarPlugin(QObject *parent)
    : CalendarEvents::CalendarEventsPlugin(parent)
{
    auto config = KSharedConfig::openConfig("plasma_calendar_alternatecalendar"_L1, KConfig::NoGlobals);
    m_generalConfigGroup = config->group("General");
    m_configWatcher = KConfigWatcher::create(config);
    connect(m_configWatcher.get(), &KConfigWatcher::configChanged, this, &AlternateCalendarPlugin::updateSettings);

    init();
}

AlternateCalendarPlugin::~AlternateCalendarPlugin()
{
}

void AlternateCalendarPlugin::loadEventsForDateRange(const QDate &startDate, const QDate &endDate)
{
    if (!endDate.isValid() || m_calendarSystem == CalendarSystem::Gregorian) {
        return;
    }

    if (m_lastStartDate == startDate && m_lastEndDate == endDate) {
        emitDataChangedSignal();
        return;
    }

    switch (m_calendarSystem) {
#if HAVE_ICU
    case CalendarSystem::Chinese:
        m_provider = new ChineseCalendarProvider(this, m_calendarSystem, startDate, endDate);
        break;
    case CalendarSystem::Indian:
        m_provider = new IndianCalendarProvider(this, m_calendarSystem, startDate, endDate);
        break;
    case CalendarSystem::Hebrew:
        m_provider = new HebrewCalendarProvider(this, m_calendarSystem, startDate, endDate);
        break;
    case CalendarSystem::Jalali:
    case CalendarSystem::Islamic:
    case CalendarSystem::IslamicCivil:
    case CalendarSystem::IslamicUmalqura:
        m_provider = new IslamicCalendarProvider(this, m_calendarSystem, startDate, endDate, m_dateOffset);
        break;
#else
    // Fall back to QtCalendar
    case CalendarSystem::Jalali:
    case CalendarSystem::IslamicCivil:
#endif
    case CalendarSystem::Julian:
    case CalendarSystem::Milankovic:
        m_provider = new QtCalendarProvider(this, m_calendarSystem, startDate, endDate, m_dateOffset);
        break;
    default:
        m_provider = new AbstractCalendarProvider(this, m_calendarSystem, startDate, endDate, m_dateOffset);
    }
    connect(m_provider,
            &AbstractCalendarProvider::dataReady,
            this,
            [this, startDate, endDate](const QHash<QDate, QCalendar::YearMonthDay> &alternateDatesData,
                                       const QHash<QDate, CalendarEvents::CalendarEventsPlugin::SubLabel> &sublabelData) {
                // Check if the sender is the latest
                if (m_provider == sender()) {
                    m_alternateDateCache = alternateDatesData;
                    m_sublabelCache = sublabelData;
                    m_lastStartDate = startDate;
                    m_lastEndDate = endDate;
                    emitDataChangedSignal();
                    m_provider = nullptr;
                }
                delete sender();
            });

    m_provider->setAutoDelete(false);
    QThreadPool::globalInstance()->start(m_provider);
}

void AlternateCalendarPlugin::updateSettings(const KConfigGroup &configGroup)
{
    if (configGroup.config()->name() != "plasma_calendar_alternatecalendar"_L1) {
        return;
    }
    init();
    loadEventsForDateRange(m_lastStartDate, m_lastEndDate);
}

void AlternateCalendarPlugin::init()
{
    m_dateOffset = m_generalConfigGroup.readEntry("dateOffset", 0);

    // Find the matched calendar system
    const QString system = m_generalConfigGroup.readEntry("calendarSystem", "Julian");
    const auto systemIt = s_calendarMap.find(system);

    if (systemIt == s_calendarMap.end()) {
        // Invalid config, fall back to Gregorian
        m_calendarSystem = CalendarSystem::Gregorian;
    } else {
        m_calendarSystem = systemIt->second.system;
    }

    // Clear the old cache when config is reloaded
    m_lastStartDate = {};
    m_lastEndDate = {};
    m_alternateDateCache.clear();
    m_sublabelCache.clear();
}

void AlternateCalendarPlugin::emitDataChangedSignal()
{
    if (!m_alternateDateCache.empty()) {
        Q_EMIT alternateCalendarDateReady(m_alternateDateCache);
    }
    Q_EMIT subLabelReady(m_sublabelCache);
}

#include "moc_alternatecalendarplugin.cpp"