/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTE_DOCUMENTCURSOR_TEST_H
#define KTE_DOCUMENTCURSOR_TEST_H

#include <QObject>

class DocumentCursorTest : public QObject
{
    Q_OBJECT

public:
    DocumentCursorTest();
    ~DocumentCursorTest() override;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testConvenienceApi();
    void testOperators();

    void testValidTextPosition();
};

#endif
