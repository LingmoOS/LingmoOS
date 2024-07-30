/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef JOB_P_H
#define JOB_P_H

#include "executewrapper_p.h"
#include <QList>
#include <QMutex>

#include <atomic>
#include <functional>

namespace ThreadWeaver
{
namespace Private
{
class THREADWEAVER_EXPORT DefaultExecutor : public ThreadWeaver::Executor
{
public:
    void begin(const JobPointer &job, Thread *thread) override;
    void execute(const JobPointer &job, Thread *thread) override;
    void end(const JobPointer &job, Thread *thread) override;
};

extern DefaultExecutor defaultExecutor;

class DebugExecuteWrapper : public ThreadWeaver::ExecuteWrapper
{
public:
    void execute(const JobPointer &job, ThreadWeaver::Thread *th) override;
};

class Job_Private
{
public:
    Job_Private();
    virtual ~Job_Private();

    /** Free the queue policies acquired before this job has been executed. */
    virtual void freeQueuePolicyResources(JobPointer);

    /* The list of QueuePolicies assigned to this Job. */
    QList<QueuePolicy *> queuePolicies;

    mutable QMutex mutex;
    /* @brief The status of the Job. */
    QAtomicInt status;

    std::atomic_bool shouldAbort;

    /** The Executor that will execute this Job. */
    QAtomicPointer<Executor> executor;

    QList<std::function<void(const JobInterface &job)>> finishHandlers;
    void handleFinish(const JobPointer &job);

    // FIXME What is the correct KDE frameworks no debug switch?
#if !defined(NDEBUG)
    /** DebugExecuteWrapper for logging of Job execution. */
    DebugExecuteWrapper debugExecuteWrapper;
#endif
};

}

}

#endif // JOB_P_H
