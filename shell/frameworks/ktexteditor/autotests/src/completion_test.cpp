/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams\gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "completion_test.h"
#include "codecompletiontestmodels.h"
// #include "codecompletiontestmodels.moc"

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>

#include <katecompletionmodel.h>
#include <katecompletiontree.h>
#include <katecompletionwidget.h>
#include <kateconfig.h>
#include <katerenderer.h>
#include <kateview.h>

#include <QApplication>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(CompletionTest)

using namespace KTextEditor;
static constexpr int TIMEOUT = 2000;

int countItems(KateCompletionModel *model)
{
    const int topLevel = model->rowCount(QModelIndex());
    if (!model->hasGroups()) {
        return topLevel;
    }
    int ret = 0;
    for (int i = 0; i < topLevel; ++i) {
        ret += model->rowCount(model->index(i, 0));
    }
    return ret;
}

static void verifyCompletionStarted(KTextEditor::ViewPrivate *view)
{
    QTRY_VERIFY_WITH_TIMEOUT(view->completionWidget()->isCompletionActive(), TIMEOUT);
}

static void verifyCompletionAborted(KTextEditor::ViewPrivate *view)
{
    QTRY_VERIFY_WITH_TIMEOUT(!view->completionWidget()->isCompletionActive(), TIMEOUT);
}

static void invokeCompletionBox(KTextEditor::ViewPrivate *view)
{
    view->userInvokedCompletion();
    verifyCompletionStarted(view);
}

void CompletionTest::init()
{
    QStandardPaths::setTestModeEnabled(true);
    Editor *editor = KTextEditor::Editor::instance();
    QVERIFY(editor);

    m_doc = editor->createDocument(this);
    QVERIFY(m_doc);
    m_doc->setText(QStringLiteral("aa bb cc\ndd"));

    KTextEditor::View *v = m_doc->createView(nullptr);
    QApplication::setActiveWindow(v);
    m_view = static_cast<KTextEditor::ViewPrivate *>(v);
    Q_ASSERT(m_view);

    // view needs to be shown as completion won't work if the cursor is off screen
    m_view->show();
}

void CompletionTest::cleanup()
{
    delete m_view;
    delete m_doc;
}

void CompletionTest::testFilterEmptyRange()
{
    KateCompletionModel *model = m_view->completionWidget()->model();

    new CodeCompletionTestModel(m_view, QStringLiteral("a"));
    m_view->setCursorPosition(Cursor(0, 0));
    invokeCompletionBox(m_view);

    QCOMPARE(countItems(model), 40);
    m_view->insertText(QStringLiteral("aa"));
    QTRY_COMPARE_WITH_TIMEOUT(countItems(model), 14, TIMEOUT);
}

void CompletionTest::testFilterWithRange()
{
    KateCompletionModel *model = m_view->completionWidget()->model();

    CodeCompletionTestModel *testModel = new CodeCompletionTestModel(m_view, QStringLiteral("a"));
    m_view->setCursorPosition(Cursor(0, 2));
    invokeCompletionBox(m_view);

    Range complRange = *m_view->completionWidget()->completionRange(testModel);
    QCOMPARE(complRange, Range(Cursor(0, 0), Cursor(0, 2)));
    QCOMPARE(countItems(model), 14);

    m_view->insertText(QStringLiteral("a"));
    QTRY_COMPARE_WITH_TIMEOUT(countItems(model), 1, TIMEOUT);
}

void CompletionTest::testAbortCursorMovedOutOfRange()
{
    KateCompletionModel *model = m_view->completionWidget()->model();

    new CodeCompletionTestModel(m_view, QStringLiteral("a"));
    m_view->setCursorPosition(Cursor(0, 2));
    invokeCompletionBox(m_view);

    QCOMPARE(countItems(model), 14);
    QVERIFY(m_view->completionWidget()->isCompletionActive());

    m_view->setCursorPosition(Cursor(0, 4));
    verifyCompletionAborted(m_view);
}

void CompletionTest::testAbortInvalidText()
{
    KateCompletionModel *model = m_view->completionWidget()->model();

    new CodeCompletionTestModel(m_view, QStringLiteral("a"));
    m_view->setCursorPosition(Cursor(0, 2));
    invokeCompletionBox(m_view);

    QCOMPARE(countItems(model), 14);
    QVERIFY(m_view->completionWidget()->isCompletionActive());

    m_view->insertText(QStringLiteral("."));
    verifyCompletionAborted(m_view);
}

void CompletionTest::testCustomRange1()
{
    m_doc->setText(QStringLiteral("$aa bb cc\ndd"));
    KateCompletionModel *model = m_view->completionWidget()->model();

    CodeCompletionTestModel *testModel = new CustomRangeModel(m_view, QStringLiteral("$a"));
    m_view->setCursorPosition(Cursor(0, 3));
    invokeCompletionBox(m_view);

    Range complRange = *m_view->completionWidget()->completionRange(testModel);
    qDebug() << complRange;
    QCOMPARE(complRange, Range(Cursor(0, 0), Cursor(0, 3)));
    QCOMPARE(countItems(model), 14);

    m_view->insertText(QStringLiteral("a"));
    QTRY_COMPARE_WITH_TIMEOUT(countItems(model), 1, TIMEOUT);
}

void CompletionTest::testCustomRange2()
{
    m_doc->setText(QStringLiteral("$ bb cc\ndd"));
    KateCompletionModel *model = m_view->completionWidget()->model();

    CodeCompletionTestModel *testModel = new CustomRangeModel(m_view, QStringLiteral("$a"));
    m_view->setCursorPosition(Cursor(0, 1));
    invokeCompletionBox(m_view);

    Range complRange = *m_view->completionWidget()->completionRange(testModel);
    QCOMPARE(complRange, Range(Cursor(0, 0), Cursor(0, 1)));
    QCOMPARE(countItems(model), 40);

    m_view->insertText(QStringLiteral("aa"));
    QTRY_COMPARE_WITH_TIMEOUT(countItems(model), 14, TIMEOUT);
}

void CompletionTest::testCustomRangeMultipleModels()
{
    m_doc->setText(QStringLiteral("$a bb cc\ndd"));
    KateCompletionModel *model = m_view->completionWidget()->model();

    CodeCompletionTestModel *testModel1 = new CustomRangeModel(m_view, QStringLiteral("$a"));
    CodeCompletionTestModel *testModel2 = new CodeCompletionTestModel(m_view, QStringLiteral("a"));
    m_view->setCursorPosition(Cursor(0, 1));
    invokeCompletionBox(m_view);

    QCOMPARE(Range(*m_view->completionWidget()->completionRange(testModel1)), Range(Cursor(0, 0), Cursor(0, 2)));
    QCOMPARE(Range(*m_view->completionWidget()->completionRange(testModel2)), Range(Cursor(0, 1), Cursor(0, 2)));
    QCOMPARE(model->currentCompletion(testModel1), QStringLiteral("$"));
    QCOMPARE(model->currentCompletion(testModel2), QString());
    QCOMPARE(countItems(model), 80);

    m_view->insertText(QStringLiteral("aa"));
    QTRY_COMPARE_WITH_TIMEOUT(model->currentCompletion(testModel1), QStringLiteral("$aa"), TIMEOUT);
    QCOMPARE(model->currentCompletion(testModel2), QStringLiteral("aa"));
    QCOMPARE(countItems(model), 14 * 2);
}

void CompletionTest::testAbortController()
{
    KateCompletionModel *model = m_view->completionWidget()->model();

    new CustomRangeModel(m_view, QStringLiteral("$a"));
    m_view->setCursorPosition(Cursor(0, 0));
    invokeCompletionBox(m_view);

    QCOMPARE(countItems(model), 40);
    QVERIFY(m_view->completionWidget()->isCompletionActive());

    m_view->insertText(QStringLiteral("$a"));
    verifyCompletionStarted(m_view);

    m_view->insertText(QStringLiteral("."));
    verifyCompletionAborted(m_view);
}

void CompletionTest::testAbortControllerMultipleModels()
{
    KateCompletionModel *model = m_view->completionWidget()->model();

    CodeCompletionTestModel *testModel1 = new CodeCompletionTestModel(m_view, QStringLiteral("aa"));
    CodeCompletionTestModel *testModel2 = new CustomAbortModel(m_view, QStringLiteral("a-"));
    m_view->setCursorPosition(Cursor(0, 0));
    invokeCompletionBox(m_view);

    QCOMPARE(countItems(model), 80);
    QVERIFY(m_view->completionWidget()->isCompletionActive());

    m_view->insertText(QStringLiteral("a"));
    verifyCompletionStarted(m_view);
    QCOMPARE(countItems(model), 80);

    m_view->insertText(QStringLiteral("-"));
    verifyCompletionStarted(m_view);
    QVERIFY(!m_view->completionWidget()->completionRanges().contains(testModel1));
    QVERIFY(m_view->completionWidget()->completionRanges().contains(testModel2));

    QCOMPARE(countItems(model), 40);

    m_view->insertText(QLatin1String(" "));
    verifyCompletionAborted(m_view);
}

void CompletionTest::testEmptyFilterString()
{
    KateCompletionModel *model = m_view->completionWidget()->model();

    new EmptyFilterStringModel(m_view, QStringLiteral("aa"));
    m_view->setCursorPosition(Cursor(0, 0));
    invokeCompletionBox(m_view);

    QCOMPARE(countItems(model), 40);

    m_view->insertText(QStringLiteral("a"));
    QTRY_COMPARE_WITH_TIMEOUT(countItems(model), 40, TIMEOUT);

    m_view->insertText(QStringLiteral("bam"));
    QTRY_COMPARE_WITH_TIMEOUT(countItems(model), 40, TIMEOUT);
}

void CompletionTest::testUpdateCompletionRange()
{
    m_doc->setText(QStringLiteral("ab    bb cc\ndd"));
    KateCompletionModel *model = m_view->completionWidget()->model();

    CodeCompletionTestModel *testModel = new UpdateCompletionRangeModel(m_view, QStringLiteral("ab ab"));
    m_view->setCursorPosition(Cursor(0, 3));
    invokeCompletionBox(m_view);

    QCOMPARE(countItems(model), 40);
    QCOMPARE(Range(*m_view->completionWidget()->completionRange(testModel)), Range(Cursor(0, 3), Cursor(0, 3)));

    m_view->insertText(QStringLiteral("ab"));
    QTRY_COMPARE_WITH_TIMEOUT(Range(*m_view->completionWidget()->completionRange(testModel)), Range(Cursor(0, 0), Cursor(0, 5)), TIMEOUT);
    QTRY_COMPARE_WITH_TIMEOUT(countItems(model), 40, TIMEOUT);
}

void CompletionTest::testCustomStartCompl()
{
#ifdef __SANITIZE_ADDRESS__
    QSKIP("issues with Qt 6.5 and sanitizer");
#endif

    KateCompletionModel *model = m_view->completionWidget()->model();

    m_view->completionWidget()->setAutomaticInvocationDelay(1);

    new StartCompletionModel(m_view, QStringLiteral("aa"));

    m_view->setCursorPosition(Cursor(0, 0));
    m_view->insertText(QStringLiteral("%"));

    verifyCompletionStarted(m_view);
    QCOMPARE(countItems(model), 40);
}

void CompletionTest::testKateCompletionModel()
{
    KateCompletionModel *model = m_view->completionWidget()->model();
    CodeCompletionTestModel *testModel1 = new CodeCompletionTestModel(m_view, QStringLiteral("aa"));
    CodeCompletionTestModel *testModel2 = new CodeCompletionTestModel(m_view, QStringLiteral("bb"));

    model->setCompletionModel(testModel1);
    QCOMPARE(countItems(model), 40);

    model->addCompletionModel(testModel2);
    QCOMPARE(countItems(model), 80);

    model->removeCompletionModel(testModel2);
    QCOMPARE(countItems(model), 40);
}

void CompletionTest::testAbortImmideatelyAfterStart()
{
    new ImmideatelyAbortCompletionModel(m_view);
    m_view->setCursorPosition(Cursor(0, 3));
    QVERIFY(!m_view->completionWidget()->isCompletionActive());
    Q_EMIT m_view->userInvokedCompletion();
    QVERIFY(!m_view->completionWidget()->isCompletionActive());
}

void CompletionTest::testJumpToListBottomAfterCursorUpWhileAtTop()
{
    new CodeCompletionTestModel(m_view, QStringLiteral("aa"));
    invokeCompletionBox(m_view);

    m_view->completionWidget()->cursorUp();
    m_view->completionWidget()->bottom();
    // TODO - better way of finding the index?
    QCOMPARE(m_view->completionWidget()->treeView()->selectionModel()->currentIndex().row(), 39);
}

void CompletionTest::testAbbreviationEngine()
{
    int s = 0;
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBar"), QStringLiteral("fb"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBar"), QStringLiteral("foob"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBar"), QStringLiteral("fbar"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBar"), QStringLiteral("fba"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBar"), QStringLiteral("foba"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBarBazBang"), QStringLiteral("fbbb"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("foo_bar_cat"), QStringLiteral("fbc"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("foo_bar_cat"), QStringLiteral("fb"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBarArr"), QStringLiteral("fba"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBarArr"), QStringLiteral("fbara"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBarArr"), QStringLiteral("fobaar"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBarArr"), QStringLiteral("fb"), s));

    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("QualifiedIdentifier"), QStringLiteral("qid"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("QualifiedIdentifier"), QStringLiteral("qualid"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("QualifiedIdentifier"), QStringLiteral("qualidentifier"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("QualifiedIdentifier"), QStringLiteral("qi"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("KateCompletionModel"), QStringLiteral("kcmodel"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("KateCompletionModel"), QStringLiteral("kc"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("KateCompletionModel"), QStringLiteral("kcomplmodel"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("KateCompletionModel"), QStringLiteral("kacomplmodel"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("KateCompletionModel"), QStringLiteral("kacom"), s));

    QVERIFY(!KateCompletionModel::matchesAbbreviation(QStringLiteral("QualifiedIdentifier"), QStringLiteral("identifier"), s));
    QVERIFY(!KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBarArr"), QStringLiteral("fobaara"), s));
    QVERIFY(!KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBarArr"), QStringLiteral("fbac"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("KateCompletionModel"), QStringLiteral("kamodel"), s));

    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("AbcdefBcdefCdefDefEfFzZ"), QStringLiteral("AbcdefBcdefCdefDefEfFzZ"), s));
    QVERIFY(!KateCompletionModel::matchesAbbreviation(QStringLiteral("AbcdefBcdefCdefDefEfFzZ"), QStringLiteral("ABCDEFX"), s));
    QVERIFY(!KateCompletionModel::matchesAbbreviation(QStringLiteral("AaaaaaBbbbbCcccDddEeFzZ"), QStringLiteral("XZYBFA"), s));

    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBar"), QStringLiteral("fb"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("FooBar"), QStringLiteral("FB"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("KateCompletionModel"), QStringLiteral("kcmodel"), s));
    QVERIFY(KateCompletionModel::matchesAbbreviation(QStringLiteral("KateCompletionModel"), QStringLiteral("KCModel"), s));
}

void CompletionTest::testAutoCompletionPreselectFirst()
{
    new CodeCompletionTestModel(m_view, QStringLiteral("a"));

    m_view->config()->setValue(KateViewConfig::AutomaticCompletionPreselectFirst, false);
    // When AutomaticCompletionPreselectFirst is disabled, immediately pressing enter
    // should result into a newline instead of completion
    m_doc->clear();
    m_doc->insertText({0, 0}, QStringLiteral("a"));
    m_view->setCursorPosition(Cursor(0, 1));
    m_view->completionWidget()->automaticInvocation();
    verifyCompletionStarted(m_view);
    auto enterKeyEvent = QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    QApplication::sendEvent(m_view->focusProxy(), &enterKeyEvent);

    verifyCompletionAborted(m_view);
    QCOMPARE(m_doc->text(), QStringLiteral("a\n"));
}

void CompletionTest::testTabCompletion()
{
    new CodeCompletionTestModel(m_view, QStringLiteral("a"));

    m_view->config()->setValue(KateViewConfig::TabCompletion, true);

    // First entry already selected
    m_view->config()->setValue(KateViewConfig::AutomaticCompletionPreselectFirst, true);

    // Nothing to do, already selected
    m_doc->clear();
    m_doc->insertText({0, 0}, QStringLiteral("a"));
    m_view->completionWidget()->automaticInvocation();
    verifyCompletionStarted(m_view);
    m_view->completionWidget()->execute();
    QVERIFY(!m_view->completionWidget()->isCompletionActive());
    QCOMPARE(m_doc->text(), QStringLiteral("aaa0"));

    // First entry already selected, going down will select the next completion
    m_doc->clear();
    m_doc->insertText({0, 0}, QStringLiteral("a"));
    m_view->completionWidget()->automaticInvocation();
    verifyCompletionStarted(m_view);
    m_view->completionWidget()->tabCompletion(KateCompletionWidget::Down);
    m_view->completionWidget()->execute();
    QVERIFY(!m_view->completionWidget()->isCompletionActive());
    QCOMPARE(m_doc->text(), QStringLiteral("aad3"));

    // First entry _not_ already selected...
    m_view->config()->setValue(KateViewConfig::AutomaticCompletionPreselectFirst, false);

    m_doc->clear();
    m_doc->insertText({0, 0}, QStringLiteral("a"));
    m_view->completionWidget()->automaticInvocation();
    verifyCompletionStarted(m_view);
    // ... Tab will select the first entry
    m_view->completionWidget()->tabCompletion(KateCompletionWidget::Down);
    m_view->completionWidget()->execute();
    QVERIFY(!m_view->completionWidget()->isCompletionActive());
    QCOMPARE(m_doc->text(), QStringLiteral("aaa0"));

    // While at the top, going up, cycles to the bottom of the list
    m_doc->clear();
    m_doc->insertText({0, 0}, QStringLiteral("a"));
    m_view->completionWidget()->automaticInvocation();
    verifyCompletionStarted(m_view);
    m_view->completionWidget()->cursorDown(); // Select first entry
    m_view->completionWidget()->tabCompletion(KateCompletionWidget::Up);
    m_view->completionWidget()->execute();
    QVERIFY(!m_view->completionWidget()->isCompletionActive());
    QCOMPARE(m_doc->text(), QStringLiteral("ac\u008738"));

    // While at the bottom, going down cycles to the top of the list
    m_doc->clear();
    m_doc->insertText({0, 0}, QStringLiteral("a"));
    m_view->completionWidget()->automaticInvocation();
    verifyCompletionStarted(m_view);
    m_view->completionWidget()->cursorDown(); // Select first entry
    m_view->completionWidget()->tabCompletion(KateCompletionWidget::Up); // Go to bottom
    // While at the bottom, Tab goes to the top of the list
    m_view->completionWidget()->tabCompletion(KateCompletionWidget::Down);
    m_view->completionWidget()->execute();
    QVERIFY(!m_view->completionWidget()->isCompletionActive());
    QCOMPARE(m_doc->text(), QStringLiteral("aaa0"));
}

void CompletionTest::benchAbbreviationEngineGoodCase()
{
    int s = 0;
    QBENCHMARK {
        for (int i = 0; i < 10000; i++) {
            QVERIFY(!KateCompletionModel::matchesAbbreviation(QStringLiteral("AaaaaaBbbbbCcccDddEeFzZ"), QStringLiteral("XZYBFA"), s));
        }
    }
}

void CompletionTest::benchAbbreviationEngineNormalCase()
{
    int s = 0;
    QBENCHMARK {
        for (int i = 0; i < 10000; i++) {
            QVERIFY(!KateCompletionModel::matchesAbbreviation(QStringLiteral("AaaaaaBbbbbCcccDddEeFzZ"), QStringLiteral("ABCDEFX"), s));
        }
    }
}

void CompletionTest::benchAbbreviationEngineWorstCase()
{
    int s = 0;
    QBENCHMARK {
        for (int i = 0; i < 10000; i++) {
            // This case is quite horrible, because it requires a branch at every letter.
            // The current code will at some point drop out and just return false.
            KateCompletionModel::matchesAbbreviation(QStringLiteral("XxBbbbbbBbbbbbBbbbbBbbbBbbbbbbBbbbbbBbbbbbBbbbFox"),
                                                     QStringLiteral("XbbbbbbbbbbbbbbbbbbbbFx"),
                                                     s);
        }
    }
}

void CompletionTest::testAbbrevAndContainsMatching()
{
    KateCompletionModel *model = m_view->completionWidget()->model();

    new AbbreviationCodeCompletionTestModel(m_view, QString());

    m_view->document()->setText(QStringLiteral("SCA"));
    invokeCompletionBox(m_view);
    QCOMPARE(model->filteredItemCount(), (uint)6);

    m_view->document()->setText(QStringLiteral("SC"));
    invokeCompletionBox(m_view);
    QCOMPARE(model->filteredItemCount(), (uint)6);

    m_view->document()->setText(QStringLiteral("sca"));
    invokeCompletionBox(m_view);
    QCOMPARE(model->filteredItemCount(), (uint)6);

    m_view->document()->setText(QStringLiteral("contains"));
    invokeCompletionBox(m_view);
    QCOMPARE(model->filteredItemCount(), (uint)2);

    m_view->document()->setText(QStringLiteral("CONTAINS"));
    invokeCompletionBox(m_view);
    QCOMPARE(model->filteredItemCount(), (uint)2);

    m_view->document()->setText(QStringLiteral("containssome"));
    invokeCompletionBox(m_view);
    QCOMPARE(model->filteredItemCount(), (uint)1);

    m_view->document()->setText(QStringLiteral("matched"));
    m_view->userInvokedCompletion();
    QApplication::processEvents();
    QCOMPARE(model->filteredItemCount(), (uint)0);
}

void CompletionTest::testAsyncMatching()
{
    KateCompletionModel *model = m_view->completionWidget()->model();

    auto asyncModel = new AsyncCodeCompletionTestModel(m_view, QString());

    m_view->document()->setText(QStringLiteral("matched"));

    m_view->userInvokedCompletion();
    QApplication::processEvents();
    asyncModel->setItems({QStringLiteral("this_should_be_matched"), QStringLiteral("do_not_find_this")});
    QCOMPARE(model->filteredItemCount(), (uint)1);
}

void CompletionTest::benchCompletionModel()
{
    const int testFactor = 1;
    const QString text(QStringLiteral("abcdefg abcdef"));
    m_doc->setText(text);
    CodeCompletionTestModel *testModel1 = new CodeCompletionTestModel(m_view, QStringLiteral("abcdefg"));
    testModel1->setRowCount(50 * testFactor);
    CodeCompletionTestModel *testModel2 = new CodeCompletionTestModel(m_view, QStringLiteral("abcdef"));
    testModel2->setRowCount(50 * testFactor);
    CodeCompletionTestModel *testModel3 = new CodeCompletionTestModel(m_view, QStringLiteral("abcde"));
    testModel3->setRowCount(50 * testFactor);
    CodeCompletionTestModel *testModel4 = new CodeCompletionTestModel(m_view, QStringLiteral("abcd"));
    testModel4->setRowCount(500 * testFactor);
    QBENCHMARK_ONCE {
        for (int i = 0; i < text.size(); ++i) {
            m_view->setCursorPosition(Cursor(0, i));
            invokeCompletionBox(m_view);
        }
    }
}

#include "moc_completion_test.cpp"
