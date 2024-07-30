/* -*- C++ -*-
    This file declares the Job class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: Job.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef THREADWEAVER_JOB_H
#define THREADWEAVER_JOB_H

#include "jobinterface.h"
#include "jobpointer.h"
#include "threadweaver_export.h"

class QMutex;
class QWaitCondition;

namespace ThreadWeaver
{
namespace Private
{
class Job_Private;
}

class Thread;
class QueuePolicy;
class QueueAPI;
class Executor;

/** @brief A Job is a simple abstraction of an action that is to be executed in a thread context.
 *
 * It is essential for the ThreadWeaver library that as a kind of convention, the different creators of Job objects do not touch
 * the protected data members of the Job until somehow notified by the Job.
 *
 * Jobs may not be executed twice. Create two different objects to perform two consecutive or parallel runs.
 * (Note: this rule is being reconsidered.)
 *
 * Jobs may declare dependencies. If Job B depends on Job A, B may not be executed before A is finished. To learn about
 * dependencies, see DependencyPolicy.
 *
 * Job objects do not inherit QObject. To connect to signals when jobs are started or finished, see QObjectDecorator.
 */
class THREADWEAVER_EXPORT Job : public JobInterface
{
public:
    /** Construct a Job. */
    Job();
    Job(Private::Job_Private *d);

    /** Destructor. */
    ~Job() override;

    /** Perform the job. The thread in which this job is executed is given as a parameter.
     *
     * Do not overload this method to create your own Job implementation, overload run().
     * Whenever the currently executed job is communicated to the outside world, use the supplied job pointer
     * to keep the reference count correct.
     *
     * job is the Job that the queue is executing. It is not necessarily equal to this. For example, Jobs that are
     * decorated expose the decorator's address, not the address of the decorated object.
     */
    void execute(const JobPointer &job, Thread *) override;

    /** Perform the job synchronously in the current thread. */
    void blockingExecute() override;

    /** Set the Executor object that is supposed to run the job.
     *
     * Returns the previously set executor. The executor can never be unset. If zero is passed in as the new executor, the Job
     * will internally reset to a default executor that only invokes run().
     */
    Executor *setExecutor(Executor *executor) override;

    /** Returns the executor currently set on the Job. */
    Executor *executor() const override;

    /** The queueing priority of the job.
     * Jobs will be sorted by their queueing priority when enqueued. A higher queueing priority will place the job in front of all
     * lower-priority jobs in the queue.
     *
     * Note: A higher or lower priority does not influence queue policies. For example, a high-priority job that has an unresolved
     * dependency will not be executed, which means an available lower-priority job will take precedence.
     *
     * The default implementation returns zero. Only if this method is overloaded for some job classes, priorities will influence
     * the execution order of jobs. */
    int priority() const override;

    /** @brief Set the status of the Job.
     *
     * Do not call this method unless you know what you are doing, please :-) */
    void setStatus(Status) override;

    /** @brief The status of the job.
     *
     * The status will be changed to Status_Success if the run() method exits normally.
     */
    Status status() const override;

    /** Return whether the Job finished successfully or not.
     * The default implementation simply returns true. Overload in derived classes if the derived Job class can fail.
     *
     * If a job fails (success() returns false), it will *NOT* resolve its dependencies when it finishes. This will make sure that
     * Jobs that depend on the failed job will not be started.
     *
     * There is an important gotcha: When a Job object it deleted, it will always resolve its dependencies. If dependent jobs should
     * not be executed after a failure, it is important to dequeue those before deleting the failed Job. A Sequence may be
     * helpful for that purpose.
     */
    bool success() const override;

    /** Abort the execution of the job.
     *
     * Call this method to ask the Job to abort if it is currently executed. Default implementation of the method sets a flag
     * causing `shouldAbort()` return true. You can reimplement this method to actually initiate an abort action.
     *
     * This method is supposed to return immediately, not after the abort has completed. It requests the abort, the Job has to act on
     * the request. */
    void requestAbort() override;

    /** @brief Whether Job should abort itself
     *
     * It will return true if `requestAbort()` was invoked before
     * but it's up to the job implementation itself to honor it
     * and some implementations might not actually abort (ie. unabortable job).
     *
     * @threadsafe
     *
     * @since 6.0
     */
    bool shouldAbort() const;

    /** The job is about to be added to the weaver's job queue.
     *
     * The job will be added right after this method finished. The default implementation does nothing. Use this method to, for
     * example, queue sub-operations as jobs before the job itself is queued.
     *
     * Note: When this method is called, the associated Weaver object's thread holds a lock on the weaver's queue. Therefore, it
     * is save to assume that recursive queueing is atomic from the queues perspective.
     *
     * @param api the QueueAPI object the job will be queued in */
    void aboutToBeQueued(QueueAPI *api) override;

    /** Called from aboutToBeQueued() while the mutex is being held. */
    void aboutToBeQueued_locked(QueueAPI *api) override;

    /** This Job is about the be dequeued from the weaver's job queue.
     *
     * The job will be removed from the queue right after this method returns. Use this method to dequeue, if necessary,
     * sub-operations (jobs) that this job has enqueued.
     *
     * Note: When this method is called, the associated Weaver object's thread does hold a lock on the weaver's queue.
     * Note: The default implementation does nothing.
     *
     * @param weaver the Weaver object from which the job will be dequeued */
    void aboutToBeDequeued(QueueAPI *api) override;

    /** Called from aboutToBeDequeued() while the mutex is being held. */
    void aboutToBeDequeued_locked(QueueAPI *api) override;

    /** Returns true if the jobs's execute method finished. */
    bool isFinished() const override;

    /** Add handler that will be invoked once job has finished
     *
     * @since 6.0
     */
    void onFinish(const std::function<void(const JobInterface &job)> &lambda);

    /** Assign a queue policy.
     *
     * Queue Policies customize the queueing (running) behaviour of sets of jobs. Examples for queue policies are dependencies
     * and resource restrictions. Every queue policy object can only be assigned once to a job, multiple assignments will be
     * IGNORED. */
    void assignQueuePolicy(QueuePolicy *) override;

    /** Remove a queue policy from this job. */
    void removeQueuePolicy(QueuePolicy *) override;

    /** @brief Return the queue policies assigned to this Job. */
    QList<QueuePolicy *> queuePolicies() const override;

    /** The mutex used to protect this job. */
    QMutex *mutex() const override;

private:
    Private::Job_Private *d_;

protected:
    Private::Job_Private *d();
    const Private::Job_Private *d() const;

    friend class Executor;
    /** The method that actually performs the job.
     *
     * It is called from execute(). This method is the one to overload it with the job's task.
     *
     * The Job will be executed in the specified thread. thread may be zero, indicating that the job is being executed some
     * other way (for example, synchronously by some other job). self specifies the job as the queue sees it. Whenever publishing
     * information about the job to the outside world, for example by emitting signals, use self, not this. self is the reference
     * counted object handled by the queue. Using it as signal parameters will amongst other things prevent thejob from being
     * memory managed and deleted.
     */
    virtual void run(JobPointer self, Thread *thread) override = 0;

    /** @brief Perform standard tasks before starting the execution of a job.
     *
     * The default implementation is empty.
     * job is the Job that the queue is executing. It is not necessarily equal to this. For example, Jobs that are
     * decorated expose the decorator's address, not the address of the decorated object. */
    void defaultBegin(const JobPointer &job, Thread *thread) override;

    /** @brief Perform standard task after the execution of a job.
     *
     * The default implementation is empty.
     * job is the Job that the queue is executing. It is not necessarily equal to this. For example, Jobs that are
     * decorated expose the decorator's address, not the address of the decorated object. */
    void defaultEnd(const JobPointer &job, Thread *thread) override;
};

}

#endif // THREADWEAVER_JOB_H
