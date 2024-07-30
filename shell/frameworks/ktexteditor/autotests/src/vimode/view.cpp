/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "view.h"
#include <QClipboard>
#include <QFontDatabase>
#include <inputmode/kateviinputmode.h>
#include <katebuffer.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <kateview.h>

#include <QMainWindow>
#include <QTest>

using namespace KTextEditor;

QTEST_MAIN(ViewTest)

void ViewTest::yankHighlightingTests()
{
    const QColor yankHighlightColour = kate_view->rendererConfig()->savedLineColor();

    BeginTest(QStringLiteral("foo bar xyz"));
    const QList<Kate::TextRange *> rangesInitial = rangesOnFirstLine();
    Q_ASSERT(rangesInitial.isEmpty() && "Assumptions about ranges are wrong - this test is invalid and may need updating!");
    TestPressKey(QStringLiteral("wyiw"));
    {
        const QList<Kate::TextRange *> rangesAfterYank = rangesOnFirstLine();
        QCOMPARE(rangesAfterYank.size(), rangesInitial.size() + 1);
        QCOMPARE(rangesAfterYank.first()->attribute()->background().color(), yankHighlightColour);
        QCOMPARE(rangesAfterYank.first()->start().line(), 0);
        QCOMPARE(rangesAfterYank.first()->start().column(), 4);
        QCOMPARE(rangesAfterYank.first()->end().line(), 0);
        QCOMPARE(rangesAfterYank.first()->end().column(), 7);
    }
    FinishTest("foo bar xyz");

    BeginTest(QStringLiteral("foom bar xyz"));
    TestPressKey(QStringLiteral("wY"));
    {
        const QList<Kate::TextRange *> rangesAfterYank = rangesOnFirstLine();
        QCOMPARE(rangesAfterYank.size(), rangesInitial.size() + 1);
        QCOMPARE(rangesAfterYank.first()->attribute()->background().color(), yankHighlightColour);
        QCOMPARE(rangesAfterYank.first()->start().line(), 0);
        QCOMPARE(rangesAfterYank.first()->start().column(), 5);
        QCOMPARE(rangesAfterYank.first()->end().line(), 0);
        QCOMPARE(rangesAfterYank.first()->end().column(), 12);
    }
    FinishTest("foom bar xyz");

    // Unhighlight on keypress.
    DoTest("foo bar xyz", "yiww", "foo bar xyz");
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());

    // Update colour on config change.
    DoTest("foo bar xyz", "yiw", "foo bar xyz");
    const QColor newYankHighlightColour = QColor(255, 0, 0);
    kate_view->rendererConfig()->setSavedLineColor(newYankHighlightColour);
    QCOMPARE(rangesOnFirstLine().first()->attribute()->background().color(), newYankHighlightColour);

    // Visual Mode.
    DoTest("foo", "viwy", "foo");
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);

    // Unhighlight on keypress in Visual Mode
    DoTest("foo", "viwyw", "foo");
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size());

    // Add a yank highlight and directly (i.e. without using Vim commands,
    // which would clear the highlight) delete all text; if this deletes the yank highlight behind our back
    // and we don't respond correctly to this, it will be double-deleted by KateViNormalMode.
    // Currently, this seems like it doesn't occur, but better safe than sorry :)
    BeginTest(QStringLiteral("foo bar xyz"));
    TestPressKey(QStringLiteral("yiw"));
    QCOMPARE(rangesOnFirstLine().size(), rangesInitial.size() + 1);
    kate_document->documentReload();
    kate_document->clear();
    vi_input_mode->reset();
    vi_input_mode_manager = vi_input_mode->viInputModeManager();
    FinishTest("");
}

void ViewTest::visualLineUpDownTests()
{
    // Need to ensure we have dynamic wrap, a fixed width font, and a decent size kate_view.
    ensureKateViewVisible();
    const QFont oldFont = kate_view->rendererConfig()->baseFont();
    QFont fixedWidthFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    kate_view->rendererConfig()->setFont(fixedWidthFont);
    const bool oldDynWordWrap = KateViewConfig::global()->dynWordWrap();
    KateViewConfig::global()->setDynWordWrap(true);
    const bool oldReplaceTabsDyn = kate_document->config()->replaceTabsDyn();
    kate_document->config()->setReplaceTabsDyn(false);
    const int oldTabWidth = kate_document->config()->tabWidth();
    const int tabWidth = 5;
    kate_document->config()->setTabWidth(tabWidth);
    KateViewConfig::global()->setValue(KateViewConfig::ShowScrollbars, KateViewConfig::ScrollbarMode::AlwaysOn);

    // Compute the maximum width of text before line-wrapping sets it.
    int textWrappingLength = 1;
    while (true) {
        QString text = QStringLiteral("X").repeated(textWrappingLength) + QLatin1Char(' ') + QLatin1Char('O');
        const int posOfO = text.length() - 1;
        kate_document->setText(text);
        if (kate_view->cursorToCoordinate(Cursor(0, posOfO)).y() != kate_view->cursorToCoordinate(Cursor(0, 0)).y()) {
            textWrappingLength++; // Number of x's, plus space.
            break;
        }
        textWrappingLength++;
    }
    const QString fillsLineAndEndsOnSpace = QStringLiteral("X").repeated(textWrappingLength - 1) + QLatin1Char(' ');

    // Create a QString consisting of enough concatenated fillsLineAndEndsOnSpace to completely
    // fill the viewport of the kate View.
    QString fillsView = fillsLineAndEndsOnSpace;
    while (true) {
        kate_document->setText(fillsView);
        const QString visibleText = kate_document->text(kate_view->visibleRange());
        if (fillsView.length() > visibleText.length() * 2) { // Overkill.
            break;
        }
        fillsView += fillsLineAndEndsOnSpace;
    }
    const int numVisibleLinesToFillView = fillsView.length() / fillsLineAndEndsOnSpace.length();

    {
        // gk/ gj when there is only one line.
        DoTest("foo", "lgkr.", "f.o");
        DoTest("foo", "lgjr.", "f.o");
    }

    {
        // gk when sticky bit is set to the end.
        const QString originalText = fillsLineAndEndsOnSpace.repeated(2);
        QString expectedText = originalText;
        kate_document->setText(originalText);
        Q_ASSERT(expectedText[textWrappingLength - 1] == QLatin1Char(' '));
        expectedText[textWrappingLength - 1] = QLatin1Char('.');
        DoTest(originalText.toUtf8().constData(), "$gkr.", expectedText.toUtf8().constData());
    }

    {
        // Regression test: more than fill the view up, go to end, and do gk on wrapped text (used to crash).
        // First work out the text that will fill up the view.
        QString expectedText = fillsView;
        Q_ASSERT(expectedText[expectedText.length() - textWrappingLength - 1] == QLatin1Char(' '));
        expectedText[expectedText.length() - textWrappingLength - 1] = QLatin1Char('.');

        DoTest(fillsView.toUtf8().constData(), "$gkr.", expectedText.toUtf8().constData());
    }

    {
        // Jump down a few lines all in one go, where we have some variable length lines to navigate.
        const int numVisualLinesOnLine[] = {3, 5, 2, 3};
        const int numLines = sizeof(numVisualLinesOnLine) / sizeof(int);
        const int startVisualLine = 2;
        const int numberLinesToGoDownInOneGo = 10;

        int totalVisualLines = 0;
        for (int i = 0; i < numLines; i++) {
            totalVisualLines += numVisualLinesOnLine[i];
        }

        QString startText;
        for (int i = 0; i < numLines; i++) {
            QString thisLine = fillsLineAndEndsOnSpace.repeated(numVisualLinesOnLine[i]);
            // Replace trailing space with carriage return.
            thisLine.chop(1);
            thisLine.append(QLatin1Char('\n'));
            startText += thisLine;
        }
        QString expectedText = startText;
        expectedText[((startVisualLine - 1) + numberLinesToGoDownInOneGo) * fillsLineAndEndsOnSpace.length()] = QLatin1Char('.');

        Q_ASSERT(numberLinesToGoDownInOneGo + startVisualLine < totalVisualLines);
        Q_ASSERT(numberLinesToGoDownInOneGo + startVisualLine < numVisibleLinesToFillView);
        DoTest(startText.toUtf8().constData(),
               QString(QStringLiteral("gj").repeated(startVisualLine - 1) + QString::number(numberLinesToGoDownInOneGo) + QStringLiteral("gjr."))
                   .toUtf8()
                   .constData(),
               expectedText.toUtf8().constData());
        // Now go up a few lines.
        const int numLinesToGoBackUp = 7;
        expectedText = startText;
        expectedText[((startVisualLine - 1) + numberLinesToGoDownInOneGo - numLinesToGoBackUp) * fillsLineAndEndsOnSpace.length()] = QLatin1Char('.');
        DoTest(startText.toUtf8().constData(),
               QString(QStringLiteral("gj").repeated(startVisualLine - 1) + QString::number(numberLinesToGoDownInOneGo) + QStringLiteral("gj")
                       + QString::number(numLinesToGoBackUp) + QStringLiteral("gkr."))
                   .toUtf8()
                   .constData(),
               expectedText.toUtf8().constData());
    }

    {
        // Move down enough lines in one go to disappear off the view.
        // About half-a-viewport past the end of the current viewport.
        const int numberLinesToGoDown = numVisibleLinesToFillView * 3 / 2;
        const int visualColumnNumber = 7;
        Q_ASSERT(fillsLineAndEndsOnSpace.length() > visualColumnNumber);
        QString expectedText = fillsView.repeated(2);
        Q_ASSERT(expectedText[expectedText.length() - textWrappingLength - 1] == QLatin1Char(' '));
        expectedText[visualColumnNumber + fillsLineAndEndsOnSpace.length() * numberLinesToGoDown] = QLatin1Char('.');

        DoTest(fillsView.repeated(2).toUtf8().constData(),
               QString(QStringLiteral("l").repeated(visualColumnNumber) + QString::number(numberLinesToGoDown) + QStringLiteral("gjr.")).toUtf8().constData(),
               expectedText.toUtf8().constData());
    }

    {
        // Deal with dynamic wrapping and indented blocks - continuations of a line are "invisibly" idented by
        // the same amount as the beginning of the line, and we have to subtract this indentation.
        const QString unindentedFirstLine = QStringLiteral("stickyhelper\n");
        const int numIndentationSpaces = 5;
        Q_ASSERT(textWrappingLength > numIndentationSpaces * 2 /* keep some wriggle room */);
        const QString indentedFillsLineEndsOnSpace =
            QStringLiteral(" ").repeated(numIndentationSpaces) + QStringLiteral("X").repeated(textWrappingLength - 1 - numIndentationSpaces) + QLatin1Char(' ');
        DoTest(QString(unindentedFirstLine + indentedFillsLineEndsOnSpace + QStringLiteral("LINE3")).toUtf8().constData(),
               QString(QStringLiteral("l").repeated(numIndentationSpaces) + QStringLiteral("jgjr.")).toUtf8().constData(),
               QString(unindentedFirstLine + indentedFillsLineEndsOnSpace + QStringLiteral(".INE3")).toUtf8().constData());

        // The first, non-wrapped portion of the line is not invisibly indented, though, so ensure we don't mess that up.
        QString expectedSecondLine = indentedFillsLineEndsOnSpace;
        expectedSecondLine[numIndentationSpaces] = QLatin1Char('.');
        DoTest(QString(unindentedFirstLine + indentedFillsLineEndsOnSpace + QStringLiteral("LINE3")).toUtf8().constData(),
               QString(QStringLiteral("l").repeated(numIndentationSpaces) + QStringLiteral("jgjgkr.")).toUtf8().constData(),
               (unindentedFirstLine + expectedSecondLine + QStringLiteral("LINE3")).toUtf8().constData());
    }

    {
        // Take into account any invisible indentation when setting the sticky column.
        const int numIndentationSpaces = 5;
        Q_ASSERT(textWrappingLength > numIndentationSpaces * 2 /* keep some wriggle room */);
        const QString indentedFillsLineEndsOnSpace =
            QStringLiteral(" ").repeated(numIndentationSpaces) + QStringLiteral("X").repeated(textWrappingLength - 1 - numIndentationSpaces) + QLatin1Char(' ');
        const int posInSecondWrappedLineToChange = 3;
        QString expectedText = indentedFillsLineEndsOnSpace + fillsLineAndEndsOnSpace;
        expectedText[textWrappingLength + posInSecondWrappedLineToChange] = QLatin1Char('.');
        DoTest((indentedFillsLineEndsOnSpace + fillsLineAndEndsOnSpace).toUtf8().constData(),
               (QString::number(textWrappingLength + posInSecondWrappedLineToChange) + QStringLiteral("lgkgjr.")).toUtf8().constData(),
               expectedText.toUtf8().constData());
        // Make sure we can do this more than once (i.e. clear any flags that need clearing).
        DoTest((indentedFillsLineEndsOnSpace + fillsLineAndEndsOnSpace).toUtf8().constData(),
               (QString::number(textWrappingLength + posInSecondWrappedLineToChange) + QStringLiteral("lgkgjr.")).toUtf8().constData(),
               expectedText.toUtf8().constData());
    }

    {
        // Take into account any invisible indentation when setting the sticky column as above, but use tabs.
        const QString indentedFillsLineEndsOnSpace = QStringLiteral("\t") + QStringLiteral("X").repeated(textWrappingLength - 1 - tabWidth) + QLatin1Char(' ');
        const int posInSecondWrappedLineToChange = 3;
        QString expectedText = indentedFillsLineEndsOnSpace + fillsLineAndEndsOnSpace;
        expectedText[textWrappingLength - tabWidth + posInSecondWrappedLineToChange] = QLatin1Char('.');
        DoTest(QString(indentedFillsLineEndsOnSpace + fillsLineAndEndsOnSpace).toUtf8().constData(),
               QString(QStringLiteral("fXf ") + QString::number(posInSecondWrappedLineToChange) + QStringLiteral("lgkgjr.")).toUtf8().constData(),
               expectedText.toUtf8().constData());
    }

    {
        // Deal with the fact that j/ k may set a sticky column that is impossible to adhere to in visual mode because
        // it is too high.
        // Here, we have one dummy line and one wrapped line.  We start from the beginning of the wrapped line and
        // move right until we wrap and end up at posInWrappedLineToChange one the second line of the wrapped line.
        // We then move up and down with j and k to set the sticky column to a value to large to adhere to in a
        // visual line, and try to move a visual line up.
        const QString dummyLineForUseWithK(QStringLiteral("dummylineforusewithk\n"));
        QString startText = dummyLineForUseWithK + fillsLineAndEndsOnSpace.repeated(2);
        const int posInWrappedLineToChange = 3;
        QString expectedText = startText;
        expectedText[dummyLineForUseWithK.length() + posInWrappedLineToChange] = QLatin1Char('.');
        DoTest(startText.toUtf8().constData(),
               QString(QStringLiteral("j") + QString::number(textWrappingLength + posInWrappedLineToChange) + QStringLiteral("lkjgkr.")).toUtf8().constData(),
               expectedText.toUtf8().constData());
    }

    {
        // Ensure gj works in Visual mode.
        Q_ASSERT(fillsLineAndEndsOnSpace.toLower() != fillsLineAndEndsOnSpace);
        QString expectedText = fillsLineAndEndsOnSpace.toLower() + fillsLineAndEndsOnSpace;
        expectedText[textWrappingLength] = expectedText[textWrappingLength].toLower();
        DoTest(fillsLineAndEndsOnSpace.repeated(2).toUtf8().constData(), QStringLiteral("vgjgu").toUtf8().constData(), expectedText.toUtf8().constData());
    }

    {
        // Ensure gk works in Visual mode.
        Q_ASSERT(fillsLineAndEndsOnSpace.toLower() != fillsLineAndEndsOnSpace);
        DoTest(fillsLineAndEndsOnSpace.repeated(2).toUtf8().constData(),
               QStringLiteral("$vgkgu").toUtf8().constData(),
               QString(fillsLineAndEndsOnSpace + fillsLineAndEndsOnSpace.toLower()).toUtf8().constData());
    }

    {
        // Some tests for how well we handle things with real tabs.
        QString beginsWithTabFillsLineEndsOnSpace = QStringLiteral("\t");
        while (beginsWithTabFillsLineEndsOnSpace.length() + (tabWidth - 1) < textWrappingLength - 1) {
            beginsWithTabFillsLineEndsOnSpace += QLatin1Char('X');
        }
        beginsWithTabFillsLineEndsOnSpace += QLatin1Char(' ');
        const QString unindentedFirstLine = QStringLiteral("stockyhelper\n");
        const int posOnThirdLineToChange = 3;
        QString expectedThirdLine = fillsLineAndEndsOnSpace;
        expectedThirdLine[posOnThirdLineToChange] = QLatin1Char('.');
        DoTest((unindentedFirstLine + beginsWithTabFillsLineEndsOnSpace + fillsLineAndEndsOnSpace).toUtf8().constData(),
               QString(QStringLiteral("l").repeated(tabWidth + posOnThirdLineToChange) + QStringLiteral("gjgjr.")).toUtf8().constData(),
               (unindentedFirstLine + beginsWithTabFillsLineEndsOnSpace + expectedThirdLine).toUtf8().constData());

        // As above, but go down twice and return to the middle line.
        const int posOnSecondLineToChange = 2;
        QString expectedSecondLine = beginsWithTabFillsLineEndsOnSpace;
        expectedSecondLine[posOnSecondLineToChange + 1 /* "+1" as we're not counting the leading tab as a pos */] = QLatin1Char('.');
        DoTest((unindentedFirstLine + beginsWithTabFillsLineEndsOnSpace + fillsLineAndEndsOnSpace).toUtf8().constData(),
               QString(QStringLiteral("l").repeated(tabWidth + posOnSecondLineToChange) + QStringLiteral("gjgjgkr.")).toUtf8().constData(),
               QString(unindentedFirstLine + expectedSecondLine + fillsLineAndEndsOnSpace).toUtf8().constData());
    }

    // Restore back to how we were before.
    kate_view->rendererConfig()->setFont(oldFont);
    KateViewConfig::global()->setDynWordWrap(oldDynWordWrap);
    kate_document->config()->setReplaceTabsDyn(oldReplaceTabsDyn);
    kate_document->config()->setTabWidth(oldTabWidth);
}

void ViewTest::ScrollViewTests()
{
    QSKIP("This is too unstable in Jenkins", SkipAll);

    // First of all, we have to initialize some sizes and fonts.
    ensureKateViewVisible();

    const QFont oldFont = kate_view->rendererConfig()->baseFont();
    QFont fixedWidthFont(QStringLiteral("Monospace"));

    fixedWidthFont.setStyleHint(QFont::TypeWriter);
    fixedWidthFont.setPixelSize(14);
    Q_ASSERT_X(QFontInfo(fixedWidthFont).fixedPitch(), "setting up ScrollViewTests", "Need a fixed pitch font!");
    kate_view->rendererConfig()->setFont(fixedWidthFont);

    // Generating our text here.
    QString text;
    for (int i = 0; i < 20; i++) {
        text += QLatin1String("    aaaaaaaaaaaaaaaa\n");
    }

    // TODO: fix the visibleRange's tests.

    // zz
    BeginTest(text);
    TestPressKey(QStringLiteral("10l9jzz"));
    QCOMPARE(kate_view->cursorPosition().line(), 9);
    QCOMPARE(kate_view->cursorPosition().column(), 10);
    QCOMPARE(kate_view->visibleRange(), Range(4, 0, 13, 20));
    FinishTest(text.toUtf8().constData());

    // z.
    BeginTest(text);
    TestPressKey(QStringLiteral("10l9jz."));
    QCOMPARE(kate_view->cursorPosition().line(), 9);
    QCOMPARE(kate_view->cursorPosition().column(), 4);
    QCOMPARE(kate_view->visibleRange(), Range(4, 0, 13, 20));
    FinishTest(text.toUtf8().constData());

    // zt
    BeginTest(text);
    TestPressKey(QStringLiteral("10l9jzt"));
    QCOMPARE(kate_view->cursorPosition().line(), 9);
    QCOMPARE(kate_view->cursorPosition().column(), 10);
    QCOMPARE(kate_view->visibleRange(), Range(9, 0, 18, 20));
    FinishTest(text.toUtf8().constData());

    // z<cr>
    BeginTest(text);
    TestPressKey(QStringLiteral("10l9jz\\return"));
    QCOMPARE(kate_view->cursorPosition().line(), 9);
    QCOMPARE(kate_view->cursorPosition().column(), 4);
    QCOMPARE(kate_view->visibleRange(), Range(9, 0, 18, 20));
    FinishTest(text.toUtf8().constData());

    // zb
    BeginTest(text);
    TestPressKey(QStringLiteral("10l9jzb"));
    QCOMPARE(kate_view->cursorPosition().line(), 9);
    QCOMPARE(kate_view->cursorPosition().column(), 10);
    QCOMPARE(kate_view->visibleRange(), Range(0, 0, 9, 20));
    FinishTest(text.toUtf8().constData());

    // z-
    BeginTest(text);
    TestPressKey(QStringLiteral("10l9jz-"));
    QCOMPARE(kate_view->cursorPosition().line(), 9);
    QCOMPARE(kate_view->cursorPosition().column(), 4);
    QCOMPARE(kate_view->visibleRange(), Range(0, 0, 9, 20));
    FinishTest(text.toUtf8().constData());

    // Restore back to how we were before.
    kate_view->rendererConfig()->setFont(oldFont);
}

void ViewTest::clipboardTests_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("commands");
    QTest::addColumn<QString>("clipboard");

    QTest::newRow("yank") << "yyfoo\nbar"
                          << "yy"
                          << "yyfoo\n";
    QTest::newRow("delete") << "ddfoo\nbar"
                            << "dd"
                            << "ddfoo\n";
    QTest::newRow("yank empty line") << "\nbar"
                                     << "yy" << QString();
    QTest::newRow("delete word") << "word foo"
                                 << "dw"
                                 << "word ";
    QTest::newRow("delete onechar word") << "w foo"
                                         << "dw"
                                         << "w ";
    QTest::newRow("delete onechar") << "word foo"
                                    << "dc" << QString();
    QTest::newRow("delete empty lines") << " \t\n\n  \nfoo"
                                        << "d3d" << QString();
}

void ViewTest::clipboardTests()
{
    QFETCH(QString, text);
    QFETCH(QString, commands);
    QFETCH(QString, clipboard);

    QApplication::clipboard()->clear();
    BeginTest(text);
    TestPressKey(commands);
    QCOMPARE(QApplication::clipboard()->text(), clipboard);
}

QList<Kate::TextRange *> ViewTest::rangesOnFirstLine()
{
    return kate_document->buffer().rangesForLine(0, kate_view, true);
}

#include "moc_view.cpp"
