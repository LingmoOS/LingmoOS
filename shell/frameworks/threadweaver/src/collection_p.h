/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COLLECTION_COLLECTION_P_H
#define COLLECTION_COLLECTION_P_H

#include <QList>
#include <QMutex>

#include "executewrapper_p.h"
#include "job_p.h"

namespace ThreadWeaver
{
class Collection;

namespace Private
{
class CollectionSelfExecuteWrapper : public ThreadWeaver::ExecuteWrapper
{
public:
    void begin(const JobPointer &, Thread *) override;
    void end(const JobPointer &, Thread *) override;

    void callBegin();
    void callEnd();

private:
    JobPointer job_;
    Thread *thread_;
};

class Collection_Private : public Job_Private
{
public:
    Collection_Private();
    ~Collection_Private() override;

    /** Dequeue all elements of the collection.
     * Note: This will not dequeue the collection itself.
     */
    void dequeueElements(Collection *collection, bool queueApiIsLocked);

    /** Perform the task usually done when one individual job is
     * finished, but in our case only when the whole collection
     * is finished or partly dequeued.
     */
    void finalCleanup(Collection *collection);

    /** @brief Enqueue the elements of the collection. */
    void enqueueElements();

    void elementStarted(Collection *collection, JobPointer, Thread *);
    void elementFinished(Collection *collection, JobPointer job, Thread *thread);

    /** @brief Prepare to enqueue the elements. */
    virtual void prepareToEnqueueElements();

    virtual JobInterface::Status updateStatus(Collection *collection, JobPointer job);

    /** @brief Process a completed element. */
    virtual void processCompletedElement(Collection *collection, JobPointer job, Thread *thread);

    /** @brief Implement stop. */
    void stop(Collection *collection);

    void requestAbort(Collection *collection);

    /** @brief Called before an element will be dequeued. */
    virtual void elementDequeued(const JobPointer &)
    {
    }

    /* The elements of the collection. */
    QList<JobPointer> elements;

    /* The Weaver interface this collection is queued in. */
    QueueAPI *api;

    /* Counter for the finished jobs.
       Set to the number of elements when started.
       When zero, all elements are done.
    */
    QAtomicInt jobCounter;
    QAtomicInt jobsStarted;
    CollectionSelfExecuteWrapper selfExecuteWrapper;
    JobPointer self;
    bool selfIsExecuting;
};

}

}

#endif // COLLECTION_COLLECTION_P_H
