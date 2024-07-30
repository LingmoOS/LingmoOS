/* -*- C++ -*-
    This file declares the Collection class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef JOBCOLLECTION_H
#define JOBCOLLECTION_H

#include "job.h"
#include "jobpointer.h"
#include "lambda.h"

#include <functional>

namespace ThreadWeaver
{
class Thread;
class CollectionExecuteWrapper;

namespace Private
{
class Collection_Private;
}

/** A Collection is a vector of Jobs that will be queued together.
 * In a Collection, the order of execution of the elements is not specified.
 *
 * It is intended that the collection is set up first and then
 * queued. After queuing, no further jobs should be added to the collection.
 */
class THREADWEAVER_EXPORT Collection : public Job
{
public:
    Collection();
    Collection(ThreadWeaver::Private::Collection_Private *d);
    ~Collection() override;

    /** Append a job to the collection.
     *
     * To use Collection, create the Job objects first, add them to the collection, and then queue it. After
     * the collection has been queued, no further Jobs are supposed to be added.
     *
     * @note Once the job has been added, execute wrappers can no more be set on it */
    virtual void addJob(JobPointer);

    /** Stop processing, dequeue all remaining Jobs.
     *
     * @since 6.0
     */
    void stop();

    /** Dequeue all remaining Jobs and request abortion of all running jobs
     * @see Job::requestAbort()
     *
     * @since 6.0
     */
    void requestAbort() override;

    /** Return the number of elements in the collection. */
    int elementCount() const;

    /** @brief Add the job to this collection by pointer. */
    Collection &operator<<(ThreadWeaver::JobInterface *job);

    /** @brief Add the job to this collection. */
    Collection &operator<<(const ThreadWeaver::JobPointer &job);
    Collection &operator<<(JobInterface &job);

protected:
    /** Overload to queue the collection. */
    void aboutToBeQueued_locked(QueueAPI *api) override;

    /** Overload to dequeue the collection. */
    void aboutToBeDequeued_locked(QueueAPI *api) override;

    /** Return a ref-erence to the job in the job list at position i. */
    JobPointer jobAt(int i);

    // FIXME remove
    /** Return the number of jobs in the joblist.
     *  Assumes that the mutex is being held.
     */
    virtual int jobListLength_locked() const;

protected:
    /** Overload the execute method. */
    void execute(const JobPointer &job, Thread *) override;

    /** Overload run().
     * We have to. */
    void run(JobPointer self, Thread *thread) override;

protected:
    friend class CollectionExecuteWrapper; // needs to access d()
    friend class Collection_Private;
    ThreadWeaver::Private::Collection_Private *d();
    const ThreadWeaver::Private::Collection_Private *d() const;
};

/**
 * Make a Collection that will execute specified callable (eg. Lambda) for each item in given iterable container
 * You can use it to have a parallel map function.
 */
template<typename Iterable, typename FN>
QSharedPointer<Collection> make_collection(Iterable iterable, FN callable)
{
    QSharedPointer<Collection> collection(new Collection());
    for (auto it = iterable.begin(); it != iterable.end(); ++it) {
        *collection << make_job([callable, item = *it, collectionJob = collection.get()]() {
            callable(item, *collectionJob);
        });
    }
    return collection;
}
}

#endif
