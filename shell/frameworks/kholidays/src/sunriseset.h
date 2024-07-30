/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2012 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KHOLIDAYS_SUNRISESET_H
#define KHOLIDAYS_SUNRISESET_H

#include "kholidays_export.h"
#include <QTime>

namespace KHolidays
{
/**
  @brief
  Methods for determining the sunrise and sunset times for a given date and Earth location.
*/

namespace SunRiseSet
{
/**
  Compute the sunrise time (UTC) for a date and Earth location.
  @param date is any valid QDate.
  @param latitude is a floating point representing a valid latitude (-90.0, 90.0)
  @param longitude is a floating point representing a valid longitude (-180.0, 180.0)
  @return the QTime of the sunrise in UTC.

  @note the latitude and longitude are truncated as needed to fit into their proper range.

*/
KHOLIDAYS_EXPORT QTime utcSunrise(const QDate &date, double latitude, double longitude);

/**
  Compute the sunset time (UTC) for a date and Earth location.
  @param date is any valid QDate.
  @param latitude is a floating point representing a valid latitude (-90.0, 90.0)
  @param longitude is a floating point representing a valid longitude (-180.0, 180.0)
  @return the QTime of the sunset in UTC.

  @note the latitude and longitude are truncated as needed to fit into their proper range.
*/
KHOLIDAYS_EXPORT QTime utcSunset(const QDate &date, double latitude, double longitude);

/**
  Compute the civil dawn time (UTC) for a date and Earth location.
  @param date is any valid QDate.
  @param latitude is a floating point representing a valid latitude (-90.0, 90.0)
  @param longitude is a floating point representing a valid longitude (-180.0, 180.0)
  @see https://en.wikipedia.org/wiki/Twilight
  @return the QTime of the sunrise in UTC.

  @note the latitude and longitude are truncated as needed to fit into their proper range.
  @since 5.77
*/
KHOLIDAYS_EXPORT QTime utcDawn(const QDate &date, double latitude, double longitude);

/**
  Compute the civil dawn time (UTC) for a date and Earth location.
  @param date is any valid QDate.
  @param latitude is a floating point representing a valid latitude (-90.0, 90.0)
  @param longitude is a floating point representing a valid longitude (-180.0, 180.0)
  @see https://en.wikipedia.org/wiki/Twilight
  @return the QTime of the sunset in UTC.

  @note the latitude and longitude are truncated as needed to fit into their proper range.
  @since 5.77
*/
KHOLIDAYS_EXPORT QTime utcDusk(const QDate &date, double latitude, double longitude);

/**
  Checks whether it is polar day on day @p date at @p latitude.
  That is, the sun stays above -0.83° relative to the horizon at all times.
  Both sunrise/sunset and dawn/dusk times will be invalid for such a day.
  @param latitude in degree (-90.0, 90.0)
  @see isPolarNight(), isPolarTwilight()
  @see https://en.wikipedia.org/wiki/Midnight_sun

  @since 5.97
*/
KHOLIDAYS_EXPORT bool isPolarDay(const QDate &date, double latitude);

/**
  Checks whether it is polar twilight on day @p date at @p latitude.
  That is, the sun rises at least above -6° relative the horizon during the day,
  but remains below -0.83° at all times. Sunrise/sunset times will be invalid
  for such a day, but dawn/dusk times will be available.
  @param latitude in degree (-90.0, 90.0)
  @see isPolarDay(), isPolarNight()
  @see https://en.wikipedia.org/wiki/Polar_night

  @since 5.97
*/
KHOLIDAYS_EXPORT bool isPolarTwilight(const QDate &date, double latitude);

/**
  Checks whether it is polar night on day @p date at @p latitude.
  That is, the sun stays below -6° relative to the horizon at all times.
  Both sunrise/sunset and dawn/dusk times will be invalid for such a day.
  @param latitude in degree (-90.0, 90.0)
  @see isPolarDay(), isPolarTwilight()
  @see https://en.wikipedia.org/wiki/Polar_night

  @since 5.97
*/
KHOLIDAYS_EXPORT bool isPolarNight(const QDate &date, double latitude);
}

}

#endif
