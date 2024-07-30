/*
    This file is part of the Kate project.

    SPDX-FileCopyrightText: 2021 Jan Paul Batrina <jpmbatrina01@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_ENCODINGTEST_H
#define KATE_ENCODINGTEST_H

#include <QObject>
#include <QTest>

//  TODO: Incorporate kateencodingtest.cpp here

class KateEncodingTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void utfBomTest();
    void nonUtfNoBomTest();
};

#endif // KATE_ENCODINGTEST_H
