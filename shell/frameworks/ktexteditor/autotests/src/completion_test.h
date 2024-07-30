/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams\gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_COMPLETIONTEST_H
#define KATE_COMPLETIONTEST_H

#include <QObject>

namespace KTextEditor
{
class Document;
}
namespace KTextEditor
{
class ViewPrivate;
}

class CompletionTest : public QObject
{
    Q_OBJECT

public:
    CompletionTest()
    {
    }
    ~CompletionTest() override
    {
    }

private Q_SLOTS:
    void init();
    void cleanup();
    void testFilterEmptyRange();
    void testFilterWithRange();
    void testCustomRange1();
    void testCustomRange2();
    void testCustomRangeMultipleModels();
    void testAbortCursorMovedOutOfRange();
    void testAbortInvalidText();
    void testAbortController();
    void testAbortControllerMultipleModels();
    void testEmptyFilterString();
    void testUpdateCompletionRange();
    void testCustomStartCompl();
    void testKateCompletionModel();
    void testAbortImmideatelyAfterStart();
    void testJumpToListBottomAfterCursorUpWhileAtTop();
    void testAbbrevAndContainsMatching();
    void testAsyncMatching();
    void testAbbreviationEngine();
    void testAutoCompletionPreselectFirst();
    void testTabCompletion();
    void benchAbbreviationEngineNormalCase();
    void benchAbbreviationEngineWorstCase();
    void benchAbbreviationEngineGoodCase();
    void benchCompletionModel();

private:
    KTextEditor::Document *m_doc;
    KTextEditor::ViewPrivate *m_view;
};

#endif
