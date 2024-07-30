/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QCoreApplication>
#include <QList>
#include <QTest>

#include <common/test.h>

#include "QueryTest.h"
#include "ResultSetQuickCheckTest.h"
#include "ResultSetTest.h"
#include "ResultWatcherTest.h"

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

        if (m_tests.size() == 0) {
            // We do not have a QCoreApplication here, calling system exit
            ::exit(0);
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

    qDebug() << app.arguments();

    bool all = (app.arguments().size() <= 1);

#define ADD_TEST(TestName)                                                                                                                                     \
    qDebug() << "Test " << #TestName << " is enabled " << (all || app.arguments().contains(QLatin1String(#TestName)));                                         \
    if (all || app.arguments().contains(QLatin1String(#TestName))) {                                                                                           \
        runner << new TestName##Test();                                                                                                                        \
    }

    ADD_TEST(Query)
    ADD_TEST(ResultSet)
    ADD_TEST(ResultSetQuickCheck)
    ADD_TEST(ResultWatcher)

    runner.start();

#undef ADD_TEST
    return app.exec();
    // QTest::qExec(&tc, argc, argv);
}
