/* -*- C++ -*-
    This file declares the SuspendingState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: SuspendingState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef SuspendingState_H
#define SuspendingState_H

#include "weaver.h"
#include "weaverimplstate.h"

namespace ThreadWeaver
{
/** SuspendingState is the state after suspend() has been called, but
 *  before all threads finished executing the current job and blocked.
 */
class SuspendingState : public WeaverImplState
{
public:
    explicit SuspendingState(Weaver *weaver);
    /** Suspend job processing. */
    void suspend() override;
    /** Resume job processing. */
    void resume() override;
    /** Assign a job to an idle thread. */
    JobPointer applyForWork(Thread *th, bool wasBusy) override;
    /** Overload. */
    void activated() override;
    /** reimpl */
    StateId stateId() const override;
};

}

#endif // SuspendingState_H
