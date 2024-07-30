/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Zoe Clifford <zoeacacia@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_BUG_205447_TEST_H
#define KATE_BUG_205447_TEST_H

#include <QObject>

class BugTest : public QObject
{
    Q_OBJECT

public:
    BugTest();
    ~BugTest() override;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void deleteSurrogates();
    void backspaceSurrogates();
};

#endif
