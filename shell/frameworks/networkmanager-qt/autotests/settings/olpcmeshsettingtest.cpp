/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "olpcmeshsettingtest.h"

#include "settings/olpcmeshsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void OlpcMeshSettingTest::testSetting_data()
{
    QTest::addColumn<QByteArray>("ssid");
    QTest::addColumn<quint32>("channel");
    QTest::addColumn<QByteArray>("dhcpAnycastAddress");

    QTest::newRow("setting1") << QByteArray("ssid") // ssid
                              << (quint32)1 // channel
                              << QByteArray("00:A0:C9:14:C8:29"); // dhcpAnycastAddress
}

void OlpcMeshSettingTest::testSetting()
{
    QFETCH(QByteArray, ssid);
    QFETCH(quint32, channel);
    QFETCH(QByteArray, dhcpAnycastAddress);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_OLPC_MESH_SSID), ssid);
    map.insert(QLatin1String(NM_SETTING_OLPC_MESH_CHANNEL), channel);
    map.insert(QLatin1String(NM_SETTING_OLPC_MESH_DHCP_ANYCAST_ADDRESS), dhcpAnycastAddress);

    NetworkManager::OlpcMeshSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(OlpcMeshSettingTest)

#include "moc_olpcmeshsettingtest.cpp"
