/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "job_p.h"
#include "debuggingaids.h"
#include "executor_p.h"
#include "queuepolicy.h"
#include "thread.h"

ThreadWeaver::Private::DefaultExecutor ThreadWeaver::Private::defaultExecutor;

ThreadWeaver::Private::Job_Private::Job_Private()
    : status(Job::Status_NoStatus)
    , shouldAbort(false)
    , executor(&defaultExecutor)
{
}

ThreadWeaver::Private::Job_Private::~Job_Private()
{
    auto executor = this->executor.loadAcquire();
    if (executor->ownedByJob())
        delete executor;
}

void ThreadWeaver::Private::Job_Private::freeQueuePolicyResources(JobPointer job)
{
    for (int index = 0; index < queuePolicies.size(); ++index) {
        queuePolicies.at(index)->free(job);
    }
}

void ThreadWeaver::Private::Job_Private::handleFinish(const JobPointer &job)
{
    QMutexLocker l(&mutex);
    QList<std::function<void(const JobInterface &job)>> handlers = finishHandlers;
    // We need to unlock mutex because
    // handlers might want to use Job methods that also need this same mutex
    l.unlock();
    for (auto handler : handlers) {
        handler(*job.get());
    }
}

void ThreadWeaver::Private::DebugExecuteWrapper::execute(const JobPointer &job, ThreadWeaver::Thread *th)
{
    Q_ASSERT_X(job, Q_FUNC_INFO, "job may not be zero!");
    TWDEBUG(3, "DefaultExecuteWrapper::execute: executing job %p in thread %i.\n", job.data(), th ? th->id() : 0);
    executeWrapped(job, th);
    TWDEBUG(3, "Job::execute: finished execution of job in thread %i.\n", th ? th->id() : 0);
}

void ThreadWeaver::Private::DefaultExecutor::begin(const JobPointer &job, Thread *thread)
{
    defaultBegin(job, thread);
}

void ThreadWeaver::Private::DefaultExecutor::execute(const JobPointer &job, Thread *thread)
{
    run(job, thread);
}

void ThreadWeaver::Private::DefaultExecutor::end(const JobPointer &job, Thread *thread)
{
    defaultEnd(job, thread);
}
