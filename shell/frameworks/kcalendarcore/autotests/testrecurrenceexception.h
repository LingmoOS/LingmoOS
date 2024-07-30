/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TESTRECURRENCEEXCEPTION_H
#define TESTRECURRENCEEXCEPTION_H

#include <QObject>

class TestRecurrenceException : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreateTodoException();
};

#endif // TESTRECURRENCEEXCEPTION_H
