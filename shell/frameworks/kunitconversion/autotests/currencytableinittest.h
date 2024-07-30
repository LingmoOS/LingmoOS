/*
 *   SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef CURRENCYTABLEINIT_TEST_H
#define CURRENCYTABLEINIT_TEST_H

#include <QObject>
#include <QTest>
#include <kunitconversion/converter.h>

using namespace KUnitConversion;

class CurrencyTableInitTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    /**
     * Check that the currency converter is correctly initialized when currency.xml is recent and available
     */
    void testCategoryInit();
};

#endif
