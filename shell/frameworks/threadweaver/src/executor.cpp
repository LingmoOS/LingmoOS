/* -*- C++ -*-
    Class to manipulate job execution in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "executor_p.h"

namespace ThreadWeaver
{
Executor::~Executor()
{
}

void Executor::defaultBegin(const JobPointer &job, Thread *thread)
{
    job->defaultBegin(job, thread);
}

void Executor::defaultEnd(const JobPointer &job, Thread *thread)
{
    job->defaultEnd(job, thread);
}

void Executor::run(const JobPointer &job, Thread *thread)
{
    job->run(job, thread);
}

}
