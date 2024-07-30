/*
    SPDX-FileCopyrightText: 2014 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ksplittercollapserbuttontest.h"
#include "ksplittercollapserbutton.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <QTextEdit>
#include <qtest.h>
#include <qtestmouse.h>

#include "windowscheck.h"

Q_DECLARE_METATYPE(Qt::Orientation)

TestSplitter::TestSplitter(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);

    splitter = new QSplitter;
    lay->addWidget(splitter);
    splitter->setObjectName(QStringLiteral("splitter"));
    edit1 = new QTextEdit;
    edit1->setObjectName(QStringLiteral("edit1"));
    splitter->addWidget(edit1);

    edit2 = new QTextEdit;
    edit2->setObjectName(QStringLiteral("edit2"));
    splitter->addWidget(edit2);
}

KSplitterCollapserButtonTest::KSplitterCollapserButtonTest(QObject *parent)
    : QObject(parent)
{
}

KSplitterCollapserButtonTest::~KSplitterCollapserButtonTest()
{
}

void KSplitterCollapserButtonTest::shouldHaveDefaultValue()
{
    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    TestSplitter testSplitter;

    KSplitterCollapserButton *collapser = new KSplitterCollapserButton(testSplitter.edit2, testSplitter.splitter);

    testSplitter.show();
    QVERIFY(QTest::qWaitForWindowExposed(&testSplitter));
    QVERIFY(testSplitter.isVisible());

    QVERIFY(!collapser->isWidgetCollapsed());

    QTextEdit *edit1 = testSplitter.findChild<QTextEdit *>(QStringLiteral("edit1"));
    QVERIFY(edit1);
    QTextEdit *edit2 = testSplitter.findChild<QTextEdit *>(QStringLiteral("edit2"));
    QVERIFY(edit2);

    QSplitter *splitter = testSplitter.findChild<QSplitter *>(QStringLiteral("splitter"));
    QVERIFY(splitter);
}

void KSplitterCollapserButtonTest::shouldCollapseWhenClickOnButton()
{
    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    TestSplitter testSplitter;
    KSplitterCollapserButton *splitterCollapser = new KSplitterCollapserButton(testSplitter.edit2, testSplitter.splitter);

    testSplitter.show();
    QVERIFY(QTest::qWaitForWindowExposed(&testSplitter));

    QVERIFY(!splitterCollapser->isWidgetCollapsed());
    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isWidgetCollapsed());

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isWidgetCollapsed());
}

void KSplitterCollapserButtonTest::shouldRestoreCorrectPosition()
{
    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    TestSplitter testSplitter;

    KSplitterCollapserButton *splitterCollapser = new KSplitterCollapserButton(testSplitter.edit2, testSplitter.splitter);

    testSplitter.show();
    QVERIFY(QTest::qWaitForWindowExposed(&testSplitter));
    QVERIFY(testSplitter.isVisible());

    QVERIFY(!splitterCollapser->isWidgetCollapsed());

    QTextEdit *edit2 = testSplitter.findChild<QTextEdit *>(QStringLiteral("edit2"));

    int size = edit2->width();

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isWidgetCollapsed());
    QCOMPARE(edit2->width(), 0);

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isWidgetCollapsed());
    QCOMPARE(edit2->width(), size);
}

void KSplitterCollapserButtonTest::shouldRestoreCorrectPositionForFirstWidget()
{
    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    TestSplitter testSplitter;

    KSplitterCollapserButton *splitterCollapser = new KSplitterCollapserButton(testSplitter.edit1, testSplitter.splitter);

    testSplitter.show();
    QVERIFY(QTest::qWaitForWindowExposed(&testSplitter));
    QVERIFY(testSplitter.isVisible());

    QVERIFY(!splitterCollapser->isWidgetCollapsed());

    QTextEdit *edit1 = testSplitter.findChild<QTextEdit *>(QStringLiteral("edit1"));

    int size = edit1->width();

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isWidgetCollapsed());
    QCOMPARE(edit1->width(), 0);

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isWidgetCollapsed());
    QCOMPARE(edit1->width(), size);
}

void KSplitterCollapserButtonTest::shouldTestVerticalSplitterFirstWidget()
{
    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    TestSplitter testSplitter;
    testSplitter.splitter->setOrientation(Qt::Vertical);
    KSplitterCollapserButton *splitterCollapser = new KSplitterCollapserButton(testSplitter.edit1, testSplitter.splitter);

    testSplitter.show();
    QVERIFY(QTest::qWaitForWindowExposed(&testSplitter));
    QVERIFY(testSplitter.isVisible());

    QVERIFY(!splitterCollapser->isWidgetCollapsed());

    QTextEdit *edit1 = testSplitter.findChild<QTextEdit *>(QStringLiteral("edit1"));

    int size = edit1->height();

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isWidgetCollapsed());
    QCOMPARE(edit1->height(), 0);

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isWidgetCollapsed());
    QCOMPARE(edit1->height(), size);
}

void KSplitterCollapserButtonTest::shouldTestVerticalSplitterSecondWidget()
{
    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    TestSplitter testSplitter;
    testSplitter.splitter->setOrientation(Qt::Vertical);
    KSplitterCollapserButton *splitterCollapser = new KSplitterCollapserButton(testSplitter.edit2, testSplitter.splitter);

    testSplitter.show();
    QVERIFY(QTest::qWaitForWindowExposed(&testSplitter));
    QVERIFY(testSplitter.isVisible());

    QVERIFY(!splitterCollapser->isWidgetCollapsed());

    QTextEdit *edit2 = testSplitter.findChild<QTextEdit *>(QStringLiteral("edit2"));

    int size = edit2->height();

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(splitterCollapser->isWidgetCollapsed());
    QCOMPARE(edit2->height(), 0);

    QTest::mouseClick(splitterCollapser, Qt::LeftButton);
    QVERIFY(!splitterCollapser->isWidgetCollapsed());
    QCOMPARE(edit2->height(), size);
}

void KSplitterCollapserButtonTest::shouldBeVisible_data()
{
    QTest::addColumn<Qt::Orientation>("splitterOrientation");
    QTest::addColumn<int>("widgetPosition");

    QTest::newRow("Horizontal, first widget") << Qt::Horizontal << 0 /*first widget*/;
    QTest::newRow("Horizontal, second widget") << Qt::Horizontal << 1 /*second widget*/;
    QTest::newRow("Vertical, first widget") << Qt::Vertical << 0 /*first widget*/;
    QTest::newRow("Vertical, first widget") << Qt::Vertical << 1 /*second widget*/;
}

void KSplitterCollapserButtonTest::shouldBeVisible()
{
    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    QFETCH(Qt::Orientation, splitterOrientation);
    QFETCH(int, widgetPosition);

    // GIVEN a splitter with two widgets and a collapser button (Horizontal splitter, and apply collapser on first widget)
    TestSplitter testSplitter;
    testSplitter.splitter->setOrientation(splitterOrientation);
    QWidget *widget = (widgetPosition == 0) ? testSplitter.edit1 : testSplitter.edit2;
    KSplitterCollapserButton *collapser = new KSplitterCollapserButton(widget, testSplitter.splitter);

    // WHEN showing the splitter
    testSplitter.show();
    QVERIFY(QTest::qWaitForWindowExposed(&testSplitter));
    QVERIFY(testSplitter.isVisible());

    // THEN the button should be visible
    QVERIFY(collapser->isVisible());
    QVERIFY(collapser->x() > 0);
    QVERIFY(collapser->y() > 0);
}

void KSplitterCollapserButtonTest::shouldBeVisibleWhenMovingHandle_data()
{
    QTest::addColumn<QList<int>>("splitterPosition");
    QTest::addColumn<bool>("expectedVisibility");
    QTest::addColumn<Qt::Orientation>("splitterOrientation");
    QTest::addColumn<int>("widgetPosition");

    QTest::newRow("(Horizontal first widget) middle") << (QList<int>() << 1 << 1) << true << Qt::Horizontal << 0;
    QTest::newRow("(Horizontal first widget) 1/3") << (QList<int>() << 1 << 3) << true << Qt::Horizontal << 0;
    QTest::newRow("(Horizontal first widget) editor collapsed") << (QList<int>() << 0 << 1) << false << Qt::Horizontal << 0;
    QTest::newRow("(Horizontal first widget) second editor collapsed") << (QList<int>() << 1 << 0) << true << Qt::Horizontal << 0;

    QTest::newRow("(Horizontal second widget) middle") << (QList<int>() << 1 << 1) << true << Qt::Horizontal << 1;
    QTest::newRow("(Horizontal second widget) 1/3") << (QList<int>() << 1 << 3) << true << Qt::Horizontal << 1;
    QTest::newRow("(Horizontal second widget) editor collapsed") << (QList<int>() << 1 << 0) << true << Qt::Horizontal << 1;
    QTest::newRow("(Horizontal second widget) first editor collapsed") << (QList<int>() << 0 << 1) << true << Qt::Horizontal << 1;

    QTest::newRow("(Vertical first widget) middle") << (QList<int>() << 1 << 1) << true << Qt::Vertical << 0;
    QTest::newRow("(Vertical first widget) 1/3") << (QList<int>() << 1 << 3) << true << Qt::Vertical << 0;
    QTest::newRow("(Vertical first widget) editor collapsed") << (QList<int>() << 0 << 1) << true << Qt::Vertical << 0;
    QTest::newRow("(Vertical first widget) second editor collapsed") << (QList<int>() << 1 << 0) << true << Qt::Vertical << 0;

    QTest::newRow("(Vertical second widget) middle") << (QList<int>() << 1 << 1) << true << Qt::Vertical << 1;
    QTest::newRow("(Vertical second widget) 1/3") << (QList<int>() << 1 << 3) << true << Qt::Vertical << 1;
    QTest::newRow("(Vertical second widget) editor collapsed") << (QList<int>() << 1 << 0) << true << Qt::Vertical << 1;
    QTest::newRow("(Vertical second widget) first editor collapsed") << (QList<int>() << 0 << 1) << false << Qt::Vertical << 1;
}

void KSplitterCollapserButtonTest::shouldBeVisibleWhenMovingHandle()
{
    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    QFETCH(QList<int>, splitterPosition);
    QFETCH(bool, expectedVisibility);
    QFETCH(Qt::Orientation, splitterOrientation);
    QFETCH(int, widgetPosition);

    // GIVEN a splitter with two widgets and a collapser button (Horizontal splitter, and apply collapser on first widget)
    TestSplitter testSplitter;
    testSplitter.splitter->setOrientation(splitterOrientation);
    QWidget *widget = (widgetPosition == 0) ? testSplitter.edit1 : testSplitter.edit2;

    KSplitterCollapserButton *collapser = new KSplitterCollapserButton(widget, testSplitter.splitter);

    // WHEN showing the splitter
    testSplitter.show();
    QVERIFY(QTest::qWaitForWindowExposed(&testSplitter));
    QVERIFY(testSplitter.isVisible());

    testSplitter.splitter->setSizes(splitterPosition);
    // THEN the button should be visible
    QVERIFY(collapser->isVisible());
    if (widgetPosition == 0) {
        QCOMPARE(collapser->x() > 0, expectedVisibility);
    } else {
        QCOMPARE(collapser->y() > 0, expectedVisibility);
    }
}

QTEST_MAIN(KSplitterCollapserButtonTest)

#include "moc_ksplittercollapserbuttontest.cpp"
