/*
    This file is part of the Kate project.

    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "camelcursortest.h"

#include <katedocument.h>
#include <kateview.h>

#include <QTest>

QTEST_MAIN(CamelCursorTest)

CamelCursorTest::CamelCursorTest(QObject *parent)
    : QObject(parent)
{
    doc = new KTextEditor::DocumentPrivate(false, false);
    view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));
    view->resize(100, 100);
    view->show();
}

CamelCursorTest::~CamelCursorTest()
{
    delete doc;
}

void CamelCursorTest::testWordMovementSingleRow_data()
{
    /**
     * NOTE: If you are here to fix a bug, try not to add a new function please.
     * Instead consider adding a row here with the label as your bug-number.
     *
     * The other test cases are small because if this works, they will work automaitcally
     * since the core functionality is correct.
     */

    // The text for document
    QTest::addColumn<QString>("text");
    // The number of Ctrl + Right/Left movements
    QTest::addColumn<int>("movements");
    // The expected positions where your cursor is supposed to land
    QTest::addColumn<QList<int>>("colPos");
    // Highlighting mode
    QTest::addColumn<QString>("HlMode");

    // clang-format off
    // row name                                         text                                no of mov       expected positions
    QTest::addRow("KateView")             << QStringLiteral("KateView")                     << 2 << QList<int>{4, 8} << QStringLiteral("C++");
    QTest::addRow("Q_LOGGING_CATEGORY")   << QStringLiteral("Q_LOGGING_CATEGORY();")        << 4 << QList<int>{2, 10, 18, 21} << QStringLiteral("C");
    QTest::addRow("Q_L11GING_CATEG0RY")   << QStringLiteral("Q_L11GING_CATEG0RY();")        << 7 << QList<int>{2, 5, 10, 14, 16, 18, 21} << QString();
    QTest::addRow("snake_case_name")      << QStringLiteral("int snake_case_name = 123;")   << 7 << QList<int>{4, 10, 15, 20, 22, 25, 26} << QString();
    QTest::addRow("bad___SNAKE_case__")   << QStringLiteral("int bad___SNAKE_case__ = 11;") << 7 << QList<int>{4, 10, 16, 23, 25, 27, 28} << QString();
    QTest::addRow("QApplication")         << QStringLiteral("QApplication app;")            << 4 << QList<int>{1, 13, 16, 17} << QString();
    QTest::addRow("ABCDead")              << QStringLiteral("ABCDead")                      << 2 << QList<int>{3, 7} << QString();
    QTest::addRow("SE_CheckBoxIndicator") << QStringLiteral("QStyle::SE_CheckBoxIndicator") << 7 << QList<int>{1, 6, 8, 11, 16, 19, 28} << QStringLiteral("C++");
    QTest::addRow("SE_CHECKBoxIndicator") << QStringLiteral("QStyle::SE_CHECKBoxIndicator") << 7 << QList<int>{1, 6, 8, 11, 16, 19, 28} << QString();
    QTest::addRow("SE_CHECKBOXINDICATOR") << QStringLiteral("QStyle::SE_CHECKBOXINDICATOR") << 5 << QList<int>{1, 6, 8, 11, 28} << QString();
    QTest::addRow("abc0_asd")             << QStringLiteral("int abc0_asd")                 << 3 << QList<int>{4, 9, 12} << QString();
    QTest::addRow("abc120_aSD")           << QStringLiteral("int abc120_aSD")               << 4 << QList<int>{4, 11, 12, 14} << QString();
    QTest::addRow("aQQ_OPEN")             << QStringLiteral("aQQ_OPEN")                     << 3 << QList<int>{1, 4, 8} << QString();
    QTest::addRow("aQQ_OPEN")             << QStringLiteral("    aQQ_OPEN")                 << 4 << QList<int>{4, 5, 8, 12} << QString();

    // PHP stuff that starts with $
    doc->setHighlightingMode(QStringLiteral("PHP/PHP"));
    QTest::addRow("$phpVar")              << QStringLiteral("$phpVar = 0;")                 << 6 << QList<int>{1, 4, 8, 10, 11, 12} << QStringLiteral("PHP/PHP");
    QTest::addRow("$php_Var")             << QStringLiteral("$php_Var = 0;")                << 6 << QList<int>{1, 5, 9, 11, 12, 13} << QStringLiteral("PHP/PHP");
    QTest::addRow("$_SESSION")            << QStringLiteral("$_SESSION[\"some\"]")          << 6 << QList<int>{1, 2, 9, 11, 15, 17} << QStringLiteral("PHP/PHP");

    // CSS Color
    QTest::addRow("#ff00ff")              << QStringLiteral("#ff00ff")                      << 2 << QList<int>{1, 7} << QStringLiteral("CSS");
    QTest::addRow("#00FF00")              << QStringLiteral("#00FF00")                      << 4 << QList<int>{1, 3, 4, 7} << QStringLiteral("HTML");

    QTest::addRow("Bug 448692")             << QStringLiteral("asdf a")                     << 2 << QList<int>{5, 6} << QString();
    // unicode
    QTest::addRow("unicode")             << QStringLiteral("𝗺𝗮𝘆∕𝗺𝗶𝗴𝗵𝘁")                   << 3 << QList<int>{6, 7, 17} << QString();

    // clang-format on
}

void CamelCursorTest::testWordMovementSingleRow()
{
    QFETCH(QString, text);
    QFETCH(int, movements);
    QFETCH(QList<int>, colPos);
    QFETCH(QString, HlMode);

    doc->setHighlightingMode(HlMode);

    doc->setText(text);
    view->setCursorPosition({0, 0});

    for (int i = 0; i < movements; ++i) {
        view->wordRight();
        QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(0, colPos.at(i)));
    }

    // reverse jumping. Reverse everything but last
    // element. Last element will be set to 0 as that
    // is our start pos
    std::reverse(colPos.begin(), colPos.end() - 1);
    colPos.back() = 0;

    for (int i = 0; i < movements; ++i) {
        view->wordLeft();
        QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(0, colPos.at(i)));
    }

    QCOMPARE(view->cursorPosition(), KTextEditor::Cursor(0, 0));
    doc->setHighlightingMode(QString());
}

void CamelCursorTest::testRtlWordMovement()
{
    doc->setText(QStringLiteral("اردو کا جملہ"));
    view->setCursorPosition({0, 0});

    // for RTL we move left
    auto pos = QList<int>{5, 8, 12};
    for (int i = 0; i < 3; ++i) {
        view->wordLeft();
        QCOMPARE(view->cursorPosition().column(), pos.at(i));
    }

    // now reverse to original pos
    pos = QList<int>{8, 5, 0};
    for (int i = 0; i < 3; ++i) {
        view->wordRight();
        QCOMPARE(view->cursorPosition().column(), pos.at(i));
    }
}

void CamelCursorTest::testWordMovementMultipleRow_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("movements");
    QTest::addColumn<QList<KTextEditor::Cursor>>("expect_cursor");

    using C = KTextEditor::Cursor;
    QTest::addRow("2 lines") << QStringLiteral("KateView\nnextLine") << 4 << QList<C>{C(0, 4), C(0, 8), C(1, 0), C(1, 4)};
    QTest::addRow("2 line caps") << QStringLiteral("Kate_VIEW\nNextLINE") << 4 << QList<C>{C(0, 5), C(0, 9), C(1, 0), C(1, 4)};
    QTest::addRow("4 lines") << QStringLiteral("Kate\nView\nNext\nLINE") << 7 << QList<C>{C(0, 4), C(1, 0), C(1, 4), C(2, 0), C(2, 4), C(3, 0), C(3, 4)};
}

void CamelCursorTest::testWordMovementMultipleRow()
{
    QFETCH(QString, text);
    QFETCH(int, movements);
    QFETCH(QList<KTextEditor::Cursor>, expect_cursor);

    doc->setText(text);
    view->setCursorPosition({0, 0});

    for (int i = 0; i < movements; ++i) {
        view->wordRight();
        QCOMPARE(view->cursorPosition(), expect_cursor.at(i));
    }

    std::reverse(expect_cursor.begin(), expect_cursor.end() - 1);
    expect_cursor.back() = KTextEditor::Cursor(0, 0);

    for (int i = 0; i < movements; ++i) {
        view->wordLeft();
        QCOMPARE(view->cursorPosition(), expect_cursor.at(i));
    }
}

void CamelCursorTest::testDeletionRight()
{
    doc->setText(QStringLiteral("SomeWord"));
    view->setCursorPosition({0, 0});

    view->deleteWordRight();
    QCOMPARE(doc->text(), QStringLiteral("Word"));
    view->deleteWordRight();
    QCOMPARE(doc->text(), QString());

    doc->setText(QStringLiteral("Some Word"));
    view->setCursorPosition({0, 0});

    view->deleteWordRight();
    QCOMPARE(doc->text(), QStringLiteral("Word"));
    view->deleteWordRight();
    QCOMPARE(doc->text(), QString());

    doc->setText(QStringLiteral("Some_WORD"));
    view->setCursorPosition({0, 0});

    view->deleteWordRight();
    QCOMPARE(doc->text(), QStringLiteral("WORD"));
    view->deleteWordRight();
    QCOMPARE(doc->text(), QString());

    doc->setText(QStringLiteral("Some      WORD"));
    view->setCursorPosition({0, 0});

    view->deleteWordRight();
    QCOMPARE(doc->text(), QStringLiteral("WORD"));
    view->deleteWordRight();
    QCOMPARE(doc->text(), QString());
}

void CamelCursorTest::testDeletionLeft()
{
    doc->setText(QStringLiteral("SomeWord"));
    view->setCursorPosition({0, 8});
    view->deleteWordLeft();
    QCOMPARE(doc->text(), QStringLiteral("Some"));
    view->deleteWordLeft();
    QCOMPARE(doc->text(), QString());

    doc->setText(QStringLiteral("Some Word"));
    view->setCursorPosition({0, 9});
    view->deleteWordLeft();
    QCOMPARE(doc->text(), QStringLiteral("Some "));
    view->deleteWordLeft();
    QCOMPARE(doc->text(), QString());

    doc->setText(QStringLiteral("Some_WORD"));
    view->setCursorPosition({0, 9});
    view->deleteWordLeft();
    QCOMPARE(doc->text(), QStringLiteral("Some_"));
    view->deleteWordLeft();
    QCOMPARE(doc->text(), QString());

    doc->setText(QStringLiteral("Some   WORD"));
    view->setCursorPosition({0, 11});
    view->deleteWordLeft();
    QCOMPARE(doc->text(), QStringLiteral("Some   "));
    view->deleteWordLeft();
    QCOMPARE(doc->text(), QString());
}

void CamelCursorTest::testSelectionRight()
{
    doc->setText(QStringLiteral("HelloWorld"));
    view->setCursorPosition({0, 0});
    view->shiftWordRight();
    QCOMPARE(view->selectionText(), QStringLiteral("Hello"));
    QCOMPARE(view->selectionRange(), KTextEditor::Range(0, 0, 0, 5));

    doc->setText(QStringLiteral("Hello\nWorld"));
    view->setCursorPosition({0, 0});
    view->shiftWordRight();
    view->shiftWordRight();
    QCOMPARE(view->selectionText(), QStringLiteral("Hello\n"));
    QCOMPARE(view->selectionRange(), KTextEditor::Range(0, 0, 1, 0));
}

void CamelCursorTest::testSelectionLeft()
{
    doc->setText(QStringLiteral("HelloWorld"));
    view->setCursorPosition({0, 10});
    view->shiftWordLeft();
    QCOMPARE(view->selectionText(), QStringLiteral("World"));
    QCOMPARE(view->selectionRange(), KTextEditor::Range(0, 5, 0, 10));

    doc->setText(QStringLiteral("Hello\nWorld"));
    view->setCursorPosition({1, 0});
    view->shiftWordLeft();
    view->shiftWordLeft();
    QCOMPARE(view->selectionText(), QStringLiteral("Hello\n"));
    QCOMPARE(view->selectionRange(), KTextEditor::Range(0, 0, 1, 0));
}

#include "moc_camelcursortest.cpp"
