/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "klineediteventhandler.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

class KLineEditEventHandlerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReturnCatching()
    {
        QDialog dialog;
        auto lineEdit = new QLineEdit(&dialog);
        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

        QSignalSpy returnSpy(lineEdit, &QLineEdit::returnPressed);
        QSignalSpy dialogSpy(&dialog, &QDialog::accepted);

        dialog.show();
        QTest::keyPress(lineEdit, Qt::Key_Enter);
        QCOMPARE(returnSpy.size(), 1);
        QCOMPARE(dialogSpy.size(), 1);

        KLineEditEventHandler::catchReturnKey(lineEdit);
        dialog.show();
        QTest::keyPress(lineEdit, Qt::Key_Enter);
        QCOMPARE(returnSpy.size(), 2);
        QCOMPARE(dialogSpy.size(), 1);
    }
};

QTEST_MAIN(KLineEditEventHandlerTest)

#include "klineediteventhandlertest.moc"
