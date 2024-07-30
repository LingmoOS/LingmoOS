/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_TEMPLATEHANDLER_TEST_H
#define KATE_TEMPLATEHANDLER_TEST_H

#include <QObject>

class TemplateHandlerTest : public QObject
{
    Q_OBJECT

public:
    TemplateHandlerTest();

private Q_SLOTS:
    void testUndo();

    void testSimpleMirror();
    void testSimpleMirror_data();

    void testAlignC();
    void testAlignC_data();

    void testDefaults();
    void testDefaults_data();

    void testDefaultMirror();

    void testFunctionMirror();

    void testNotEditableFields();
    void testNotEditableFields_data();

    void testAdjacentRanges();

    void testTab();
    void testTab_data();

    void testExitAtCursor();

    void testAutoSelection();

    void testCanRetrieveSelection();

    void testEscapes();
};

#endif
