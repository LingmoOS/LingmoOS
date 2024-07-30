/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TESTOCCURRENCEITERATOR_H
#define TESTOCCURRENCEITERATOR_H
#include <QObject>

class TestOccurrenceIterator : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIterationWithExceptions();
    void testEventsAndTodos();
    void testFilterCompletedTodos();
    void testAllDayEvents();
    void testWithExceptionThisAndFuture();
    void testSubDailyRecurrences();
    void testJournals();
    void testEndDate();
};

#endif // TESTOCCURRENCEITERATOR_H
