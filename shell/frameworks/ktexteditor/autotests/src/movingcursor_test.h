/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_MOVINGCURSOR_TEST_H
#define KATE_MOVINGCURSOR_TEST_H

#include <QObject>

class MovingCursorTest : public QObject
{
    Q_OBJECT

public:
    MovingCursorTest();
    ~MovingCursorTest() override;

private Q_SLOTS:
    void testMovingCursor();
    void testConvenienceApi();
    void testOperators();
    void testInvalidMovingCursor();
};

#endif // KATE_MOVINGCURSOR_TEST_H
