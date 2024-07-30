/*
   SPDX-FileCopyrightText: 2022 Martin Seher <martin.seher@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "hlsearch.h"
#include "emulatedcommandbarsetupandteardown.h"

#include <inputmode/kateviinputmode.h>
#include <katebuffer.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <kateview.h>
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>

#include <QMainWindow>
#include <QTest>

using namespace KTextEditor;

#define TestHighlight(...) TestHighlight_(__LINE__, __FILE__, __VA_ARGS__)

QTEST_MAIN(HlSearchTest)

void HlSearchTest::highlightModeTests()
{
    EmulatedCommandBarSetUpAndTearDown emulatedCommandBarSetUpAndTearDown(vi_input_mode, kate_view, mainWindow);

    setWindowSize();

    const QList<Kate::TextRange *> rangesInitial = rangesOnLine(0);
    Q_ASSERT(rangesInitial.isEmpty() && "Assumptions about ranges are wrong - this test is invalid and may need updating!");

    const QColor searchHighlightColor = kate_view->rendererConfig()->searchHighlightColor();

    // test commands exist
    {
        QCOMPARE(vi_input_mode->viModeEmulatedCommandBar()->executeCommand(QStringLiteral("set-hls")).size(), 0);
        QCOMPARE(vi_input_mode->viModeEmulatedCommandBar()->executeCommand(QStringLiteral("set-hlsearch")).size(), 0);
        QCOMPARE(vi_input_mode->viModeEmulatedCommandBar()->executeCommand(QStringLiteral("set-nohls")).size(), 0);
        QCOMPARE(vi_input_mode->viModeEmulatedCommandBar()->executeCommand(QStringLiteral("set-nohlsearch")).size(), 0);
        QCOMPARE(vi_input_mode->viModeEmulatedCommandBar()->executeCommand(QStringLiteral("noh")).size(), 0);
        QCOMPARE(vi_input_mode->viModeEmulatedCommandBar()->executeCommand(QStringLiteral("nohlsearch")).size(), 0);
    }
    // test highlight initiated by *
    {
        QString text = QStringLiteral("foo bar xyz foo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        QCOMPARE(vr.end().column(), text.length());

        TestPressKey(QStringLiteral("w*"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 2);

            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
            TestHighlight(*ranges[1], {0, 19}, {0, 22}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test highlight initiated by #
    {
        QString text = QStringLiteral("foo bar xyz foo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        QCOMPARE(vr.end().column(), text.length());

        TestPressKey(QStringLiteral("w#"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 2);

            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
            TestHighlight(*ranges[1], {0, 19}, {0, 22}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test highlight initiated by /
    {
        QString text = QStringLiteral("foo bar xyz foo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        QCOMPARE(vr.end().column(), text.length());

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 2);

            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
            TestHighlight(*ranges[1], {0, 19}, {0, 22}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test highlight initiated by ?
    {
        QString text = QStringLiteral("foo bar xyz foo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        QCOMPARE(vr.end().column(), text.length());

        TestPressKey(QStringLiteral("?bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 2);

            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
            TestHighlight(*ranges[1], {0, 19}, {0, 22}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that aborting search removes highlights
    {
        QString text = QStringLiteral("foo bar xyz foo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        QCOMPARE(vr.end().column(), text.length());

        TestPressKey(QStringLiteral("/bar"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 2);

            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
            TestHighlight(*ranges[1], {0, 19}, {0, 22}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("\\esc"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        FinishTest(text.toUtf8().constData());
    }
    // test empty matches handled
    {
        QString text = QStringLiteral("foo bar xyz");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        QCOMPARE(vr.end().column(), text.length());

        TestPressKey(QStringLiteral("/\\\\<\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 6);

            TestHighlight(*ranges[0], {0, 0}, {0, 1}, searchHighlightColor);
            TestHighlight(*ranges[3], {0, 7}, {0, 8}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that only visible matches are highlighted
    {
        QString text = QStringLiteral("foo bar xyz\n\n\n\n\nfoo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        // ensure that last line is not visible
        QVERIFY(vr.end().line() < 4);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
            ranges = rangesOnLine(5);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }

        kate_view->bottom();
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
            ranges = rangesOnLine(5);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {5, 7}, {5, 10}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test highlighting when typing in search triggers a visual range change
    {
        QString text = QStringLiteral("foo bar xyz\n\n\n\n\nfoo ab barx");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        // ensure that last line is not visible
        QVERIFY(vr.end().line() < 4);

        TestPressKey(QStringLiteral("/barx"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
            ranges = rangesOnLine(5);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {5, 7}, {5, 11}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("\\enter"));

        FinishTest(text.toUtf8().constData());
    }
    // test that normal search highlight is deactivated when hls mode is active
    {
        QString text = QStringLiteral("foo bar xyz");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("\\enter"));
        FinishTest(text.toUtf8().constData());
    }
    // test that :noh turns of current highlight
    {
        QString text = QStringLiteral("foo bar xyz\n\n\n\n\nfoo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        // ensure that last line is not visible
        QVERIFY(vr.end().line() < 4);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":noh\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        // changing view range should not activate highlighting again
        kate_view->bottom();
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(5);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that :nohlsearch turns of current highlight
    {
        QString text = QStringLiteral("foo bar xyz\n\n\n\n\nfoo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        // ensure that last line is not visible
        QVERIFY(vr.end().line() < 4);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":nohlsearch\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        // changing view range should not activate highlighting again
        kate_view->bottom();
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(5);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that new search activates highlighting after :noh
    {
        QString text = QStringLiteral("foo bar xyz foo");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":noh\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that word search activates highlighting after :noh
    {
        QString text = QStringLiteral("foo bar xyz foo");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":noh\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        TestPressKey(QStringLiteral("*"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that next match ('n') activates highlighting after :noh
    {
        QString text = QStringLiteral("foo bar xyz foo");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":noh\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        TestPressKey(QStringLiteral("n"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that previous match ('N') activates highlighting after :noh
    {
        QString text = QStringLiteral("foo bar xyz foo");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":noh\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        TestPressKey(QStringLiteral("N"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that :set-nohls turns of highlight
    {
        QString text = QStringLiteral("foo bar xyz\n\n\n\n\nfoo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        // ensure that last line is not visible
        QVERIFY(vr.end().line() < 4);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":set-nohls\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        // changing view range should not activate highlighting again
        kate_view->bottom();
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(5);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that :set-nohlsearch turns of highlight
    {
        QString text = QStringLiteral("foo bar xyz\n\n\n\n\nfoo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        // ensure that last line is not visible
        QVERIFY(vr.end().line() < 4);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":set-nohlsearch\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        // changing view range should not activate highlighting again
        kate_view->bottom();
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(5);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that new search does not activate highlighting after :set-nohls
    {
        QString text = QStringLiteral("foo bar xyz foo");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":set-nohls\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that word search does not activate highlighting after :set-nohls
    {
        QString text = QStringLiteral("foo bar xyz foo");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":set-nohls\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        TestPressKey(QStringLiteral("*"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that highlighting can be activated with :set-hls after :set-nohls
    {
        QString text = QStringLiteral("foo bar xyz foo");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":set-nohls\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        TestPressKey(QStringLiteral(":set-hls\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that highlighting can be activated with :set-hlsearch after :set-nohls
    {
        QString text = QStringLiteral("foo bar xyz foo");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        TestPressKey(QStringLiteral(":set-nohls\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        TestPressKey(QStringLiteral(":set-hlsearch\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that switching to normal mode turns off highlighting
    {
        QString text = QStringLiteral("foo bar xyz foo");
        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 7}, searchHighlightColor);
        }

        kate_view->setInputMode(KTextEditor::View::InputMode::NormalInputMode);
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        kate_view->setInputMode(KTextEditor::View::InputMode::ViInputMode);
        FinishTest(text.toUtf8().constData());
    }
    // test that opening search bar does not hide previous results
    {
        QString text = QStringLiteral("foo xbar barx bar");

        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 3);
            TestHighlight(*ranges[0], {0, 5}, {0, 8}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("/"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 3);
            TestHighlight(*ranges[0], {0, 5}, {0, 8}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 3);
            TestHighlight(*ranges[0], {0, 5}, {0, 8}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("/"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 3);
            TestHighlight(*ranges[0], {0, 5}, {0, 8}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("\\esc"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 3);
            TestHighlight(*ranges[0], {0, 5}, {0, 8}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that deleting all text in search bar removes highlights
    {
        QString text = QStringLiteral("foo xbar barx bar");

        BeginTest(text);

        TestPressKey(QStringLiteral("/bar"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 3);
            TestHighlight(*ranges[0], {0, 5}, {0, 8}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("\\backspace\\backspace\\backspace"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size());
        }
        TestPressKey(QStringLiteral("\\esc"));
        FinishTest(text.toUtf8().constData());
    }
    // test that previously results are shown when current search is aborted
    {
        QString text = QStringLiteral("foo xbar barx bar");

        BeginTest(text);

        TestPressKey(QStringLiteral("/bar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 3);
            TestHighlight(*ranges[0], {0, 5}, {0, 8}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("/rx"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 11}, {0, 13}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("\\esc"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 3);
            TestHighlight(*ranges[0], {0, 5}, {0, 8}, searchHighlightColor);
        }
        FinishTest(text.toUtf8().constData());
    }
    // test that newly inserted text will be highlighted
    {
        QString text = QStringLiteral("foo xbar abcd bar");

        TestPressKey(QStringLiteral("/xbar\\enter"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 1);
            TestHighlight(*ranges[0], {0, 4}, {0, 8}, searchHighlightColor);
        }
        TestPressKey(QStringLiteral("wwix\\esc"));
        {
            QList<Kate::TextRange *> ranges = rangesOnLine(0);
            QCOMPARE(ranges.size(), rangesInitial.size() + 2);
            TestHighlight(*ranges[0], {0, 4}, {0, 8}, searchHighlightColor);
            TestHighlight(*ranges[1], {0, 14}, {0, 18}, searchHighlightColor);
        }

        BeginTest(text);
        FinishTest(text.toUtf8().constData());
    }
    // test that no endless loop is triggered
    {
        QString text = QStringLiteral("foo bar xyz\nabc def\nghi jkl\nmno pqr\nstu vwx\nfoo ab bar x");
        BeginTest(text);
        auto vr = kate_view->visibleRange();
        // ensure that last line is not visible
        QVERIFY(vr.end().line() < 4);

        TestPressKey(QStringLiteral("/\\\\<\\enter"));

        FinishTest(text.toUtf8().constData());
    }
}

QList<Kate::TextRange *> HlSearchTest::rangesOnLine(int line)
{
    QList<Kate::TextRange *> arr = kate_document->buffer().rangesForLine(line, kate_view, true);
    std::sort(arr.begin(), arr.end(), [](const Kate::TextRange *l, const Kate::TextRange *r) {
        return l->toRange().start() < r->toRange().start();
    });
    return arr;
}

void HlSearchTest::setWindowSize()
{
    const QFont font = kate_view->rendererConfig()->baseFont();
    QFontMetrics fm(font);
    auto fontHeight = fm.height();

    kate_document->setText(QStringLiteral("\n\n\n\n"));
    for (int i = 250; i > 0; i -= fontHeight) {
        mainWindow->setMaximumHeight(i);
        if (kate_view->visibleRange().end().line() == 3)
            break;
    }
    QVERIFY(kate_view->visibleRange().end().line() == 3);
}

void HlSearchTest::TestHighlight_(int line, const char *file, const Kate::TextRange &r, std::array<int, 2> start, std::array<int, 2> end, const QColor &bg)
{
    QTest::qCompare(r.attribute()->background().color(), bg, "bgcolor", "bgcolor", file, line);
    QTest::qCompare(r.start().line(), start[0], "start_line", "start_line", file, line);
    QTest::qCompare(r.start().column(), start[1], "start_column", "start_column", file, line);
    QTest::qCompare(r.end().line(), end[0], "end_line", "end_line", file, line);
    QTest::qCompare(r.end().column(), end[1], "end_column", "end_column", file, line);
}

#include "moc_hlsearch.cpp"
