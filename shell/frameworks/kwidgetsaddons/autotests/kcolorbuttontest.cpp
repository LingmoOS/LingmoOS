/*
    SPDX-FileCopyrightText: 2013 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorbuttontest.h"

#include <QColorDialog>
#include <QTest>
#include <kcolorbutton.h>

#include "windowscheck.h"

QTEST_MAIN(KColorButtonTest)

void KColorButtonTest::testOpenDialog()
{
    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    KColorButton colorButton(Qt::red);
    colorButton.show();
    QVERIFY(QTest::qWaitForWindowExposed(&colorButton));
    QTest::mouseClick(&colorButton, Qt::LeftButton);
    QColorDialog *dialog = colorButton.findChild<QColorDialog *>();
    QVERIFY(dialog != nullptr);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));
    QCOMPARE(dialog->currentColor(), QColor(Qt::red));
}

#include "moc_kcolorbuttontest.cpp"
