/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "../src/decorationshadow.h"
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(QMargins)

class DecorationShadowTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPadding_data();
    void testPadding();
    void testSizes_data();
    void testSizes();
};

void DecorationShadowTest::testPadding_data()
{
    QTest::addColumn<QByteArray>("propertyName");
    QTest::addColumn<QMargins>("padding");

    QTest::newRow("top") << QByteArrayLiteral("paddingTop") << QMargins(0, 10, 0, 0);
    QTest::newRow("right") << QByteArrayLiteral("paddingRight") << QMargins(0, 0, 10, 0);
    QTest::newRow("bottom") << QByteArrayLiteral("paddingBottom") << QMargins(0, 0, 0, 10);
    QTest::newRow("left") << QByteArrayLiteral("paddingLeft") << QMargins(10, 0, 0, 0);
}

void DecorationShadowTest::testPadding()
{
    using namespace KDecoration2;
    DecorationShadow shadow;

    QFETCH(QByteArray, propertyName);

    const int propertyIndex = shadow.metaObject()->indexOfProperty(propertyName.constData());
    QVERIFY(propertyIndex != -1);
    QMetaProperty metaProperty = shadow.metaObject()->property(propertyIndex);
    QCOMPARE(metaProperty.isReadable(), true);
    QCOMPARE(metaProperty.hasNotifySignal(), true);
    QCOMPARE(metaProperty.type(), QVariant::Int);
    QSignalSpy changedSpy(&shadow, &KDecoration2::DecorationShadow::paddingChanged);
    QVERIFY(changedSpy.isValid());

    QCOMPARE(shadow.property(propertyName.constData()).isValid(), true);
    QCOMPARE(shadow.property(propertyName.constData()).toInt(), 0);
    QFETCH(QMargins, padding);
    shadow.setPadding(padding);
    QCOMPARE(shadow.padding(), padding);
    QCOMPARE(shadow.property(propertyName.constData()).toInt(), 10);
    QCOMPARE(changedSpy.count(), 1);

    // trying to set to same value shouldn't emit the signal
    shadow.setPadding(padding);
    QCOMPARE(shadow.property(propertyName.constData()).toInt(), 10);
    QCOMPARE(changedSpy.count(), 1);

    // changing to different value should emit signal
    padding += 1;
    shadow.setPadding(padding);
    QCOMPARE(shadow.padding(), padding);
    QCOMPARE(shadow.property(propertyName.constData()).toInt(), 11);
    QCOMPARE(changedSpy.count(), 2);
}

void DecorationShadowTest::testSizes_data()
{
    QTest::addColumn<QByteArray>("propertyName");
    QTest::addColumn<QRect>("innerShadowRect");
    QTest::addColumn<QRect>("shadowRect");
    QTest::addColumn<QSize>("shadowSize");

    QTest::newRow("topLeft") << QByteArrayLiteral("topLeftGeometry") << QRect(1, 2, 5, 5) << QRect(0, 0, 1, 2) << QSize(6, 7);
    QTest::newRow("top") << QByteArrayLiteral("topGeometry") << QRect(1, 2, 1, 5) << QRect(1, 0, 1, 2) << QSize(3, 7);
    QTest::newRow("topRight") << QByteArrayLiteral("topRightGeometry") << QRect(0, 2, 2, 1) << QRect(2, 0, 1, 2) << QSize(3, 3);
    QTest::newRow("right") << QByteArrayLiteral("rightGeometry") << QRect(0, 0, 1, 2) << QRect(1, 0, 1, 2) << QSize(2, 4);
    QTest::newRow("bottomRight") << QByteArrayLiteral("bottomRightGeometry") << QRect(0, 0, 1, 4) << QRect(1, 4, 1, 2) << QSize(2, 6);
    QTest::newRow("bottom") << QByteArrayLiteral("bottomGeometry") << QRect(0, 0, 1, 1) << QRect(0, 1, 1, 2) << QSize(1, 3);
    QTest::newRow("bottomLeft") << QByteArrayLiteral("bottomLeftGeometry") << QRect(1, 0, 1, 1) << QRect(0, 1, 1, 2) << QSize(2, 3);
    QTest::newRow("left") << QByteArrayLiteral("leftGeometry") << QRect(1, 0, 1, 2) << QRect(0, 0, 1, 2) << QSize(2, 2);
}

void DecorationShadowTest::testSizes()
{
    using namespace KDecoration2;
    DecorationShadow shadow;

    QFETCH(QByteArray, propertyName);

    const int propertyIndex = shadow.metaObject()->indexOfProperty(propertyName.constData());
    QVERIFY(propertyIndex != -1);
    QMetaProperty metaProperty = shadow.metaObject()->property(propertyIndex);
    QCOMPARE(metaProperty.isReadable(), true);
    QCOMPARE(metaProperty.hasNotifySignal(), true);
    QCOMPARE(metaProperty.type(), QVariant::Rect);
    QSignalSpy changedSpy(&shadow, &KDecoration2::DecorationShadow::innerShadowRectChanged);
    QVERIFY(changedSpy.isValid());

    QCOMPARE(shadow.innerShadowRect(), QRect());
    QCOMPARE(shadow.property(propertyName.constData()).isValid(), true);
    QCOMPARE(shadow.property(propertyName.constData()).toRect(), QRect());
    QFETCH(QRect, innerShadowRect);
    QFETCH(QRect, shadowRect);
    QFETCH(QSize, shadowSize);
    shadow.setInnerShadowRect(innerShadowRect);
    QCOMPARE(shadow.innerShadowRect(), innerShadowRect);
    // property should still be invalid as the image is not yet set
    QCOMPARE(shadow.property(propertyName.constData()).toRect(), QRect());
    shadow.setShadow(QImage(shadowSize, QImage::Format_ARGB32));
    QCOMPARE(shadow.property(propertyName.constData()).toRect(), shadowRect);
    QCOMPARE(changedSpy.count(), 1);

    // trying to set to same value shouldn't emit the signal
    shadow.setInnerShadowRect(innerShadowRect);
    QCOMPARE(shadow.property(propertyName.constData()).toRect(), shadowRect);
    QCOMPARE(changedSpy.count(), 1);

    // changing to different value should emit signal
    shadow.setInnerShadowRect(innerShadowRect.adjusted(1, 1, 1, 1));
    QCOMPARE(changedSpy.count(), 2);
    QCOMPARE(shadow.innerShadowRect(), innerShadowRect.adjusted(1, 1, 1, 1));
}

QTEST_MAIN(DecorationShadowTest)
#include "shadowtest.moc"
