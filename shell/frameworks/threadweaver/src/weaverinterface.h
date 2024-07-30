/* -*- C++ -*-
    This file declares the WeaverInterface class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WEAVER_INTERFACE_H
#define WEAVER_INTERFACE_H

#include "jobpointer.h"

namespace ThreadWeaver
{
class Job;
class Thread;

class THREADWEAVER_EXPORT WeaverInterface
{
public:
    virtual ~WeaverInterface()
    {
    }

    /** @brief Assign a job to an idle thread.
     * @param th the thread to give a new Job to
     * @param wasBusy true if a job was previously assigned to the calling thread
     */
    virtual JobPointer applyForWork(Thread *th, bool wasBusy) = 0;

    /** @brief Wait (by suspending the calling thread) until a job becomes available. */
    virtual void waitForAvailableJob(Thread *th) = 0;
};

}

#endif // WEAVER_INTERFACE_H
