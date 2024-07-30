/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "JobLoggingWeaver.h"

using namespace ThreadWeaver;

JobLoggingWeaver::JobLoggingWeaver(QObject *parent)
    : Weaver(parent)
{
}

void JobLoggingWeaver::enqueue(const QList<JobPointer> &jobs)
{
    QList<JobPointer> decorated;
    std::transform(jobs.begin(), jobs.end(), std::back_inserter(decorated), [this](const JobPointer &job) {
        return JobPointer(new JobLoggingDecorator(job, &collector_));
    });
    Weaver::enqueue(decorated);
}

#include "moc_JobLoggingWeaver.cpp"
