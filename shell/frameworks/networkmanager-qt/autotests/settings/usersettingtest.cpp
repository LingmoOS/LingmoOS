/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "usersettingtest.h"

#include "settings/usersetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 8, 0)
#define NM_SETTING_USER_DATA "data"
#endif

void UserSettingTest::testSetting_data()
{
    QTest::addColumn<NMStringMap>("data");

    NMStringMap data;
    data.insert("dat", "abc");

    QTest::newRow("setting1") << data; // data
}

void UserSettingTest::testSetting()
{
    QFETCH(NMStringMap, data);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_USER_DATA), QVariant::fromValue<NMStringMap>(data));

    NetworkManager::UserSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    NMStringMap stringMap1 = map.value(QLatin1String(NM_SETTING_USER_DATA)).value<NMStringMap>();
    NMStringMap stringMap2 = map1.value(QLatin1String(NM_SETTING_USER_DATA)).value<NMStringMap>();

    NMStringMap::const_iterator it = stringMap1.constBegin();
    while (it != stringMap1.constEnd()) {
        QCOMPARE(it.value(), stringMap2.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(UserSettingTest)

#include "moc_usersettingtest.cpp"
