/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the FreeBusyCache abstract base class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef KCALCORE_FREEBUSYCACHE_H
#define KCALCORE_FREEBUSYCACHE_H

#include "kcalendarcore_export.h"

#include "freebusy.h"

class QString;

namespace KCalendarCore
{
class Person;

/**
  @brief
  An abstract base class to allow different implementations of storing
  free busy information, e.g. local storage or storage on a Kolab server.
*/
class KCALENDARCORE_EXPORT FreeBusyCache
{
public:
    /**
      Destructor.
    */
    virtual ~FreeBusyCache();

    /**
      Save freebusy information belonging to an email.

      @param freebusy is a pointer to a valid FreeBusy instance.
      @param person is a valid Person instance.
      @return true if the save was successful; false otherwise.
    */
    virtual bool saveFreeBusy(const FreeBusy::Ptr &freebusy, const Person &person) = 0;

    /**
      Load freebusy information belonging to an email.

      @param email is a QString containing a email string in the
        "FirstName LastName <emailaddress>" format.
      @return A pointer to the FreeBusy object loaded for the specified email; returns 0 if
        there was some problem attempting to load the FreeBusy information.
    */
    virtual FreeBusy::Ptr loadFreeBusy(const QString &email) = 0;

protected:
    /**
      @copydoc IncidenceBase::virtual_hook()
    */
    virtual void virtual_hook(int id, void *data);
};

}

#endif
