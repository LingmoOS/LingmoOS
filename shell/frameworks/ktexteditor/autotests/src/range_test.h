/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2005 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_RANGE_TEST_H
#define KATE_RANGE_TEST_H

#include <QObject>

#include <ktexteditor/range.h>

class RangeTest : public QObject
{
    Q_OBJECT

public:
    RangeTest();
    ~RangeTest() override;

private Q_SLOTS:
    void testTextEditorRange();
    void testTextRange();
    void testInsertText();
    void testCornerCaseInsertion();
    void testCursorStringConversion();
    void testRangeStringConversion();
    void testLineRangeStringConversion();
    void testLineRange();

private:
    void rangeCheck(KTextEditor::Range &valid);
    void lineRangeCheck(KTextEditor::LineRange &range);
};

#endif
