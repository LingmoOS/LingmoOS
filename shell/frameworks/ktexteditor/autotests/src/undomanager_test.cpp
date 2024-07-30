/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2009-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "undomanager_test.h"

#include <katedocument.h>
#include <kateundomanager.h>
#include <kateview.h>

#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(UndoManagerTest)

using namespace KTextEditor;

UndoManagerTest::UndoManagerTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

void UndoManagerTest::testUndoRedoCount()
{
    KTextEditor::DocumentPrivate doc;

    // no undo/redo items at the beginning
    QCOMPARE(doc.undoCount(), 0u);
    QCOMPARE(doc.redoCount(), 0u);

    doc.insertText(Cursor(0, 0), QStringLiteral("a"));

    // create one insert-group
    QCOMPARE(doc.undoCount(), 1u);
    QCOMPARE(doc.redoCount(), 0u);

    doc.undo();

    // move insert-group to redo items
    QCOMPARE(doc.undoCount(), 0u);
    QCOMPARE(doc.redoCount(), 1u);

    doc.redo();

    // move insert-group back to undo items
    QCOMPARE(doc.undoCount(), 1u);
    QCOMPARE(doc.redoCount(), 0u);

    doc.insertText(Cursor(0, 1), QStringLiteral("b"));

    // merge "b" into insert-group
    QCOMPARE(doc.undoCount(), 1u);
    QCOMPARE(doc.redoCount(), 0u);

    doc.removeText(Range(0, 1, 0, 2));

    // create an additional remove-group
    QCOMPARE(doc.undoCount(), 2u);
    QCOMPARE(doc.redoCount(), 0u);

    doc.undo();

    // move remove-group to redo items
    QCOMPARE(doc.undoCount(), 1u);
    QCOMPARE(doc.redoCount(), 1u);

    doc.insertText(Cursor(0, 1), QStringLiteral("b"));

    // merge "b" into insert-group
    // and remove remove-group
    QCOMPARE(doc.undoCount(), 1u);
    QCOMPARE(doc.redoCount(), 0u);
}

void UndoManagerTest::testSafePoint()
{
    KTextEditor::DocumentPrivate doc;
    KateUndoManager *undoManager = doc.undoManager();

    doc.insertText(Cursor(0, 0), QStringLiteral("a"));

    // create one undo group
    QCOMPARE(doc.undoCount(), 1u);
    QCOMPARE(doc.redoCount(), 0u);

    undoManager->undoSafePoint();
    doc.insertText(Cursor(0, 1), QStringLiteral("b"));

    // create a second undo group (don't merge)
    QCOMPARE(doc.undoCount(), 2u);

    doc.undo();

    // move second undo group to redo items
    QCOMPARE(doc.undoCount(), 1u);
    QCOMPARE(doc.redoCount(), 1u);

    doc.insertText(Cursor(0, 1), QStringLiteral("b"));

    // create a second undo group again, (don't merge)
    QCOMPARE(doc.undoCount(), 2u);
    QCOMPARE(doc.redoCount(), 0u);

    doc.editStart();
    doc.insertText(Cursor(0, 2), QStringLiteral("c"));
    undoManager->undoSafePoint();
    doc.insertText(Cursor(0, 3), QStringLiteral("d"));
    doc.editEnd();

    // merge both edits into second undo group
    QCOMPARE(doc.undoCount(), 2u);
    QCOMPARE(doc.redoCount(), 0u);

    doc.insertText(Cursor(0, 4), QStringLiteral("e"));

    // create a third undo group (don't merge)
    QCOMPARE(doc.undoCount(), 3u);
    QCOMPARE(doc.redoCount(), 0u);
}

void UndoManagerTest::testCursorPosition()
{
    KTextEditor::DocumentPrivate doc;
    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));

    doc.setText(
        QStringLiteral("aaaa bbbb cccc\n"
                       "dddd  ffff"));
    view->setCursorPosition(KTextEditor::Cursor(1, 5));

    doc.typeChars(view, QStringLiteral("eeee"));

    // cursor position: "dddd eeee| ffff"
    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(1, 9));

    // undo once to remove "eeee", cursor position: "dddd | ffff"
    doc.undo();
    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(1, 5));

    // redo once to insert "eeee" again. cursor position: "dddd eeee| ffff"
    doc.redo();
    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(1, 9));

    delete view;
}

void UndoManagerTest::testSelectionUndo()
{
    KTextEditor::DocumentPrivate doc;
    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));

    doc.setText(
        QStringLiteral("aaaa bbbb cccc\n"
                       "dddd eeee ffff"));
    view->setCursorPosition(KTextEditor::Cursor(1, 9));
    KTextEditor::Range selectionRange(KTextEditor::Cursor(0, 5), KTextEditor::Cursor(1, 9));
    view->setSelection(selectionRange);

    doc.typeChars(view, QStringLiteral("eeee"));

    // cursor position: "aaaa eeee| ffff", no selection anymore
    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(0, 9));
    QCOMPARE(view->selection(), false);

    // undo to remove "eeee" and add selection and text again
    doc.undo();
    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(1, 9));
    QCOMPARE(view->selection(), true);
    QCOMPARE(view->selectionRange(), selectionRange);

    // redo to insert "eeee" again and remove selection
    // cursor position: "aaaa eeee| ffff", no selection anymore
    doc.redo();
    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(0, 9));
    QCOMPARE(view->selection(), false);

    delete view;
}

void UndoManagerTest::testUndoWordWrapBug301367()
{
    KTextEditor::DocumentPrivate doc;
    doc.setWordWrap(true);
    doc.setWordWrapAt(20);
    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));

    QString text = QString::fromLatin1(
        "1234 1234 1234 1234\n"
        "1234 1234 1234 1234");

    doc.setText(text);
    view->setCursorPosition(KTextEditor::Cursor(0, 0));

    doc.typeChars(view, QStringLiteral("           "));

    while (doc.undoCount() > 1) {
        doc.undo();
    }

    // test must be exactly the same as before
    QCOMPARE(doc.text(), text);

    while (doc.redoCount() > 1) {
        doc.redo();
    }

    while (doc.undoCount() > 1) {
        doc.undo();
    }

    // test must be exactly the same as before
    QCOMPARE(doc.text(), text);

    delete view;
}

void UndoManagerTest::testUndoIndentBug373009()
{
    KTextEditor::DocumentPrivate doc;
    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));

    doc.setMode(QStringLiteral("C"));

    QString text = QString::fromLatin1(
        "    while (whatever) printf (\"please fix indentation.\\n\");\n"
        "    return 0;");
    doc.setText(text);

    // position cursor right before return
    view->setCursorPosition(KTextEditor::Cursor(1, 4));

    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(1, 4));
    QCOMPARE(doc.characterAt(view->cursorPosition()), QLatin1Char('r'));

    view->keyReturn();

    QCOMPARE(doc.undoCount(), 2);

    // After indent we should be able to revert with
    // one undo operation
    doc.undo();
    QCOMPARE(doc.text(), text);

    delete view;
}

void UndoManagerTest::testUndoAfterPastingWrappingLine()
{
    KTextEditor::DocumentPrivate doc;
    std::unique_ptr<KTextEditor::ViewPrivate> view(static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr)));

    const QString originalText = QStringLiteral("First Line\nSecond Line Four Words");
    doc.setText(originalText);

    // put cursor in first line and copy the whole line
    view->setCursorPosition({0, 0});
    view->copy();

    // select 3rd word in line
    view->setSelection(KTextEditor::Range(1, 12, 1, 16));

    // paste the copied stuff
    view->paste();

    view->show();

    // undo the paste
    while (doc.undoCount() > 1) {
        doc.undo();
    }

    // text should be same as original now
    QCOMPARE(doc.text(), originalText);
}

#include "moc_undomanager_test.cpp"
