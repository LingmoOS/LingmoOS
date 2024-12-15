/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCALCORE_SCHEDULEMESSAGE_H
#define KCALCORE_SCHEDULEMESSAGE_H

#include "incidencebase.h"

namespace KCalendarCore {
class IncidenceBase;

/**
   iTIP methods.
*/
enum iTIPMethod {
    iTIPPublish, /**< Event, to-do, journal or freebusy posting */
    iTIPRequest, /**< Event, to-do or freebusy scheduling request */
    iTIPReply, /**< Event, to-do or freebusy reply to request */
    iTIPAdd, /**< Event, to-do or journal additional property request */
    iTIPCancel, /**< Event, to-do or journal cancellation notice */
    iTIPRefresh, /**< Event or to-do description update request */
    iTIPCounter, /**< Event or to-do submit counter proposal */
    iTIPDeclineCounter, /**< Event or to-do decline a counter proposal */
    iTIPNoMethod, /**< No method */
};

/**
  @brief
  A Scheduling message class.

  This class provides an encapsulation of a scheduling message.
  It associates an incidence with an iTIPMethod and status information.
*/
class Q_CORE_EXPORT ScheduleMessage
{
public:
    /**
      Message status.
    */
    enum Status {
        PublishNew, /**< New message posting */
        PublishUpdate, /**< Updated message */
        Obsolete, /**< obsolete */
        RequestNew, /**< Request new message posting */
        RequestUpdate, /**< Request updated message */
        Unknown, /**< No status */
    };

    /**
      A shared pointer to a ScheduleMessage.
    */
    typedef QSharedPointer<ScheduleMessage> Ptr;

    /**
      Creates a scheduling message with method as defined in iTIPMethod and a status.
      @param incidence a pointer to a valid Incidence to be associated with this message.
      @param method an iTIPMethod.
      @param status a Status.
    */
    ScheduleMessage(const IncidenceBase::Ptr &incidence, iTIPMethod method, Status status);

    /**
      Destructor.
    */
    ~ScheduleMessage();

    /**
      Returns the event associated with this message.
    */
    IncidenceBase::Ptr event() const;

    /**
      Returns the iTIP method associated with this message.
    */
    Q_REQUIRED_RESULT iTIPMethod method() const;

    /**
      Returns a machine-readable (not translatable) name for a iTIP method.
      @param method an iTIPMethod.
    */
    Q_REQUIRED_RESULT static QString methodName(iTIPMethod method);

    /**
      Returns the status of this message.
    */
    Q_REQUIRED_RESULT Status status() const;

    /**
      Returns the error message if there is any.
    */
    Q_REQUIRED_RESULT QString error() const;

private:
    //@cond PRIVATE
    Q_DISABLE_COPY(ScheduleMessage)
    class Private;
    Private *const d;
    //@endcond
};

} // namespace KCalendarCore

#endif
