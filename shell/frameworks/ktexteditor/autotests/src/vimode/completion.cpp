/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "completion.h"
#include "vimode/globalstate.h"
#include "vimode/mappings.h"
#include <katecompletionwidget.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <kateglobal.h>
#include <kateview.h>
#include <katewordcompletion.h>
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>

#include <QMainWindow>
#include <QTest>

using namespace KTextEditor;
using KateVi::Mappings;

QTEST_MAIN(CompletionTest)

// BEGIN: VimCodeCompletionTestModel

VimCodeCompletionTestModel::VimCodeCompletionTestModel(KTextEditor::View *parent)
    : KTextEditor::CodeCompletionModel(parent)
{
    setRowCount(3);
    parent->setAutomaticInvocationEnabled(true);
    // It would add additional items and we don't want that in tests
    parent->unregisterCompletionModel(KTextEditor::EditorPrivate::self()->wordCompletionModel());
    parent->registerCompletionModel(this);
}

QVariant VimCodeCompletionTestModel::data(const QModelIndex &index, int role) const
{
    // Order is important, here, as the completion widget seems to do its own sorting.
    const char *completions[] = {"completion1", "completion2", "completion3"};
    if (role == Qt::DisplayRole) {
        if (index.column() == Name) {
            return QString::fromUtf8(completions[index.row()]);
        }
    }
    return QVariant();
}

// END: VimCodeCompletionTestModel

// BEGIN: CodeCompletionInterface

FailTestOnInvocationModel::FailTestOnInvocationModel(KTextEditor::View *parent)
    : KTextEditor::CodeCompletionModel(parent)
{
    setRowCount(3);
    parent->setAutomaticInvocationEnabled(true);
    // It would add additional items and we don't want that in tests.
    parent->unregisterCompletionModel(EditorPrivate::self()->wordCompletionModel());
    parent->registerCompletionModel(this);
}

QVariant FailTestOnInvocationModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);
    Q_UNUSED(role);

    failTest();
    return QVariant();
}

void FailTestOnInvocationModel::failTest() const
{
    QFAIL("Shouldn't be invoking me!");
}

// END: CodeCompletionInterface

// BEGIN: CompletionTest

void CompletionTest::FakeCodeCompletionTests()
{
    // Test that FakeCodeCompletionTestModel behaves similar to the code-completion in e.g. KDevelop.
    const bool oldStealKeys = KateViewConfig::global()->viInputModeStealKeys();
    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, true); // For Ctrl-P, Ctrl-N etc
    ensureKateViewVisible(); // KTextEditor::ViewPrivate needs to be visible for the completion widget.
    FakeCodeCompletionTestModel *fakeCodeCompletionModel = new FakeCodeCompletionTestModel(kate_view);
    kate_view->registerCompletionModel(fakeCodeCompletionModel);
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("completionB"), QStringLiteral("completionC")});
    DoTest("", ("i\\ctrl-p\\enter"), ("completionC"));
    DoTest("", ("i\\ctrl-p\\ctrl-p\\enter"), ("completionB"));
    DoTest("", ("i\\ctrl-p\\ctrl-p\\ctrl-p\\enter"), ("completionA"));
    DoTest("", ("i\\ctrl-p\\ctrl-p\\ctrl-p\\ctrl-p\\enter"), ("completionC"));
    // If no word before cursor, don't delete any text.
    BeginTest(QString());
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("i\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.length(), 1);
    FinishTest(("completionA"));
    // Apparently, we must delete the word before the cursor upon completion
    // (even if we replace it with identical text!)
    BeginTest(QStringLiteral("compl"));
    TestPressKey(QStringLiteral("ea"));
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 2);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextRemoved);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 0), Cursor(0, 5)));
    QCOMPARE(m_docChanges[1].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 0), Cursor(0, 11)));
    QCOMPARE(m_docChanges[1].newText(), QStringLiteral("completionA"));
    FinishTest(("completionA"));
    // A "word" is currently alphanumeric, plus underscore.
    fakeCodeCompletionModel->setCompletions({QStringLiteral("w_123completion")});
    BeginTest(QStringLiteral("(w_123"));
    TestPressKey(QStringLiteral("ea"));
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 2);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextRemoved);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 1), Cursor(0, 6)));
    QCOMPARE(m_docChanges[1].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 1), Cursor(0, 16)));
    QCOMPARE(m_docChanges[1].newText(), QString(QStringLiteral("w_123completion")));
    FinishTest(("(w_123completion"));
    // "Removing tail on complete" is apparently done in three stages:
    // delete word up to the cursor; insert new word; then delete remainder.
    const bool oldRemoveTailOnCompletion = KateViewConfig::global()->wordCompletionRemoveTail();
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    BeginTest(QStringLiteral("(w_123comp"));
    TestPressKey(QStringLiteral("6li"));
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    FinishTest(("(w_123completion"));

    // If we don't remove tail, just delete up to the cursor and insert.
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
    fakeCodeCompletionModel->setRemoveTailOnComplete(false);
    BeginTest(QStringLiteral("(w_123comp"));
    TestPressKey(QStringLiteral("6li"));
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    FinishTest("(w_123completioncomp");

    // If no opening bracket after the cursor, a function taking no arguments
    // is added as "function()", and the cursor placed after the closing ")".
    // The addition of "function()" is done in two steps: first "function", then "()".
    BeginTest(QStringLiteral("object->"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall()")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("$a\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 2);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[0].newText(), QStringLiteral("functionCall"));
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 20), Cursor(0, 22)));
    QCOMPARE(m_docChanges[1].newText(), QStringLiteral("()"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall()X");

    // If no opening bracket after the cursor, a function taking at least one argument
    // is added as "function()", and the cursor placed after the opening "(".
    // The addition of "function()" is done in two steps: first "function", then "()".
    qDebug() << "Fleep";
    BeginTest(QStringLiteral("object->"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall(...)")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("$a\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 2);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[0].newText(), QStringLiteral("functionCall"));
    QCOMPARE(m_docChanges[1].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 20), Cursor(0, 22)));
    QCOMPARE(m_docChanges[1].newText(), QStringLiteral("()"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall(X)");

    // If there is an opening bracket after the cursor, we merge the function call
    // with that.
    // Even if the function takes no arguments, we still place the cursor after the opening bracket,
    // in contrast to the case where there is no opening bracket after the cursor.
    // No brackets are added.  No removals occur if there is no word before the cursor.
    BeginTest(QStringLiteral("object->("));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall()")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("f(i\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 1);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[0].newText(), QStringLiteral("functionCall"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall(X");

    // There can't be any non-whitespace between cursor position and opening bracket, though!
    BeginTest(QStringLiteral("object->|(   ("));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall()")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("f>a\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 2);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[0].newText(), QStringLiteral("functionCall"));
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 20), Cursor(0, 22)));
    QCOMPARE(m_docChanges[1].newText(), QStringLiteral("()"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall()X|(   (");

    // Whitespace before the bracket is fine, though.
    BeginTest(QStringLiteral("object->    (<-Cursor here!"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall()")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("f>a\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 1);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[0].newText(), QStringLiteral("functionCall"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall    (X<-Cursor here!");

    // Be careful with positioning the cursor if we delete leading text!
    BeginTest(QStringLiteral("object->    (<-Cursor here!"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall()")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("f>afunct"));
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 2);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextRemoved);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 13)));
    QCOMPARE(m_docChanges[1].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[1].newText(), QStringLiteral("functionCall"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall    (X<-Cursor here!");

    // If we're removing tail on complete, it's whether there is a suitable opening
    // bracket *after* the word (not the cursor) that's important.
    BeginTest(QStringLiteral("object->function    (<-Cursor here!"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall()")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("12li")); // Start inserting before the "t" in "function"
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall    (X<-Cursor here!");

    // Repeat of bracket-merging stuff, this time for functions that take at least one argument.
    BeginTest(QStringLiteral("object->("));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall(...)")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("f(i\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 1);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextInserted);
    qDebug() << "Range: " << m_docChanges[0].changeRange();
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[0].newText(), QStringLiteral("functionCall"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall(X");

    // There can't be any non-whitespace between cursor position and opening bracket, though!
    BeginTest(QStringLiteral("object->|(   ("));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall(...)")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("f>a\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 2);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[0].newText(), QStringLiteral("functionCall"));
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 20), Cursor(0, 22)));
    QCOMPARE(m_docChanges[1].newText(), QStringLiteral("()"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall(X)|(   (");

    // Whitespace before the bracket is fine, though.
    BeginTest(QStringLiteral("object->    (<-Cursor here!"));
    qDebug() << "NooooO";
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall(...)")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("f>a\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 1);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[0].newText(), QStringLiteral("functionCall"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall    (X<-Cursor here!");

    // Be careful with positioning the cursor if we delete leading text!
    BeginTest(QStringLiteral("object->    (<-Cursor here!"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall(...)")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("f>afunct"));
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 2);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextRemoved);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 8), Cursor(0, 13)));
    QCOMPARE(m_docChanges[1].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 8), Cursor(0, 20)));
    QCOMPARE(m_docChanges[1].newText(), QStringLiteral("functionCall"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall    (X<-Cursor here!");

    // If we're removing tail on complete, it's whether there is a suitable opening
    // bracket *after* the word (not the cursor) that's important.
    BeginTest(QStringLiteral("object->function    (<-Cursor here!"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall(...)")});
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("12li")); // Start inserting before the "t" in "function"
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    TestPressKey(QStringLiteral("X"));
    FinishTest("object->functionCall    (X<-Cursor here!");

    // Deal with function completions which add a ";".
    BeginTest(QString());
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall();")});
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("ifun"));
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 3);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextRemoved);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 0), Cursor(0, 3)));
    QCOMPARE(m_docChanges[1].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 0), Cursor(0, 12)));
    QCOMPARE(m_docChanges[1].newText(), QStringLiteral("functionCall"));
    QCOMPARE(m_docChanges[2].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[2].changeRange(), Range(Cursor(0, 12), Cursor(0, 15)));
    QCOMPARE(m_docChanges[2].newText(), QStringLiteral("();"));
    FinishTest("functionCall();");

    BeginTest(QString());
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall();")});
    TestPressKey(QStringLiteral("ifun\\ctrl- \\enterX"));
    FinishTest("functionCall();X");

    BeginTest(QString());
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall(...);")});
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("ifun"));
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("\\ctrl- \\enter"));
    QCOMPARE(m_docChanges.size(), 3);
    QCOMPARE(m_docChanges[0].changeType(), DocChange::TextRemoved);
    QCOMPARE(m_docChanges[0].changeRange(), Range(Cursor(0, 0), Cursor(0, 3)));
    QCOMPARE(m_docChanges[1].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[1].changeRange(), Range(Cursor(0, 0), Cursor(0, 12)));
    QCOMPARE(m_docChanges[1].newText(), QStringLiteral("functionCall"));
    QCOMPARE(m_docChanges[2].changeType(), DocChange::TextInserted);
    QCOMPARE(m_docChanges[2].changeRange(), Range(Cursor(0, 12), Cursor(0, 15)));
    QCOMPARE(m_docChanges[2].newText(), QStringLiteral("();"));
    FinishTest("functionCall();");

    BeginTest(QString());
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall(...);")});
    TestPressKey(QStringLiteral("ifun\\ctrl- \\enterX"));
    FinishTest("functionCall(X);");

    // Completions ending with ";" do not participate in bracket merging.
    BeginTest(QStringLiteral("(<-old bracket"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall();")});
    TestPressKey(QStringLiteral("ifun\\ctrl- \\enterX"));
    FinishTest("functionCall();X(<-old bracket");
    BeginTest(QStringLiteral("(<-old bracket"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("functionCall(...);")});
    TestPressKey(QStringLiteral("ifun\\ctrl- \\enterX"));
    FinishTest("functionCall(X);(<-old bracket");

    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, oldRemoveTailOnCompletion);
    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, oldStealKeys);
    kate_view->hide();
    mainWindow->hide();
    kate_view->unregisterCompletionModel(fakeCodeCompletionModel);
    delete fakeCodeCompletionModel;
}

void CompletionTest::CompletionTests()
{
    const bool oldRemoveTailOnCompletion = KateViewConfig::global()->wordCompletionRemoveTail();
    // For these tests, assume we don't swallow the tail on completion.
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);

    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, true); // For Ctrl-P, Ctrl-N etc
    ensureKateViewVisible(); // KTextEditor::ViewPrivate needs to be visible for the completion widget.
    VimCodeCompletionTestModel *testModel = new VimCodeCompletionTestModel(kate_view);

    BeginTest(QString());
    TestPressKey(QStringLiteral("i\\ctrl-p"));
    waitForCompletionWidgetToActivate();
    TestPressKey(QStringLiteral("\\return"));
    FinishTest("completion3");

    BeginTest(QString());
    TestPressKey(QStringLiteral("i\\ctrl- "));
    waitForCompletionWidgetToActivate();
    TestPressKey(QStringLiteral("\\return"));
    FinishTest("completion1");

    BeginTest(QString());
    TestPressKey(QStringLiteral("i\\ctrl-n"));
    waitForCompletionWidgetToActivate();
    TestPressKey(QStringLiteral("\\return"));
    FinishTest("completion1");

    // Test wraps around from top to bottom.
    BeginTest(QString());
    TestPressKey(QStringLiteral("i\\ctrl- \\ctrl-p"));
    waitForCompletionWidgetToActivate();
    TestPressKey(QStringLiteral("\\return"));
    FinishTest("completion3");

    // Test wraps around from bottom to top.
    BeginTest(QString());
    TestPressKey(QStringLiteral("i\\ctrl- \\ctrl-n\\ctrl-n\\ctrl-n"));
    waitForCompletionWidgetToActivate();
    TestPressKey(QStringLiteral("\\return"));
    FinishTest("completion1");

    // Test does not re-invoke completion when doing a "." repeat.
    BeginTest(QString());
    TestPressKey(QStringLiteral("i\\ctrl- "));
    waitForCompletionWidgetToActivate();
    TestPressKey(QStringLiteral("\\return\\ctrl-c"));
    kate_view->unregisterCompletionModel(testModel);
    FailTestOnInvocationModel *failsTestOnInvocation = new FailTestOnInvocationModel(kate_view);
    TestPressKey(QStringLiteral("gg."));
    FinishTest("completion1completion1");
    kate_view->unregisterCompletionModel(failsTestOnInvocation);
    kate_view->registerCompletionModel(testModel);

    // Test that the full completion is repeated when repeat an insert that uses completion,
    // where the completion list was not manually invoked.
    BeginTest(QString());
    TestPressKey(QStringLiteral("i"));
    // Simulate "automatic" invoking of completion.
    kate_view->completionWidget()->userInvokedCompletion();
    waitForCompletionWidgetToActivate();
    TestPressKey(QStringLiteral("\\return\\ctrl-cgg."));
    FinishTest("completion1completion1");

    clearAllMappings();
    // Make sure the "Enter"/ "Return" used when invoking completions is not swallowed before being
    // passed to the key mapper.
    kate_view->registerCompletionModel(testModel);
    vi_global->mappings()->add(Mappings::InsertModeMapping, QStringLiteral("cb"), QStringLiteral("mapped-shouldntbehere"), Mappings::Recursive);
    BeginTest(QStringLiteral(""));
    TestPressKey(QStringLiteral("ic"));
    kate_view->userInvokedCompletion();
    waitForCompletionWidgetToActivate();
    QVERIFY(kate_view->completionWidget()->isCompletionActive());
    TestPressKey(QStringLiteral("\\enterb"));
    FinishTest("completion1b");
    BeginTest(QString());
    TestPressKey(QStringLiteral("ic"));
    kate_view->userInvokedCompletion();
    waitForCompletionWidgetToActivate();
    QVERIFY(kate_view->completionWidget()->isCompletionActive());
    TestPressKey(QStringLiteral("\\returnb"));
    FinishTest("completion1b");

    // Make sure the completion widget is dismissed on ESC, ctrl-c and ctrl-[.
    BeginTest(QString());
    TestPressKey(QStringLiteral("ic"));
    kate_view->userInvokedCompletion();
    waitForCompletionWidgetToActivate();
    QVERIFY(kate_view->completionWidget()->isCompletionActive());
    TestPressKey(QStringLiteral("\\esc"));
    QVERIFY(!kate_view->completionWidget()->isCompletionActive());
    FinishTest("c");
    BeginTest(QString());
    TestPressKey(QStringLiteral("ic"));
    kate_view->userInvokedCompletion();
    waitForCompletionWidgetToActivate();
    QVERIFY(kate_view->completionWidget()->isCompletionActive());
    TestPressKey(QStringLiteral("\\ctrl-c"));
    QVERIFY(!kate_view->completionWidget()->isCompletionActive());
    FinishTest("c");
    BeginTest(QString());
    TestPressKey(QStringLiteral("ic"));
    kate_view->userInvokedCompletion();
    waitForCompletionWidgetToActivate();
    QVERIFY(kate_view->completionWidget()->isCompletionActive());
    TestPressKey(QStringLiteral("\\ctrl-["));
    QVERIFY(!kate_view->completionWidget()->isCompletionActive());
    FinishTest("c");
    kate_view->unregisterCompletionModel(testModel);

    // Check that the repeat-last-change handles Completions in the same way as Macros do
    // i.e. fairly intelligently :)
    FakeCodeCompletionTestModel *fakeCodeCompletionModel = new FakeCodeCompletionTestModel(kate_view);
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    kate_view->registerCompletionModel(fakeCodeCompletionModel);
    clearTrackedDocumentChanges();
    clearAllMacros();
    BeginTest(QStringLiteral("funct\nnoa\ncomtail\ncomtail"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA"), QStringLiteral("functionwithargs(...)"), QStringLiteral("noargfunction()")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    // Record 'a'.
    TestPressKey(QStringLiteral("i\\right\\right\\right\\right\\right\\ctrl- \\enterfirstArg")); // Function with args.
    TestPressKey(QStringLiteral("\\home\\down\\right\\right\\right\\ctrl- \\enter")); // Function no args.
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    TestPressKey(QStringLiteral("\\home\\down\\right\\right\\right\\ctrl- \\enter")); // Cut off tail.
    fakeCodeCompletionModel->setRemoveTailOnComplete(false);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, false);
    TestPressKey(QStringLiteral("\\home\\down\\right\\right\\right\\ctrl- \\enter\\ctrl-c")); // Don't cut off tail.
    fakeCodeCompletionModel->setRemoveTailOnComplete(true);
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, true);
    FinishTest("functionwithargs(firstArg)\nnoargfunction()\ncompletionA\ncompletionAtail");

    // Replay.
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    kate_document->setText(QStringLiteral("funct\nnoa\ncomtail\ncomtail"));
    clearTrackedDocumentChanges();
    TestPressKey(QStringLiteral("gg."));
    FinishTest("functionwithargs(firstArg)\nnoargfunction()\ncompletionA\ncompletionAtail");

    // Clear our log of completions for each change.
    BeginTest(QString());
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionA")});
    fakeCodeCompletionModel->setFailTestOnInvocation(false);
    TestPressKey(QStringLiteral("ciw\\ctrl- \\enter\\ctrl-c"));
    fakeCodeCompletionModel->setCompletions({QStringLiteral("completionB")});
    TestPressKey(QStringLiteral("ciw\\ctrl- \\enter\\ctrl-c"));
    fakeCodeCompletionModel->setFailTestOnInvocation(true);
    TestPressKey(QStringLiteral("."));
    FinishTest("completionB");

    kate_view->unregisterCompletionModel(fakeCodeCompletionModel);
    delete fakeCodeCompletionModel;
    KateViewConfig::global()->setValue(KateViewConfig::WordCompletionRemoveTail, oldRemoveTailOnCompletion);

    // Hide the kate_view for subsequent tests.
    kate_view->hide();
    mainWindow->hide();
}

void CompletionTest::waitForCompletionWidgetToActivate()
{
    BaseTest::waitForCompletionWidgetToActivate(kate_view);
}

void CompletionTest::clearTrackedDocumentChanges()
{
    m_docChanges.clear();
}

#include "moc_completion.cpp"
// END: CompletionTest
