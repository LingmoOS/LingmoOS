/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sequence_p.h"
#include "debuggingaids.h"

namespace ThreadWeaver
{
namespace Private
{
Sequence_Private::Sequence_Private()
{
}

BlockerPolicy *Sequence_Private::blocker()
{
    return &blocker_;
}

void Sequence_Private::prepareToEnqueueElements()
{
    Q_ASSERT(!mutex.tryLock());
    const int jobs = elements.count();
    // probably incorrect:
    completed_.storeRelease(0);
    // block the execution of the later jobs:
    for (int i = 0; i < jobs; ++i) {
        TWDEBUG(4, "Sequence_Private::prepareToEnqueueElements: blocking %p\n", elements.at(i).data());
        JobPointer nextJob = elements.at(i);
        QMutexLocker l(nextJob->mutex());
        nextJob->assignQueuePolicy(blocker());
    }
}

void Sequence_Private::processCompletedElement(Collection *collection, JobPointer job, Thread *)
{
    Q_ASSERT(!mutex.tryLock());
    Q_ASSERT(job != nullptr);
    Q_ASSERT(!self.isNull());

    auto updatedStatus = updateStatus(collection, job);
    if (updatedStatus != JobInterface::Status_Running) {
        // We need to unlock mutex so that `stop` can
        // properly stop us
        mutex.unlock();
        stop(collection);
        mutex.lock();
        // stop might have changed our status
        // so lets restore back to original
        collection->setStatus(updatedStatus);
    }
    const int next = completed_.fetchAndAddAcquire(1);
    const int count = elements.count();
    if (count > 0) {
        if (next < count) {
            TWDEBUG(4, "Sequence_Private::processCompletedElement: unblocking %p\n", elements.at(next).data());
            JobPointer nextJob = elements.at(next);
            QMutexLocker l(nextJob->mutex());
            nextJob->removeQueuePolicy(blocker());
        }
    }
}

void Sequence_Private::elementDequeued(const JobPointer &job)
{
    Q_ASSERT(!mutex.tryLock());
    QMutexLocker l(job->mutex());
    job->removeQueuePolicy(blocker());
}

void BlockerPolicy::destructed(JobInterface *)
{
}

bool BlockerPolicy::canRun(JobPointer)
{
    return false;
}

void BlockerPolicy::free(JobPointer)
{
}

void BlockerPolicy::release(JobPointer)
{
}

}

}
