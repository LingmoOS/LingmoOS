/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2019 Konrad Materka <materka@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QMenu>
#include <QSignalSpy>
#include <QTest>

#include "../src/platformtheme/kdeplatformsystemtrayicon.h"

class KDEPlatformSystemTrayIcon_UnitTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // test for BUG 365105
    void testMenuRecreate();
    void testAddActionAfterMenuRecreate();
};

void KDEPlatformSystemTrayIcon_UnitTest::testMenuRecreate()
{
    QMenu *trayIconMenu = new QMenu();
    trayIconMenu->addAction(QStringLiteral("testAction"));

    KDEPlatformSystemTrayIcon *kpsti = new KDEPlatformSystemTrayIcon();

    // simulates first QSystemTrayIcon::show()
    kpsti->init();
    SystemTrayMenu *ourMenu = qobject_cast<SystemTrayMenu *>(kpsti->createMenu());
    trayIconMenu->setPlatformMenu(ourMenu);
    kpsti->updateMenu(trayIconMenu->platformMenu());

    QMenu *firstMenu = ourMenu->menu();
    QSignalSpy menuDestroyedSpy(firstMenu, &QObject::destroyed);
    QCOMPARE(firstMenu->actions().size(), 1);
    QCOMPARE(firstMenu->actions().first()->text(), "testAction");

    // simulates QSystemTrayIcon::hide()
    kpsti->cleanup();

    // simulates second QSystemTrayIcon::show()
    kpsti->init();
    kpsti->updateMenu(trayIconMenu->platformMenu());

    QMenu *recreatedMenu = ourMenu->menu();
    QVERIFY(firstMenu != recreatedMenu);
    QCOMPARE(recreatedMenu->actions().size(), 1);
    QCOMPARE(recreatedMenu->actions().first()->text(), "testAction");
    QCOMPARE(menuDestroyedSpy.count(), 1);
}

void KDEPlatformSystemTrayIcon_UnitTest::testAddActionAfterMenuRecreate()
{
    QMenu *trayIconMenu = new QMenu();
    trayIconMenu->addAction(QStringLiteral("testAction1"));

    KDEPlatformSystemTrayIcon *kpsti = new KDEPlatformSystemTrayIcon();

    // simulates first QSystemTrayIcon::show()
    kpsti->init();
    SystemTrayMenu *ourMenu = qobject_cast<SystemTrayMenu *>(kpsti->createMenu());
    trayIconMenu->setPlatformMenu(ourMenu);
    kpsti->updateMenu(trayIconMenu->platformMenu());

    QMenu *firstMenu = ourMenu->menu();
    QSignalSpy menuDestroyedSpy(firstMenu, &QObject::destroyed);
    QCOMPARE(firstMenu->actions().size(), 1);
    QCOMPARE(firstMenu->actions().first()->text(), "testAction1");

    // simulates QSystemTrayIcon::hide()
    kpsti->cleanup();

    // add action, internal menu is destroyed
    trayIconMenu->addAction(QStringLiteral("testAction2"));

    // simulates second QSystemTrayIcon::show()
    kpsti->init();
    kpsti->updateMenu(trayIconMenu->platformMenu());

    QMenu *recreatedMenu = ourMenu->menu();
    QVERIFY(firstMenu != recreatedMenu);
    QCOMPARE(recreatedMenu->actions().size(), 2);
    QCOMPARE(recreatedMenu->actions().first()->text(), "testAction1");
    QCOMPARE(recreatedMenu->actions().last()->text(), "testAction2");
    QCOMPARE(menuDestroyedSpy.count(), 1);
}

QTEST_MAIN(KDEPlatformSystemTrayIcon_UnitTest)

#include "kdeplatformsystemtrayicon_unittest.moc"
