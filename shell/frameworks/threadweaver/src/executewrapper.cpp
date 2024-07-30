/* -*- C++ -*-
    Class to manipulate job execution in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "executewrapper_p.h"

namespace ThreadWeaver
{
ExecuteWrapper::ExecuteWrapper()
{
}

ExecuteWrapper::~ExecuteWrapper()
{
    auto wrapped = this->wrapped.loadAcquire();
    if (wrapped && wrapped->ownedByJob())
        delete wrapped;
}

Executor *ExecuteWrapper::wrap(Executor *previous)
{
    return wrapped.fetchAndStoreOrdered(previous);
}

Executor *ExecuteWrapper::unwrap(JobInterface *job)
{
    Executor *executor = job->setExecutor(wrapped.fetchAndStoreOrdered(nullptr));
    Q_ASSERT_X(executor == this, Q_FUNC_INFO, "ExecuteWrapper can only unwrap itself!");
    return executor;
}

void ExecuteWrapper::begin(const JobPointer &job, Thread *thread)
{
    Q_ASSERT(wrapped.loadAcquire() != nullptr);
    wrapped.loadAcquire()->begin(job, thread);
}

void ExecuteWrapper::execute(const JobPointer &job, Thread *thread)
{
    executeWrapped(job, thread);
}

void ExecuteWrapper::executeWrapped(const JobPointer &job, Thread *thread)
{
    Executor *executor = wrapped.loadAcquire();
    Q_ASSERT_X(executor != nullptr, Q_FUNC_INFO, "Wrapped Executor cannot be zero!");
    executor->execute(job, thread);
}

void ExecuteWrapper::end(const JobPointer &job, Thread *thread)
{
    Q_ASSERT(wrapped.loadAcquire() != nullptr);
    wrapped.loadAcquire()->end(job, thread);
}

}

#include "executewrapper_p.h"
