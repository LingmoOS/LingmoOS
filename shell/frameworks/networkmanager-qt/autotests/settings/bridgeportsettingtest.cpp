/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bridgeportsettingtest.h"

#include "settings/bridgeportsetting.h"

#include <nm-version.h>

#define NM_SETTING_BRIDGE_MAC_ADDRESS "mac-address"

#include <QTest>

void BridgePortSettingTest::testSetting_data()
{
    QTest::addColumn<quint16>("priority");
    QTest::addColumn<quint16>("pathCost");
    QTest::addColumn<bool>("hairpinMode");

    QTest::newRow("setting1") << (quint16)60 // priority
                              << (quint16)30000 // pathCost
                              << true; // hairpinMode
}

void BridgePortSettingTest::testSetting()
{
    QFETCH(quint16, priority);
    QFETCH(quint16, pathCost);
    QFETCH(bool, hairpinMode);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_BRIDGE_PORT_PRIORITY), priority);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_PORT_PATH_COST), pathCost);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_PORT_PATH_COST), hairpinMode);

    NetworkManager::BridgePortSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(BridgePortSettingTest)

#include "moc_bridgeportsettingtest.cpp"
