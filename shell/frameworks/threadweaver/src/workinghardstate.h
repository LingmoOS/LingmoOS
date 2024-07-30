/* -*- C++ -*-
    This file declares the WorkingHardState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: WorkingHardState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef WorkingHardState_H
#define WorkingHardState_H

#include "weaver.h"
#include "weaverimplstate.h"

namespace ThreadWeaver
{
/* WorkingHardState handles the state dependent calls in the state where
 * jobs are executed.  */
class WorkingHardState : public WeaverImplState
{
public:
    explicit WorkingHardState(Weaver *weaver);
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

#endif // WorkingHardIState_H
