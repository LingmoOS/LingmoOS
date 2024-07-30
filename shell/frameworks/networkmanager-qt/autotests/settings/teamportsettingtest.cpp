/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "teamportsettingtest.h"

#include "settings/teamportsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_TEAM_PORT_CONFIG "config"
#define NM_SETTING_TEAM_PORT_QUEUE_ID "queue-id"
#define NM_SETTING_TEAM_PORT_PRIO "prio"
#define NM_SETTING_TEAM_PORT_STICKY "sticky"
#define NM_SETTING_TEAM_PORT_LACP_PRIO "lacp-prio"
#define NM_SETTING_TEAM_PORT_LACP_KEY "lacp-key"
#define NM_SETTING_TEAM_PORT_LINK_WATCHERS "link-watchers"
#endif

void TeamPortSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("config");
    QTest::addColumn<qint32>("lacpKey");
    QTest::addColumn<qint32>("lacpPrio");
    QTest::addColumn<qint32>("prio");
    QTest::addColumn<qint32>("queueId");
    QTest::addColumn<bool>("sticky");
    QTest::addColumn<NMVariantMapList>("linkWatchers");

    NMVariantMapList linkWatchers;
    QVariantMap linkWatcher;
    linkWatcher["one"] = "1";
    linkWatcher["two"] = 2;
    linkWatchers.append(linkWatcher);

    QTest::newRow("setting1") << QString("abc") // config
                              << (qint32)1 // lacpKey
                              << (qint32)1 // lacpPrio
                              << (qint32)1 // prio
                              << (qint32)1 // queueId
                              << true // sticky
                              << linkWatchers; // linkWatchers
}

void TeamPortSettingTest::testSetting()
{
    QFETCH(QString, config);
    QFETCH(qint32, lacpKey);
    QFETCH(qint32, lacpPrio);
    QFETCH(qint32, prio);
    QFETCH(qint32, queueId);
    QFETCH(bool, sticky);
    QFETCH(NMVariantMapList, linkWatchers);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_TEAM_PORT_CONFIG), config);
    map.insert(QLatin1String(NM_SETTING_TEAM_PORT_LACP_KEY), lacpKey);
    map.insert(QLatin1String(NM_SETTING_TEAM_PORT_LACP_PRIO), lacpPrio);
    map.insert(QLatin1String(NM_SETTING_TEAM_PORT_PRIO), prio);
    map.insert(QLatin1String(NM_SETTING_TEAM_PORT_QUEUE_ID), queueId);
    map.insert(QLatin1String(NM_SETTING_TEAM_PORT_STICKY), sticky);
    map.insert(QLatin1String(NM_SETTING_TEAM_PORT_LINK_WATCHERS), QVariant::fromValue(linkWatchers));

    NetworkManager::TeamPortSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        if (it.key() != QLatin1String(NM_SETTING_TEAM_PORT_LINK_WATCHERS)) {
            QCOMPARE(it.value(), map1.value(it.key()));
        }
        ++it;
    }

    NMVariantMapList list = map.value(QLatin1String(NM_SETTING_TEAM_PORT_LINK_WATCHERS)).value<NMVariantMapList>();
    NMVariantMapList list1 = map1.value(QLatin1String(NM_SETTING_TEAM_PORT_LINK_WATCHERS)).value<NMVariantMapList>();

    QCOMPARE(list.count(), list1.count());

    int comparedMaps = 0;
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap varMap = list.at(i);
        for (int j = 0; j < list1.size(); ++j) {
            QVariantMap varMap1 = list1.at(i);
            QVariantMap::const_iterator ite = varMap.constBegin();
            int comparedvals = 0;
            while (ite != varMap.constEnd()) {
                QVariantMap::const_iterator val1 = varMap1.constFind(ite.key());
                if (val1 != varMap1.constEnd()) {
                    if (varMap.value(ite.key()) == val1.value()) {
                        ++comparedvals;
                    }
                }
                ++ite;
            }
            if (comparedvals == varMap.size()) {
                comparedMaps++;
            }
        }
    }
    ++it;
    QCOMPARE(comparedMaps, list.count());
}

QTEST_MAIN(TeamPortSettingTest)

#include "moc_teamportsettingtest.cpp"
