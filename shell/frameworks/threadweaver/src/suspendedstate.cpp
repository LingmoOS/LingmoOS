/* -*- C++ -*-
    This file implements the SuspendedState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: SuspendedState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "suspendedstate.h"

#include "threadweaver.h"

namespace ThreadWeaver
{
SuspendedState::SuspendedState(Weaver *weaver)
    : WeaverImplState(weaver)
{
}

void SuspendedState::suspend()
{
    // this request is not handled in Suspended state
}

void SuspendedState::resume()
{
    weaver()->setState(WorkingHard);
}

JobPointer SuspendedState::applyForWork(Thread *th, bool wasBusy)
{
    // suspend all threads in case they wake up:
    Q_ASSERT(wasBusy == 0);
    weaver()->waitForAvailableJob(th);
    return weaver()->applyForWork(th, wasBusy);
}

StateId SuspendedState::stateId() const
{
    return Suspended;
}

}
