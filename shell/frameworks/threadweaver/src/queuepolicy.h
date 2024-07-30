/* -*- C++ -*-
    This file declares the QueuePolicy class.

    SPDX-FileCopyrightText: 2004, 2005, 2006 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef QUEUEPOLICY_H
#define QUEUEPOLICY_H

#include "jobpointer.h"
#include "threadweaver_export.h"

namespace ThreadWeaver
{
class JobInterface;

/** @brief QueuePolicy is an interface for customizations of the queueing behaviour of jobs.
 *
 *  A job can have a number of queue policies assigned. In that case, the job is only
 *  executed when the method canRun() of all assigned policies return true. For every call to
 *  canRun() that returns true, it is guaranteed that the method free() or the method release()
 *  is called. Calling free() means the job has been executed, while calling release() means
 *  the job was not executed for external reasons, and will be tried later on.
 *
 *  As an example, dependencies can be implemented using a QueuePolicy: canRun() returns true
 *  when the job has no unresolved dependencies. free() and release() are empty.
 *
 *  A job can have multiple queue policies assigned, and will only be executed if all of them
 *  return true from canRun() within the same execution attempt. Jobs only keep a reference to the
 *  QueuePolicy. Therefore, the same policy object can be assigned to multiple jobs and this way
 *  control the way all those jobs are executed. Jobs never assume ownership of their assigned queue
 *  policies.
 */
class THREADWEAVER_EXPORT QueuePolicy
{
public:
    virtual ~QueuePolicy()
    {
    }

    /** @brief canRun() is called before the job is executed.
     *  The job will only be executed if canRun() returns true.
     */
    virtual bool canRun(JobPointer) = 0;

    /** @brief free() is called after the job has been executed.
     *  It is guaranteed that free is called only after canRun()
     *  returned true at an earlier time.
     */
    virtual void free(JobPointer) = 0;

    /** @brief release() is called if canRun() returned true, but the job has not been executed for external reasons.
     *
     *  For example, a second QueuePolicy could have returned false from canRun() for the same job.
     */
    virtual void release(JobPointer) = 0;

    /** @brief destructing() is called when a Job that has this queue policy assigned gets destructed.
     */
    virtual void destructed(JobInterface *job) = 0;
};

}

#endif
