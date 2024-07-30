/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "multicursortest.h"

#include <QClipboard>
#include <katebuffer.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <kateundomanager.h>
#include <kateview.h>
#include <kateviewinternal.h>
#include <ktexteditor/message.h>
#include <ktexteditor/movingcursor.h>

#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QTest>

using namespace KTextEditor;

QTEST_MAIN(MulticursorTest)

struct DocAndView {
    DocumentPrivate *doc;
    ViewPrivate *view;

    ~DocAndView()
    {
        delete view;
        delete doc;
    }
};

DocAndView createDocAndView(const QString &text, int line, int column)
{
    auto doc = new DocumentPrivate();
    doc->setText(text);
    auto view = new ViewPrivate(doc, nullptr);
    view->setCursorPosition({line, column});
    return {.doc=doc, .view=view};
}

template<typename Cont>
bool isSorted(const Cont &c)
{
    return std::is_sorted(c.begin(), c.end());
}

MulticursorTest::MulticursorTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

MulticursorTest::~MulticursorTest()
{
}

static QWidget *findViewInternal(KTextEditor::ViewPrivate *view)
{
    for (QObject *child : view->children()) {
        if (child->metaObject()->className() == QByteArrayLiteral("KateViewInternal")) {
            return qobject_cast<QWidget *>(child);
        }
    }
    return nullptr;
}

static void clickAtPosition(ViewPrivate *view, QWidget *internalView, Cursor pos, Qt::KeyboardModifiers m)
{
    QPoint p = view->cursorToCoordinate(pos);
    QVERIFY(p.x() >= 0 && p.y() >= 0);
    auto me = QMouseEvent(QEvent::MouseButtonPress, p, internalView->mapToGlobal(p), Qt::LeftButton, Qt::LeftButton, m);
    QCoreApplication::sendEvent(internalView, &me);
}

void MulticursorTest::testKillline()
{
    KTextEditor::DocumentPrivate doc;
    doc.insertLines(0, {QStringLiteral("foo"), QStringLiteral("bar"), QStringLiteral("baz")});
    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    view->setCursorPositionInternal(KTextEditor::Cursor(0, 0));
    view->addSecondaryCursor(KTextEditor::Cursor(1, 0));
    view->addSecondaryCursor(KTextEditor::Cursor(2, 0));
    QVERIFY(isSorted(view->secondaryCursors()));

    view->killLine();

    QCOMPARE(doc.text(), QString());
}

void MulticursorTest::insertRemoveText()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\n"), 0, 0);
    QObject *internalView = findViewInternal(view);
    QVERIFY(internalView);

    { // Same line
        view->addSecondaryCursor({0, 1});
        view->addSecondaryCursor({0, 2});
        view->addSecondaryCursor({0, 3});
        QVERIFY(isSorted(view->secondaryCursors()));
        QCOMPARE(view->secondaryCursors().size(), 3);
        QKeyEvent ke(QKeyEvent::KeyPress, Qt::Key_L, Qt::NoModifier, QStringLiteral("L"));
        QCoreApplication::sendEvent(internalView, &ke);

        QCOMPARE(doc->line(0), QStringLiteral("LfLoLoL"));

        // Removal
        view->backspace();
        QCOMPARE(doc->line(0), QStringLiteral("foo"));

        view->clearSecondaryCursors();
    }

    { // Different lines
        view->setCursorPosition(Cursor(0, 0));
        view->addSecondaryCursor({1, 0});
        view->addSecondaryCursor({2, 0});
        QKeyEvent ke(QKeyEvent::KeyPress, Qt::Key_L, Qt::NoModifier, QStringLiteral("L"));
        QCoreApplication::sendEvent(internalView, &ke);

        QCOMPARE(doc->line(0), QStringLiteral("Lfoo"));
        QCOMPARE(doc->line(1), QStringLiteral("Lbar"));
        QCOMPARE(doc->line(2), QStringLiteral("Lfoo"));

        view->backspace();
        QVERIFY(isSorted(view->secondaryCursors()));

        QCOMPARE(doc->line(0), QStringLiteral("foo"));
        QCOMPARE(doc->line(1), QStringLiteral("bar"));
        QCOMPARE(doc->line(2), QStringLiteral("foo"));

        QVERIFY(isSorted(view->secondaryCursors()));
        view->clearSecondaryCursors();
    }

    // Three empty lines
    doc->setText(QStringLiteral("\n\n\n"));
    view->setCursorPosition({0, 0});
    view->addSecondaryCursor({1, 0});
    view->addSecondaryCursor({2, 0});
    QVERIFY(isSorted(view->secondaryCursors()));

    // cursors should merge
    view->backspace();
    QCOMPARE(view->secondaryCursors().size(), 0);
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
}

void MulticursorTest::backspace()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nbaz"), 0, 3);
    QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> cursors;

    {
        // Mixed cursors, one doesn't have selection
        // the two below have selection
        cursors.append({.pos=Cursor(1, 3), .range=Range(1, 0, 1, 3)});
        cursors.append({.pos=Cursor(2, 3), .range=Range(2, 0, 2, 3)});
        view->addSecondaryCursorsWithSelection(cursors);

        // Pressing backspace should only remove selected text
        view->backspace();
        QVERIFY(isSorted(view->secondaryCursors()));
        QCOMPARE(doc->text(), QStringLiteral("foo\n\n"));

        // Pressing backspace again
        view->backspace();
        QVERIFY(view->secondaryCursors().empty());
        QCOMPARE(doc->text(), QStringLiteral("fo"));
    }

    {
        // No selection
        doc->setText(QStringLiteral("foo\nbar\nbaz"));
        view->setCursors({Cursor(0, 3), Cursor(1, 3), Cursor(2, 3)});
        view->backspace();
        QCOMPARE(doc->text(), QStringLiteral("fo\nba\nba"));
        view->backspace();
        view->backspace();
        QCOMPARE(doc->text(), QStringLiteral("\n\n"));
        QCOMPARE(view->cursors().size(), 3);
    }
}

void MulticursorTest::keyDelete()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nbaz"), 0, 0);
    QList<KTextEditor::ViewPrivate::PlainSecondaryCursor> cursors;

    {
        // Mixed cursors, one doesn't have selection
        // the two below have selection
        cursors.append({.pos=Cursor(1, 0), .range=Range(1, 0, 1, 3)});
        cursors.append({.pos=Cursor(2, 0), .range=Range(2, 0, 2, 3)});
        view->addSecondaryCursorsWithSelection(cursors);

        // Pressing del should only remove selected text
        view->keyDelete();
        QVERIFY(isSorted(view->secondaryCursors()));
        QCOMPARE(doc->text(), QStringLiteral("foo\n\n"));

        // Pressing del again
        view->keyDelete();
        QCOMPARE(view->secondaryCursors().size(), 1);
        QCOMPARE(doc->text(), QStringLiteral("oo\n"));
    }

    {
        // No selection
        doc->setText(QStringLiteral("foo\nbar\nbaz"));
        view->setCursors({Cursor(0, 0), Cursor(1, 0), Cursor(2, 0)});
        view->keyDelete();
        QCOMPARE(doc->text(), QStringLiteral("oo\nar\naz"));
        view->keyDelete();
        view->keyDelete();
        QCOMPARE(doc->text(), QStringLiteral("\n\n"));
        QCOMPARE(view->cursors().size(), 3);
    }
}

void MulticursorTest::testUndoRedo()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo"), 0, 3);

    // single cursor backspace
    view->backspace();
    QCOMPARE(doc->text(), QStringLiteral("fo\nfoo"));
    doc->undoManager()->undoSafePoint();

    // backspace with 2 cursors
    view->setCursors({view->cursorPosition(), Cursor{1, 3}});
    view->backspace();
    QCOMPARE(doc->text(), QStringLiteral("f\nfo"));

    view->doc()->undo();
    QCOMPARE(doc->text(), QStringLiteral("fo\nfoo"));
    QCOMPARE(view->secondaryCursors().size(), 1);
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 3));

    // Another undo, multicursor should be gone
    view->doc()->undo();
    QCOMPARE(doc->text(), QStringLiteral("foo\nfoo"));
    QCOMPARE(view->secondaryCursors().size(), 0);

    // One redo
    view->doc()->redo();
    QCOMPARE(doc->text(), QStringLiteral("fo\nfoo"));

    // Second redo, multicursor should be back
    view->doc()->redo();
    QCOMPARE(doc->text(), QStringLiteral("f\nfo"));
    QCOMPARE(view->secondaryCursors().size(), 1);
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 2));
}

void MulticursorTest::testUndoRedoWithSelection()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo"), 0, 3);
    view->setCursors({Cursor(0, 3), Cursor(1, 3)});

    // select a word & remove it
    view->shiftWordLeft();
    view->backspace();

    QCOMPARE(doc->text(), QStringLiteral("\n"));
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(view->secondaryCursors().size(), 1);
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 0));

    view->doc()->undo();

    QCOMPARE(doc->text(), QStringLiteral("foo\nfoo"));
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(view->secondaryCursors().size(), 1);
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 0));
    QCOMPARE(*view->secondaryCursors().at(0).range, Range(1, 0, 1, 3));
    QCOMPARE(view->secondaryCursors().at(0).anchor, Cursor(1, 3));
}

void MulticursorTest::keyReturnIndentTest()
{
    auto [doc, view] = createDocAndView(QStringLiteral("\n\n"), 0, 0);
    QCOMPARE(doc->lines(), 3);
    doc->setMode(QStringLiteral("C++"));
    view->config()->setValue(KateViewConfig::AutoBrackets, true);

    view->addSecondaryCursorDown();
    view->addSecondaryCursorDown();
    QCOMPARE(view->secondaryCursors().size(), 2);
    QVERIFY(isSorted(view->secondaryCursors()));

    doc->typeChars(view, QStringLiteral("{"));
    QCOMPARE(doc->text(), QStringLiteral("{}\n{}\n{}"));
    QCOMPARE(view->secondaryCursors().size(), 2);
    QVERIFY(isSorted(view->secondaryCursors()));

    view->keyReturn();
    QCOMPARE(doc->text(), QStringLiteral("{\n    \n}\n{\n    \n}\n{\n    \n}"));
}

void MulticursorTest::wrapSelectionWithCharsTest()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo\nfoo"), 0, 3);

    view->addSecondaryCursorDown();
    view->addSecondaryCursorDown();
    QCOMPARE(view->secondaryCursors().size(), 2);

    view->shiftWordLeft();
    doc->typeChars(view, QStringLiteral("{"));
    QCOMPARE(doc->text(), QStringLiteral("{foo}\n{foo}\n{foo}"));
}

void MulticursorTest::insertAutoBrackets()
{
    auto [doc, view] = createDocAndView(QStringLiteral("hello\nhello"), 0, 0);
    QCOMPARE(doc->lines(), 2);
    doc->setMode(QStringLiteral("C++"));
    view->config()->setValue(KateViewConfig::AutoBrackets, true);
    view->setSecondaryCursors({Cursor(0, 0), Cursor(1, 0)});
    QCOMPARE(view->cursors().size(), 2);

    doc->typeChars(view, QStringLiteral("("));
    QCOMPARE(doc->text(), QStringLiteral("(hello\n(hello"));
}

void MulticursorTest::testCreateMultiCursor()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\n"), 0, 0);

    auto *internalView = findViewInternal(view);
    QVERIFY(internalView);

    // Alt + click should add a cursor
    auto primary = view->cursorPosition();
    clickAtPosition(view, internalView, {1, 0}, Qt::AltModifier);
    QCOMPARE(view->secondaryCursors().size(), 1);
    // primary cursor moved to the position which is clicked
    QCOMPARE(view->cursorPosition(), Cursor(1, 0));
    // secondary was created where primary cursor was
    QCOMPARE(view->secondaryCursors().at(0).cursor(), primary);

    // Alt + click at the same point should remove the cursor
    clickAtPosition(view, internalView, {1, 0}, Qt::AltModifier);
    QCOMPARE(view->secondaryCursors().size(), 0);

    // Create two cursors using alt+click
    clickAtPosition(view, internalView, {1, 0}, Qt::AltModifier);
    clickAtPosition(view, internalView, {1, 1}, Qt::AltModifier);
    QCOMPARE(view->secondaryCursors().size(), 2);
    QVERIFY(isSorted(view->secondaryCursors()));

    // now simple click => should remove all secondary cursors
    clickAtPosition(view, internalView, {1, 0}, Qt::NoModifier);
    QCOMPARE(view->secondaryCursors().size(), 0);
    QCOMPARE(view->cursorPosition(), Cursor(1, 0));
}

void MulticursorTest::testCreateMultiCursorFromSelection()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo"), 2, 3);
    view->setSelection(KTextEditor::Range(0, 0, 2, 3));
    // move primary cursor to beginning of line, so we can check whether it is moved to end of line
    view->setCursorPosition({view->cursorPosition().line(), 0});
    view->createMultiCursorsFromSelection();
    QVERIFY(isSorted(view->secondaryCursors()));
    QCOMPARE(view->cursorPosition().column(), 3);

    const auto &cursors = view->secondaryCursors();
    QCOMPARE(cursors.size(), doc->lines() - 1); // 1 cursor is primary, not included

    int i = 0;
    for (const auto &c : cursors) {
        QCOMPARE(c.cursor(), KTextEditor::Cursor(i, 3));
        i++;
    }
}

void MulticursorTest::testMulticursorToggling()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo"), 0, 0);
    view->setSelections({Range(0, 0, 0, 3), Range(1, 0, 1, 3)});
    QCOMPARE(view->selectionRanges().size(), 2);

    // Trying to add a cursor in one of the selection region
    // will remove it
    view->addSecondaryCursor(Cursor(0, 2));
    QCOMPARE(view->selectionRanges().size(), 1);

    // Trying to toggle last remaining selection will do nothing
    view->addSecondaryCursor(Cursor(1, 2));
    QCOMPARE(view->selectionRanges().size(), 1);
}

void MulticursorTest::moveCharTest()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\n"), 0, 0);
    view->setCursors({Cursor(0, 0), Cursor(1, 0)});

    // Simple left right
    view->cursorRight();
    QCOMPARE(view->cursorPosition(), Cursor(0, 1));
    QCOMPARE(view->secondaryCursors().at(0).cursor(), Cursor(1, 1));

    view->cursorLeft();
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(view->secondaryCursors().at(0).cursor(), Cursor(1, 0));

    // Shift pressed
    view->shiftCursorRight();
    QCOMPARE(view->cursorPosition(), Cursor(0, 1));
    QCOMPARE(view->secondaryCursors().at(0).cursor(), Cursor(1, 1));
    QCOMPARE(view->secondaryCursors().at(0).range->toRange(), Range(1, 0, 1, 1));

    view->shiftCursorLeft();
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(view->secondaryCursors().at(0).cursor(), Cursor(1, 0));
    QCOMPARE(view->secondaryCursors().at(0).range->toRange(), Range(1, 0, 1, 0));

    view->clearSecondaryCursors();

    // Selection merge test => merge into primary cursor
    view->setCursors({Cursor(0, 2), Cursor(0, 3)}); // fo|o|
    // Two shift left should result in one cursor
    view->shiftCursorLeft();
    view->shiftCursorLeft();
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(view->secondaryCursors().size(), 0);
    QCOMPARE(view->selectionRange(), Range(0, 0, 0, 3));

    view->clearSelection();

    // Selection merge test => merge primary into multi => multi becomes primary
    view->setCursorPosition({0, 0}); // fo|o
    view->addSecondaryCursor({0, 1}); // foo|
    // Two shift left should result in one cursor
    view->shiftCursorRight();
    view->shiftCursorRight();
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(view->secondaryCursors().size(), 0);
    QCOMPARE(view->selectionRange(), Range(0, 0, 0, 3));
}

void MulticursorTest::moveCharInFirstOrLastLineTest()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo"), 0, 0);
    view->addSecondaryCursor({0, 1});
    // |f|oo

    view->cursorLeft();
    QCOMPARE(view->secondaryCursors().size(), 0);
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));

    view->setCursorPosition({0, 2});
    view->addSecondaryCursor({0, 3});
    view->cursorRight();
    QCOMPARE(view->secondaryCursors().size(), 0);
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
}

void MulticursorTest::moveWordTest()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\n"), 0, 0);
    view->setCursors({Cursor(0, 0), Cursor(1, 0)});

    // Simple left right
    view->wordRight();
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 3));

    view->wordLeft();
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 0));

    // Shift pressed
    view->shiftWordRight();
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 3));
    QCOMPARE(view->secondaryCursors().at(0).range->toRange(), Range(1, 0, 1, 3));

    view->shiftWordLeft();
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 0));
    QCOMPARE(view->secondaryCursors().at(0).range->toRange(), Range(1, 0, 1, 0));

    view->clearSecondaryCursors();

    // Two cursors in same word, => word movement should merge them (sel)
    view->setCursorPosition({0, 0}); // |foo
    view->addSecondaryCursor({0, 1}); // f|oo
    view->shiftWordRight(); // foo|
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(view->secondaryCursors().size(), 0);
    QCOMPARE(view->selectionRange(), Range(0, 0, 0, 3));

    // Three cursors in same word, => word movement should merge them (no sel)
    view->setCursorPosition({0, 3}); // foo|
    view->addSecondaryCursor({0, 2}); // fo|o
    view->addSecondaryCursor({0, 1}); // f|oo
    view->wordLeft(); // foo|
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(view->secondaryCursors().size(), 0);
}

void MulticursorTest::homeEndKeyTest()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\n"), 0, 0);
    view->setCursors({Cursor(0, 0), Cursor(0, 1)});

    // Two cursor in same line => home should merge them
    view->home();
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(view->secondaryCursors().size(), 0);

    // Two cursor in same line => end should merge them
    view->setCursors({Cursor(0, 0), Cursor(0, 1)});
    view->end();
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(view->secondaryCursors().size(), 0);

    view->setCursors({Cursor(0, 3), Cursor(1, 0)});
    view->end();
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 3));

    view->clearSecondaryCursors();

    view->setCursors({Cursor(0, 3), Cursor(1, 3)});
    view->home();
    QCOMPARE(view->cursorPosition(), Cursor(0, 0));
    QCOMPARE(*view->secondaryCursors().at(0).pos, Cursor(1, 0));
}

void MulticursorTest::moveUpDown()
{
    /** TEST UP **/
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo"), 0, 0);

    view->setSecondaryCursors({Cursor(1, 0), Cursor(2, 0)});
    QCOMPARE(view->secondaryCursors().size(), 2);
    QVERIFY(isSorted(view->secondaryCursors()));

    view->up();
    QCOMPARE(view->secondaryCursors().size(), 1);

    view->up();
    QCOMPARE(view->secondaryCursors().size(), 0);

    /** TEST DOWN **/

    view->setSecondaryCursors({Cursor(1, 0), Cursor(2, 0)});
    QCOMPARE(view->secondaryCursors().size(), 2);

    view->down();
    QCOMPARE(view->secondaryCursors().size(), 2); // last cursor moves to end of line
    QCOMPARE(*view->secondaryCursors().at(1).pos, Cursor(2, 3));
    QVERIFY(isSorted(view->secondaryCursors()));

    view->down();
    QCOMPARE(view->secondaryCursors().size(), 1);

    view->down();
    QCOMPARE(view->secondaryCursors().size(), 0);
    QCOMPARE(view->cursorPosition(), Cursor(2, 3));
}

void MulticursorTest::testSelectionMerge()
{
    // 8 lines
    {
        // Left movement, cursor at top
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo\nfoo\nfoo\nfoo\nfoo\nfoo"), 0, 3);

        view->selectAll();
        view->createMultiCursorsFromSelection();
        QVERIFY(isSorted(view->secondaryCursors()));

        QCOMPARE(view->secondaryCursors().size(), 6);

        view->shiftWordLeft();
        QVERIFY(isSorted(view->secondaryCursors()));
        view->shiftWordLeft();
        QVERIFY(isSorted(view->secondaryCursors()));
        view->shiftWordLeft();

        QCOMPARE(view->secondaryCursors().size(), 0);
        QCOMPARE(view->cursorPosition(), Cursor(0, 0));
        QCOMPARE(view->selectionRange(), Range(0, 0, 6, 3));
    }

    {
        // Left movement, cursor at bottom
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo\nfoo\nfoo\nfoo\nfoo\nfoo"), 6, 3);

        view->selectAll();
        view->createMultiCursorsFromSelection();
        QVERIFY(isSorted(view->secondaryCursors()));

        QCOMPARE(view->secondaryCursors().size(), 6);
        QCOMPARE(view->cursorPosition(), Cursor(6, 3));

        view->shiftWordLeft();
        QVERIFY(isSorted(view->secondaryCursors()));
        view->shiftWordLeft();
        QVERIFY(isSorted(view->secondaryCursors()));
        view->shiftWordLeft();

        QCOMPARE(view->secondaryCursors().size(), 0);
        QCOMPARE(view->cursorPosition(), Cursor(0, 0));
        QCOMPARE(view->selectionRange(), Range(0, 0, 6, 3));
    }

    {
        // Left word movement, cursor in the middle
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo\nfoo\nfoo\nfoo\nfoo\nfoo"), 3, 3);

        for (int i = 0; i < 10; ++i) {
            view->addSecondaryCursorUp();
            view->addSecondaryCursorDown();
        }

        QCOMPARE(view->secondaryCursors().size(), 6);

        view->shiftWordLeft();
        view->shiftWordLeft();
        view->shiftWordLeft();

        QCOMPARE(view->secondaryCursors().size(), 0);
        QCOMPARE(view->cursorPosition(), Cursor(0, 0));
        QCOMPARE(view->selectionRange(), Range(0, 0, 6, 3));
    }

    {
        // Left word + char movement, cursor in the middle
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo\nfoo\nfoo\nfoo\nfoo\nfoo"), 3, 3);

        view->addSecondaryCursorUp();
        view->addSecondaryCursorUp();
        view->addSecondaryCursorDown();
        view->addSecondaryCursorDown();
        QVERIFY(isSorted(view->secondaryCursors()));

        QCOMPARE(view->secondaryCursors().size(), 4);

        view->shiftWordLeft();
        view->shiftCursorLeft();
        view->shiftCursorLeft();
        view->shiftCursorLeft();

        QCOMPARE(view->secondaryCursors().size(), 0);
        QCOMPARE(view->cursorPosition(), Cursor(0, 1));
        QCOMPARE(view->selectionRange(), Range(0, 1, 5, 3));
    }

    {
        // Right movement, cursor at bottom line
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo\nfoo\nfoo\nfoo\nfoo\nfoo"), 6, 0);

        for (int i = 0; i < 10; ++i) {
            view->addSecondaryCursorUp();
        }

        QCOMPARE(view->secondaryCursors().size(), 6);

        view->shiftWordRight();
        QVERIFY(isSorted(view->secondaryCursors()));
        view->shiftWordRight();
        QVERIFY(isSorted(view->secondaryCursors()));
        view->shiftWordRight();

        QCOMPARE(view->secondaryCursors().size(), 0);
        QCOMPARE(view->cursorPosition(), Cursor(6, 3));
        QCOMPARE(view->selectionRange(), Range(0, 0, 6, 3));
    }

    {
        // Right movement, cursor at top line
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo\nfoo\nfoo\nfoo\nfoo\nfoo"), 0, 0);

        for (int i = 0; i < 10; ++i) {
            view->addSecondaryCursorDown();
        }

        QCOMPARE(view->secondaryCursors().size(), 6);

        view->shiftWordRight();
        view->shiftWordRight();
        view->shiftWordRight();

        QCOMPARE(view->secondaryCursors().size(), 0);
        QCOMPARE(view->cursorPosition(), Cursor(6, 3));
        QCOMPARE(view->selectionRange(), Range(0, 0, 6, 3));
    }

    {
        // Right word + char movement, cursor in the middle
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nfoo\nfoo\nfoo\nfoo\nfoo\nfoo"), 3, 0);

        view->addSecondaryCursorUp();
        view->addSecondaryCursorUp();
        view->addSecondaryCursorDown();
        view->addSecondaryCursorDown();
        QVERIFY(isSorted(view->secondaryCursors()));

        QCOMPARE(view->secondaryCursors().size(), 4);

        view->shiftWordRight();
        QVERIFY(isSorted(view->secondaryCursors()));
        view->shiftCursorRight();
        QVERIFY(isSorted(view->secondaryCursors()));
        view->shiftCursorRight();
        QVERIFY(isSorted(view->secondaryCursors()));
        view->shiftCursorRight();

        QCOMPARE(view->secondaryCursors().size(), 0);
        QCOMPARE(view->cursorPosition(), Cursor(6, 2));
        QCOMPARE(view->selectionRange(), Range(1, 0, 6, 2));
    }
}

void MulticursorTest::findNextOccurenceTest()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\nfoo"), 0, 0);

    // No selection
    view->findNextOccurunceAndSelect();
    QCOMPARE(view->selectionRange(), Range(0, 0, 0, 3));
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(view->secondaryCursors().size(), 0);

    view->clearSelection();
    // with selection
    view->setSelection(Range(0, 0, 0, 3));
    view->findNextOccurunceAndSelect();
    QCOMPARE(view->secondaryCursors().size(), 1);
    QCOMPARE(view->secondaryCursors().at(0).cursor(), Cursor(0, 3));
    QCOMPARE(view->secondaryCursors().at(0).range->toRange(), Range(0, 0, 0, 3));
    // primary cursor has the last selection
    QCOMPARE(view->cursorPosition(), Cursor(2, 3));
    QCOMPARE(view->selectionRange(), Range(2, 0, 2, 3));

    // find another
    view->findNextOccurunceAndSelect();
    QCOMPARE(view->secondaryCursors().size(), 2);
    QVERIFY(isSorted(view->secondaryCursors()));
    QCOMPARE(view->secondaryCursors().at(0).cursor(), Cursor(0, 3));
    QCOMPARE(view->secondaryCursors().at(0).range->toRange(), Range(0, 0, 0, 3));
    QCOMPARE(view->secondaryCursors().at(1).cursor(), Cursor(2, 3));
    QCOMPARE(view->secondaryCursors().at(1).range->toRange(), Range(2, 0, 2, 3));
    // primary cursor has the last selection
    QCOMPARE(view->cursorPosition(), Cursor(3, 3));
    QCOMPARE(view->selectionRange(), Range(3, 0, 3, 3));

    // Try to find another, there is none so nothing should change
    // except that the primary cursor position is moved to newest found
    view->findNextOccurunceAndSelect();
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(view->selectionRange(), Range(0, 0, 0, 3));
    QVERIFY(isSorted(view->secondaryCursors()));
}

void MulticursorTest::findAllOccurenceTest()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\nfoo"), 0, 0);

    // No selection
    view->findAllOccuruncesAndSelect();
    QCOMPARE(view->selectionRange(), Range(0, 0, 0, 3));
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(view->secondaryCursors().size(), 2);
    // first
    QCOMPARE(view->secondaryCursors().at(0).cursor(), Cursor(2, 3));
    QCOMPARE(view->secondaryCursors().at(0).range->toRange(), Range(2, 0, 2, 3));
    // second
    QCOMPARE(view->secondaryCursors().at(1).cursor(), Cursor(3, 3));
    QCOMPARE(view->secondaryCursors().at(1).range->toRange(), Range(3, 0, 3, 3));

    // Try to find another, there is none so nothing should change
    view->findAllOccuruncesAndSelect();
    QCOMPARE(view->cursorPosition(), Cursor(0, 3));
    QCOMPARE(view->selectionRange(), Range(0, 0, 0, 3));
}

void MulticursorTest::testMultiCopyPaste()
{
    // Create two docs, copy from one to the other
    {
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\nfoo"), 0, 0);
        view->addSecondaryCursor({1, 0});
        view->addSecondaryCursor({2, 0});
        view->addSecondaryCursor({3, 0});
        view->shiftWordRight();
        view->copy();
    }

    // Same number of cursors when pasting => each line gets pasted into matching cursor postion
    {
        KTextEditor::DocumentPrivate doc;
        doc.setText(QStringLiteral("\n\n\n\n"));
        KTextEditor::ViewPrivate *v = new KTextEditor::ViewPrivate(&doc, nullptr);
        v->setCursorPosition({0, 0});
        v->addSecondaryCursor({1, 0});
        v->addSecondaryCursor({2, 0});
        v->addSecondaryCursor({3, 0});
        v->paste();
        QCOMPARE(doc.text(), QStringLiteral("foo\nbar\nfoo\nfoo\n"));

        // Different number of cursors
        v->clear();
        QVERIFY(doc.clear());
        doc.setText(QStringLiteral("\n\n"));
        v->setCursorPosition({0, 0});
        v->addSecondaryCursor({1, 0});
        QCOMPARE(v->secondaryCursors().size(), 1);

        v->paste();
        QString text = doc.text();
        QCOMPARE(text, QStringLiteral("foo\nbar\nfoo\nfoo\nfoo\nbar\nfoo\nfoo\n"));
    }
}

void MulticursorTest::testSelectionTextOrdering()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\nfoo"), 0, 0);
    view->addSecondaryCursor({1, 0});
    view->addSecondaryCursor({2, 0});
    view->shiftWordRight();
    QVERIFY(isSorted(view->secondaryCursors()));

    QString selText = view->selectionText();
    QCOMPARE(selText, QStringLiteral("foo\nbar\nfoo"));

    view->copy();
    QCOMPARE(QApplication::clipboard()->text(QClipboard::Clipboard), selText);
}

void MulticursorTest::testViewClear()
{
    auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar"), 0, 0);
    view->addSecondaryCursor({1, 0});
    QCOMPARE(view->secondaryCursors().size(), 1);
    view->clear();
    QCOMPARE(view->secondaryCursors().size(), 0);
}

void MulticursorTest::testSetGetCursors()
{
    using Cursors = QList<Cursor>;
    // Simple check
    {
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo\nfoo"), 0, 0);

        // primary included
        QCOMPARE(view->cursors(), Cursors{Cursor(0, 0)});

        const Cursors cursors = {{0, 1}, {1, 1}, {2, 1}, {3, 1}};
        view->setCursors(cursors);
        QCOMPARE(view->cursors(), cursors);
        QVERIFY(isSorted(view->cursors()));
        QCOMPARE(view->cursorPosition(), Cursor(0, 1));
        // We have no selection
        QVERIFY(!view->selection());
        QCOMPARE(view->selectionRanges(), QList<Range>{});
    }

    // Test duplicate cursor positions
    {
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar"), 0, 0);

        QCOMPARE(view->cursors(), Cursors{Cursor(0, 0)});
        const Cursors cursors = {{0, 0}, {1, 1}, {0, 0}, {1, 1}};
        view->setCursors(cursors);
        auto expectedCursors = Cursors{Cursor(0, 0), Cursor(1, 1)};
        QCOMPARE(view->cursors(), expectedCursors);
        QVERIFY(isSorted(view->cursors()));
        QCOMPARE(view->cursorPosition(), Cursor(0, 0));

        QVERIFY(view->cursors().size() > 1);
        view->setCursors({});
        QVERIFY(view->cursors().size() == 1);
    }
}

void MulticursorTest::testSetGetSelections()
{
    // Set cursors => press shift+right
    {
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar\nfoo"), 0, 0);
        QCOMPARE(view->cursors(), QList<Cursor>{Cursor(0, 0)});
        QList<Cursor> cursors = {{0, 1}, {1, 1}, {2, 1}};
        view->setCursors(cursors);
        QCOMPARE(view->cursors(), cursors);
        QVERIFY(isSorted(view->cursors()));
        view->shiftCursorRight();
        QVERIFY(view->selection());
        cursors = {{0, 2}, {1, 2}, {2, 2}};
        QCOMPARE(view->cursors(), cursors);
        QList<Range> selections = {Range(0, 1, 0, 2), Range(1, 1, 1, 2), Range(2, 1, 2, 2)};
        QCOMPARE(view->selectionRanges(), selections);
        QVERIFY(isSorted(view->selectionRanges()));
        QCOMPARE(view->selectionRange(), selections.front());
    }

    // Set cursors including an invalid position cursor
    // - primary already has selection
    // - try to get selection
    {
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar"), 0, 0);
        view->shiftWordRight();
        QVERIFY(view->selection());
        QCOMPARE(view->selectionRange(), Range(0, 0, 0, 3));

        QList<Cursor> cursors = {{0, 1}, {1, 1}, {2, 1}};
        view->setCursors(cursors);
        QVERIFY(!view->selection()); // selection is lost
        auto expectedCursors = QList<Cursor>{Cursor(0, 1), Cursor(1, 1)};
        QCOMPARE(view->cursors(), expectedCursors);
    }

    // Set selections
    {
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar"), 0, 0);

        QVERIFY(!view->selection());
        QList<Range> selections = {Range(0, 0, 0, 1), Range(1, 0, 1, 1)};
        view->setSelections(selections);
        QVERIFY(view->selection());
        QCOMPARE(view->selectionRanges(), selections);
    }

    // Set overlapping selections
    {
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar"), 0, 0);

        QVERIFY(!view->selection());
        QList<Range> selections = {Range(0, 0, 0, 3), Range(0, 1, 0, 2), Range(0, 0, 0, 1)};
        view->setSelections(selections);
        QVERIFY(view->selection());
        QList<Range> expectedSelections = {Range(0, 0, 0, 3)};
        QCOMPARE(view->selectionRanges(), expectedSelections);

        view->setSelections({});
        QVERIFY(!view->selection());
    }

    // Set selections with invalid range
    {
        auto [doc, view] = createDocAndView(QStringLiteral("foo\nbar"), 0, 0);

        QVERIFY(!view->selection());
        QList<Range> selections = {Range(0, 0, 0, 3), Range(1, 0, 1, 1), Range(2, 0, 2, 1)};
        view->setSelections(selections);
        QVERIFY(view->selection());
        QList<Range> expectedSelections = {Range(0, 0, 0, 3), Range(1, 0, 1, 1)};
        QCOMPARE(view->selectionRanges(), expectedSelections);
    }
}

#include "moc_multicursortest.cpp"

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
