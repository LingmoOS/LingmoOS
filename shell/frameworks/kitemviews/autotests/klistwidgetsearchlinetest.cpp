/*
    SPDX-FileCopyrightText: 2011 Frank Reininghaus <frank78ac@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include <klistwidgetsearchline.h>

#include <QListWidget>

class KListWidgetSearchLineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testAddItems();
};

/**
 * If items are added to the list view or modified, KListWidgetSearchLine
 * should hide them if they do not match the search string, see
 *
 * https://bugs.kde.org/show_bug.cgi?id=265709
 */

void KListWidgetSearchLineTest::testAddItems()
{
    QListWidget listWidget;
    listWidget.addItem(QStringLiteral("Matching test item"));
    listWidget.addItem(QStringLiteral("Another test item"));

    KListWidgetSearchLine searchLine(nullptr, &listWidget);
    searchLine.setText(QStringLiteral("match"));

    // The initial filtering is delayed; we have to wait
    while (!listWidget.item(1)->isHidden()) {
        QTest::qWait(50);
    }

    QVERIFY(!listWidget.item(0)->isHidden());
    QVERIFY(listWidget.item(1)->isHidden());

    // Add two items
    listWidget.addItem(QStringLiteral("Another item that matches the search pattern"));
    listWidget.addItem(QStringLiteral("This item should be hidden"));

    QVERIFY(!listWidget.item(0)->isHidden());
    QVERIFY(listWidget.item(1)->isHidden());
    QVERIFY(!listWidget.item(2)->isHidden());
    QVERIFY(listWidget.item(3)->isHidden());

    // Modify an item
    listWidget.item(3)->setText(QStringLiteral("Now this item matches"));

    QVERIFY(!listWidget.item(0)->isHidden());
    QVERIFY(listWidget.item(1)->isHidden());
    QVERIFY(!listWidget.item(2)->isHidden());
    QVERIFY(!listWidget.item(3)->isHidden());
}

QTEST_MAIN(KListWidgetSearchLineTest)

#include "klistwidgetsearchlinetest.moc"
