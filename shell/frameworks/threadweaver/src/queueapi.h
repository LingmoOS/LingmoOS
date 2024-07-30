/* -*- C++ -*-
    The detailed API for Weavers in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QUEUEAPI_H
#define QUEUEAPI_H

#include "jobpointer.h"
#include "queuesignals.h"
#include "state.h"
#include "weaverinterface.h"

namespace ThreadWeaver
{
class QueueAPI : public QueueSignals, public WeaverInterface
{
    Q_OBJECT

public:
    explicit QueueAPI(QObject *parent = nullptr);
    explicit QueueAPI(ThreadWeaver::Private::QueueSignals_Private *d, QObject *parent = nullptr);

    virtual void shutDown_p() = 0;
    const State *state() const override = 0;
    virtual State *state() = 0;
    virtual void setMaximumNumberOfThreads_p(int cap) = 0;
    virtual int maximumNumberOfThreads_p() const = 0;
    virtual int currentNumberOfThreads_p() const = 0;
    virtual bool dequeue_p(JobPointer) = 0;
    virtual void dequeue_p() = 0;
    virtual void finish_p() = 0;
    virtual void suspend_p() = 0;
    virtual void resume_p() = 0;
    virtual bool isEmpty_p() const = 0;
    virtual bool isIdle_p() const = 0;
    virtual int queueLength_p() const = 0;
    virtual void requestAbort_p() = 0;
};

}

#endif // QUEUEAPI_H
