/*
 * SPDX-FileCopyrightText: 2020 Glen Ditchfield <GJDitchfield@acm.org>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TESTDATESERIALIZATION_H
#define TESTDATESERIALIZATION_H
#include <QObject>

class TestDateSerialization : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testNewRecurringTodo();
    void testTodoCompletedOnce();
    void testUTCOffset();
};

#endif // TESTDATESERIALIZATION_H
