/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2005-2007 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "zodiac.h"

#include <QCoreApplication>
#include <QDate>
#include <QSharedData>

using namespace KHolidays;

class Q_DECL_HIDDEN KHolidays::ZodiacPrivate : public QSharedData
{
public:
    explicit ZodiacPrivate(Zodiac::ZodiacType type)
        : mType(type)
    {
    }

    ZodiacPrivate(const ZodiacPrivate &other)
        : QSharedData(other)
    {
        mType = other.mType;
    }

    Zodiac::ZodiacType mType;
};

Zodiac::Zodiac(ZodiacType type)
    : d(new ZodiacPrivate(type))
{
}

Zodiac::Zodiac(const Zodiac &other)
    : d(other.d)
{
}

Zodiac::~Zodiac()
{
}

Zodiac &Zodiac::operator=(const Zodiac &other)
{
    if (&other != this) {
        d = other.d;
    }

    return *this;
}

QString Zodiac::signNameAtDate(const QDate &date) const
{
    return signName(signAtDate(date));
}

QString Zodiac::signName(Zodiac::ZodiacSigns sign)
{
    switch (sign) {
    case Aries:
        return QCoreApplication::translate("Zodiac", "Aries");
    case Taurus:
        return QCoreApplication::translate("Zodiac", "Taurus");
    case Gemini:
        return QCoreApplication::translate("Zodiac", "Gemini");
    case Cancer:
        return QCoreApplication::translate("Zodiac", "Cancer");
    case Leo:
        return QCoreApplication::translate("Zodiac", "Leo");
    case Virgo:
        return QCoreApplication::translate("Zodiac", "Virgo");
    case Libra:
        return QCoreApplication::translate("Zodiac", "Libra");
    case Scorpio:
        return QCoreApplication::translate("Zodiac", "Scorpio");
    case Sagittarius:
        return QCoreApplication::translate("Zodiac", "Sagittarius");
    case Capricorn:
        return QCoreApplication::translate("Zodiac", "Capricorn");
    case Aquarius:
        return QCoreApplication::translate("Zodiac", "Aquarius");
    case Pisces:
        return QCoreApplication::translate("Zodiac", "Pisces");
    case None:
        return QString();
    }
    return QString();
}

Zodiac::ZodiacSigns Zodiac::signAtDate(const QDate &date) const
{
    QDate startdate;
    QDate enddate;

    switch (d->mType) {
    case Tropical:
        startdate = QDate(date.year(), 1, 1);
        enddate = QDate(date.year(), 1, 19);
        if (date >= startdate && date <= enddate) {
            return Capricorn;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(29);
        if (date >= startdate && date <= enddate) {
            return Aquarius;
        }

        startdate = enddate.addDays(1);
        enddate = QDate(date.year(), 3, 20);
        if (date >= startdate && date <= enddate) {
            return Pisces;
        }

        startdate = enddate.addDays(1); // March 21
        enddate = startdate.addDays(29);
        if (date >= startdate && date <= enddate) {
            return Aries;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(30);
        if (date >= startdate && date <= enddate) {
            return Taurus;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(30);
        if (date >= startdate && date <= enddate) {
            return Gemini;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(31);
        if (date >= startdate && date <= enddate) {
            return Cancer;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(30);
        if (date >= startdate && date <= enddate) {
            return Leo;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(30);
        if (date >= startdate && date <= enddate) {
            return Virgo;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(29);
        if (date >= startdate && date <= enddate) {
            return Libra;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(29);
        if (date >= startdate && date <= enddate) {
            return Scorpio;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(29);
        if (date >= startdate && date <= enddate) {
            return Sagittarius;
        }

        return Capricorn;

    case Sidereal:
        startdate = QDate(date.year(), 1, 1);
        enddate = QDate(date.year(), 1, 14);
        if (date >= startdate && date <= enddate) {
            return Sagittarius;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(28);
        if (date >= startdate && date <= enddate) {
            return Capricorn;
        }

        startdate = enddate.addDays(1);
        enddate = QDate(date.year(), 3, 14);
        if (date >= startdate && date <= enddate) {
            return Aquarius;
        }

        startdate = enddate.addDays(1);
        enddate = QDate(date.year(), 4, 13);
        if (date >= startdate && date <= enddate) {
            return Pisces;
        }

        startdate = QDate(date.year(), 4, 14); // April 14
        enddate = startdate.addDays(30);
        if (date >= startdate && date <= enddate) {
            return Aries;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(30);
        if (date >= startdate && date <= enddate) {
            return Taurus;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(31);
        if (date >= startdate && date <= enddate) {
            return Gemini;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(30);
        if (date >= startdate && date <= enddate) {
            return Cancer;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(30);
        if (date >= startdate && date <= enddate) {
            return Leo;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(30);
        if (date >= startdate && date <= enddate) {
            return Virgo;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(29);
        if (date >= startdate && date <= enddate) {
            return Libra;
        }

        startdate = enddate.addDays(1);
        enddate = startdate.addDays(28);
        if (date >= startdate && date <= enddate) {
            return Scorpio;
        }

        return Sagittarius;
    }
    return None;
}

QString Zodiac::signSymbol(Zodiac::ZodiacSigns sign)
{
    switch (sign) {
    case Aries:
        return QCoreApplication::translate("HolidayRegion", "ram", "zodiac symbol for Aries");
    case Taurus:
        return QCoreApplication::translate("HolidayRegion", "bull", "zodiac symbol for Taurus");
    case Gemini:
        return QCoreApplication::translate("HolidayRegion", "twins", "zodiac symbol for Gemini");
    case Cancer:
        return QCoreApplication::translate("HolidayRegion", "crab", "zodiac symbol for Cancer");
    case Leo:
        return QCoreApplication::translate("HolidayRegion", "lion", "zodiac symbol for Leo");
    case Virgo:
        return QCoreApplication::translate("HolidayRegion", "virgin", "zodiac symbol for Virgo");
    case Libra:
        return QCoreApplication::translate("HolidayRegion", "scales", "zodiac symbol for Libra");
    case Scorpio:
        return QCoreApplication::translate("HolidayRegion", "scorpion", "zodiac symbol for Scorpion");
    case Sagittarius:
        return QCoreApplication::translate("HolidayRegion", "archer", "zodiac symbol for Sagittarius");
    case Capricorn:
        return QCoreApplication::translate("HolidayRegion", "goat", "zodiac symbol for Capricorn");
    case Aquarius:
        return QCoreApplication::translate("HolidayRegion", "water carrier", "zodiac symbol for Aquarius");
    case Pisces:
        return QCoreApplication::translate("HolidayRegion", "fish", "zodiac symbol for Pices");
    case None:
        return QString();
    }
    return QString();
}
