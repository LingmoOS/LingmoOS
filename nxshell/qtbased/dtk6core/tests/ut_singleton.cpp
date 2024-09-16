// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ut_singleton.h"
#include <gtest/gtest.h>
#include <QTest>
#include <QThread>

Singleton::Singleton(QObject *parent)
    : QObject(parent),
      count(0)
{
}

MultiSingletonTester::MultiSingletonTester(QObject *parent)
    : QObject(parent)
{
}

void MultiSingletonTester::run()
{
    Singleton::ref().count.ref();
}

int MultiSingletonTester::count() const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return Singleton::ref().count.loadRelaxed();
#else
    return Singleton::ref().count.load();
#endif
}

TEST(ut_DSingleton, testDSingleton)
{
    const int exampleCount = 5;
    QVector<QThread*> threads;
    QVector<MultiSingletonTester*> testers;
    threads.reserve(exampleCount);
    testers.reserve(exampleCount);
    for (int i = 0; i < exampleCount; i++) {
        auto thread = new QThread();
        auto tester = new MultiSingletonTester;
        tester->moveToThread(thread);
        QObject::connect(thread, &QThread::started, tester, &MultiSingletonTester::run);

        threads.push_back(thread);
        testers.push_back(tester);
        thread->start();
    }

    for (auto thread : threads) {
        thread->quit();
    }

    ASSERT_TRUE(QTest::qWaitFor([threads] {
        for (auto thread : threads) {
            if (!thread->isFinished()) {
                return false;
            }
        }
        return true;
    }));

    for (auto tester : testers) {
        ASSERT_EQ(tester->count(), exampleCount);
    }

    qDeleteAll(threads);
    qDeleteAll(testers);
}
