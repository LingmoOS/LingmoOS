/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bluetoothsettingtest.h"

#include "settings/bluetoothsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void BluetoothSettingTest::testSetting_data()
{
    QTest::addColumn<QByteArray>("address");
    QTest::addColumn<QString>("type");

    QTest::newRow("setting1") << QByteArray("00:02:72:00:d4:1a") // address
                              << QString("panu"); // type
}

void BluetoothSettingTest::testSetting()
{
    QFETCH(QByteArray, address);
    QFETCH(QString, type);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_BLUETOOTH_BDADDR), address);
    map.insert(QLatin1String(NM_SETTING_BLUETOOTH_TYPE), type);

    NetworkManager::BluetoothSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(BluetoothSettingTest)

#include "moc_bluetoothsettingtest.cpp"
