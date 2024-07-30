/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_REGEXPSEARCH_TEST_H
#define KATE_REGEXPSEARCH_TEST_H

#include <QObject>

class RegExpSearchTest : public QObject
{
    Q_OBJECT

public:
    RegExpSearchTest();
    ~RegExpSearchTest() override;

private Q_SLOTS:
    void testReplaceEscapeSequences_data();
    void testReplaceEscapeSequences();

    void testReplacementReferences_data();
    void testReplacementReferences();

    void testReplacementCaseConversion_data();
    void testReplacementCaseConversion();

    void testReplacementCounter_data();
    void testReplacementCounter();

    void testAnchoredRegexp_data();
    void testAnchoredRegexp();

    void testSearchForward();

    void testSearchBackwardInSelection();

    void test();
    void testUnicode();
};

#endif
