/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateview_test.h"
#include "moc_kateview_test.cpp"

#include <katebuffer.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <kateview.h>
#include <kateviewinternal.h>
#include <ktexteditor/message.h>
#include <ktexteditor/movingcursor.h>

#include <QScrollBar>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QtTestWidgets>

#define testNewRow() (QTest::newRow(QStringLiteral("line %1").arg(__LINE__).toLatin1().data()))

using namespace KTextEditor;

QTEST_MAIN(KateViewTest)

KateViewTest::KateViewTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

KateViewTest::~KateViewTest()
{
}

void KateViewTest::testCoordinatesToCursor()
{
    KTextEditor::DocumentPrivate doc(false, false);
    doc.setText(QStringLiteral("Hi World!\nHi\n"));

    KTextEditor::View *view1 = static_cast<KTextEditor::View *>(doc.createView(nullptr));
    view1->resize(400, 300);
    view1->show();

    QCOMPARE(view1->coordinatesToCursor(view1->cursorToCoordinate(KTextEditor::Cursor(0, 2))), KTextEditor::Cursor(0, 2));
    QCOMPARE(view1->coordinatesToCursor(view1->cursorToCoordinate(KTextEditor::Cursor(1, 1))), KTextEditor::Cursor(1, 1));
    // behind end of line should give an invalid cursor
    QCOMPARE(view1->coordinatesToCursor(view1->cursorToCoordinate(KTextEditor::Cursor(1, 5))), KTextEditor::Cursor::invalid());
    QCOMPARE(view1->cursorToCoordinate(KTextEditor::Cursor(3, 1)), QPoint(-1, -1));

    // check consistency between cursorToCoordinate(view->cursorPosition() and cursorPositionCoordinates()
    // random position
    view1->setCursorPosition(KTextEditor::Cursor(0, 3));
    QCOMPARE(view1->coordinatesToCursor(view1->cursorToCoordinate(view1->cursorPosition())), KTextEditor::Cursor(0, 3));
    QCOMPARE(view1->coordinatesToCursor(view1->cursorPositionCoordinates()), KTextEditor::Cursor(0, 3));
    // end of line
    view1->setCursorPosition(KTextEditor::Cursor(0, 9));
    QCOMPARE(view1->coordinatesToCursor(view1->cursorToCoordinate(KTextEditor::Cursor(0, 9))), KTextEditor::Cursor(0, 9));
    QCOMPARE(view1->coordinatesToCursor(view1->cursorPositionCoordinates()), KTextEditor::Cursor(0, 9));
    // empty line
    view1->setCursorPosition(KTextEditor::Cursor(2, 0));
    QCOMPARE(view1->coordinatesToCursor(view1->cursorToCoordinate(KTextEditor::Cursor(2, 0))), KTextEditor::Cursor(2, 0));
    QCOMPARE(view1->coordinatesToCursor(view1->cursorPositionCoordinates()), KTextEditor::Cursor(2, 0));

    // same test again, but with message widget on top visible
    KTextEditor::Message *message = new KTextEditor::Message(QStringLiteral("Jo World!"), KTextEditor::Message::Information);
    doc.postMessage(message);

    // wait 500ms until show animation is finished, so the message widget is visible
    QTest::qWait(500);

    QCOMPARE(view1->coordinatesToCursor(view1->cursorToCoordinate(KTextEditor::Cursor(0, 2))), KTextEditor::Cursor(0, 2));
    QCOMPARE(view1->coordinatesToCursor(view1->cursorToCoordinate(KTextEditor::Cursor(1, 1))), KTextEditor::Cursor(1, 1));
    // behind end of line should give an invalid cursor
    QCOMPARE(view1->coordinatesToCursor(view1->cursorToCoordinate(KTextEditor::Cursor(1, 5))), KTextEditor::Cursor::invalid());
    QCOMPARE(view1->cursorToCoordinate(KTextEditor::Cursor(3, 1)), QPoint(-1, -1));
}

void KateViewTest::testCursorToCoordinates()
{
    KTextEditor::DocumentPrivate doc(false, false);
    doc.setText(QStringLiteral("int a;"));

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    view->config()->setDynWordWrap(true);
    view->show();

    // don't crash, see https://bugs.kde.org/show_bug.cgi?id=337863
    view->cursorToCoordinate(Cursor(0, 0));
    view->cursorToCoordinate(Cursor(1, 0));
    view->cursorToCoordinate(Cursor(-1, 0));
}

void KateViewTest::testReloadMultipleViews()
{
    QTemporaryFile file(QStringLiteral("XXXXXX.cpp"));
    file.open();
    QTextStream stream(&file);
    const QString line = QStringLiteral("const char* foo = \"asdf\"\n");
    for (int i = 0; i < 200; ++i) {
        stream << line;
    }
    stream << Qt::flush;
    file.close();

    KTextEditor::DocumentPrivate doc;
    QVERIFY(doc.openUrl(QUrl::fromLocalFile(file.fileName())));
    QCOMPARE(doc.highlightingMode(), QStringLiteral("C++"));

    KTextEditor::ViewPrivate *view1 = new KTextEditor::ViewPrivate(&doc, nullptr);
    KTextEditor::ViewPrivate *view2 = new KTextEditor::ViewPrivate(&doc, nullptr);
    view1->show();
    view2->show();
    QCOMPARE(doc.views().count(), 2);

    QVERIFY(doc.documentReload());
}

void KateViewTest::testTabCursorOnReload()
{
    // testcase for https://bugs.kde.org/show_bug.cgi?id=258480
    QTemporaryFile file(QStringLiteral("XXXXXX.cpp"));
    file.open();
    QTextStream stream(&file);
    stream << "\tfoo\n";
    file.close();

    KTextEditor::DocumentPrivate doc;
    QVERIFY(doc.openUrl(QUrl::fromLocalFile(file.fileName())));

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    const KTextEditor::Cursor cursor(0, 4);
    view->setCursorPosition(cursor);
    QCOMPARE(view->cursorPosition(), cursor);
    QVERIFY(doc.documentReload());
    QCOMPARE(view->cursorPosition(), cursor);
}

void KateViewTest::testLowerCaseBlockSelection()
{
    // testcase for https://bugs.kde.org/show_bug.cgi?id=258480
    KTextEditor::DocumentPrivate doc;
    doc.setText(QStringLiteral("nY\nnYY\n"));

    KTextEditor::ViewPrivate *view1 = new KTextEditor::ViewPrivate(&doc, nullptr);
    view1->setBlockSelection(true);
    view1->setSelection(Range(0, 1, 1, 3));
    view1->lowercase();

    QCOMPARE(doc.text(), QStringLiteral("ny\nnyy\n"));
}

namespace
{
QWidget *findViewInternal(KTextEditor::View *view)
{
    for (QObject *child : view->children()) {
        if (child->metaObject()->className() == QByteArrayLiteral("KateViewInternal")) {
            return qobject_cast<QWidget *>(child);
        }
    }
    return nullptr;
}
}

void KateViewTest::testSelection()
{
    // see also: https://bugs.kde.org/show_bug.cgi?id=277422
    // wrong behavior before:
    // Open file with text
    // click at end of some line (A) and drag to right, i.e. without selecting anything
    // click somewhere else (B)
    // shift click to another place (C)
    // => expected: selection from B to C
    // => actual: selection from A to C

    QTemporaryFile file(QStringLiteral("XXXXXX.txt"));
    file.open();
    QTextStream stream(&file);
    stream << "A\n"
           << "B\n"
           << "C";
    stream << Qt::flush;
    file.close();

    KTextEditor::DocumentPrivate doc;
    QVERIFY(doc.openUrl(QUrl::fromLocalFile(file.fileName())));

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    view->resize(100, 200);
    view->show();

    auto *internalView = findViewInternal(view);
    QVERIFY(internalView);

    const QPoint afterA = view->cursorToCoordinate(Cursor(0, 1));
    const QPoint afterB = view->cursorToCoordinate(Cursor(1, 1));
    const QPoint afterC = view->cursorToCoordinate(Cursor(2, 1));

    // click after A
    auto me = QMouseEvent(QEvent::MouseButtonPress, afterA, internalView->mapToGlobal(afterA), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(internalView, &me);

    auto me1 = QMouseEvent(QEvent::MouseButtonRelease, afterA, internalView->mapToGlobal(afterA), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(internalView, &me1);
    QCOMPARE(view->cursorPosition(), Cursor(0, 1));

    // drag to right
    auto me2 = QMouseEvent(QEvent::MouseButtonPress, afterA, internalView->mapToGlobal(afterA), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(internalView, &me2);

    auto me3 = QMouseEvent(QEvent::MouseMove,
                           afterA + QPoint(50, 0),
                           internalView->mapToGlobal(afterA + QPoint(50, 0)),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    QCoreApplication::sendEvent(internalView, &me3);

    auto me4 = QMouseEvent(QEvent::MouseButtonRelease,
                           afterA + QPoint(50, 0),
                           internalView->mapToGlobal(afterA + QPoint(50, 0)),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    QCoreApplication::sendEvent(internalView, &me4);

    QCOMPARE(view->cursorPosition(), Cursor(0, 1));
    QVERIFY(!view->selection());

    // click after C
    auto me5 = QMouseEvent(QEvent::MouseButtonPress, afterC, internalView->mapToGlobal(afterC), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(internalView, &me5);

    auto me6 = QMouseEvent(QEvent::MouseButtonRelease, afterC, internalView->mapToGlobal(afterC), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(internalView, &me6);

    QCOMPARE(view->cursorPosition(), Cursor(2, 1));
    // shift+click after B
    auto me7 = QMouseEvent(QEvent::MouseButtonPress, afterB, internalView->mapToGlobal(afterB), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
    QCoreApplication::sendEvent(internalView, &me7);

    auto me8 = QMouseEvent(QEvent::MouseButtonRelease, afterB, internalView->mapToGlobal(afterB), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
    QCoreApplication::sendEvent(internalView, &me8);

    QCOMPARE(view->cursorPosition(), Cursor(1, 1));
    QCOMPARE(view->selectionRange(), Range(1, 1, 2, 1));
}

void KateViewTest::testDeselectByArrowKeys_data()
{
    QTest::addColumn<QString>("text");

    testNewRow() << QStringLiteral("foobarhaz");
    testNewRow() << QStringLiteral("كلسشمن يتبكسب"); // We all win, translates Google
}

void KateViewTest::testDeselectByArrowKeys()
{
    QFETCH(QString, text);

    KTextEditor::DocumentPrivate doc;
    doc.setText(text);
    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    KTextEditor::Cursor cur1(0, 3); // Start of bar: foo|barhaz
    KTextEditor::Cursor cur2(0, 6); //   End of bar: foobar|haz
    KTextEditor::Cursor curDelta(0, 1);
    Range range(cur1, cur2); // Select "bar"

    // RTL drives me nuts!
    KTextEditor::Cursor help;
    if (text.isRightToLeft()) {
        help = cur1;
        cur1 = cur2;
        cur2 = help;
    }

    view->setSelection(range);
    view->setCursorPositionInternal(cur1);
    view->cursorLeft();
    QCOMPARE(view->cursorPosition(), cur1); // Be at begin: foo|barhaz
    QCOMPARE(view->selection(), false);

    view->setSelection(range);
    view->setCursorPositionInternal(cur1);
    view->cursorRight();
    QCOMPARE(view->cursorPosition(), cur2); // Be at end: foobar|haz
    QCOMPARE(view->selection(), false);

    view->config()->setValue(KateViewConfig::PersistentSelection, true);

    view->setSelection(range);
    view->setCursorPositionInternal(cur1);
    view->cursorLeft();
    // RTL drives me nuts!
    help = text.isRightToLeft() ? (cur1 + curDelta) : (cur1 - curDelta);
    QCOMPARE(view->cursorPosition(), help); // Be one left: fo|obarhaz
    QCOMPARE(view->selection(), true);

    view->setSelection(range);
    view->setCursorPositionInternal(cur1);
    view->cursorRight();
    // RTL drives me nuts!
    help = text.isRightToLeft() ? (cur1 - curDelta) : (cur1 + curDelta);
    QCOMPARE(view->cursorPosition(), help); // Be one right: foob|arhaz
    QCOMPARE(view->selection(), true);
}

void KateViewTest::testKillline()
{
    KTextEditor::DocumentPrivate doc;
    doc.insertLines(0, {QStringLiteral("foo"), QStringLiteral("bar"), QStringLiteral("baz")});

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);

    view->setCursorPositionInternal(KTextEditor::Cursor(1, 2));
    view->killLine();

    QCOMPARE(doc.text(), QLatin1String("foo\nbaz\n"));

    doc.clear();
    QVERIFY(doc.isEmpty());

    doc.insertLines(0, {QStringLiteral("foo"), QStringLiteral("bar"), QStringLiteral("baz"), QStringLiteral("xxx")});

    view->setCursorPositionInternal(KTextEditor::Cursor(1, 2));
    view->shiftDown();
    view->killLine();

    QCOMPARE(doc.text(), QLatin1String("foo\nxxx\n"));
}

void KateViewTest::testKeyDeleteBlockSelection()
{
    KTextEditor::DocumentPrivate doc;
    doc.insertLines(0, {QStringLiteral("foo"), QStringLiteral("12345"), QStringLiteral("bar"), QStringLiteral("baz")});

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);

    view->setBlockSelection(true);
    view->setSelection(Range(0, 1, 2, 1));
    QCOMPARE(view->selectionRange(), Range(0, 1, 2, 1));

    view->keyDelete();
    QCOMPARE(doc.text(), QLatin1String("fo\n1345\nbr\nbaz\n"));
    view->keyDelete();
    QCOMPARE(doc.text(), QLatin1String("f\n145\nb\nbaz\n"));
    view->keyDelete();
    QCOMPARE(doc.text(), QLatin1String("f\n15\nb\nbaz\n"));
    view->keyDelete();
    QCOMPARE(doc.text(), QLatin1String("f\n1\nb\nbaz\n"));
    view->keyDelete();
    QCOMPARE(doc.text(), QLatin1String("f\n1\nb\nbaz\n"));

    doc.clear();
    QVERIFY(doc.isEmpty());

    doc.insertLines(0, {QStringLiteral("foo"), QStringLiteral("12345"), QStringLiteral("bar"), QStringLiteral("baz")});

    view->setSelection(Range(0, 1, 2, 3));

    view->keyDelete();
    QCOMPARE(doc.text(), QLatin1String("f\n145\nb\nbaz\n"));
}

void KateViewTest::testScrollPastEndOfDocument()
{
    // bug 306745
    KTextEditor::DocumentPrivate doc;
    doc.setText(
        QStringLiteral("0000000000\n"
                       "1111111111\n"
                       "2222222222\n"
                       "3333333333\n"
                       "4444444444"));
    QCOMPARE(doc.lines(), 5);

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    view->setCursorPosition({3, 5});
    view->resize(400, 300);
    view->show();

    // enable "[x] Scroll past end of document"
    view->config()->setValue(KateViewConfig::ScrollPastEnd, true);
    QCOMPARE(view->config()->scrollPastEnd(), true);

    // disable dynamic word wrap
    view->config()->setDynWordWrap(false);
    QCOMPARE(view->config()->dynWordWrap(), false);

    view->scrollDown();
    view->scrollDown();
    view->scrollDown();
    // at this point, only lines 3333333333 and 4444444444 are visible.
    view->down();
    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(4, 5));
    // verify, that only lines 3333333333 and 4444444444 are still visible.
    QCOMPARE(view->firstDisplayedLineInternal(KTextEditor::View::RealLine), 3);
}

void KateViewTest::testFoldFirstLine()
{
    QTemporaryFile file(QStringLiteral("XXXXXX.cpp"));
    file.open();
    QTextStream stream(&file);
    stream << "/**\n"
           << " * foo\n"
           << " */\n"
           << "\n"
           << "int main() {}\n";
    file.close();

    KTextEditor::DocumentPrivate doc;
    QVERIFY(doc.openUrl(QUrl::fromLocalFile(file.fileName())));
    QCOMPARE(doc.highlightingMode(), QStringLiteral("C++"));

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    view->config()->setValue(KateViewConfig::FoldFirstLine, false);
    view->setCursorPosition({4, 0});

    // initially, nothing is folded
    QVERIFY(view->textFolding().isLineVisible(1));

    // now change the config, and expect the header to be folded
    view->config()->setValue(KateViewConfig::FoldFirstLine, true);
    qint64 foldedRangeId = 0;
    QVERIFY(!view->textFolding().isLineVisible(1, &foldedRangeId));

    // now unfold the range
    QVERIFY(view->textFolding().unfoldRange(foldedRangeId));
    QVERIFY(view->textFolding().isLineVisible(1));

    // and save the file, we do not expect the folding to change then
    doc.setModified(true);
    doc.saveFile();
    QVERIFY(view->textFolding().isLineVisible(1));

    // now reload the document, nothing should change
    doc.setModified(false);
    QVERIFY(doc.documentReload());
    QVERIFY(view->textFolding().isLineVisible(1));
}

// test for bug https://bugs.kde.org/374163
void KateViewTest::testDragAndDrop()
{
    // mouse move only on X11
    if (qApp->platformName() != QLatin1String("xcb")) {
        QSKIP("mouse moving only on X11");
    }

    KTextEditor::DocumentPrivate doc(false, false);
    doc.setText(
        QStringLiteral("line0\n"
                       "line1\n"
                       "line2\n"
                       "\n"
                       "line4"));

    KTextEditor::View *view = static_cast<KTextEditor::View *>(doc.createView(nullptr));
    view->show();
    view->resize(400, 300);

    QWidget *internalView = findViewInternal(view);
    QVERIFY(internalView);

    // select "line1\n"
    view->setSelection(Range(1, 0, 2, 0));
    QCOMPARE(view->selectionRange(), Range(1, 0, 2, 0));

    (void)QTest::qWaitForWindowExposed(view);
    QTest::qWait(100); // For whatever reason needed

    const QPoint startDragPos = internalView->mapFrom(view, view->cursorToCoordinate(KTextEditor::Cursor(1, 2)));
    const QPoint endDragPos = internalView->mapFrom(view, view->cursorToCoordinate(KTextEditor::Cursor(3, 0)));
    const QPoint gStartDragPos = internalView->mapToGlobal(startDragPos);
    const QPoint gEndDragPos = internalView->mapToGlobal(endDragPos);

    // now drag and drop selected text to Cursor(3, 0)
    QMouseEvent pressEvent(QEvent::MouseButtonPress, startDragPos, gStartDragPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(internalView, &pressEvent);

    // ugly workaround: Drag & Drop has own blocking event queue. Therefore, we need a single-shot timer to
    // break out of the blocking event queue, see (*)
    QTimer::singleShot(50, [&]() {
        QMouseEvent moveEvent(QEvent::MouseMove, endDragPos + QPoint(5, 0), gEndDragPos + QPoint(5, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, endDragPos, gEndDragPos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(internalView, &moveEvent);
        QCoreApplication::sendEvent(internalView, &releaseEvent);
    });

    // (*) this somehow blocks...
    QMouseEvent moveEvent1(QEvent::MouseMove, endDragPos + QPoint(10, 0), gEndDragPos + QPoint(10, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(internalView, &moveEvent1);

    QTest::qWait(100);

    // final tests of dragged text
    QCOMPARE(doc.text(),
             QStringLiteral("line0\n"
                            "line2\n"
                            "line1\n"
                            "\n"
                            "line4"));

    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(3, 0));
    QCOMPARE(view->selectionRange(), Range(2, 0, 3, 0));
}

// test for bug https://bugs.kde.org/402594
void KateViewTest::testGotoMatchingBracket()
{
    KTextEditor::DocumentPrivate doc(false, false);
    doc.setText(QStringLiteral("foo(bar)baz[]"));
    //           0123456789

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    const KTextEditor::Cursor cursor1(0, 3); // Starting point on open (
    const KTextEditor::Cursor cursor2(0, 8); // Insert Mode differ slightly from...
    const KTextEditor::Cursor cursor3(0, 7); // Overwrite Mode
    const KTextEditor::Cursor cursor4(0, 11); // Test adjacents brackets (start at [)

    doc.config()->setOvr(false); // Insert Mode

    view->setCursorPosition(cursor1);
    view->toMatchingBracket();
    QCOMPARE(view->cursorPosition(), cursor2);
    view->toMatchingBracket();
    QCOMPARE(view->cursorPosition(), cursor1);

    // Currently has it in Insert Mode also to work when the cursor is placed inside the parentheses
    view->setCursorPosition(cursor1 + KTextEditor::Cursor(0, 1));
    view->toMatchingBracket();
    QCOMPARE(view->cursorPosition(), cursor2);
    view->setCursorPosition(cursor2 + KTextEditor::Cursor(0, -1));
    view->toMatchingBracket();
    QCOMPARE(view->cursorPosition(), cursor1);

    view->setCursorPosition(cursor4 + KTextEditor::Cursor(0, 1));
    view->toMatchingBracket();
    QCOMPARE(view->cursorPosition(), cursor4);
    view->toMatchingBracket();
    view->setCursorPosition(view->cursorPosition() + KTextEditor::Cursor(0, 1));
    view->toMatchingBracket();
    QCOMPARE(view->cursorPosition(), cursor4);

    doc.config()->setOvr(true); // Overwrite Mode

    view->setCursorPosition(cursor1);
    view->toMatchingBracket();
    QCOMPARE(view->cursorPosition(), cursor3);
    view->toMatchingBracket();
    QCOMPARE(view->cursorPosition(), cursor1);
}

void KateViewTest::testFindSelected()
{
    KTextEditor::DocumentPrivate doc(false, false);
    doc.setText(
        QStringLiteral("foo\n"
                       "bar\n"
                       "foo\n"
                       "bar\n"));
    //           0123456789

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    const KTextEditor::Cursor cursor1(0, 0); // before 1. foo
    const KTextEditor::Cursor cursor2(0, 3); // after 1. foo
    const KTextEditor::Range range1({0, 0}, {0, 3}); // 1. foo range
    const KTextEditor::Cursor cursor3(2, 3); // after 2. foo
    const KTextEditor::Range range2({2, 0}, {2, 3}); // 2. foo range

    QVERIFY(!view->selection());
    QCOMPARE(view->cursorPosition(), cursor1);

    // first time we call this without a selection, just select the word under the cursor
    view->findSelectedForwards();
    QVERIFY(view->selection());
    QCOMPARE(view->selectionRange(), range1);
    // cursor jumps to the end of the word
    QCOMPARE(view->cursorPosition(), cursor2);

    // second time we call this it actually jumps to the next occurance
    view->findSelectedForwards();
    QCOMPARE(view->selectionRange(), range2);
    QCOMPARE(view->cursorPosition(), cursor3);

    // wrap around
    view->findSelectedForwards();
    QCOMPARE(view->selectionRange(), range1);
    QCOMPARE(view->cursorPosition(), cursor2);

    // search backwards, wrap again
    view->findSelectedBackwards();
    QCOMPARE(view->selectionRange(), range2);
    QCOMPARE(view->cursorPosition(), cursor3);
}

void KateViewTest::testTransposeWord()
{
    KTextEditor::DocumentPrivate doc(false, false);
    doc.setText(
        QStringLiteral("swaps forward\n"
                       "wordAbove\n"
                       "wordLeft (_skips]Spaces.__And___}Sym_bols))))    wordRight)\n"
                       "wordBelow anotherWord yetAnotherWord\n"));

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    const KTextEditor::Cursor swaps(0, 2); // swa|ps
    const KTextEditor::Cursor wordAbove(1, 4); // wordA|bove
    const KTextEditor::Cursor wordLeft(2, 1); // wo|rdLeft
    const KTextEditor::Cursor skips(2, 10); // |_skips
    const KTextEditor::Cursor And(2, 27); // __A|nd___
    const KTextEditor::Cursor wordBelow(3, 0); // w|ordBelow

    view->setCursorPosition(swaps);
    QCOMPARE(view->cursorPosition(), swaps);
    QCOMPARE(view->doc()->characterAt(view->cursorPosition()), QLatin1Char('a'));
    view->transposeWord();
    QCOMPARE(view->cursorPosition(), swaps + KTextEditor::Cursor(0, 8)); // " forward" has 8 characters
    QCOMPARE(view->doc()->characterAt(view->cursorPosition()), QLatin1Char('a')); // retain relative position inside the word

    view->transposeWord();
    QCOMPARE(view->cursorPosition(), swaps); // when the word is already last in line, swap backwards instead

    view->setCursorPosition(wordAbove);
    view->transposeWord();
    QCOMPARE(view->cursorPosition(), wordAbove); // when there is no other word in the line, do nothing
    QCOMPARE(view->doc()->characterAt(view->cursorPosition()), QLatin1Char('A'));

    view->setCursorPosition(wordLeft);
    view->transposeWord();
    QCOMPARE(view->cursorPosition(), wordLeft); // when next word is invalid (made up of only symbols, in this case "(") do nothing
    QCOMPARE(view->doc()->characterAt(view->cursorPosition()), QLatin1Char('o'));

    view->setCursorPosition(skips);
    view->transposeWord();
    QCOMPARE(view->cursorPosition(), skips + KTextEditor::Cursor(0, 7)); // transpose word beginning with a symbol
    QCOMPARE(view->doc()->characterAt(view->cursorPosition()), QLatin1Char('_'));

    view->setCursorPosition(And);
    view->transposeWord();
    // skip multiple symbols if there is no space between current word and the symbols (in contrast to the case of wordLeft).
    // Can be useful for e.g. transposing function arguments
    QCOMPARE(view->cursorPosition(), And + KTextEditor::Cursor(0, 9));

    view->transposeWord();
    QCOMPARE(view->cursorPosition(), And + KTextEditor::Cursor(0, 9 + 17)); // next "word" is invalid as it is only a single "(", so do nothing

    view->setCursorPosition(wordBelow);
    view->transposeWord();
    QCOMPARE(view->cursorPosition(), wordBelow + KTextEditor::Cursor(0, 12));
    view->transposeWord();
    QCOMPARE(view->cursorPosition(), wordBelow + KTextEditor::Cursor(0, 12 + 15));
    view->transposeWord();
    QCOMPARE(view->cursorPosition(), wordBelow + KTextEditor::Cursor(0, 12)); // end of line, transpose backwards instead
}

void KateViewTest::testFindMatchingFoldingMarker()
{
    KTextEditor::DocumentPrivate doc(false, false);
    doc.setMode(QStringLiteral("Bash"));
    doc.setHighlightingMode(QStringLiteral("Bash"));

    doc.setText(
        QStringLiteral("for i in 0 1 2; do\n"
                       "    if [[ i -lt 1 ]]; then echo $i; fi\n"
                       "    if [[ i -eq 2 ]]; then\n"
                       "        echo 'hello :)'\n"
                       "    fi\n"
                       "done\n"));

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    KateViewInternal *viewInternal = view->getViewInternal();

    const auto ifvalue = doc.buffer().computeFoldings(1)[0].foldingRegion;
    const auto dovalue = doc.buffer().computeFoldings(0)[0].foldingRegion;

    const KTextEditor::Range firstDo(0, 16, 0, 18);
    const KTextEditor::Range firstDoMatching(5, 0, 5, 4);
    const KTextEditor::Range firstIf(1, 4, 1, 6);
    const KTextEditor::Range firstIfMatching(1, 36, 1, 38);

    // first test the do folding marker with cursor above first position of the word.
    QCOMPARE(viewInternal->findMatchingFoldingMarker(firstDo.start(), dovalue, 2000), firstDoMatching);
    // with cursor above last position of the word
    QCOMPARE(viewInternal->findMatchingFoldingMarker(firstDo.end(), dovalue, 2000), firstDoMatching);
    // now to test the maxLines param.
    QCOMPARE(viewInternal->findMatchingFoldingMarker(firstDo.start(), dovalue, 2), KTextEditor::Range::invalid());

    // it must work from end folding to start folding too.
    QCOMPARE(viewInternal->findMatchingFoldingMarker(firstDoMatching.start(), dovalue.sibling(), 2000), firstDo);
    QCOMPARE(viewInternal->findMatchingFoldingMarker(firstDoMatching.start(), dovalue.sibling(), 2), KTextEditor::Range::invalid());

    // folding in the same line
    QCOMPARE(viewInternal->findMatchingFoldingMarker(firstIf.start(), ifvalue, 2000), firstIfMatching);
    QCOMPARE(viewInternal->findMatchingFoldingMarker(firstIfMatching.start(), ifvalue.sibling(), 2000), firstIf);
}

void KateViewTest::testUpdateFoldingMarkersHighlighting()
{
    KTextEditor::DocumentPrivate doc(false, false);
    doc.setMode(QStringLiteral("Bash"));
    doc.setHighlightingMode(QStringLiteral("Bash"));

    doc.setText(
        QStringLiteral("for i in 0 1 2; do\n"
                       "    if [[ i -lt 1 ]]; then echo $i; fi\n"
                       "    if [[ i -eq 2 ]]; then\n"
                       "        echo 'hello :)'\n"
                       "    fi\n"
                       "done\n"));

    KTextEditor::ViewPrivate *view = new KTextEditor::ViewPrivate(&doc, nullptr);
    KateViewInternal *viewInternal = view->getViewInternal();

    const KTextEditor::Cursor positionWithoutMarker(0, 4);
    const KTextEditor::Range firstDo(0, 16, 0, 18);
    const KTextEditor::Range firstDoMatching(5, 0, 5, 4);

    KTextEditor::MovingRange *foldingMarkerStart = viewInternal->m_fmStart.get();
    KTextEditor::MovingRange *foldingMarkerEnd = viewInternal->m_fmEnd.get();

    // If the cursor is not above any folding marker, the highlighting range is invalid
    view->editSetCursor(positionWithoutMarker);
    viewInternal->updateFoldingMarkersHighlighting();
    QCOMPARE(foldingMarkerStart->toRange(), KTextEditor::Range::invalid());
    QCOMPARE(foldingMarkerEnd->toRange(), KTextEditor::Range::invalid());

    // If the cursor is above a opening folding marker, the highlighting range is the range of both opening and end folding markers words
    view->editSetCursor(firstDo.start());
    viewInternal->updateFoldingMarkersHighlighting();
    QCOMPARE(foldingMarkerStart->toRange(), firstDo);
    QCOMPARE(foldingMarkerEnd->toRange(), firstDoMatching);

    // If the cursor is above a ending folding marker, then same rule above
    view->editSetCursor(firstDoMatching.start());
    viewInternal->updateFoldingMarkersHighlighting();
    QCOMPARE(foldingMarkerStart->toRange(), firstDo);
    QCOMPARE(foldingMarkerEnd->toRange(), firstDoMatching);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
