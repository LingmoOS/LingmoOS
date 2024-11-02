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
#include "appinfomanager.h"

// add necessary includes here

class TestAppInfoManager : public QObject
{
    Q_OBJECT

public:
    TestAppInfoManager();
    ~TestAppInfoManager();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();
    void test_case2();
    void test_appinfo();
};

TestAppInfoManager::TestAppInfoManager()
{

}

TestAppInfoManager::~TestAppInfoManager()
{

}

void TestAppInfoManager::initTestCase()
{

}

void TestAppInfoManager::cleanupTestCase()
{

}

void TestAppInfoManager::test_case1()
{
    AppInfoManager aim(std::make_shared<ConfigManager>());
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/lingmo-menu.desktop") == sched_policy::AppType::Session);
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/lingmo-kwin.desktop") == sched_policy::AppType::Session);
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/lingmo-panel.desktop") == sched_policy::AppType::Session);
    QVERIFY(aim.getAppTypeByDesktopFile("/usr/share/applications/lingmo-video.desktop") == sched_policy::AppType::Normal);
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/lingmo-sidebar.desktop") == sched_policy::AppType::Session);
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/1234.desktop") == sched_policy::AppType::Unknown);
    QVERIFY(aim.getAppTypeByDesktopFile("/usr/share/applications/lingmo-music.desktop") == sched_policy::AppType::Normal);
    QVERIFY(aim.getAppTypeByDesktopFile("/usr/share/applications/lingmo1234.desktop") == sched_policy::AppType::Unknown);
}

void TestAppInfoManager::test_case2()
{
    QEventLoop eventLoop;
    AppInfoManager aim(std::make_shared<ConfigManager>());
    aim.setAppStartedCallback([&eventLoop](const AppInfo &appInfo) {
        qDebug() << "appStarted: "
                 << QString::fromStdString(appInfo.name())
                 << QString::fromStdString(appInfo.appId());
        eventLoop.quit();
    });

    aim.handleWindowAdded("373293063");

    eventLoop.exec();
}

void TestAppInfoManager::test_appinfo()
{
    AppInfo ai1("/usr/share/applications/lingmo-video.desktop",
        45878, 0, sched_policy::AppType::Normal, sched_policy::AppState::Focus, nullptr);
    AppInfo ai2("/usr/share/applications/lingmo-video.desktop",
        45878, 0, sched_policy::AppType::Normal, sched_policy::AppState::Focus, nullptr);
    AppInfo ai3("/usr/share/applications/lingmo-video.desktop",
        45879, 0, sched_policy::AppType::Normal, sched_policy::AppState::Focus, nullptr);
    QVERIFY(ai1 == ai2);
    QVERIFY(!(ai1 == ai3));
    QVERIFY(!(ai2 == ai3));
}

QTEST_MAIN(TestAppInfoManager)

#include "tst_appinfomanager.moc"
