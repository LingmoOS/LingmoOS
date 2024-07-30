/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "matchsettingtest.h"

#include "settings/matchsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 14, 0)
#define NM_SETTING_MATCH_INTERFACE_NAME "interface-name"
#endif

void MatchSettingTest::testSetting_data()
{
    QTest::addColumn<QStringList>("interfaceName");

    QTest::newRow("setting1") << QStringList{QString("name1"), QString("name2")}; // interfaceName
}

void MatchSettingTest::testSetting()
{
    QFETCH(QStringList, interfaceName);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_MATCH_INTERFACE_NAME), interfaceName);

    NetworkManager::MatchSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(MatchSettingTest)

#include "moc_matchsettingtest.cpp"
