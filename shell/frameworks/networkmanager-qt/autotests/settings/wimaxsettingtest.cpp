/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wimaxsettingtest.h"

#include "settings/wimaxsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void WimaxSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("networkName");
    QTest::addColumn<QByteArray>("macAddress");

    QTest::newRow("setting1") << QString("networkname") // networkName
                              << QByteArray("01:23:45:67:89:ab"); // macAddress
}

void WimaxSettingTest::testSetting()
{
    QFETCH(QString, networkName);
    QFETCH(QByteArray, macAddress);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_WIMAX_NETWORK_NAME), networkName);
    map.insert(QLatin1String(NM_SETTING_WIMAX_MAC_ADDRESS), macAddress);

    NetworkManager::WimaxSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(WimaxSettingTest)

#include "moc_wimaxsettingtest.cpp"
