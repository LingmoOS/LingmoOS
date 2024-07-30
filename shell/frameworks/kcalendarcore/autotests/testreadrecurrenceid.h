/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TESTREADRECURRENCEID_H
#define TESTREADRECURRENCEID_H
#include <QObject>

class TestReadRecurrenceId : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReadSingleException();
    void testReadSingleExceptionWithThisAndFuture();
    void testReadWriteSingleExceptionWithThisAndFuture();
    void testReadExceptionWithMainEvent();
};

#endif // TESTREADRECURRENCEID_H
