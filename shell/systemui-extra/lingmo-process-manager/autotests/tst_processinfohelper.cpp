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
#include <QGSettings/qgsettings.h>
#include "processinfohelper.h"

// add necessary includes here

class TestProcessInfoHelper : public QObject
{
    Q_OBJECT

public:
    TestProcessInfoHelper();
    ~TestProcessInfoHelper();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();
};

TestProcessInfoHelper::TestProcessInfoHelper()
{

}

TestProcessInfoHelper::~TestProcessInfoHelper()
{

}

void TestProcessInfoHelper::initTestCase()
{

}

void TestProcessInfoHelper::cleanupTestCase()
{

}

void TestProcessInfoHelper::test_case1()
{
    QVERIFY(process_info_helper::processExists(1));
    QVERIFY(!process_info_helper::processExists(9999999));

    QVERIFY(process_info_helper::cmdline(1) == "/sbin/init splash");
}

QTEST_MAIN(TestProcessInfoHelper)

#include "tst_processinfohelper.moc"
