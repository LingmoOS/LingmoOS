/* -*- C++ -*-
    This file contains a testsuite for global queue customizations in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QString>
#include <QTest>

#include <ThreadWeaver/IdDecorator>
#include <ThreadWeaver/Queue>
#include <ThreadWeaver/QueueSignals>
#include <ThreadWeaver/ThreadWeaver>
#include <weaver.h>

using namespace ThreadWeaver;
QAtomicInt counter;

class CountingJobDecorator : public IdDecorator
{
public:
    explicit CountingJobDecorator(const JobPointer &job)
        : IdDecorator(job.data(), false)
        , original_(job)
    {
    }

    void run(JobPointer self, Thread *thread) override
    {
        counter.fetchAndAddRelease(1);
        IdDecorator::run(self, thread);
        counter.fetchAndAddAcquire(1);
    }

    JobPointer original_;
};

class JobCountingWeaver : public Weaver
{
    Q_OBJECT
public:
    explicit JobCountingWeaver(QObject *parent = nullptr)
        : Weaver(parent)
    {
    }
    void enqueue(const QList<JobPointer> &jobs) override
    {
        QList<JobPointer> decorated;
        std::transform(jobs.begin(), jobs.end(), std::back_inserter(decorated), [](const JobPointer &job) {
            return JobPointer(new CountingJobDecorator(job));
        });
        Weaver::enqueue(decorated);
    }
};

class CountingGlobalQueueFactory : public Queue::GlobalQueueFactory
{
    Queue *create(QObject *parent = nullptr) override
    {
        return new Queue(new JobCountingWeaver, parent);
    }
};

int argc = 0;

class QueueFactoryTests : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testQueueFactory()
    {
        counter.storeRelease(0);
        QCoreApplication app(argc, (char **)nullptr);
        Queue queue(new JobCountingWeaver(this));
        queue.enqueue(make_job([]() {})); // nop
        queue.finish();
        QCOMPARE(counter.loadAcquire(), 2);
    }

    void testGlobalQueueFactory()
    {
        Queue::setGlobalQueueFactory(new CountingGlobalQueueFactory());
        QCoreApplication app(argc, (char **)nullptr);
        counter.storeRelease(0);
        Queue::instance()->enqueue(make_job([]() {})); // nop
        Queue::instance()->finish();
        QCOMPARE(counter.loadAcquire(), 2);
    }
};

QTEST_APPLESS_MAIN(QueueFactoryTests)

#include "QueueFactoryTests.moc"
