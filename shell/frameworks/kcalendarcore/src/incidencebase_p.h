/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001,2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  SPDX-FileContributor: Alvaro Manera <alvaro.manera@nokia.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALCORE_INCIDENCEBASE_P_H
#define KCALCORE_INCIDENCEBASE_P_H

#include "incidencebase.h"      // For IncidenceBase::Field

namespace KCalendarCore
{

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class IncidenceBasePrivate
{
public:
    IncidenceBasePrivate() = default;

    IncidenceBasePrivate(const IncidenceBasePrivate &other)
    {
        init(other);
    }

    virtual ~IncidenceBasePrivate() = default;

    void init(const IncidenceBasePrivate &other);

    QDateTime mLastModified; // incidence last modified date
    QDateTime mDtStart; // incidence start time
    Person mOrganizer; // incidence person (owner)
    QString mUid; // incidence unique id
    Duration mDuration; // incidence duration
    int mUpdateGroupLevel = 0; // if non-zero, suppresses update() calls
    bool mUpdatedPending = false; // true if an update has occurred since startUpdates()
    bool mAllDay = false; // true if the incidence is all-day
    bool mHasDuration = false; // true if the incidence has a duration
    Attendee::List mAttendees; // list of incidence attendees
    QStringList mComments; // list of incidence comments
    QStringList mContacts; // list of incidence contacts
    QList<IncidenceBase::IncidenceObserver *> mObservers; // list of incidence observers
    QSet<IncidenceBase::Field> mDirtyFields; // Fields that changed since last time the incidence was created
    // or since resetDirtyFlags() was called
    QUrl mUrl; // incidence url property
};

//@endcond

}

#endif
