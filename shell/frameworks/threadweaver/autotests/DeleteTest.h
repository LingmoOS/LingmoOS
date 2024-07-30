/* -*- C++ -*-
    This file contains a testsuite for the memory management in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DELETETEST_H
#define DELETETEST_H

#include <QAtomicInt>
#include <QObject>
#include <QTest>

#include <ThreadWeaver/JobPointer>

namespace ThreadWeaver
{
class Job;
}

using namespace ThreadWeaver;

class DeleteTest : public QObject
{
    Q_OBJECT
public:
    DeleteTest();

private Q_SLOTS:
    void DeleteJobsTest();
    void MutexLockingAssertsTest();
    void DeleteCollectionTest();
    void DeleteDecoratedCollectionTest();
    void DeleteSequenceTest();

public Q_SLOTS: // not a test!
    void deleteSequence(ThreadWeaver::JobPointer job);
    void countCompletedDecoratedCollection(ThreadWeaver::JobPointer job);

Q_SIGNALS:
    void deleteSequenceTestCompleted();
    void deleteDecoratedCollectionTestCompleted();

private:
    QAtomicInt m_finishCount;
};

#endif
