/* -*- C++ -*-
    This file implements the StateImplementation class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: InConstructionState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "weaverimplstate.h"

namespace ThreadWeaver
{
WeaverImplState::WeaverImplState(QueueSignals *weaver)
    : State(weaver)
{
}

const State *WeaverImplState::state() const
{
    return this;
}

void WeaverImplState::shutDown()
{
    weaver()->shutDown_p();
}

Weaver *WeaverImplState::weaver()
{
    Q_ASSERT(dynamic_cast<Weaver *>(State::weaver()));
    return static_cast<Weaver *>(State::weaver());
}

const Weaver *WeaverImplState::weaver() const
{
    Q_ASSERT(dynamic_cast<const Weaver *>(State::weaver()));
    return static_cast<const Weaver *>(State::weaver());
}

void WeaverImplState::setMaximumNumberOfThreads(int cap)
{
    weaver()->setMaximumNumberOfThreads_p(cap);
}

int WeaverImplState::maximumNumberOfThreads() const
{
    return weaver()->maximumNumberOfThreads_p();
}

int WeaverImplState::currentNumberOfThreads() const
{
    return weaver()->currentNumberOfThreads_p();
}

void WeaverImplState::enqueue(const QList<JobPointer> &jobs)
{
    weaver()->enqueue_p(jobs);
}

bool WeaverImplState::dequeue(const JobPointer &job)
{
    return weaver()->dequeue_p(job);
}

void WeaverImplState::dequeue()
{
    weaver()->dequeue_p();
}

void WeaverImplState::finish()
{
    weaver()->finish_p();
}

bool WeaverImplState::isEmpty() const
{
    return weaver()->isEmpty_p();
}

bool WeaverImplState::isIdle() const
{
    return weaver()->isIdle_p();
}

int WeaverImplState::queueLength() const
{
    return weaver()->queueLength_p();
}

void WeaverImplState::requestAbort()
{
    return weaver()->requestAbort_p();
}

void WeaverImplState::reschedule()
{
    // Not implemented, performed directly in Weaver::reschedule().
}

void WeaverImplState::waitForAvailableJob(Thread *th)
{
    weaver()->blockThreadUntilJobsAreBeingAssigned(th);
}

}
