/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SEARCHBAR_TEST_H
#define KATE_SEARCHBAR_TEST_H

#include <QObject>

class SearchBarTest : public QObject
{
    Q_OBJECT

public:
    SearchBarTest();
    ~SearchBarTest() override;

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void testFindNextIncremental();

    void testFindNextZeroLengthMatch();

    void testFindNextNoNewLineAtEnd();

    void testSetMatchCaseIncremental();

    void testSetMatchCasePower();

    void testSetSelectionOnlyPower();

    void testSetSearchPattern_data();
    void testSetSearchPattern();

    void testSetSelectionOnly();

    void testFindAll_data();
    void testFindAll();

    void testReplaceInSelectionOnly();
    void testReplaceAll();

    void testFindSelectionForward_data();
    void testFindSelectionForward();

    void testRemoveWithSelectionForward_data();
    void testRemoveWithSelectionForward();

    void testRemoveInSelectionForward_data();
    void testRemoveInSelectionForward();

    void testReplaceWithDoubleSelecion_data();
    void testReplaceWithDoubleSelecion();

    void testReplaceDollar();

    void testSearchHistoryIncremental();
    void testSearchHistoryPower();

    void testReplaceInBlockMode();
    void testNonEmptyBlockSelectionAndSearchInSelectionOnlyDisabled();

    void testReplaceManyCapturesBug365124();

    void testReplaceEscapeSequence_data();
    void testReplaceEscapeSequence();
};

#endif
