/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Matej Koss <koss@miesto.sk>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kabstractwidgetjobtracker.h"
#include "kabstractwidgetjobtracker_p.h"

#include <QWidget>

KAbstractWidgetJobTracker::KAbstractWidgetJobTracker(QWidget *parent)
    : KAbstractWidgetJobTracker(*new KAbstractWidgetJobTrackerPrivate(this), parent)
{
}

KAbstractWidgetJobTracker::KAbstractWidgetJobTracker(KAbstractWidgetJobTrackerPrivate &dd, QWidget *parent)
    : KJobTrackerInterface(parent)
    , d(&dd)
{
}

KAbstractWidgetJobTracker::~KAbstractWidgetJobTracker() = default;

void KAbstractWidgetJobTracker::registerJob(KJob *job)
{
    KJobTrackerInterface::registerJob(job);
}

void KAbstractWidgetJobTracker::unregisterJob(KJob *job)
{
    KJobTrackerInterface::unregisterJob(job);
}

void KAbstractWidgetJobTracker::setStopOnClose(KJob *job, bool stopOnClose)
{
    Q_D(KAbstractWidgetJobTracker);

    d->setStopOnClose(job, stopOnClose);
}

bool KAbstractWidgetJobTracker::stopOnClose(KJob *job) const
{
    Q_D(const KAbstractWidgetJobTracker);

    return d->stopOnClose(job);
}

void KAbstractWidgetJobTracker::setAutoDelete(KJob *job, bool autoDelete)
{
    Q_D(KAbstractWidgetJobTracker);

    d->setAutoDelete(job, autoDelete);
}

bool KAbstractWidgetJobTracker::autoDelete(KJob *job) const
{
    Q_D(const KAbstractWidgetJobTracker);

    return d->autoDelete(job);
}

void KAbstractWidgetJobTracker::finished(KJob *job)
{
    Q_UNUSED(job);
}

void KAbstractWidgetJobTracker::slotStop(KJob *job)
{
    if (job) {
        job->kill(KJob::EmitResult); // notify that the job has been killed
        Q_EMIT stopped(job);
    }
}

void KAbstractWidgetJobTracker::slotSuspend(KJob *job)
{
    if (job) {
        job->suspend();
        Q_EMIT suspend(job);
    }
}

void KAbstractWidgetJobTracker::slotResume(KJob *job)
{
    if (job) {
        job->resume();
        Q_EMIT resume(job);
    }
}

void KAbstractWidgetJobTracker::slotClean(KJob *job)
{
    Q_UNUSED(job);
}
