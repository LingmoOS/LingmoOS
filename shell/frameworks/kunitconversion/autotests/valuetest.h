/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef VALUE_TEST_H
#define VALUE_TEST_H

#include <QObject>
#include <QTest>
#include <kunitconversion/converter.h>

using namespace KUnitConversion;

class ValueTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testStrings();
    void testRound();
    void testConvert();
    void testInvalid();
    void testCurrencyNotDownloaded();

private:
    Value v1;
    Value v2;
    Value v3;
};

#endif
