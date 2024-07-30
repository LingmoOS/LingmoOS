/* -*- C++ -*-
    This file implements the public interfaces of the WeaverImpl class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WeaverImpl_H
#define WeaverImpl_H

#include "queueapi.h"
#include <QObject>

namespace ThreadWeaver
{
class State;
class Job;
class Thread;
class WeaverImplState;
class SuspendingState;

namespace Private
{
class Weaver_Private;
}

/** @brief A Weaver manages worker threads.
 *
 * It creates an inventory of Thread objects to which it assigns jobs from its queue.
 * It extends the API of Queue, hiding methods that need to be public to implement state handling, but
 * should not be exposed in general.
 */
class THREADWEAVER_EXPORT Weaver : public QueueAPI
{
    Q_OBJECT
public:
    explicit Weaver(QObject *parent = nullptr);
    ~Weaver() override;
    void shutDown() override;
    void shutDown_p() override;

    const State *state() const override;
    State *state() override;

    void setMaximumNumberOfThreads(int cap) override;
    int maximumNumberOfThreads() const override;
    int currentNumberOfThreads() const override;

    void setState(StateId);
    void enqueue(const QList<JobPointer> &jobs) override;
    bool dequeue(const JobPointer &job) override;
    void dequeue() override;
    void finish() override;
    void suspend() override;
    void resume() override;
    bool isEmpty() const override;
    bool isIdle() const override;
    int queueLength() const override;
    JobPointer applyForWork(Thread *thread, bool wasBusy) override;
    void waitForAvailableJob(Thread *th) override;
    void blockThreadUntilJobsAreBeingAssigned(Thread *th);
    void blockThreadUntilJobsAreBeingAssigned_locked(Thread *th);
    void incActiveThreadCount();
    void decActiveThreadCount();
    int activeThreadCount();

    void threadEnteredRun(Thread *thread);
    JobPointer takeFirstAvailableJobOrSuspendOrWait(Thread *th, bool threadWasBusy, bool suspendIfAllThreadsInactive, bool justReturning);
    void requestAbort() override;
    void reschedule() override;

    // FIXME: rename _p to _locked:
    friend class WeaverImplState;
    friend class SuspendingState;
    void setState_p(StateId);
    void setMaximumNumberOfThreads_p(int cap) override;
    int maximumNumberOfThreads_p() const override;
    int currentNumberOfThreads_p() const override;
    void enqueue_p(const QList<JobPointer> &jobs);
    bool dequeue_p(JobPointer job) override;
    void dequeue_p() override;
    void finish_p() override;
    void suspend_p() override;
    void resume_p() override;
    bool isEmpty_p() const override;
    bool isIdle_p() const override;
    int queueLength_p() const override;
    void requestAbort_p() override;

Q_SIGNALS:
    /** @brief A Thread has been created. */
    void threadStarted(ThreadWeaver::Thread *);
    /** @brief A thread has exited. */
    void threadExited(ThreadWeaver::Thread *);
    /** @brief A thread has been suspended. */
    void threadSuspended(ThreadWeaver::Thread *);

protected:
    void adjustActiveThreadCount(int diff);
    virtual Thread *createThread();
    void adjustInventory(int noOfNewJobs);

private:
    ThreadWeaver::Private::Weaver_Private *d();
    const ThreadWeaver::Private::Weaver_Private *d() const;
};

} // namespace ThreadWeaver

#endif // WeaverImpl_H
