/* -*- C++ -*-
    Class to manipulate job execution in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "job.h"

namespace ThreadWeaver
{
class Job;
class Thread;

// FIXME Pimpl, make part of the API, document
// TODO can JobPointer references be used?
class THREADWEAVER_EXPORT Executor
{
public:
    virtual ~Executor();
    virtual void begin(const JobPointer &, Thread *) = 0;
    void defaultBegin(const JobPointer &job, Thread *thread);
    virtual void execute(const JobPointer &, Thread *) = 0;
    virtual void end(const JobPointer &, Thread *) = 0;
    void defaultEnd(const JobPointer &job, Thread *thread);

    /// @return true when this executor should be owned by the job and deleted alongside it
    virtual bool ownedByJob() const
    {
        return false;
    }

    void run(const JobPointer &job, Thread *thread);
};

}

#endif // EXECUTOR_H
