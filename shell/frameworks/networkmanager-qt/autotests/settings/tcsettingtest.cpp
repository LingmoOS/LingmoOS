/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tcsettingtest.h"

#include "settings/tcsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_TC_CONFIG_QDISCS "qdiscs"
#define NM_SETTING_TC_CONFIG_TFILTERS "tfilters"
#endif

void TcSettingTest::testSetting_data()
{
    QTest::addColumn<NMVariantMapList>("qdiscs");
    QTest::addColumn<NMVariantMapList>("tfilters");

    NMVariantMapList qdiscs;
    QVariantMap qdisc;
    qdisc["one"] = "1";
    qdisc["two"] = 2;
    qdiscs.append(qdisc);

    NMVariantMapList tfilters;
    QVariantMap tfilter;
    tfilter["three"] = "3";
    tfilter["four"] = 4;
    tfilters.append(tfilter);

    QTest::newRow("setting1") << qdiscs // qdiscs
                              << tfilters; // tfilters
}

void TcSettingTest::testSetting()
{
    QFETCH(NMVariantMapList, tfilters);
    QFETCH(NMVariantMapList, qdiscs);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_TC_CONFIG_TFILTERS), QVariant::fromValue(tfilters));
    map.insert(QLatin1String(NM_SETTING_TC_CONFIG_QDISCS), QVariant::fromValue(qdiscs));

    NetworkManager::TcSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        NMVariantMapList list = it.value().value<NMVariantMapList>();
        NMVariantMapList list1 = map1.value(it.key()).value<NMVariantMapList>();

        QCOMPARE(list.count(), list1.count());

        int comparedMaps = 0;
        for (int i = 0; i < list.size(); ++i) {
            QVariantMap varMap = list.at(i);
            for (int j = 0; j < list1.size(); ++j) {
                QVariantMap varMap1 = list1.at(j);
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
}

QTEST_MAIN(TcSettingTest)

#include "moc_tcsettingtest.cpp"
