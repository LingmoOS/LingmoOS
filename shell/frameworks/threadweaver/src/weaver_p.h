/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WEAVER_P_H
#define WEAVER_P_H

#include <QSemaphore>
#include <QSharedPointer>
#include <QWaitCondition>

#include "jobpointer.h"
#include "state.h"
#include "thread.h"

#include "queuesignals_p.h"

namespace ThreadWeaver
{
namespace Private
{
class Weaver_Private : public QueueSignals_Private
{
public:
    Weaver_Private();
    ~Weaver_Private() override;

    void dumpJobs();

    bool canBeExecuted(JobPointer);
    void deleteExpiredThreads();

    /** The thread inventory. */
    QList<Thread *> inventory;
    /** Threads that have exited and can be deleted. */
    QList<Thread *> expiredThreads;
    /** The job queue. */
    QList<JobPointer> assignments;
    /** The number of jobs that are assigned to the worker threads, but not finished. */
    int active;
    /** The maximum number of worker threads. */
    int inventoryMax;
    /** Wait condition all idle or done threads wait for. */
    QWaitCondition jobAvailable;
    /** Wait for a job to finish. */
    QWaitCondition jobFinished;
    /** Mutex to serialize operations. */
    QMutex *mutex;
    /** Semaphore to ensure thread startup is in sequence. */
    QSemaphore semaphore;
    /** Before shutdown can proceed to close the running threads, it needs to ensure that all of them
     *  entered the run method. */
    QAtomicInt createdThreads;
    /** The state of the art.
     * @see StateId
     */
    QAtomicPointer<State> state;
    /** The state objects. */
    QSharedPointer<State> states[NoOfStates];
};

}

}

#endif // WEAVER_P_H
