/* -*- C++ -*-
    This file implements the Collection class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collection.h"

#include "collection_p.h"
#include "debuggingaids.h"
#include "queueapi.h"
#include "queueing.h"

#include <QList>
#include <QObject>
#include <QPointer>

#include "dependencypolicy.h"
#include "executewrapper_p.h"
#include "thread.h"

namespace ThreadWeaver
{
class CollectionExecuteWrapper : public ExecuteWrapper
{
public:
    CollectionExecuteWrapper()
        : collection(nullptr)
    {
    }

    void setCollection(Collection *collection_)
    {
        collection = collection_;
    }

    void begin(const JobPointer &job, Thread *thread) override
    {
        TWDEBUG(4, "CollectionExecuteWrapper::begin: collection %p\n", collection);
        ExecuteWrapper::begin(job, thread);
        Q_ASSERT(collection);
        collection->d()->elementStarted(collection, job, thread);
        ExecuteWrapper::begin(job, thread);
    }

    void end(const JobPointer &job, Thread *thread) override
    {
        TWDEBUG(4, "CollectionExecuteWrapper::end: collection %p\n", collection);
        Q_ASSERT(collection);
        ExecuteWrapper::end(job, thread);
        collection->d()->elementFinished(collection, job, thread);
    }

    bool ownedByJob() const override
    {
        return true;
    }

private:
    ThreadWeaver::Collection *collection;
};

Collection::Collection()
    : Job(new Private::Collection_Private)
{
}

Collection::Collection(Private::Collection_Private *d__)
    : Job(d__)
{
}

Collection::~Collection()
{
    MUTEX_ASSERT_UNLOCKED(mutex());
    // dequeue all remaining jobs:
    QMutexLocker l(mutex());
    Q_UNUSED(l);
    if (d()->api != nullptr) { // still queued
        d()->dequeueElements(this, false);
    }
}

void Collection::addJob(JobPointer job)
{
    QMutexLocker l(mutex());
    Q_UNUSED(l);
    REQUIRE(d()->api == nullptr || d()->selfIsExecuting == true); // not queued yet or still running
    REQUIRE(job != nullptr);

    CollectionExecuteWrapper *wrapper = new CollectionExecuteWrapper();
    wrapper->setCollection(this);
    wrapper->wrap(job->setExecutor(wrapper));
    d()->elements.append(job);
}

void Collection::stop()
{
    d()->stop(this);
}

void Collection::requestAbort()
{
    Job::requestAbort();
    d()->requestAbort(this);
}

void Collection::aboutToBeQueued_locked(QueueAPI *api)
{
    Q_ASSERT(!mutex()->tryLock());
    Q_ASSERT(d()->api == nullptr); // never queue twice
    d()->api = api;
    d()->selfExecuteWrapper.wrap(setExecutor(&d()->selfExecuteWrapper));
    CollectionExecuteWrapper *wrapper = new CollectionExecuteWrapper();
    wrapper->setCollection(this);
    wrapper->wrap(setExecutor(wrapper));
    Job::aboutToBeQueued_locked(api);
}

void Collection::aboutToBeDequeued_locked(QueueAPI *api)
{
    Q_ASSERT(!mutex()->tryLock());
    Q_ASSERT(api && d()->api == api);
    d()->dequeueElements(this, true);
    d()->api = nullptr;

    Q_ASSERT(dynamic_cast<CollectionExecuteWrapper *>(executor()));
    auto wrapper = static_cast<CollectionExecuteWrapper *>(executor());
    wrapper->unwrap(this);
    delete wrapper;

    Q_ASSERT(executor() == &d()->selfExecuteWrapper);
    d()->selfExecuteWrapper.unwrap(this);

    Job::aboutToBeDequeued_locked(api);
}

void Collection::execute(const JobPointer &job, Thread *thread)
{
    {
        QMutexLocker l(mutex());
        Q_UNUSED(l);
        Q_ASSERT(d()->self.isNull());
        Q_ASSERT(d()->api != nullptr);
        d()->self = job;
        d()->selfIsExecuting = true; // reset in elementFinished
    }
    Job::execute(job, thread);
}

void Collection::run(JobPointer, Thread *)
{
    // empty
}

Private::Collection_Private *Collection::d()
{
    return reinterpret_cast<Private::Collection_Private *>(Job::d());
}

const Private::Collection_Private *Collection::d() const
{
    return reinterpret_cast<const Private::Collection_Private *>(Job::d());
}

JobPointer Collection::jobAt(int i)
{
    Q_ASSERT(!mutex()->tryLock());
    Q_ASSERT(i >= 0 && i < d()->elements.size());
    return d()->elements.at(i);
}

int Collection::elementCount() const
{
    QMutexLocker l(mutex());
    Q_UNUSED(l);
    return jobListLength_locked();
}

int Collection::jobListLength_locked() const
{
    return d()->elements.size();
}

Collection &Collection::operator<<(JobInterface *job)
{
    addJob(make_job(job));
    return *this;
}

Collection &Collection::operator<<(const JobPointer &job)
{
    addJob(job);
    return *this;
}

Collection &Collection::operator<<(JobInterface &job)
{
    addJob(make_job_raw(&job));
    return *this;
}

}
