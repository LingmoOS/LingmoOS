/* -*- C++ -*-
    This file implements the InConstructionState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: InConstructionState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "inconstructionstate.h"

#include "threadweaver.h"
#include "weaver.h"

namespace ThreadWeaver
{
InConstructionState::InConstructionState(QueueSignals *weaver)
    : WeaverImplState(weaver)
{
}

void InConstructionState::suspend()
{
    // this request is not handled in InConstruction state
}

void InConstructionState::resume()
{
    // this request is not handled in InConstruction state
}

JobPointer InConstructionState::applyForWork(Thread *th, bool wasBusy)
{
    Q_ASSERT(wasBusy == false);
    // As long as we are in the construction state, no jobs will be given
    // to the worker threads. The threads will be suspended. They will
    // return from the blocked state when jobs are queued. By then, we
    // should not be in InConstruction state anymore, and we hand the job
    // application over to the then active state.
    while (weaver()->state()->stateId() == InConstruction) {
        weaver()->waitForAvailableJob(th);
    }
    return weaver()->applyForWork(th, wasBusy);
}

StateId InConstructionState::stateId() const
{
    return InConstruction;
}

}
