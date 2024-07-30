/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SEQUENCE_P_H
#define SEQUENCE_P_H

#include <QAtomicInt>

#include "collection_p.h"
#include "queuepolicy.h"
#include "sequence.h"

namespace ThreadWeaver
{
namespace Private
{
class BlockerPolicy : public QueuePolicy
{
public:
    bool canRun(JobPointer) override;
    void free(JobPointer) override;
    void release(JobPointer) override;
    void destructed(JobInterface *job) override;
};

class Sequence_Private : public Collection_Private
{
public:
    Sequence_Private();
    BlockerPolicy *blocker();
    void prepareToEnqueueElements() override;
    void processCompletedElement(Collection *collection, JobPointer job, Thread *thread) override;
    void elementDequeued(const JobPointer &job) override;
    BlockerPolicy blocker_;
    QAtomicInt completed_;
};

}

}

#endif // SEQUENCE_P_H
