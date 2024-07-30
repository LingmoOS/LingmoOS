/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_PLAINTEXTSEARCH_TEST_H
#define KATE_PLAINTEXTSEARCH_TEST_H

#include <QObject>

namespace KTextEditor
{
class DocumentPrivate;
}
class KatePlainTextSearch;

class PlainTextSearchTest : public QObject
{
    Q_OBJECT

public:
    PlainTextSearchTest();
    ~PlainTextSearchTest() override;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testSearchBackward_data();
    void testSearchBackward();

    void testSingleLineDocument_data();
    void testSingleLineDocument();

    void testMultilineSearch_data();
    void testMultilineSearch();

private:
    KTextEditor::DocumentPrivate *m_doc = nullptr;
    KatePlainTextSearch *m_search = nullptr;

public:
    static QtMessageHandler s_msgHandler;
};

#endif
