/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "serialsettingtest.h"

#include "settings/serialsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void SerialSettingTest::testSetting_data()
{
    QTest::addColumn<quint32>("baud");
    QTest::addColumn<quint32>("bits");
    QTest::addColumn<QChar>("parity");
    QTest::addColumn<quint32>("stopbits");
    QTest::addColumn<quint64>("sendDelay");

    QTest::newRow("setting1") << (quint32)56000 // baud
                              << (quint32)16 // bits
                              << QChar('E') // parity
                              << (quint32)2 // stopbits
                              << (quint64)1000; // senddelay
}

void SerialSettingTest::testSetting()
{
    QFETCH(quint32, baud);
    QFETCH(quint32, bits);
    QFETCH(QChar, parity);
    QFETCH(quint32, stopbits);
    QFETCH(quint64, sendDelay);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_SERIAL_BAUD), baud);
    map.insert(QLatin1String(NM_SETTING_SERIAL_BITS), bits);
    map.insert(QLatin1String(NM_SETTING_SERIAL_PARITY), parity);
    map.insert(QLatin1String(NM_SETTING_SERIAL_STOPBITS), stopbits);
    map.insert(QLatin1String(NM_SETTING_SERIAL_SEND_DELAY), sendDelay);

    NetworkManager::SerialSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(SerialSettingTest)

#include "moc_serialsettingtest.cpp"
