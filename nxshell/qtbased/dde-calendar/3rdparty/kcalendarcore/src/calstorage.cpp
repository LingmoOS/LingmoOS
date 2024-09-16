/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2002, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CalStorage abstract base class.

  @brief
  An abstract base class that provides a calendar storage interface.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "calstorage.h"

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::CalStorage::Private
{
public:
    Private(const Calendar::Ptr &cal)
        : mCalendar(cal)
    {
    }
    Calendar::Ptr mCalendar;
};
//@endcond

CalStorage::CalStorage(const Calendar::Ptr &calendar)
    : d(new KCalendarCore::CalStorage::Private(calendar))
{
}

CalStorage::~CalStorage()
{
    delete d;
}

Calendar::Ptr CalStorage::calendar() const
{
    return d->mCalendar;
}
