/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTLUNAR_H
#define TESTLUNAR_H

#include <QObject>

class LunarTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test2005();
    void test2007();
    void testIntermediatePhases();
};

#endif
