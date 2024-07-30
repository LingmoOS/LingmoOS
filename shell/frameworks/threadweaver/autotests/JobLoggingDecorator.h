/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef JOBLOGGINGDECORATOR_H
#define JOBLOGGINGDECORATOR_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QList>
#include <QMutex>

#include "src/iddecorator.h"

class JobLoggingDecoratorCollector;

class JobLoggingDecorator : public ThreadWeaver::IdDecorator
{
public:
    struct JobData {
        int threadId;
        QString description;
        qint64 start;
        qint64 end;
        JobData()
            : threadId(0)
        {
        }
    };

    explicit JobLoggingDecorator(const ThreadWeaver::JobPointer &job, JobLoggingDecoratorCollector *collector);
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread) override;

private:
    JobData data_;
    JobLoggingDecoratorCollector *collector_;
};

class JobLoggingDecoratorCollector
{
public:
    JobLoggingDecoratorCollector();
    void storeJobData(const JobLoggingDecorator::JobData &data);
    qint64 time();

private:
    QList<JobLoggingDecorator::JobData> jobData_;
    QElapsedTimer elapsed_;
    QDateTime start_;
    QMutex mutex_;
};

#endif // JOBLOGGINGDECORATOR_H
