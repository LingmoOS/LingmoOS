/* -*- C++ -*-
    This file contains a testsuite for sequences in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCoreApplication>
#include <QString>
#include <QTest>

#include <ThreadWeaver/DebuggingAids>
#include <ThreadWeaver/ThreadWeaver>

#include "AppendCharacterJob.h"
#include "JobLoggingWeaver.h"
#include "SequencesTests.h"
#include "WaitForIdleAndFinished.h"

using namespace ThreadWeaver;

QMutex s_GlobalMutex;

SequencesTests::SequencesTests()
{
    setDebugLevel(true, 1);
}

void SequencesTests::RecursiveStopTest()
{
    //    auto logger = new JobLoggingWeaver();
    //    Queue queue(logger);
    Queue queue;
    WAITFORIDLEANDFINISHED(&queue);
    QString result;
    Sequence innerSequence;
    innerSequence << new AppendCharacterJob('b', &result) //
                  << new FailingAppendCharacterJob('c', &result) //
                  << new AppendCharacterJob('d', &result);

    Sequence outerSequence;
    outerSequence << new AppendCharacterJob('a', &result) //
                  << innerSequence //
                  << new AppendCharacterJob('e', &result);

    queue.stream() << outerSequence;
    queue.finish();
    QCOMPARE(result, QString::fromLatin1("abc"));
}

QTEST_MAIN(SequencesTests)

#include "moc_SequencesTests.cpp"
