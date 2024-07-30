/* -*- C++ -*-
    This file contains a testsuite for job processing in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef JOBTESTS_H
#define JOBTESTS_H

#include <QObject>
#include <ThreadWeaver/JobPointer>

namespace ThreadWeaver
{
class Job;
}

using ThreadWeaver::Job;

class JobTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase();
    void WeaverLazyThreadCreationTest();
    void ReduceWorkerCountTest();
    void SimpleJobTest();
    void SimpleJobCollectionTest();
    void EmptyJobCollectionTest();
    void CollectionQueueingTest();
    void GeneratingCollectionTest();
    void ShortJobSequenceTest();
    void ShortDecoratedJobSequenceTest();
    void EmptyJobSequenceTest();
    void GeneratingSequenceTest();
    void IncompleteCollectionTest();
    void EmitStartedOnFirstElementTest();
    void CollectionDependenciesTest();
    void QueueAndDequeueCollectionTest();
    void QueueAndDequeueSequenceTest();
    void BlockingExecuteTest();
    void RecursiveSequenceTest();
    void RecursiveQueueAndDequeueCollectionTest();
    void RecursiveQueueAndDequeueSequenceTest();
    void QueueAndDequeueAllCollectionTest();
    void QueueAndDequeueAllSequenceTest();
    void RecursiveQueueAndDequeueAllCollectionTest();
    void RecursiveQueueAndDequeueAllSequenceTest();
    void MassiveJobSequenceTest();
    void SimpleRecursiveSequencesTest();
    void SequenceOfSequencesTest();
    void QueueAndStopTest();
    void ResourceRestrictionPolicyBasicsTest();
    void JobSignalsAreEmittedAsynchronouslyTest();
    void JobSignalsDeliveryTest();
    void JobPointerExecutionTest();
    void DequeueSuspendedSequenceTest();
    void IdDecoratorDecoratesTest();
    void IdDecoratorAutoDeleteTest();
    void IdDecoratorSingleAllocationTest();
    void JobsAreDestroyedAfterFinishTest();
    void JobExitStatusByExceptionTest();
    void JobManualExitStatusTest();
    void QueueStreamLifecycleTest();
    void NestedGeneratingCollectionsTest();
    void NestedGeneratingSequencesTest();
    void DeeperNestedGeneratingCollectionsTest();
    void RequestAbortCollectionTest();
    void RequestAbortSequenceTest();
    void JobOnFinishTest();
    void JobOnFinishAbortTest();

public Q_SLOTS: // slots used during tests that are not test cases
    void jobStarted(ThreadWeaver::JobPointer);
    void jobDone(ThreadWeaver::JobPointer);

    void deliveryTestJobDone(ThreadWeaver::JobPointer);
};

#endif
