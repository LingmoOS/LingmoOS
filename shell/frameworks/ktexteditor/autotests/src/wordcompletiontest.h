/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_WORDCOMPLETIONTEST_H
#define KATE_WORDCOMPLETIONTEST_H

#include <QObject>
#include <ktexteditor/document.h>

class WordCompletionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void benchWordRetrievalDistinct();
    void benchWordRetrievalSame();
    void benchWordRetrievalMixed();

private:
    KTextEditor::Document *m_doc;
};

#endif // KATE_WORDCOMPLETIONTEST_H
