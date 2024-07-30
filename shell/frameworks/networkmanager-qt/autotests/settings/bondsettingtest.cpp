/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bondsettingtest.h"

#include "settings/bondsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void BondSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("interfaceName");
    QTest::addColumn<NMStringMap>("options");

    NMStringMap options;
    options.insert("mode", "802.3ad");

    QTest::newRow("setting1") << QString("eth0") // interfaceName
                              << options; // options
}

void BondSettingTest::testSetting()
{
    QFETCH(QString, interfaceName);
    QFETCH(NMStringMap, options);

    QVariantMap map;

    map.insert(QLatin1String("interface-name"), interfaceName);
    map.insert(QLatin1String(NM_SETTING_BOND_OPTIONS), QVariant::fromValue<NMStringMap>(options));

    NetworkManager::BondSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    QCOMPARE(map.value(QLatin1String("interface-name")), map1.value(QLatin1String("interface-name")));
    NMStringMap stringMap1 = map.value(QLatin1String(NM_SETTING_BOND_OPTIONS)).value<NMStringMap>();
    NMStringMap stringMap2 = map1.value(QLatin1String(NM_SETTING_BOND_OPTIONS)).value<NMStringMap>();

    NMStringMap::const_iterator it = stringMap1.constBegin();
    while (it != stringMap1.constEnd()) {
        QCOMPARE(it.value(), stringMap2.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(BondSettingTest)

#include "moc_bondsettingtest.cpp"
