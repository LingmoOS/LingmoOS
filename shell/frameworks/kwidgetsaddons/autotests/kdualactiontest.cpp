/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include <kdualaction.h>

#include <QSignalSpy>
#include <QTest>

#include "kguiitem.h"

static const QString INACTIVE_TEXT = QStringLiteral("Show Foo");
static const QString ACTIVE_TEXT = QStringLiteral("Hide Foo");

class KDualActionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qRegisterMetaType<QAction *>("QAction*");
    }

    void testSetGuiItem()
    {
        KDualAction action(nullptr);
        action.setInactiveGuiItem(KGuiItem(INACTIVE_TEXT));
        action.setActiveGuiItem(KGuiItem(ACTIVE_TEXT));
        QCOMPARE(action.inactiveText(), INACTIVE_TEXT);
        QCOMPARE(action.activeText(), ACTIVE_TEXT);
        QCOMPARE(action.text(), INACTIVE_TEXT);
    }

    void testSetIconForStates()
    {
        QIcon icon(QPixmap(16, 16));
        QVERIFY(!icon.isNull());
        KDualAction action(nullptr);
        QVERIFY(action.inactiveIcon().isNull());
        QVERIFY(action.activeIcon().isNull());
        action.setIconForStates(icon);
        QCOMPARE(action.inactiveIcon(), icon);
        QCOMPARE(action.activeIcon(), icon);
    }

    void testSetActive()
    {
        KDualAction action(INACTIVE_TEXT, ACTIVE_TEXT, nullptr);
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QSignalSpy activeChangedSpy(&action, &KDualAction::activeChanged);
        QSignalSpy activeChangedByUserSpy(&action, &KDualAction::activeChangedByUser);

        action.setActive(true);
        QVERIFY(action.isActive());
        QCOMPARE(action.text(), ACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), true);
        QCOMPARE(activeChangedByUserSpy.count(), 0);

        action.setActive(false);
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), false);
        QCOMPARE(activeChangedByUserSpy.count(), 0);
    }

    void testTrigger()
    {
        KDualAction action(INACTIVE_TEXT, ACTIVE_TEXT, nullptr);
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QSignalSpy activeChangedSpy(&action, &KDualAction::activeChanged);
        QSignalSpy activeChangedByUserSpy(&action, &KDualAction::activeChangedByUser);

        action.trigger();
        QVERIFY(action.isActive());
        QCOMPARE(action.text(), ACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), true);
        QCOMPARE(activeChangedByUserSpy.count(), 1);
        QCOMPARE(activeChangedByUserSpy.takeFirst().at(0).toBool(), true);

        action.trigger();
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), false);
        QCOMPARE(activeChangedByUserSpy.count(), 1);
        QCOMPARE(activeChangedByUserSpy.takeFirst().at(0).toBool(), false);

        // Turn off autoToggle, nothing should happen
        action.setAutoToggle(false);
        action.trigger();
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 0);
        QCOMPARE(activeChangedByUserSpy.count(), 0);

        // Turn on autoToggle, action should change
        action.setAutoToggle(true);
        action.trigger();
        QCOMPARE(action.text(), ACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), true);
        QCOMPARE(activeChangedByUserSpy.count(), 1);
        QCOMPARE(activeChangedByUserSpy.takeFirst().at(0).toBool(), true);
    }
};

QTEST_MAIN(KDualActionTest)

#include "kdualactiontest.moc"
