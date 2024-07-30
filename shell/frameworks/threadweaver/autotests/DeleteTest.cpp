/* -*- C++ -*-
    This file contains a testsuite for the memory management in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QEventLoop>

#include "DeleteTest.h"

#include <ThreadWeaver/DebuggingAids>
#include <ThreadWeaver/QObjectDecorator>
#include <ThreadWeaver/Sequence>
#include <ThreadWeaver/ThreadWeaver>

#include "AppendCharacterJob.h"

class InstanceCountingBusyJob : public BusyJob
{
public:
    explicit InstanceCountingBusyJob()
        : BusyJob()
    {
        instances_.fetchAndAddAcquire(1);
    }

    ~InstanceCountingBusyJob() override
    {
        instances_.fetchAndAddAcquire(-1);
    }

    static int instances()
    {
        return instances_.loadAcquire();
    }

private:
    static QAtomicInt instances_;
};

QAtomicInt InstanceCountingBusyJob::instances_;

class InstanceCountingCollection : public Collection
{
public:
    explicit InstanceCountingCollection()
        : Collection()
    {
        instances_.fetchAndAddAcquire(1);
    }

    ~InstanceCountingCollection() override
    {
        instances_.fetchAndAddAcquire(-1);
    }

    static int instances()
    {
        return instances_.loadAcquire();
    }

private:
    static QAtomicInt instances_;
};

QAtomicInt InstanceCountingCollection::instances_;

class InstanceCountingSequence : public Sequence
{
public:
    explicit InstanceCountingSequence()
        : Sequence()
    {
        instances_.fetchAndAddAcquire(1);
    }

    ~InstanceCountingSequence() override
    {
        instances_.fetchAndAddAcquire(-1);
    }

    static int instances()
    {
        return instances_.loadAcquire();
    }

private:
    static QAtomicInt instances_;
};

QAtomicInt InstanceCountingSequence::instances_;

DeleteTest::DeleteTest()
{
    ThreadWeaver::setDebugLevel(true, 1);
}

void DeleteTest::DeleteJobsTest()
{
    const int NumberOfJobs = 100;
    ThreadWeaver::Queue queue;
    //    queue.setMaximumNumberOfThreads(1);
    QCOMPARE(InstanceCountingBusyJob::instances(), 0);
    { // just to be sure instance counting works:
        InstanceCountingBusyJob job;
        Q_UNUSED(job);
        QCOMPARE(InstanceCountingBusyJob::instances(), 1);
    }
    QCOMPARE(InstanceCountingBusyJob::instances(), 0);
    queue.suspend();
    for (int i = 0; i < NumberOfJobs; ++i) {
        queue.stream() << new InstanceCountingBusyJob;
    }
    queue.resume();
    queue.finish();
    // The used Weaver instance needs to be shut down. The threads may still hold a reference to the previous job while
    // waiting for the next one or blocking because the queue is empty. If all threads have exited, no references to any jobs are
    // held anymore.
    queue.shutDown();
    TWDEBUG(3, "DeleteTest::DeleteJobsTest: instances: %i\n", InstanceCountingBusyJob::instances());
    // QCOMPARE(InstanceCountingBusyJob::instances(), NumberOfJobs); // held by signals about the job being started and finished
    QCOMPARE(InstanceCountingBusyJob::instances(), 0); // zero, since threads are not sending the jobStarted/jobDone signals anymore
    qApp->processEvents();
    TWDEBUG(3, "DeleteTest::DeleteJobsTest: instances: %i\n", InstanceCountingBusyJob::instances());
    QCOMPARE(InstanceCountingBusyJob::instances(), 0);
}

void DeleteTest::MutexLockingAssertsTest()
{
    QMutex mutex;
    MUTEX_ASSERT_UNLOCKED(&mutex);
    mutex.lock();
    MUTEX_ASSERT_LOCKED(&mutex);
    mutex.unlock();
    MUTEX_ASSERT_UNLOCKED(&mutex);
}

void DeleteTest::DeleteCollectionTest()
{
    const int NumberOfCollections = 100;
    const int NumberOfJobs = 10;
    ThreadWeaver::Queue queue;
    //    queue.setMaximumNumberOfThreads(1);
    QCOMPARE(InstanceCountingCollection::instances(), 0);
    QCOMPARE(InstanceCountingBusyJob::instances(), 0);
    queue.suspend();
    JobPointer temp; // for debugging
    for (int i = 0; i < NumberOfCollections; ++i) {
        QSharedPointer<InstanceCountingCollection> col(new InstanceCountingCollection);
        if (i == 0) {
            temp = col;
        }
        for (int j = 0; j < NumberOfJobs; ++j) {
            col->addJob(JobPointer(new InstanceCountingBusyJob));
        }
        queue.enqueue(col);
    }
    queue.resume();
    queue.finish();
    // The used Weaver instance needs to be shut down. The threads may still hold a reference to the previous job while
    // waiting for the next one or blocking because the queue is empty. If all threads have exited, no references to any jobs are
    // held anymore.
    queue.shutDown();

    TWDEBUG(3,
            "DeleteTest::DeleteJobsTest: collection instances: %i, job instances: %i\n",
            InstanceCountingCollection::instances(),
            InstanceCountingBusyJob::instances());
    // held by signals about the job being started and finished:
    // QCOMPARE(InstanceCountingCollection::instances(), NumberOfCollections);
    // one, since threads are not sending the jobStarted/jobDone signals anymore:
    QCOMPARE(InstanceCountingCollection::instances(), 1);
    qApp->processEvents();
    TWDEBUG(3,
            "DeleteTest::DeleteJobsTest: collection instances: %i, job instances: %i\n",
            InstanceCountingCollection::instances(),
            InstanceCountingBusyJob::instances());
    // these are held in temp:
    QCOMPARE(InstanceCountingBusyJob::instances(), NumberOfJobs);
    QCOMPARE(InstanceCountingCollection::instances(), 1);
    temp.clear();
    QCOMPARE(InstanceCountingCollection::instances(), 0);
    QCOMPARE(InstanceCountingBusyJob::instances(), 0);
}

void DeleteTest::DeleteDecoratedCollectionTest()
{
    const int NumberOfCollections = 100;
    const int NumberOfJobs = 10;
    m_finishCount.storeRelease(0);
    ThreadWeaver::Queue queue;
    //    queue.setMaximumNumberOfThreads(1);
    QCOMPARE(InstanceCountingCollection::instances(), 0);
    QCOMPARE(InstanceCountingBusyJob::instances(), 0);
    queue.suspend();
    JobPointer temp; // for debugging
    for (int i = 0; i < NumberOfCollections; ++i) {
        QJobPointer col(new QObjectDecorator(new InstanceCountingCollection));
        if (i == 0) {
            temp = col;
        }
        for (int j = 0; j < NumberOfJobs; ++j) {
            col->collection()->addJob(JobPointer(new InstanceCountingBusyJob));
        }
        QVERIFY(connect(col.data(), SIGNAL(done(ThreadWeaver::JobPointer)), SLOT(countCompletedDecoratedCollection(ThreadWeaver::JobPointer))));

        queue.enqueue(col);
        m_finishCount.fetchAndAddRelease(1);
    }
    QCOMPARE(m_finishCount.loadAcquire(), NumberOfCollections);
    QEventLoop loop;
    QVERIFY(connect(this, SIGNAL(deleteDecoratedCollectionTestCompleted()), &loop, SLOT(quit()), Qt::QueuedConnection));
    queue.resume();
    queue.finish();
    loop.exec();
    QCOMPARE(m_finishCount.loadAcquire(), 0);
    // The used Weaver instance needs to be shut down. The threads may still hold a reference to the previous job while
    // waiting for the next one or blocking because the queue is empty. If all threads have exited, no references to any jobs are
    // held anymore.
    queue.shutDown();

    TWDEBUG(3, "DeleteTest::DeleteJobsTest: instances: %i\n", InstanceCountingCollection::instances());
    QCOMPARE(InstanceCountingCollection::instances(), 1); // held in temp
    temp.clear();
    TWDEBUG(3, "DeleteTest::DeleteJobsTest: instances: %i\n", InstanceCountingCollection::instances());
    QCOMPARE(InstanceCountingBusyJob::instances(), 0);
    QCOMPARE(InstanceCountingCollection::instances(), 0);
}

void DeleteTest::DeleteSequenceTest()
{
    const int NumberOfSequences = 100;
    const int NumberOfJobs = 10;
    m_finishCount.storeRelease(0);
    ThreadWeaver::Queue queue;
    //    queue.setMaximumNumberOfThreads(1);
    QCOMPARE(InstanceCountingSequence::instances(), 0);
    QCOMPARE(InstanceCountingBusyJob::instances(), 0);
    queue.suspend();
    for (int i = 0; i < NumberOfSequences; ++i) {
        QJobPointer seq(new QObjectDecorator(new InstanceCountingSequence));
        for (int j = 0; j < NumberOfJobs; ++j) {
            seq->sequence()->addJob(JobPointer(new InstanceCountingBusyJob));
        }
        QVERIFY(connect(seq.data(), SIGNAL(done(ThreadWeaver::JobPointer)), SLOT(deleteSequence(ThreadWeaver::JobPointer))));

        queue.enqueue(seq);
        m_finishCount.fetchAndAddRelease(1);
    }
    QCOMPARE(m_finishCount.loadAcquire(), NumberOfSequences);
    QEventLoop loop;
    QVERIFY(connect(this, SIGNAL(deleteSequenceTestCompleted()), &loop, SLOT(quit()), Qt::QueuedConnection));
    queue.resume();
    queue.finish();
    loop.exec();
    QCOMPARE(m_finishCount.loadAcquire(), 0);
    // The used Weaver instance needs to be shut down. The threads may still hold a reference to the previous job while
    // waiting for the next one or blocking because the queue is empty. If all threads have exited, no references to any jobs are
    // held anymore.
    queue.shutDown();
    QCOMPARE(InstanceCountingBusyJob::instances(), 0);
    QCOMPARE(InstanceCountingSequence::instances(), 0);
}

void DeleteTest::deleteSequence(ThreadWeaver::JobPointer)
{
    if (m_finishCount.fetchAndAddRelease(-1) == 1) { // if it *was* 1...
        Q_EMIT deleteSequenceTestCompleted();
    }
}

void DeleteTest::countCompletedDecoratedCollection(JobPointer)
{
    if (m_finishCount.fetchAndAddRelease(-1) == 1) { // if it *was* 1...
        Q_EMIT deleteDecoratedCollectionTestCompleted();
    }
}

QMutex s_GlobalMutex;

QTEST_MAIN(DeleteTest)

#include "moc_DeleteTest.cpp"
