/* -*- C++ -*-
    Base class for job decorators in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef THREADWEAVER_IDDECORATOR_H
#define THREADWEAVER_IDDECORATOR_H

#include <QObject>

#include "jobinterface.h"
#include "threadweaver_export.h"

namespace ThreadWeaver
{
class Collection;
class Sequence;

/** @brief IdDecorator decorates a job without changing it's behaviour.
 *
 *  It is supposed to be used as the base class for actual decorators that do change the behaviour of jobs. */
class THREADWEAVER_EXPORT IdDecorator : public JobInterface
{
public:
    explicit IdDecorator(JobInterface *job, bool autoDelete = true);
    ~IdDecorator() override;
    /** Retrieve the decorated job. */
    const JobInterface *job() const;
    /** Retrieve the decorated job. */
    JobInterface *job();
    /** Auto-delete the decoratee or not. */
    void setAutoDelete(bool onOff);
    /** Will the decoratee be auto-deleted? */
    bool autoDelete() const;
    /** Retrieve the decorated job as a Collection.
     *  If the decorated Job is not a Collection, 0 is returned. */
    const Collection *collection() const;
    /** Retrieve the decorated job as a Collection.
     *  If the decorated Job is not a Collection, 0 is returned. */
    Collection *collection();
    /** Retrieve the decorated job as a Sequence.
     *  If the decorated Job is not a Sequence, 0 is returned. */
    const Sequence *sequence() const;
    /** Retrieve the decorated job as a Sequence.
     *  If the decorated Job is not a Sequence, 0 is returned. */
    Sequence *sequence();

    void execute(const JobPointer &job, Thread *) override;
    void blockingExecute() override;
    Executor *setExecutor(Executor *executor) override;
    Executor *executor() const override;
    int priority() const override;
    void setStatus(Status) override;
    Status status() const override;
    bool success() const override;
    void requestAbort() override;
    void aboutToBeQueued(QueueAPI *api) override;
    void aboutToBeQueued_locked(QueueAPI *api) override;
    void aboutToBeDequeued(QueueAPI *api) override;
    void aboutToBeDequeued_locked(QueueAPI *api) override;
    bool isFinished() const override;
    void assignQueuePolicy(QueuePolicy *) override;
    void removeQueuePolicy(QueuePolicy *) override;
    QList<QueuePolicy *> queuePolicies() const override;
    QMutex *mutex() const override;

protected:
    void run(JobPointer self, Thread *thread) override;
    void defaultBegin(const JobPointer &job, Thread *thread) override;
    void defaultEnd(const JobPointer &job, Thread *thread) override;

private:
    class Private1;
    Private1 *const d1;
    class Private2;
    Private2 *d2;
};

}

#endif // THREADWEAVER_IDDECORATOR_H
