/*
 *  SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kmultitabbartest.h"

#include <QSignalSpy>
#include <QTest>

#include <KMultiTabBar>

KMultiTabBarTest::KMultiTabBarTest(QObject *parent)
    : QObject(parent)
{
}

void KMultiTabBarTest::testTabsAppendAndRemove()
{
    KMultiTabBar tb;

    int id1 = 1;
    int id2 = 2;

    tb.appendTab(QIcon(), id1);
    tb.appendTab(QIcon(), id2);

    // verify that tabs were appended
    QVERIFY(tb.tab(id1));
    QVERIFY(tb.tab(id2));

    tb.removeTab(id1);
    tb.removeTab(id2);

    // verify that tabs were removed
    QVERIFY(!tb.tab(id1));
    QVERIFY(!tb.tab(id2));
}

void KMultiTabBarTest::testTabStyleChanged()
{
    KMultiTabBar tb;
    QCOMPARE(tb.tabStyle(), KMultiTabBar::VSNET);

    tb.setStyle(KMultiTabBar::KDEV3ICON);
    QCOMPARE(tb.tabStyle(), KMultiTabBar::KDEV3ICON);
}

void KMultiTabBarTest::testTabRaised()
{
    KMultiTabBar tb;

    tb.appendTab(QIcon(), /*id : */ 1);
    tb.appendTab(QIcon(), /*id : */ 2);

    // set id = 1 as active tab
    tb.setTab(1, true);

    QVERIFY(tb.isTabRaised(1));
    QVERIFY(tb.tab(1)->isChecked());
    QVERIFY(!tb.isTabRaised(2));
}

void KMultiTabBarTest::shouldEmitClicked()
{
    KMultiTabBar tb;
    int id1 = 1;
    tb.appendTab(QIcon(), id1);
    KMultiTabBarTab *tab = tb.tab(id1);
    QVERIFY(tab);

    QSignalSpy spy(tab, &KMultiTabBarTab::clicked);

    QTest::mouseClick(tab, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(KMultiTabBarTest)

#include "moc_kmultitabbartest.cpp"
