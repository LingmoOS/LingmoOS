/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QCoreApplication>
#include <QList>

#include <common/test.h>

#include "CleanOnlineTest.h"
#include "OfflineTest.h"
#include "Process.h"

class TestRunner : public QObject
{
public:
    TestRunner()
        : m_nextToStart(0)
    {
    }

    TestRunner &addTest(Test *test)
    {
        if (m_nextToStart == 0) {
            m_tests << test;
        }
        return *this;
    }

    TestRunner &operator<<(Test *test)
    {
        addTest(test);
        return *this;
    }

    void start()
    {
        if (m_nextToStart) {
            return;
        }

        next();
    }

private:
    void next()
    {
        if (m_nextToStart >= m_tests.size()) {
            QCoreApplication::exit(0);
            return;
        }

        Test *test = m_tests[m_nextToStart++];

        QObject::connect(test, &Test::testFinished, this, &TestRunner::next, Qt::QueuedConnection);

        QTest::qExec(test);
    }

private:
    QList<Test *> m_tests;
    int m_nextToStart;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TestRunner &runner = *(new TestRunner());

    (runner << Process::exec(Process::Kill)

            // Running the tests for when the service is offline
            << new OfflineTest()

            // Running the offline tests again so that we are sure
            // nothing has changed -- no activities created, changed etc.
            << new OfflineTest()

            // Starting the manager
            << Process::exec(Process::Start)

            // Starting the online tests
            << new CleanOnlineTest() << new CleanOnlineSetup()
            << new OnlineTest()

            // Starting the manager
            << Process::exec(Process::Stop)

            << new OfflineTest() << new OfflineTest()

            << Process::exec(Process::Start) << new OnlineTest()

            << Process::exec(Process::Stop)

         )
        .start();

    return app.exec();
    // QTest::qExec(&tc, argc, argv);
}
