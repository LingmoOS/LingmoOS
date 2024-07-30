/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kholidaysdeclarativeplugin.h"
#include "holidayregionsmodel.h"

#include <KHolidays/LunarPhase>
#include <KHolidays/SunRiseSet>

#include <QCoreApplication>
#include <QQmlEngine>
#include <QTimeZone>

// convert to/from QDateTime for JS
class LunarPhaseWrapper
{
    Q_GADGET
public:
    static Q_INVOKABLE KHolidays::LunarPhase::Phase phaseAtDate(const QDateTime &date)
    {
        return KHolidays::LunarPhase::phaseAtDate(date.date());
    }
    static Q_INVOKABLE QString phaseNameAtDate(const QDateTime &date)
    {
        return KHolidays::LunarPhase::phaseNameAtDate(date.date());
    }
    static Q_INVOKABLE QString phaseName(KHolidays::LunarPhase::Phase phase)
    {
        return KHolidays::LunarPhase::phaseName(phase);
    }
};

class SunRiseSetWrapper
{
    Q_GADGET
public:
    static Q_INVOKABLE QDateTime utcSunrise(const QDateTime &date, double latitude, double longitude)
    {
        const auto time = KHolidays::SunRiseSet::utcSunrise(date.date(), latitude, longitude);
        return time.isValid() ? QDateTime(date.date(), time, QTimeZone::utc()) : QDateTime();
    }
    static Q_INVOKABLE QDateTime utcSunset(const QDateTime &date, double latitude, double longitude)
    {
        const auto time = KHolidays::SunRiseSet::utcSunset(date.date(), latitude, longitude);
        return time.isValid() ? QDateTime(date.date(), time, QTimeZone::utc()) : QDateTime();
    }
    static Q_INVOKABLE QDateTime utcDawn(const QDateTime &date, double latitude, double longitude)
    {
        const auto time = KHolidays::SunRiseSet::utcDawn(date.date(), latitude, longitude);
        return time.isValid() ? QDateTime(date.date(), time, QTimeZone::utc()) : QDateTime();
    }
    static Q_INVOKABLE QDateTime utcDusk(const QDateTime &date, double latitude, double longitude)
    {
        const auto time = KHolidays::SunRiseSet::utcDusk(date.date(), latitude, longitude);
        return time.isValid() ? QDateTime(date.date(), time, QTimeZone::utc()) : QDateTime();
    }
    static Q_INVOKABLE bool isPolarDay(const QDateTime &date, double latitude)
    {
        return KHolidays::SunRiseSet::isPolarDay(date.date(), latitude);
    }
    static Q_INVOKABLE bool isPolarTwilight(const QDateTime &date, double latitude)
    {
        return KHolidays::SunRiseSet::isPolarTwilight(date.date(), latitude);
    }
    static Q_INVOKABLE bool isPolarNight(const QDateTime &date, double latitude)
    {
        return KHolidays::SunRiseSet::isPolarNight(date.date(), latitude);
    }
};

void KHolidaysDeclarativePlugin::registerTypes(const char *uri)
{
    qmlRegisterType<HolidayRegionsDeclarativeModel>(uri, 1, 0, "HolidayRegionsModel");
    qRegisterMetaType<KHolidays::LunarPhase::Phase>();
    qmlRegisterUncreatableMetaObject(KHolidays::LunarPhase::staticMetaObject, uri, 1, 0, "LunarPhase", {});

    // HACK qmlplugindump chokes on gadget singletons, to the point of breaking ecm_find_qmlmodule()
    if (QCoreApplication::applicationName() != QLatin1String("qmlplugindump")) {
        qmlRegisterSingletonType(uri, 1, 0, "Lunar", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
            return engine->toScriptValue(LunarPhaseWrapper());
        });
        qmlRegisterSingletonType(uri, 1, 0, "SunRiseSet", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
            return engine->toScriptValue(SunRiseSetWrapper());
        });
    }
}

#include "kholidaysdeclarativeplugin.moc"
#include "moc_kholidaysdeclarativeplugin.cpp"
