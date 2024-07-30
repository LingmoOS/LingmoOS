/* -*- C++ -*-
    This file contains a testsuite for the queueing behaviour in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "QueueTests.h"

#include <QChar>

#include <QDebug>
#include <QObject>
#include <QTest>

#include <ThreadWeaver/Collection>
#include <ThreadWeaver/DebuggingAids>
#include <ThreadWeaver/DependencyPolicy>
#include <ThreadWeaver/Job>
#include <ThreadWeaver/QueuePolicy>
#include <ThreadWeaver/Queueing>
#include <ThreadWeaver/ResourceRestrictionPolicy>
#include <ThreadWeaver/Sequence>
#include <ThreadWeaver/State>

#include <ThreadWeaver/QObjectDecorator>
#include <ThreadWeaver/Thread>
#include <ThreadWeaver/ThreadWeaver>

QMutex s_GlobalMutex;

LowPriorityAppendCharacterJob::LowPriorityAppendCharacterJob(QChar c, QString *stringref)
    : AppendCharacterJob(c, stringref)
{
}

int LowPriorityAppendCharacterJob ::priority() const
{
    return -1;
}

HighPriorityAppendCharacterJob::HighPriorityAppendCharacterJob(QChar c, QString *stringref)
    : AppendCharacterJob(c, stringref)
{
}

int HighPriorityAppendCharacterJob::priority() const
{
    return 1;
}

SecondThreadThatQueues::SecondThreadThatQueues()
    : QThread()
{
}

void SecondThreadThatQueues::run()
{
    QString sequence;
    AppendCharacterJob a('a', &sequence);

    ThreadWeaver::enqueue_raw(&a);
    ThreadWeaver::Queue::instance()->finish();
    QCOMPARE(sequence, QString("a"));
}

QueueTests::QueueTests(QObject *parent)
    : QObject(parent)
    , autoDeleteJob(nullptr)
{
}

void QueueTests::initTestCase()
{
    ThreadWeaver::setDebugLevel(true, 1);
}

void QueueTests::SimpleQueuePrioritiesTest()
{
    using namespace ThreadWeaver;

    Queue weaver;
    weaver.setMaximumNumberOfThreads(1); // just one thread
    QString sequence;
    LowPriorityAppendCharacterJob jobA(QChar('a'), &sequence);
    AppendCharacterJob jobB(QChar('b'), &sequence);
    HighPriorityAppendCharacterJob jobC(QChar('c'), &sequence);

    // queue low priority, then normal priority, then high priority
    // if priorities are processed correctly, the jobs will be executed in reverse order

    weaver.suspend();

    enqueue_raw(&weaver, &jobA);
    enqueue_raw(&weaver, &jobB);
    enqueue_raw(&weaver, &jobC);

    weaver.resume();
    weaver.finish();

    QCOMPARE(sequence, QString("cba"));
}

void QueueTests::WeaverInitializationTest()
{
    // this one mostly tests the sanity of the startup behaviour
    ThreadWeaver::Queue weaver;
    QCOMPARE(weaver.currentNumberOfThreads(), 0);
    QVERIFY(weaver.isEmpty());
    QVERIFY(weaver.isIdle());
    QVERIFY(weaver.queueLength() == 0);
    weaver.finish();
}

void QueueTests::QueueFromSecondThreadTest()
{
    ThreadWeaver::Queue::instance(); // create global instance in the main thread
    SecondThreadThatQueues thread;
    thread.start();
    thread.wait();
    QVERIFY(ThreadWeaver::Queue::instance()->isIdle());
}

void QueueTests::deleteJob(ThreadWeaver::JobPointer job)
{
    // test that signals are properly emitted (asynchronously, that is):
    QVERIFY(thread() == QThread::currentThread());
    QVERIFY(job == autoDeleteJob);
    delete autoDeleteJob;
    autoDeleteJob = nullptr;
}

void QueueTests::DeleteDoneJobsFromSequenceTest()
{
    using namespace ThreadWeaver;
    QString sequence;
    autoDeleteJob = new QObjectDecorator(new AppendCharacterJob(QChar('a'), &sequence));
    AppendCharacterJob b(QChar('b'), &sequence);
    AppendCharacterJob c(QChar('c'), &sequence);
    Collection collection;
    collection << make_job_raw(autoDeleteJob) << b << c;
    QVERIFY(autoDeleteJob != nullptr);
    QVERIFY(connect(autoDeleteJob, SIGNAL(done(ThreadWeaver::JobPointer)), SLOT(deleteJob(ThreadWeaver::JobPointer))));
    stream() << collection;
    QTest::qWait(100); // return to event queue to make sure signals are delivered
    Queue::instance()->finish();
    QTest::qWait(100); // return to event queue to make sure signals are delivered
    // no need to delete a, that should be done in deleteJob
    QVERIFY(autoDeleteJob == nullptr);
}

void QueueTests::deleteCollection(ThreadWeaver::JobPointer collection)
{
    QVERIFY(thread() == QThread::currentThread());
    QVERIFY(collection == autoDeleteCollection);
    delete autoDeleteCollection;
    autoDeleteCollection = nullptr;
}

void QueueTests::DeleteCollectionOnDoneTest()
{
    using namespace ThreadWeaver;
    QString sequence;
    autoDeleteCollection = new QObjectDecorator(new Collection);
    QVERIFY(connect(autoDeleteCollection, SIGNAL(done(ThreadWeaver::JobPointer)), SLOT(deleteCollection(ThreadWeaver::JobPointer))));

    AppendCharacterJob a(QChar('a'), &sequence);
    AppendCharacterJob b(QChar('b'), &sequence);
    *autoDeleteCollection->collection() << a << b;

    enqueue_raw(autoDeleteCollection);
    // return to event queue to make sure signals are delivered
    // (otherwise, no slot calls would happen before the end of this function)
    // I assume the amount of time that we wait does not matter
    QTest::qWait(10);
    Queue::instance()->finish();
    // return to event queue to make sure signals are delivered
    QTest::qWait(10);
    // no need to delete a, that should be done in deleteJob
    QVERIFY(sequence.length() == 2);
    QVERIFY(autoDeleteCollection == nullptr);
}

QTEST_MAIN(QueueTests)

#include "moc_QueueTests.cpp"
