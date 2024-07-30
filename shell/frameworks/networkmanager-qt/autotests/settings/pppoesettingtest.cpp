/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pppoesettingtest.h"

#include "settings/pppoesetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void PppoeSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("service");
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("password");
    QTest::addColumn<quint32>("passwordFlags");

    QTest::newRow("setting1") << QString("service") // service
                              << QString("username") // username
                              << QString("password") // password
                              << (quint32)3; // passwordFlags
}

void PppoeSettingTest::testSetting()
{
    QFETCH(QString, service);
    QFETCH(QString, username);
    QFETCH(QString, password);
    QFETCH(quint32, passwordFlags);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_PPPOE_SERVICE), service);
    map.insert(QLatin1String(NM_SETTING_PPPOE_USERNAME), username);
    map.insert(QLatin1String(NM_SETTING_PPPOE_PASSWORD), password);
    map.insert(QLatin1String(NM_SETTING_PPPOE_PASSWORD_FLAGS), passwordFlags);

    NetworkManager::PppoeSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(PppoeSettingTest)

#include "moc_pppoesettingtest.cpp"
