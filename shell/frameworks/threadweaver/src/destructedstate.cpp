/* -*- C++ -*-
    This file implements the DestructedState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: DestructedState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "destructedstate.h"

namespace ThreadWeaver
{
DestructedState::DestructedState(QueueSignals *)
    : WeaverImplState(nullptr) // make sure we cannot use weaver, ever :-)
{
}

void DestructedState::shutDown()
{
}

Weaver *DestructedState::weaver()
{
    return nullptr;
}

const Weaver *DestructedState::weaver() const
{
    return nullptr;
}

void DestructedState::setMaximumNumberOfThreads(int)
{
}

int DestructedState::maximumNumberOfThreads() const
{
    return 0;
}

int DestructedState::currentNumberOfThreads() const
{
    return 0;
}

void DestructedState::enqueue(const QList<JobPointer> &)
{
}

bool DestructedState::dequeue(const JobPointer &)
{
    return false;
}

void DestructedState::dequeue()
{
}

void DestructedState::finish()
{
}

bool DestructedState::isEmpty() const
{
    return true;
}

bool DestructedState::isIdle() const
{
    return true;
}

int DestructedState::queueLength() const
{
    return 0;
}

void DestructedState::requestAbort()
{
}

void DestructedState::suspend()
{
}

void DestructedState::resume()
{
}

JobPointer DestructedState::applyForWork(Thread *, bool wasBusy)
{
    Q_UNUSED(wasBusy) // except in Q_ASSERT
    Q_ASSERT(wasBusy == false);
    return JobPointer();
}

void DestructedState::waitForAvailableJob(Thread *)
{
}

StateId DestructedState::stateId() const
{
    return Destructed;
}

}
