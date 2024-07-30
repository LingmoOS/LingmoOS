/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "adslsettingtest.h"

#include "settings/adslsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void AdslSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("password");
    QTest::addColumn<quint32>("passwordFlags");
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("encapsulation");
    QTest::addColumn<quint32>("vpi");
    QTest::addColumn<quint32>("vci");

    QTest::newRow("setting1") << QString("username") // username
                              << QString("password") // options
                              << (quint32)2 // passwordFlags
                              << "pppoa" // protocol
                              << "llc" // encapsulation
                              << (quint32)8 // vpi
                              << (quint32)35; // vci
}

void AdslSettingTest::testSetting()
{
    QFETCH(QString, username);
    QFETCH(QString, password);
    QFETCH(quint32, passwordFlags);
    QFETCH(QString, protocol);
    QFETCH(QString, encapsulation);
    QFETCH(quint32, vpi);
    QFETCH(quint32, vci);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_ADSL_USERNAME), username);
    map.insert(QLatin1String(NM_SETTING_ADSL_PASSWORD), password);
    map.insert(QLatin1String(NM_SETTING_ADSL_PASSWORD_FLAGS), passwordFlags);
    map.insert(QLatin1String(NM_SETTING_ADSL_PROTOCOL), protocol);
    map.insert(QLatin1String(NM_SETTING_ADSL_ENCAPSULATION), encapsulation);
    map.insert(QLatin1String(NM_SETTING_ADSL_VPI), vpi);
    map.insert(QLatin1String(NM_SETTING_ADSL_VCI), vci);

    NetworkManager::AdslSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(AdslSettingTest)

#include "moc_adslsettingtest.cpp"
