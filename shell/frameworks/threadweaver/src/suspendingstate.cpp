/* -*- C++ -*-
    This file implements the SuspendingState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: SuspendingState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "suspendingstate.h"

#include "state.h"
#include "threadweaver.h"

namespace ThreadWeaver
{
SuspendingState::SuspendingState(Weaver *weaver)
    : WeaverImplState(weaver)
{
}

void SuspendingState::suspend()
{
    // this request is not handled in Suspending state (we are already suspending...)
}

void SuspendingState::resume()
{
    weaver()->setState(WorkingHard);
}

void SuspendingState::activated()
{
    weaver()->reschedule();
}

JobPointer SuspendingState::applyForWork(Thread *th, bool wasBusy)
{
    weaver()->takeFirstAvailableJobOrSuspendOrWait(th, wasBusy, true, true);
    weaver()->waitForAvailableJob(th);
    return weaver()->applyForWork(th, false);
}

StateId SuspendingState::stateId() const
{
    return Suspending;
}

}
