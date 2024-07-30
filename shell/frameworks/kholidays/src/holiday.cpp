/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2004 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2008 David Jarvie <djarvie@kde.org>
    SPDX-FileCopyrightText: 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "holiday.h"
#include "holiday_p.h"

#include <QSharedData>

using namespace KHolidays;

Holiday::Holiday()
    : d(new HolidayPrivate)
{
}

Holiday::Holiday(const Holiday &other)
    : d(other.d)
{
}

Holiday::~Holiday()
{
}

Holiday &Holiday::operator=(const Holiday &other)
{
    if (&other != this) {
        d = other.d;
    }

    return *this;
}

bool Holiday::operator<(const Holiday &rhs) const
{
    return d->mObservedDate < rhs.d->mObservedDate;
}

bool Holiday::operator>(const Holiday &rhs) const
{
    return d->mObservedDate > rhs.d->mObservedDate;
}

QDate Holiday::observedStartDate() const
{
    return d->mObservedDate;
}

QDate Holiday::observedEndDate() const
{
    return d->mObservedDate.addDays(d->mDuration - 1);
}

int Holiday::duration() const
{
    return d->mDuration;
}

QString Holiday::name() const
{
    return d->mName;
}

QString Holiday::description() const
{
    return d->mDescription;
}

Holiday::DayType Holiday::dayType() const
{
    return d->mDayType;
}

QStringList Holiday::categoryList() const
{
    return d->mCategoryList;
}
