/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALCORE_INCIDENCE_P_H
#define KCALCORE_INCIDENCE_P_H

#include "incidence.h"      // For Incidence::RelType, etc.
#include "incidencebase_p.h"

namespace KCalendarCore
{

//@cond PRIVATE
/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/

class Incidence;

class IncidencePrivate : public KCalendarCore::IncidenceBasePrivate
{
public:
    IncidencePrivate();
    IncidencePrivate(const IncidencePrivate &p);
    explicit IncidencePrivate(const Incidence &);
    void clear();
    void init(Incidence *q, const IncidencePrivate &other);

    virtual bool validStatus(Incidence::Status);

    QDateTime mCreated; // creation datetime
    QString mDescription; // description string
    QString mSummary; // summary string
    QString mLocation; // location string
    QStringList mCategories; // category list
    Attachment::List mAttachments; // attachments list
    Alarm::List mAlarms; // alarms list
    QStringList mResources; // resources list (not calendar resources)
    QString mStatusString; // status string, for custom status
    QString mSchedulingID; // ID for scheduling mails
    QMap<Incidence::RelType, QString> mRelatedToUid; // incidence uid this is related to, for each relType
    QDateTime mRecurrenceId; // recurrenceId
    Conference::List mConferences; // conference list

    float mGeoLatitude = INVALID_LATLON; // Specifies latitude in decimal degrees
    float mGeoLongitude = INVALID_LATLON; // Specifies longitude in decimal degrees
    mutable Recurrence *mRecurrence = nullptr; // recurrence
    int mRevision = 0; // revision number
    int mPriority = 0; // priority: 0 = unknown, 1 = highest,  ... 9 = lowest.
    Incidence::Status mStatus = Incidence::StatusNone; // status
    Incidence::Secrecy mSecrecy = Incidence::SecrecyPublic; // secrecy
    QString mColor; // background color
    bool mDescriptionIsRich = false; // description string is richtext.
    bool mSummaryIsRich = false; // summary string is richtext.
    bool mLocationIsRich = false; // location string is richtext.
    bool mThisAndFuture = false;
    bool mLocalOnly = false; // allow changes that won't go to the server
};
//@endcond

}

#endif
