/*
    SPDX-FileCopyrightText: 2014 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPAGEDIALOGAUTOTEST_H
#define KPAGEDIALOGAUTOTEST_H

#include <QObject>

class KPageDialogAutoTest : public QObject
{
    Q_OBJECT
public:
    KPageDialogAutoTest();

private Q_SLOTS:
    void shouldHaveDefaultValuesOnCreation();
    void shouldAddAnActionButton();
    void shouldAddTwoActionButton();
    void shouldNotAddTwoSameActionButton();
};

#endif // KPAGEDIALOGAUTOTEST_H
