/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "proxysettingtest.h"

#include "settings/proxysetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 6, 0)
#define NM_SETTING_PROXY_BROWSER_ONLY "browser-only"
#define NM_SETTING_PROXY_METHOD "method"
#define NM_SETTING_PROXY_PAC_SCRIPT "pac-script"
#define NM_SETTING_PROXY_PAC_URL "pac-url"
#endif

void ProxySettingTest::testSetting_data()
{
    QTest::addColumn<bool>("browserOnly");
    QTest::addColumn<quint32>("method");
    QTest::addColumn<QString>("pacScript");
    QTest::addColumn<QString>("pacUrl");

    QTest::newRow("setting1") << false // browserOnly
                              << (quint32)1 // method
                              << QString("script") // pacScript
                              << QString("url"); // pacUrl
}

void ProxySettingTest::testSetting()
{
    QFETCH(bool, browserOnly);
    QFETCH(quint32, method);
    QFETCH(QString, pacScript);
    QFETCH(QString, pacUrl);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_PROXY_BROWSER_ONLY), browserOnly);
    map.insert(QLatin1String(NM_SETTING_PROXY_METHOD), method);
    map.insert(QLatin1String(NM_SETTING_PROXY_PAC_SCRIPT), pacScript);
    map.insert(QLatin1String(NM_SETTING_PROXY_PAC_URL), pacUrl);

    NetworkManager::ProxySetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(ProxySettingTest)

#include "moc_proxysettingtest.cpp"
