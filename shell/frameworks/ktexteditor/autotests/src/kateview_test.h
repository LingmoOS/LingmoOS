/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_VIEW_TEST_H
#define KATE_VIEW_TEST_H

#include <QObject>

class KateViewTest : public QObject
{
    Q_OBJECT

public:
    KateViewTest();
    ~KateViewTest() override;

private Q_SLOTS:
    void testReloadMultipleViews();
    void testTabCursorOnReload();
    void testLowerCaseBlockSelection();
    void testCoordinatesToCursor();
    void testCursorToCoordinates();
    void testSelection();
    void testDeselectByArrowKeys_data();
    void testDeselectByArrowKeys();
    void testKillline();
    void testKeyDeleteBlockSelection();
    void testScrollPastEndOfDocument();
    void testFoldFirstLine();
    void testDragAndDrop();
    void testGotoMatchingBracket();
    void testFindSelected();
    void testTransposeWord();

    void testFindMatchingFoldingMarker();
    void testUpdateFoldingMarkersHighlighting();
};

#endif // KATE_VIEW_TEST_H
