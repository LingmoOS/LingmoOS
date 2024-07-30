/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QClipboard>
#include <QTest>

#include <ktextedit.h>

class KTextEdit_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testPaste();
    // These tests are probably invalid due to using invalid html.
    //     void testImportWithHorizontalTraversal();
    //     void testImportWithVerticalTraversal();
    //     void testBrInsideParagraphThroughTextEdit();
};

void KTextEdit_UnitTest::testPaste()
{
    const QString origText = QApplication::clipboard()->text();
    const QString pastedText = QStringLiteral("Test paste from ktextedit_unittest");
    QApplication::clipboard()->setText(pastedText);
    KTextEdit w;
    w.setPlainText(QStringLiteral("Hello world"));
    w.selectAll();
    QTest::keyClick(&w, Qt::Key_V, Qt::ControlModifier);
    QCOMPARE(w.toPlainText(), pastedText);
    QApplication::clipboard()->setText(origText);
}

// void KTextEdit_UnitTest::testImportWithVerticalTraversal()
// {
//     QTextEdit *te = new QTextEdit();
//
//     te->setHtml("<p>Foo</p><br /><br /><br /><p>Bar</p>");
//
//     QTextCursor cursor = te->textCursor();
//     cursor.movePosition(QTextCursor::Start);
//     QVERIFY(cursor.block().text() == QString( "Foo" ));
//     cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 4);
//
//     // Cursor is at the beginning of the block.
//     QVERIFY(cursor.block().position() == cursor.position());
//     QVERIFY(cursor.block().text() == QString( "Bar" ));
// }
//
// void KTextEdit_UnitTest::testImportWithHorizontalTraversal()
// {
//     QTextEdit *te = new QTextEdit();
//
//     te->setHtml("<p>Foo</p><br /><p>Bar</p>");
//
//     // br elements should be represented just like empty paragraphs.
//
//     QTextCursor cursor = te->textCursor();
//     cursor.movePosition(QTextCursor::Start);
//     QVERIFY(cursor.block().text() == QString( "Foo" ));
//     cursor.movePosition(QTextCursor::EndOfBlock);
//     cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 2);
//
//     // Cursor is at the beginning of the block.
//     QVERIFY(cursor.block().position() == cursor.position());
//     QVERIFY(cursor.block().text() == QString( "Bar" ));
// }
//
// void KTextEdit_UnitTest::testBrInsideParagraphThroughTextEdit()
// {
//     QSKIP("This is worked around during export");
//     QTextEdit *te = new QTextEdit();
//
//     te->setHtml("<p>Foo<br />Bar</p>");
//
//     // br elements inside paragraphs should be a single linebreak.
//
//     QTextCursor cursor = te->textCursor();
//     cursor.movePosition(QTextCursor::Start);
//
//     // This doesn't work, because Qt puts Foo and Bar in the same block, separated by a QChar::LineSeparator
//
//     QVERIFY(cursor.block().text() == QString( "Foo" ));
//     cursor.movePosition(QTextCursor::EndOfBlock);
//     cursor.movePosition(QTextCursor::Right);
//
//     // Cursor is at the beginning of the block.
//     QVERIFY(cursor.block().position() == cursor.position());
//     QVERIFY(cursor.block().text() == QString( "Bar" ));
//
// }

QTEST_MAIN(KTextEdit_UnitTest)

#include "ktextedit_unittest.moc"
