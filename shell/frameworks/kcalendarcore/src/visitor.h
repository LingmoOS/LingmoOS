/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  SPDX-FileContributor: Alvaro Manera <alvaro.manera@nokia.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALCORE_VISITOR_H
#define KCALCORE_VISITOR_H

#include "event.h"
#include "freebusy.h"
#include "journal.h"
#include "todo.h"

namespace KCalendarCore
{
/**
  This class provides the interface for a visitor of calendar components.
  It serves as base class for concrete visitors, which implement certain
  actions on calendar components. It allows to add functions, which operate
  on the concrete types of calendar components, without changing the
  calendar component classes.
*/
class KCALENDARCORE_EXPORT Visitor // krazy:exclude=dpointer
{
public:
    /** Destruct Incidence::Visitor */
    virtual ~Visitor();

    /**
      Reimplement this function in your concrete subclass of
      IncidenceBase::Visitor to perform actions on an Event object.
      @param event is a pointer to a valid Event object.
    */
    virtual bool visit(const Event::Ptr &event);

    /**
      Reimplement this function in your concrete subclass of
      IncidenceBase::Visitor to perform actions on a Todo object.
      @param todo is a pointer to a valid Todo object.
    */
    virtual bool visit(const Todo::Ptr &todo);

    /**
      Reimplement this function in your concrete subclass of
      IncidenceBase::Visitor to perform actions on an Journal object.
      @param journal is a pointer to a valid Journal object.
    */
    virtual bool visit(const Journal::Ptr &journal);

    /**
      Reimplement this function in your concrete subclass of
      IncidenceBase::Visitor to perform actions on a FreeBusy object.
      @param freebusy is a pointer to a valid FreeBusy object.
    */
    virtual bool visit(const FreeBusy::Ptr &freebusy);

protected:
    /**
      Constructor is protected to prevent direct creation of visitor
      base class.
    */
    Visitor();
};

} // end namespace

#endif
