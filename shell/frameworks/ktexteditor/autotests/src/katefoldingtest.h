/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_FOLDING_TEST_H
#define KATE_FOLDING_TEST_H

#include <QObject>

class KateFoldingTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void testCrash311866();
    void testBug295632();
    void testCrash367466();
    void testUnfoldingInImportFoldingRanges();
};

#endif // KATE_FOLDING_TEST_H
