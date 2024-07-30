/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Zoe Clifford <zoeacacia@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bug205447.h"

#include <katedocument.h>
#include <kateview.h>
#include <kmainwindow.h>
#include <ktexteditor/documentcursor.h>

#include <QStandardPaths>
#include <QTest>

#include "testutils.h"

QTEST_MAIN(BugTest)

using namespace KTextEditor;

BugTest::BugTest()
    : QObject()
{
}

BugTest::~BugTest()
{
}

void BugTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void BugTest::cleanupTestCase()
{
}

void BugTest::deleteSurrogates()
{
    // set up document and view and open test file
    KTextEditor::DocumentPrivate doc;
    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    const QUrl url = QUrl::fromLocalFile(QLatin1String(TEST_DATA_DIR "bug205447.txt"));
    doc.setEncoding(QStringLiteral("UTF-8"));
    QVERIFY(doc.openUrl(url));

    // test delete
    // get UTF-32 representation of original line (before any deletes)
    QList<uint> line = doc.line(0).toUcs4();
    QVERIFY(line.size() == 23);
    // delete from start of line
    view->setCursorPosition(Cursor(0, 0));
    QVERIFY(DocumentCursor(&doc, view->cursorPosition()).isValidTextPosition());
    for (int i = 0; i < line.size(); i++) {
        // get the current line, after `i` delete presses, and convert it to utf32
        // then ensure it's the expected substring of the original line
        QList<uint> current = doc.line(0).toUcs4();
        QCOMPARE(current, line.mid(i));

        // press the delete key and verify that the new text position isn't invalid
        view->keyDelete();
        QVERIFY(DocumentCursor(&doc, view->cursorPosition()).isValidTextPosition());
    }
    QCOMPARE(doc.lineLength(0), 0);
}

void BugTest::backspaceSurrogates()
{
    // set up document and view and open test file
    KTextEditor::DocumentPrivate doc;
    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    const QUrl url = QUrl::fromLocalFile(QLatin1String(TEST_DATA_DIR "bug205447.txt"));
    doc.setEncoding(QStringLiteral("UTF-8"));
    QVERIFY(doc.openUrl(url));

    // test backspace
    // get UTF-32 representation of original line (before any backspaces)
    QList<uint> line = doc.line(0).toUcs4();
    QVERIFY(line.size() == 23);
    // backspace from end of line
    view->setCursorPosition(Cursor(0, doc.line(0).size()));
    QVERIFY(DocumentCursor(&doc, view->cursorPosition()).isValidTextPosition());
    for (int i = 0; i < line.size(); i++) {
        // get the current line, after `i` delete presses, and convert it to utf32
        // then ensure it's the expected substring of the original line
        QList<uint> current = doc.line(0).toUcs4();
        QCOMPARE(current, line.mid(0, line.size() - i));

        // press the backspace key and verify that the new text position isn't invalid
        view->backspace();
        QVERIFY(DocumentCursor(&doc, view->cursorPosition()).isValidTextPosition());
    }
    QCOMPARE(doc.lineLength(0), 0);
}

#include "moc_bug205447.cpp"
