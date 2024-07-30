/* -*- C++ -*-
    This file is part of ThreadWeaver. It implements the Thread class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: Thread.cpp 25 2005-08-14 12:41:38Z mirko $
*/

#include "thread.h"

#include <QCoreApplication>
#include <QDebug>
#include <QPointer>

#include "debuggingaids.h"
#include "exception.h"
#include "job.h"
#include "threadweaver.h"
#include "weaver.h"

using namespace ThreadWeaver;

class Q_DECL_HIDDEN Thread::Private
{
public:
    explicit Private(Weaver *theParent)
        : parent(theParent)
        , id(makeId())
        , job(nullptr)
    {
        Q_ASSERT(parent);
    }

    Weaver *parent;
    const unsigned int id;
    JobPointer job;
    QMutex mutex;

    static unsigned int makeId()
    {
        static QAtomicInt s_id(1);
        return s_id.fetchAndAddRelease(1);
    }
};

Thread::Thread(Weaver *parent)
    : QThread() // no parent, because the QObject hierarchy of this thread
    // does not have a parent (see QObject::pushToThread)
    , d(new Private(parent))
{
    const QString queueName =
        parent->objectName().isEmpty() ? QString::fromLatin1("Queue(0x%1)").arg(quintptr(parent), 0, 16, QChar::fromLatin1('0')) : parent->objectName();
    setObjectName(QString::fromLatin1("%1[%2]").arg(queueName).arg(QString::number(id()), 2, QChar::fromLatin1('0')));
}

Thread::~Thread()
{
    delete d;
}

unsigned int Thread::id() const
{
    return d->id; // id is const
}

void Thread::run()
{
    Q_ASSERT(d->parent);
    Q_ASSERT(QCoreApplication::instance() != nullptr);
    d->parent->threadEnteredRun(this);

    TWDEBUG(3, "Thread::run [%u]: running.\n", id());

    bool wasBusy = false;
    while (true) {
        TWDEBUG(3, "Thread::run [%u]: trying to execute the next job.\n", id());

        try {
            // the assignment is intentional: newJob needs to go out of scope at the end of the if statement
            if (JobPointer newJob = d->parent->applyForWork(this, wasBusy)) {
                QMutexLocker l(&d->mutex);
                Q_UNUSED(l);
                d->job = newJob;
            } else {
                break;
            }
        } catch (AbortThread &) {
            break;
        }

        wasBusy = true;
        d->job->execute(d->job, this);
        JobPointer oldJob;
        { // When finally destroying the last reference to d->job, do not hold the mutex.
            // It may trigger destruction of the job, which can execute arbitrary code.
            QMutexLocker l(&d->mutex);
            Q_UNUSED(l);
            oldJob = d->job;
            d->job.clear();
        }
        oldJob.clear();
    }
    TWDEBUG(3, "Thread::run [%u]: exiting.\n", id());
}

void Thread::requestAbort()
{
    QMutexLocker l(&d->mutex);
    Q_UNUSED(l);
    if (d->job) {
        d->job->requestAbort();
    }
}

#include "moc_thread.cpp"
