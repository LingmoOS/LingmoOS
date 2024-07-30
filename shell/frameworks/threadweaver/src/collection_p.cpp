/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collection_p.h"
#include "collection.h"
#include "debuggingaids.h"
#include "managedjobpointer.h"
#include "queueapi.h"

namespace ThreadWeaver
{
namespace Private
{
Collection_Private::Collection_Private()
    : api(nullptr)
    , jobCounter(0)
    , selfIsExecuting(false)
{
}

Collection_Private::~Collection_Private()
{
}

void Collection_Private::finalCleanup(Collection *collection)
{
    Q_ASSERT(!self.isNull());
    Q_ASSERT(!mutex.tryLock());
    if (collection->shouldAbort()) {
        collection->setStatus(Job::Status_Aborted);
    } else if (collection->status() < Job::Status_Success) {
        collection->setStatus(Job::Status_Success);
    } else {
        // At this point we either should have been running
        // in which case above would mark Success
        // or otherwise we already should be in Failed or Aborted state
        Q_ASSERT(collection->status() == Job::Status_Failed || collection->status() == Job::Status_Aborted);
    }
    freeQueuePolicyResources(self);
    api = nullptr;
}

void Collection_Private::enqueueElements()
{
    Q_ASSERT(!mutex.tryLock());
    prepareToEnqueueElements();
    jobCounter.fetchAndStoreOrdered(elements.count() + 1); // including self
    api->enqueue(elements);
}

void Collection_Private::elementStarted(Collection *, JobPointer job, Thread *)
{
    QMutexLocker l(&mutex);
    Q_UNUSED(l);
    Q_UNUSED(job) // except in Q_ASSERT
    Q_ASSERT(!self.isNull());
    if (jobsStarted.fetchAndAddOrdered(1) == 0) {
        // emit started() signal on beginning of first job execution
        selfExecuteWrapper.callBegin();
    }
}

namespace
{
struct MutexUnlocker {
    QMutexLocker<QMutex> *locker;

    MutexUnlocker(QMutexLocker<QMutex> *l)
        : locker(l)
    {
        locker->unlock();
    }
    ~MutexUnlocker()
    {
        locker->relock();
    }
    MutexUnlocker(const MutexUnlocker &) = delete;
    MutexUnlocker &operator=(const MutexUnlocker &) = delete;
};
}

void Collection_Private::elementFinished(Collection *collection, JobPointer job, Thread *thread)
{
    JobPointer saveYourSelf = self;
    Q_UNUSED(saveYourSelf);
    QMutexLocker l(&mutex);
    Q_UNUSED(l);
    Q_ASSERT(!self.isNull());
    Q_UNUSED(job) // except in Q_ASSERT
    if (selfIsExecuting) {
        // the element that is finished is the collection itself
        // the collection is always executed first
        // No need to queue elements if we were aborted
        if (!collection->shouldAbort()) {
            // queue the collection elements:
            enqueueElements();
        }
        selfIsExecuting = false;
    }
    const int started = jobsStarted.loadAcquire();
    Q_ASSERT(started >= 0);
    Q_UNUSED(started);
    processCompletedElement(collection, job, thread);
    const int remainingJobs = jobCounter.fetchAndAddOrdered(-1) - 1;
    TWDEBUG(4, "Collection_Private::elementFinished: %i\n", remainingJobs);
    if (remainingJobs <= -1) {
        // its no use to count, the elements have been dequeued, now the threads call back that have been processing jobs in the meantime
    } else {
        Q_ASSERT(remainingJobs >= 0);
        if (remainingJobs == 0) {
            // all elements can only be done if self has been executed:
            // there is a small chance that (this) has been dequeued in the
            // meantime, in this case, there is nothing left to clean up
            finalCleanup(collection);
            {
                MutexUnlocker u(&l);
                Q_UNUSED(u);
                selfExecuteWrapper.callEnd();
            }
            self.clear();
        }
    }
}

void Collection_Private::prepareToEnqueueElements()
{
    // empty in Collection
}

JobInterface::Status Collection_Private::updateStatus(Collection *collection, JobPointer job)
{
    // Keep our collection status in running until all jobs have finished
    // but make failures sticky so on first failed job we keep it counting as failure
    auto newStatus = Job::Status_Running;
    const auto jobStatus = job->status();
    if (jobStatus != JobInterface::Status_Success) {
        newStatus = jobStatus;
    }
    collection->setStatus(newStatus);
    return newStatus;
}

void Collection_Private::processCompletedElement(Collection *collection, JobPointer job, Thread *)
{
    updateStatus(collection, job);
}

void Collection_Private::stop(Collection *collection)
{
    QMutexLocker l(&mutex);
    if (api != nullptr) {
        // We can't dequeue ourselves while locked because we will
        // get deadlock when our own aboutToBeDequeued will be invoked
        // which will try to acquire this same lock
        // and we need our own `api` because `finalCleanup` can be invoked in between
        auto currentApi = api;
        l.unlock();
        TWDEBUG(4, "Collection::stop: dequeueing %p.\n", collection);
        if (!currentApi->dequeue(ManagedJobPointer<Collection>(collection))) {
            l.relock();
            dequeueElements(collection, false);
        }
    }
}

void Collection_Private::dequeueElements(Collection *collection, bool queueApiIsLocked)
{
    // dequeue everything:
    Q_ASSERT(!mutex.tryLock());
    if (api == nullptr) {
        return; // not queued
    }

    for (const auto &job : elements) {
        bool result;
        if (queueApiIsLocked) {
            result = api->dequeue_p(job);
        } else {
            result = api->dequeue(job);
        }
        if (result) {
            jobCounter.fetchAndAddOrdered(-1);
        }
        TWDEBUG(3,
                "Collection::Private::dequeueElements: dequeueing %p (%s, %i jobs left).\n",
                (void *)job.data(),
                result ? "found" : "not found",
                jobCounter.loadAcquire());
        elementDequeued(job);
    }

    if (jobCounter.loadAcquire() == 1) {
        finalCleanup(collection);
    }
}

void Collection_Private::requestAbort(Collection *collection)
{
    stop(collection);
    QMutexLocker l(&mutex);
    for (auto job = elements.begin(); job != elements.end(); ++job) {
        if ((*job)->status() <= JobInterface::Status_Running) {
            (*job)->requestAbort();
        }
    }
}

void CollectionSelfExecuteWrapper::begin(const JobPointer &job, Thread *thread)
{
    job_ = job;
    thread_ = thread;
}

void CollectionSelfExecuteWrapper::end(const JobPointer &job, Thread *thread)
{
    Q_ASSERT(job_ == job && thread_ == thread);
    Q_UNUSED(job);
    Q_UNUSED(thread); // except in assert
}

void CollectionSelfExecuteWrapper::callBegin()
{
    ExecuteWrapper::begin(job_, thread_);
}

void CollectionSelfExecuteWrapper::callEnd()
{
    ExecuteWrapper::end(job_, thread_);
    job_.clear();
}

}

}
