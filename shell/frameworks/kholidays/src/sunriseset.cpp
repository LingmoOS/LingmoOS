/*
    This file is part of the kholidays library.

    Adapted from the Javascript found at https://www.esrl.noaa.gov/gmd/grad/solcalc
    which is in the public domain.

    SPDX-FileCopyrightText: 2012 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sunriseset.h"

#include <cmath>
#include <numbers>

static const double MaxLat = 89.99;
static const double MaxLong = 179.99;

using namespace KHolidays;
using namespace SunRiseSet;

static double degToRad(double degree)
{
    return (degree * std::numbers::pi) / 180.00;
}

static double radToDeg(double rad)
{
    return (rad * 180.0) / std::numbers::pi;
}

// convert Julian Day to centuries since J2000.0.
static double calcTimeJulianCent(int jday)
{
    return (double(jday) - 2451545.0) / 36525.0;
}

// calculate the mean obliquity of the ecliptic (in degrees)
static double calcMeanObliquityOfEcliptic(double t)
{
    double seconds = 21.448 - t * (46.8150 + t * (0.00059 - t * 0.001813));
    double e0 = 23.0 + (26.0 + (seconds / 60.0)) / 60.0;
    return e0; // in degrees
}

// calculate the corrected obliquity of the ecliptic (in degrees)
static double calcObliquityCorrection(double t)
{
    double e0 = calcMeanObliquityOfEcliptic(t);
    double omega = 125.04 - 1934.136 * t;
    double e = e0 + 0.00256 * cos(degToRad(omega));
    return e; // in degrees
}

// calculate the Geometric Mean Longitude of the Sun (in degrees)
static double calcGeomMeanLongSun(double t)
{
    double L0 = 280.46646 + t * (36000.76983 + 0.0003032 * t);
    while (L0 > 360.0) {
        L0 -= 360.0;
    }
    while (L0 < 0.0) {
        L0 += 360.0;
    }
    return L0; // in degrees
}

// calculate the Geometric Mean Anomaly of the Sun (in degrees)
static double calcGeomMeanAnomalySun(double t)
{
    double M = 357.52911 + t * (35999.05029 - 0.0001537 * t);
    return M; // in degrees
}

// calculate the equation of center for the sun (in degrees)
static double calcSunEqOfCenter(double t)
{
    double m = calcGeomMeanAnomalySun(t);
    double mrad = degToRad(m);
    double sinm = sin(mrad);
    double sin2m = sin(mrad + mrad);
    double sin3m = sin(mrad + mrad + mrad);
    double C = (sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) //
                + sin2m * (0.019993 - 0.000101 * t) //
                + sin3m * 0.000289);
    return C; // in degrees
}

// calculate the true longitude of the sun (in degrees)
static double calcSunTrueLong(double t)
{
    double l0 = calcGeomMeanLongSun(t);
    double c = calcSunEqOfCenter(t);
    double O = l0 + c;
    return O; // in degrees
}

// calculate the apparent longitude of the sun (in degrees)
static double calcSunApparentLong(double t)
{
    double o = calcSunTrueLong(t);
    double omega = 125.04 - 1934.136 * t;
    double lambda = o - 0.00569 - 0.00478 * sin(degToRad(omega));
    return lambda; // in degrees
}

// calculate the declination of the sun (in degrees)
static double calcSunDeclination(double t)
{
    double e = calcObliquityCorrection(t);
    double lambda = calcSunApparentLong(t);

    double sint = sin(degToRad(e)) * sin(degToRad(lambda));
    double theta = radToDeg(asin(sint));
    return theta; // in degrees
}

// calculate the eccentricity of earth's orbit (unitless)
static double calcEccentricityEarthOrbit(double t)
{
    double e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
    return e; // unitless
}

// calculate the difference between true solar time and mean solar time
//(output: equation of time in minutes of time)
static double calcEquationOfTime(double t)
{
    double epsilon = calcObliquityCorrection(t);
    double l0 = calcGeomMeanLongSun(t);
    double e = calcEccentricityEarthOrbit(t);
    double m = calcGeomMeanAnomalySun(t);

    double y = tan(degToRad(epsilon) / 2.0);
    y *= y;

    double sin2l0 = sin(2.0 * degToRad(l0));
    double sinm = sin(degToRad(m));
    double cos2l0 = cos(2.0 * degToRad(l0));
    double sin4l0 = sin(4.0 * degToRad(l0));
    double sin2m = sin(2.0 * degToRad(m));

    /* clang-format off */
    double Etime = (y * sin2l0
                    - 2.0 * e * sinm
                    + 4.0 * e * y * sinm * cos2l0
                    - 0.5 * y * y * sin4l0
                    - 1.25 * e * e * sin2m);
    /* clang-format on */
    return radToDeg(Etime) * 4.0; // in minutes of time
}

// sun positions (in degree relative to the horizon) for certain events
static constexpr const double Sunrise = -0.833; // see https://en.wikipedia.org/wiki/Sunrise
static constexpr const double CivilTwilight = -6.0; // see https://en.wikipedia.org/wiki/Twilight

static constexpr const double Up = 1.0;
static constexpr const double Down = -1.0;

// calculate the hour angle of the sun at angle @p sunHeight for the latitude (in radians)
static double calcHourAngleSunrise(double latitude, double solarDec, double sunHeight)
{
    double latRad = degToRad(latitude);
    double sdRad = degToRad(solarDec);
    double HAarg = (cos(degToRad(90.0 - sunHeight)) / (cos(latRad) * cos(sdRad)) - tan(latRad) * tan(sdRad));
    double HA = acos(HAarg);
    return HA; // in radians (for sunset, use -HA)
}

static QTime calcSunEvent(const QDate &date, double latitude, double longitude, double sunHeight, double direction)
{
    latitude = qMax(qMin(MaxLat, latitude), -MaxLat);
    longitude = qMax(qMin(MaxLong, longitude), -MaxLong);

    double t = calcTimeJulianCent(date.toJulianDay());
    double eqTime = calcEquationOfTime(t);
    double solarDec = calcSunDeclination(t);
    double hourAngle = direction * calcHourAngleSunrise(latitude, solarDec, sunHeight);
    double delta = longitude + radToDeg(hourAngle);
    if (std::isnan(delta)) {
        return {};
    }
    QTime timeUTC(0, 0);
    timeUTC = timeUTC.addSecs((720 - (4.0 * delta) - eqTime) * 60);

    // round to the nearest minute
    if (timeUTC.second() > 29) {
        return QTime(timeUTC.hour(), timeUTC.minute()).addSecs(60);
    }
    return QTime(timeUTC.hour(), timeUTC.minute());
}

QTime KHolidays::SunRiseSet::utcSunrise(const QDate &date, double latitude, double longitude)
{
    return calcSunEvent(date, latitude, longitude, Sunrise, Up);
}

QTime KHolidays::SunRiseSet::utcSunset(const QDate &date, double latitude, double longitude)
{
    return calcSunEvent(date, latitude, longitude, Sunrise, Down);
}

QTime SunRiseSet::utcDawn(const QDate &date, double latitude, double longitude)
{
    return calcSunEvent(date, latitude, longitude, CivilTwilight, Up);
}

QTime SunRiseSet::utcDusk(const QDate &date, double latitude, double longitude)
{
    return calcSunEvent(date, latitude, longitude, CivilTwilight, Down);
}

// see https://en.wikipedia.org/wiki/Solar_zenith_angle
bool SunRiseSet::isPolarDay(const QDate &date, double latitude)
{
    const double t = calcTimeJulianCent(date.toJulianDay());
    const double solarDec = calcSunDeclination(t);
    const double maxSolarZenithAngle = 180.0 - std::abs(latitude + solarDec);

    return maxSolarZenithAngle <= 90.0 - Sunrise;
}

bool SunRiseSet::isPolarTwilight(const QDate &date, double latitude)
{
    const double t = calcTimeJulianCent(date.toJulianDay());
    const double solarDec = calcSunDeclination(t);
    const double minSolarZenithAngle = std::abs(latitude - solarDec);

    return minSolarZenithAngle > 90.0 - Sunrise && minSolarZenithAngle <= 90.0 - CivilTwilight;
}

bool SunRiseSet::isPolarNight(const QDate &date, double latitude)
{
    const double t = calcTimeJulianCent(date.toJulianDay());
    const double solarDec = calcSunDeclination(t);
    const double minSolarZenithAngle = std::abs(latitude - solarDec);

    return minSolarZenithAngle > 90.0 - CivilTwilight;
}
