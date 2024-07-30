/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "cdmasettingtest.h"

#include "settings/cdmasetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void CdmaSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("number");
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("password");
    QTest::addColumn<quint32>("passwordFlags");

    QTest::newRow("setting1") << QString("#123456789") // number
                              << QString("username") // username
                              << QString("password") // password
                              << (quint32)2; // passwordFlags
}

void CdmaSettingTest::testSetting()
{
    QFETCH(QString, number);
    QFETCH(QString, username);
    QFETCH(QString, password);
    QFETCH(quint32, passwordFlags);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_CDMA_NUMBER), number);
    map.insert(QLatin1String(NM_SETTING_CDMA_USERNAME), username);
    map.insert(QLatin1String(NM_SETTING_CDMA_PASSWORD), password);
    map.insert(QLatin1String(NM_SETTING_CDMA_PASSWORD_FLAGS), passwordFlags);

    NetworkManager::CdmaSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(CdmaSettingTest)

#include "moc_cdmasettingtest.cpp"
