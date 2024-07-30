/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SCRIPTDOCUMENT_TEST_H
#define KATE_SCRIPTDOCUMENT_TEST_H

#include <QObject>

namespace KTextEditor
{
class View;
}

namespace KTextEditor
{
class DocumentPrivate;
}
class KateScriptDocument;

class ScriptDocumentTest : public QObject
{
    Q_OBJECT

public:
    ScriptDocumentTest();
    ~ScriptDocumentTest() override;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testRfind_data();
    void testRfind();

private:
    KTextEditor::DocumentPrivate *m_doc = nullptr;
    KTextEditor::View *m_view = nullptr;
    KateScriptDocument *m_scriptDoc = nullptr;

public:
    static QtMessageHandler s_msgHandler;
};

#endif
