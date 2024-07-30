/*
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpassworddialogautotest.h"

#include <QAction>
#include <QTest>

#include <KPasswordDialog>
#include <KPasswordLineEdit>

QTEST_MAIN(KPasswordDialogAutotest)

void KPasswordDialogAutotest::shouldNotHideVisibilityActionInPlaintextMode()
{
    KPasswordDialog dialog;

    auto linePassword = dialog.findChild<KPasswordLineEdit *>(QStringLiteral("passEdit"));
    QVERIFY(linePassword);

    auto lineEdit = linePassword->lineEdit();
    QVERIFY(lineEdit);

    auto visibilityAction = lineEdit->findChild<QAction *>(QStringLiteral("visibilityAction"));
    QVERIFY(visibilityAction && !visibilityAction->isVisible());

    linePassword->lineEdit()->setText(QStringLiteral("1234"));
    QVERIFY(visibilityAction->isVisible());

    visibilityAction->trigger();
    linePassword->clear();
    QVERIFY(visibilityAction->isVisible());
}

#include "moc_kpassworddialogautotest.cpp"
