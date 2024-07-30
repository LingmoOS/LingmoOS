/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTICALFORMAT_H
#define TESTICALFORMAT_H

#include <QObject>

class ICalFormatTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDeserializeSerialize();
    void testCharsets();
    void testVolatileProperties();
    void testCuType();
    void testAlarm();
    void testDateTimeSerialization_data();
    void testDateTimeSerialization();
    void testRDate();
    void testDateTime_data();
    void testDateTime();
    void testUidGeneration();
    void testUidGenerationStability();
    void testUidGenerationUniqueness();
    void testIcalFormat();
    void testNonTextCustomProperties();
    void testAllDaySchedulingMessage();
    void testAllDayRecurringUntil();
};

#endif
