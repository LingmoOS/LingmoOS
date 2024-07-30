/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TESTCREATEDDATECOMPAT_H
#define TESTCREATEDDATECOMPAT_H

class testcreateddatecompat
{
};
#include <QObject>

class CreatedDateCompatTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCompat32();
    void testCompat33();
};

#endif // TESTCREATEDDATECOMPAT_H
