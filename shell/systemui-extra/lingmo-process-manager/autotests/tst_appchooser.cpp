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
#include <QDebug>
#include "appchooser.h"

class TestAppChooser : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_getDefaultApp();
    void test_getAvailableAppList();

private:
    AppChooser m_appChooser;
};

void TestAppChooser::test_getDefaultApp()
{
    std::string defaultApp = m_appChooser.getDefaultAppForUrl("https://www.baidu.com");
    QVERIFY(!defaultApp.empty());

    defaultApp = m_appChooser.getDefaultAppForUrl("file:///tmp/test.txt");
    QVERIFY(!defaultApp.empty());

    defaultApp = m_appChooser.getDefaultAppForUrl("file:///tmp/picture.png");
    QVERIFY(!defaultApp.empty());
}

void TestAppChooser::test_getAvailableAppList()
{
    std::vector<std::string> appList =
        m_appChooser.getAvailableAppListForFile("/tmp/test.txt");
    QVERIFY(!appList.empty());

    appList = m_appChooser.getAvailableAppListForFile("/tmp/picture.png");
    QVERIFY(!appList.empty());
}


QTEST_MAIN(TestAppChooser)

#include "tst_appchooser.moc"
