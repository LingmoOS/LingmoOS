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
#include <iostream>
#include "misc.h"

// add necessary includes here

class TestMisc : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_case1();
};

void TestMisc::test_case1()
{
    QVERIFY(misc::string::startWith("lingmo-process-manager", "kyl"));
    QVERIFY(!misc::string::startWith("/home/lingmo/", "lingmo"));
    QVERIFY(misc::string::startWith("/home/lingmo/", "/home"));

    auto strList = misc::string::split(std::string("/app.slice/app-top.slice/"), '/');
    QVERIFY((strList == std::vector<std::string>{ "app.slice", "app-top.slice"}));
    QVERIFY(strList.back() == "app-top.slice");
}

QTEST_MAIN(TestMisc)

#include "tst_misc.moc"
