/* -*- C++ -*-
    Base class for job decorators in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collection.h"
#include "managedjobpointer.h"
#include "sequence.h"

#include "iddecorator.h"

namespace
{
const quintptr IdDecorator_AutoDelete = 1;

}

namespace ThreadWeaver
{
// Pssst: IdDecorator uses the d pointer to hold decoratee. It also uses d2 as a bitfield to store the
// autoDelete setting. The goal is not to require a dynamic allocation on creation.
IdDecorator::IdDecorator(JobInterface *decoratee, bool autoDelete)
    : d1(reinterpret_cast<Private1 *>(decoratee))
    , d2(nullptr)
{
    setAutoDelete(autoDelete);
}

IdDecorator::~IdDecorator()
{
    // Do not assert here. IdDecorator can decorate a null pointer. Only assert if a method is called on a decorated
    // null  pointer.
    if (autoDelete()) {
        delete job();
    }
}

QMutex *IdDecorator::mutex() const
{
    Q_ASSERT(d1);
    return job()->mutex();
}

void IdDecorator::run(JobPointer self, Thread *thread)
{
    Q_ASSERT(d1);
    job()->run(self, thread);
}

void IdDecorator::defaultBegin(const JobPointer &self, Thread *thread)
{
    Q_ASSERT(d1);
    job()->defaultBegin(self, thread);
}

void IdDecorator::defaultEnd(const JobPointer &self, Thread *thread)
{
    Q_ASSERT(d1);
    job()->defaultEnd(self, thread);
}

void IdDecorator::removeQueuePolicy(QueuePolicy *policy)
{
    Q_ASSERT(d1);
    job()->removeQueuePolicy(policy);
}

QList<QueuePolicy *> IdDecorator::queuePolicies() const
{
    Q_ASSERT(d1);
    return job()->queuePolicies();
}

void IdDecorator::assignQueuePolicy(QueuePolicy *policy)
{
    Q_ASSERT(d1);
    job()->assignQueuePolicy(policy);
}

bool IdDecorator::isFinished() const
{
    Q_ASSERT(d1);
    return job()->isFinished();
}

void IdDecorator::aboutToBeQueued(QueueAPI *api)
{
    Q_ASSERT(d1);
    job()->aboutToBeQueued(api);
}

void IdDecorator::aboutToBeQueued_locked(QueueAPI *api)
{
    Q_ASSERT(d1);
    job()->aboutToBeQueued_locked(api);
}

void IdDecorator::aboutToBeDequeued(QueueAPI *api)
{
    Q_ASSERT(d1);
    job()->aboutToBeDequeued(api);
}

void IdDecorator::aboutToBeDequeued_locked(QueueAPI *api)
{
    Q_ASSERT(d1);
    job()->aboutToBeDequeued_locked(api);
}

void IdDecorator::requestAbort()
{
    Q_ASSERT(d1);
    job()->requestAbort();
}

bool IdDecorator::success() const
{
    Q_ASSERT(d1);
    return job()->success();
}

int IdDecorator::priority() const
{
    Q_ASSERT(d1);
    return job()->priority();
}

void IdDecorator::setStatus(JobInterface::Status status)
{
    Q_ASSERT(d1);
    job()->setStatus(status);
}

JobInterface::Status IdDecorator::status() const
{
    Q_ASSERT(d1);
    return job()->status();
}

Executor *IdDecorator::executor() const
{
    Q_ASSERT(d1);
    return job()->executor();
}

Executor *IdDecorator::setExecutor(Executor *executor)
{
    Q_ASSERT(d1);
    return job()->setExecutor(executor);
}

void IdDecorator::execute(const JobPointer &self, ThreadWeaver::Thread *thread)
{
    Q_ASSERT(d1);
    job()->execute(self, thread);
}

void IdDecorator::blockingExecute()
{
    Q_ASSERT(d1);
    job()->blockingExecute();
}

const ThreadWeaver::JobInterface *IdDecorator::job() const
{
    return reinterpret_cast<JobInterface *>(d1);
}

JobInterface *IdDecorator::job()
{
    return reinterpret_cast<JobInterface *>(d1);
}

void IdDecorator::setAutoDelete(bool onOff)
{
    if (onOff) {
        d2 = reinterpret_cast<IdDecorator::Private2 *>(IdDecorator_AutoDelete);
    } else {
        d2 = nullptr;
    }
}

bool IdDecorator::autoDelete() const
{
    return d2 == reinterpret_cast<IdDecorator::Private2 *>(IdDecorator_AutoDelete);
}

const ThreadWeaver::Collection *IdDecorator::collection() const
{
    return dynamic_cast<const Collection *>(job());
}

Collection *IdDecorator::collection()
{
    return dynamic_cast<Collection *>(job());
}

const Sequence *IdDecorator::sequence() const
{
    return dynamic_cast<const Sequence *>(job());
}

Sequence *IdDecorator::sequence()
{
    return dynamic_cast<Sequence *>(job());
}

}
