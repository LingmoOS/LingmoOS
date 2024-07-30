/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "../src/decorationsettings.h"
#include "mockbridge.h"
#include "mockclient.h"
#include "mockdecoration.h"
#include "mocksettings.h"
#include <QSignalSpy>
#include <QTest>
#include <QVariant>

class DecorationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreate();
    void testOpaque();
    void testSection_data();
    void testSection();
};

#ifdef _MSC_VER
QMap<QString, QVariant> makeMap(const QString &key, const QVariant &value);
#endif
void DecorationTest::testCreate()
{
    // just test that creating the Decoration doesn't crash
    MockBridge bridge;
    const QString bridgeKey = QStringLiteral("bridge");
    MockDecoration deco1(nullptr,
                         QVariantList({
#ifdef _MSC_VER
                             makeMap(bridgeKey, QVariant::fromValue(5)),
#else
                             QVariantMap({{bridgeKey, QVariant::fromValue(5)}}),
#endif
                             QVariant::fromValue(bridgeKey),
                             QVariantMap(),
#ifdef _MSC_VER
                             makeMap(bridgeKey, QVariant::fromValue(&bridge)),
#else
                             QVariantMap({{bridgeKey, QVariant::fromValue(&bridge)}})
#endif
                         }));
    QVERIFY(deco1.client());
}

void DecorationTest::testOpaque()
{
    MockBridge bridge;
    MockDecoration deco(&bridge);
    QSignalSpy opaqueChangedSpy(&deco, &KDecoration2::Decoration::opaqueChanged);
    QVERIFY(opaqueChangedSpy.isValid());
    QCOMPARE(deco.isOpaque(), false);
    deco.setOpaque(false);
    QVERIFY(opaqueChangedSpy.isEmpty());
    deco.setOpaque(true);
    QCOMPARE(opaqueChangedSpy.count(), 1);
    QCOMPARE(opaqueChangedSpy.first().first().toBool(), true);
    QCOMPARE(deco.isOpaque(), true);
    deco.setOpaque(true);
    QCOMPARE(opaqueChangedSpy.count(), 1);
    deco.setOpaque(false);
    QCOMPARE(opaqueChangedSpy.count(), 2);
    QCOMPARE(opaqueChangedSpy.first().first().toBool(), true);
    QCOMPARE(opaqueChangedSpy.last().first().toBool(), false);
    QCOMPARE(deco.isOpaque(), false);
}

Q_DECLARE_METATYPE(QMargins)
Q_DECLARE_METATYPE(Qt::WindowFrameSection)

void DecorationTest::testSection_data()
{
    QTest::addColumn<QRect>("titleBar");
    QTest::addColumn<QMargins>("margins");
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<Qt::WindowFrameSection>("expected");

    QRect r(1, 1, 98, 8);
    QMargins m(1, 10, 1, 1);
    QTest::newRow("topLeft") << r << m << QPoint(0, 0) << Qt::TopLeftSection;
    QTest::newRow("top@Left") << r << m << QPoint(1, 0) << Qt::TopSection;
    QTest::newRow("top@Right") << r << m << QPoint(100, 0) << Qt::TopSection;
    QTest::newRow("topRight") << r << m << QPoint(101, 0) << Qt::TopRightSection;
    QTest::newRow("right@top") << r << m << QPoint(101, 1) << Qt::RightSection;
    QTest::newRow("right@bottom") << r << m << QPoint(101, 109) << Qt::RightSection;
    QTest::newRow("bottomRight") << r << m << QPoint(101, 110) << Qt::BottomRightSection;
    QTest::newRow("bottom@right") << r << m << QPoint(100, 110) << Qt::BottomSection;
    QTest::newRow("bottom@left") << r << m << QPoint(1, 110) << Qt::BottomSection;
    QTest::newRow("bottomLeft") << r << m << QPoint(0, 110) << Qt::BottomLeftSection;
    QTest::newRow("left@Top") << r << m << QPoint(0, 1) << Qt::LeftSection;
    QTest::newRow("left@Bottom") << r << m << QPoint(0, 109) << Qt::LeftSection;
    QTest::newRow("title") << r << m << QPoint(1, 1) << Qt::TitleBarArea;
}

void DecorationTest::testSection()
{
    MockBridge bridge;
    auto decoSettings = std::make_shared<KDecoration2::DecorationSettings>(&bridge);
    MockDecoration deco(&bridge);
    deco.setSettings(decoSettings);

    MockSettings *settings = bridge.lastCreatedSettings();
    settings->setLargeSpacing(0);

    MockClient *client = bridge.lastCreatedClient();
    client->setWidth(100);
    client->setHeight(100);
    QCOMPARE(deco.size(), QSize(100, 100));
    QCOMPARE(deco.borderLeft(), 0);
    QCOMPARE(deco.borderTop(), 0);
    QCOMPARE(deco.borderRight(), 0);
    QCOMPARE(deco.borderBottom(), 0);
    QCOMPARE(deco.titleBar(), QRect());
    QCOMPARE(deco.sectionUnderMouse(), Qt::NoSection);

    QFETCH(QRect, titleBar);
    QFETCH(QMargins, margins);
    deco.setBorders(margins);
    QCOMPARE(deco.borderLeft(), margins.left());
    QCOMPARE(deco.borderTop(), margins.top());
    QCOMPARE(deco.borderRight(), margins.right());
    QCOMPARE(deco.borderBottom(), margins.bottom());
    deco.setTitleBar(titleBar);
    QCOMPARE(deco.titleBar(), titleBar);
    QCOMPARE(deco.size(), QSize(100 + deco.borderLeft() + deco.borderRight(), 100 + deco.borderTop() + deco.borderBottom()));

    QSignalSpy spy(&deco, &KDecoration2::Decoration::sectionUnderMouseChanged);
    QVERIFY(spy.isValid());
    QFETCH(QPoint, pos);
    QHoverEvent event(QEvent::HoverMove, QPointF(pos), QPointF(pos));
    QCoreApplication::sendEvent(&deco, &event);
    QFETCH(Qt::WindowFrameSection, expected);
    QCOMPARE(deco.sectionUnderMouse(), expected);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<Qt::WindowFrameSection>(), expected);

    QHoverEvent event2(QEvent::HoverMove, QPointF(50, 50), QPointF(50, 50));
    QCoreApplication::sendEvent(&deco, &event2);
    QCOMPARE(deco.sectionUnderMouse(), Qt::NoSection);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.first().first().value<Qt::WindowFrameSection>(), expected);
    QCOMPARE(spy.last().first().value<Qt::WindowFrameSection>(), Qt::NoSection);
}

QTEST_MAIN(DecorationTest)
#include "decorationtest.moc"
