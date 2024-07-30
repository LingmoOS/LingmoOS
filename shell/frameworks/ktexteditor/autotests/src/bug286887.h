/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_BUG_286887_TEST_H
#define KATE_BUG_286887_TEST_H

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

    void ctrlShiftLeft();
};

#endif
