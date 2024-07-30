/* -*- C++ -*-
    This file is part of ThreadWeaver, a KDE framework.

    SPDX-FileCopyrightText: 2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QList>

#include "queue.h"
#include "queueing.h"
#include "queuestream.h"

namespace ThreadWeaver
{
class Q_DECL_HIDDEN QueueStream::Private
{
public:
    Private(Queue *w)
        : weaver(w)
    {
    }
    Queue *weaver;
    QList<JobPointer> jobs;
};

QueueStream::QueueStream(Queue *queue)
    : d(new Private(queue))
{
    Q_ASSERT(queue);
}

QueueStream::~QueueStream()
{
    flush();
    delete d;
}

void ThreadWeaver::QueueStream::add(const ThreadWeaver::JobPointer &job)
{
    d->jobs.append(job);
}

void QueueStream::flush()
{
    if (d->jobs.isEmpty()) {
        return;
    }
    Q_ASSERT(d->weaver);
    d->weaver->enqueue(d->jobs);
    d->jobs.clear();
}

QueueStream &QueueStream::operator<<(const JobPointer &job)
{
    add(job);
    return *this;
}

QueueStream &QueueStream::operator<<(JobInterface *job)
{
    add(make_job(job));
    return *this;
}

QueueStream &QueueStream::operator<<(Job &job)
{
    add(make_job_raw(&job));
    return *this;
}

/** @brief Return a stream the enqueues jobs in the ThreadWeaver global queue.
 *  Using this is synonymous to Weaver::instance()::stream(). */
QueueStream stream()
{
    return QueueStream(Queue::instance());
}

}
