/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006-2007 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTMEMORYCALENDAR_H
#define TESTMEMORYCALENDAR_H

#include <QObject>

class MemoryCalendarTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testInvalidTimeZone();
    void testEvents();
    void testIncidences();
    void testRelationsCrash();
    void testRecurrenceExceptions();
    void testChangeRecurId();
    void testRawEvents();
    void testRawEventsForDate();
    void testDeleteIncidence();
    void testUpdateIncidence();
};

#endif
