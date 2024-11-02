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
#include "configmanager.h"

// add necessary includes here

class TestConfigManager : public QObject
{
    Q_OBJECT

public:
    TestConfigManager();
    ~TestConfigManager();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();
};

TestConfigManager::TestConfigManager()
{

}

TestConfigManager::~TestConfigManager()
{

}

void TestConfigManager::initTestCase()
{

}

void TestConfigManager::cleanupTestCase()
{

}

void TestConfigManager::test_case1()
{
    ConfigManager cm;
    QGSettings gsettings("com.lingmo.ProcessManager");

    bool limitEnabled = gsettings.get("resourceLimitEnabled").toBool();
    QVERIFY(cm.background2CachedStateInterval(sched_policy::DeviceMode::PC) > 0);
    QVERIFY(cm.background2CachedStateInterval(sched_policy::DeviceMode::Tablet) > 0);
    QVERIFY(cm.background2CachedStateInterval(sched_policy::DeviceMode::SoftFreeze) > 0);
    QVERIFY(cm.reousrceLimitEnabled() == gsettings.get("resourceLimitEnabled").toBool());

    gsettings.set("resourceLimitEnabled", !limitEnabled);
    QVERIFY(cm.reousrceLimitEnabled() == gsettings.get("resourceLimitEnabled").toBool());
}

QTEST_MAIN(TestConfigManager)

#include "tst_configmanager.moc"
