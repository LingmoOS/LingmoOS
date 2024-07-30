/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef CATEGORYTEST_H
#define CATEGORYTEST_H

#include <QObject>
#include <QTest>
#include <kunitconversion/converter.h>

using namespace KUnitConversion;

class CategoryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testInfo();
    void testUnits();
    void testConvert();
    void testInvalid();
    void testCurrencyTableUpdate();

private:
    Converter c;
};

#endif // CATEGORYTEST_H
