/*
    SPDX-FileCopyrightText: 2014 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kpluralhandlingspinboxtest.h"
#include "kpluralhandlingspinbox.h"

#include <QTest>

QTEST_MAIN(KPluralHandlingSpinBoxTest)

KPluralHandlingSpinBoxTest::KPluralHandlingSpinBoxTest()
{
}

void KPluralHandlingSpinBoxTest::shouldHaveDefautValue()
{
    KPluralHandlingSpinBox spinbox;
    QCOMPARE(spinbox.suffix(), QString());
}

void KPluralHandlingSpinBoxTest::shouldUseSingularValueWhenUseValueEqualToOne()
{
    KPluralHandlingSpinBox spinbox;
    spinbox.setSuffix(ki18np("singular", "plural"));
    spinbox.setValue(1);
    QCOMPARE(spinbox.suffix(), QLatin1String("singular"));
}

void KPluralHandlingSpinBoxTest::shouldUsePlurialValueWhenUseValueSuperiorToOne()
{
    KPluralHandlingSpinBox spinbox;
    spinbox.setSuffix(ki18np("singular", "plural"));
    spinbox.setValue(2);
    QCOMPARE(spinbox.suffix(), QLatin1String("plural"));
}

void KPluralHandlingSpinBoxTest::shouldUseSingularValueWhenWeChangeValueAndFinishWithValueEqualOne()
{
    KPluralHandlingSpinBox spinbox;
    spinbox.setSuffix(ki18np("singular", "plural"));
    spinbox.setValue(2);
    spinbox.setValue(1);
    QCOMPARE(spinbox.suffix(), QLatin1String("singular"));
    QCOMPARE(spinbox.value(), 1);
}

void KPluralHandlingSpinBoxTest::shouldReturnEmptySuffix()
{
    KPluralHandlingSpinBox spinbox;
    spinbox.setValue(2);
    QCOMPARE(spinbox.suffix(), QString());
}

#include "moc_kpluralhandlingspinboxtest.cpp"
