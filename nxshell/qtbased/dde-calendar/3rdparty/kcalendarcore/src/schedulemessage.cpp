/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "schedulemessage.h"

#include <QString>

using namespace KCalendarCore;

//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::ScheduleMessage::Private
{
public:
    Private()
    {
    }

    IncidenceBase::Ptr mIncidence;
    iTIPMethod mMethod;
    Status mStatus;
    QString mError;

    ~Private()
    {
    }
};
//@endcond

ScheduleMessage::ScheduleMessage(const IncidenceBase::Ptr &incidence, iTIPMethod method, ScheduleMessage::Status status)
    : d(new KCalendarCore::ScheduleMessage::Private)
{
    d->mIncidence = incidence;
    d->mMethod = method;
    d->mStatus = status;
}

ScheduleMessage::~ScheduleMessage()
{
    delete d;
}

IncidenceBase::Ptr ScheduleMessage::event() const
{
    return d->mIncidence;
}

iTIPMethod ScheduleMessage::method() const
{
    return d->mMethod;
}

QString ScheduleMessage::methodName(iTIPMethod method)
{
    switch (method) {
    case iTIPPublish:
        return QStringLiteral("Publish");
    case iTIPRequest:
        return QStringLiteral("Request");
    case iTIPRefresh:
        return QStringLiteral("Refresh");
    case iTIPCancel:
        return QStringLiteral("Cancel");
    case iTIPAdd:
        return QStringLiteral("Add");
    case iTIPReply:
        return QStringLiteral("Reply");
    case iTIPCounter:
        return QStringLiteral("Counter");
    case iTIPDeclineCounter:
        return QStringLiteral("Decline Counter");
    default:
        return QStringLiteral("Unknown");
    }
}

ScheduleMessage::Status ScheduleMessage::status() const
{
    return d->mStatus;
}

QString ScheduleMessage::error() const
{
    return d->mError;
}
