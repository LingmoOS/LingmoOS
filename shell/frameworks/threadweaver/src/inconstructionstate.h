/* -*- C++ -*-
    This file declares the InConstructionState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: InConstructionState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef InConstructionState_H
#define InConstructionState_H

#include "weaverimplstate.h"

namespace ThreadWeaver
{
class Weaver;
class Queue;

/** InConstructionState handles the calls to the Weaver
        object until the constructor has finished.
    */
class InConstructionState : public WeaverImplState
{
public:
    explicit InConstructionState(QueueSignals *weaver);
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

#endif // InConstructionState_H
