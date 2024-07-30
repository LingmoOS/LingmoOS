/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTRECURTODO_H
#define TESTRECURTODO_H

#include <QObject>

class RecurTodoTest : public QObject
{
    Q_OBJECT
private:
    void setTimeZone(const char *zonename);
private Q_SLOTS:
    void testAllDay();
    void testNonAllDay();
    void testIsAllDay();
    void testDtStart();
    void testRecurrenceBasedOnDtStart();
    void testRecurrenceBasedOnDue();
    void testRecurrenceExdates();
    void testRecurrenceStart();
    void testHasDueDate();

    void testRecurTodo_data();
    void testRecurTodo();
};

#endif
