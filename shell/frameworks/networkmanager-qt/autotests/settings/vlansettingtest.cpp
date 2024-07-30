/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vlansettingtest.h"

#include "settings/vlansetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void VlanSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("interfaceName");
    QTest::addColumn<QString>("parent");
    QTest::addColumn<quint32>("id");
    QTest::addColumn<quint32>("flags");
    QTest::addColumn<QStringList>("ingressPriorityMap");
    QTest::addColumn<QStringList>("egressPriorityMap");

    QStringList mapping;
    mapping << "7:3";
    mapping << "8:4";

    QTest::newRow("setting1") << QString("eth2.1") // interfaceName
                              << QString("2815492f-7e56-435e-b2e9-246bd7cdc668") // parent
                              << (quint32)2 // id
                              << (quint32)0x02 // flags
                              << mapping // ingressPriorityMap
                              << mapping; // egressPriorityMap
}

void VlanSettingTest::testSetting()
{
    QFETCH(QString, interfaceName);
    QFETCH(QString, parent);
    QFETCH(quint32, id);
    QFETCH(quint32, flags);
    QFETCH(QStringList, ingressPriorityMap);
    QFETCH(QStringList, egressPriorityMap);

    QVariantMap map;

    map.insert(QLatin1String("interface-name"), interfaceName);
    map.insert(QLatin1String(NM_SETTING_VLAN_PARENT), parent);
    map.insert(QLatin1String(NM_SETTING_VLAN_ID), id);
    map.insert(QLatin1String(NM_SETTING_VLAN_FLAGS), flags);
    map.insert(QLatin1String(NM_SETTING_VLAN_INGRESS_PRIORITY_MAP), ingressPriorityMap);
    map.insert(QLatin1String(NM_SETTING_VLAN_EGRESS_PRIORITY_MAP), egressPriorityMap);

    NetworkManager::VlanSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(VlanSettingTest)

#include "moc_vlansettingtest.cpp"
