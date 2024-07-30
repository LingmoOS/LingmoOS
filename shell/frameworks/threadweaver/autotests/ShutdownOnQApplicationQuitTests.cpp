/* -*- C++ -*-
    This file contains a testsuite for global queue instantiation in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QString>
#include <QTest>

#include <ThreadWeaver/ThreadWeaver>

// The tests checks if the global ThreadWeaver instance is properly
// destroyed along with QCoreApplication. After that all the puppies
// are sad and the kittens cry, and the test exits.
class ShutdownOnQApplicationQuitTests : public QObject
{
    Q_OBJECT

public:
    ShutdownOnQApplicationQuitTests();

private Q_SLOTS:
    void testShutdownOnQApplicationQuit();
};

ShutdownOnQApplicationQuitTests::ShutdownOnQApplicationQuitTests()
{
}

void ShutdownOnQApplicationQuitTests::testShutdownOnQApplicationQuit()
{
    {
        int argc = 0;
        QCoreApplication app(argc, (char **)nullptr);
        QVERIFY(ThreadWeaver::Queue::instance() != nullptr);
        ThreadWeaver::Queue::instance()->suspend();
        ThreadWeaver::Queue::instance()->resume();
        QTest::qWait(10);
    }
    QVERIFY(ThreadWeaver::Queue::instance() == nullptr);
}

QTEST_APPLESS_MAIN(ShutdownOnQApplicationQuitTests)

#include "ShutdownOnQApplicationQuitTests.moc"
