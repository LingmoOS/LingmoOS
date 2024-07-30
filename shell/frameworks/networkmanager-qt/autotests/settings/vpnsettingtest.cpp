/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vpnsettingtest.h"

#include "settings/vpnsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void VpnSettingTest::testSetting_data()
{
    QTest::addColumn<NMStringMap>("data");
    QTest::addColumn<bool>("persistent");
    QTest::addColumn<NMStringMap>("secrets");
    QTest::addColumn<QString>("serviceType");
    QTest::addColumn<uint>("timeout");
    QTest::addColumn<QString>("userName");

    QTest::newRow("setting1") << NMStringMap{{"gateway", "kde.org"}} // data
                              << true // persistent
                              << NMStringMap{{"password", "kde"}} // secrets
                              << QString("openvpn") // service-type
                              << (uint)30 // timeout
                              << QString("test-user"); // user-name
}

void VpnSettingTest::testSetting()
{
    QFETCH(NMStringMap, data);
    QFETCH(bool, persistent);
    QFETCH(NMStringMap, secrets);
    QFETCH(QString, serviceType);
    QFETCH(uint, timeout);
    QFETCH(QString, userName);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_VPN_DATA), QVariant::fromValue<NMStringMap>(data));
    map.insert(QLatin1String(NM_SETTING_VPN_PERSISTENT), persistent);
    map.insert(QLatin1String(NM_SETTING_VPN_SECRETS), QVariant::fromValue<NMStringMap>(secrets));
    map.insert(QLatin1String(NM_SETTING_VPN_SERVICE_TYPE), serviceType);
    map.insert(QLatin1String(NM_SETTING_VPN_TIMEOUT), timeout);
    map.insert(QLatin1String(NM_SETTING_VPN_USER_NAME), userName);

    NetworkManager::VpnSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(VpnSettingTest)

#include "moc_vpnsettingtest.cpp"
