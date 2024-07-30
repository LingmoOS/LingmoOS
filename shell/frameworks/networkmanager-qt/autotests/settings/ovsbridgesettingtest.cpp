/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ovsbridgesettingtest.h"

#include "settings/ovsbridgesetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_OVS_BRIDGE_SETTING_NAME "ovs-bridge"
#define NM_SETTING_OVS_BRIDGE_FAIL_MODE "fail-mode"
#define NM_SETTING_OVS_BRIDGE_MCAST_SNOOPING_ENABLE "mcast-snooping-enable"
#define NM_SETTING_OVS_BRIDGE_RSTP_ENABLE "rstp-enable"
#define NM_SETTING_OVS_BRIDGE_STP_ENABLE "stp-enable"
#endif

void OvsBridgeSettingTest::testSetting_data()
{
    QTest::addColumn<bool>("mcastSnoopingEnable");
    QTest::addColumn<bool>("rstpEnable");
    QTest::addColumn<bool>("stpEnable");
    QTest::addColumn<QString>("failMode");

    QTest::newRow("setting1") << true // mcastSnoopingEnable
                              << true // rstpEnable
                              << true // stpEnable
                              << QString("secure"); // failMode
}

void OvsBridgeSettingTest::testSetting()
{
    QFETCH(bool, mcastSnoopingEnable);
    QFETCH(bool, rstpEnable);
    QFETCH(bool, stpEnable);
    QFETCH(QString, failMode);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_OVS_BRIDGE_MCAST_SNOOPING_ENABLE), mcastSnoopingEnable);
    map.insert(QLatin1String(NM_SETTING_OVS_BRIDGE_RSTP_ENABLE), rstpEnable);
    map.insert(QLatin1String(NM_SETTING_OVS_BRIDGE_STP_ENABLE), stpEnable);
    map.insert(QLatin1String(NM_SETTING_OVS_BRIDGE_FAIL_MODE), failMode);

    NetworkManager::OvsBridgeSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(OvsBridgeSettingTest)

#include "moc_ovsbridgesettingtest.cpp"
