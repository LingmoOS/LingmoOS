/*
    SPDX-FileCopyrightText: 2018 Billy Laws <blaws05@gmai.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vxlansettingtest.h"

#include "settings/vxlansetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void VxlanSettingTest::testSetting_data()
{
    QTest::addColumn<quint32>("ageing");
    QTest::addColumn<quint32>("destinationPort");
    QTest::addColumn<quint32>("id");
    QTest::addColumn<bool>("l2Miss");
    QTest::addColumn<bool>("l3Miss");
    QTest::addColumn<bool>("learning");
    QTest::addColumn<quint32>("limit");
    QTest::addColumn<QString>("local");
    QTest::addColumn<QString>("parent");
    QTest::addColumn<bool>("proxy");
    QTest::addColumn<QString>("remote");
    QTest::addColumn<bool>("rsc");
    QTest::addColumn<quint32>("sourcePortMax");
    QTest::addColumn<quint32>("sourcePortMin");
    QTest::addColumn<quint32>("tos");
    QTest::addColumn<quint32>("ttl");

    QTest::newRow("setting1") << (quint32)2 // ageing
                              << (quint32)334 // destinationPort
                              << (quint32)2 // id
                              << (bool)true // l2Miss
                              << (bool)true // l3Miss
                              << (bool)false // learning
                              << (quint32)2 // limit
                              << QString("foo") // local
                              << QString("bar") // parent
                              << (bool)true // proxy
                              << QString("foo") // remote
                              << (bool)true // rsc
                              << (quint32)2 // sourcePortMax
                              << (quint32)2 // sourcePortMin
                              << (quint32)2 // tos
                              << (quint32)2; // ttl
}

void VxlanSettingTest::testSetting()
{
    QFETCH(quint32, ageing);
    QFETCH(quint32, destinationPort);
    QFETCH(quint32, id);
    QFETCH(bool, l2Miss);
    QFETCH(bool, l3Miss);
    QFETCH(bool, learning);
    QFETCH(quint32, limit);
    QFETCH(QString, local);
    QFETCH(QString, parent);
    QFETCH(bool, proxy);
    QFETCH(QString, remote);
    QFETCH(bool, rsc);
    QFETCH(quint32, sourcePortMax);
    QFETCH(quint32, sourcePortMin);
    QFETCH(quint32, tos);
    QFETCH(quint32, ttl);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_VXLAN_AGEING), ageing);
    map.insert(QLatin1String(NM_SETTING_VXLAN_DESTINATION_PORT), destinationPort);
    map.insert(QLatin1String(NM_SETTING_VXLAN_ID), id);
    map.insert(QLatin1String(NM_SETTING_VXLAN_L2_MISS), l2Miss);
    map.insert(QLatin1String(NM_SETTING_VXLAN_L3_MISS), l3Miss);
    map.insert(QLatin1String(NM_SETTING_VXLAN_LEARNING), learning);
    map.insert(QLatin1String(NM_SETTING_VXLAN_LIMIT), limit);
    map.insert(QLatin1String(NM_SETTING_VXLAN_LOCAL), local);
    map.insert(QLatin1String(NM_SETTING_VXLAN_PARENT), parent);
    map.insert(QLatin1String(NM_SETTING_VXLAN_PROXY), proxy);
    map.insert(QLatin1String(NM_SETTING_VXLAN_REMOTE), remote);
    map.insert(QLatin1String(NM_SETTING_VXLAN_RSC), rsc);
    map.insert(QLatin1String(NM_SETTING_VXLAN_SOURCE_PORT_MAX), sourcePortMax);
    map.insert(QLatin1String(NM_SETTING_VXLAN_SOURCE_PORT_MIN), sourcePortMin);
    map.insert(QLatin1String(NM_SETTING_VXLAN_TOS), tos);
    map.insert(QLatin1String(NM_SETTING_VXLAN_TTL), ttl);

    NetworkManager::VxlanSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(VxlanSettingTest)

#include "moc_vxlansettingtest.cpp"
