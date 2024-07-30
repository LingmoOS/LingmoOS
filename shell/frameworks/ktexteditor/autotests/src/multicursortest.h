/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTE_MULTI_CURSOR_TEST_H
#define KTE_MULTI_CURSOR_TEST_H

#include <QObject>

class MulticursorTest : public QObject
{
    Q_OBJECT

public:
    MulticursorTest();
    ~MulticursorTest() override;

private Q_SLOTS:
    // Creation
    static void testCreateMultiCursor();
    static void testCreateMultiCursorFromSelection();
    static void testMulticursorToggling();

    // Text transformations
    static void testKillline();

    // Insert & Remove tests
    static void insertRemoveText();
    static void backspace();
    static void keyDelete();
    static void testUndoRedo();
    static void testUndoRedoWithSelection();
    static void keyReturnIndentTest();
    static void wrapSelectionWithCharsTest();
    static void insertAutoBrackets();

    // Movement
    static void moveCharTest();
    static void moveCharInFirstOrLastLineTest();
    static void moveWordTest();
    static void homeEndKeyTest();
    static void moveUpDown();
    static void testSelectionMerge();

    // Find occurunce
    static void findNextOccurenceTest();
    static void findAllOccurenceTest();

    // Multicopypaste
    static void testMultiCopyPaste();

    // Misc
    static void testSelectionTextOrdering();
    static void testViewClear();

    // API
    static void testSetGetCursors();
    static void testSetGetSelections();
};

#endif // KATE_VIEW_TEST_H
