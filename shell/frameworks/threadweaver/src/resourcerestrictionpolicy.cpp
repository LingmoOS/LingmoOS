/* -*- C++ -*-
    This file implements the ResourceRestrictionPolicy class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: Job.h 32 2005-08-17 08:38:01Z mirko $
*/

#include "resourcerestrictionpolicy.h"

#include <QList>
#include <QMutex>

#include "debuggingaids.h"
#include "managedjobpointer.h"

using namespace ThreadWeaver;

class Q_DECL_HIDDEN ResourceRestrictionPolicy::Private
{
public:
    Private(int theCap)
        : cap(theCap)
    {
    }
    QMutex *mutex()
    {
        return &mutex_;
    }

    int cap;
    QList<JobPointer> customers;
    QMutex mutex_;
};

ResourceRestrictionPolicy::ResourceRestrictionPolicy(int cap)
    : QueuePolicy()
    , d(new Private(cap))
{
}

ResourceRestrictionPolicy::~ResourceRestrictionPolicy()
{
    delete d;
}

void ResourceRestrictionPolicy::setCap(int cap)
{
    QMutexLocker l(d->mutex());
    d->cap = cap;
}

int ResourceRestrictionPolicy::cap() const
{
    QMutexLocker l(d->mutex());
    return d->cap;
}

bool ResourceRestrictionPolicy::canRun(JobPointer job)
{
    QMutexLocker l(d->mutex());
    if (d->customers.size() < d->cap) {
        d->customers.append(job);
        TWDEBUG(4, "ResourceRestrictionPolicy::canRun: job %p added, %i customers (cap %i).\n", (void *)job.data(), d->customers.count(), d->cap);
        return true;
    } else {
        return false;
    }
}

void ResourceRestrictionPolicy::free(JobPointer job)
{
    QMutexLocker l(d->mutex());
    int position = d->customers.indexOf(job);

    if (position != -1) {
        d->customers.removeAt(position);
        TWDEBUG(4, "ResourceRestrictionPolicy::free: job %p completed, %i customers (cap %i).\n", (void *)job.data(), d->customers.count(), d->cap);
    }
}

void ResourceRestrictionPolicy::release(JobPointer job)
{
    free(job);
}

void ResourceRestrictionPolicy::destructed(JobInterface *job)
{
    free(ManagedJobPointer<JobInterface>(job));
}
