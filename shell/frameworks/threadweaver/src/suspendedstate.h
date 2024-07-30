/* -*- C++ -*-
    This file declares the SuspendedState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: SuspendedState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef SuspendedState_H
#define SuspendedState_H

#include "weaver.h"
#include "weaverimplstate.h"

namespace ThreadWeaver
{
/** In SuspendedState, jobs are queued, but will not be executed. All
 *  thread remains blocked.  */
class SuspendedState : public WeaverImplState
{
public:
    explicit SuspendedState(Weaver *weaver);
    /** Suspend job processing. */
    void suspend() override;
    /** Resume job processing. */
    void resume() override;
    /** Assign a job to an idle thread. */
    JobPointer applyForWork(Thread *th, bool wasBusy) override;
    /** reimpl */
    StateId stateId() const override;
};

}

#endif // SuspendedState_H
