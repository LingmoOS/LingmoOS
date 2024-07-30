/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "keys.h"
#include "emulatedcommandbarsetupandteardown.h"
#include "fakecodecompletiontestmodel.h"
#include "vimode/globalstate.h"
#include "vimode/mappings.h"
#include <KLocalizedString>
#include <inputmode/kateviinputmode.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <kateview.h>
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>
#include <vimode/keymapper.h>
#include <vimode/keyparser.h>

#include <QMainWindow>
#include <QTest>

using namespace KTextEditor;
using KateVi::KeyParser;
using KateVi::Mappings;

QTEST_MAIN(KeysTest)

// BEGIN: KeysTest

void KeysTest::MappingTests()
{
    const int mappingTimeoutMSOverride = QString::fromLocal8Bit(qgetenv("KATE_VIMODE_TEST_MAPPINGTIMEOUTMS")).toInt();
    const int mappingTimeoutMS = (mappingTimeoutMSOverride > 0) ? mappingTimeoutMSOverride : 5;
    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, true); // For tests involving e.g. <c-a>
    {
        // Check storage and retrieval of mapping recursion.
        clearAllMappings();

        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'"), QStringLiteral("ihello"), Mappings::Recursive);
        QVERIFY(vi_global->mappings()->isRecursive(Mappings::NormalModeMapping, QStringLiteral("'")));

        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("ihello"), Mappings::NonRecursive);
        QVERIFY(!vi_global->mappings()->isRecursive(Mappings::NormalModeMapping, QStringLiteral("a")));
    }

    clearAllMappings();

    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'"), QStringLiteral("<esc>ihello<esc>^aworld<esc>"), Mappings::Recursive);
    DoTest("", "'", "hworldello");

    // Ensure that the non-mapping logged keypresses are cleared before we execute a mapping
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'a"), QStringLiteral("rO"), Mappings::Recursive);
    DoTest("X", "'a", "O");

    {
        // Check that '123 is mapped after the timeout, given that we also have mappings that
        // extend it (e.g. '1234, '12345, etc) and which it itself extends ('1, '12, etc).
        clearAllMappings();
        BeginTest(QString());
        vi_input_mode_manager->keyMapper()->setMappingTimeout(mappingTimeoutMS);

        QString consectiveDigits;
        for (int i = 1; i < 9; i++) {
            consectiveDigits += QString::number(i);
            vi_global->mappings()->add(Mappings::NormalModeMapping,
                                       QLatin1Char('\'') + consectiveDigits,
                                       QStringLiteral("iMapped from ") + consectiveDigits + QStringLiteral("<esc>"),
                                       Mappings::Recursive);
        }
        TestPressKey(QStringLiteral("'123"));
        QCOMPARE(kate_document->text(), QString()); // Shouldn't add anything until after the timeout!
        QTRY_COMPARE_WITH_TIMEOUT(kate_document->text(), QLatin1String("Mapped from 123"), 2 * mappingTimeoutMS);
        FinishTest("Mapped from 123");
    }

    // Mappings are not "counted": any count entered applies to the first command/ motion in the mapped sequence,
    // and is not used to replay the entire mapped sequence <count> times in a row.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'downmapping"), QStringLiteral("j"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'testmapping"), QStringLiteral("ifoo<esc>ibar<esc>"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'testmotionmapping"), QStringLiteral("lj"), Mappings::Recursive);
    DoTest("AAAA\nXXXX\nXXXX\nXXXX\nXXXX\nBBBB\nCCCC\nDDDD", "jd3'downmapping", "AAAA\nBBBB\nCCCC\nDDDD");
    DoTest("", "5'testmapping", "foofoofoofoofobaro");
    DoTest("XXXX\nXXXX\nXXXX\nXXXX", "3'testmotionmappingrO", "XXXX\nXXXO\nXXXX\nXXXX");

    // Regression test for a weird mistake I made: *completely* remove all counting for the
    // first command in the sequence; don't just set it to 1! If it is set to 1, then "%"
    // will mean "go to position 1 percent of the way through the document" rather than
    // go to matching item.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("gl"), QStringLiteral("%"), Mappings::Recursive);
    DoTest("0\n1\n2\n3\n4\n5\nfoo bar(xyz) baz", "jjjjjjwdgl", "0\n1\n2\n3\n4\n5\nfoo  baz");

    // Test that countable mappings work even when triggered by timeouts.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'testmapping"), QStringLiteral("ljrO"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'testmappingdummy"), QStringLiteral("dummy"), Mappings::Recursive);
    BeginTest(QStringLiteral("XXXX\nXXXX\nXXXX\nXXXX"));
    vi_input_mode_manager->keyMapper()->setMappingTimeout(mappingTimeoutMS);

    TestPressKey(QStringLiteral("3'testmapping"));
    QTRY_COMPARE_WITH_TIMEOUT(kate_document->text(), QLatin1String("XXXX\nXXXO\nXXXX\nXXXX"), 2 * mappingTimeoutMS);
    FinishTest("XXXX\nXXXO\nXXXX\nXXXX");

    // Test that telescoping mappings don't interfere with built-in commands. Assumes that gp
    // is implemented and working.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("gdummy"), QStringLiteral("idummy"), Mappings::Recursive);
    DoTest("hello", "yiwgpx", "hhellollo");

    // Test that we can map a sequence of keys that extends a built-in command and use
    // that sequence without the built-in command firing.
    // Once again, assumes that gp is implemented and working.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("gpa"), QStringLiteral("idummy"), Mappings::Recursive);
    DoTest("hello", "yiwgpa", "dummyhello");

    // Test that we can map a sequence of keys that extends a built-in command and still
    // have the original built-in command fire if we timeout after entering that command.
    // Once again, assumes that gp is implemented and working.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("gpa"), QStringLiteral("idummy"), Mappings::Recursive);
    BeginTest(QStringLiteral("hello"));
    vi_input_mode_manager->keyMapper()->setMappingTimeout(mappingTimeoutMS);

    TestPressKey(QStringLiteral("yiwgp"));
    QTest::qWait(2 * mappingTimeoutMS);
    TestPressKey(QStringLiteral("x"));
    FinishTest("hhellollo");

    // Test that something that starts off as a partial mapping following a command
    // (the "g" in the first "dg" is a partial mapping of "gj"), when extended to something
    // that is definitely not a mapping ("gg"), results in the full command being executed ("dgg").
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("gj"), QStringLiteral("aj"), Mappings::Recursive);
    DoTest("foo\nbar\nxyz", "jjdgg", "");

    // Make sure that a mapped sequence of commands is merged into a single undo-able edit.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'a"), QStringLiteral("ofoo<esc>ofoo<esc>ofoo<esc>"), Mappings::Recursive);
    DoTest("bar", "'au", "bar");

    // Make sure that a counted mapping is merged into a single undoable edit.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'a"), QStringLiteral("ofoo<esc>"), Mappings::Recursive);
    DoTest("bar", "5'au", "bar");

    // Some test setup for non-recursive mapping g -> gj (cf: bug:314415)
    // Firstly: work out the expected result of gj (this might be fragile as default settings
    // change, etc.).  We use BeginTest & FinishTest for the setup and teardown etc, but this is
    // not an actual test - it's just computing the expected result of the real test!
    const QString multiVirtualLineText = QStringLiteral(
        "foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo "
        "foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo "
        "foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo");
    ensureKateViewVisible(); // Needs to be visible in order for virtual lines to make sense.
    KateViewConfig::global()->setDynWordWrap(true);
    BeginTest(multiVirtualLineText);
    TestPressKey(QStringLiteral("gjrX"));
    const QString expectedAfterVirtualLineDownAndChange = kate_document->text();
    Q_ASSERT_X(expectedAfterVirtualLineDownAndChange.contains(QStringLiteral("X")) && !expectedAfterVirtualLineDownAndChange.startsWith(QLatin1Char('X')),
               "setting up j->gj testcase data",
               "gj doesn't seem to have worked correctly!");
    FinishTest(expectedAfterVirtualLineDownAndChange.toUtf8().constData());

    // Test that non-recursive mappings are not expanded.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("j"), QStringLiteral("gj"), Mappings::NonRecursive);
    DoTest(multiVirtualLineText.toUtf8().constData(), "jrX", expectedAfterVirtualLineDownAndChange.toUtf8().constData());
    KateViewConfig::global()->setDynWordWrap(false);

    // Test that recursive mappings are expanded.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("X"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("X"), QStringLiteral("rx"), Mappings::Recursive);
    DoTest("foo", "la", "fxo");

    // Test that the flag that stops mappings being expanded is reset after the mapping has been executed.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("j"), QStringLiteral("gj"), Mappings::NonRecursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("X"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("X"), QStringLiteral("rx"), Mappings::Recursive);
    DoTest("foo", "jla", "fxo");

    // Even if we start with a recursive mapping, as soon as we hit one that is not recursive, we should stop
    // expanding.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("X"), Mappings::NonRecursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("X"), QStringLiteral("r."), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("i"), QStringLiteral("a"), Mappings::Recursive);
    DoTest("foo", "li", "oo");

    // Regression test: Using a mapping may trigger a call to updateSelection(), which can change the mode
    // from VisualLineMode to plain VisualMode.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("gA"), QStringLiteral("%"), Mappings::NonRecursive);
    DoTest("xyz\nfoo\n{\nbar\n}", "jVjgAdgglP", "foo\n{\nbar\n}\nxyz");
    // Piggy back on the previous test with a regression test for issue where, if gA is mapped to %, vgly
    // will yank one more character than it should.
    DoTest("foo(bar)X", "vgAyp", "ffoo(bar)oo(bar)X");
    // Make sure that a successful mapping does not break the "if we select stuff externally in Normal mode,
    // we should switch to Visual Mode" thing.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("gA"), QStringLiteral("%"), Mappings::NonRecursive);
    BeginTest(QStringLiteral("foo bar xyz()"));
    TestPressKey(QStringLiteral("gAr."));
    kate_view->setSelection(Range(0, 1, 0, 4)); // Actually selects "oo " (i.e. without the "b").
    TestPressKey(QStringLiteral("d"));
    FinishTest("fbar xyz(.");

    // Regression tests for BUG:260655
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("f"), Mappings::NonRecursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("d"), QStringLiteral("i"), Mappings::NonRecursive);
    DoTest("foo dar", "adr.", "foo .ar");
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("F"), Mappings::NonRecursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("d"), QStringLiteral("i"), Mappings::NonRecursive);
    DoTest("foo dar", "$adr.", "foo .ar");
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("t"), Mappings::NonRecursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("d"), QStringLiteral("i"), Mappings::NonRecursive);
    DoTest("foo dar", "adr.", "foo.dar");
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("T"), Mappings::NonRecursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("d"), QStringLiteral("i"), Mappings::NonRecursive);
    DoTest("foo dar", "$adr.", "foo d.r");
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("r"), Mappings::NonRecursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("d"), QStringLiteral("i"), Mappings::NonRecursive);
    DoTest("foo dar", "ad", "doo dar");
    // Feel free to map the keypress after that, though.
    DoTest("foo dar", "addber\\esc", "berdoo dar");
    // Also, be careful about how we interpret "waiting for find char/ replace char"
    DoTest("foo dar", "ffas", "soo dar");

    // Ignore raw "Ctrl", "Shift", "Meta" and "Alt" keys, which will almost certainly end up being pressed as
    // we try to trigger mappings that contain these keys.
    clearAllMappings();
    {
        // Ctrl.
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("<c-a><c-b>"), QStringLiteral("ictrl<esc>"), Mappings::NonRecursive);
        BeginTest(QLatin1String(""));
        QKeyEvent *ctrlKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Control, Qt::NoModifier);
        QApplication::postEvent(kate_view->focusProxy(), ctrlKeyDown);
        QApplication::sendPostedEvents();
        TestPressKey(QStringLiteral("\\ctrl-a"));
        ctrlKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Control, Qt::NoModifier);
        QApplication::postEvent(kate_view->focusProxy(), ctrlKeyDown);
        QApplication::sendPostedEvents();
        TestPressKey(QStringLiteral("\\ctrl-b"));
        FinishTest("ctrl");
    }
    {
        // Shift.
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("<c-a>C"), QStringLiteral("ishift<esc>"), Mappings::NonRecursive);
        BeginTest(QLatin1String(""));
        QKeyEvent *ctrlKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Control, Qt::NoModifier);
        QApplication::postEvent(kate_view->focusProxy(), ctrlKeyDown);
        QApplication::sendPostedEvents();
        TestPressKey(QStringLiteral("\\ctrl-a"));
        QKeyEvent *shiftKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier);
        QApplication::postEvent(kate_view->focusProxy(), shiftKeyDown);
        QApplication::sendPostedEvents();
        TestPressKey(QStringLiteral("C"));
        FinishTest("shift");
    }
    {
        // Alt.
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("<c-a><a-b>"), QStringLiteral("ialt<esc>"), Mappings::NonRecursive);
        BeginTest(QLatin1String(""));
        QKeyEvent *ctrlKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Control, Qt::NoModifier);
        QApplication::postEvent(kate_view->focusProxy(), ctrlKeyDown);
        QApplication::sendPostedEvents();
        TestPressKey(QStringLiteral("\\ctrl-a"));
        QKeyEvent *altKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Alt, Qt::NoModifier);
        QApplication::postEvent(kate_view->focusProxy(), altKeyDown);
        QApplication::sendPostedEvents();
        TestPressKey(QStringLiteral("\\alt-b"));
        FinishTest("alt");
    }
    {
        // Meta.
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("<c-a><m-b>"), QStringLiteral("imeta<esc>"), Mappings::NonRecursive);
        BeginTest(QLatin1String(""));
        QKeyEvent *ctrlKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Control, Qt::NoModifier);
        QApplication::postEvent(kate_view->focusProxy(), ctrlKeyDown);
        QApplication::sendPostedEvents();
        TestPressKey(QStringLiteral("\\ctrl-a"));
        QKeyEvent *metaKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Meta, Qt::NoModifier);
        QApplication::postEvent(kate_view->focusProxy(), metaKeyDown);
        QApplication::sendPostedEvents();
        TestPressKey(QStringLiteral("\\meta-b"));
        FinishTest("meta");
    }
    {
        // Can have mappings in Visual mode, distinct from Normal mode..
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("a"), QStringLiteral("3l"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("inose<esc>"), Mappings::NonRecursive);
        DoTest("0123456", "lvad", "056");

        // The recursion in Visual Mode is distinct from that of  Normal mode.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("b"), QStringLiteral("<esc>"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("a"), QStringLiteral("b"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("b"), Mappings::Recursive);
        DoTest("XXX\nXXX", "lvajd", "XXX");
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("b"), QStringLiteral("<esc>"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("a"), QStringLiteral("b"), Mappings::Recursive);
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("b"), Mappings::NonRecursive);
        DoTest("XXX\nXXX", "lvajd", "XXX\nXXX");

        // A Visual mode mapping applies to all Visual modes (line, block, etc).
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("a"), QStringLiteral("2j"), Mappings::NonRecursive);
        DoTest("123\n456\n789", "lvad", "19");
        DoTest("123\n456\n789", "l\\ctrl-vad", "13\n46\n79");
        DoTest("123\n456\n789", "lVad", "");
        // Same for recursion.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("b"), QStringLiteral("2j"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("a"), QStringLiteral("b"), Mappings::Recursive);
        DoTest("123\n456\n789", "lvad", "19");
        DoTest("123\n456\n789", "l\\ctrl-vad", "13\n46\n79");
        DoTest("123\n456\n789", "lVad", "");

        // Can clear Visual mode mappings.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("h"), QStringLiteral("l"), Mappings::Recursive);
        vi_global->mappings()->clear(Mappings::VisualModeMapping);
        DoTest("123\n456\n789", "lvhd", "3\n456\n789");
        DoTest("123\n456\n789", "l\\ctrl-vhd", "3\n456\n789");
        DoTest("123\n456\n789", "lVhd", "456\n789");
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("h"), QStringLiteral("l"), Mappings::Recursive);
        vi_global->mappings()->clear(Mappings::VisualModeMapping);
        DoTest("123\n456\n789", "lvhd", "3\n456\n789");
        DoTest("123\n456\n789", "l\\ctrl-vhd", "3\n456\n789");
        DoTest("123\n456\n789", "lVhd", "456\n789");
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("h"), QStringLiteral("l"), Mappings::Recursive);
        vi_global->mappings()->clear(Mappings::VisualModeMapping);
        DoTest("123\n456\n789", "lvhd", "3\n456\n789");
        DoTest("123\n456\n789", "l\\ctrl-vhd", "3\n456\n789");
        DoTest("123\n456\n789", "lVhd", "456\n789");
    }
    {
        // Can have mappings in Insert mode.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("a"), QStringLiteral("xyz"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("inose<esc>"), Mappings::NonRecursive);
        DoTest("foo", "ia\\esc", "xyzfoo");

        // Recursion for Insert mode.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("b"), QStringLiteral("c"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("a"), QStringLiteral("b"), Mappings::NonRecursive);
        DoTest("", "ia\\esc", "b");
        clearAllMappings();
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("b"), QStringLiteral("c"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("a"), QStringLiteral("b"), Mappings::Recursive);
        DoTest("", "ia\\esc", "c");

        clearAllMappings();
        // Clear mappings for Insert mode.
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("a"), QStringLiteral("b"), Mappings::NonRecursive);
        vi_global->mappings()->clear(Mappings::InsertModeMapping);
        DoTest("", "ia\\esc", "a");
    }

    {
        EmulatedCommandBarSetUpAndTearDown vimStyleCommandBarTestsSetUpAndTearDown(vi_input_mode, kate_view, mainWindow);
        // Can have mappings in Emulated Command Bar.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::CommandModeMapping, QStringLiteral("a"), QStringLiteral("xyz"), Mappings::NonRecursive);
        DoTest(" a xyz", "/a\\enterrX", " a Xyz");
        // Use mappings from Normal mode as soon as we exit command bar via Enter.
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("a"), QStringLiteral("ixyz<c-c>"), Mappings::NonRecursive);
        DoTest(" a xyz", "/a\\entera", " a xyzxyz");
        // Multiple mappings.
        vi_global->mappings()->add(Mappings::CommandModeMapping, QStringLiteral("b"), QStringLiteral("123"), Mappings::NonRecursive);
        DoTest("  xyz123", "/ab\\enterrX", "  Xyz123");
        // Recursive mappings.
        vi_global->mappings()->add(Mappings::CommandModeMapping, QStringLiteral("b"), QStringLiteral("a"), Mappings::Recursive);
        DoTest("  xyz", "/b\\enterrX", "  Xyz");
        // Can clear all.
        vi_global->mappings()->clear(Mappings::CommandModeMapping);
        DoTest("  ab xyz xyz123", "/ab\\enterrX", "  Xb xyz xyz123");
    }

    // Test that not *both* of the mapping and the mapped keys are logged for repetition via "."
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("ixyz"), QStringLiteral("iabc"), Mappings::NonRecursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("gl"), QStringLiteral("%"), Mappings::NonRecursive);
    DoTest("", "ixyz\\esc.", "ababcc");
    DoTest("foo()X\nbarxyz()Y", "cglbaz\\escggj.", "bazX\nbazY");

    // Regression test for a crash when executing a mapping that switches to Normal mode.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("h"), QStringLiteral("d"), Mappings::Recursive);
    DoTest("foo", "vlh", "o");

    {
        // Test that we can set/ unset mappings from the command-line.
        clearAllMappings();
        DoTest("", "\\:nn foo ibar<esc>\\foo", "bar");

        // "nn" is not recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("l"), QStringLiteral("iabc<esc>"), Mappings::NonRecursive);
        DoTest("xxx", "\\:nn foo l\\foorX", "xXx");

        // "no" is not recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("l"), QStringLiteral("iabc<esc>"), Mappings::NonRecursive);
        DoTest("xxx", "\\:no foo l\\foorX", "xXx");

        // "noremap" is not recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("l"), QStringLiteral("iabc<esc>"), Mappings::NonRecursive);
        DoTest("xxx", "\\:noremap foo l\\foorX", "xXx");

        // "nm" is recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("l"), QStringLiteral("iabc<esc>"), Mappings::NonRecursive);
        DoTest("xxx", "\\:nm foo l\\foorX", "abXxxx");

        // "nmap" is recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("l"), QStringLiteral("iabc<esc>"), Mappings::NonRecursive);
        DoTest("xxx", "\\:nmap foo l\\foorX", "abXxxx");

        // Unfortunately, "map" is a reserved word :/
        clearAllMappings();
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("l"), QStringLiteral("iabc<esc>"), Mappings::NonRecursive);
        DoTest("xxx", "\\:map foo l\\foorX", "abXxxx", ShouldFail, "'map' is reserved for other stuff in Kate command line");

        // nunmap works in normal mode.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("w"), QStringLiteral("ciwabc<esc>"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("b"), QStringLiteral("ciwxyz<esc>"), Mappings::NonRecursive);
        DoTest(" 123 456 789", "\\:nunmap b\\WWwbrX", " 123 Xbc 789");

        // nmap and nunmap whose "from" is a complex encoded expression.
        clearAllMappings();
        BeginTest(QStringLiteral("123"));
        TestPressKey(QStringLiteral("\\:nmap <c-9> ciwxyz<esc>\\"));
        TestPressKey(QStringLiteral("\\ctrl-9"));
        FinishTest("xyz");
        BeginTest(QStringLiteral("123"));
        TestPressKey(QStringLiteral("\\:nunmap <c-9>\\"));
        TestPressKey(QStringLiteral("\\ctrl-9"));
        FinishTest("123");

        // vmap works in Visual mode and is recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
        DoTest("abco", "\\:vmap foo l\\v\\rightfoogU", "co");

        // vmap does not work in Normal mode.
        clearAllMappings();
        DoTest("xxx", "\\:vmap foo l\\foorX", "xxx\nrX");

        // vm works in Visual mode and is recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
        DoTest("abco", "\\:vm foo l\\v\\rightfoogU", "co");

        // vn works in Visual mode and is not recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
        DoTest("abco", "\\:vn foo l\\v\\rightfoogU", "ABCo");

        // vnoremap works in Visual mode and is not recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
        DoTest("abco", "\\:vnoremap foo l\\v\\rightfoogU", "ABCo");

        // vunmap works in Visual Mode.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("l"), QStringLiteral("w"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::VisualModeMapping, QStringLiteral("gU"), QStringLiteral("2b"), Mappings::NonRecursive);
        DoTest("foo bar xyz", "\\:vunmap gU\\wvlgUd", "foo BAR Xyz");

        // imap works in Insert mode and is recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
        DoTest("", "\\:imap foo l\\ifoo\\esc", "d");

        // im works in Insert mode and is recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
        DoTest("", "\\:im foo l\\ifoo\\esc", "d");

        // ino works in Insert mode and is not recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
        DoTest("", "\\:ino foo l\\ifoo\\esc", "l");

        // inoremap works in Insert mode and is not recursive.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
        DoTest("", "\\:inoremap foo l\\ifoo\\esc", "l");

        // iunmap works in Insert mode.
        clearAllMappings();
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
        vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("m"), QStringLiteral("e"), Mappings::NonRecursive);
        DoTest("", "\\:iunmap l\\ilm\\esc", "le");

        {
            EmulatedCommandBarSetUpAndTearDown vimStyleCommandBarTestsSetUpAndTearDown(vi_input_mode, kate_view, mainWindow);
            // cmap works in emulated command bar and is recursive.
            // NOTE: need to do the cmap call using the direct execution (i.e. \\:cmap blah blah\\), *not* using
            // the emulated command bar (:cmap blah blah\\enter), as this will be subject to mappings, which
            // can interfere with the tests!
            clearAllMappings();
            vi_global->mappings()->add(Mappings::CommandModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
            DoTest(" l d foo", "\\:cmap foo l\\/foo\\enterrX", " l X foo");

            // cm works in emulated command bar and is recursive.
            clearAllMappings();
            vi_global->mappings()->add(Mappings::CommandModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
            DoTest(" l d foo", "\\:cm foo l\\/foo\\enterrX", " l X foo");

            // cnoremap works in emulated command bar and is recursive.
            clearAllMappings();
            vi_global->mappings()->add(Mappings::CommandModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
            DoTest(" l d foo", "\\:cnoremap foo l\\/foo\\enterrX", " X d foo");

            // cno works in emulated command bar and is recursive.
            clearAllMappings();
            vi_global->mappings()->add(Mappings::CommandModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
            DoTest(" l d foo", "\\:cno foo l\\/foo\\enterrX", " X d foo");

            // cunmap works in emulated command bar.
            clearAllMappings();
            vi_global->mappings()->add(Mappings::CommandModeMapping, QStringLiteral("l"), QStringLiteral("d"), Mappings::NonRecursive);
            vi_global->mappings()->add(Mappings::CommandModeMapping, QStringLiteral("m"), QStringLiteral("e"), Mappings::NonRecursive);
            DoTest(" de le", "\\:cunmap l\\/lm\\enterrX", " de Xe");
        }

        // Can use <space> to signify a space.
        clearAllMappings();
        DoTest("", "\\:nn h<space> i<space>a<space>b<esc>\\h ", " a b");
    }

    // More recursion tests - don't lose characters from a Recursive mapping if it looks like they might
    // be part of a different mapping (but end up not being so).
    // (Here, the leading "i" in "irecursive<c-c>" could be part of the mapping "ihello<c-c>").
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'"), QStringLiteral("ihello<c-c>"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("ihello<c-c>"), QStringLiteral("irecursive<c-c>"), Mappings::Recursive);
    DoTest("", "'", "recursive");

    // Capslock in insert mode is not handled by Vim nor by KateViewInternal, and ends up
    // being sent to KateViInputModeManager::handleKeypress twice (it could be argued that this is
    // incorrect behaviour on the part of KateViewInternal), which can cause infinite
    // recursion if we are not careful about identifying replayed rejected keypresses.
    BeginTest(QStringLiteral("foo bar"));
    TestPressKey(QStringLiteral("i"));
    QKeyEvent *capslockKeyPress = new QKeyEvent(QEvent::KeyPress, Qt::Key_CapsLock, Qt::NoModifier);
    QApplication::postEvent(kate_view->focusProxy(), capslockKeyPress);
    QApplication::sendPostedEvents();
    FinishTest("foo bar");

    // Mapping the u and the U commands to other keys.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("t"), QStringLiteral("u"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("r"), QStringLiteral("U"), Mappings::Recursive);
    DoTest("", "ihello\\esct", "");
    DoTest("", "ihello\\esctr", "hello");

    // <nop>
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("l"), QStringLiteral("<nop>"), Mappings::Recursive);
    DoTest("Hello", "lrr", "rello");
    clearAllMappings();
    vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("l"), QStringLiteral("<nop>"), Mappings::Recursive);
    DoTest("Hello", "sl\\esc", "ello");
    clearAllMappings();
    vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("l"), QStringLiteral("<nop>abc"), Mappings::Recursive);
    DoTest("Hello", "sl\\esc", "abcello");

    // Clear mappings for subsequent tests.
    clearAllMappings();

    {
        // Test that g<up> and g<down> work as gk and gj (BUG: 418486)
        ensureKateViewVisible(); // Needs to be visible in order for virtual lines to make sense.
        KateViewConfig::global()->setDynWordWrap(true);

        BeginTest(multiVirtualLineText);
        TestPressKey(QStringLiteral("gjrX"));
        const QString expectedAfterVirtualLineDownAndChange = kate_document->text();
        QVERIFY(expectedAfterVirtualLineDownAndChange.contains(QLatin1String("X")) && !expectedAfterVirtualLineDownAndChange.startsWith(QStringLiteral("X")));
        FinishTest(expectedAfterVirtualLineDownAndChange.toUtf8().constData());

        BeginTest(multiVirtualLineText);
        TestPressKey(QStringLiteral("g\\downrX"));
        const QString expectedAfterVirtualLineDownAndChangeCursor = kate_document->text();
        QVERIFY(expectedAfterVirtualLineDownAndChangeCursor == expectedAfterVirtualLineDownAndChange);
        FinishTest(expectedAfterVirtualLineDownAndChangeCursor.toUtf8().constData());

        BeginTest(multiVirtualLineText);
        TestPressKey(QStringLiteral("gkrX"));
        const QString expectedAfterVirtualLineUpAndChange = kate_document->text();
        QVERIFY(expectedAfterVirtualLineUpAndChange.contains(QLatin1String("X")) && !expectedAfterVirtualLineUpAndChange.endsWith(QLatin1Char('X')));
        FinishTest(expectedAfterVirtualLineUpAndChange.toUtf8().constData());

        BeginTest(multiVirtualLineText);
        TestPressKey(QLatin1String("g\\uprX"));
        const QString expectedAfterVirtualLineUpAndChangeCursor = kate_document->text();
        QVERIFY(expectedAfterVirtualLineUpAndChangeCursor == expectedAfterVirtualLineUpAndChange);
        FinishTest(expectedAfterVirtualLineUpAndChangeCursor.toUtf8().constData());

        KateViewConfig::global()->setDynWordWrap(false);
    }
}

void KeysTest::LeaderTests()
{
    // Clean slate.
    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, true);
    clearAllMappings();

    // By default the backslash character is the leader. The default leader
    // is picked from the config. If we don't want to mess this from other
    // tests, it's better if we mock the config.
    const QString viTestKConfigFileName = QStringLiteral("vimodetest-leader-katevimoderc");
    KConfig viTestKConfig(viTestKConfigFileName);
    vi_global->mappings()->setLeader(QChar());
    vi_global->readConfig(&viTestKConfig);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("<leader>i"), QStringLiteral("ii"), Mappings::Recursive);
    DoTest("", "\\\\i", "i");

    // We can change the leader and it will work.
    clearAllMappings();
    vi_global->readConfig(&viTestKConfig);
    vi_global->mappings()->setLeader(QChar::fromLatin1(','));
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("<leader>i"), QStringLiteral("ii"), Mappings::Recursive);
    DoTest("", ",i", "i");

    // Mixing up the <leader> with its value.
    clearAllMappings();
    vi_global->readConfig(&viTestKConfig);
    vi_global->mappings()->setLeader(QChar::fromLatin1(','));
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("<leader>,"), QStringLiteral("ii"), Mappings::Recursive);
    DoTest("", ",,", "i");
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral(",<leader>"), QStringLiteral("ii"), Mappings::Recursive);
    DoTest("", ",,", "i");

    // It doesn't work outside normal mode.
    clearAllMappings();
    vi_global->readConfig(&viTestKConfig);
    vi_global->mappings()->setLeader(QChar::fromLatin1(','));
    vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("<leader>i"), QStringLiteral("ii"), Mappings::Recursive);
    DoTest("", "i,ii", ",ii");

    // Clear mappings for subsequent tests.
    clearAllMappings();
}

void KeysTest::ParsingTests()
{
    // BUG #298726
    const QChar char_o_diaeresis(246);

    // Test that we can correctly translate finnish key ö
    QKeyEvent k(QEvent::KeyPress, 214, Qt::NoModifier, 47, 246, 16400, char_o_diaeresis);
    QCOMPARE(KeyParser::self()->KeyEventToQChar(k), QChar(246));

    // Test that it can be used in mappings
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, char_o_diaeresis, QStringLiteral("ifoo"), Mappings::Recursive);
    DoTest2(__LINE__, __FILE__, QStringLiteral("hello"), QStringLiteral("ll%1bar").arg(char_o_diaeresis), QStringLiteral("hefoobarllo"));

    // Test that <cr> is parsed like <enter>
    QCOMPARE(KeyParser::self()->vi2qt(QStringLiteral("cr")), int(Qt::Key_Enter));
    const QString &enter = KeyParser::self()->encodeKeySequence(QStringLiteral("<cr>"));
    QCOMPARE(KeyParser::self()->decodeKeySequence(enter), QLatin1String("<cr>"));
}

void KeysTest::AltGr()
{
    QKeyEvent *altGrDown;
    QKeyEvent *altGrUp;
    // Test Alt-gr still works - this isn't quite how things work in "real-life": in real-life, something like
    // Alt-gr+7 would be a "{", but I don't think this can be reproduced without sending raw X11
    // keypresses to Qt, so just duplicate the keypress events we would receive if we pressed
    // Alt-gr+7 (that is: Alt-gr down; "{"; Alt-gr up).

    // Ensure we have auto brackets off, or the test will fail
    kate_view->config()->setValue(KateViewConfig::AutoBrackets, false);

    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("i"));
    altGrDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_AltGr, Qt::NoModifier);
    QApplication::postEvent(kate_view->focusProxy(), altGrDown);
    QApplication::sendPostedEvents();
    // Not really Alt-gr and 7, but this is the key event that is reported by Qt if we press that.
    QKeyEvent *altGrAnd7 = new QKeyEvent(QEvent::KeyPress, Qt::Key_BraceLeft, Qt::GroupSwitchModifier, QStringLiteral("{"));
    QApplication::postEvent(kate_view->focusProxy(), altGrAnd7);
    QApplication::sendPostedEvents();
    altGrUp = new QKeyEvent(QEvent::KeyRelease, Qt::Key_AltGr, Qt::NoModifier);
    QApplication::postEvent(kate_view->focusProxy(), altGrUp);
    QApplication::sendPostedEvents();
    TestPressKey(QStringLiteral("\\ctrl-c"));
    FinishTest("{");

    // French Bepo keyabord AltGr + Shift + s = Ù = Unicode(0x00D9);
    const QString ugrave = QString(QChar(0x00D9));
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("i"));
    altGrDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_AltGr, Qt::NoModifier);
    QApplication::postEvent(kate_view->focusProxy(), altGrDown);
    QApplication::sendPostedEvents();
    altGrDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::ShiftModifier | Qt::GroupSwitchModifier);
    QApplication::postEvent(kate_view->focusProxy(), altGrDown);
    QApplication::sendPostedEvents();
    QKeyEvent *altGrAndUGrave = new QKeyEvent(QEvent::KeyPress, Qt::Key_Ugrave, Qt::ShiftModifier | Qt::GroupSwitchModifier, ugrave);
    qDebug() << QStringLiteral("%1").arg(altGrAndUGrave->modifiers(), 10, 16);
    QApplication::postEvent(kate_view->focusProxy(), altGrAndUGrave);
    QApplication::sendPostedEvents();
    altGrUp = new QKeyEvent(QEvent::KeyRelease, Qt::Key_AltGr, Qt::NoModifier);
    QApplication::postEvent(kate_view->focusProxy(), altGrUp);
    QApplication::sendPostedEvents();
    FinishTest(ugrave.toUtf8().constData());
}

void KeysTest::MacroTests()
{
    // Update the status on qa.
    const QString macroIsRecordingStatus = u"(" + i18n("recording") + u")";
    clearAllMacros();
    BeginTest(QLatin1String(""));
    QVERIFY(!kate_view->viewModeHuman().contains(macroIsRecordingStatus));
    TestPressKey(QStringLiteral("qa"));
    QVERIFY(kate_view->viewModeHuman().contains(macroIsRecordingStatus));
    TestPressKey(QStringLiteral("q"));
    QVERIFY(!kate_view->viewModeHuman().contains(macroIsRecordingStatus));
    FinishTest("");

    // The closing "q" is not treated as the beginning of a new "begin recording macro" command.
    clearAllMacros();
    BeginTest(QStringLiteral("foo"));
    TestPressKey(QStringLiteral("qaqa"));
    QVERIFY(!kate_view->viewModeHuman().contains(macroIsRecordingStatus));
    TestPressKey(QStringLiteral("xyz\\esc"));
    FinishTest("fxyzoo");

    // Record and playback a single keypress into macro register "a".
    clearAllMacros();
    DoTest("foo bar", "qawqgg@arX", "foo Xar");
    // Two macros - make sure the old one is cleared.
    clearAllMacros();
    DoTest("123 foo bar xyz", "qawqqabqggww@arX", "123 Xoo bar xyz");

    // Update the status on qb.
    clearAllMacros();
    BeginTest(QLatin1String(""));
    QVERIFY(!kate_view->viewModeHuman().contains(macroIsRecordingStatus));
    TestPressKey(QStringLiteral("qb"));
    QVERIFY(kate_view->viewModeHuman().contains(macroIsRecordingStatus));
    TestPressKey(QStringLiteral("q"));
    QVERIFY(!kate_view->viewModeHuman().contains(macroIsRecordingStatus));
    FinishTest("");

    // Record and playback a single keypress into macro register "b".
    clearAllMacros();
    DoTest("foo bar", "qbwqgg@brX", "foo Xar");

    // More complex macros.
    clearAllMacros();
    DoTest("foo", "qcrXql@c", "XXo");

    // Re-recording a macro should only clear that macro.
    clearAllMacros();
    DoTest("foo 123", "qaraqqbrbqqbrBqw@a", "Boo a23");

    // Empty macro clears it.
    clearAllMacros();
    DoTest("", "qaixyz\\ctrl-cqqaq@a", "xyz");

    // Hold two macros in memory simultanenously so both can be played.
    clearAllMacros();
    DoTest("foo 123", "qaraqqbrbqw@al@b", "boo ab3");

    // Do more complex things, including switching modes and using ctrl codes.
    clearAllMacros();
    DoTest("foo bar", "qainose\\ctrl-c~qw@a", "nosEfoo nosEbar");
    clearAllMacros();
    DoTest("foo bar", "qayiwinose\\ctrl-r0\\ctrl-c~qw@a", "nosefoOfoo nosebaRbar");
    clearAllMacros();
    DoTest("foo bar", "qavldqw@a", "o r");
    // Make sure we can use "q" in insert mode while recording a macro.
    clearAllMacros();
    DoTest("foo bar", "qaiqueequeg\\ctrl-cqw@a", "queequegfoo queequegbar");
    // Can invoke a macro in Visual Mode.
    clearAllMacros();
    DoTest("foo bar", "qa~qvlll@a", "FOO Bar");
    // Invoking a macro in Visual Mode does not exit Visual Mode.
    clearAllMacros();
    DoTest("foo bar", "qallqggv@a~", "FOO bar");
    ;
    // Can record & macros in Visual Mode for playback in Normal Mode.
    clearAllMacros();
    DoTest("foo bar", "vqblq\\ctrl-c@b~", "foO bar");
    // Recording a macro in Visual Mode does not exit Visual Mode.
    clearAllMacros();
    DoTest("foo bar", "vqblql~", "FOO bar");
    // Recognize correctly numbered registers
    clearAllMacros();
    DoTest("foo", "q1iX\\escq@1", "XXfoo");

    {
        // Ensure that we can call emulated command bar searches, and that we don't record
        // synthetic keypresses.
        EmulatedCommandBarSetUpAndTearDown vimStyleCommandBarTestsSetUpAndTearDown(vi_input_mode, kate_view, mainWindow);
        clearAllMacros();
        DoTest("foo bar\nblank line", "qa/bar\\enterqgg@arX", "foo Xar\nblank line");
        // More complex searching stuff.
        clearAllMacros();
        DoTest("foo 123foo123\nbar 123bar123", "qayiw/\\ctrl-r0\\enterrXqggj@a", "foo 123Xoo123\nbar 123Xar123");
    }

    // Expand mappings,  but don't do *both* original keypresses and executed keypresses.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'"), QStringLiteral("ihello<c-c>"), Mappings::Recursive);
    clearAllMacros();
    DoTest("", "qa'q@a", "hellhelloo");
    // Actually, just do the mapped keypresses, not the executed mappings (like Vim).
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'"), QStringLiteral("ihello<c-c>"), Mappings::Recursive);
    clearAllMacros();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("qa'q"));
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'"), QStringLiteral("igoodbye<c-c>"), Mappings::Recursive);
    TestPressKey(QStringLiteral("@a"));
    FinishTest("hellgoodbyeo");
    // Clear the "stop recording macro keypresses because we're executing a mapping" when the mapping has finished
    // executing.
    clearAllMappings();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'"), QStringLiteral("ihello<c-c>"), Mappings::Recursive);
    clearAllMacros();
    DoTest("", "qa'ixyz\\ctrl-cq@a", "hellxyhellxyzozo");
    // ... make sure that *all* mappings have finished, though: take into account recursion.
    clearAllMappings();
    clearAllMacros();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'"), QStringLiteral("ihello<c-c>"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("ihello<c-c>"), QStringLiteral("irecursive<c-c>"), Mappings::Recursive);
    DoTest("", "qa'q@a", "recursivrecursivee");
    clearAllMappings();
    clearAllMacros();
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("'"), QStringLiteral("ihello<c-c>ixyz<c-c>"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("ihello<c-c>"), QStringLiteral("irecursive<c-c>"), Mappings::Recursive);
    DoTest("", "qa'q@a", "recursivxyrecursivxyzeze");

    clearAllMappings();
    clearAllMacros();
    // Don't save the trailing "q" with macros, and also test that we can call one macro from another,
    // without one of the macros being repeated.
    DoTest("", "qaixyz\\ctrl-cqqb@aq@b", "xyxyxyzzz");
    clearAllMappings();
    clearAllMacros();
    // More stringent test that macros called from another macro aren't repeated - requires more nesting
    // of macros ('a' calls 'b' calls 'c').
    DoTest("",
           "qciC\\ctrl-cq"
           "qb@ciB\\ctrl-cq"
           "qa@biA\\ctrl-cq"
           "dd@a",
           "ABC");
    // Don't crash if we invoke a non-existent macro.
    clearAllMacros();
    DoTest("", "@x", "");
    // Make macros "counted".
    clearAllMacros();
    DoTest("XXXX\nXXXX\nXXXX\nXXXX", "qarOljq3@a", "OXXX\nXOXX\nXXOX\nXXXO");

    // A macro can be undone with one undo.
    clearAllMacros();
    DoTest("foo bar", "qaciwxyz\\ctrl-ci123\\ctrl-cqw@au", "xy123z bar");
    // As can a counted macro.
    clearAllMacros();
    DoTest("XXXX\nXXXX\nXXXX\nXXXX", "qarOljq3@au", "OXXX\nXXXX\nXXXX\nXXXX");

    {
        EmulatedCommandBarSetUpAndTearDown vimStyleCommandBarTestsSetUpAndTearDown(vi_input_mode, kate_view, mainWindow);
        // Make sure we can macro-ise an interactive sed replace.
        clearAllMacros();
        DoTest("foo foo foo foo\nfoo foo foo foo", "qa:s/foo/bar/gc\\enteryynyAdone\\escqggj@a", "bar bar foo bardone\nbar bar foo bardone");
        // Make sure the closing "q" in the interactive sed replace isn't mistaken for a macro's closing "q".
        clearAllMacros();
        DoTest("foo foo foo foo\nfoo foo foo foo", "qa:s/foo/bar/gc\\enteryyqAdone\\escqggj@a", "bar bar foo foodone\nbar bar foo foodone");
        clearAllMacros();
        DoTest("foo foo foo foo\nfoo foo foo foo", "qa:s/foo/bar/gc\\enteryyqqAdone\\escggj@aAdone\\esc", "bar bar foo foodone\nbar bar foo foodone");
    }

    clearAllMappings();
    clearAllMacros();
    // Expand mapping in an executed macro, if the invocation of the macro "@a" is a prefix of a mapping M, and
    // M ends up not being triggered.
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("@aaaa"), QStringLiteral("idummy<esc>"), Mappings::Recursive);
    vi_global->mappings()->add(Mappings::NormalModeMapping, QStringLiteral("S"), QStringLiteral("ixyz<esc>"), Mappings::Recursive);
    DoTest("", "qaSq@abrX", "Xyxyzz");
    clearAllMappings();

    // Can't play old version of macro while recording new version.
    clearAllMacros();
    DoTest("", "qaiaaa\\ctrl-cqqa@aq", "aaa");

    // Can't play the macro while recording it.
    clearAllMacros();
    DoTest("", "qaiaaa\\ctrl-c@aq", "aaa");

    // "@@" plays back macro "a" if "a" was the last macro we played back.
    clearAllMacros();
    DoTest("", "qaia\\ctrl-cq@adiw@@", "a");
    // "@@" plays back macro "b" if "b" was the last macro we played back.
    clearAllMacros();
    DoTest("", "qbib\\ctrl-cq@bdiw@@", "b");
    // "@@" does nothing if no macro was previously played.
    clearAllMacros();
    DoTest("", "qaia\\ctrl-cq@@", "a");
    // Nitpick: "@@" replays the last played back macro, even if that macro had not been defined
    // when it was first played back.
    clearAllMacros();
    DoTest("", "@aqaia\\ctrl-cq@@", "aa");
    // "@@" is counted.
    clearAllMacros();
    DoTest("", "qaia\\ctrl-cq@adiw5@@", "aaaaa");

    // Test that we can save and restore a single macro.
    const QString viTestKConfigFileName = QStringLiteral("vimodetest-katevimoderc");
    {
        clearAllMacros();
        KConfig viTestKConfig(viTestKConfigFileName);
        BeginTest(QLatin1String(""));
        TestPressKey(QStringLiteral("qaia\\ctrl-cq"));
        vi_global->writeConfig(&viTestKConfig);
        viTestKConfig.sync();
        // Overwrite macro "a", and clear the document.
        TestPressKey(QStringLiteral("qaidummy\\ctrl-cqdd"));
        vi_global->readConfig(&viTestKConfig);
        TestPressKey(QStringLiteral("@a"));
        FinishTest("a");
    }

    {
        // Test that we can save and restore several macros.
        clearAllMacros();
        const QString viTestKConfigFileName = QStringLiteral("vimodetest-katevimoderc");
        KConfig viTestKConfig(viTestKConfigFileName);
        BeginTest(QLatin1String(""));
        TestPressKey(QStringLiteral("qaia\\ctrl-cqqbib\\ctrl-cq"));
        vi_global->writeConfig(&viTestKConfig);
        viTestKConfig.sync();
        // Overwrite macros "a" & "b", and clear the document.
        TestPressKey(QStringLiteral("qaidummy\\ctrl-cqqbidummy\\ctrl-cqdd"));
        vi_global->readConfig(&viTestKConfig);
        TestPressKey(QStringLiteral("@a@b"));
        FinishTest("ba");
    }

    // Ensure that we don't crash when a "repeat change" occurs in a macro we execute.
    clearAllMacros();
    DoTest("", "qqixyz\\ctrl-c.q@qdd", "");
    // Don't record both the "." *and* the last-change keypresses when recording a macro;
    // just record the "."
    clearAllMacros();
    DoTest("", "ixyz\\ctrl-cqq.qddi123\\ctrl-c@q", "121233");

    // Test dealing with auto-completion.
    FakeCodeCompletionTestModel *fakeCodeCompletionModel = new FakeCodeCompletionTestModel(kate_view);
    kate_view->registerCompletionModel(fakeCodeCompletionModel);
    // Completion tests require a visible kate_view.
    ensureKateViewVisible();
    // Want Vim mode to intercept ctrl-p, ctrl-n shortcuts, etc.
    const bool oldStealKeys = KateViewConfig::global()->viInputModeStealKeys();
    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, true);

    // Don't invoke completion via ctrl-space when replaying a macro.
    clearAllMacros();
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("qqico\\ctrl- \\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("@q"));
    FinishTest("ccoo");

    // Don't invoke completion via ctrl-p when replaying a macro.
    clearAllMacros();
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("qqico\\ctrl-p\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("@q"));
    FinishTest("ccoo");

    // Don't invoke completion via ctrl-n when replaying a macro.
    clearAllMacros();
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("qqico\\ctrl-n\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("@q"));
    FinishTest("ccoo");

    // An "enter" in insert mode when no completion is activated (so, a newline)
    // is treated as a newline when replayed as a macro, even if completion is
    // active when the "enter" is replayed.
    clearAllMacros();
    fakeCodeCompletionModel->setCompletions(QStringList()); // Prevent any completions.
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    fakeCodeCompletionModel->clearWasInvoked();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("qqicompl\\enterX\\ctrl-cqdddd"));
    QVERIFY(!fakeCodeCompletionModel->wasInvoked()); // Error in test setup!
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->forceInvocationIfDocTextIs(QStringLiteral("compl"));
    fakeCodeCompletionModel->clearWasInvoked();
    TestPressKey(QStringLiteral("@q"));
    QVERIFY(fakeCodeCompletionModel->wasInvoked()); // Error in test setup!
    fakeCodeCompletionModel->doNotForceInvocation();
    FinishTest("compl\nX");
    // Same for "return".
    clearAllMacros();
    fakeCodeCompletionModel->setCompletions(QStringList()); // Prevent any completions.
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    fakeCodeCompletionModel->clearWasInvoked();
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("qqicompl\\returnX\\ctrl-cqdddd"));
    QVERIFY(!fakeCodeCompletionModel->wasInvoked()); // Error in test setup!
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->forceInvocationIfDocTextIs(QStringLiteral("compl"));
    fakeCodeCompletionModel->clearWasInvoked();
    TestPressKey(QStringLiteral("@q"));
    QVERIFY(fakeCodeCompletionModel->wasInvoked()); // Error in test setup!
    fakeCodeCompletionModel->doNotForceInvocation();
    FinishTest("compl\nX");

    // If we do a plain-text completion in a macro, this should be repeated when we replay it.
    clearAllMacros();
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqicompl\\ctrl- \\enter\\ctrl-cq"));
    kate_document->clear();
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("@q"));
    FinishTest("completionA");

    // Should replace only the current word when we repeat the completion.
    clearAllMacros();
    BeginTest(QStringLiteral("compl"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqfla\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("(compl)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("(completionA)");

    // Tail-clearing completions should be undoable with one undo.
    clearAllMacros();
    BeginTest(QStringLiteral("compl"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqfla\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("(compl)"));
    TestPressKey(QStringLiteral("gg@qu"));
    FinishTest("(compl)");

    // Should be able to store multiple completions.
    clearAllMacros();
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqicom\\ctrl-p\\enter com\\ctrl-p\\ctrl-p\\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("dd@q"));
    FinishTest("completionC completionB");

    // Clear the completions for a macro when we start recording.
    clearAllMacros();
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionOrig")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqicom\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionSecond")});
    TestPressKey(QStringLiteral("ddqqicom\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("dd@q"));
    FinishTest("completionSecond");

    // Completions are per macro.
    clearAllMacros();
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qaicom\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionB")});
    TestPressKey(QStringLiteral("ddqbicom\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("dd@aA\\enter\\ctrl-c@b"));
    FinishTest("completionA\ncompletionB");

    // Make sure completions work with recursive macros.
    clearAllMacros();
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA1"), QStringLiteral("completionA2")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    // Record 'a', which calls the (non-yet-existent) macro 'b'.
    TestPressKey(QStringLiteral("qaicom\\ctrl- \\enter\\ctrl-cA\\enter\\ctrl-c@bA\\enter\\ctrl-cicom\\ctrl- \\ctrl-p\\enter\\ctrl-cq"));
    // Clear document and record 'b'.
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionB")});
    TestPressKey(QStringLiteral("ggdGqbicom\\ctrl- \\enter\\ctrl-cq"));
    TestPressKey(QStringLiteral("dd@a"));
    FinishTest("completionA1\ncompletionB\ncompletionA2");

    // Test that non-tail-removing completions are respected.
    // Note that there is no way (in general) to determine if a completion was
    // non-tail-removing, so we explicitly set the config to false.
    const bool oldRemoveTailOnCompletion = KateViewConfig::global()->wordCompletionRemoveTail();
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
    const bool oldReplaceTabsDyn = kate_document->config()->replaceTabsDyn();
    kate_document->config()->setReplaceTabsDyn(false);
    fakeCodeCompletionModel->setRemoveTailOnComplete(false);
    clearAllMacros();
    BeginTest(QStringLiteral("compTail"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqfTi\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("compTail"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("completionATail");

    // A "word" consists of letters & numbers, plus "_".
    clearAllMacros();
    BeginTest(QStringLiteral("(123_compTail"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("123_completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqfTi\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("(123_compTail"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("(123_completionATail");

    // Correctly remove word if we are set to remove tail.
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    clearAllMacros();
    BeginTest(QStringLiteral("(123_compTail)"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("123_completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    TestPressKey(QStringLiteral("qqfTi\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("(123_compTail)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("(123_completionA)");

    // Again, a "word" consists of letters & numbers & underscores.
    clearAllMacros();
    BeginTest(QStringLiteral("(123_compTail_456)"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("123_completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    TestPressKey(QStringLiteral("qqfTi\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("(123_compTail_456)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("(123_completionA)");

    // Actually, let whether the tail is swallowed or not depend on the value when the
    // completion occurred, not when we replay it.
    clearAllMacros();
    BeginTest(QStringLiteral("(123_compTail_456)"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("123_completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    TestPressKey(QStringLiteral("qqfTi\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
    kate_document->setText(QStringLiteral("(123_compTail_456)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("(123_completionA)");
    clearAllMacros();
    BeginTest(QStringLiteral("(123_compTail_456)"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("123_completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    fakeCodeCompletionModel->setRemoveTailOnComplete(false);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
    TestPressKey(QStringLiteral("qqfTi\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    kate_document->setText(QStringLiteral("(123_compTail_456)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("(123_completionATail_456)");

    // Can have remove-tail *and* non-remove-tail completions in one macro.
    clearAllMacros();
    BeginTest(QStringLiteral("(123_compTail_456)\n(123_compTail_456)"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("123_completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    TestPressKey(QStringLiteral("qqfTi\\ctrl- \\enter\\ctrl-c"));
    fakeCodeCompletionModel->setRemoveTailOnComplete(false);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
    TestPressKey(QStringLiteral("j^fTi\\ctrl- \\enter\\ctrl-cq"));
    kate_document->setText(QStringLiteral("(123_compTail_456)\n(123_compTail_456)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("(123_completionA)\n(123_completionATail_456)");

    // Can repeat plain-text completions when there is no word to the left of the cursor.
    clearAllMacros();
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("123_completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqi\\ctrl- \\enter\\ctrl-cq"));
    kate_document->clear();
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("123_completionA");

    // Shouldn't swallow the letter under the cursor if we're not swallowing tails.
    clearAllMacros();
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("123_completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    fakeCodeCompletionModel->setRemoveTailOnComplete(false);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
    TestPressKey(QStringLiteral("qqi\\ctrl- \\enter\\ctrl-cq"));
    kate_document->setText(QStringLiteral("oldwordshouldbeuntouched"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("123_completionAoldwordshouldbeuntouched");

    // ... but do if we are swallowing tails.
    clearAllMacros();
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("123_completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    TestPressKey(QStringLiteral("qqi\\ctrl- \\enter\\ctrl-cq"));
    kate_document->setText(QStringLiteral("oldwordshouldbedeleted"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("123_completionA");

    // Completion of functions.
    // Currently, not removing the tail on function completion is not supported.
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    // A completed, no argument function "function()" is repeated correctly.
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function()")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("dd@q"));
    FinishTest("function()");

    // Cursor is placed after the closing bracket when completion a no-arg function.
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function()")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enter.something();\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("dd@q"));
    FinishTest("function().something();");

    // A function taking some arguments, repeated where there is no opening bracket to
    // merge with, is repeated as "function()").
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...)")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enter\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("dd@q"));
    FinishTest("function()");

    // A function taking some arguments, repeated where there is no opening bracket to
    // merge with, places the cursor after the opening bracket.
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...)")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enterfirstArg\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("dd@q"));
    FinishTest("function(firstArg)");

    // A function taking some arguments, recorded where there was an opening bracket to merge
    // with but repeated where there is no such bracket, is repeated as "function()" and the
    // cursor placed appropriately.
    BeginTest(QStringLiteral("(<-Mergeable opening bracket)"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...)")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enterfirstArg\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("dd@q"));
    FinishTest("function(firstArg)");

    // A function taking some arguments, recorded where there was no opening bracket to merge
    // with but repeated where there is such a bracket, is repeated as "function" and the
    // cursor moved to after the merged opening bracket.
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...)")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enterfirstArg\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("(<-firstArg goes here)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function(firstArg<-firstArg goes here)");

    // A function taking some arguments, recorded where there was an opening bracket to merge
    // with and repeated where there is also such a bracket, is repeated as "function" and the
    // cursor moved to after the merged opening bracket.
    BeginTest(QStringLiteral("(<-mergeablebracket)"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...)")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enterfirstArg\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("(<-firstArg goes here)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function(firstArg<-firstArg goes here)");

    // The mergeable bracket can be separated by whitespace; the cursor is still placed after the
    // opening bracket.
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...)")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enterfirstArg\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("   \t (<-firstArg goes here)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function   \t (firstArg<-firstArg goes here)");

    // Whitespace only, though!
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...)")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enterfirstArg\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("|   \t ()"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function(firstArg)|   \t ()");

    // The opening bracket can actually be after the current word (with optional whitespace).
    // Note that this wouldn't be the case if we weren't swallowing tails when completion functions,
    // but this is not currently supported.
    BeginTest(QStringLiteral("function"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...)")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqfta\\ctrl- \\enterfirstArg\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("functxyz    (<-firstArg goes here)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function    (firstArg<-firstArg goes here)");

    // Regression test for weird issue with replaying completions when the character to the left of the cursor
    // is not a word char.
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqciw\\ctrl- \\enter\\ctrl-cq"));
    TestPressKey(QStringLiteral("ddi.xyz\\enter123\\enter456\\ctrl-cggl")); // Position cursor just after the "."
    TestPressKey(QStringLiteral("@q"));
    FinishTest(".completionA\n123\n456");
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqciw\\ctrl- \\enter\\ctrl-cq"));
    TestPressKey(QStringLiteral("ddi.xyz.abc\\enter123\\enter456\\ctrl-cggl")); // Position cursor just after the "."
    TestPressKey(QStringLiteral("@q"));
    FinishTest(".completionA.abc\n123\n456");

    // Functions taking no arguments are never bracket-merged.
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function()")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifunc\\ctrl- \\enter.something();\\ctrl-cq"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("(<-don't merge this bracket)"));
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function().something();(<-don't merge this bracket)");

    // Not-removing-tail when completing functions is not currently supported,
    // so ignore the "do-not-remove-tail" settings when we try this.
    BeginTest(QStringLiteral("funct"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...)")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
    TestPressKey(QStringLiteral("qqfta\\ctrl- \\enterfirstArg\\ctrl-cq"));
    kate_document->setText(QStringLiteral("functxyz"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function(firstArg)");
    BeginTest(QStringLiteral("funct"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function()")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
    TestPressKey(QStringLiteral("qqfta\\ctrl- \\enter\\ctrl-cq"));
    kate_document->setText(QStringLiteral("functxyz"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function()");
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);

    // Deal with cases where completion ends with ";".
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function();")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifun\\ctrl- \\enter\\ctrl-cq"));
    kate_document->clear();
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function();");
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function();")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifun\\ctrl- \\enterX\\ctrl-cq"));
    kate_document->clear();
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function();X");
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...);")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifun\\ctrl- \\enter\\ctrl-cq"));
    kate_document->clear();
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function();");
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...);")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifun\\ctrl- \\enterX\\ctrl-cq"));
    kate_document->clear();
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function(X);");
    // Tests for completions ending in ";" where bracket merging should happen on replay.
    // NB: bracket merging when recording is impossible with completions that end in ";".
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...);")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifun\\ctrl- \\enter\\ctrl-cq"));
    kate_document->setText(QStringLiteral("(<-mergeable bracket"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function(<-mergeable bracket");
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function(...);")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifun\\ctrl- \\enterX\\ctrl-cq"));
    kate_document->setText(QStringLiteral("(<-mergeable bracket"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function(X<-mergeable bracket");
    // Don't merge no arg functions.
    BeginTest(QLatin1String(""));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("function();")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("qqifun\\ctrl- \\enterX\\ctrl-cq"));
    kate_document->setText(QStringLiteral("(<-mergeable bracket"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("gg@q"));
    FinishTest("function();X(<-mergeable bracket");

    {
        const QString viTestKConfigFileName = QStringLiteral("vimodetest-katevimoderc");
        KConfig viTestKConfig(viTestKConfigFileName);
        // Test loading and saving of macro completions.
        clearAllMacros();
        BeginTest(QStringLiteral("funct\nnoa\ncomtail\ncomtail\ncom"));
        fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("functionwithargs(...)"), QStringLiteral("noargfunction()")});
        fakeCodeCompletionModel->setFailTestOnInvocation(false);
        // Record 'a'.
        TestPressKey(QStringLiteral("qafta\\ctrl- \\enterfirstArg\\ctrl-c")); // Function with args.
        TestPressKey(QStringLiteral("\\enterea\\ctrl- \\enter\\ctrl-c")); // Function no args.
        fakeCodeCompletionModel->setRemoveTailOnComplete(true);
        KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
        TestPressKey(QStringLiteral("\\enterfti\\ctrl- \\enter\\ctrl-c")); // Cut off tail.
        fakeCodeCompletionModel->setRemoveTailOnComplete(false);
        KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
        TestPressKey(QStringLiteral("\\enterfti\\ctrl- \\enter\\ctrl-cq")); // Don't cut off tail.
        fakeCodeCompletionModel->setRemoveTailOnComplete(true);
        KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
        // Record 'b'.
        fakeCodeCompletionModel->setCompletions(
            {QStringLiteral("completionB"), QStringLiteral("semicolonfunctionnoargs();"), QStringLiteral("semicolonfunctionwithargs(...);")});
        TestPressKey(
            QStringLiteral("\\enterqbea\\ctrl- \\enter\\ctrl-cosemicolonfunctionw\\ctrl- \\enterX\\ctrl-cosemicolonfunctionn\\ctrl- \\enterX\\ctrl-cq"));
        // Save.
        vi_global->writeConfig(&viTestKConfig);
        viTestKConfig.sync();
        // Overwrite 'a' and 'b' and their completions.
        fakeCodeCompletionModel->setCompletions({QStringLiteral("blah1")});
        kate_document->setText(QLatin1String(""));
        TestPressKey(QStringLiteral("ggqaiblah\\ctrl- \\enter\\ctrl-cq"));
        TestPressKey(QStringLiteral("ddqbiblah\\ctrl- \\enter\\ctrl-cq"));
        // Reload.
        vi_global->readConfig(&viTestKConfig);
        // Replay reloaded.
        fakeCodeCompletionModel->setFailTestOnInvocation(true);
        kate_document->setText(QStringLiteral("funct\nnoa\ncomtail\ncomtail\ncom"));
        TestPressKey(QStringLiteral("gg@a\\enter@b"));
        FinishTest(
            "functionwithargs(firstArg)\nnoargfunction()\ncompletionA\ncompletionAtail\ncompletionB\nsemicolonfunctionwithargs(X);\nsemicolonfunctionnoargs();"
            "X");
    }

    // Check that undo/redo operations work properly with macros.
    {
        clearAllMacros();
        BeginTest(QLatin1String(""));
        TestPressKey(QStringLiteral("ihello\\ctrl-cqauq"));
        TestPressKey(QStringLiteral("@a\\enter"));
        FinishTest("");
    }
    {
        clearAllMacros();
        BeginTest(QLatin1String(""));
        TestPressKey(QStringLiteral("ihello\\ctrl-cui.bye\\ctrl-cu"));
        TestPressKey(QStringLiteral("qa\\ctrl-r\\enterq"));
        TestPressKey(QStringLiteral("@a\\enter"));
        FinishTest(".bye");
    }

    // When replaying a last change in the process of replaying a macro, take the next completion
    // event from the last change completions log, rather than the macro completions log.
    // Ensure that the last change completions log is kept up to date even while we're replaying the macro.
    if (false) { // FIXME: test currently fails in newer Qt >= 5.11, but works with Qt 5.10
        clearAllMacros();
        BeginTest(QLatin1String(""));
        fakeCodeCompletionModel->setCompletions({QStringLiteral("completionMacro"), QStringLiteral("completionRepeatLastChange")});
        fakeCodeCompletionModel->setFailTestOnInvocation(false);
        TestPressKey(QStringLiteral("qqicompletionM\\ctrl- \\enter\\ctrl-c"));
        TestPressKey(QStringLiteral("a completionRep\\ctrl- \\enter\\ctrl-c"));
        TestPressKey(QStringLiteral(".q"));
        qDebug() << "text: " << kate_document->text();
        kate_document->clear();
        TestPressKey(QStringLiteral("gg@q"));
        FinishTest("completionMacro completionRepeatLastChange completionRepeatLastChange");
    }

    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, oldRemoveTailOnCompletion);
    kate_document->config()->setReplaceTabsDyn(oldReplaceTabsDyn);

    kate_view->unregisterCompletionModel(fakeCodeCompletionModel);
    delete fakeCodeCompletionModel;
    fakeCodeCompletionModel = nullptr;
    // Hide the kate_view for subsequent tests.
    kate_view->hide();
    mainWindow->hide();
    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, oldStealKeys);
}

void KeysTest::MarkTests()
{
    // Difference between ` and '
    DoTest("  a\n    b", "jmak'aii", "  a\n    ib");
    DoTest("  a\n    b", "jmak`aii", "  a\ni    b");

    // Last edit markers.
    DoTest("foo", "ean\\escgg`.r.", "foo.");
    DoTest("foo", "ean\\escgg`[r[", "foo[");
    DoTest("foo", "ean\\escgg`]r]", "foo]");
    DoTest("foo bar", "ean\\escgg`]r]", "foon]bar");
    DoTest("", "ibar\\escgg`.r.", "ba.");
    DoTest("", "ibar\\escgggUiw`.r.", ".AR");
    DoTest("", "2ibar\\escgg`]r]", "barba]");
    DoTest("", "2ibar\\escgg`[r[", "[arbar");
    DoTest("", "3ibar\\escgg`.r.", "barbar.ar"); // Vim is weird.
    DoTest("", "abar\\esc.gg`]r]", "barba]");
    DoTest("foo bar", "wgUiwgg`]r]", "foo BA]");
    DoTest("foo bar", "wgUiwgg`.r.", "foo .AR");
    DoTest("foo bar", "gUiwgg`]r.", "FO. bar");
    DoTest("foo bar", "wdiwgg`[r[", "foo[");
    DoTest("foo bar", "wdiwgg`]r]", "foo]");
    DoTest("foo bar", "wdiwgg`.r.", "foo.");
    DoTest("foo bar", "wciwnose\\escgg`.r.", "foo nos.");
    DoTest("foo bar", "wciwnose\\escgg`[r[", "foo [ose");
    DoTest("foo bar", "wciwnose\\escgg`]r]", "foo nos]");
    DoTest("foo", "~ibar\\escgg`[r[", "F[aroo");
    DoTest("foo bar", "lragg`.r.", "f.o bar");
    DoTest("foo bar", "lragg`[r[", "f[o bar");
    DoTest("foo bar", "lragg`]r]", "f]o bar");
    DoTest("", "ifoo\\ctrl-hbar\\esc`[r[", "[obar");
    DoTest("", "ifoo\\ctrl-wbar\\esc`[r[", "[ar");
    DoTest("", "if\\ctrl-hbar\\esc`[r[", "[ar");
    DoTest("", "5ofoo\\escgg`[r[", "\n[oo\nfoo\nfoo\nfoo\nfoo");
    DoTest("", "5ofoo\\escgg`]r]", "\nfoo\nfoo\nfoo\nfoo\nfo]");
    DoTest("", "5ofoo\\escgg`.r.", "\nfoo\nfoo\nfoo\nfoo\n.oo");
    DoTest("foo", "yyp`[r[", "foo\n[oo");
    DoTest("xyz\nfoo", "ja\\returnbar\\esc`[r[", "xyz\n[\nbaroo");
    DoTest("foo", "lrayypgg`[r[", "fao\n[ao");
    DoTest("foo", "l~u`[r[", "[oo");
    DoTest("foo", "l~u`.r.", ".oo");
    DoTest("foo", "l~u`]r]", "]oo");
    DoTest("foo", "lia\\escu`[r[", "[oo");
    DoTest("foo", "lia\\escu`.r.", ".oo");
    DoTest("foo", "lia\\escu`]r]", "]oo");
    DoTest("foo", "l~u~`[r[", "f[o");
    DoTest("foo\nbar\nxyz", "jyypu`[r[", "foo\nbar\n[yz");
    DoTest("foo\nbar\nxyz", "jyypu`.r.", "foo\nbar\n.yz");
    DoTest("foo\nbar\nxyz", "jyypu`]r]", "foo\nbar\n]yz");
    DoTest("foo\nbar\nxyz\n123", "jdju`[r[", "foo\n[ar\nxyz\n123");
    DoTest("foo\nbar\nxyz\n123", "jdju`.r.", "foo\n.ar\nxyz\n123");
    DoTest("foo\nbar\nxyz\n123", "jdju`]r]", "foo\nbar\n]yz\n123");
    DoTest("foo\nbar\nxyz\n123", "jVj~u\\esc`[r[", "foo\n[ar\nxyz\n123", ShouldFail, "Vim is weird.");

    // unicode, bug 462437
    DoTest("nahrávání", "ean\\escgg`.r.", "nahrávání.");
    DoTest("nahrávání", "ex", "nahráván");
}

// END: KeysTest

#include "moc_keys.cpp"
