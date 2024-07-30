/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QTest>

#include "datasource/ArraySource.h"

class ArraySourceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreate()
    {
        // Basic creation should create an empty source.
        auto source = new ArraySource{};

        QCOMPARE(source->itemCount(), 0);
        QCOMPARE(source->item(0), QVariant{});
        QCOMPARE(source->minimum(), QVariant{});
        QCOMPARE(source->maximum(), QVariant{});
    }

    void testWithArray_data()
    {
        QTest::addColumn<QVariantList>("array");
        QTest::addColumn<int>("itemCount");
        QTest::addColumn<QVariant>("firstItem");
        QTest::addColumn<QVariant>("lastItem");
        QTest::addColumn<QVariant>("minimum");
        QTest::addColumn<QVariant>("maximum");

        QTest::newRow("simple ints") << QVariantList{0, 1, 2, 3, 4} << 5 << QVariant{0} << QVariant{4} << QVariant{0} << QVariant{4};
        QTest::newRow("random ints") << QVariantList{-3, 6, 4, 9, 4} << 5 << QVariant{-3} << QVariant{4} << QVariant{-3} << QVariant{9};
        QTest::newRow("floats") << QVariantList{2.56, 4.45, 2.5, 5.3} << 4 << QVariant{2.56} << QVariant{5.3} << QVariant{2.5} << QVariant{5.3};
    }

    void testWithArray()
    {
        auto source = new ArraySource{};
        QFETCH(QVariantList, array);
        source->setArray(array);

        QFETCH(int, itemCount);
        QCOMPARE(source->itemCount(), itemCount);

        QFETCH(QVariant, firstItem);
        QCOMPARE(source->item(0), firstItem);

        QFETCH(QVariant, lastItem);
        QCOMPARE(source->item(itemCount - 1), lastItem);

        QCOMPARE(source->item(itemCount), QVariant{});

        QFETCH(QVariant, minimum);
        QCOMPARE(source->minimum(), minimum);

        QFETCH(QVariant, maximum);
        QCOMPARE(source->maximum(), maximum);
    }

    void testWrap_data()
    {
        QTest::addColumn<QVariantList>("array");
        QTest::addColumn<int>("itemCount");
        QTest::addColumn<QVariant>("firstItem");
        QTest::addColumn<QVariant>("lastItem");

        QTest::newRow("simple ints") << QVariantList{0, 1, 2, 3, 4} << 5 << QVariant{0} << QVariant{4};
        QTest::newRow("random ints") << QVariantList{-3, 6, 4, 9, 4} << 5 << QVariant{-3} << QVariant{4};
        QTest::newRow("floats") << QVariantList{2.56, 4.45, 2.5, 5.3} << 4 << QVariant{2.56} << QVariant{5.3};
    }

    void testWrap()
    {
        auto source = new ArraySource{};
        QFETCH(QVariantList, array);
        source->setArray(array);
        source->setWrap(true);

        QFETCH(int, itemCount);
        QFETCH(QVariant, firstItem);
        QFETCH(QVariant, lastItem);

        QCOMPARE(source->item(0), firstItem);
        QCOMPARE(source->item(itemCount - 1), lastItem);
        QCOMPARE(source->item(itemCount), firstItem);
        QCOMPARE(source->item(itemCount * 2 - 1), lastItem);

        QCOMPARE(source->item(itemCount * 99), firstItem);
    }
};

QTEST_GUILESS_MAIN(ArraySourceTest)

#include "ArraySourceTest.moc"
