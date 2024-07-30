/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tunsettingtest.h"

#include "settings/tunsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void TunSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("group");
    QTest::addColumn<quint32>("mode");
    QTest::addColumn<bool>("multiQueue");
    QTest::addColumn<QString>("owner");
    QTest::addColumn<bool>("pi");
    QTest::addColumn<bool>("vnetHdr");

    QTest::newRow("setting1") << QString("randomGroup") // group
                              << (quint32)2 // mode
                              << true // multi-queue
                              << QString("randomOwner") // owner
                              << true // pi
                              << true; // vnet-hdr
}

void TunSettingTest::testSetting()
{
    QFETCH(QString, group);
    QFETCH(quint32, mode);
    QFETCH(bool, multiQueue);
    QFETCH(QString, owner);
    QFETCH(bool, pi);
    QFETCH(bool, vnetHdr);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_TUN_GROUP), group);
    map.insert(QLatin1String(NM_SETTING_TUN_MODE), mode);
    map.insert(QLatin1String(NM_SETTING_TUN_MULTI_QUEUE), multiQueue);
    map.insert(QLatin1String(NM_SETTING_TUN_OWNER), owner);
    map.insert(QLatin1String(NM_SETTING_TUN_PI), pi);
    map.insert(QLatin1String(NM_SETTING_TUN_VNET_HDR), vnetHdr);

    NetworkManager::TunSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(TunSettingTest)

#include "moc_tunsettingtest.cpp"
