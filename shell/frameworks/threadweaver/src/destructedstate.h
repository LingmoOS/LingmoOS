/* -*- C++ -*-
    This file declares the DestructedState class.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: DestructedState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef DestructedState_H
#define DestructedState_H

#include "weaverimplstate.h"

namespace ThreadWeaver
{
class QueueSignals;

/** DestructedState is only active after the thread have been destroyed by
 *  the destructor, but before superclass destructors have finished.
 */
class DestructedState : public WeaverImplState
{
public:
    explicit DestructedState(QueueSignals *weaver);
    void shutDown() override;
    Weaver *weaver() override;
    const Weaver *weaver() const override;
    void setMaximumNumberOfThreads(int cap) override;
    int maximumNumberOfThreads() const override;
    int currentNumberOfThreads() const override;
    void enqueue(const QList<JobPointer> &job) override;
    bool dequeue(const JobPointer &job) override;
    void dequeue() override;
    void finish() override;
    bool isEmpty() const override;
    bool isIdle() const override;
    int queueLength() const override;
    void requestAbort() override;
    void suspend() override;
    void resume() override;
    JobPointer applyForWork(Thread *th, bool wasBusy) override;
    void waitForAvailableJob(Thread *th) override;
    StateId stateId() const override;
};

}

#endif // DestructedState_H
