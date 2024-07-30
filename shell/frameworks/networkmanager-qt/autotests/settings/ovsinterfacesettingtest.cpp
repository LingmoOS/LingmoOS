/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ovsinterfacesettingtest.h"

#include "settings/ovsinterfacesetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_OVS_INTERFACE_TYPE "type"
#endif

void OvsInterfaceSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("setting1") << QString("internal"); // type
}

void OvsInterfaceSettingTest::testSetting()
{
    QFETCH(QString, type);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_OVS_INTERFACE_TYPE), type);

    NetworkManager::OvsInterfaceSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(OvsInterfaceSettingTest)

#include "moc_ovsinterfacesettingtest.cpp"
