/* -*- C++ -*-
    Interface for jobs in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef JOBINTERFACE_H
#define JOBINTERFACE_H

#include <QSharedPointer>

#include "jobpointer.h"
#include "threadweaver_export.h"

class QMutex;

namespace ThreadWeaver
{
class Thread;
class Executor;
class QueueAPI;
class QueuePolicy;

class JobInterface;
typedef QSharedPointer<JobInterface> JobPointer;

class THREADWEAVER_EXPORT JobInterface
{
public:
    enum Status {
        Status_NoStatus = 0,
        Status_New,
        Status_Queued,
        Status_Running,
        Status_Success,
        Status_Failed,
        Status_Aborted,
        Status_NumberOfStatuses,
    };

    virtual ~JobInterface()
    {
    }
    virtual void execute(const JobPointer &job, Thread *) = 0;
    virtual void blockingExecute() = 0;
    virtual Executor *setExecutor(Executor *executor) = 0;
    virtual Executor *executor() const = 0;
    virtual int priority() const = 0;
    virtual Status status() const = 0;
    virtual void setStatus(Status) = 0;
    virtual bool success() const = 0;
    virtual void requestAbort() = 0;
    virtual void aboutToBeQueued(QueueAPI *api) = 0;
    virtual void aboutToBeQueued_locked(QueueAPI *api) = 0;
    virtual void aboutToBeDequeued(QueueAPI *api) = 0;
    virtual void aboutToBeDequeued_locked(QueueAPI *api) = 0;
    virtual bool isFinished() const = 0;
    virtual void assignQueuePolicy(QueuePolicy *) = 0;
    virtual void removeQueuePolicy(QueuePolicy *) = 0;
    virtual QList<QueuePolicy *> queuePolicies() const = 0;
    virtual void run(JobPointer self, Thread *thread) = 0;
    friend class Executor;
    virtual void defaultBegin(const JobPointer &job, Thread *thread) = 0;
    virtual void defaultEnd(const JobPointer &job, Thread *thread) = 0;
    virtual QMutex *mutex() const = 0;
};

}

Q_DECLARE_METATYPE(ThreadWeaver::JobPointer)

#endif // JOBINTERFACE_H
