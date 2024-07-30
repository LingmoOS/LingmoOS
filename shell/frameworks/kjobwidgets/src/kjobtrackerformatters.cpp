/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1997, 2001 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1999-2002 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2002 Lukas Tinkl <lukas@kde.org>
    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>
    SPDX-FileCopyrightText: 2009, 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kjobtrackerformatters_p.h"
#include <KFormat>

#include <QCoreApplication>

QString KJobTrackerFormatters::byteSize(double size)
{
    return KFormat().formatByteSize(size);
}

QString KJobTrackerFormatters::daysDuration(int n)
{
    //~ singular %n day
    //~ plural %n days
    return QCoreApplication::translate("KJobTrackerFormatters", "%n day(s)", "@item:intext", n);
}

QString KJobTrackerFormatters::hoursDuration(int n)
{
    //~ singular %n hour
    //~ plural %n hours
    return QCoreApplication::translate("KJobTrackerFormatters", "%n hour(s)", "@item:intext", n);
}

QString KJobTrackerFormatters::minutesDuration(int n)
{
    //~ singular %n minute
    //~ plural %n minutes
    return QCoreApplication::translate("KJobTrackerFormatters", "%n minute(s)", "@item:intext", n);
}

QString KJobTrackerFormatters::secondsDuration(int n)
{
    //~ singular %n second
    //~ plural %n seconds
    return QCoreApplication::translate("KJobTrackerFormatters", "%n second(s)", "@item:intext", n);
}

QString KJobTrackerFormatters::duration(unsigned long mSec)
{
    unsigned long ms = mSec;
    int days = ms / (24 * 3600000);
    ms = ms % (24 * 3600000);
    int hours = ms / 3600000;
    ms = ms % 3600000;
    int minutes = ms / 60000;
    ms = ms % 60000;
    int seconds = qRound(ms / 1000.0);

    // Handle correctly problematic case #1 (look at KLocaleTest::prettyFormatDuration()
    // at klocaletest.cpp)
    if (seconds == 60) {
        return duration(mSec - ms + 60000);
    }

    if (days && hours) {
        return QCoreApplication::translate("KJobTrackerFormatters", "%1 and %2", "@item:intext days and hours.").arg(daysDuration(days), hoursDuration(hours));
    } else if (days) {
        return daysDuration(days);
    } else if (hours && minutes) {
        return QCoreApplication::translate("KJobTrackerFormatters", "%1 and %2", "@item:intext hours and minutes.")
            .arg(hoursDuration(hours), minutesDuration(minutes));
    } else if (hours) {
        return hoursDuration(hours);
    } else if (minutes && seconds) {
        return QCoreApplication::translate("KJobTrackerFormatters", "%1 and %2", "@item:intext minutes and seconds.")
            .arg(minutesDuration(minutes), secondsDuration(seconds));
    } else if (minutes) {
        return minutesDuration(minutes);
    } else {
        return secondsDuration(seconds);
    }
}
