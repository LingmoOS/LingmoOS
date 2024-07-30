/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "templatehandler_test.h"

#include <KTextEditor/Cursor>
#include <kateconfig.h>
#include <katedocument.h>
#include <katetemplatehandler.h>
#include <kateview.h>

#include <QStandardPaths>
#include <QString>
#include <QTest>
#include <QTestKeyEvent>

QTEST_MAIN(TemplateHandlerTest)

using namespace KTextEditor;

TemplateHandlerTest::TemplateHandlerTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TemplateHandlerTest::testUndo()
{
    const QString snippet = QStringLiteral(
        "for (${type=\"int\"} ${index=\"i\"} = ; ${index} < ; ++${index})\n"
        "{\n"
        "    ${index}\n"
        "}");

    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));

    // fixed indentation options
    doc->config()->setTabWidth(8);
    doc->config()->setIndentationWidth(4);
    doc->config()->setReplaceTabsDyn(true);

    view->insertTemplate(KTextEditor::Cursor(0, 0), snippet);

    const QString result = QStringLiteral(
        "for (int i = ; i < ; ++i)\n"
        "{\n"
        "    i\n"
        "}");
    QCOMPARE(doc->text(), result);

    doc->replaceText(Range(0, 9, 0, 10), QStringLiteral("j"));

    const QString result2 = QStringLiteral(
        "for (int j = ; j < ; ++j)\n"
        "{\n"
        "    j\n"
        "}");
    QCOMPARE(doc->text(), result2);

    doc->undo();

    QCOMPARE(doc->text(), result);

    doc->redo();

    QCOMPARE(doc->text(), result2);

    doc->insertText(Cursor(0, 10), QStringLiteral("j"));
    doc->insertText(Cursor(0, 11), QStringLiteral("j"));

    const QString result3 = QStringLiteral(
        "for (int jjj = ; jjj < ; ++jjj)\n"
        "{\n"
        "    jjj\n"
        "}");
    QCOMPARE(doc->text(), result3);

    doc->undo();

    QCOMPARE(doc->text(), result);

    doc->redo();

    QCOMPARE(doc->text(), result3);

    doc->undo();
    QCOMPARE(doc->text(), result);

    doc->undo();
    QCOMPARE(doc->text(), QString());
}

void TemplateHandlerTest::testEscapes()
{
    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));
    view->insertTemplate({0, 0}, QStringLiteral("\\${field} ${bar} \\${foo=3} \\\\${baz=7}"));
    QCOMPARE(doc->text(), QStringLiteral("${field} bar ${foo=3} \\${baz=7}"));
}

void TemplateHandlerTest::testSimpleMirror()
{
    QFETCH(QString, text);

    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));
    view->insertTemplate({0, 0}, text);

    QCOMPARE(doc->text(), QString(text).replace(QStringLiteral("${foo}"), QStringLiteral("foo")));

    doc->insertText({0, 0}, QStringLiteral("xx"));
    QCOMPARE(doc->text(), QString(text).replace(QStringLiteral("${foo}"), QStringLiteral("xxfoo")));

    doc->removeText(KTextEditor::Range({0, 0}, {0, 2}));
    QCOMPARE(doc->text(), QString(text).replace(QStringLiteral("${foo}"), QStringLiteral("foo")));

    delete doc;
}

void TemplateHandlerTest::testSimpleMirror_data()
{
    QTest::addColumn<QString>("text");

    QTest::newRow("one") << QStringLiteral("${foo}");
    QTest::newRow("several") << QStringLiteral("${foo} ${foo} Foo ${foo}");
}

void TemplateHandlerTest::testAlignC()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);

    auto doc = new KTextEditor::DocumentPrivate();
    doc->setHighlightingMode(QStringLiteral("C"));
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));
    view->insertTemplate({0, 0}, input);

    QCOMPARE(doc->text(), expected);

    delete doc;
}

void TemplateHandlerTest::testAlignC_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("one") << QStringLiteral("/* ${foo} */") << QStringLiteral("/* foo */");
    QTest::newRow("simple") << QStringLiteral("/**\n* ${foo}\n*/") << QStringLiteral("/**\n * foo\n */");
    QTest::newRow("complex") << QStringLiteral("/**\n* @brief: ${...}\n* \n*/") << QStringLiteral("/**\n * @brief: ...\n * \n */");
}

void TemplateHandlerTest::testAdjacentRanges()
{
    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));

    view->insertTemplate({0, 0}, QStringLiteral("${foo} ${foo}"));
    QCOMPARE(doc->text(), QStringLiteral("foo foo"));
    doc->removeText(KTextEditor::Range({0, 3}, {0, 4}));
    QCOMPARE(doc->text(), QStringLiteral("foofoo"));
    doc->insertText({0, 1}, QStringLiteral("x"));
    QCOMPARE(doc->text(), QStringLiteral("fxoofxoo"));
    doc->insertText({0, 4}, QStringLiteral("y"));
    QCOMPARE(doc->text(), QStringLiteral("fxooyfxooy"));
    doc->removeText(KTextEditor::Range({0, 4}, {0, 5}));
    QCOMPARE(doc->text(), QStringLiteral("fxoofxoo"));

    delete doc;
}

void TemplateHandlerTest::testTab()
{
    QFETCH(QString, tpl);
    QFETCH(int, cursor);

    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));

    view->insertTemplate({0, 0}, tpl);
    view->setCursorPosition({0, cursor});

    // no idea why the event needs to be posted to the focus proxy
    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QTEST(view->cursorPosition().column(), "expected_cursor");

    QTest::keyClick(view->focusProxy(), Qt::Key_Tab, Qt::ShiftModifier);
    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QTEST(view->cursorPosition().column(), "expected_cursor");

    delete doc;
}

void TemplateHandlerTest::testTab_data()
{
    QTest::addColumn<QString>("tpl");
    QTest::addColumn<int>("cursor");
    QTest::addColumn<int>("expected_cursor");

    QTest::newRow("simple_start") << "${foo} ${bar}" << 0 << 4;
    QTest::newRow("simple_mid") << "${foo} ${bar}" << 2 << 4;
    QTest::newRow("simple_end") << "${foo} ${bar}" << 3 << 4;
    QTest::newRow("wrap_start") << "${foo} ${bar}" << 4 << 0;
    QTest::newRow("wrap_mid") << "${foo} ${bar}" << 5 << 0;
    QTest::newRow("wrap_end") << "${foo} ${bar}" << 6 << 0;
    QTest::newRow("non_editable_start") << "${foo} ${foo}" << 0 << 0;
    QTest::newRow("non_editable_mid") << "${foo} ${foo}" << 2 << 0;
    QTest::newRow("non_editable_end") << "${foo} ${foo}" << 3 << 0;
    QTest::newRow("skip_non_editable") << "${foo} ${foo} ${bar}" << 0 << 8;
    QTest::newRow("skip_non_editable_at_end") << "${foo} ${bar} ${foo}" << 4 << 0;
    QTest::newRow("jump_to_cursor") << "${foo} ${cursor}" << 0 << 4;
    QTest::newRow("jump_to_cursor_last") << "${foo} ${cursor} ${bar}" << 0 << 5;
    QTest::newRow("jump_to_cursor_last2") << "${foo} ${cursor} ${bar}" << 5 << 4;
}

void TemplateHandlerTest::testExitAtCursor()
{
    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));

    view->insertTemplate({0, 0}, QStringLiteral("${foo} ${bar} ${cursor} ${foo}"));
    view->setCursorPosition({0, 0});

    // check it jumps to the cursor
    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QCOMPARE(view->cursorPosition().column(), 4);
    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QCOMPARE(view->cursorPosition().column(), 8);

    // insert an a at cursor position
    QTest::keyClick(view->focusProxy(), Qt::Key_A);
    // check it was inserted
    QCOMPARE(doc->text(), QStringLiteral("foo bar a foo"));

    // required to process the deleteLater() used to exit the template handler
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QApplication::processEvents();

    // go to the first field and verify it's not mirrored any more (i.e. the handler exited)
    view->setCursorPosition({0, 0});
    QTest::keyClick(view->focusProxy(), Qt::Key_A);
    QCOMPARE(doc->text(), QStringLiteral("afoo bar a foo"));

    delete doc;
}

void TemplateHandlerTest::testDefaultMirror()
{
    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));

    view->insertTemplate({0, 0},
                         QStringLiteral("${foo=uppercase(\"hi\")} ${bar=3} ${foo}"),
                         QStringLiteral("function uppercase(x) { return x.toUpperCase(); }"));
    QCOMPARE(doc->text(), QStringLiteral("HI 3 HI"));
    doc->insertText({0, 0}, QStringLiteral("xy@"));
    QCOMPARE(doc->text(), QStringLiteral("xy@HI 3 xy@HI"));

    delete doc;
}

void TemplateHandlerTest::testFunctionMirror()
{
    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));

    view->insertTemplate({0, 0}, QStringLiteral("${foo} hi ${uppercase(foo)}"), QStringLiteral("function uppercase(x) { return x.toUpperCase(); }"));
    QCOMPARE(doc->text(), QStringLiteral("foo hi FOO"));
    doc->insertText({0, 0}, QStringLiteral("xy@"));
    QCOMPARE(doc->text(), QStringLiteral("xy@foo hi XY@FOO"));

    delete doc;
}

void TemplateHandlerTest::testAutoSelection()
{
    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));

    view->insertTemplate({0, 0}, QStringLiteral("${foo} ${bar} ${bar} ${cursor} ${baz}"));
    QCOMPARE(doc->text(), QStringLiteral("foo bar bar  baz"));
    QCOMPARE(view->selectionText(), QStringLiteral("foo"));

    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QCOMPARE(view->selectionText(), QStringLiteral("bar"));

    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QCOMPARE(view->selectionText(), QStringLiteral("baz"));

    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QVERIFY(view->selectionRange().isEmpty());

    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QCOMPARE(view->selectionText(), QStringLiteral("foo"));
    QTest::keyClick(view->focusProxy(), Qt::Key_A);
    QCOMPARE(doc->text(), QStringLiteral("a bar bar  baz"));

    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QTest::keyClick(view->focusProxy(), Qt::Key_Tab);
    QVERIFY(view->selectionRange().isEmpty());
}

void TemplateHandlerTest::testNotEditableFields()
{
    QFETCH(QString, input);
    QFETCH(int, change_offset);

    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));
    view->insertTemplate({0, 0}, input);

    doc->insertText({0, change_offset}, QStringLiteral("xxx"));
    QTEST(doc->text(), "expected");
}

void TemplateHandlerTest::testNotEditableFields_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("change_offset");
    QTest::addColumn<QString>("expected");

    QTest::newRow("mirror") << QStringLiteral("${foo} ${foo}") << 6 << "foo foxxxo";
}

void TemplateHandlerTest::testCanRetrieveSelection()
{
    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));
    view->insertText(QStringLiteral("hi world"));
    view->setSelection(KTextEditor::Range(0, 1, 0, 4));
    view->insertTemplate({0, 1}, QStringLiteral("xx${foo=sel()}xx"), QStringLiteral("function sel() { return view.selectedText(); }"));
    QCOMPARE(doc->text(), QStringLiteral("hxxi wxxorld"));
}

void TemplateHandlerTest::testDefaults_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<QString>("function");

    using S = QString;
    QTest::newRow("empty") << S() << S() << S();
    QTest::newRow("foo") << QStringLiteral("${foo}") << QStringLiteral("foo") << S();
    QTest::newRow("foo=3") << QStringLiteral("${foo=3}") << QStringLiteral("3") << S();
    QTest::newRow("${foo=3+5}") << QStringLiteral("${foo=3+5}") << QStringLiteral("8") << S();
    QTest::newRow("string") << QStringLiteral("${foo=\"3+5\"}") << QStringLiteral("3+5") << S();
    QTest::newRow("string_mirror") << QStringLiteral("${foo=\"Bar\"} ${foo}") << QStringLiteral("Bar Bar") << S();
    QTest::newRow("func_simple") << QStringLiteral("${foo=myfunc()}") << QStringLiteral("hi") << QStringLiteral("function myfunc() { return 'hi'; }");
    QTest::newRow("func_fixed") << QStringLiteral("${myfunc()}") << QStringLiteral("hi") << QStringLiteral("function myfunc() { return 'hi'; }");
    QTest::newRow("func_constant_arg") << QStringLiteral("${foo=uppercase(\"Foo\")}") << QStringLiteral("FOO")
                                       << QStringLiteral("function uppercase(x) { return x.toUpperCase(); }");
    QTest::newRow("func_constant_arg_mirror") << QStringLiteral("${foo=uppercase(\"hi\")} ${bar=3} ${foo}") << QStringLiteral("HI 3 HI")
                                              << QStringLiteral("function uppercase(x) { return x.toUpperCase(); }");
    QTest::newRow("cursor") << QStringLiteral("${foo} ${cursor}") << QStringLiteral("foo ") << S();
    QTest::newRow("only_cursor") << QStringLiteral("${cursor}") << QStringLiteral("") << S();
    QTest::newRow("only_cursor_stuff") << QStringLiteral("fdas ${cursor} asdf") << QStringLiteral("fdas  asdf") << S();
}

void TemplateHandlerTest::testDefaults()
{
    auto doc = new KTextEditor::DocumentPrivate();
    auto view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));

    QFETCH(QString, input);
    QFETCH(QString, function);

    view->insertTemplate(KTextEditor::Cursor(0, 0), input, function);
    QTEST(doc->text(), "expected");

    view->selectAll();
    view->keyDelete();
    QCOMPARE(doc->text(), QString());

    delete doc;
}

#include "moc_templatehandler_test.cpp"
