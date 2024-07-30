/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_EMULATED_COMMAND_BAR_SEARCHMODE_H
#define KATEVI_EMULATED_COMMAND_BAR_SEARCHMODE_H

#include "../searcher.h"
#include "activemode.h"

namespace KTextEditor
{
class ViewPrivate;
}

#include <KTextEditor/Cursor>

namespace KateVi
{
class EmulatedCommandBar;
QString vimRegexToQtRegexPattern(const QString &vimRegexPattern); // TODO - move these generic helper functions into their own file?
QString withCaseSensitivityMarkersStripped(const QString &originalSearchTerm);
QString ensuredCharEscaped(const QString &originalString, QChar charToEscape);
QStringList reversed(const QStringList &originalList);

class SearchMode : public ActiveMode
{
public:
    SearchMode(EmulatedCommandBar *emulatedCommandBar,
               MatchHighlighter *matchHighlighter,
               InputModeManager *viInputModeManager,
               KTextEditor::ViewPrivate *view,
               QLineEdit *edit);
    ~SearchMode() override
    {
    }
    enum class SearchDirection { Forward, Backward };
    void init(SearchDirection);
    bool handleKeyPress(const QKeyEvent *keyEvent) override;
    void editTextChanged(const QString &newText) override;
    CompletionStartParams completionInvoked(Completer::CompletionInvocation invocationType) override;
    void completionChosen() override;
    void deactivate(bool wasAborted) override;
    bool isSendingSyntheticSearchCompletedKeypress() const
    {
        return m_isSendingSyntheticSearchCompletedKeypress;
    }

private:
    QLineEdit *m_edit = nullptr;
    SearchDirection m_searchDirection;
    KTextEditor::Cursor m_startingCursorPos;
    KateVi::Searcher::SearchParams m_currentSearchParams;
    CompletionStartParams activateSearchHistoryCompletion();
    enum BarBackgroundStatus { Normal, MatchFound, NoMatchFound };
    void setBarBackground(BarBackgroundStatus status);
    bool m_isSendingSyntheticSearchCompletedKeypress = false;
};
}

#endif
