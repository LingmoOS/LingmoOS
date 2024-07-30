/*
    This file is part of the Kate project.
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katefoldingtest.h"

#include <katebuffer.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <katetextfolding.h>
#include <kateview.h>

#include <QJsonDocument>
#include <QStandardPaths>
#include <QTest>

#include <memory>

using namespace KTextEditor;

QTEST_MAIN(KateFoldingTest)

void KateFoldingTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void KateFoldingTest::cleanupTestCase()
{
}

// This is a unit test for bug 311866 (https://bugs.kde.org/show_bug.cgi?id=311866)
// It loads 5 lines of C++ code, places the cursor in line 4, and then folds
// the code.
// Expected behavior: the cursor should be moved so it stays visible
// Buggy behavior: the cursor is hidden, and moving the hidden cursor crashes kate
void KateFoldingTest::testCrash311866()
{
    KTextEditor::DocumentPrivate doc;
    const QUrl url = QUrl::fromLocalFile(QStringLiteral(TEST_DATA_DIR "bug311866.cpp"));
    doc.openUrl(url);
    doc.setHighlightingMode(QStringLiteral("C++"));
    doc.buffer().ensureHighlighted(6);

    const std::unique_ptr<ViewPrivate> view{static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr))};
    view->show();
    view->resize(400, 300);
    view->setCursorPosition(Cursor(3, 0));
    QTest::qWait(100);

    view->slotFoldToplevelNodes();
    doc.buffer().ensureHighlighted(6);

    qDebug() << "!!! Does the next line crash?";
    view->up();
}

// This test makes sure that,
// - if you have selected text
// - that spans a folded range,
// - and the cursor is at the end of the text selection,
// - and you type a char, e.g. 'x',
// then the resulting text is correct, and changing region
// visibility does not mess around with the text cursor.
//
// See https://bugs.kde.org/show_bug.cgi?id=295632
void KateFoldingTest::testBug295632()
{
    KTextEditor::DocumentPrivate doc;
    QString text = QStringLiteral(
        "oooossssssss\n"
        "{\n"
        "\n"
        "}\n"
        "ssssss----------");
    doc.setText(text);

    // view must be visible...
    const std::unique_ptr<ViewPrivate> view{static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr))};
    view->show();
    view->resize(400, 300);

    qint64 foldId = view->textFolding().newFoldingRange(KTextEditor::Range(1, 0, 3, 1));
    view->textFolding().foldRange(foldId);
    QVERIFY(view->textFolding().isLineVisible(0));
    QVERIFY(view->textFolding().isLineVisible(1));
    QVERIFY(!view->textFolding().isLineVisible(2));
    QVERIFY(!view->textFolding().isLineVisible(3));
    QVERIFY(view->textFolding().isLineVisible(4));

    view->setSelection(Range(Cursor(0, 4), Cursor(4, 6)));
    view->setCursorPosition(Cursor(4, 6));

    QTest::qWait(100);
    doc.typeChars(view.get(), QStringLiteral("x"));
    QTest::qWait(100);

    QString line = doc.line(0);
    QCOMPARE(line, QStringLiteral("oooox----------"));
}

// This testcase tests the following:
// - the cursor is first set into the word 'hello'
// - then lines 0-3 are folded.
// - the real text cursor is still in the word 'hello'
// - the important issue is: The display cursor must be in the visible line range
// --> if this test passes, KateViewInternal's m_displayCursor is properly adapted.
void KateFoldingTest::testCrash367466()
{
    KTextEditor::DocumentPrivate doc;

    // we use only x to have equal width characters, else we fail for non-fixed width fonts
    QString text = QStringLiteral(
        "xxxx xxxx\n"
        "\n"
        "\n"
        "xxxx xxx\n"
        "xxxxx\n"
        "xxxxx\n");
    doc.setText(text);

    // view must be visible...
    const std::unique_ptr<ViewPrivate> view{static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr))};
    view->show();
    view->resize(400, 300);
    view->setCursorPosition(KTextEditor::Cursor(5, 2));
    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(5, 2));
    qint64 foldId = view->textFolding().newFoldingRange(KTextEditor::Range(0, 0, 3, 8));

    view->textFolding().foldRange(foldId);
    QVERIFY(view->textFolding().isLineVisible(0));
    QVERIFY(!view->textFolding().isLineVisible(1));
    QVERIFY(!view->textFolding().isLineVisible(2));
    QVERIFY(!view->textFolding().isLineVisible(3));
    QVERIFY(view->textFolding().isLineVisible(4));
    QVERIFY(view->textFolding().isLineVisible(5));

    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(5, 2));
    view->up();
    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(4, 2));
}

void KateFoldingTest::testUnfoldingInImportFoldingRanges()
{
    DocumentPrivate doc;
    const auto text = QStringLiteral(
        "int f(bool one) {\n"
        "    if (one) {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n"
        "\n"
        "int g() {\n"
        "    return 123;\n"
        "}\n");
    doc.setText(text);

    // view must be visible...
    const std::unique_ptr<ViewPrivate> view{static_cast<ViewPrivate *>(doc.createView(nullptr))};
    view->show();
    view->resize(400, 300);

    auto &textFolding = view->textFolding();

    const auto addFoldedRange = [&textFolding](Range range, Kate::TextFolding::FoldingRangeFlags extraFlags = {}) {
        textFolding.newFoldingRange(range, Kate::TextFolding::Folded | extraFlags);
    };
    addFoldedRange(Range(0, 16, 6, 1)); // f()
    addFoldedRange(Range(8, 8, 10, 1)); // g()
    addFoldedRange(Range(1, 13, 3, 5), Kate::TextFolding::Persistent); // if
    addFoldedRange(Range(3, 11, 5, 5)); // else

    textFolding.importFoldingRanges(QJsonDocument{});
    // TextFolding::importFoldingRanges() should remove all existing folding ranges
    // - both top-level and nested -  before importing new ones.
    QCOMPARE(textFolding.debugDump(), QLatin1String("tree  - folded "));
}

#include "moc_katefoldingtest.cpp"
