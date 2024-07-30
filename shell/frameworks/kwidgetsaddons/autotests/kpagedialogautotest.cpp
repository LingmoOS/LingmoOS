/*
    SPDX-FileCopyrightText: 2014 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpagedialogautotest.h"
#include <kpagedialog.h>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QTest>

QTEST_MAIN(KPageDialogAutoTest)

KPageDialogAutoTest::KPageDialogAutoTest()
{
}

void KPageDialogAutoTest::shouldHaveDefaultValuesOnCreation()
{
    KPageDialog page;
    QDialogButtonBox *dialogbuttonbox = page.findChild<QDialogButtonBox *>(QStringLiteral("buttonbox"));
    QVERIFY(dialogbuttonbox);
    QDialogButtonBox::StandardButtons standardButton = dialogbuttonbox->standardButtons();
    QDialogButtonBox::StandardButtons defaultButton = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
    QCOMPARE(standardButton, defaultButton);
}

void KPageDialogAutoTest::shouldAddAnActionButton()
{
    KPageDialog page;
    QDialogButtonBox *dialogbuttonbox = page.findChild<QDialogButtonBox *>(QStringLiteral("buttonbox"));
    QPushButton *actionButton = new QPushButton(QStringLiteral("Action1"));
    page.addActionButton(actionButton);
    QCOMPARE(dialogbuttonbox->buttons().count(), 3);
    QVERIFY(dialogbuttonbox->buttons().contains(actionButton));
}

void KPageDialogAutoTest::shouldAddTwoActionButton()
{
    KPageDialog page;
    QDialogButtonBox *dialogbuttonbox = page.findChild<QDialogButtonBox *>(QStringLiteral("buttonbox"));
    QPushButton *actionButton = new QPushButton(QStringLiteral("Action1"));
    page.addActionButton(actionButton);

    QPushButton *actionButton2 = new QPushButton(QStringLiteral("Action2"));
    page.addActionButton(actionButton2);

    QCOMPARE(dialogbuttonbox->buttons().count(), 4);
}

void KPageDialogAutoTest::shouldNotAddTwoSameActionButton()
{
    KPageDialog page;
    QDialogButtonBox *dialogbuttonbox = page.findChild<QDialogButtonBox *>(QStringLiteral("buttonbox"));
    QPushButton *actionButton = new QPushButton(QStringLiteral("Action1"));
    page.addActionButton(actionButton);

    page.addActionButton(actionButton);
    QCOMPARE(dialogbuttonbox->buttons().count(), 3);
}

#include "moc_kpagedialogautotest.cpp"
