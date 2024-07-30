/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "krecentfilesactiontest.h"

#include <QActionGroup>
#include <QMenu>
#include <QTest>
#include <krecentfilesaction.h>

KRecentFilesActionTest::KRecentFilesActionTest(QObject *parent)
    : QObject(parent)
{
}

KRecentFilesActionTest::~KRecentFilesActionTest()
{
}

QStringList KRecentFilesActionTest::extractActionNames(QMenu *menu)
{
    QStringList ret;
    const auto lstActions = menu->actions();
    for (const QAction *action : lstActions) {
        ret.append(action->objectName());
    }
    return ret;
}

QList<bool> KRecentFilesActionTest::extractActionEnableVisibleState(QMenu *menu)
{
    QList<bool> ret;
    const auto lstActions = menu->actions();
    for (const QAction *action : lstActions) {
        ret.append(action->isEnabled());
        ret.append(action->isVisible());
    }
    return ret;
}

void KRecentFilesActionTest::shouldHaveDefaultValue()
{
    KRecentFilesAction recentAction(nullptr);
    QVERIFY(recentAction.urls().isEmpty());
    QVERIFY(recentAction.menu());
    QVERIFY(!recentAction.menu()->actions().isEmpty());
    QCOMPARE(recentAction.menu()->actions().count(), 3);
    QCOMPARE(extractActionNames(recentAction.menu()),
             QStringList() << QLatin1String("no_entries") << QLatin1String("separator") << QLatin1String("clear_action"));
    QCOMPARE(extractActionEnableVisibleState(recentAction.menu()),
             QList<bool>() << false << true /*no_entries*/
                           << false << false /*separator*/
                           << false << false /*clear_action*/
    );
}

void KRecentFilesActionTest::shouldAddActionInTop()
{
    KRecentFilesAction recentAction(nullptr);
    recentAction.addUrl(QUrl(QStringLiteral("http://www.kde.org")));
    QList<QAction *> lstAction = recentAction.menu()->actions();
    QCOMPARE(lstAction.count(), 4);

    QCOMPARE(extractActionNames(recentAction.menu()),
             QStringList() << QString() << QLatin1String("no_entries") << QLatin1String("separator") << QLatin1String("clear_action"));
    QCOMPARE(extractActionEnableVisibleState(recentAction.menu()),
             QList<bool>() << true << true /* new action*/
                           << false << false /*no_entries*/
                           << true << true /*separator*/
                           << true << true /*clear_action*/
    );
}

void KRecentFilesActionTest::shouldClearMenu()
{
    KRecentFilesAction recentAction(nullptr);
    recentAction.addUrl(QUrl(QStringLiteral("http://www.kde.org")));
    QList<QAction *> lstAction = recentAction.menu()->actions();
    QCOMPARE(lstAction.count(), 4);
    recentAction.clear();

    lstAction = recentAction.menu()->actions();
    QCOMPARE(lstAction.count(), 3);

    QCOMPARE(extractActionNames(recentAction.menu()),
             QStringList() << QLatin1String("no_entries") << QLatin1String("separator") << QLatin1String("clear_action"));
    QCOMPARE(extractActionEnableVisibleState(recentAction.menu()),
             QList<bool>() << false << true /*no_entries*/
                           << false << false /*separator*/
                           << false << false /*clear_action*/
    );
}

void KRecentFilesActionTest::testUrlsOrder()
{
    const QUrl kde = QUrl("http://www.kde.org");
    const QUrl foo = QUrl("http://www.foo.org");
    const QUrl bar = QUrl("http://www.bar.org");

    KRecentFilesAction recentActions(nullptr);
    recentActions.addUrl(kde);
    recentActions.addUrl(foo);
    recentActions.addUrl(bar);

    const QList<QAction *> list = recentActions.menu()->actions();
    // 3 + no_entries, separator, clear_action
    QCOMPARE(list.size(), 6);

    QCOMPARE(list.at(0)->text(), " [" + bar.toString() + ']');
    QCOMPARE(list.at(1)->text(), " [" + foo.toString() + ']');
    QCOMPARE(list.at(2)->text(), " [" + kde.toString() + ']');

    const QList<QUrl> urlList = recentActions.urls();
    QCOMPARE(urlList.size(), 3);
    // urls() method returns the urls in the same order as they appear in the menu
    QCOMPARE(urlList.at(0), bar);
    QCOMPARE(urlList.at(1), foo);
    QCOMPARE(urlList.at(2), kde);
}

void KRecentFilesActionTest::addUrlAlreadyInList()
{
    const QUrl kde = QUrl("http://www.kde.org");
    const QUrl foo = QUrl("http://www.foo.org");
    const QUrl bar = QUrl("http://www.bar.org");

    KRecentFilesAction recentAction(nullptr);
    recentAction.addUrl(kde);
    recentAction.addUrl(foo);
    recentAction.addUrl(bar);

    QList<QAction *> list = recentAction.menu()->actions();
    // 3 + no_entries, separator, clear_action
    QCOMPARE(list.size(), 6);

    QCOMPARE(list.at(0)->text(), " [" + bar.toString() + ']');

    // Add kde url again
    recentAction.addUrl(kde);
    list = recentAction.menu()->actions();
    // Number of actions is the same
    QCOMPARE(list.size(), 6);
    // kde url should be now at the top
    QCOMPARE(list.at(0)->text(), " [" + kde.toString() + ']');
    // bar url is second
    QCOMPARE(list.at(1)->text(), " [" + bar.toString() + ']');

    // Add foo url again
    recentAction.addUrl(foo);
    list = recentAction.menu()->actions();
    // Number of actions is the same
    QCOMPARE(list.size(), 6);
    // foo url should be now at the top
    QCOMPARE(list.at(0)->text(), " [" + foo.toString() + ']');

    QCOMPARE(list.at(1)->text(), " [" + kde.toString() + ']');
    QCOMPARE(list.at(2)->text(), " [" + bar.toString() + ']');
}

void KRecentFilesActionTest::removeExecessItems()
{
    const QUrl kde = QUrl("http://www.kde.org");
    const QUrl foo = QUrl("http://www.foo.org");
    const QUrl bar = QUrl("http://www.bar.org");

    KRecentFilesAction recentAction(nullptr);
    recentAction.addUrl(kde);
    recentAction.addUrl(foo);
    recentAction.addUrl(bar);

    QList<QAction *> list = recentAction.selectableActionGroup()->actions();
    QCOMPARE(list.size(), 3);

    recentAction.setMaxItems(2);
    list = recentAction.menu()->actions();
    QCOMPARE(recentAction.selectableActionGroup()->actions().size(), 2);
    // Oldest url was removed
    QCOMPARE(list.at(0)->text(), " [" + bar.toString() + ']');
    QCOMPARE(list.at(1)->text(), " [" + foo.toString() + ']');
}

QTEST_MAIN(KRecentFilesActionTest)

#include "moc_krecentfilesactiontest.cpp"
