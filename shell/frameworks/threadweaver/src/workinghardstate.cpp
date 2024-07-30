/* -*- C++ -*-
    This file implements the WorkingHardState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: WorkingHardState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "workinghardstate.h"

#include "debuggingaids.h"
#include "job.h"
#include "thread.h"
#include "threadweaver.h"

namespace ThreadWeaver
{
void WorkingHardState::activated()
{
    weaver()->reschedule();
}

WorkingHardState::WorkingHardState(Weaver *weaver)
    : WeaverImplState(weaver)
{
}

void WorkingHardState::suspend()
{
    weaver()->setState(Suspending);
}

void WorkingHardState::resume()
{
}

JobPointer WorkingHardState::applyForWork(Thread *th, bool wasBusy)
{
    // beware: this code is executed in the applying thread!
    TWDEBUG(2, "WorkingHardState::applyForWork: thread %i applies for work in %s state.\n", th->id(), qPrintable(weaver()->state()->stateName()));
    JobPointer next = weaver()->takeFirstAvailableJobOrSuspendOrWait(th, wasBusy, false, false);
    if (next) {
        return next;
    } else {
        // this is no infinite recursion: the state may have changed meanwhile, or jobs may have become available:
        TWDEBUG(2, "WorkingHardState::applyForWork: repeating for thread %i in %s state.\n", th->id(), qPrintable(weaver()->state()->stateName()));
        return weaver()->applyForWork(th, false);
    }
}

StateId WorkingHardState::stateId() const
{
    return WorkingHard;
}

}
