/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_EMULATED_COMMAND_BAR_ACTIVEMODE_H
#define KATEVI_EMULATED_COMMAND_BAR_ACTIVEMODE_H

#include "completer.h"

class QKeyEvent;
class QString;
class QWidget;

namespace KTextEditor
{
class Cursor;
class Range;
class ViewPrivate;
}

namespace KateVi
{
class EmulatedCommandBar;
struct CompletionStartParams;
class MatchHighlighter;
class InputModeManager;

class ActiveMode
{
public:
    ActiveMode(EmulatedCommandBar *emulatedCommandBar, MatchHighlighter *matchHighlighter, InputModeManager *viInputModeManager, KTextEditor::ViewPrivate *view)
        : m_emulatedCommandBar(emulatedCommandBar)
        , m_viInputModeManager(viInputModeManager)
        , m_view(view)
        , m_matchHighligher(matchHighlighter)
    {
    }
    virtual ~ActiveMode() = 0;
    virtual bool handleKeyPress(const QKeyEvent *keyEvent) = 0;
    virtual void editTextChanged(const QString &newText)
    {
        Q_UNUSED(newText);
    }
    virtual KateVi::CompletionStartParams completionInvoked(Completer::CompletionInvocation invocationType);
    virtual void completionChosen()
    {
    }
    virtual void deactivate(bool wasAborted) = 0;
    void setViInputModeManager(InputModeManager *viInputModeManager);

protected:
    // Helper methods.
    void hideAllWidgetsExcept(QWidget *widgetToKeepVisible);
    void updateMatchHighlight(KTextEditor::Range matchRange);
    void close(bool wasAborted);
    void closeWithStatusMessage(const QString &exitStatusMessage);
    void startCompletion(const CompletionStartParams &completionStartParams);
    void moveCursorTo(const KTextEditor::Cursor cursorPos);
    EmulatedCommandBar *emulatedCommandBar();
    KTextEditor::ViewPrivate *view();
    InputModeManager *viInputModeManager();

private:
    EmulatedCommandBar *m_emulatedCommandBar = nullptr;
    InputModeManager *m_viInputModeManager = nullptr;
    KTextEditor::ViewPrivate *m_view = nullptr;
    MatchHighlighter *m_matchHighligher = nullptr;
};
}
#endif
