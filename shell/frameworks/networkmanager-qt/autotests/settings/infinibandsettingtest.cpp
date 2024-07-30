/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "infinibandsettingtest.h"

#include "settings/infinibandsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void InfinibandSettingTest::testSetting_data()
{
    QTest::addColumn<QByteArray>("macAddress");
    QTest::addColumn<quint32>("mtu");
    QTest::addColumn<QString>("transportMode");

    QTest::newRow("setting1") << QByteArray("eth0") // macAddress
                              << (quint32)1000 // mtu
                              << "connected"; // transportMode
}

void InfinibandSettingTest::testSetting()
{
    QFETCH(QByteArray, macAddress);
    QFETCH(quint32, mtu);
    QFETCH(QString, transportMode);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_INFINIBAND_MAC_ADDRESS), macAddress);
    map.insert(QLatin1String(NM_SETTING_INFINIBAND_MTU), mtu);
    map.insert(QLatin1String(NM_SETTING_INFINIBAND_TRANSPORT_MODE), transportMode);

    NetworkManager::InfinibandSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(InfinibandSettingTest)

#include "moc_infinibandsettingtest.cpp"
