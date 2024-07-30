/* -*- C++ -*-
    This file declares the ShuttingDownState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: ShuttingDownState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef ShuttingDownState_H
#define ShuttingDownState_H

#include "weaverimplstate.h"

namespace ThreadWeaver
{
class Queue;

/** ShuttingDownState is enabled when the Weaver destructor is entered. It
 *  prevents threads from still accessing queue management methods, and new jobs being queued.
 */
class ShuttingDownState : public WeaverImplState
{
public:
    explicit ShuttingDownState(QueueSignals *weaver);

    /** Shut down the queue. */
    void shutDown() override;
    /** Suspend job processing. */
    void suspend() override;
    /** Resume job processing. */
    void resume() override;
    /** Assign a job to an idle thread. */
    JobPointer applyForWork(Thread *th, bool wasBusy) override;
    /** Wait (by suspending the calling thread) until a job becomes available. */
    void waitForAvailableJob(Thread *th) override;
    /** reimpl */
    StateId stateId() const override;
};

}

#endif // ShuttingDownState_H
