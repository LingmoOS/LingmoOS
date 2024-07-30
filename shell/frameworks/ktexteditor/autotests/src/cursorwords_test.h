/*
    This file is part of the KDE libraries

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CURSOR_WORDS_TEST_H
#define CURSOR_WORDS_TEST_H

#include <QObject>

class CursorWordsTest : public QObject
{
    Q_OBJECT
public:
    CursorWordsTest(QObject *parent = nullptr);
    ~CursorWordsTest() override;

private Q_SLOTS:
    void testMoveToNextWordSingleLine();
    void testMoveToPrevWordSingleLine();
    void testMoveToWordsMultipleLines();
};

#endif // CURSOR_WORDS_TEST_H
