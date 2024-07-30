/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
*/

#include "kcursorsavertest.h"
#include "util/kcursorsaver.h"
#include <QTest>
QTEST_MAIN(KCursorSaverTest)

KCursorSaverTest::KCursorSaverTest(QObject *parent)
    : QObject(parent)
{
}

void KCursorSaverTest::ignoreWarning()
{
    {
        // Emit qwarning when we call twice restoreCursor
        KCursorSaver saver(Qt::WaitCursor);
        saver.restoreCursor();
        QTest::ignoreMessage(QtWarningMsg, "This KCursorSaver doesn't own the cursor anymore, invalid call to restoreCursor().");
        saver.restoreCursor();
    }
    {
        // Emit qwarning when "moving A to B and then calling A::restoreCursor()"
        KCursorSaver saverA(Qt::WaitCursor);
        KCursorSaver saverB = std::move(saverA);
        QTest::ignoreMessage(QtWarningMsg, "This KCursorSaver doesn't own the cursor anymore, invalid call to restoreCursor().");
        saverA.restoreCursor();
        QTest::ignoreMessage(QtWarningMsg, "This KCursorSaver doesn't own the cursor anymore, invalid call to restoreCursor().");
        saverB.restoreCursor();
    }
}

#include "moc_kcursorsavertest.cpp"
