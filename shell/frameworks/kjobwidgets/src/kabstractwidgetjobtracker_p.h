/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Matej Koss <koss@miesto.sk>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KABSTRACTWIDGETJOBTRACKER_P
#define KABSTRACTWIDGETJOBTRACKER_P

#include "kabstractwidgetjobtracker.h"

class KAbstractWidgetJobTrackerPrivate
{
    Q_DECLARE_PUBLIC(KAbstractWidgetJobTracker)
public:
    KAbstractWidgetJobTrackerPrivate(KAbstractWidgetJobTracker *qq)
        : q_ptr(qq)
    {
    }

    virtual ~KAbstractWidgetJobTrackerPrivate()
    {
    }

    KAbstractWidgetJobTracker *const q_ptr;

    //### KDE 5: make this methods virtual on KAbstractWidgetJobTracker and get rid out of this
    //           workaround. (ereslibre)
    virtual void setStopOnClose(KJob *job, bool stopOnClose)
    {
        Q_UNUSED(job);
        Q_UNUSED(stopOnClose);
    }

    virtual bool stopOnClose(KJob *job) const
    {
        Q_UNUSED(job);
        return true;
    }

    virtual void setAutoDelete(KJob *job, bool autoDelete)
    {
        Q_UNUSED(job);
        Q_UNUSED(autoDelete);
    }

    virtual bool autoDelete(KJob *job) const
    {
        Q_UNUSED(job);
        return true;
    }
};

#endif // KABSTRACTWIDGETJOBTRACKER_P
