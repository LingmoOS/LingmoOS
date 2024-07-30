/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "gsmsettingtest.h"

#include "settings/gsmsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void GsmSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("number");
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("password");
    QTest::addColumn<qint32>("passwordFlags");
    QTest::addColumn<QString>("apn");
    QTest::addColumn<QString>("networkId");
    QTest::addColumn<qint32>("networkType");
    QTest::addColumn<QString>("pin");
    QTest::addColumn<qint32>("pinFlags");
    QTest::addColumn<quint32>("allowedBand");
    QTest::addColumn<bool>("homeOnly");
    QTest::addColumn<QString>("deviceId");
    QTest::addColumn<QString>("simId");
    QTest::addColumn<QString>("simOperatorId");

    QTest::newRow("setting1") << QString("0123456789") // number
                              << QString("foo") // username
                              << QString("bar") // password
                              << (qint32)2 // passwordFlags
                              << QString("test") // apn
                              << QString("MCC-MNC") // networkId
                              << (qint32)1 // networkType
                              << QString("1010") // pin
                              << (qint32)4 // pinFlags
                              << (quint32)0 // allowedBand
                              << true // homeOnly
                              << QString("someDeviceId") // deviceId
                              << QString("someSimId") // simId
                              << QString("someSimOperatorId"); // simOperatorId
}

void GsmSettingTest::testSetting()
{
    QFETCH(QString, number);
    QFETCH(QString, username);
    QFETCH(QString, password);
    QFETCH(qint32, passwordFlags);
    QFETCH(QString, apn);
    QFETCH(QString, networkId);
    QFETCH(qint32, networkType);
    QFETCH(QString, pin);
    QFETCH(qint32, pinFlags);
    QFETCH(quint32, allowedBand);
    QFETCH(bool, homeOnly);
    QFETCH(QString, deviceId);
    QFETCH(QString, simId);
    QFETCH(QString, simOperatorId);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_GSM_NUMBER), number);
    map.insert(QLatin1String(NM_SETTING_GSM_USERNAME), username);
    map.insert(QLatin1String(NM_SETTING_GSM_PASSWORD), password);
    map.insert(QLatin1String(NM_SETTING_GSM_PASSWORD_FLAGS), passwordFlags);
    map.insert(QLatin1String(NM_SETTING_GSM_APN), apn);
    map.insert(QLatin1String(NM_SETTING_GSM_NETWORK_ID), networkId);
    map.insert(QLatin1String("network-type"), networkType);
    map.insert(QLatin1String("allowed-bands"), allowedBand);
    map.insert(QLatin1String(NM_SETTING_GSM_PIN), pin);
    map.insert(QLatin1String(NM_SETTING_GSM_PIN_FLAGS), pinFlags);
    map.insert(QLatin1String(NM_SETTING_GSM_HOME_ONLY), homeOnly);
    map.insert(QLatin1String(NM_SETTING_GSM_DEVICE_ID), deviceId);
    map.insert(QLatin1String(NM_SETTING_GSM_SIM_ID), simId);
    map.insert(QLatin1String(NM_SETTING_GSM_SIM_OPERATOR_ID), simOperatorId);

    NetworkManager::GsmSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(GsmSettingTest)

#include "moc_gsmsettingtest.cpp"
