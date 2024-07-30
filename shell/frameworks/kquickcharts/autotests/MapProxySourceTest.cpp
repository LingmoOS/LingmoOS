/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QTest>

#include "datasource/ArraySource.h"
#include "datasource/MapProxySource.h"

#define qs QStringLiteral

class MapProxySourceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreate()
    {
        // Basic creation should create an empty source.
        auto source = new MapProxySource{};

        QCOMPARE(source->itemCount(), 0);
        QCOMPARE(source->item(0), QVariant{});
        QCOMPARE(source->minimum(), QVariant{});
        QCOMPARE(source->maximum(), QVariant{});
        QCOMPARE(source->source(), nullptr);
    }

    void testWithArray_data()
    {
        QTest::addColumn<QVariantList>("array");
        QTest::addColumn<QVariantMap>("map");
        QTest::addColumn<QVariantList>("expected");
        QTest::addColumn<QVariant>("minimum");
        QTest::addColumn<QVariant>("maximum");

        // clang-format off
        QTest::newRow("basic mapping")
            << QVariantList{qs("one"), qs("two"), qs("three"), qs("four")}
            << QVariantMap{{qs("one"), 4}, {qs("two"), 3}, {qs("three"), 2}, {qs("four"), 1}}
            << QVariantList{4, 3, 2, 1}
            << QVariant{1}
            << QVariant{4};

        QTest::newRow("colors")
            << QVariantList{qs("red"), qs("green"), qs("blue"), qs("red"), qs("green"), qs("blue")}
            << QVariantMap{{qs("red"), QColor{Qt::red}.rgba()}, {qs("green"), QColor{Qt::green}.rgba()}, {qs("blue"), QColor{Qt::blue}.rgba()}}
            << QVariantList{QColor{Qt::red}.rgba(), QColor{Qt::green}.rgba(), QColor{Qt::blue}.rgba(), QColor{Qt::red}.rgba(), QColor{Qt::green}.rgba(), QColor{Qt::blue}.rgba()}
            << QVariant{QColor{Qt::blue}.rgba()}
            << QVariant{QColor{Qt::red}.rgba()};
        // clang-format on
    }

    void testWithArray()
    {
        auto arraySource = new ArraySource{};
        QFETCH(QVariantList, array);
        arraySource->setArray(array);

        auto mapSource = new MapProxySource{};
        mapSource->setSource(arraySource);

        QFETCH(QVariantMap, map);
        mapSource->setMap(map);

        QFETCH(QVariantList, expected);
        QCOMPARE(mapSource->itemCount(), expected.size());

        for (int i = 0; i < mapSource->itemCount(); ++i) {
            QCOMPARE(mapSource->item(i), expected.at(i));
        }

        QCOMPARE(mapSource->item(expected.size()), QVariant{});

        QFETCH(QVariant, minimum);
        QCOMPARE(mapSource->minimum(), minimum);

        QFETCH(QVariant, maximum);
        QCOMPARE(mapSource->maximum(), maximum);
    }
};

QTEST_GUILESS_MAIN(MapProxySourceTest)

#include "MapProxySourceTest.moc"
