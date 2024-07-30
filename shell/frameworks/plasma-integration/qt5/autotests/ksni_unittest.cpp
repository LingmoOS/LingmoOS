/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QAbstractEventDispatcher>
#include <QMenu>
#include <QSignalSpy>
#include <QSystemTrayIcon>
#include <QTest>

class KSniUnitTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // test for BUG 343976
    void testHideDontCrash();
};

void KSniUnitTest::testHideDontCrash()
{
    QSystemTrayIcon *sti = new QSystemTrayIcon(this);
    QMenu *trayIconMenu = new QMenu();
    QAction *dummyAction = new QAction(QStringLiteral("foo"), sti);
    trayIconMenu->addAction(dummyAction);
    QSignalSpy menuDestroyedSpy(trayIconMenu, &QObject::destroyed);
    QVERIFY(menuDestroyedSpy.isValid());
    sti->setContextMenu(trayIconMenu);

    connect(sti, &QSystemTrayIcon::destroyed, trayIconMenu, &QMenu::deleteLater);
    sti->setVisible(true);
    sti->show();

    // now delete it
    delete sti;
    QVERIFY(menuDestroyedSpy.wait());
}

QTEST_MAIN(KSniUnitTest)

#include "ksni_unittest.moc"
