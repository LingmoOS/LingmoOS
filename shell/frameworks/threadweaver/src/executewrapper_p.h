/* -*- C++ -*-
    Class to manipulate job execution in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EXECUTEWRAPPER_H
#define EXECUTEWRAPPER_H

#include <QAtomicPointer>

#include "executor_p.h"
#include "job.h"

namespace ThreadWeaver
{
class Job;
class Thread;

// FIXME Pimpl, make part of the API, document
// Find a way to avoid new/delete operation, this is rather performance sensitive area
class ExecuteWrapper : public Executor
{
public:
    ExecuteWrapper();
    ~ExecuteWrapper();
    Executor *wrap(Executor *previous);
    Executor *unwrap(JobInterface *job);
    void begin(const JobPointer &job, Thread *) override;
    void execute(const JobPointer &job, Thread *thread) override;
    void executeWrapped(const JobPointer &job, Thread *thread);
    void end(const JobPointer &job, Thread *) override;

private:
    QAtomicPointer<Executor> wrapped;
};

}

#endif // EXECUTEWRAPPER_H
