/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2004 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2008 David Jarvie <djarvie@kde.org>
    SPDX-FileCopyrightText: 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KHOLIDAYS_HOLIDAY_P_H
#define KHOLIDAYS_HOLIDAY_P_H

#include "holiday.h"
#include <QDateTime>
#include <QSharedData>

namespace KHolidays
{
class HolidayPrivate : public QSharedData
{
public:
    QDate mObservedDate;
    int mDuration;
    QString mName;
    QString mDescription;
    QStringList mCategoryList;
    Holiday::DayType mDayType;
};

}

#endif // KHOLIDAYS_HOLIDAY_P_H
