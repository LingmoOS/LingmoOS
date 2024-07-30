/* -*- C++ -*-
    This file contains a testsuite for JobPointer behaviour in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QString>
#include <QTest>

#include <ThreadWeaver/Job>
#include <ThreadWeaver/JobPointer>
#include <ThreadWeaver/ThreadWeaver>

class NotifyOnDeletejob : public ThreadWeaver::Job
{
public:
    explicit NotifyOnDeletejob(bool &exists)
        : ThreadWeaver::Job()
        , m_exists(exists)
    {
        exists = true;
    }

    ~NotifyOnDeletejob() override
    {
        m_exists = false;
    }

    void run(ThreadWeaver::JobPointer, ThreadWeaver::Thread *) override
    {
    }

private:
    bool &m_exists;
};

class LifecycleTests : public QObject
{
    Q_OBJECT

public:
    LifecycleTests();

private Q_SLOTS:
    void testJobAutoDeletionBasics();
    void testJobAutoDeletion();
};

LifecycleTests::LifecycleTests()
{
}

void LifecycleTests::testJobAutoDeletionBasics()
{
    bool job1Exists = false;
    bool job2Exists = false;
    ThreadWeaver::JobPointer job2(new NotifyOnDeletejob(job2Exists));
    Q_UNUSED(job2);
    QCOMPARE(true, job2Exists);
    {
        ThreadWeaver::JobPointer job1(new NotifyOnDeletejob(job1Exists));
        Q_UNUSED(job1);
        QCOMPARE(job1Exists, true);
    }
    QCOMPARE(job1Exists, false);
    QCOMPARE(job2Exists, true);
}

void LifecycleTests::testJobAutoDeletion()
{
    bool job1Exists = false;
    bool job2Exists = false;
    {
        ThreadWeaver::JobPointer job1(new NotifyOnDeletejob(job1Exists));
        QCOMPARE(job1Exists, true);
        int argc = 0;
        QCoreApplication app(argc, (char **)nullptr);
        Q_UNUSED(app);
        QVERIFY(ThreadWeaver::Queue::instance());
        ThreadWeaver::Queue::instance()->suspend();
        ThreadWeaver::Queue::instance()->enqueue(job1);
        ThreadWeaver::Queue::instance()->enqueue(ThreadWeaver::JobPointer(new NotifyOnDeletejob(job2Exists)));
        QCOMPARE(job2Exists, true);
        ThreadWeaver::Queue::instance()->resume();
        ThreadWeaver::Queue::instance()->finish();
        QVERIFY(ThreadWeaver::Queue::instance()->isIdle());
        ThreadWeaver::Queue::instance()->suspend();
        QCOMPARE(job2Exists, false);
        QCOMPARE(job1Exists, true);
    }
    QVERIFY(ThreadWeaver::Queue::instance() == nullptr);
    QCOMPARE(job2Exists, false);
    QCOMPARE(job1Exists, false);
}

QTEST_APPLESS_MAIN(LifecycleTests)

#include "LifecycleTests.moc"
