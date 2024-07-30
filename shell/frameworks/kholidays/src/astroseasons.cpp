/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2004, 2006-2007 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "astroseasons.h"

#include <cmath>
#include <numeric>

#include <QCoreApplication>
#include <QDate>

using namespace KHolidays;

/* Correction tables and calculation algorithms below are based on the book
 * Astronomical Algorithms by Jean Meeus, (c) 1991 by Willman-Bell, Inc.,
 *
 * The correction tables are precise for years -1000 to +3000, but according
 * to the book, they can be used for several centuries before and after that
 * period with the error still being quite small. As we only care about date
 * and not the time, the precision of the algorithm is good enough even for
 * a greater time span, therefore no checks for the given year are performed
 * anywhere in the code.
 */
namespace
{
double meanJDE(AstroSeasons::Season season, int year)
{
    if (year <= 1000) {
        // Astronomical Algorithms, Jean Meeus, chapter 26, table 26.A
        // mean season Julian dates for years -1000 to 1000
        const double y = year / 1000.0;
        switch (season) {
        case AstroSeasons::MarchEquinox:
            return 1721139.29189 + (365242.13740 * y) + (0.06134 * std::pow(y, 2)) + (0.00111 * std::pow(y, 3)) - (0.00071 * std::pow(y, 4));
        case AstroSeasons::JuneSolstice:
            return 1721233.25401 + (365241.72562 * y) - (0.05323 * std::pow(y, 2)) + (0.00907 * std::pow(y, 3)) + (0.00025 * std::pow(y, 4));
        case AstroSeasons::SeptemberEquinox:
            return 1721325.70455 + (365242.49558 * y) - (0.11677 * std::pow(y, 2)) - (0.00297 * std::pow(y, 3)) + (0.00074 * std::pow(y, 4));
        case AstroSeasons::DecemberSolstice:
            return 1721414.39987 + (365242.88257 * y) - (0.00769 * std::pow(y, 2)) - (0.00933 * std::pow(y, 3)) - (0.00006 * std::pow(y, 4));
        case AstroSeasons::None:
            Q_ASSERT(false);
            return 0;
        }
    } else {
        // Astronomical Algorithms, Jean Meeus, chapter 26, table 26.B
        // mean season Julian dates for years 1000 to 3000
        const double y = (year - 2000) / 1000.0;
        switch (season) {
        case AstroSeasons::MarchEquinox:
            return 2451623.80984 + (365242.37404 * y) + (0.05169 * std::pow(y, 2)) - (0.00411 * std::pow(y, 3)) - (0.00057 * std::pow(y, 4));
        case AstroSeasons::JuneSolstice:
            return 2451716.56767 + (365241.62603 * y) + (0.00325 * std::pow(y, 2)) + (0.00888 * std::pow(y, 3)) - (0.00030 * std::pow(y, 4));
        case AstroSeasons::SeptemberEquinox:
            return 2451810.21715 + (365242.01767 * y) - (0.11575 * std::pow(y, 2)) + (0.00337 * std::pow(y, 3)) + (0.00078 * std::pow(y, 4));
        case AstroSeasons::DecemberSolstice:
            return 2451900.05952 + (365242.74049 * y) - (0.06223 * std::pow(y, 2)) - (0.00823 * std::pow(y, 3)) + (0.00032 * std::pow(y, 4));
        case AstroSeasons::None:
            Q_ASSERT(false);
            return 0;
        }
    }

    return 0;
}

double periodicTerms(double t)
{
    // Astronomical Algorithms, Jean Meeus, chapter 26, table 26.C
    // The table gives the periodic terms in degrees, but the values are converted to radians
    // at compile time so that they can be passed to std::cos()
    struct Periodic {
        constexpr Periodic(int a, double b_deg, double c_deg)
            : a(a)
            , b_rad(b_deg * (M_PI / 180.0))
            , c_rad(c_deg * (M_PI / 180.0))
        {
        }

        int a;
        double b_rad;
        double c_rad;
    }
    // clang-format off
    periodic[] = {
        {485, 324.96,  1934.136}, {203, 337.23, 32964.467}, {199, 342.08,    20.186}, {182,  27.85, 445267.112},
        {156,  73.14, 45036.886}, {136, 171.52, 22518.443}, { 77, 222.54, 65928.934}, { 74, 296.72,   3034.906},
        { 70, 243.58,  9037.513}, { 58, 119.81, 33718.147}, { 52, 297.17,   150.678}, { 50,  21.02,   2281.226},
        { 45, 247.54, 29929.562}, { 44, 325.15, 31555.956}, { 29,  60.93,  4443.417}, { 18, 155.12,  67555.328},
        { 17, 288.79,  4562.452}, { 16, 198.04, 62894.029}, { 14, 199.76, 31436.921}, { 12,  95.39,  14577.848},
        { 12, 287.11, 31931.756}, { 12, 320.81, 34777.259}, {  9, 227.73,  1222.114}, {  8,  15.45,  16859.074}
    };
    // clang-format on

    return std::accumulate(std::begin(periodic), std::end(periodic), 0.0, [t](double s, const Periodic &p) {
        return s + p.a * std::cos(p.b_rad + p.c_rad * t);
    });
}

// Returns julian date of given season in given year
double seasonJD(AstroSeasons::Season season, int year)
{
    // Astronimical Algorithms, Jean Meeus, chapter 26
    const auto jde0 = meanJDE(season, year);
    const auto T = (jde0 - 2451545.0) / 36525;
    const auto W_deg = 35999.373 * T + 2.47;
    const auto W_rad = W_deg * (M_PI / 180.0);
    const auto dLambda = 1 + (0.0334 * std::cos(W_rad)) + (0.0007 * std::cos(2 * W_rad));
    const auto S = periodicTerms(T);
    return jde0 + (0.00001 * S) / dLambda;
}

}

QDate AstroSeasons::seasonDate(Season season, int year)
{
    if (season == None) {
        return {};
    }
    const qint64 jd = round(seasonJD(season, year));
    return QDate::fromJulianDay(jd);
}

QString AstroSeasons::seasonNameAtDate(const QDate &date)
{
    return seasonName(seasonAtDate(date));
}

QString AstroSeasons::seasonName(AstroSeasons::Season season)
{
    switch (season) {
    case JuneSolstice:
        return QCoreApplication::translate("AstroSeasons", "June Solstice");
    case DecemberSolstice:
        return QCoreApplication::translate("AstroSeasons", "December Solstice");
    case MarchEquinox:
        return QCoreApplication::translate("AstroSeasons", "March Equinox");
    case SeptemberEquinox:
        return QCoreApplication::translate("AstroSeasons", "September Equinox");
    case None:
        return QString();
    }
    return QString();
}

AstroSeasons::Season AstroSeasons::seasonAtDate(const QDate &date)
{
    for (auto season : {JuneSolstice, DecemberSolstice, MarchEquinox, SeptemberEquinox}) {
        if (seasonDate(season, date.year()) == date) {
            return season;
        }
    }
    return None;
}
