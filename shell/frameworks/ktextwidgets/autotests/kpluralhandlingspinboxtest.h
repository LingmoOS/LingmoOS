/*
    SPDX-FileCopyrightText: 2014 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KPLURALHANDLINGSPINBOXTEST_H
#define KPLURALHANDLINGSPINBOXTEST_H

#include <QObject>

class KPluralHandlingSpinBoxTest : public QObject
{
    Q_OBJECT
public:
    KPluralHandlingSpinBoxTest();

private Q_SLOTS:
    void shouldHaveDefautValue();
    void shouldUseSingularValueWhenUseValueEqualToOne();
    void shouldUsePlurialValueWhenUseValueSuperiorToOne();
    void shouldUseSingularValueWhenWeChangeValueAndFinishWithValueEqualOne();
    void shouldReturnEmptySuffix();
};

#endif // KPLURALHANDLINGSPINBOXTEST_H
