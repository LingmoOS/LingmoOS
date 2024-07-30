/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pppsettingtest.h"

#include "settings/pppsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void PppSettingTest::testSetting_data()
{
    QTest::addColumn<bool>("noAuth");
    QTest::addColumn<bool>("refuseEap");
    QTest::addColumn<bool>("refusePap");
    QTest::addColumn<bool>("refuseChap");
    QTest::addColumn<bool>("refuseMschap");
    QTest::addColumn<bool>("refuseMschapv2");
    QTest::addColumn<bool>("noBsdComp");
    QTest::addColumn<bool>("noDeflate");
    QTest::addColumn<bool>("noVjCompare");
    QTest::addColumn<bool>("requireMppe");
    QTest::addColumn<bool>("requireMppe128");
    QTest::addColumn<bool>("mppeStateful");
    QTest::addColumn<bool>("cRtsCts");
    QTest::addColumn<quint32>("baud");
    QTest::addColumn<quint32>("mru");
    QTest::addColumn<quint32>("mtu");
    QTest::addColumn<quint32>("lcpEchoFailure");
    QTest::addColumn<quint32>("lcpEchoInterval");

    QTest::newRow("setting1") << false // noAuth
                              << true // refuseEap
                              << true // refusePap
                              << true // refuseChap
                              << true // refuseMschap
                              << true // refuseMschapv2
                              << true // noBsdComp
                              << true // noDeflate
                              << true // noVjCompare
                              << true // requireMppe
                              << true // requireMppe128
                              << true // mppeStateful
                              << true // cRtsCts
                              << (quint32)1000 // baud
                              << (quint32)128 // mru
                              << (quint32)500 // mtu
                              << (quint32)2 // lcpEchoFailure
                              << (quint32)3; // lcpEchoInterval
}

void PppSettingTest::testSetting()
{
    QFETCH(bool, noAuth);
    QFETCH(bool, refuseEap);
    QFETCH(bool, refusePap);
    QFETCH(bool, refuseChap);
    QFETCH(bool, refuseMschap);
    QFETCH(bool, refuseMschapv2);
    QFETCH(bool, noBsdComp);
    QFETCH(bool, noDeflate);
    QFETCH(bool, noVjCompare);
    QFETCH(bool, requireMppe);
    QFETCH(bool, requireMppe128);
    QFETCH(bool, mppeStateful);
    QFETCH(bool, cRtsCts);
    QFETCH(quint32, baud);
    QFETCH(quint32, mru);
    QFETCH(quint32, mtu);
    QFETCH(quint32, lcpEchoFailure);
    QFETCH(quint32, lcpEchoInterval);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_PPP_NOAUTH), noAuth);
    map.insert(QLatin1String(NM_SETTING_PPP_REFUSE_EAP), refuseEap);
    map.insert(QLatin1String(NM_SETTING_PPP_REFUSE_PAP), refusePap);
    map.insert(QLatin1String(NM_SETTING_PPP_REFUSE_CHAP), refuseChap);
    map.insert(QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAP), refuseMschap);
    map.insert(QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAPV2), refuseMschapv2);
    map.insert(QLatin1String(NM_SETTING_PPP_NOBSDCOMP), noBsdComp);
    map.insert(QLatin1String(NM_SETTING_PPP_NODEFLATE), noDeflate);
    map.insert(QLatin1String(NM_SETTING_PPP_NO_VJ_COMP), noVjCompare);
    map.insert(QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE), requireMppe);
    map.insert(QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE_128), requireMppe128);
    map.insert(QLatin1String(NM_SETTING_PPP_MPPE_STATEFUL), mppeStateful);
    map.insert(QLatin1String(NM_SETTING_PPP_CRTSCTS), cRtsCts);
    map.insert(QLatin1String(NM_SETTING_PPP_BAUD), baud);
    map.insert(QLatin1String(NM_SETTING_PPP_MRU), mru);
    map.insert(QLatin1String(NM_SETTING_PPP_MTU), mtu);
    map.insert(QLatin1String(NM_SETTING_PPP_LCP_ECHO_FAILURE), lcpEchoFailure);
    map.insert(QLatin1String(NM_SETTING_PPP_LCP_ECHO_INTERVAL), lcpEchoInterval);

    NetworkManager::PppSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(PppSettingTest)

#include "moc_pppsettingtest.cpp"
