/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scriptdocument_test.h"

#include "ktexteditor/cursor.h"
#include <katedocument.h>
#include <katescriptdocument.h>
#include <ktexteditor/view.h>

#include <QJSEngine>
#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(ScriptDocumentTest)

QtMessageHandler ScriptDocumentTest::s_msgHandler = nullptr;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    switch (type) {
    case QtDebugMsg:
        /* do nothing */
        break;
    default:
        ScriptDocumentTest::s_msgHandler(type, context, msg);
    }
}

void ScriptDocumentTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    s_msgHandler = qInstallMessageHandler(myMessageOutput);
}

void ScriptDocumentTest::cleanupTestCase()
{
    qInstallMessageHandler(nullptr);
}

ScriptDocumentTest::ScriptDocumentTest()
    : QObject()
{
}

ScriptDocumentTest::~ScriptDocumentTest()
{
}

void ScriptDocumentTest::init()
{
    m_doc = new KTextEditor::DocumentPrivate;
    m_view = m_doc->createView(nullptr);
    m_scriptDoc = new KateScriptDocument(nullptr, this);
    m_scriptDoc->setDocument(m_doc);
}

void ScriptDocumentTest::cleanup()
{
    delete m_scriptDoc;
    delete m_view;
    delete m_doc;
}

#if 0
void ScriptDocumentTest::testRfind_data()
{
    QTest::addColumn<KTextEditor::Range>("searchRange");
    QTest::addColumn<KTextEditor::Range>("expectedResult");

    QTest::newRow("") << KTextEditor::Range(0, 0, 1, 10) << KTextEditor::Range(1,  6, 1, 10);
    QTest::newRow("") << KTextEditor::Range(0, 0, 1,  5) << KTextEditor::Range(1,  0, 1,  4);
    QTest::newRow("") << KTextEditor::Range(0, 0, 1,  0) << KTextEditor::Range(0, 10, 0, 14);
}

void ScriptDocumentTest::testRfind()
{
    QFETCH(KTextEditor::Range, searchRange);
    QFETCH(KTextEditor::Range, expectedResult);

    m_doc->setText("aaaa aaaa aaaa\n"
                   "aaaa  aaaa");

    QCOMPARE(m_search->search(searchRange, "aaaa", true), expectedResult);
}
#endif

void ScriptDocumentTest::testRfind_data()
{
    QTest::addColumn<KTextEditor::Cursor>("searchStart");
    QTest::addColumn<KTextEditor::Cursor>("result");

    QTest::newRow("a a a a a a a a a a a a|") << KTextEditor::Cursor(0, 23) << KTextEditor::Cursor(0, 18);
    QTest::newRow("a a a a a a a a a a a |a") << KTextEditor::Cursor(0, 22) << KTextEditor::Cursor(0, 16);
    QTest::newRow("a a a a| a a a a a a a a") << KTextEditor::Cursor(0, 7) << KTextEditor::Cursor(0, 2);
    QTest::newRow("a a a |a a a a a a a a a") << KTextEditor::Cursor(0, 6) << KTextEditor::Cursor(0, 0);
    QTest::newRow("a a a| a a a a a a a a a") << KTextEditor::Cursor(0, 5) << KTextEditor::Cursor(0, 0);
    QTest::newRow("a a |a a a a a a a a a a") << KTextEditor::Cursor(0, 4) << KTextEditor::Cursor::invalid();
}

void ScriptDocumentTest::testRfind()
{
    QFETCH(KTextEditor::Cursor, searchStart);
    QFETCH(KTextEditor::Cursor, result);

    m_scriptDoc->setText(QStringLiteral("a a a a a a a a a a a a"));

    KTextEditor::Cursor cursor = m_scriptDoc->rfind(searchStart, QStringLiteral("a a a"));
    QCOMPARE(cursor, result);
}

#include "moc_scriptdocument_test.cpp"
