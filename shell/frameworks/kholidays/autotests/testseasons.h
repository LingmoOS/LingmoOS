/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTSEASONS_H
#define TESTSEASONS_H

#include <QObject>

class SeasonsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test_data();
    void test();
};

#endif
