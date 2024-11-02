/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QtTest>
#include <QCoreApplication>
#include <QEventLoop>
#include "timewheel.h"

class TestTimeWheel : public QObject
{
    Q_OBJECT

public:
    TestTimeWheel();
    ~TestTimeWheel();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();
};

TestTimeWheel::TestTimeWheel()
{

}

TestTimeWheel::~TestTimeWheel()
{

}

void TestTimeWheel::initTestCase()
{

}

void TestTimeWheel::cleanupTestCase()
{

}

void TestTimeWheel::test_case1()
{
    QEventLoop eventLoop;
    timewheel::TimeWheel tw;
    int counter = 0;
    tw.addTimer(1, [&counter]() {
        ++ counter;
    });

    auto timerId = tw.addTimer(3, [&counter]() {
        ++ counter;
    });

    tw.addTimer(2, [&tw, &counter, &timerId]() {
        ++ counter;
        tw.deleteTimer(timerId);
    });

    tw.addTimer(4, [&eventLoop, &counter, &tw]() {
        QVERIFY(counter == 2);
        eventLoop.quit();
    });
    eventLoop.exec();
}

QTEST_MAIN(TestTimeWheel)

#include "tst_timewheel.moc"
