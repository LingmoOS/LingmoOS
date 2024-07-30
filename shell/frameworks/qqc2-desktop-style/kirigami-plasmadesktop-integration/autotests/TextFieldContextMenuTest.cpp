/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTest>

#include "common.h"

using namespace Qt::StringLiterals;

class TextFieldContextMenuTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testBug481293();

private:
    QImage screenshot();
    QQmlApplicationEngine engine;
    QQuickWindow *m_rootWindow = nullptr;
};

QImage TextFieldContextMenuTest::screenshot()
{
    const auto result = m_rootWindow->contentItem()->grabToImage();
    QSignalSpy resultSpy(result.get(), &QQuickItemGrabResult::ready);
    if (result->image().isNull()) {
        Q_ASSERT(resultSpy.wait());
    }
    const QImage img = result->image();
    Q_ASSERT(!img.isNull());
    return img;
}

void TextFieldContextMenuTest::initTestCase()
{
    Common::initTestCase();

    QSignalSpy objectCreatedSpy(&engine, &QQmlApplicationEngine::objectCreated);
    engine.load(QFINDTESTDATA(u"TextFieldContextMenuTest.qml"_s));
    if (objectCreatedSpy.empty()) {
        QVERIFY(objectCreatedSpy.wait());
    }
    QCOMPARE(engine.rootObjects().size(), 1);
    m_rootWindow = static_cast<QQuickItem *>(engine.rootObjects()[0]->children()[1])->window();
    QSignalSpy readySpy(m_rootWindow->contentItem(), &QQuickItem::heightChanged);
    readySpy.wait();
}

void TextFieldContextMenuTest::cleanupTestCase()
{
}

void TextFieldContextMenuTest::testBug481293()
{
    // Take the first screenshot without the menu
    const QImage img1 = screenshot();
    // Start right click
    QTest::mousePress(m_rootWindow, Qt::RightButton);
    QTest::qWait(3000);
    // Take the second screenshot with the menu
    const QImage img2 = screenshot();
    QTest::mouseRelease(m_rootWindow, Qt::RightButton);
    // Make sure the menu is visible by comparing the two images
    QCOMPARE(img1.size(), img2.size());
    QCOMPARE_NE(img1, img2);
}

QTEST_MAIN(TextFieldContextMenuTest)

#include "TextFieldContextMenuTest.moc"
