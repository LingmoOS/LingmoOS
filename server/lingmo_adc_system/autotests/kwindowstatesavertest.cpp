/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwindowstatesaver.h"
#include "kconfiggroup.h"
#include "ksharedconfig.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <QFontDialog>

class KWindowStateSaverTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testTopLevelDialog();
    void testSubDialog();
};

void KWindowStateSaverTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void KWindowStateSaverTest::testTopLevelDialog()
{
    auto cfg = KSharedConfig::openStateConfig();
    cfg->deleteGroup("topLevelDialogTest");
    QSize dlgSize(720, 720);

    {
        QFontDialog dlg;
        new KWindowStateSaver(&dlg, "topLevelDialogTest");
        dlg.show();
        QTest::qWait(10); // give the window time to show up, so we simulate a user-triggered resize
        dlg.resize(dlgSize);
        QTest::qWait(500); // give the state saver time to trigger
        QCOMPARE(dlg.size(), dlgSize);
    }

    QVERIFY(cfg->hasGroup("topLevelDialogTest"));

    {
        QFontDialog dlg;
        new KWindowStateSaver(&dlg, "topLevelDialogTest");
        dlg.show();
        QTest::qWait(100); // give the window time to show up properly
        QCOMPARE(dlg.size(), dlgSize);
    }
}

void KWindowStateSaverTest::testSubDialog()
{
    QWidget mainWindow;
    mainWindow.show();
    QTest::qWait(10);

    auto cfg = KSharedConfig::openStateConfig();
    cfg->deleteGroup("subDialogTest");
    QSize dlgSize(700, 500);

    {
        auto dlg = new QFontDialog(&mainWindow);
        new KWindowStateSaver(dlg, "subDialogTest");
        dlg->show();
        QTest::qWait(10); // give the window time to show up, so we simulate a user-triggered resize
        dlg->resize(dlgSize);
        QTest::qWait(500); // give the state saver time to trigger
        QCOMPARE(dlg->size(), dlgSize);
        delete dlg;
    }

    QVERIFY(cfg->hasGroup("subDialogTest"));

    {
        auto dlg = new QFontDialog(&mainWindow);
        new KWindowStateSaver(dlg, "subDialogTest");
        dlg->show();
        QTest::qWait(100); // give the window time to show up properly
        QCOMPARE(dlg->size(), dlgSize);
    }
}

QTEST_MAIN(KWindowStateSaverTest)
#include "kwindowstatesavertest.moc"
