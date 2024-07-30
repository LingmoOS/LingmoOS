/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2011 Kuzmich Svyatoslav
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "emulatedcommandbar.h"
#include "emulatedcommandbarsetupandteardown.h"
#include "keys.h"
#include "view.h"
#include "vimode/globalstate.h"
#include "vimode/mappings.h"
#include <inputmode/kateviinputmode.h>
#include <katebuffer.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <kateview.h>
#include <ktexteditor/range.h>
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>
#include <vimode/history.h>

#include <QAbstractItemView>
#include <QClipboard>
#include <QCompleter>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QStringListModel>
#include <QTest>

#include <KActionCollection>
#include <KColorScheme>

QTEST_MAIN(EmulatedCommandBarTest)

using namespace KTextEditor;
using namespace KateVi;

void EmulatedCommandBarTest::EmulatedCommandBarTests()
{
    // Ensure that some preconditions for these tests are setup, and (more importantly)
    // ensure that they are reverted no matter how these tests end.
    EmulatedCommandBarSetUpAndTearDown emulatedCommandBarSetUpAndTearDown(vi_input_mode, kate_view, mainWindow);

    // Verify that we can get a non-null pointer to the emulated command bar.
    EmulatedCommandBar *emulatedCommandBar = vi_input_mode->viModeEmulatedCommandBar();
    QVERIFY(emulatedCommandBar);

    // Should initially be hidden.
    QVERIFY(!emulatedCommandBar->isVisible());

    // Test that "/" invokes the emulated command bar (if we are configured to use it)
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/"));
    QVERIFY(emulatedCommandBar->isVisible());
    QCOMPARE(emulatedCommandTypeIndicator()->text(), QStringLiteral("/"));
    QVERIFY(emulatedCommandTypeIndicator()->isVisible());
    QVERIFY(emulatedCommandBarTextEdit());
    QVERIFY(emulatedCommandBarTextEdit()->text().isEmpty());
    // Make sure the keypresses end up changing the text.
    QVERIFY(emulatedCommandBarTextEdit()->isVisible());
    TestPressKey(QStringLiteral("foo"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo"));
    // Make sure ctrl-c dismisses it (assuming we allow Vim to steal the ctrl-c shortcut).
    TestPressKey(QStringLiteral("\\ctrl-c"));
    QVERIFY(!emulatedCommandBar->isVisible());

    // Ensure that ESC dismisses it, too.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/"));
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\esc"));
    QVERIFY(!emulatedCommandBar->isVisible());
    FinishTest("");

    // Ensure that Ctrl-[ dismisses it, too.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/"));
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-["));
    QVERIFY(!emulatedCommandBar->isVisible());
    FinishTest("");

    // Ensure that Enter dismisses it, too.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/"));
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    QVERIFY(!emulatedCommandBar->isVisible());
    FinishTest("");

    // Ensure that Return dismisses it, too.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/"));
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\return"));
    QVERIFY(!emulatedCommandBar->isVisible());
    FinishTest("");

    // Ensure that text is always initially empty.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/a\\enter"));
    TestPressKey(QStringLiteral("/"));
    QVERIFY(emulatedCommandBarTextEdit()->text().isEmpty());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check backspace works.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo\\backspace"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("fo"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-h works.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/bar\\ctrl-h"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("ba"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // ctrl-h should dismiss bar when empty.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/\\ctrl-h"));
    QVERIFY(!emulatedCommandBar->isVisible());
    FinishTest("");

    // ctrl-h should not dismiss bar when there is stuff to the left of cursor.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/a\\ctrl-h"));
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // ctrl-h should not dismiss bar when bar is not empty, even if there is nothing to the left of cursor.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/a\\left\\ctrl-h"));
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Same for backspace.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/bar\\backspace"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("ba"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/\\backspace"));
    QVERIFY(!emulatedCommandBar->isVisible());
    FinishTest("");
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/a\\backspace"));
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/a\\left\\backspace"));
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-b works.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/bar foo xyz\\ctrl-bX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("Xbar foo xyz"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-e works.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/bar foo xyz\\ctrl-b\\ctrl-eX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("bar foo xyzX"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w works.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo bar\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo "));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w works on empty command bar.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(""));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w works in middle of word.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo bar\\left\\left\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo ar"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w leaves the cursor in the right place when in the middle of word.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo bar\\left\\left\\ctrl-wX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo Xar"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w works when at the beginning of the text.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo\\left\\left\\left\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w works when the character to the left is a space.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo bar   \\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo "));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w works when all characters to the left of the cursor are spaces.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/   \\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(""));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w works when all characters to the left of the cursor are non-spaces.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(""));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w does not continue to delete subsequent alphanumerics if the characters to the left of the cursor
    // are non-space, non-alphanumerics.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo!!!\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");
    // Check ctrl-w does not continue to delete subsequent alphanumerics if the characters to the left of the cursor
    // are non-space, non-alphanumerics.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo!!!\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w deletes underscores and alphanumerics to the left of the cursor, but stops when it reaches a
    // character that is none of these.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo!!!_d1\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo!!!"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w doesn't swallow the spaces preceding the block of non-word chars.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo !!!\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo "));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check ctrl-w doesn't swallow the spaces preceding the word.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo 1d_\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo "));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Check there is a "waiting for register" indicator, initially hidden.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/"));
    QLabel *waitingForRegisterIndicator = emulatedCommandBar->findChild<QLabel *>(QStringLiteral("waitingforregisterindicator"));
    QVERIFY(waitingForRegisterIndicator);
    QVERIFY(!waitingForRegisterIndicator->isVisible());
    QCOMPARE(waitingForRegisterIndicator->text(), QStringLiteral("\""));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Test that ctrl-r causes it to become visible.  It is displayed to the right of the text edit.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/\\ctrl-r"));
    QVERIFY(waitingForRegisterIndicator->isVisible());
    QVERIFY(waitingForRegisterIndicator->x() >= emulatedCommandBarTextEdit()->x() + emulatedCommandBarTextEdit()->width());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("");

    // The first ctrl-c after ctrl-r (when no register entered) hides the waiting for register
    // indicator, but not the bar.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/\\ctrl-r"));
    QVERIFY(waitingForRegisterIndicator->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    QVERIFY(!waitingForRegisterIndicator->isVisible());
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss the bar.
    FinishTest("");

    // The first ctrl-c after ctrl-r (when no register entered) aborts waiting for register.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\"cyiw/\\ctrl-r\\ctrl-ca"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("a"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss the bar.
    FinishTest("foo");

    // Same as above, but for ctrl-[ instead of ctrl-c.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/\\ctrl-r"));
    QVERIFY(waitingForRegisterIndicator->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-["));
    QVERIFY(!waitingForRegisterIndicator->isVisible());
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss the bar.
    FinishTest("");
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\"cyiw/\\ctrl-r\\ctrl-[a"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("a"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss the bar.
    FinishTest("foo");

    // Check ctrl-r works with registers, and hides the "waiting for register" indicator.
    BeginTest(QStringLiteral("xyz"));
    TestPressKey(QStringLiteral("\"ayiw/foo\\ctrl-ra"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("fooxyz"));
    QVERIFY(!waitingForRegisterIndicator->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("xyz");

    // Check ctrl-r inserts text at the current cursor position.
    BeginTest(QStringLiteral("xyz"));
    TestPressKey(QStringLiteral("\"ayiw/foo\\left\\ctrl-ra"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foxyzo"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("xyz");

    // Check ctrl-r ctrl-w inserts word under the cursor, and hides the "waiting for register" indicator.
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("w/\\left\\ctrl-r\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("bar"));
    QVERIFY(!waitingForRegisterIndicator->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar xyz");

    // Check ctrl-r ctrl-w doesn't insert the contents of register w!
    BeginTest(QStringLiteral("foo baz xyz"));
    TestPressKey(QStringLiteral("\"wyiww/\\left\\ctrl-r\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("baz"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo baz xyz");

    // Check ctrl-r ctrl-w inserts at the current cursor position.
    BeginTest(QStringLiteral("foo nose xyz"));
    TestPressKey(QStringLiteral("w/bar\\left\\ctrl-r\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("banoser"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo nose xyz");

    // Cursor position is at the end of the inserted text after ctrl-r ctrl-w.
    BeginTest(QStringLiteral("foo nose xyz"));
    TestPressKey(QStringLiteral("w/bar\\left\\ctrl-r\\ctrl-wX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("banoseXr"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo nose xyz");

    // Cursor position is at the end of the inserted register contents after ctrl-r.
    BeginTest(QStringLiteral("xyz"));
    TestPressKey(QStringLiteral("\"ayiw/foo\\left\\ctrl-raX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foxyzXo"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("xyz");

    // Insert clipboard contents on ctrl-r +.  We implicitly need to test the ability to handle
    // shift key key events when waiting for register (they should be ignored).
    BeginTest(QStringLiteral("xyz"));
    QApplication::clipboard()->setText(QStringLiteral("vimodetestclipboardtext"));
    TestPressKey(QStringLiteral("/\\ctrl-r"));
    QKeyEvent *shiftKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier);
    QApplication::postEvent(emulatedCommandBarTextEdit(), shiftKeyDown);
    QApplication::sendPostedEvents();
    TestPressKey(QStringLiteral("+"));
    QKeyEvent *shiftKeyUp = new QKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier);
    QApplication::postEvent(emulatedCommandBarTextEdit(), shiftKeyUp);
    QApplication::sendPostedEvents();
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("vimodetestclipboardtext"));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("xyz");

    // Similarly, test that we can press "ctrl" after ctrl-r without it being taken for a register.
    BeginTest(QStringLiteral("wordundercursor"));
    TestPressKey(QStringLiteral("/\\ctrl-r"));
    QKeyEvent *ctrlKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Control, Qt::NoModifier);
    QApplication::postEvent(emulatedCommandBarTextEdit(), ctrlKeyDown);
    QApplication::sendPostedEvents();
    QKeyEvent *ctrlKeyUp = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Control, Qt::NoModifier);
    QApplication::postEvent(emulatedCommandBarTextEdit(), ctrlKeyUp);
    QApplication::sendPostedEvents();
    QVERIFY(waitingForRegisterIndicator->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("wordundercursor"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss the bar.
    FinishTest("wordundercursor");

    // Begin tests for ctrl-g, which is almost identical to ctrl-r save that the contents, when added,
    // are escaped for searching.
    // Normal register contents/ word under cursor are added as normal.
    BeginTest(QStringLiteral("wordinregisterb wordundercursor"));
    TestPressKey(QStringLiteral("\"byiw"));
    TestPressKey(QStringLiteral("/\\ctrl-g"));
    QVERIFY(waitingForRegisterIndicator->isVisible());
    QVERIFY(waitingForRegisterIndicator->x() >= emulatedCommandBarTextEdit()->x() + emulatedCommandBarTextEdit()->width());
    TestPressKey(QStringLiteral("b"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("wordinregisterb"));
    QVERIFY(!waitingForRegisterIndicator->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c\\ctrl-cw/\\ctrl-g\\ctrl-w"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("wordundercursor"));
    QVERIFY(!waitingForRegisterIndicator->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("wordinregisterb wordundercursor");

    // \'s must be escaped when inserted via ctrl-g.
    DoTest("foo a\\b\\\\c\\\\\\d", "wYb/\\ctrl-g0\\enterrX", "foo X\\b\\\\c\\\\\\d");
    // $'s must be escaped when inserted via ctrl-g.
    DoTest("foo a$b", "wYb/\\ctrl-g0\\enterrX", "foo X$b");
    DoTest("foo a$b$c", "wYb/\\ctrl-g0\\enterrX", "foo X$b$c");
    DoTest("foo a\\$b\\$c", "wYb/\\ctrl-g0\\enterrX", "foo X\\$b\\$c");
    // ^'s must be escaped when inserted via ctrl-g.
    DoTest("foo a^b", "wYb/\\ctrl-g0\\enterrX", "foo X^b");
    DoTest("foo a^b^c", "wYb/\\ctrl-g0\\enterrX", "foo X^b^c");
    DoTest("foo a\\^b\\^c", "wYb/\\ctrl-g0\\enterrX", "foo X\\^b\\^c");
    // .'s must be escaped when inserted via ctrl-g.
    DoTest("foo axb a.b", "wwYgg/\\ctrl-g0\\enterrX", "foo axb X.b");
    DoTest("foo a\\xb Na\\.b", "fNlYgg/\\ctrl-g0\\enterrX", "foo a\\xb NX\\.b");
    // *'s must be escaped when inserted via ctrl-g
    DoTest("foo axxxxb ax*b", "wwYgg/\\ctrl-g0\\enterrX", "foo axxxxb Xx*b");
    DoTest("foo a\\xxxxb Na\\x*X", "fNlYgg/\\ctrl-g0\\enterrX", "foo a\\xxxxb NX\\x*X");
    // /'s must be escaped when inserted via ctrl-g.
    DoTest("foo a a/b", "wwYgg/\\ctrl-g0\\enterrX", "foo a X/b");
    DoTest("foo a a/b/c", "wwYgg/\\ctrl-g0\\enterrX", "foo a X/b/c");
    DoTest("foo a a\\/b\\/c", "wwYgg/\\ctrl-g0\\enterrX", "foo a X\\/b\\/c");
    // ['s and ]'s must be escaped when inserted via ctrl-g.
    DoTest("foo axb a[xyz]b", "wwYgg/\\ctrl-g0\\enterrX", "foo axb X[xyz]b");
    DoTest("foo a[b", "wYb/\\ctrl-g0\\enterrX", "foo X[b");
    DoTest("foo a[b[c", "wYb/\\ctrl-g0\\enterrX", "foo X[b[c");
    DoTest("foo a\\[b\\[c", "wYb/\\ctrl-g0\\enterrX", "foo X\\[b\\[c");
    DoTest("foo a]b", "wYb/\\ctrl-g0\\enterrX", "foo X]b");
    DoTest("foo a]b]c", "wYb/\\ctrl-g0\\enterrX", "foo X]b]c");
    DoTest("foo a\\]b\\]c", "wYb/\\ctrl-g0\\enterrX", "foo X\\]b\\]c");
    // Test that expressions involving {'s and }'s work when inserted via ctrl-g.
    DoTest("foo {", "wYgg/\\ctrl-g0\\enterrX", "foo X");
    DoTest("foo }", "wYgg/\\ctrl-g0\\enterrX", "foo X");
    DoTest("foo aaaaa \\aaaaa a\\{5}", "WWWYgg/\\ctrl-g0\\enterrX", "foo aaaaa \\aaaaa X\\{5}");
    DoTest("foo }", "wYgg/\\ctrl-g0\\enterrX", "foo X");
    // Transform newlines into "\\n" when inserted via ctrl-g.
    DoTest(" \nfoo\nfoo\nxyz\nbar\n123", "jjvjjllygg/\\ctrl-g0\\enterrX", " \nfoo\nXoo\nxyz\nbar\n123");
    DoTest(" \nfoo\nfoo\nxyz\nbar\n123", "jjvjjllygg/\\ctrl-g0/e\\enterrX", " \nfoo\nfoo\nxyz\nbaX\n123");
    // Don't do any escaping for ctrl-r, though.
    BeginTest(QStringLiteral("foo .*$^\\/"));
    TestPressKey(QStringLiteral("wY/\\ctrl-r0"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(".*$^\\/"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo .*$^\\/");
    // Ensure that the flag that says "next register insertion should be escaped for searching"
    // is cleared if we do ctrl-g but then abort with ctrl-c.
    DoTest("foo a$b", "/\\ctrl-g\\ctrl-c\\ctrl-cwYgg/\\ctrl-r0\\enterrX", "Xoo a$b");

    // Ensure that we actually perform a search while typing.
    BeginTest(QStringLiteral("abcd"));
    TestPressKey(QStringLiteral("/c"));
    verifyCursorAt(Cursor(0, 2));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("abcd");

    // Ensure that the search is from the cursor.
    BeginTest(QStringLiteral("acbcd"));
    TestPressKey(QStringLiteral("ll/c"));
    verifyCursorAt(Cursor(0, 3));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("acbcd");

    // Reset the cursor to the original position on Ctrl-C
    BeginTest(QStringLiteral("acbcd"));
    TestPressKey(QStringLiteral("ll/c\\ctrl-crX"));
    FinishTest("acXcd");

    // Reset the cursor to the original position on Ctrl-[
    BeginTest(QStringLiteral("acbcd"));
    TestPressKey(QStringLiteral("ll/c\\ctrl-[rX"));
    FinishTest("acXcd");

    // Reset the cursor to the original position on ESC
    BeginTest(QStringLiteral("acbcd"));
    TestPressKey(QStringLiteral("ll/c\\escrX"));
    FinishTest("acXcd");

    // *Do not* reset the cursor to the original position on Enter.
    BeginTest(QStringLiteral("acbcd"));
    TestPressKey(QStringLiteral("ll/c\\enterrX"));
    FinishTest("acbXd");

    // *Do not* reset the cursor to the original position on Return.
    BeginTest(QStringLiteral("acbcd"));
    TestPressKey(QStringLiteral("ll/c\\returnrX"));
    FinishTest("acbXd");

    // Should work with mappings.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'testmapping"), QStringLiteral("/c<enter>rX"), Mappings::Recursive);
    BeginTest(QStringLiteral("acbcd"));
    TestPressKey(QStringLiteral("'testmapping"));
    FinishTest("aXbcd");
    clearAllMappings();
    // Don't send keys that were part of a mapping to the emulated command bar.
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("H"), QStringLiteral("/a"), Mappings::Recursive);
    BeginTest(QStringLiteral("foo a aH"));
    TestPressKey(QStringLiteral("H\\enterrX"));
    FinishTest("foo X aH");
    clearAllMappings();

    // Incremental searching from the original position.
    BeginTest(QStringLiteral("foo bar foop fool food"));
    TestPressKey(QStringLiteral("ll/foo"));
    verifyCursorAt(Cursor(0, 8));
    TestPressKey(QStringLiteral("l"));
    verifyCursorAt(Cursor(0, 13));
    TestPressKey(QStringLiteral("\\backspace"));
    verifyCursorAt(Cursor(0, 8));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar foop fool food");

    // End up back at the start if no match found
    BeginTest(QStringLiteral("foo bar foop fool food"));
    TestPressKey(QStringLiteral("ll/fool"));
    verifyCursorAt(Cursor(0, 13));
    TestPressKey(QStringLiteral("\\backspacex"));
    verifyCursorAt(Cursor(0, 2));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar foop fool food");

    // Wrap around if no match found.
    BeginTest(QStringLiteral("afoom bar foop fool food"));
    TestPressKey(QStringLiteral("lll/foom"));
    verifyCursorAt(Cursor(0, 1));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("afoom bar foop fool food");

    // SmartCase: match case-insensitively if the search text is all lower-case.
    DoTest("foo BaR", "ll/bar\\enterrX", "foo XaR");

    // SmartCase: match case-sensitively if the search text is mixed case.
    DoTest("foo BaR bAr", "ll/bAr\\enterrX", "foo BaR XAr");

    // Assume regex by default.
    DoTest("foo bwibblear", "ll/b.*ar\\enterrX", "foo Xwibblear");

    // Set the last search pattern.
    DoTest("foo bar", "ll/bar\\enterggnrX", "foo Xar");

    // Make sure the last search pattern is a regex, too.
    DoTest("foo bwibblear", "ll/b.*ar\\enterggnrX", "foo Xwibblear");

    // 'n' should search case-insensitively if the original search was case-insensitive.
    DoTest("foo bAR", "ll/bar\\enterggnrX", "foo XAR");

    // 'n' should search case-sensitively if the original search was case-sensitive.
    DoTest("foo bar bAR", "ll/bAR\\enterggnrX", "foo bar XAR");

    // 'N' should search case-insensitively if the original search was case-insensitive.
    DoTest("foo bAR xyz", "ll/bar\\enter$NrX", "foo XAR xyz");

    // 'N' should search case-sensitively if the original search was case-sensitive.
    DoTest("foo bAR bar", "ll/bAR\\enter$NrX", "foo XAR bar");

    // Don't forget to set the last search to case-insensitive.
    DoTest("foo bAR bar", "ll/bAR\\enter^/bar\\enter^nrX", "foo XAR bar");

    // Usage of \C for manually specifying case sensitivity.
    // Strip occurrences of "\C" from the pattern to find.
    DoTest("foo bar", "/\\\\Cba\\\\Cr\\enterrX", "foo Xar");
    // Be careful about escaping, though!
    DoTest("foo \\Cba\\Cr", "/\\\\\\\\Cb\\\\Ca\\\\\\\\C\\\\C\\\\Cr\\enterrX", "foo XCba\\Cr");
    // The item added to the search history should contain all the original \C's.
    clearSearchHistory();
    BeginTest(QStringLiteral("foo \\Cba\\Cr"));
    TestPressKey(QStringLiteral("/\\\\\\\\Cb\\\\Ca\\\\\\\\C\\\\C\\\\Cr\\enterrX"));
    QCOMPARE(searchHistory().constFirst(), QStringLiteral("\\\\Cb\\Ca\\\\C\\C\\Cr"));
    FinishTest("foo XCba\\Cr");
    // If there is an escaped C, assume case sensitivity.
    DoTest("foo bAr BAr bar", "/ba\\\\Cr\\enterrX", "foo bAr BAr Xar");
    // The last search pattern should be the last search with escaped C's stripped.
    DoTest("foo \\Cbar\nfoo \\Cbar", "/\\\\\\\\Cba\\\\C\\\\Cr\\enterggjnrX", "foo \\Cbar\nfoo XCbar");
    // If the last search pattern had an escaped "\C", then the next search should be case-sensitive.
    DoTest("foo bar\nfoo bAr BAr bar", "/ba\\\\Cr\\enterggjnrX", "foo bar\nfoo bAr BAr Xar");

    // Don't set the last search parameters if we abort, though.
    DoTest("foo bar xyz", "/bar\\enter/xyz\\ctrl-cggnrX", "foo Xar xyz");
    DoTest("foo bar bAr", "/bar\\enter/bA\\ctrl-cggnrX", "foo Xar bAr");
    DoTest("foo bar bar", "/bar\\enter?ba\\ctrl-cggnrX", "foo Xar bar");

    // Don't let ":" trample all over the search parameters, either.
    DoTest("foo bar xyz foo", "/bar\\entergg*:yank\\enterggnrX", "foo bar xyz Xoo");

    // Some mirror tests for "?"

    // Test that "?" summons the search bar, with empty text and with the "?" indicator.
    QVERIFY(!emulatedCommandBar->isVisible());
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("?"));
    QVERIFY(emulatedCommandBar->isVisible());
    QCOMPARE(emulatedCommandTypeIndicator()->text(), QStringLiteral("?"));
    QVERIFY(emulatedCommandTypeIndicator()->isVisible());
    QVERIFY(emulatedCommandBarTextEdit());
    QVERIFY(emulatedCommandBarTextEdit()->text().isEmpty());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("");

    // Search backwards.
    DoTest("foo foo bar foo foo", "ww?foo\\enterrX", "foo Xoo bar foo foo");

    // Reset cursor if we find nothing.
    BeginTest(QStringLiteral("foo foo bar foo foo"));
    TestPressKey(QStringLiteral("ww?foo"));
    verifyCursorAt(Cursor(0, 4));
    TestPressKey(QStringLiteral("d"));
    verifyCursorAt(Cursor(0, 8));
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo foo bar foo foo");

    // Wrap to the end if we find nothing.
    DoTest("foo foo bar xyz xyz", "ww?xyz\\enterrX", "foo foo bar xyz Xyz");

    // Specify that the last was backwards when using '?'
    DoTest("foo foo bar foo foo", "ww?foo\\enter^wwnrX", "foo Xoo bar foo foo");

    // ... and make sure we do  the equivalent with "/"
    BeginTest(QStringLiteral("foo foo bar foo foo"));
    TestPressKey(QStringLiteral("ww?foo\\enter^ww/foo"));
    QCOMPARE(emulatedCommandTypeIndicator()->text(), QStringLiteral("/"));
    TestPressKey(QStringLiteral("\\enter^wwnrX"));
    FinishTest("foo foo bar Xoo foo");

    // If we are at the beginning of a word, that word is not the first match in a search
    // for that word.
    DoTest("foo foo foo", "w/foo\\enterrX", "foo foo Xoo");
    DoTest("foo foo foo", "w?foo\\enterrX", "Xoo foo foo");
    // When searching backwards, ensure we can find a match whose range includes the starting cursor position,
    // if we allow it to wrap around.
    DoTest("foo foofoofoo bar", "wlll?foofoofoo\\enterrX", "foo Xoofoofoo bar");
    // When searching backwards, ensure we can find a match whose range includes the starting cursor position,
    // even if we don't allow it to wrap around.
    DoTest("foo foofoofoo foofoofoo", "wlll?foofoofoo\\enterrX", "foo Xoofoofoo foofoofoo");
    // The same, but where we the match ends at the end of the line or document.
    DoTest("foo foofoofoo\nfoofoofoo", "wlll?foofoofoo\\enterrX", "foo Xoofoofoo\nfoofoofoo");
    DoTest("foo foofoofoo", "wlll?foofoofoo\\enterrX", "foo Xoofoofoo");

    // Searching forwards for just "/" repeats last search.
    DoTest("foo bar", "/bar\\entergg//\\enterrX", "foo Xar");
    // The "last search" can be one initiated via e.g. "*".
    DoTest("foo bar foo", "/bar\\entergg*gg//\\enterrX", "foo bar Xoo");
    // Searching backwards for just "?" repeats last search.
    DoTest("foo bar bar", "/bar\\entergg??\\enterrX", "foo bar Xar");
    // Search forwards treats "?" as a literal.
    DoTest("foo ?ba?r", "/?ba?r\\enterrX", "foo Xba?r");
    // As always, be careful with escaping!
    DoTest("foo ?ba\\?r", "/?ba\\\\\\\\\\\\?r\\enterrX", "foo Xba\\?r");
    // Searching forwards for just "?" finds literal question marks.
    DoTest("foo ??", "/?\\enterrX", "foo X?");
    // Searching backwards for just "/" finds literal forward slashes.
    DoTest("foo //", "?/\\enterrX", "foo /X");
    // Searching forwards, stuff after (and including) an unescaped "/" is ignored.
    DoTest("foo ba bar bar/xyz", "/bar/xyz\\enterrX", "foo ba Xar bar/xyz");
    // Needs to be unescaped, though!
    DoTest("foo bar bar/xyz", "/bar\\\\/xyz\\enterrX", "foo bar Xar/xyz");
    DoTest("foo bar bar\\/xyz", "/bar\\\\\\\\/xyz\\enterrX", "foo bar Xar\\/xyz");
    // Searching backwards, stuff after (and including) an unescaped "?" is ignored.
    DoTest("foo bar bar?xyz bar ba", "?bar?xyz\\enterrX", "foo bar bar?xyz Xar ba");
    // Needs to be unescaped, though!
    DoTest("foo bar bar?xyz bar ba", "?bar\\\\?xyz\\enterrX", "foo bar Xar?xyz bar ba");
    DoTest("foo bar bar\\?xyz bar ba", "?bar\\\\\\\\?xyz\\enterrX", "foo bar Xar\\?xyz bar ba");
    // If, in a forward search, the first character after the first unescaped "/" is an e, then
    // we place the cursor at the end of the word.
    DoTest("foo ba bar bar/eyz", "/bar/e\\enterrX", "foo ba baX bar/eyz");
    // Needs to be unescaped, though!
    DoTest("foo bar bar/eyz", "/bar\\\\/e\\enterrX", "foo bar Xar/eyz");
    DoTest("foo bar bar\\/xyz", "/bar\\\\\\\\/e\\enterrX", "foo bar barX/xyz");
    // If, in a backward search, the first character after the first unescaped "?" is an e, then
    // we place the cursor at the end of the word.
    DoTest("foo bar bar?eyz bar ba", "?bar?e\\enterrX", "foo bar bar?eyz baX ba");
    // Needs to be unescaped, though!
    DoTest("foo bar bar?eyz bar ba", "?bar\\\\?e\\enterrX", "foo bar Xar?eyz bar ba");
    DoTest("foo bar bar\\?eyz bar ba", "?bar\\\\\\\\?e\\enterrX", "foo bar barX?eyz bar ba");
    // Quick check that repeating the last search and placing the cursor at the end of the match works.
    DoTest("foo bar bar", "/bar\\entergg//e\\enterrX", "foo baX bar");
    DoTest("foo bar bar", "?bar\\entergg??e\\enterrX", "foo bar baX");
    // When repeating a change, don't try to convert from Vim to Qt regex again.
    DoTest("foo bar()", "/bar()\\entergg//e\\enterrX", "foo bar(X");
    DoTest("foo bar()", "?bar()\\entergg??e\\enterrX", "foo bar(X");
    // If the last search said that we should place the cursor at the end of the match, then
    // do this with n & N.
    DoTest("foo bar bar foo", "/bar/e\\enterggnrX", "foo baX bar foo");
    DoTest("foo bar bar foo", "/bar/e\\enterggNrX", "foo bar baX foo");
    // Don't do this if that search was aborted, though.
    DoTest("foo bar bar foo", "/bar\\enter/bar/e\\ctrl-cggnrX", "foo Xar bar foo");
    DoTest("foo bar bar foo", "/bar\\enter/bar/e\\ctrl-cggNrX", "foo bar Xar foo");
    // "#" and "*" reset the "place cursor at the end of the match" to false.
    DoTest("foo bar bar foo", "/bar/e\\enterggw*nrX", "foo Xar bar foo");
    DoTest("foo bar bar foo", "/bar/e\\enterggw#nrX", "foo Xar bar foo");

    // "/" and "?" should be usable as motions.
    DoTest("foo bar", "ld/bar\\enter", "fbar");
    // They are not linewise.
    DoTest("foo bar\nxyz", "ld/yz\\enter", "fyz");
    DoTest("foo bar\nxyz", "jld?oo\\enter", "fyz");
    // Should be usable in Visual Mode without aborting Visual Mode.
    DoTest("foo bar", "lv/bar\\enterd", "far");
    // Same for ?.
    DoTest("foo bar", "$hd?oo\\enter", "far");
    DoTest("foo bar", "$hv?oo\\enterd", "fr");
    DoTest("foo bar", "lv?bar\\enterd", "far");
    // If we abort the "/" / "?" motion, the command should be aborted, too.
    DoTest("foo bar", "d/bar\\esc", "foo bar");
    DoTest("foo bar", "d/bar\\ctrl-c", "foo bar");
    DoTest("foo bar", "d/bar\\ctrl-[", "foo bar");
    // We should be able to repeat a command using "/" or "?" as the motion.
    DoTest("foo bar bar bar", "d/bar\\enter.", "bar bar");
    // The "synthetic" Enter keypress should not be logged as part of the command to be repeated.
    DoTest("foo bar bar bar\nxyz", "d/bar\\enter.rX", "Xar bar\nxyz");
    // Counting.
    DoTest("foo bar bar bar", "2/bar\\enterrX", "foo bar Xar bar");
    // Counting with wraparound.
    DoTest("foo bar bar bar", "4/bar\\enterrX", "foo Xar bar bar");
    // Counting in Visual Mode.
    DoTest("foo bar bar bar", "v2/bar\\enterd", "ar bar");
    // Should update the selection in Visual Mode as we search.
    BeginTest(QStringLiteral("foo bar bbc"));
    TestPressKey(QStringLiteral("vl/b"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("foo b"));
    TestPressKey(QStringLiteral("b"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("foo bar b"));
    TestPressKey(QStringLiteral("\\ctrl-h"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("foo b"));
    TestPressKey(QStringLiteral("notexists"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("fo"));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    QCOMPARE(kate_view->selectionText(), QStringLiteral("fo"));
    FinishTest("foo bar bbc");
    BeginTest(QStringLiteral("foo\nxyz\nbar\nbbc"));
    TestPressKey(QStringLiteral("Vj/b"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("foo\nxyz\nbar"));
    TestPressKey(QStringLiteral("b"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("foo\nxyz\nbar\nbbc"));
    TestPressKey(QStringLiteral("\\ctrl-h"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("foo\nxyz\nbar"));
    TestPressKey(QStringLiteral("notexists"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("foo\nxyz"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo\nxyz\nbar\nbbc");
    // Dismissing the search bar in visual mode should leave original selection.
    BeginTest(QStringLiteral("foo bar bbc"));
    TestPressKey(QStringLiteral("vl/\\ctrl-c"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("fo"));
    FinishTest("foo bar bbc");
    BeginTest(QStringLiteral("foo bar bbc"));
    TestPressKey(QStringLiteral("vl?\\ctrl-c"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("fo"));
    FinishTest("foo bar bbc");
    BeginTest(QStringLiteral("foo bar bbc"));
    TestPressKey(QStringLiteral("vl/b\\ctrl-c"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("fo"));
    FinishTest("foo bar bbc");
    BeginTest(QStringLiteral("foo\nbar\nbbc"));
    TestPressKey(QStringLiteral("Vl/b\\ctrl-c"));
    QCOMPARE(kate_view->selectionText(), QStringLiteral("foo"));
    FinishTest("foo\nbar\nbbc");

    // Search-highlighting tests.
    const QColor searchHighlightColour = kate_view->rendererConfig()->searchHighlightColor();
    BeginTest(QStringLiteral("foo bar xyz"));

    // Sanity test.
    const QList<Kate::TextRange *> rangesInitial = rangesOnFirstLine();
    Q_ASSERT(rangesInitial.isEmpty() && "Assumptions about ranges are wrong - this test is invalid and may need updating!");
    FinishTest("foo bar xyz");

    // Test highlighting single character match.
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("/b"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->attribute()->background().color(), searchHighlightColour);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().column(), 4);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().column(), 5);
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar xyz");

    // Test highlighting two character match.
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("/ba"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().column(), 4);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().column(), 6);
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar xyz");

    // Test no highlighting if no longer a match.
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("/baz"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar xyz");

    // Test highlighting on wraparound.
    BeginTest(QStringLiteral(" foo bar xyz"));
    TestPressKey(QStringLiteral("ww/foo"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().column(), 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().column(), 4);
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest(" foo bar xyz");

    // Test highlighting backwards
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("$?ba"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().column(), 4);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().column(), 6);
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar xyz");

    // Test no highlighting when no match is found searching backwards
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("$?baz"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar xyz");

    // Test highlight when wrapping around after searching backwards.
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("w?xyz"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().column(), 8);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().column(), 11);
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar xyz");

    // Test no highlighting when bar is dismissed.
    DoTest("foo bar xyz", "/bar\\ctrl-c", "foo bar xyz");
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());
    DoTest("foo bar xyz", ":set-nohls\\enter/bar\\enter", "foo bar xyz");
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());
    DoTest("foo bar xyz", "/bar\\ctrl-[", "foo bar xyz");
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());
    DoTest("foo bar xyz", ":set-nohls\\enter/bar\\return", "foo bar xyz");
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());
    DoTest("foo bar xyz", "/bar\\esc", "foo bar xyz");
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());

    // Update colour on config change.
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("/xyz"));
    const QColor newSearchHighlightColour = QColor(255, 0, 0);
    kate_view->rendererConfig()->setSearchHighlightColor(newSearchHighlightColour);
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->attribute()->background().color(), newSearchHighlightColour);
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar xyz");

    // Set the background colour appropriately.
    KColorScheme currentColorScheme(QPalette::Normal);
    const QColor normalBackgroundColour = QPalette().brush(QPalette::Base).color();
    const QColor matchBackgroundColour = currentColorScheme.background(KColorScheme::PositiveBackground).color();
    const QColor noMatchBackgroundColour = currentColorScheme.background(KColorScheme::NegativeBackground).color();
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("/xyz"));
    verifyTextEditBackgroundColour(matchBackgroundColour);
    TestPressKey(QStringLiteral("a"));
    verifyTextEditBackgroundColour(noMatchBackgroundColour);
    TestPressKey(QStringLiteral("\\ctrl-w"));
    verifyTextEditBackgroundColour(normalBackgroundColour);
    TestPressKey(QStringLiteral("/xyz\\enter/"));
    verifyTextEditBackgroundColour(normalBackgroundColour);
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar xyz");

    // Escape regex's in a Vim-ish style.
    // Unescaped ( and ) are always literals.
    DoTest("foo bar( xyz", "/bar(\\enterrX", "foo Xar( xyz");
    DoTest("foo bar) xyz", "/bar)\\enterrX", "foo Xar) xyz");
    // + is literal, unless it is already escaped.
    DoTest("foo bar+ xyz", "/bar+ \\enterrX", "foo Xar+ xyz");
    DoTest("  foo+AAAAbar", "/foo+A\\\\+bar\\enterrX", "  Xoo+AAAAbar");
    DoTest("  foo++++bar", "/foo+\\\\+bar\\enterrX", "  Xoo++++bar");
    DoTest("  foo++++bar", "/+\\enterrX", "  fooX+++bar");
    // An escaped "\" is a literal, of course.
    DoTest("foo x\\y", "/x\\\\\\\\y\\enterrX", "foo X\\y");
    // ( and ), if escaped, are not literals.
    DoTest("foo  barbarxyz", "/ \\\\(bar\\\\)\\\\+xyz\\enterrX", "foo Xbarbarxyz");
    // Handle escaping correctly if we have an escaped and unescaped bracket next to each other.
    DoTest("foo  x(A)y", "/x(\\\\(.\\\\))y\\enterrX", "foo  X(A)y");
    // |, if unescaped, is literal.
    DoTest("foo |bar", "/|\\enterrX", "foo Xbar");
    // |, if escaped, is not a literal.
    DoTest("foo xfoo\\y xbary", "/x\\\\(foo\\\\|bar\\\\)y\\enterrX", "foo xfoo\\y Xbary");
    // A single [ is a literal.
    DoTest("foo bar[", "/bar[\\enterrX", "foo Xar[");
    // A single ] is a literal.
    DoTest("foo bar]", "/bar]\\enterrX", "foo Xar]");
    // A matching [ and ] are *not* literals.
    DoTest("foo xbcay", "/x[abc]\\\\+y\\enterrX", "foo Xbcay");
    DoTest("foo xbcay", "/[abc]\\\\+y\\enterrX", "foo xXcay");
    DoTest("foo xbaadcdcy", "/x[ab]\\\\+[cd]\\\\+y\\enterrX", "foo Xbaadcdcy");
    // Need to be an unescaped match, though.
    DoTest("foo xbcay", "/x[abc\\\\]\\\\+y\\enterrX", "Xoo xbcay");
    DoTest("foo xbcay", "/x\\\\[abc]\\\\+y\\enterrX", "Xoo xbcay");
    DoTest("foo x[abc]]]]]y", "/x\\\\[abc]\\\\+y\\enterrX", "foo X[abc]]]]]y");
    // An escaped '[' between matching unescaped '[' and ']' is treated as a literal '['
    DoTest("foo xb[cay", "/x[a\\\\[bc]\\\\+y\\enterrX", "foo Xb[cay");
    // An escaped ']' between matching unescaped '[' and ']' is treated as a literal ']'
    DoTest("foo xb]cay", "/x[a\\\\]bc]\\\\+y\\enterrX", "foo Xb]cay");
    // An escaped '[' not between other square brackets is a literal.
    DoTest("foo xb[cay", "/xb\\\\[\\enterrX", "foo Xb[cay");
    DoTest("foo xb[cay", "/\\\\[ca\\enterrX", "foo xbXcay");
    // An escaped ']' not between other square brackets is a literal.
    DoTest("foo xb]cay", "/xb\\\\]\\enterrX", "foo Xb]cay");
    DoTest("foo xb]cay", "/\\\\]ca\\enterrX", "foo xbXcay");
    // An unescaped '[' not between other square brackets is a literal.
    DoTest("foo xbaba[y", "/x[ab]\\\\+[y\\enterrX", "foo Xbaba[y");
    DoTest("foo xbaba[dcdcy", "/x[ab]\\\\+[[cd]\\\\+y\\enterrX", "foo Xbaba[dcdcy");
    // An unescaped ']' not between other square brackets is a literal.
    DoTest("foo xbaba]y", "/x[ab]\\\\+]y\\enterrX", "foo Xbaba]y");
    DoTest("foo xbaba]dcdcy", "/x[ab]\\\\+][cd]\\\\+y\\enterrX", "foo Xbaba]dcdcy");
    // Be more clever about how we identify escaping: the presence of a preceding
    // backslash is not always sufficient!
    DoTest("foo x\\babay", "/x\\\\\\\\[ab]\\\\+y\\enterrX", "foo X\\babay");
    DoTest("foo x\\[abc]]]]y", "/x\\\\\\\\\\\\[abc]\\\\+y\\enterrX", "foo X\\[abc]]]]y");
    DoTest("foo xa\\b\\c\\y", "/x[abc\\\\\\\\]\\\\+y\\enterrX", "foo Xa\\b\\c\\y");
    DoTest("foo x[abc\\]]]]y", "/x[abc\\\\\\\\\\\\]\\\\+y\\enterrX", "foo X[abc\\]]]]y");
    DoTest("foo xa[\\b\\[y", "/x[ab\\\\\\\\[]\\\\+y\\enterrX", "foo Xa[\\b\\[y");
    DoTest("foo x\\[y", "/x\\\\\\\\[y\\enterrX", "foo X\\[y");
    DoTest("foo x\\]y", "/x\\\\\\\\]y\\enterrX", "foo X\\]y");
    DoTest("foo x\\+y", "/x\\\\\\\\+y\\enterrX", "foo X\\+y");
    // A dot is not a literal, nor is a star.
    DoTest("foo bar", "/o.*b\\enterrX", "fXo bar");
    // Escaped dots and stars are literals, though.
    DoTest("foo xay x.y", "/x\\\\.y\\enterrX", "foo xay X.y");
    DoTest("foo xaaaay xa*y", "/xa\\\\*y\\enterrX", "foo xaaaay Xa*y");
    // Unescaped curly braces are literals.
    DoTest("foo x{}y", "/x{}y\\enterrX", "foo X{}y");
    // Escaped curly brackets are quantifers.
    DoTest("foo xaaaaay", "/xa\\\\{5\\\\}y\\enterrX", "foo Xaaaaay");
    // Matching curly brackets where only the first is escaped are also quantifiers.
    DoTest("foo xaaaaaybbbz", "/xa\\\\{5}yb\\\\{3}z\\enterrX", "foo Xaaaaaybbbz");
    // Make sure it really is escaped, though!
    DoTest("foo xa\\{5}", "/xa\\\\\\\\{5}\\enterrX", "foo Xa\\{5}");
    // Don't crash if the first character is a }
    DoTest("foo aaaaay", "/{\\enterrX", "Xoo aaaaay");
    // Vim's '\<' and '\>' map, roughly, to Qt's '\b'
    DoTest("foo xbar barx bar", "/bar\\\\>\\enterrX", "foo xXar barx bar");
    DoTest("foo xbar barx bar", "/\\\\<bar\\enterrX", "foo xbar Xarx bar");
    DoTest("foo xbar barx bar ", "/\\\\<bar\\\\>\\enterrX", "foo xbar barx Xar ");
    DoTest("foo xbar barx bar", "/\\\\<bar\\\\>\\enterrX", "foo xbar barx Xar");
    DoTest("foo xbar barx\nbar", "/\\\\<bar\\\\>\\enterrX", "foo xbar barx\nXar");
    // Escaped "^" and "$" are treated as literals.
    DoTest("foo x^$y", "/x\\\\^\\\\$y\\enterrX", "foo X^$y");
    // Ensure that it is the escaped version of the pattern that is recorded as the last search pattern.
    DoTest("foo bar( xyz", "/bar(\\enterggnrX", "foo Xar( xyz");

    // Don't log keypresses sent to the emulated command bar as commands to be repeated via "."!
    DoTest("foo", "/diw\\enterciwbar\\ctrl-c.", "bar");

    // Don't leave Visual mode on aborting a search.
    DoTest("foo bar", "vw/\\ctrl-cd", "ar");
    DoTest("foo bar", "vw/\\ctrl-[d", "ar");

    // Don't crash on leaving Visual Mode on aborting a search. This is perhaps the most opaque regression
    // test ever; what it's testing for is the situation where the synthetic keypress issue by the emulated
    // command bar on the "ctrl-[" is sent to the key mapper.  This in turn converts it into a weird character
    // which is then, upon not being recognised as part of a mapping, sent back around the keypress processing,
    // where it ends up being sent to the emulated command bar's text edit, which in turn issues a "text changed"
    // event where the text is still empty, which tries to move the cursor to (-1, -1), which causes a crash deep
    // within Kate. So, in a nutshell: this test ensures that the keymapper never handles the synthetic keypress :)
    DoTest("", "ifoo\\ctrl-cv/\\ctrl-[", "foo");

    // History auto-completion tests.
    clearSearchHistory();
    QVERIFY(searchHistory().isEmpty());
    vi_global->searchHistory()->append(QStringLiteral("foo"));
    vi_global->searchHistory()->append(QStringLiteral("bar"));
    QCOMPARE(searchHistory(), QStringList() << QStringLiteral("foo") << QStringLiteral("bar"));
    clearSearchHistory();
    QVERIFY(searchHistory().isEmpty());

    // Ensure current search bar text is added to the history if we press enter.
    DoTest("foo bar", "/bar\\enter", "foo bar");
    DoTest("foo bar", "/xyz\\enter", "foo bar");
    QCOMPARE(searchHistory(), QStringList() << QStringLiteral("bar") << QStringLiteral("xyz"));
    // Interesting - Vim adds the search bar text to the history even if we abort via e.g. ctrl-c, ctrl-[, etc.
    clearSearchHistory();
    DoTest("foo bar", "/baz\\ctrl-[", "foo bar");
    QCOMPARE(searchHistory(), QStringList() << QStringLiteral("baz"));
    clearSearchHistory();
    DoTest("foo bar", "/foo\\esc", "foo bar");
    QCOMPARE(searchHistory(), QStringList() << QStringLiteral("foo"));
    clearSearchHistory();
    DoTest("foo bar", "/nose\\ctrl-c", "foo bar");
    QCOMPARE(searchHistory(), QStringList() << QStringLiteral("nose"));

    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("foo"));
    vi_global->searchHistory()->append(QStringLiteral("bar"));
    QVERIFY(emulatedCommandBarCompleter() != nullptr);
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("/\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    // Make sure the completion appears in roughly the correct place: this is a little fragile :/
    const QPoint completerRectTopLeft = emulatedCommandBarCompleter()->popup()->mapToGlobal(emulatedCommandBarCompleter()->popup()->rect().topLeft());
    const QPoint barEditBottomLeft = emulatedCommandBarTextEdit()->mapToGlobal(emulatedCommandBarTextEdit()->rect().bottomLeft());
    QCOMPARE(completerRectTopLeft.x(), barEditBottomLeft.x());
    QVERIFY(qAbs(completerRectTopLeft.y() - barEditBottomLeft.y()) <= 1);
    // Will activate the current completion item, activating the search, and dismissing the bar.
    TestPressKey(QStringLiteral("\\enter"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    // Close the command bar.
    FinishTest("foo bar");

    // Don't show completion with an empty search bar.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("foo"));
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("/"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar");

    // Don't auto-complete, either.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("foo"));
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("/f"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    FinishTest("foo bar");

    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyz"));
    vi_global->searchHistory()->append(QStringLiteral("bar"));
    QVERIFY(emulatedCommandBarCompleter() != nullptr);
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("/\\ctrl-p"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("bar"));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("foo bar");

    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyz"));
    vi_global->searchHistory()->append(QStringLiteral("bar"));
    vi_global->searchHistory()->append(QStringLiteral("foo"));
    QVERIFY(emulatedCommandBarCompleter() != nullptr);
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("/\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("foo"));
    QCOMPARE(emulatedCommandBarCompleter()->popup()->currentIndex().row(), 0);
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("bar"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("bar"));
    QCOMPARE(emulatedCommandBarCompleter()->popup()->currentIndex().row(), 1);
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("xyz"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("xyz"));
    QCOMPARE(emulatedCommandBarCompleter()->popup()->currentIndex().row(), 2);
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("foo")); // Wrap-around
    QCOMPARE(emulatedCommandBarCompleter()->popup()->currentIndex().row(), 0);
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("foo bar");

    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyz"));
    vi_global->searchHistory()->append(QStringLiteral("bar"));
    vi_global->searchHistory()->append(QStringLiteral("foo"));
    QVERIFY(emulatedCommandBarCompleter() != nullptr);
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("/\\ctrl-n"));
    verifyCommandBarCompletionVisible();
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("xyz"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("xyz"));
    QCOMPARE(emulatedCommandBarCompleter()->popup()->currentIndex().row(), 2);
    TestPressKey(QStringLiteral("\\ctrl-n"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("bar"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("bar"));
    QCOMPARE(emulatedCommandBarCompleter()->popup()->currentIndex().row(), 1);
    TestPressKey(QStringLiteral("\\ctrl-n"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("foo"));
    QCOMPARE(emulatedCommandBarCompleter()->popup()->currentIndex().row(), 0);
    TestPressKey(QStringLiteral("\\ctrl-n"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("xyz"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("xyz")); // Wrap-around.
    QCOMPARE(emulatedCommandBarCompleter()->popup()->currentIndex().row(), 2);
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("foo bar");

    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyz"));
    vi_global->searchHistory()->append(QStringLiteral("bar"));
    vi_global->searchHistory()->append(QStringLiteral("foo"));
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("/\\ctrl-n\\ctrl-n"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("bar"));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("foo bar");

    // If we add something to the history, remove any earliest occurrences (this is what Vim appears to do)
    // and append to the end.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("bar"));
    vi_global->searchHistory()->append(QStringLiteral("xyz"));
    vi_global->searchHistory()->append(QStringLiteral("foo"));
    vi_global->searchHistory()->append(QStringLiteral("xyz"));
    QCOMPARE(searchHistory(), QStringList() << QStringLiteral("bar") << QStringLiteral("foo") << QStringLiteral("xyz"));

    // Push out older entries if we have too many search items in the history.
    const int HISTORY_SIZE_LIMIT = 100;
    clearSearchHistory();
    for (int i = 1; i <= HISTORY_SIZE_LIMIT; i++) {
        vi_global->searchHistory()->append(QStringLiteral("searchhistoryitem %1").arg(i));
    }
    QCOMPARE(searchHistory().size(), HISTORY_SIZE_LIMIT);
    QCOMPARE(searchHistory().constFirst(), QStringLiteral("searchhistoryitem 1"));
    QCOMPARE(searchHistory().last(), QStringLiteral("searchhistoryitem 100"));
    vi_global->searchHistory()->append(QStringLiteral("searchhistoryitem %1").arg(HISTORY_SIZE_LIMIT + 1));
    QCOMPARE(searchHistory().size(), HISTORY_SIZE_LIMIT);
    QCOMPARE(searchHistory().constFirst(), QStringLiteral("searchhistoryitem 2"));
    QCOMPARE(searchHistory().last(), QStringLiteral("searchhistoryitem %1").arg(HISTORY_SIZE_LIMIT + 1));

    // Don't add empty searches to the history.
    clearSearchHistory();
    DoTest("foo bar", "/\\enter", "foo bar");
    QVERIFY(searchHistory().isEmpty());

    // "*" and "#" should add the relevant word to the search history, enclosed between \< and \>
    clearSearchHistory();
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("*"));
    QVERIFY(!searchHistory().isEmpty());
    QCOMPARE(searchHistory().last(), QStringLiteral("\\<foo\\>"));
    TestPressKey(QStringLiteral("w#"));
    QCOMPARE(searchHistory().size(), 2);
    QCOMPARE(searchHistory().last(), QStringLiteral("\\<bar\\>"));

    // Auto-complete words from the document on ctrl-space.
    // Test that we can actually find a single word and add it to the list of completions.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral("/\\ctrl- "));
    verifyCommandBarCompletionVisible();
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("foo");

    // Count digits and underscores as being part of a word.
    BeginTest(QStringLiteral("foo_12"));
    TestPressKey(QStringLiteral("/\\ctrl- "));
    verifyCommandBarCompletionVisible();
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("foo_12"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("foo_12");

    // This feels a bit better to me, usability-wise: in the special case of completion from document, where
    // the completion list is manually summoned, allow one to press Enter without the bar being dismissed
    // (just dismiss the completion list instead).
    BeginTest(QStringLiteral("foo_12"));
    TestPressKey(QStringLiteral("/\\ctrl- \\ctrl-p\\enter"));
    QVERIFY(emulatedCommandBar->isVisible());
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("foo_12");

    // Check that we can find multiple words on one line.
    BeginTest(QStringLiteral("bar (foo) [xyz]"));
    TestPressKey(QStringLiteral("/\\ctrl- "));
    QStringListModel *completerStringListModel = static_cast<QStringListModel *>(emulatedCommandBarCompleter()->model());
    Q_ASSERT(completerStringListModel);
    QCOMPARE(completerStringListModel->stringList(), QStringList() << QStringLiteral("bar") << QStringLiteral("foo") << QStringLiteral("xyz"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("bar (foo) [xyz]");

    // Check that we arrange the found words in case-insensitive sorted order.
    BeginTest(QStringLiteral("D c e a b f"));
    TestPressKey(QStringLiteral("/\\ctrl- "));
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("c") << QStringLiteral("D")
                                                     << QStringLiteral("e") << QStringLiteral("f"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("D c e a b f");

    // Check that we don't include the same word multiple times.
    BeginTest(QStringLiteral("foo bar bar bar foo"));
    TestPressKey(QStringLiteral("/\\ctrl- "));
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("bar") << QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("foo bar bar bar foo");

    // Check that we search only a narrow portion of the document, around the cursor (4096 lines either
    // side, say).
    QStringList manyLines;
    for (int i = 1; i < 2 * 4096 + 3; i++) {
        // Pad the digits so that when sorted alphabetically, they are also sorted numerically.
        manyLines << QStringLiteral("word%1").arg(i, 5, 10, QLatin1Char('0'));
    }
    QStringList allButFirstAndLastOfManyLines = manyLines;
    allButFirstAndLastOfManyLines.removeFirst();
    allButFirstAndLastOfManyLines.removeLast();

    BeginTest(manyLines.join(QStringLiteral("\n")));
    TestPressKey(QStringLiteral("4097j/\\ctrl- "));
    verifyCommandBarCompletionsMatches(allButFirstAndLastOfManyLines);
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest(manyLines.join(QStringLiteral("\n")).toUtf8().constData());

    // "The current word" means the word before the cursor in the command bar, and includes numbers
    // and underscores. Make sure also that the completion prefix is set when the completion is first invoked.
    BeginTest(QStringLiteral("foo fee foa_11 foa_11b"));
    // Write "bar(foa112$nose" and position cursor before the "2", then invoke completion.
    TestPressKey(QStringLiteral("/bar(foa_112$nose\\left\\left\\left\\left\\left\\left\\ctrl- "));
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("foa_11") << QStringLiteral("foa_11b"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("foo fee foa_11 foa_11b");

    // But don't count "-" as being part of the current word.
    BeginTest(QStringLiteral("foo_12"));
    TestPressKey(QStringLiteral("/bar-foo\\ctrl- "));
    verifyCommandBarCompletionVisible();
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("foo_12"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("foo_12");

    // Be case insensitive.
    BeginTest(QStringLiteral("foo Fo12 fOo13 FO45"));
    TestPressKey(QStringLiteral("/fo\\ctrl- "));
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("Fo12") << QStringLiteral("FO45") << QStringLiteral("foo") << QStringLiteral("fOo13"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("foo Fo12 fOo13 FO45");

    // Feed the current word to complete to the completer as we type/ edit.
    BeginTest(QStringLiteral("foo fee foa foab"));
    TestPressKey(QStringLiteral("/xyz|f\\ctrl- o"));
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("foa") << QStringLiteral("foab") << QStringLiteral("foo"));
    TestPressKey(QStringLiteral("a"));
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("foa") << QStringLiteral("foab"));
    TestPressKey(QStringLiteral("\\ctrl-h"));
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("foa") << QStringLiteral("foab") << QStringLiteral("foo"));
    TestPressKey(QStringLiteral("o"));
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("foo fee foa foab");

    // Upon selecting a completion with an empty command bar, add the completed text to the command bar.
    BeginTest(QStringLiteral("foo fee fob foables"));
    TestPressKey(QStringLiteral("/\\ctrl- foa\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foables"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("foo fee fob foables");

    // If bar is non-empty, replace the word under the cursor.
    BeginTest(QStringLiteral("foo fee foa foab"));
    TestPressKey(QStringLiteral("/xyz|f$nose\\left\\left\\left\\left\\left\\ctrl- oa\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("xyz|foab$nose"));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("foo fee foa foab");

    // Place the cursor at the end of the completed text.
    BeginTest(QStringLiteral("foo fee foa foab"));
    TestPressKey(QStringLiteral("/xyz|f$nose\\left\\left\\left\\left\\left\\ctrl- oa\\ctrl-p\\enterX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("xyz|foabX$nose"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completion, then bar.
    FinishTest("foo fee foa foab");

    // If we're completing from history, though, the entire text gets set, and the completion prefix
    // is the beginning of the entire text, not the current word before the cursor.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("foo(bar"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo(b\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("foo(bar"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo(bar"));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("");

    // If we're completing from history and we abort the completion via ctrl-c or ctrl-[, we revert the whole
    // text to the last manually typed text.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("foo(b|ar"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/foo(b\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("foo(b|ar"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo(b|ar"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completion.
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo(b"));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("");

    // Scroll completion list if necessary so that currently selected completion is visible.
    BeginTest(QStringLiteral("a b c d e f g h i j k l m n o p q r s t u v w x y z"));
    TestPressKey(QStringLiteral("/\\ctrl- "));
    const int lastItemRow = 25;
    const QRect initialLastCompletionItemRect =
        emulatedCommandBarCompleter()->popup()->visualRect(emulatedCommandBarCompleter()->popup()->model()->index(lastItemRow, 0));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->rect().contains(
        initialLastCompletionItemRect)); // If this fails, then we have an error in the test setup: initially, the last item in the list should be outside of
                                         // the bounds of the popup.
    TestPressKey(QStringLiteral("\\ctrl-n"));
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("z"));
    const QRect lastCompletionItemRect =
        emulatedCommandBarCompleter()->popup()->visualRect(emulatedCommandBarCompleter()->popup()->model()->index(lastItemRow, 0));
    QVERIFY(emulatedCommandBarCompleter()->popup()->rect().contains(lastCompletionItemRect));
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("a b c d e f g h i j k l m n o p q r s t u v w x y z");

    // Ensure that the completion list changes size appropriately as the number of candidate completions changes.
    BeginTest(QStringLiteral("a ab abc"));
    TestPressKey(QStringLiteral("/\\ctrl- "));
    const int initialPopupHeight = emulatedCommandBarCompleter()->popup()->height();
    TestPressKey(QStringLiteral("ab"));
    const int popupHeightAfterEliminatingOne = emulatedCommandBarCompleter()->popup()->height();
    QVERIFY(popupHeightAfterEliminatingOne < initialPopupHeight);
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("a ab abc");

    // Ensure that the completion list disappears when no candidate completions are found, but re-appears
    // when some are found.
    BeginTest(QStringLiteral("a ab abc"));
    TestPressKey(QStringLiteral("/\\ctrl- "));
    TestPressKey(QStringLiteral("abd"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-h"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\enter\\enter")); // Dismiss completion, then bar.
    FinishTest("a ab abc");

    // ctrl-c and ctrl-[ when the completion list is visible should dismiss the completion list, but *not*
    // the emulated command bar. TODO - same goes for ESC, but this is harder as KateViewInternal dismisses it
    // itself.
    BeginTest(QStringLiteral("a ab abc"));
    TestPressKey(QStringLiteral("/\\ctrl- \\ctrl-cdiw"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    TestPressKey(QStringLiteral("/\\ctrl- \\ctrl-[diw"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    QVERIFY(emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("a ab abc");

    // If we implicitly choose an element from the summoned completion list (by highlighting it, then
    // continuing to edit the text), the completion box should not re-appear unless explicitly summoned
    // again, even if the current word has a valid completion.
    BeginTest(QStringLiteral("a ab abc"));
    TestPressKey(QStringLiteral("/\\ctrl- \\ctrl-p"));
    TestPressKey(QStringLiteral(".a"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("a ab abc");

    // If we dismiss the summoned completion list via ctrl-c or ctrl-[, it should not re-appear unless explicitly summoned
    // again, even if the current word has a valid completion.
    BeginTest(QStringLiteral("a ab abc"));
    TestPressKey(QStringLiteral("/\\ctrl- \\ctrl-c"));
    TestPressKey(QStringLiteral(".a"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    TestPressKey(QStringLiteral("/\\ctrl- \\ctrl-["));
    TestPressKey(QStringLiteral(".a"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("a ab abc");

    // If we select a completion from an empty bar, but then dismiss it via ctrl-c or ctrl-[, then we
    // should restore the empty text.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral("/\\ctrl- \\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    QVERIFY(emulatedCommandBar->isVisible());
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(""));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("foo");
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral("/\\ctrl- \\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("foo"));
    TestPressKey(QStringLiteral("\\ctrl-["));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    QVERIFY(emulatedCommandBar->isVisible());
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(""));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("foo");

    // If we select a completion but then dismiss it via ctrl-c or ctrl-[, then we
    // should restore the last manually typed word.
    BeginTest(QStringLiteral("fooabc"));
    TestPressKey(QStringLiteral("/f\\ctrl- o\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("fooabc"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    QVERIFY(emulatedCommandBar->isVisible());
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("fo"));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("fooabc");

    // If we select a completion but then dismiss it via ctrl-c or ctrl-[, then we
    // should restore the word currently being typed to the last manually typed word.
    BeginTest(QStringLiteral("fooabc"));
    TestPressKey(QStringLiteral("/ab\\ctrl- |fo\\ctrl-p"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("ab|fo"));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("fooabc");

    // Set the completion prefix for the search history completion as soon as it is shown.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("foo(bar"));
    vi_global->searchHistory()->append(QStringLiteral("xyz"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("/f\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("foo(bar"));
    TestPressKey(QStringLiteral("\\enter")); // Dismiss bar.
    FinishTest("");

    // Command Mode (:) tests.
    // ":" should summon the command bar, with ":" as the label.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":"));
    QVERIFY(emulatedCommandBar->isVisible());
    QCOMPARE(emulatedCommandTypeIndicator()->text(), QStringLiteral(":"));
    QVERIFY(emulatedCommandTypeIndicator()->isVisible());
    QVERIFY(emulatedCommandBarTextEdit());
    QVERIFY(emulatedCommandBarTextEdit()->text().isEmpty());
    TestPressKey(QStringLiteral("\\esc"));
    FinishTest("");

    // If we have a selection, it should be encoded as a range in the text edit.
    BeginTest(QStringLiteral("d\nb\na\nc"));
    TestPressKey(QStringLiteral("Vjjj:"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("'<,'>"));
    TestPressKey(QStringLiteral("\\esc"));
    FinishTest("d\nb\na\nc");

    // If we have a count, it should be encoded as a range in the text edit.
    BeginTest(QStringLiteral("d\nb\na\nc"));
    TestPressKey(QStringLiteral("7:"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(".,.+6"));
    TestPressKey(QStringLiteral("\\esc"));
    FinishTest("d\nb\na\nc");

    // Don't go doing an incremental search when we press keys!
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral(":bar"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());
    TestPressKey(QStringLiteral("\\esc"));
    FinishTest("foo bar xyz");

    // Execute the command on Enter.
    DoTest("d\nb\na\nc", "Vjjj:sort\\enter", "a\nb\nc\nd");

    // Don't crash if we call a non-existent command with a range.
    DoTest("123", ":42nonexistentcommand\\enter", "123");

    // Bar background should always be normal for command bar.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral("/foo\\enter:"));
    verifyTextEditBackgroundColour(normalBackgroundColour);
    TestPressKey(QStringLiteral("\\ctrl-c/bar\\enter:"));
    verifyTextEditBackgroundColour(normalBackgroundColour);
    TestPressKey(QStringLiteral("\\esc"));
    FinishTest("foo");

    const int commandResponseMessageTimeOutMSOverride = QString::fromLatin1(qgetenv("KATE_VIMODE_TEST_COMMANDRESPONSEMESSAGETIMEOUTMS")).toInt();
    const long commandResponseMessageTimeOutMS = (commandResponseMessageTimeOutMSOverride > 0) ? commandResponseMessageTimeOutMSOverride : 2000;
    {
        // If there is any output from the command, show it in a label for a short amount of time
        // (make sure the bar type indicator is hidden, here, as it looks messy).
        emulatedCommandBar->setCommandResponseMessageTimeout(commandResponseMessageTimeOutMS);
        BeginTest(QStringLiteral("foo bar xyz"));
        const QDateTime timeJustBeforeCommandExecuted = QDateTime::currentDateTime();
        TestPressKey(QStringLiteral(":commandthatdoesnotexist\\enter"));
        QVERIFY(emulatedCommandBar->isVisible());
        QVERIFY(commandResponseMessageDisplay());
        QVERIFY(commandResponseMessageDisplay()->isVisible());
        QVERIFY(!emulatedCommandBarTextEdit()->isVisible());
        QVERIFY(!emulatedCommandTypeIndicator()->isVisible());
        // Be a bit vague about the exact message, due to i18n, etc.
        QVERIFY(commandResponseMessageDisplay()->text().contains(QStringLiteral("commandthatdoesnotexist")));
        waitForEmulatedCommandBarToHide(4 * commandResponseMessageTimeOutMS);
        QVERIFY(timeJustBeforeCommandExecuted.msecsTo(QDateTime::currentDateTime())
                >= commandResponseMessageTimeOutMS - 500); // "- 500" because coarse timers can fire up to 500ms *prematurely*.
        QVERIFY(!emulatedCommandBar->isVisible());
        // Piggy-back on this test, as the bug we're about to test for would actually make setting
        // up the conditions again in a separate test impossible ;)
        // When we next summon the bar, the response message should be invisible; the editor visible & editable;
        // and the bar type indicator visible again.
        TestPressKey(QStringLiteral("/"));
        QVERIFY(!commandResponseMessageDisplay()->isVisible());
        QVERIFY(emulatedCommandBarTextEdit()->isVisible());
        QVERIFY(emulatedCommandBarTextEdit()->isEnabled());
        QVERIFY(emulatedCommandBar->isVisible());
        TestPressKey(QStringLiteral("\\esc")); // Dismiss the bar.
        FinishTest("foo bar xyz");
    }

    {
        // Show the same message twice in a row.
        BeginTest(QStringLiteral("foo bar xyz"));
        TestPressKey(QStringLiteral(":othercommandthatdoesnotexist\\enter"));
        QDateTime startWaitingForMessageToHide = QDateTime::currentDateTime();
        waitForEmulatedCommandBarToHide(4 * commandResponseMessageTimeOutMS);
        TestPressKey(QStringLiteral(":othercommandthatdoesnotexist\\enter"));
        QVERIFY(commandResponseMessageDisplay()->isVisible());
        // Wait for it to disappear again, as a courtesy for the next test.
        waitForEmulatedCommandBarToHide(4 * commandResponseMessageTimeOutMS);
    }

    {
        // Emulated command bar should not steal keypresses when it is merely showing the results of an executed command.
        BeginTest(QStringLiteral("foo bar"));
        TestPressKey(QStringLiteral(":commandthatdoesnotexist\\enterrX"));
        Q_ASSERT_X(commandResponseMessageDisplay()->isVisible(), "running test", "Need to increase timeJustBeforeCommandExecuted!");
        FinishTest("Xoo bar");
    }

    {
        // Don't send the synthetic "enter" keypress (for making search-as-a-motion work) when we finally hide.
        BeginTest(QStringLiteral("foo bar\nbar"));
        TestPressKey(QStringLiteral(":commandthatdoesnotexist\\enter"));
        Q_ASSERT_X(commandResponseMessageDisplay()->isVisible(), "running test", "Need to increase timeJustBeforeCommandExecuted!");
        waitForEmulatedCommandBarToHide(commandResponseMessageTimeOutMS * 4);
        TestPressKey(QStringLiteral("rX"));
        FinishTest("Xoo bar\nbar");
    }

    {
        // The timeout should be cancelled when we invoke the command bar again.
        BeginTest(QLatin1String(""));
        TestPressKey(QStringLiteral(":commandthatdoesnotexist\\enter"));
        TestPressKey(QStringLiteral(":"));
        // Wait ample time for the timeout to fire.  Do not use waitForEmulatedCommandBarToHide for this!
        QTRY_VERIFY_WITH_TIMEOUT(emulatedCommandBar->isVisible(), commandResponseMessageTimeOutMS * 2);
        TestPressKey(QStringLiteral("\\esc")); // Dismiss the bar.
        FinishTest("");
    }

    {
        // The timeout should not cause kate_view to regain focus if we have manually taken it away.
        qDebug() << " NOTE: this test is weirdly fragile, so if it starts failing, comment it out and e-mail me:  it may well be more trouble that it's worth.";
        BeginTest(QLatin1String(""));
        TestPressKey(QStringLiteral(":commandthatdoesnotexist\\enter"));
        // Wait for any focus changes to take effect.
        QApplication::processEvents();
        QLineEdit *dummyToFocus = new QLineEdit(QStringLiteral("Sausage"), mainWindow);
        // Take focus away from kate_view by giving it to dummyToFocus.
        QApplication::setActiveWindow(mainWindow);
        kate_view->setFocus();
        mainWindowLayout->addWidget(dummyToFocus);
        dummyToFocus->show();
        dummyToFocus->setEnabled(true);
        dummyToFocus->setFocus();
        // Allow dummyToFocus to receive focus.
        QTRY_VERIFY(dummyToFocus->hasFocus());
        // Wait ample time for the timeout to fire.  Do not use waitForEmulatedCommandBarToHide for this -
        // the bar never actually hides in this instance, and I think it would take some deep changes in
        // Kate to make it do so (the KateCommandLineBar as the same issue).
        QTest::qWait(commandResponseMessageTimeOutMS * 2);
        QVERIFY(dummyToFocus->hasFocus());
        QVERIFY(emulatedCommandBar->isVisible());
        mainWindowLayout->removeWidget(dummyToFocus);
        // Restore focus to the kate_view.
        kate_view->setFocus();
        QTRY_VERIFY(kate_view->hasFocus());
        // *Now* wait for the command bar to disappear - giving kate_view focus should trigger it.
        waitForEmulatedCommandBarToHide(commandResponseMessageTimeOutMS * 4);
        FinishTest("");
    }

    {
        // No completion should be shown when the bar is first shown: this gives us an opportunity
        // to invoke command history via ctrl-p and ctrl-n.
        BeginTest(QLatin1String(""));
        TestPressKey(QStringLiteral(":"));
        QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
        FinishTest("");
    }

    {
        // Should be able to switch to completion from document, even when we have a completion from commands.
        BeginTest(QStringLiteral("soggy1 soggy2"));
        TestPressKey(QStringLiteral(":so"));
        verifyCommandBarCompletionContains(QStringList() << QStringLiteral("sort"));
        TestPressKey(QStringLiteral("\\ctrl- "));
        verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("soggy1") << QStringLiteral("soggy2"));
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
        FinishTest("soggy1 soggy2");
    }

    {
        // If we dismiss the command completion then change the text, it should summon the completion
        // again.
        BeginTest(QLatin1String(""));
        TestPressKey(QStringLiteral(":so"));
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
        TestPressKey(QStringLiteral("r"));
        verifyCommandBarCompletionVisible();
        verifyCommandBarCompletionContains(QStringList() << QStringLiteral("sort"));
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
        FinishTest("");
    }

    {
        // Completion should be dismissed when we are showing command response text.
        BeginTest(QLatin1String(""));
        TestPressKey(QStringLiteral(":set-au\\enter"));
        QVERIFY(commandResponseMessageDisplay()->isVisible());
        QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
        waitForEmulatedCommandBarToHide(commandResponseMessageTimeOutMS * 4);
        FinishTest("");
    }

    // If we abort completion via ctrl-c or ctrl-[, we should revert the current word to the last
    // manually entered word.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":se\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    QVERIFY(emulatedCommandBarTextEdit()->text() != QStringLiteral("se"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("se"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");

    // In practice, it's annoying if, as we enter ":s/se", completions pop up after the "se":
    // for now, only summon completion if we are on the first word in the text.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/se"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":.,.+7s/se"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");

    // Don't blank the text if we activate command history completion with no command history.
    BeginTest(QLatin1String(""));
    clearCommandHistory();
    TestPressKey(QStringLiteral(":s/se\\ctrl-p"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/se"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");

    // On completion, only update the command in front of the cursor.
    BeginTest(QLatin1String(""));
    clearCommandHistory();
    TestPressKey(QStringLiteral(":.,.+6s/se\\left\\left\\leftet-auto-in\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(".,.+6set-auto-indent/se"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer.
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");

    // On completion, place the cursor after the new command.
    BeginTest(QLatin1String(""));
    clearCommandHistory();
    TestPressKey(QStringLiteral(":.,.+6s/fo\\left\\left\\leftet-auto-in\\ctrl-pX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(".,.+6set-auto-indentX/fo"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer.
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");

    // "The current word", for Commands, can contain "-".
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":set-\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    QVERIFY(emulatedCommandBarTextEdit()->text() != QLatin1String("set-"));
    QVERIFY(emulatedCommandBarCompleter()->currentCompletion().startsWith(QLatin1String("set-")));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), emulatedCommandBarCompleter()->currentCompletion());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completion.
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    {
        // Don't switch from word-from-document to command-completion just because we press a key, though!
        BeginTest(QStringLiteral("soggy1 soggy2"));
        TestPressKey(QStringLiteral(":\\ctrl- s"));
        TestPressKey(QStringLiteral("o"));
        verifyCommandBarCompletionVisible();
        verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("soggy1") << QStringLiteral("soggy2"));
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
        FinishTest("soggy1 soggy2");
    }

    {
        // If we're in a place where there is no command completion allowed, don't go hiding the word
        // completion as we type.
        BeginTest(QStringLiteral("soggy1 soggy2"));
        TestPressKey(QStringLiteral(":s/s\\ctrl- o"));
        verifyCommandBarCompletionVisible();
        verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("soggy1") << QStringLiteral("soggy2"));
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
        FinishTest("soggy1 soggy2");
    }

    {
        // Don't show command completion before we start typing a command: we want ctrl-p/n
        // to go through command history instead (we'll test for that second part later).
        BeginTest(QStringLiteral("soggy1 soggy2"));
        TestPressKey(QStringLiteral(":"));
        QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
        TestPressKey(QStringLiteral("\\ctrl-cvl:"));
        QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
        TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
        FinishTest("soggy1 soggy2");
    }

    {
        // Aborting ":" should leave us in normal mode with no selection.
        BeginTest(QStringLiteral("foo bar"));
        TestPressKey(QStringLiteral("vw:\\ctrl-["));
        QVERIFY(kate_view->selectionText().isEmpty());
        TestPressKey(QStringLiteral("wdiw"));
        BeginTest(QStringLiteral("foo "));
    }

    // Command history tests.
    clearCommandHistory();
    QVERIFY(commandHistory().isEmpty());
    vi_global->commandHistory()->append(QStringLiteral("foo"));
    vi_global->commandHistory()->append(QStringLiteral("bar"));
    QCOMPARE(commandHistory(), QStringList() << QStringLiteral("foo") << QStringLiteral("bar"));
    clearCommandHistory();
    QVERIFY(commandHistory().isEmpty());

    // If we add something to the history, remove any earliest occurrences (this is what Vim appears to do)
    // and append to the end.
    clearCommandHistory();
    vi_global->commandHistory()->append(QStringLiteral("bar"));
    vi_global->commandHistory()->append(QStringLiteral("xyz"));
    vi_global->commandHistory()->append(QStringLiteral("foo"));
    vi_global->commandHistory()->append(QStringLiteral("xyz"));
    QCOMPARE(commandHistory(), QStringList() << QStringLiteral("bar") << QStringLiteral("foo") << QStringLiteral("xyz"));

    // Push out older entries if we have too many command items in the history.
    clearCommandHistory();
    for (int i = 1; i <= HISTORY_SIZE_LIMIT; i++) {
        vi_global->commandHistory()->append(QStringLiteral("commandhistoryitem %1").arg(i));
    }
    QCOMPARE(commandHistory().size(), HISTORY_SIZE_LIMIT);
    QCOMPARE(commandHistory().constFirst(), QStringLiteral("commandhistoryitem 1"));
    QCOMPARE(commandHistory().last(), QStringLiteral("commandhistoryitem 100"));
    vi_global->commandHistory()->append(QStringLiteral("commandhistoryitem %1").arg(HISTORY_SIZE_LIMIT + 1));
    QCOMPARE(commandHistory().size(), HISTORY_SIZE_LIMIT);
    QCOMPARE(commandHistory().constFirst(), QStringLiteral("commandhistoryitem 2"));
    QCOMPARE(commandHistory().last(), QStringLiteral("commandhistoryitem %1").arg(HISTORY_SIZE_LIMIT + 1));

    // Don't add empty commands to the history.
    clearCommandHistory();
    DoTest("foo bar", ":\\enter", "foo bar");
    QVERIFY(commandHistory().isEmpty());

    clearCommandHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":sort\\enter"));
    QCOMPARE(commandHistory(), QStringList() << QStringLiteral("sort"));
    TestPressKey(QStringLiteral(":yank\\enter"));
    QCOMPARE(commandHistory(), QStringList() << QStringLiteral("sort") << QStringLiteral("yank"));
    // Add to history immediately: don't wait for the command response display to timeout.
    TestPressKey(QStringLiteral(":commandthatdoesnotexist\\enter"));
    QCOMPARE(commandHistory(), QStringList() << QStringLiteral("sort") << QStringLiteral("yank") << QStringLiteral("commandthatdoesnotexist"));
    // Vim adds aborted commands to the history too, oddly.
    TestPressKey(QStringLiteral(":abortedcommand\\ctrl-c"));
    QCOMPARE(commandHistory(),
             QStringList() << QStringLiteral("sort") << QStringLiteral("yank") << QStringLiteral("commandthatdoesnotexist")
                           << QStringLiteral("abortedcommand"));
    // Only add for commands, not searches!
    TestPressKey(QStringLiteral("/donotaddme\\enter?donotaddmeeither\\enter/donotaddme\\ctrl-c?donotaddmeeither\\ctrl-c"));
    QCOMPARE(commandHistory(),
             QStringList() << QStringLiteral("sort") << QStringLiteral("yank") << QStringLiteral("commandthatdoesnotexist")
                           << QStringLiteral("abortedcommand"));
    FinishTest("");

    // Commands should not be added to the search history!
    clearCommandHistory();
    clearSearchHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":sort\\enter"));
    QVERIFY(searchHistory().isEmpty());
    FinishTest("");

    // With an empty command bar, ctrl-p / ctrl-n should go through history.
    clearCommandHistory();
    vi_global->commandHistory()->append(QStringLiteral("command1"));
    vi_global->commandHistory()->append(QStringLiteral("command2"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("command2"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), emulatedCommandBarCompleter()->currentCompletion());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");
    clearCommandHistory();
    vi_global->commandHistory()->append(QStringLiteral("command1"));
    vi_global->commandHistory()->append(QStringLiteral("command2"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":\\ctrl-n"));
    verifyCommandBarCompletionVisible();
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("command1"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), emulatedCommandBarCompleter()->currentCompletion());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");

    // If we're at a place where command completions are not allowed, ctrl-p/n should go through history.
    clearCommandHistory();
    vi_global->commandHistory()->append(QStringLiteral("s/command1"));
    vi_global->commandHistory()->append(QStringLiteral("s/command2"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("s/command2"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), emulatedCommandBarCompleter()->currentCompletion());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");
    clearCommandHistory();
    vi_global->commandHistory()->append(QStringLiteral("s/command1"));
    vi_global->commandHistory()->append(QStringLiteral("s/command2"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/\\ctrl-n"));
    verifyCommandBarCompletionVisible();
    QCOMPARE(emulatedCommandBarCompleter()->currentCompletion(), QStringLiteral("s/command1"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), emulatedCommandBarCompleter()->currentCompletion());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("");

    // Cancelling word-from-document completion should revert the whole text to what it was before.
    BeginTest(QStringLiteral("sausage bacon"));
    TestPressKey(QStringLiteral(":s/b\\ctrl- \\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/bacon"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/b"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar
    FinishTest("sausage bacon");

    // "Replace" history tests.
    clearReplaceHistory();
    QVERIFY(replaceHistory().isEmpty());
    vi_global->replaceHistory()->append(QStringLiteral("foo"));
    vi_global->replaceHistory()->append(QStringLiteral("bar"));
    QCOMPARE(replaceHistory(), QStringList() << QStringLiteral("foo") << QStringLiteral("bar"));
    clearReplaceHistory();
    QVERIFY(replaceHistory().isEmpty());

    // If we add something to the history, remove any earliest occurrences (this is what Vim appears to do)
    // and append to the end.
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QStringLiteral("bar"));
    vi_global->replaceHistory()->append(QStringLiteral("xyz"));
    vi_global->replaceHistory()->append(QStringLiteral("foo"));
    vi_global->replaceHistory()->append(QStringLiteral("xyz"));
    QCOMPARE(replaceHistory(), QStringList() << QStringLiteral("bar") << QStringLiteral("foo") << QStringLiteral("xyz"));

    // Push out older entries if we have too many replace items in the history.
    clearReplaceHistory();
    for (int i = 1; i <= HISTORY_SIZE_LIMIT; i++) {
        vi_global->replaceHistory()->append(QStringLiteral("replacehistoryitem %1").arg(i));
    }
    QCOMPARE(replaceHistory().size(), HISTORY_SIZE_LIMIT);
    QCOMPARE(replaceHistory().constFirst(), QStringLiteral("replacehistoryitem 1"));
    QCOMPARE(replaceHistory().last(), QStringLiteral("replacehistoryitem 100"));
    vi_global->replaceHistory()->append(QStringLiteral("replacehistoryitem %1").arg(HISTORY_SIZE_LIMIT + 1));
    QCOMPARE(replaceHistory().size(), HISTORY_SIZE_LIMIT);
    QCOMPARE(replaceHistory().constFirst(), QStringLiteral("replacehistoryitem 2"));
    QCOMPARE(replaceHistory().last(), QStringLiteral("replacehistoryitem %1").arg(HISTORY_SIZE_LIMIT + 1));

    // Don't add empty replaces to the history.
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QLatin1String(""));
    QVERIFY(replaceHistory().isEmpty());

    // Some misc SedReplace tests.
    DoTest("x\\/y", ":s/\\\\//replace/g\\enter", "x\\replacey");
    DoTest("x\\/y", ":s/\\\\\\\\\\\\//replace/g\\enter", "xreplacey");
    DoTest("x\\/y", ":s:/:replace:g\\enter", "x\\replacey");
    DoTest("foo\nbar\nxyz", ":%delete\\enter", "");
    DoTest("foo\nbar\nxyz\nbaz", "jVj:delete\\enter", "foo\nbaz");
    DoTest("foo\nbar\nxyz\nbaz", "j2:delete\\enter", "foo\nbaz");

    // Test that 0 is accepted as a line index (and treated as 1) in a range specifier
    DoTest("bar\nbar\nbar", ":0,$s/bar/foo/g\\enter", "foo\nfoo\nfoo");
    DoTest("bar\nbar\nbar", ":1,$s/bar/foo/g\\enter", "foo\nfoo\nfoo");
    DoTest("bar\nbar\nbar", ":0,2s/bar/foo/g\\enter", "foo\nfoo\nbar");

    // On ctrl-d, delete the "search" term in a s/search/replace/xx
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":s/x\\\\\\\\\\\\/yz/rep\\\\\\\\\\\\/lace/g\\ctrl-d"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s//rep\\\\\\/lace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // Move cursor to position of deleted search term.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":s/x\\\\\\\\\\\\/yz/rep\\\\\\\\\\\\/lace/g\\ctrl-dX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/X/rep\\\\\\/lace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // Do nothing on ctrl-d in search mode.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("/s/search/replace/g\\ctrl-d"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/search/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c?s/searchbackwards/replace/g\\ctrl-d"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/searchbackwards/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // On ctrl-f, delete "replace" term in a s/search/replace/xx
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":s/a\\\\\\\\\\\\/bc/rep\\\\\\\\\\\\/lace/g\\ctrl-f"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/a\\\\\\/bc//g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // Move cursor to position of deleted replace term.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":s:a/bc:replace:g\\ctrl-fX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:a/bc:X:g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // Do nothing on ctrl-d in search mode.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("/s/search/replace/g\\ctrl-f"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/search/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c?s/searchbackwards/replace/g\\ctrl-f"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/searchbackwards/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // Do nothing on ctrl-d / ctrl-f if the current expression is not a sed expression.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":s/notasedreplaceexpression::gi\\ctrl-f\\ctrl-dX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/notasedreplaceexpression::giX"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // Need to convert Vim-style regex's to Qt one's in Sed Replace.
    DoTest("foo xbacba(boo)|[y", ":s/x[abc]\\\\+(boo)|[y/boo/g\\enter", "foo boo");
    DoTest("foo xbacba(boo)|[y\nfoo xbacba(boo)|[y", "Vj:s/x[abc]\\\\+(boo)|[y/boo/g\\enter", "foo boo\nfoo boo");
    // Just convert the search term, please :)
    DoTest("foo xbacba(boo)|[y", ":s/x[abc]\\\\+(boo)|[y/boo()/g\\enter", "foo boo()");
    // With an empty search expression, ctrl-d should still position the cursor correctly.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":s//replace/g\\ctrl-dX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/X/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":s::replace:g\\ctrl-dX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:X:replace:g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // With an empty replace expression, ctrl-f should still position the cursor correctly.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":s/sear\\\\/ch//g\\ctrl-fX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/sear\\/ch/X/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":s:sear\\\\:ch::g\\ctrl-fX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:sear\\:ch:X:g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // With both empty search *and* replace expressions, ctrl-f should still position the cursor correctly.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":s///g\\ctrl-fX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s//X/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":s:::g\\ctrl-fX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s::X:g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // Should be able to undo ctrl-f or ctrl-d.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":s/find/replace/g\\ctrl-d"));
    emulatedCommandBarTextEdit()->undo();
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/find/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-f"));
    emulatedCommandBarTextEdit()->undo();
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/find/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // ctrl-f / ctrl-d should cleanly finish sed find/ replace history completion.
    clearReplaceHistory();
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("searchxyz"));
    vi_global->replaceHistory()->append(QStringLiteral("replacexyz"));
    TestPressKey(QStringLiteral(":s///g\\ctrl-d\\ctrl-p"));
    QVERIFY(emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-f"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/searchxyz//g"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QVERIFY(emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-d"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s//replacexyz/g"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // Don't hang if we execute a sed replace with empty search term.
    DoTest("foo bar", ":s//replace/g\\enter", "foo bar");

    // ctrl-f & ctrl-d should work even when there is a range expression at the beginning of the sed replace.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral(":'<,'>s/search/replace/g\\ctrl-d"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("'<,'>s//replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c:.,.+6s/search/replace/g\\ctrl-f"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(".,.+6s/search//g"));
    TestPressKey(QStringLiteral("\\ctrl-c:%s/search/replace/g\\ctrl-f"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("%s/search//g"));
    // Place the cursor in the right place even when there is a range expression.
    TestPressKey(QStringLiteral("\\ctrl-c:.,.+6s/search/replace/g\\ctrl-fX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(".,.+6s/search/X/g"));
    TestPressKey(QStringLiteral("\\ctrl-c:%s/search/replace/g\\ctrl-fX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("%s/search/X/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("foo bar");
    // Don't crash on ctrl-f/d if we have an empty command.
    DoTest("", ":\\ctrl-f\\ctrl-d\\ctrl-c", "");
    // Parser regression test: Don't crash on ctrl-f/d with ".,.+".
    DoTest("", ":.,.+\\ctrl-f\\ctrl-d\\ctrl-c", "");

    // Command-completion should be invoked on the command being typed even when preceded by a range expression.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":0,'>so"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Command-completion should ignore the range expression.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":.,.+6so"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // A sed-replace should immediately add the search term to the search history.
    clearSearchHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/search/replace/g\\enter"));
    QCOMPARE(searchHistory(), QStringList() << QStringLiteral("search"));
    FinishTest("");

    // An aborted sed-replace should not add the search term to the search history.
    clearSearchHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/search/replace/g\\ctrl-c"));
    QCOMPARE(searchHistory(), QStringList());
    FinishTest("");

    // A non-sed-replace should leave the search history unchanged.
    clearSearchHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s,search/replace/g\\enter"));
    QCOMPARE(searchHistory(), QStringList());
    FinishTest("");

    // A sed-replace should immediately add the replace term to the replace history.
    clearReplaceHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/search/replace/g\\enter"));
    QCOMPARE(replaceHistory(), QStringList() << QStringLiteral("replace"));
    clearReplaceHistory();
    TestPressKey(QStringLiteral(":'<,'>s/search/replace1/g\\enter"));
    QCOMPARE(replaceHistory(), QStringList() << QStringLiteral("replace1"));
    FinishTest("");

    // An aborted sed-replace should not add the replace term to the replace history.
    clearReplaceHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/search/replace/g\\ctrl-c"));
    QCOMPARE(replaceHistory(), QStringList());
    FinishTest("");

    // A non-sed-replace should leave the replace history unchanged.
    clearReplaceHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s,search/replace/g\\enter"));
    QCOMPARE(replaceHistory(), QStringList());
    FinishTest("");

    // Misc tests for sed replace.  These are for the *generic* Kate sed replace; they should all
    // use EmulatedCommandBarTests' built-in command execution stuff (\\:<commandtoexecute>\\\) rather than
    // invoking a EmulatedCommandBar and potentially doing some Vim-specific transforms to
    // the command.
    DoTest("foo foo foo", "\\:s/foo/bar/\\", "bar foo foo");
    DoTest("foo foo xyz foo", "\\:s/foo/bar/g\\", "bar bar xyz bar");
    DoTest("foofooxyzfoo", "\\:s/foo/bar/g\\", "barbarxyzbar");
    DoTest("foofooxyzfoo", "\\:s/foo/b/g\\", "bbxyzb");
    DoTest("ffxyzf", "\\:s/f/b/g\\", "bbxyzb");
    DoTest("ffxyzf", "\\:s/f/bar/g\\", "barbarxyzbar");
    DoTest("foo Foo fOO FOO foo", "\\:s/foo/bar/\\", "bar Foo fOO FOO foo");
    DoTest("Foo foo fOO FOO foo", "\\:s/foo/bar/\\", "Foo bar fOO FOO foo");
    DoTest("Foo foo fOO FOO foo", "\\:s/foo/bar/g\\", "Foo bar fOO FOO bar");
    DoTest("foo Foo fOO FOO foo", "\\:s/foo/bar/i\\", "bar Foo fOO FOO foo");
    DoTest("Foo foo fOO FOO foo", "\\:s/foo/bar/i\\", "bar foo fOO FOO foo");
    DoTest("Foo foo fOO FOO foo", "\\:s/foo/bar/gi\\", "bar bar bar bar bar");
    DoTest("Foo foo fOO FOO foo", "\\:s/foo/bar/ig\\", "bar bar bar bar bar");
    // There are some oddities to do with how EmulatedCommandBarTest's "execute command directly" stuff works with selected ranges:
    // basically, we need to do our selection in Visual mode, then exit back to Normal mode before running the
    // command.
    DoTest("foo foo\nbar foo foo\nxyz foo foo\nfoo bar foo", "jVj\\esc\\:'<,'>s/foo/bar/\\", "foo foo\nbar bar foo\nxyz bar foo\nfoo bar foo");
    DoTest("foo foo\nbar foo foo\nxyz foo foo\nfoo bar foo", "jVj\\esc\\:'<,'>s/foo/bar/g\\", "foo foo\nbar bar bar\nxyz bar bar\nfoo bar foo");
    DoTest("Foo foo fOO FOO foo", "\\:s/foo/barfoo/g\\", "Foo barfoo fOO FOO barfoo");
    DoTest("Foo foo fOO FOO foo", "\\:s/foo/foobar/g\\", "Foo foobar fOO FOO foobar");
    DoTest("axyzb", "\\:s/a(.*)b/d\\\\1f/\\", "dxyzf");
    DoTest("ayxzzyxzfddeefdb", "\\:s/a([xyz]+)([def]+)b/<\\\\1|\\\\2>/\\", "<yxzzyxz|fddeefd>");
    DoTest("foo", "\\:s/.*//g\\", "");
    DoTest("foo", "\\:s/.*/f/g\\", "f");
    DoTest("foo/bar", "\\:s/foo\\\\/bar/123\\\\/xyz/g\\", "123/xyz");
    DoTest("foo:bar", "\\:s:foo\\\\:bar:123\\\\:xyz:g\\", "123:xyz");
    const bool oldReplaceTabsDyn = kate_document->config()->replaceTabsDyn();
    kate_document->config()->setReplaceTabsDyn(false);
    DoTest("foo\tbar", "\\:s/foo\\\\tbar/replace/g\\", "replace");
    DoTest("foo\tbar", "\\:s/foo\\\\tbar/rep\\\\tlace/g\\", "rep\tlace");
    kate_document->config()->setReplaceTabsDyn(oldReplaceTabsDyn);
    DoTest("foo", "\\:s/foo/replaceline1\\\\nreplaceline2/g\\", "replaceline1\nreplaceline2");
    DoTest("foofoo", "\\:s/foo/replaceline1\\\\nreplaceline2/g\\", "replaceline1\nreplaceline2replaceline1\nreplaceline2");
    DoTest("foofoo\nfoo", "\\:s/foo/replaceline1\\\\nreplaceline2/g\\", "replaceline1\nreplaceline2replaceline1\nreplaceline2\nfoo");
    DoTest("fooafoob\nfooc\nfood",
           "Vj\\esc\\:'<,'>s/foo/replaceline1\\\\nreplaceline2/g\\",
           "replaceline1\nreplaceline2areplaceline1\nreplaceline2b\nreplaceline1\nreplaceline2c\nfood");
    DoTest("fooafoob\nfooc\nfood",
           "Vj\\esc\\:'<,'>s/foo/replaceline1\\\\nreplaceline2/\\",
           "replaceline1\nreplaceline2afoob\nreplaceline1\nreplaceline2c\nfood");
    DoTest("fooafoob\nfooc\nfood",
           "Vj\\esc\\:'<,'>s/foo/replaceline1\\\\nreplaceline2\\\\nreplaceline3/g\\",
           "replaceline1\nreplaceline2\nreplaceline3areplaceline1\nreplaceline2\nreplaceline3b\nreplaceline1\nreplaceline2\nreplaceline3c\nfood");
    DoTest("foofoo", "\\:s/foo/replace\\\\nfoo/g\\", "replace\nfooreplace\nfoo");
    DoTest("foofoo", "\\:s/foo/replacefoo\\\\nfoo/g\\", "replacefoo\nfooreplacefoo\nfoo");
    DoTest("foofoo", "\\:s/foo/replacefoo\\\\n/g\\", "replacefoo\nreplacefoo\n");
    DoTest("ff", "\\:s/f/f\\\\nf/g\\", "f\nff\nf");
    DoTest("ff", "\\:s/f/f\\\\n/g\\", "f\nf\n");
    DoTest("foo\nbar", "\\:s/foo\\\\n//g\\", "bar");
    DoTest("foo\n\n\nbar", "\\:s/foo(\\\\n)*bar//g\\", "");
    DoTest("foo\n\n\nbar", "\\:s/foo(\\\\n*)bar/123\\\\1456/g\\", "123\n\n\n456");
    DoTest("xAbCy", "\\:s/x(.)(.)(.)y/\\\\L\\\\1\\\\U\\\\2\\\\3/g\\", "aBC");
    DoTest("foo", "\\:s/foo/\\\\a/g\\", "\x07");
    // End "generic" (i.e. not involving any Vi mode tricks/ transformations) sed replace tests: the remaining
    // ones should go via the EmulatedCommandBar.
    BeginTest(QStringLiteral("foo foo\nxyz\nfoo"));
    TestPressKey(QStringLiteral(":%s/foo/bar/g\\enter"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(3, 2);
    FinishTest("bar bar\nxyz\nbar");

    // ctrl-p on the first character of the search term in a sed-replace should
    // invoke search history completion.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("search"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/search/replace/g\\ctrl-b\\right\\right\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":'<,'>s/search/replace/g\\ctrl-b\\right\\right\\right\\right\\right\\right\\right\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // ctrl-p on the last character of the search term in a sed-replace should
    // invoke search history completion.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyz"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/xyz/replace/g\\ctrl-b\\right\\right\\right\\right\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    QVERIFY(!emulatedCommandBar->isVisible());
    TestPressKey(QStringLiteral(":'<,'>s/xyz/replace/g\\ctrl-b\\right\\right\\right\\right\\right\\right\\right\\right\\right\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // ctrl-p on some arbitrary character of the search term in a sed-replace should
    // invoke search history completion.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyzaaaaaa"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/xyzaaaaaa/replace/g\\ctrl-b\\right\\right\\right\\right\\right\\right\\right\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(
        QStringLiteral(":'<,'>s/xyzaaaaaa/replace/g\\ctrl-b\\right\\right\\right\\right\\right\\right\\right\\right\\right\\right\\right\\right\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // ctrl-p on some character *after" the search term should
    // *not* invoke search history completion.
    // Note: in s/xyz/replace/g, the "/" after the "z" is counted as part of the find term;
    // this allows us to do xyz<ctrl-p> and get completions.
    clearSearchHistory();
    clearCommandHistory();
    clearReplaceHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyz"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/xyz/replace/g\\ctrl-b\\right\\right\\right\\right\\right\\right\\ctrl-p"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    clearSearchHistory();
    clearCommandHistory();
    TestPressKey(QStringLiteral(":'<,'>s/xyz/replace/g\\ctrl-b\\right\\right\\right\\right\\right\\right\\right\\right\\right\\right\\right\\right\\ctrl-p"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.

    // Make sure it's the search history we're invoking.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyzaaaaaa"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s//replace/g\\ctrl-b\\right\\right\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("xyzaaaaaa"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":.,.+6s//replace/g\\ctrl-b\\right\\right\\right\\right\\right\\right\\right\\ctrl-p"));
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("xyzaaaaaa"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // (Search history should be reversed).
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyzaaaaaa"));
    vi_global->searchHistory()->append(QStringLiteral("abc"));
    vi_global->searchHistory()->append(QStringLiteral("def"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s//replace/g\\ctrl-b\\right\\right\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("def") << QStringLiteral("abc") << QStringLiteral("xyzaaaaaa"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Completion prefix is the current find term.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xy:zaaaaaa"));
    vi_global->searchHistory()->append(QStringLiteral("abc"));
    vi_global->searchHistory()->append(QStringLiteral("def"));
    vi_global->searchHistory()->append(QStringLiteral("xy:zbaaaaa"));
    vi_global->searchHistory()->append(QStringLiteral("xy:zcaaaaa"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s//replace/g\\ctrl-dxy:z\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("xy:zcaaaaa") << QStringLiteral("xy:zbaaaaa") << QStringLiteral("xy:zaaaaaa"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Replace entire search term with completion.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("ab,cd"));
    vi_global->searchHistory()->append(QStringLiteral("ab,xy"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s//replace/g\\ctrl-dab,\\ctrl-p\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/ab,cd/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":'<,'>s//replace/g\\ctrl-dab,\\ctrl-p\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("'<,'>s/ab,cd/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Place the cursor at the end of find term.
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("ab,xy"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s//replace/g\\ctrl-dab,\\ctrl-pX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/ab,xyX/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":.,.+7s//replace/g\\ctrl-dab,\\ctrl-pX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(".,.+7s/ab,xyX/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Leave find term unchanged if there is no search history.
    clearSearchHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/nose/replace/g\\ctrl-b\\right\\right\\right\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/nose/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Leave cursor position unchanged if there is no search history.
    clearSearchHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/nose/replace/g\\ctrl-b\\right\\right\\right\\ctrl-pX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/nXose/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // ctrl-p on the first character of the replace term in a sed-replace should
    // invoke replace history completion.
    clearSearchHistory();
    clearReplaceHistory();
    clearCommandHistory();
    vi_global->replaceHistory()->append(QStringLiteral("replace"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/search/replace/g\\left\\left\\left\\left\\left\\left\\left\\left\\left\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":'<,'>s/search/replace/g\\left\\left\\left\\left\\left\\left\\left\\left\\left\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // ctrl-p on the last character of the replace term in a sed-replace should
    // invoke replace history completion.
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QStringLiteral("replace"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/xyz/replace/g\\left\\left\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":'<,'>s/xyz/replace/g\\left\\left\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // ctrl-p on some arbitrary character of the search term in a sed-replace should
    // invoke search history completion.
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QStringLiteral("replaceaaaaaa"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/xyzaaaaaa/replace/g\\left\\left\\left\\left\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":'<,'>s/xyzaaaaaa/replace/g\\left\\left\\left\\left\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // ctrl-p on some character *after" the replace term should
    // *not* invoke replace history completion.
    // Note: in s/xyz/replace/g, the "/" after the "e" is counted as part of the replace term;
    // this allows us to do replace<ctrl-p> and get completions.
    clearSearchHistory();
    clearCommandHistory();
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QStringLiteral("xyz"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/xyz/replace/g\\left\\ctrl-p"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    clearSearchHistory();
    clearCommandHistory();
    TestPressKey(QStringLiteral(":'<,'>s/xyz/replace/g\\left\\ctrl-p"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.

    // (Replace history should be reversed).
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QStringLiteral("xyzaaaaaa"));
    vi_global->replaceHistory()->append(QStringLiteral("abc"));
    vi_global->replaceHistory()->append(QStringLiteral("def"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/search//g\\left\\left\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("def") << QStringLiteral("abc") << QStringLiteral("xyzaaaaaa"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Completion prefix is the current replace term.
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QStringLiteral("xy:zaaaaaa"));
    vi_global->replaceHistory()->append(QStringLiteral("abc"));
    vi_global->replaceHistory()->append(QStringLiteral("def"));
    vi_global->replaceHistory()->append(QStringLiteral("xy:zbaaaaa"));
    vi_global->replaceHistory()->append(QStringLiteral("xy:zcaaaaa"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":'<,'>s/replace/search/g\\ctrl-fxy:z\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    verifyCommandBarCompletionsMatches(QStringList() << QStringLiteral("xy:zcaaaaa") << QStringLiteral("xy:zbaaaaa") << QStringLiteral("xy:zaaaaaa"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Replace entire search term with completion.
    clearReplaceHistory();
    clearSearchHistory();
    vi_global->replaceHistory()->append(QStringLiteral("ab,cd"));
    vi_global->replaceHistory()->append(QStringLiteral("ab,xy"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/search//g\\ctrl-fab,\\ctrl-p\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/search/ab,cd/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":'<,'>s/search//g\\ctrl-fab,\\ctrl-p\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("'<,'>s/search/ab,cd/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Place the cursor at the end of replace term.
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QStringLiteral("ab,xy"));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/search//g\\ctrl-fab,\\ctrl-pX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/search/ab,xyX/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    TestPressKey(QStringLiteral(":.,.+7s/search//g\\ctrl-fab,\\ctrl-pX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral(".,.+7s/search/ab,xyX/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Leave replace term unchanged if there is no replace history.
    clearReplaceHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/nose/replace/g\\left\\left\\left\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/nose/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Leave cursor position unchanged if there is no replace history.
    clearSearchHistory();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":s/nose/replace/g\\left\\left\\left\\left\\ctrl-pX"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/nose/replaXce/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Invoke replacement history even when the "find" term is empty.
    BeginTest(QLatin1String(""));
    clearReplaceHistory();
    clearSearchHistory();
    vi_global->replaceHistory()->append(QStringLiteral("ab,xy"));
    vi_global->searchHistory()->append(QStringLiteral("whoops"));
    TestPressKey(QStringLiteral(":s///g\\ctrl-f\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s//ab,xy/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Move the cursor back to the last manual edit point when aborting completion.
    BeginTest(QLatin1String(""));
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("xyzaaaaa"));
    TestPressKey(QStringLiteral(":s/xyz/replace/g\\ctrl-b\\right\\right\\right\\right\\righta\\ctrl-p\\ctrl-[X"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/xyzaX/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Don't blank the "find" term if there is no search history that begins with the
    // current "find" term.
    BeginTest(QLatin1String(""));
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("doesnothavexyzasaprefix"));
    TestPressKey(QStringLiteral(":s//replace/g\\ctrl-dxyz\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/xyz/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Escape the delimiter if it occurs in a search history term - searching for it likely won't
    // work, but at least it won't crash!
    BeginTest(QLatin1String(""));
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("search"));
    vi_global->searchHistory()->append(QStringLiteral("aa/aa\\/a"));
    vi_global->searchHistory()->append(QStringLiteral("ss/ss"));
    TestPressKey(QStringLiteral(":s//replace/g\\ctrl-d\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/ss\\/ss/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/aa\\/aa\\/a/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/search/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    clearSearchHistory(); // Now do the same, but with a different delimiter.
    vi_global->searchHistory()->append(QStringLiteral("search"));
    vi_global->searchHistory()->append(QStringLiteral("aa:aa\\:a"));
    vi_global->searchHistory()->append(QStringLiteral("ss:ss"));
    TestPressKey(QStringLiteral(":s::replace:g\\ctrl-d\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:ss\\:ss:replace:g"));
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:aa\\:aa\\:a:replace:g"));
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:search:replace:g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Remove \C if occurs in search history.
    BeginTest(QLatin1String(""));
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("s\\Cear\\\\Cch"));
    TestPressKey(QStringLiteral(":s::replace:g\\ctrl-d\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:sear\\\\Cch:replace:g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Don't blank the "replace" term if there is no search history that begins with the
    // current "replace" term.
    BeginTest(QLatin1String(""));
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QStringLiteral("doesnothavexyzasaprefix"));
    TestPressKey(QStringLiteral(":s/search//g\\ctrl-fxyz\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/search/xyz/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Escape the delimiter if it occurs in a replace history term - searching for it likely won't
    // work, but at least it won't crash!
    BeginTest(QLatin1String(""));
    clearReplaceHistory();
    vi_global->replaceHistory()->append(QStringLiteral("replace"));
    vi_global->replaceHistory()->append(QStringLiteral("aa/aa\\/a"));
    vi_global->replaceHistory()->append(QStringLiteral("ss/ss"));
    TestPressKey(QStringLiteral(":s/search//g\\ctrl-f\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/search/ss\\/ss/g"));
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/search/aa\\/aa\\/a/g"));
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/search/replace/g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    clearReplaceHistory(); // Now do the same, but with a different delimiter.
    vi_global->replaceHistory()->append(QStringLiteral("replace"));
    vi_global->replaceHistory()->append(QStringLiteral("aa:aa\\:a"));
    vi_global->replaceHistory()->append(QStringLiteral("ss:ss"));
    TestPressKey(QStringLiteral(":s:search::g\\ctrl-f\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:search:ss\\:ss:g"));
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:search:aa\\:aa\\:a:g"));
    TestPressKey(QStringLiteral("\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s:search:replace:g"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // In search mode, don't blank current text on completion if there is no item in the search history which
    // has the current text as a prefix.
    BeginTest(QLatin1String(""));
    clearSearchHistory();
    vi_global->searchHistory()->append(QStringLiteral("doesnothavexyzasaprefix"));
    TestPressKey(QStringLiteral("/xyz\\ctrl-p"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("xyz"));
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Don't dismiss the command completion just because the cursor ends up *temporarily* at a place where
    // command completion is disallowed when cycling through completions.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral(":set/se\\left\\left\\left-\\ctrl-p"));
    verifyCommandBarCompletionVisible();
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss completer
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss bar.
    FinishTest("");

    // Don't expand mappings meant for Normal mode in the emulated command bar.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("foo"), QStringLiteral("xyz"), Mappings::NonRecursive);
    DoTest("bar foo xyz", "/foo\\enterrX", "bar Xoo xyz");
    clearAllMappings();

    // Incremental search and replace.
    QLabel *interactiveSedReplaceLabel = emulatedCommandBar->findChild<QLabel *>(QStringLiteral("interactivesedreplace"));
    QVERIFY(interactiveSedReplaceLabel);

    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    QVERIFY(interactiveSedReplaceLabel->isVisible());
    QVERIFY(!commandResponseMessageDisplay()->isVisible());
    QVERIFY(!emulatedCommandBarTextEdit()->isVisible());
    QVERIFY(!emulatedCommandTypeIndicator()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c")); // Dismiss search and replace.
    QVERIFY(!emulatedCommandBar->isVisible());
    FinishTest("foo");

    // Clear the flag that stops the command response from being shown after an incremental search and
    // replace, and also make sure that the edit and bar type indicator are not forcibly hidden.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter\\ctrl-c"));
    TestPressKey(QStringLiteral(":s/foo/bar/"));
    QVERIFY(emulatedCommandBarTextEdit()->isVisible());
    QVERIFY(emulatedCommandTypeIndicator()->isVisible());
    TestPressKey(QStringLiteral("\\enter"));
    QVERIFY(commandResponseMessageDisplay()->isVisible());
    FinishTest("bar");

    // Hide the incremental search and replace label when we show the bar.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter\\ctrl-c"));
    TestPressKey(QStringLiteral(":"));
    QVERIFY(!interactiveSedReplaceLabel->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");

    // The "c" marker can be anywhere in the three chars following the delimiter.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/cgi\\enter"));
    QVERIFY(interactiveSedReplaceLabel->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/igc\\enter"));
    QVERIFY(interactiveSedReplaceLabel->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/icg\\enter"));
    QVERIFY(interactiveSedReplaceLabel->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/ic\\enter"));
    QVERIFY(interactiveSedReplaceLabel->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/ci\\enter"));
    QVERIFY(interactiveSedReplaceLabel->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");

    // Emulated command bar is still active during an incremental search and replace.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    TestPressKey(QStringLiteral("idef\\esc"));
    FinishTest("foo");

    // Emulated command bar text is not edited during an incremental search and replace.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    TestPressKey(QStringLiteral("def"));
    QCOMPARE(emulatedCommandBarTextEdit()->text(), QStringLiteral("s/foo/bar/c"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");

    // Pressing "n" when there is only a single  change we can make aborts incremental search
    // and replace.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    TestPressKey(QStringLiteral("n"));
    QVERIFY(!interactiveSedReplaceLabel->isVisible());
    TestPressKey(QStringLiteral("ixyz\\esc"));
    FinishTest("xyzfoo");

    // Pressing "n" when there is only a single  change we can make aborts incremental search
    // and replace, and shows the no replacements on no lines.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    TestPressKey(QStringLiteral("n"));
    QVERIFY(commandResponseMessageDisplay()->isVisible());
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(0, 0);
    FinishTest("foo");

    // First possible match is highlighted when we start an incremental search and replace, and
    // cleared if we press 'n'.
    BeginTest(QStringLiteral(" xyz  123 foo bar"));
    TestPressKey(QStringLiteral(":s/foo/bar/gc\\enter"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().column(), 10);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().column(), 13);
    TestPressKey(QStringLiteral("n"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());
    FinishTest(" xyz  123 foo bar");

    // Second possible match highlighted if we start incremental search and replace and press 'n',
    // cleared if we press 'n' again.
    BeginTest(QStringLiteral(" xyz  123 foo foo bar"));
    TestPressKey(QStringLiteral(":s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("n"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().column(), 14);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().column(), 17);
    TestPressKey(QStringLiteral("n"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());
    FinishTest(" xyz  123 foo foo bar");

    // Perform replacement if we press 'y' on the first match.
    BeginTest(QStringLiteral(" xyz  foo 123 foo bar"));
    TestPressKey(QStringLiteral(":s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("y"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest(" xyz  bar 123 foo bar");

    // Replacement uses grouping, etc.
    BeginTest(QStringLiteral(" xyz  def 123 foo bar"));
    TestPressKey(QStringLiteral(":s/d\\\\(e\\\\)\\\\(f\\\\)/x\\\\1\\\\U\\\\2/gc\\enter"));
    TestPressKey(QStringLiteral("y"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest(" xyz  xeF 123 foo bar");

    // On replacement, highlight next match.
    BeginTest(QStringLiteral(" xyz  foo 123 foo bar"));
    TestPressKey(QStringLiteral(":s/foo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("y"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().column(), 14);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().column(), 17);
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest(" xyz  bar 123 foo bar");

    // On replacement, if there is no further match, abort incremental search and replace.
    BeginTest(QStringLiteral(" xyz  foo 123 foa bar"));
    TestPressKey(QStringLiteral(":s/foo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("y"));
    QVERIFY(commandResponseMessageDisplay()->isVisible());
    TestPressKey(QStringLiteral("ggidone\\esc"));
    FinishTest("done xyz  bar 123 foa bar");

    // After replacement, the next match is sought after the end of the replacement text.
    BeginTest(QStringLiteral("foofoo"));
    TestPressKey(QStringLiteral(":s/foo/barfoo/cg\\enter"));
    TestPressKey(QStringLiteral("y"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->start().column(), 6);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().line(), 0);
    QCOMPARE(rangesOnFirstLine().constFirst()->end().column(), 9);
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("barfoofoo");
    BeginTest(QStringLiteral("xffy"));
    TestPressKey(QStringLiteral(":s/f/bf/cg\\enter"));
    TestPressKey(QStringLiteral("yy"));
    FinishTest("xbfbfy");

    // Make sure the incremental search bar label contains the "instruction" keypresses.
    const QString interactiveSedReplaceShortcuts = QStringLiteral("(y/n/a/q/l)");
    BeginTest(QStringLiteral("foofoo"));
    TestPressKey(QStringLiteral(":s/foo/barfoo/cg\\enter"));
    QVERIFY(interactiveSedReplaceLabel->text().contains(interactiveSedReplaceShortcuts));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foofoo");

    // Make sure the incremental search bar label contains a reference to the text we're going to
    // replace with.
    // We're going to be a bit vague about the precise text due to localization issues.
    BeginTest(QStringLiteral("fabababbbar"));
    TestPressKey(QStringLiteral(":s/f\\\\([ab]\\\\+\\\\)/1\\\\U\\\\12/c\\enter"));
    QVERIFY(interactiveSedReplaceLabel->text().contains(QStringLiteral("1ABABABBBA2")));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("fabababbbar");

    // Replace newlines in the "replace?" message with "\\n"
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar\\\\nxyz\\\\n123/c\\enter"));
    QVERIFY(interactiveSedReplaceLabel->text().contains(QStringLiteral("bar\\nxyz\\n123")));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");

    // Update the "confirm replace?" message on pressing "y".
    BeginTest(QStringLiteral("fabababbbar fabbb"));
    TestPressKey(QStringLiteral(":s/f\\\\([ab]\\\\+\\\\)/1\\\\U\\\\12/gc\\enter"));
    TestPressKey(QStringLiteral("y"));
    QVERIFY(interactiveSedReplaceLabel->text().contains(QStringLiteral("1ABBB2")));
    QVERIFY(interactiveSedReplaceLabel->text().contains(interactiveSedReplaceShortcuts));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("1ABABABBBA2r fabbb");

    // Update the "confirm replace?" message on pressing "n".
    BeginTest(QStringLiteral("fabababbbar fabab"));
    TestPressKey(QStringLiteral(":s/f\\\\([ab]\\\\+\\\\)/1\\\\U\\\\12/gc\\enter"));
    TestPressKey(QStringLiteral("n"));
    QVERIFY(interactiveSedReplaceLabel->text().contains(QStringLiteral("1ABAB2")));
    QVERIFY(interactiveSedReplaceLabel->text().contains(interactiveSedReplaceShortcuts));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("fabababbbar fabab");

    // Cursor is placed at the beginning of first match.
    BeginTest(QStringLiteral("  foo foo foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    verifyCursorAt(Cursor(0, 2));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("  foo foo foo");

    // "y" and "n" update the cursor pos.
    BeginTest(QStringLiteral("  foo   foo foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("y"));
    verifyCursorAt(Cursor(0, 8));
    TestPressKey(QStringLiteral("n"));
    verifyCursorAt(Cursor(0, 12));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("  bar   foo foo");

    // If we end due to a "y" or "n" on the final match, leave the cursor at the beginning of the final match.
    BeginTest(QStringLiteral("  foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    TestPressKey(QStringLiteral("y"));
    verifyCursorAt(Cursor(0, 2));
    FinishTest("  bar");
    BeginTest(QStringLiteral("  foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    TestPressKey(QStringLiteral("n"));
    verifyCursorAt(Cursor(0, 2));
    FinishTest("  foo");

    // Respect ranges.
    BeginTest(QStringLiteral("foo foo\nfoo foo\nfoo foo\nfoo foo\n"));
    TestPressKey(QStringLiteral("jVj:s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("ynny"));
    QVERIFY(commandResponseMessageDisplay()->isVisible());
    TestPressKey(QStringLiteral("ggidone \\ctrl-c"));
    FinishTest("done foo foo\nbar foo\nfoo bar\nfoo foo\n");
    BeginTest(QStringLiteral("foo foo\nfoo foo\nfoo foo\nfoo foo\n"));
    TestPressKey(QStringLiteral("jVj:s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("nyyn"));
    QVERIFY(commandResponseMessageDisplay()->isVisible());
    TestPressKey(QStringLiteral("ggidone \\ctrl-c"));
    FinishTest("done foo foo\nfoo bar\nbar foo\nfoo foo\n");
    BeginTest(QStringLiteral("foo foo\nfoo foo\nfoo foo\nfoo foo\n"));
    TestPressKey(QStringLiteral("j:s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("ny"));
    QVERIFY(commandResponseMessageDisplay()->isVisible());
    TestPressKey(QStringLiteral("ggidone \\ctrl-c"));
    FinishTest("done foo foo\nfoo bar\nfoo foo\nfoo foo\n");
    BeginTest(QStringLiteral("foo foo\nfoo foo\nfoo foo\nfoo foo\n"));
    TestPressKey(QStringLiteral("j:s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("yn"));
    QVERIFY(commandResponseMessageDisplay()->isVisible());
    TestPressKey(QStringLiteral("ggidone \\ctrl-c"));
    FinishTest("done foo foo\nbar foo\nfoo foo\nfoo foo\n");

    // If no initial match can be found, abort and show a "no replacements" message.
    // The cursor position should remain unnchanged.
    BeginTest(QStringLiteral("fab"));
    TestPressKey(QStringLiteral("l:s/fee/bar/c\\enter"));
    QVERIFY(commandResponseMessageDisplay()->isVisible());
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(0, 0);
    QVERIFY(!interactiveSedReplaceLabel->isVisible());
    TestPressKey(QStringLiteral("rX"));
    BeginTest(QStringLiteral("fXb"));

    // Case-sensitive by default.
    BeginTest(QStringLiteral("foo Foo FOo foo foO"));
    TestPressKey(QStringLiteral(":s/foo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donebar Foo FOo bar foO");

    // Case-insensitive if "i" flag is used.
    BeginTest(QStringLiteral("foo Foo FOo foo foO"));
    TestPressKey(QStringLiteral(":s/foo/bar/icg\\enter"));
    TestPressKey(QStringLiteral("yyyyyggidone\\esc"));
    FinishTest("donebar bar bar bar bar");

    // Only one replacement per-line unless "g" flag is used.
    BeginTest(QStringLiteral("boo foo 123 foo\nxyz foo foo\nfoo foo foo\nxyz\nfoo foo\nfoo 123 foo"));
    TestPressKey(QStringLiteral("jVjjj:s/foo/bar/c\\enter"));
    TestPressKey(QStringLiteral("yynggidone\\esc"));
    FinishTest("doneboo foo 123 foo\nxyz bar foo\nbar foo foo\nxyz\nfoo foo\nfoo 123 foo");
    BeginTest(QStringLiteral("boo foo 123 foo\nxyz foo foo\nfoo foo foo\nxyz\nfoo foo\nfoo 123 foo"));
    TestPressKey(QStringLiteral("jVjjj:s/foo/bar/c\\enter"));
    TestPressKey(QStringLiteral("nnyggidone\\esc"));
    FinishTest("doneboo foo 123 foo\nxyz foo foo\nfoo foo foo\nxyz\nbar foo\nfoo 123 foo");

    // If replacement contains new lines, adjust the end line down.
    BeginTest(QStringLiteral("foo\nfoo1\nfoo2\nfoo3"));
    TestPressKey(QStringLiteral("jVj:s/foo/bar\\\\n/gc\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donefoo\nbar\n1\nbar\n2\nfoo3");
    BeginTest(QStringLiteral("foo\nfoo1\nfoo2\nfoo3"));
    TestPressKey(QStringLiteral("jVj:s/foo/bar\\\\nboo\\\\n/gc\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donefoo\nbar\nboo\n1\nbar\nboo\n2\nfoo3");

    // With "g" and a replacement that involves multiple lines, resume search from the end of the last line added.
    BeginTest(QStringLiteral("foofoo"));
    TestPressKey(QStringLiteral(":s/foo/bar\\\\n/gc\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donebar\nbar\n");
    BeginTest(QStringLiteral("foofoo"));
    TestPressKey(QStringLiteral(":s/foo/bar\\\\nxyz\\\\nfoo/gc\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donebar\nxyz\nfoobar\nxyz\nfoo");

    // Without "g" and with a replacement that involves multiple lines, resume search from the line after the line just added.
    BeginTest(QStringLiteral("foofoo1\nfoo2\nfoo3"));
    TestPressKey(QStringLiteral("Vj:s/foo/bar\\\\nxyz\\\\nfoo/c\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donebar\nxyz\nfoofoo1\nbar\nxyz\nfoo2\nfoo3");

    // Regression test: handle 'g' when it occurs before 'i' and 'c'.
    BeginTest(QStringLiteral("foo fOo"));
    TestPressKey(QStringLiteral(":s/foo/bar/gci\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donebar bar");

    // When the search terms swallows several lines, move the endline up accordingly.
    BeginTest(QStringLiteral("foo\nfoo1\nfoo\nfoo2\nfoo\nfoo3"));
    TestPressKey(QStringLiteral("V3j:s/foo\\\\nfoo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donebar1\nbar2\nfoo\nfoo3");
    BeginTest(QStringLiteral("foo\nfoo\nfoo1\nfoo\nfoo\nfoo2\nfoo\nfoo\nfoo3"));
    TestPressKey(QStringLiteral("V5j:s/foo\\\\nfoo\\\\nfoo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donebar1\nbar2\nfoo\nfoo\nfoo3");
    // Make sure we still adjust endline down if the replacement text has '\n's.
    BeginTest(QStringLiteral("foo\nfoo\nfoo1\nfoo\nfoo\nfoo2\nfoo\nfoo\nfoo3"));
    TestPressKey(QStringLiteral("V5j:s/foo\\\\nfoo\\\\nfoo/bar\\\\n/cg\\enter"));
    TestPressKey(QStringLiteral("yyggidone\\esc"));
    FinishTest("donebar\n1\nbar\n2\nfoo\nfoo\nfoo3");

    // Status reports.
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    TestPressKey(QStringLiteral("y"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(1, 1);
    FinishTest("bar");
    BeginTest(QStringLiteral("foo foo foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("yyy"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(3, 1);
    FinishTest("bar bar bar");
    BeginTest(QStringLiteral("foo foo foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("yny"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(2, 1);
    FinishTest("bar foo bar");
    BeginTest(QStringLiteral("foo\nfoo"));
    TestPressKey(QStringLiteral(":%s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("yy"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(2, 2);
    FinishTest("bar\nbar");
    BeginTest(QStringLiteral("foo foo\nfoo foo\nfoo foo"));
    TestPressKey(QStringLiteral(":%s/foo/bar/gc\\enter"));
    TestPressKey(QStringLiteral("yynnyy"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(4, 2);
    FinishTest("bar bar\nfoo foo\nbar bar");
    BeginTest(QStringLiteral("foofoo"));
    TestPressKey(QStringLiteral(":s/foo/bar\\\\nxyz/gc\\enter"));
    TestPressKey(QStringLiteral("yy"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(2, 1);
    FinishTest("bar\nxyzbar\nxyz");
    BeginTest(QStringLiteral("foofoofoo"));
    TestPressKey(QStringLiteral(":s/foo/bar\\\\nxyz\\\\nboo/gc\\enter"));
    TestPressKey(QStringLiteral("yyy"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(3, 1);
    FinishTest("bar\nxyz\nboobar\nxyz\nboobar\nxyz\nboo");
    // Tricky one: how many lines are "touched" if a single replacement
    // swallows multiple lines? I'm going to say the number of lines swallowed.
    BeginTest(QStringLiteral("foo\nfoo\nfoo"));
    TestPressKey(QStringLiteral(":s/foo\\\\nfoo\\\\nfoo/bar/c\\enter"));
    TestPressKey(QStringLiteral("y"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(1, 3);
    FinishTest("bar");
    BeginTest(QStringLiteral("foo\nfoo\nfoo\n"));
    TestPressKey(QStringLiteral(":s/foo\\\\nfoo\\\\nfoo\\\\n/bar/c\\enter"));
    TestPressKey(QStringLiteral("y"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(1, 4);
    FinishTest("bar");

    // "Undo" undoes last replacement.
    BeginTest(QStringLiteral("foo foo foo foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("nyynu"));
    FinishTest("foo bar foo foo");

    // "l" does the current replacement then exits.
    BeginTest(QStringLiteral("foo foo foo foo foo foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("nnl"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(1, 1);
    FinishTest("foo foo bar foo foo foo");

    // "q" just exits.
    BeginTest(QStringLiteral("foo foo foo foo foo foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("yyq"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(2, 1);
    FinishTest("bar bar foo foo foo foo");

    // "a" replaces all remaining, then exits.
    BeginTest(QStringLiteral("foo foo foo foo foo foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("nna"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(4, 1);
    FinishTest("foo foo bar bar bar bar");

    // The results of "a" can be undone in one go.
    BeginTest(QStringLiteral("foo foo foo foo foo foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/cg\\enter"));
    TestPressKey(QStringLiteral("ya"));
    verifyShowsNumberOfReplacementsAcrossNumberOfLines(6, 1);
    TestPressKey(QStringLiteral("u"));
    FinishTest("bar foo foo foo foo foo");
#if 0
    // XXX - as of Qt 5.5, simply replaying the correct QKeyEvents does *not* cause shortcuts
    // to be triggered, so these tests cannot pass.
    // It's possible that a solution involving QTestLib will be workable in the future, though.

  {
    // Test the test suite: ensure that shortcuts are still being sent and received correctly.
    // The test shortcut chosen should be one that does not conflict with built-in Kate ones.
    FailsIfSlotNotCalled failsIfActionNotTriggered;
    QAction *dummyAction = kate_view->actionCollection()->addAction("Woo");
    dummyAction->setShortcut(QKeySequence("Ctrl+]"));
    QVERIFY(connect(dummyAction, SIGNAL(triggered()), &failsIfActionNotTriggered, SLOT(slot())));
    DoTest("foo", "\\ctrl-]", "foo");
    // Processing shortcuts seems to require events to be processed.
    while (QApplication::hasPendingEvents())
    {
      QApplication::processEvents();
    }
    delete dummyAction;
  }
  {
    // Test that shortcuts involving ctrl+<digit> work correctly.
    FailsIfSlotNotCalled failsIfActionNotTriggered;
    QAction *dummyAction = kate_view->actionCollection()->addAction("Woo");
    dummyAction->setShortcut(QKeySequence("Ctrl+1"));
    QVERIFY(connect(dummyAction, SIGNAL(triggered()), &failsIfActionNotTriggered, SLOT(slot())));
    DoTest("foo", "\\ctrl-1", "foo");
    // Processing shortcuts seems to require events to be processed.
    while (QApplication::hasPendingEvents())
    {
      QApplication::processEvents();
    }
    delete dummyAction;
  }
  {
    // Test that shortcuts involving alt+<digit> work correctly.
    FailsIfSlotNotCalled failsIfActionNotTriggered;
    QAction *dummyAction = kate_view->actionCollection()->addAction("Woo");
    dummyAction->setShortcut(QKeySequence("Alt+1"));
    QVERIFY(connect(dummyAction, SIGNAL(triggered()), &failsIfActionNotTriggered, SLOT(slot())));
    DoTest("foo", "\\alt-1", "foo");
    // Processing shortcuts seems to require events to be processed.
    while (QApplication::hasPendingEvents())
    {
      QApplication::processEvents();
    }
    delete dummyAction;
  }
#endif

    // Find the "Print" action for later use.
    QAction *printAction = nullptr;
    const auto viewActions = kate_view->actionCollection()->actions();
    for (QAction *action : viewActions) {
        if (action->shortcut() == QKeySequence(QStringLiteral("Ctrl+p"))) {
            printAction = action;
            break;
        }
    }

    // Test that we don't inadvertantly trigger shortcuts in kate_view when typing them in the
    // emulated command bar.  Requires the above test for shortcuts to be sent and received correctly
    // to pass.
    {
        QVERIFY(mainWindow->isActiveWindow());
        QVERIFY(printAction);
        FailsIfSlotCalled failsIfActionTriggered(QStringLiteral("The kate_view shortcut should not be triggered by typing it in emulated  command bar!"));
        // Don't invoke Print on failure, as this hangs instead of failing.
        // disconnect(printAction, SIGNAL(triggered(bool)), kate_document, SLOT(print()));
        connect(printAction, &QAction::triggered, &failsIfActionTriggered, &FailsIfSlotCalled::slot);
        DoTest("foo bar foo bar", "/bar\\enterggd/\\ctrl-p\\enter.", "bar");
        // Processing shortcuts seems to require events to be processed.
        QApplication::processEvents();
    }

    // Test that the interactive search replace does not handle general keypresses like ctrl-p ("invoke
    // completion in emulated command bar").
    // Unfortunately, "ctrl-p" in kate_view, which is what will be triggered if this
    // test succeeds, hangs due to showing the print dialog, so we need to temporarily
    // block the Print action.
    clearCommandHistory();
    if (printAction) {
        printAction->blockSignals(true);
    }
    vi_global->commandHistory()->append(QStringLiteral("s/foo/bar/caa"));
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\ctrl-b\\enter\\ctrl-p"));
    QVERIFY(!emulatedCommandBarCompleter()->popup()->isVisible());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    if (printAction) {
        printAction->blockSignals(false);
    }
    FinishTest("foo");

    // The interactive sed replace command is added to the history straight away.
    clearCommandHistory();
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/foo/bar/c\\enter"));
    QCOMPARE(commandHistory(), QStringList() << QStringLiteral("s/foo/bar/c"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");
    clearCommandHistory();
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral(":s/notfound/bar/c\\enter"));
    QCOMPARE(commandHistory(), QStringList() << QStringLiteral("s/notfound/bar/c"));
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("foo");

    // Should be usable in mappings.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("H"), QStringLiteral(":s/foo/bar/gc<enter>nnyyl"), Mappings::Recursive);
    DoTest("foo foo foo foo foo foo", "H", "foo foo bar bar bar foo");
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("H"), QStringLiteral(":s/foo/bar/gc<enter>nna"), Mappings::Recursive);
    DoTest("foo foo foo foo foo foo", "H", "foo foo bar bar bar bar");
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("H"), QStringLiteral(":s/foo/bar/gc<enter>nnyqggidone<esc>"), Mappings::Recursive);
    DoTest("foo foo foo foo foo foo", "H", "donefoo foo bar foo foo foo");

#ifndef Q_OS_MACOS
    // Don't swallow "Ctrl+<key>" meant for the text edit.
    // On MacOS the QKeySequence for undo is defined as "Ctrl+Z" as well, however it's actually Cmd+Z...
    if (QKeySequence::keyBindings(QKeySequence::Undo).contains(QKeySequence(QStringLiteral("Ctrl+Z")))) {
        DoTest("foo bar", "/bar\\ctrl-z\\enterrX", "Xoo bar");
    } else {
        qWarning() << "Skipped test: Ctrl+Z is not Undo on this platform";
    }
#endif

    // Don't give invalid cursor position to updateCursor in Visual Mode: it will cause a crash!
    DoTest("xyz\nfoo\nbar\n123", "/foo\\\\nbar\\\\n\\enterggv//e\\enter\\ctrl-crX", "xyz\nfoo\nbaX\n123");
    DoTest("\nfooxyz\nbar;\n", "/foo.*\\\\n.*;\\enterggv//e\\enter\\ctrl-crX", "\nfooxyz\nbarX\n");
}

QCompleter *EmulatedCommandBarTest::emulatedCommandBarCompleter()
{
    return vi_input_mode->viModeEmulatedCommandBar()->findChild<QCompleter *>(QStringLiteral("completer"));
}

void EmulatedCommandBarTest::verifyCommandBarCompletionVisible()
{
    if (!emulatedCommandBarCompleter()->popup()->isVisible()) {
        qDebug() << "Emulated command bar completer not visible.";
        QStringListModel *completionModel = qobject_cast<QStringListModel *>(emulatedCommandBarCompleter()->model());
        Q_ASSERT(completionModel);
        const QStringList allAvailableCompletions = completionModel->stringList();
        qDebug() << " Completion list: " << allAvailableCompletions;
        qDebug() << " Completion prefix: " << emulatedCommandBarCompleter()->completionPrefix();
        bool candidateCompletionFound = false;
        for (const QString &availableCompletion : allAvailableCompletions) {
            if (availableCompletion.startsWith(emulatedCommandBarCompleter()->completionPrefix())) {
                candidateCompletionFound = true;
                break;
            }
        }
        if (candidateCompletionFound) {
            qDebug() << " The current completion prefix is a prefix of one of the available completions, so either complete() was not called, or the popup was "
                        "manually hidden since then";
        } else {
            qDebug() << " The current completion prefix is not a prefix of one of the available completions; this may or may not be why it is not visible";
        }
    }
    QVERIFY(emulatedCommandBarCompleter()->popup()->isVisible());
}

void EmulatedCommandBarTest::verifyCommandBarCompletionsMatches(const QStringList &expectedCompletionList)
{
    verifyCommandBarCompletionVisible();
    QStringList actualCompletionList;
    for (int i = 0; emulatedCommandBarCompleter()->setCurrentRow(i); i++) {
        actualCompletionList << emulatedCommandBarCompleter()->currentCompletion();
    }
    if (expectedCompletionList != actualCompletionList) {
        qDebug() << "Actual completions:\n " << actualCompletionList << "\n\ndo not match expected:\n" << expectedCompletionList;
    }

    QCOMPARE(actualCompletionList, expectedCompletionList);
}

void EmulatedCommandBarTest::verifyCommandBarCompletionContains(const QStringList &expectedCompletionList)
{
    verifyCommandBarCompletionVisible();
    QStringList actualCompletionList;

    for (int i = 0; emulatedCommandBarCompleter()->setCurrentRow(i); i++) {
        actualCompletionList << emulatedCommandBarCompleter()->currentCompletion();
    }

    for (const QString &expectedCompletion : expectedCompletionList) {
        if (!actualCompletionList.contains(expectedCompletion)) {
            qDebug() << "Whoops: " << actualCompletionList << " does not contain " << expectedCompletion;
        }
        QVERIFY(actualCompletionList.contains(expectedCompletion));
    }
}

QLabel *EmulatedCommandBarTest::emulatedCommandTypeIndicator()
{
    return emulatedCommandBar()->findChild<QLabel *>(QStringLiteral("bartypeindicator"));
}

void EmulatedCommandBarTest::verifyCursorAt(Cursor expectedCursorPos)
{
    QCOMPARE(kate_view->cursorPosition().line(), expectedCursorPos.line());
    QCOMPARE(kate_view->cursorPosition().column(), expectedCursorPos.column());
}

void EmulatedCommandBarTest::clearSearchHistory()
{
    vi_global->searchHistory()->clear();
}

QStringList EmulatedCommandBarTest::searchHistory()
{
    return vi_global->searchHistory()->items();
}

void EmulatedCommandBarTest::clearCommandHistory()
{
    vi_global->commandHistory()->clear();
}

QStringList EmulatedCommandBarTest::commandHistory()
{
    return vi_global->commandHistory()->items();
}

void EmulatedCommandBarTest::clearReplaceHistory()
{
    vi_global->replaceHistory()->clear();
}

QStringList EmulatedCommandBarTest::replaceHistory()
{
    return vi_global->replaceHistory()->items();
}

QList<Kate::TextRange *> EmulatedCommandBarTest::rangesOnFirstLine()
{
    return kate_document->buffer().rangesForLine(0, kate_view, true);
}

void EmulatedCommandBarTest::verifyTextEditBackgroundColour(const QColor &expectedBackgroundColour)
{
    QCOMPARE(emulatedCommandBarTextEdit()->palette().brush(QPalette::Base).color(), expectedBackgroundColour);
}

QLabel *EmulatedCommandBarTest::commandResponseMessageDisplay()
{
    QLabel *commandResponseMessageDisplay = emulatedCommandBar()->findChild<QLabel *>(QStringLiteral("commandresponsemessage"));
    Q_ASSERT(commandResponseMessageDisplay);
    return commandResponseMessageDisplay;
}

void EmulatedCommandBarTest::waitForEmulatedCommandBarToHide(long int timeout)
{
    QTRY_VERIFY_WITH_TIMEOUT(!emulatedCommandBar()->isVisible(), timeout);
}

void EmulatedCommandBarTest::verifyShowsNumberOfReplacementsAcrossNumberOfLines(int numReplacements, int acrossNumLines)
{
    QVERIFY(commandResponseMessageDisplay()->isVisible());
    QVERIFY(!emulatedCommandTypeIndicator()->isVisible());
    const QString commandMessageResponseText = commandResponseMessageDisplay()->text();
    const QString expectedNumReplacementsAsString = QString::number(numReplacements);
    const QString expectedAcrossNumLinesAsString = QString::number(acrossNumLines);
    // Be a bit vague about the actual contents due to e.g. localization.
    // TODO - see if we can insist that en_US is available on the Kate Jenkins server and
    // insist that we use it ... ?
    static const QRegularExpression numReplacementsMessageRegex(QStringLiteral("^.*(\\d+).*(\\d+).*$"));
    const auto match = numReplacementsMessageRegex.match(commandMessageResponseText);
    QVERIFY(match.hasMatch());
    const QString actualNumReplacementsAsString = match.captured(1);
    const QString actualAcrossNumLinesAsString = match.captured(2);
    QCOMPARE(actualNumReplacementsAsString, expectedNumReplacementsAsString);
    QCOMPARE(actualAcrossNumLinesAsString, expectedAcrossNumLinesAsString);
}

FailsIfSlotNotCalled::FailsIfSlotNotCalled()
    : QObject()
{
}

FailsIfSlotNotCalled::~FailsIfSlotNotCalled()
{
    QVERIFY(m_slotWasCalled);
}

void FailsIfSlotNotCalled::slot()
{
    m_slotWasCalled = true;
}

FailsIfSlotCalled::FailsIfSlotCalled(const QString &failureMessage)
    : QObject()
    , m_failureMessage(failureMessage)
{
}

void FailsIfSlotCalled::slot()
{
    QFAIL(qPrintable(m_failureMessage));
}

#include "moc_emulatedcommandbar.cpp"
