/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2004, 2007, 2009 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lunarphase.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QTimeZone>

#include <numbers>

using namespace KHolidays;

QString LunarPhase::phaseNameAtDate(const QDate &date)
{
    return phaseName(phaseAtDate(date));
}

QString LunarPhase::phaseName(LunarPhase::Phase phase)
{
    switch (phase) {
    case NewMoon:
        return (QCoreApplication::translate("LunarPhase", "New Moon"));
    case FullMoon:
        return (QCoreApplication::translate("LunarPhase", "Full Moon"));
    case FirstQuarter:
        return (QCoreApplication::translate("LunarPhase", "First Quarter Moon"));
    case LastQuarter:
        return (QCoreApplication::translate("LunarPhase", "Last Quarter Moon"));
    case None:
        return QString();
    case WaxingCrescent:
        return (QCoreApplication::translate("LunarPhase", "Waxing Crescent"));
    case WaxingGibbous:
        return (QCoreApplication::translate("LunarPhase", "Waxing Gibbous"));
    case WaningGibbous:
        return (QCoreApplication::translate("LunarPhase", "Waning Gibbous"));
    case WaningCrescent:
        return (QCoreApplication::translate("LunarPhase", "Waning Crescent"));
    }
    return QString();
}

static double phaseAngle(int64_t unixmsec);

LunarPhase::Phase LunarPhase::phaseAtDate(const QDate &date)
{
    Phase retPhase = None;

    const QTime midnight(0, 0, 0);
    const QDateTime todayStart(date, midnight, QTimeZone::utc());
    const double startAngle = phaseAngle(todayStart.toMSecsSinceEpoch());
    const QDateTime todayEnd(date.addDays(1), midnight, QTimeZone::utc());
    const double endAngle = phaseAngle(todayEnd.toMSecsSinceEpoch());

    if (startAngle > endAngle) {
        retPhase = NewMoon;
    } else if (startAngle < 90.0 && endAngle > 90.0) {
        retPhase = FirstQuarter;
    } else if (startAngle < 180.0 && endAngle > 180.0) {
        retPhase = FullMoon;
    } else if (startAngle < 270.0 && endAngle > 270.0) {
        retPhase = LastQuarter;
    } else if (endAngle < 90.0) {
        retPhase = WaxingCrescent;
    } else if (endAngle < 180.0) {
        retPhase = WaxingGibbous;
    } else if (endAngle < 270.0) {
        retPhase = WaningGibbous;
    } else if (endAngle < 360.0) {
        retPhase = WaningCrescent;
    }

    return retPhase;
}

/*
    Algorithm adapted from Moontool by John Walker.
    Moontool is in the public domain: "Do what thou wilt shall be the whole of the law".
    Unnecessary calculations have been removed.
    See https://fourmilab.ch/moontool .
*/

#include <cmath>

constexpr int64_t epoch = 315446400000; // "1980 January 0.0", a.k.a. midnight on 1979-12-31, in ms Unix time
constexpr double elonge = 278.833540; // ecliptic longitude of sun at epoch
constexpr double elongp = 282.596403; // ecliptic longitude of sun at perigee
constexpr double earthEcc = 0.016718; // eccentricity of earth orbit
static const double ecPrefactor = sqrt((1 + earthEcc) / (1 - earthEcc));

constexpr double mmlong = 64.975464; // mean longitude of moon at epoch
constexpr double mmlongp = 349.383063; // mean longitude of moon at perigee

static double fixAngle(double degrees)
{
    return degrees - floor(degrees / 360.0) * 360.0;
}

static constexpr double radToDeg(double rad)
{
    return rad / std::numbers::pi * 180.0;
}

static constexpr double degToRad(double deg)
{
    return deg / 180.0 * std::numbers::pi;
}

constexpr double epsilon = 1e-6;

static double kepler(double m, double ecc)
{
    double mrad = degToRad(m);
    double e = mrad;
    double delta;
    do {
        delta = e - ecc * sin(e) - mrad;
        e -= delta / (1 - ecc * cos(e));
    } while (abs(delta) > epsilon);
    return e;
}

static double phaseAngle(int64_t unixmsec)
{
    int64_t msec = unixmsec - epoch;

    double sunMeanAnomaly = fixAngle(msec * (360.0 / 365.2422 / 86400000.0) + elonge - elongp);
    double trueAnomaly = 2 * radToDeg(atan(ecPrefactor * tan(kepler(sunMeanAnomaly, earthEcc) / 2)));
    double geocentricEclipticLong = fixAngle(trueAnomaly + elongp);

    double moonMeanLong = fixAngle(msec * (13.1763966 / 86400000.0) + mmlong);
    double moonMeanAnomaly = fixAngle(moonMeanLong - msec * (0.1114041 / 86400000.0) - mmlongp);
    double evection = 1.2739 * sin(degToRad(2 * (moonMeanLong - geocentricEclipticLong) - moonMeanAnomaly));
    double annualEquation = 0.1858 * sin(degToRad(sunMeanAnomaly));
    double a3 = 0.37 * sin(degToRad(sunMeanAnomaly));
    double correctedAnomaly = moonMeanAnomaly + evection - annualEquation - a3;
    double centreCorrection = 6.2886 * sin(degToRad(correctedAnomaly));
    double a4 = 0.214 * sin(degToRad(2 * correctedAnomaly));
    double correctedLong = moonMeanLong + evection + centreCorrection - annualEquation + a4;
    double variation = 0.6583 * sin(degToRad(2 * (correctedLong - geocentricEclipticLong)));
    double trueLong = correctedLong + variation;

    return fixAngle(trueLong - geocentricEclipticLong);
}

#include "moc_lunarphase.cpp"
