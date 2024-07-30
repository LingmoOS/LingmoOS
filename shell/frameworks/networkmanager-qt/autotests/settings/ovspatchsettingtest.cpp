/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ovspatchsettingtest.h"

#include "settings/ovspatchsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_OVS_PATCH_PEER "peer"
#endif

void OvsPatchSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("peer");

    QTest::newRow("setting1") << QString("abc"); // peer
}

void OvsPatchSettingTest::testSetting()
{
    QFETCH(QString, peer);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_OVS_PATCH_PEER), peer);

    NetworkManager::OvsPatchSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(OvsPatchSettingTest)

#include "moc_ovspatchsettingtest.cpp"
