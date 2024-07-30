/* -*- C++ -*-
    This file implements the ShuttingDownState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: ShuttingDownState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "shuttingdownstate.h"

namespace ThreadWeaver
{
ShuttingDownState::ShuttingDownState(QueueSignals *weaver)
    : WeaverImplState(weaver)
{
}

void ShuttingDownState::shutDown()
{
}

void ShuttingDownState::suspend()
{
    // ignored: when shutting down, we do not return to the suspended state
}

void ShuttingDownState::resume()
{
    // ignored: when shutting down, we do not return from the suspended state
}

JobPointer ShuttingDownState::applyForWork(Thread *, bool wasBusy)
{
    Q_UNUSED(wasBusy) // except in Q_ASSERT
    Q_ASSERT(wasBusy == false);
    return JobPointer(); // tell threads to exit
}

void ShuttingDownState::waitForAvailableJob(Thread *)
{
    // immediately return here
}

StateId ShuttingDownState::stateId() const
{
    return ShuttingDown;
}

}
