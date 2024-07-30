/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "src/thread.h"

#include "JobLoggingDecorator.h"

using namespace ThreadWeaver;

JobLoggingDecorator::JobLoggingDecorator(const JobPointer &job, JobLoggingDecoratorCollector *collector)
    : IdDecorator(job.data(), false)
    , collector_(collector)
{
    Q_ASSERT(collector);
}

void JobLoggingDecorator::run(JobPointer self, Thread *thread)
{
    data_.start = collector_->time();
    if (thread) {
        data_.threadId = thread->id();
    } else {
        data_.threadId = -1;
    }
    IdDecorator::run(self, thread);
    data_.end = collector_->time();
    collector_->storeJobData(data_);
}

JobLoggingDecoratorCollector::JobLoggingDecoratorCollector()
{
    elapsed_.start();
    start_ = QDateTime::currentDateTime();
}

void JobLoggingDecoratorCollector::storeJobData(const JobLoggingDecorator::JobData &data)
{
    QMutexLocker m(&mutex_);
    jobData_.append(data);
}

qint64 JobLoggingDecoratorCollector::time()
{
    return elapsed_.nsecsElapsed();
}
