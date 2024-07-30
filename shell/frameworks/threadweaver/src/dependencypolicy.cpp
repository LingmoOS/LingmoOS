/* -*- C++ -*-
    This file implements the DependencyPolicy class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: DebuggingAids.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include "dependencypolicy.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMutex>

#include "debuggingaids.h"
#include "job.h"
#include "managedjobpointer.h"

#include "dependency.h"

using namespace ThreadWeaver;

typedef QMultiMap<JobPointer, JobPointer> JobMultiMap;

class Q_DECL_HIDDEN DependencyPolicy::Private
{
public:
    /** A container to keep track of Job dependencies.
     *  For each dependency A->B, which means Job B depends on Job A and may only be executed after A has been
     *  finished, an entry will be added with key A and value B. When A is finished, the entry will be removed.
     */
    JobMultiMap &dependencies()
    {
        return depMap_;
    }

    QMutex *mutex()
    {
        return &mutex_;
    }

    JobMultiMap depMap_;
    QMutex mutex_;
};

DependencyPolicy::DependencyPolicy()
    : QueuePolicy()
    , d(new Private())
{
}

DependencyPolicy::~DependencyPolicy()
{
    delete d;
}

void DependencyPolicy::addDependency(JobPointer jobA, JobPointer jobB)
{
    // jobA depends on jobB
    REQUIRE(jobA != nullptr && jobB != nullptr && jobA != jobB);

    QMutexLocker a(jobA->mutex());
    QMutexLocker b(jobB->mutex());
    QMutexLocker l(d->mutex());
    jobA->assignQueuePolicy(this);
    jobB->assignQueuePolicy(this);
    d->dependencies().insert(jobA, jobB);
    TWDEBUG(2, "inserted dependency %p->%p.\n", jobA.data(), jobB.data());
    ENSURE(d->dependencies().contains(jobA));
}

void DependencyPolicy::addDependency(const Dependency &dep)
{
    addDependency(dep.dependent(), dep.dependee());
}

bool DependencyPolicy::removeDependency(JobPointer jobA, JobPointer jobB)
{
    REQUIRE(jobA != nullptr && jobB != nullptr);
    bool result = false;
    QMutexLocker l(d->mutex());

    // there may be only one (!) occurrence of [this, dep]:
    QMutableMultiMapIterator<JobPointer, JobPointer> it(d->dependencies());
    while (it.hasNext()) {
        it.next();
        if (it.key() == jobA && it.value() == jobB) {
            it.remove();
            TWDEBUG(2, "removed dependency %p->%p.\n", jobA.data(), jobB.data());
            result = true;
            break;
        }
    }
    TWDEBUG(result == false, 2, "cannot remove dependency %p->%p, not found.\n", jobA.data(), jobB.data());
    ENSURE(!d->dependencies().keys(jobB).contains(jobA));
    return result;
}

bool DependencyPolicy::removeDependency(const Dependency &dep)
{
    return removeDependency(dep.dependent(), dep.dependee());
}

void DependencyPolicy::resolveDependencies(JobPointer job)
{
    if (job->success()) {
        QMutexLocker l(d->mutex());
        QMutableMultiMapIterator<JobPointer, JobPointer> it(d->dependencies());
        // there has to be a better way to do this: (?)
        while (it.hasNext()) { // we remove all entries where jobs depend on *this* :
            it.next();
            if (it.value() == job) {
                TWDEBUG(2, "resolved dependencies for %p: %p->%p.\n", job.data(), it.key().data(), it.value().data());
                it.remove();
            }
        }
    }
}

// QList<JobPointer> DependencyPolicy::getDependencies(JobPointer job) const
//{
//    REQUIRE (job != 0);
//    QList<JobInterface*> result;
//    JobMultiMap::const_iterator it;
//    QMutexLocker l( & d->mutex() );

//    for ( it = d->dependencies().constBegin(); it != d->dependencies().constEnd(); ++it )
//    {
//        if ( it.key() == job )
//        {
//            result.append( it.value() );
//        }
//    }
//    return result;
//}

bool DependencyPolicy::hasUnresolvedDependencies(JobPointer job) const
{
    REQUIRE(job != nullptr);
    QMutexLocker l(d->mutex());
    return d->dependencies().contains(job);
}

bool DependencyPolicy::isEmpty() const
{
    QMutexLocker l(d->mutex());
    return d->dependencies().isEmpty();
}

DependencyPolicy &DependencyPolicy::instance()
{
    static DependencyPolicy policy;
    return policy;
}

bool DependencyPolicy::canRun(JobPointer job)
{
    REQUIRE(job != nullptr);
    return !hasUnresolvedDependencies(job);
}

void DependencyPolicy::free(JobPointer job)
{
    REQUIRE(job != nullptr);
    REQUIRE(job->status() > Job::Status_Running);
    if (job->success()) {
        resolveDependencies(job);
        TWDEBUG(3, "DependencyPolicy::free: dependencies resolved for job %p.\n", (void *)job.data());
    } else {
        TWDEBUG(3, "DependencyPolicy::free: not resolving dependencies for %p (execution not successful).\n", (void *)job.data());
    }
    ENSURE((!hasUnresolvedDependencies(job) && job->success()) || !job->success());
}

void DependencyPolicy::release(JobPointer job)
{
    REQUIRE(job != nullptr);
    Q_UNUSED(job)
}

void DependencyPolicy::destructed(JobInterface *job)
{
    REQUIRE(job != nullptr);
    resolveDependencies(ManagedJobPointer<JobInterface>(job));
}

// void DependencyPolicy::dumpJobDependencies()
//{
//    QMutexLocker l( & d->mutex() );

//    debug ( 0, "Job Dependencies (left depends on right side):\n" );
//    for ( JobMultiMap::const_iterator it = d->dependencies().constBegin(); it != d->dependencies().constEnd(); ++it )
//    {
//        debug( 0, "  : %p <-- %p\n", (void*)it.key(), (void*)it.value());
//    }
//    debug ( 0, "-----------------\n" );
//}
