/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "activemode.h"
#include "emulatedcommandbar.h"
#include "matchhighlighter.h"

#include <vimode/inputmodemanager.h>
#include <vimode/modes/visualvimode.h>

#include "kateview.h"

using namespace KateVi;

CompletionStartParams ActiveMode::completionInvoked(Completer::CompletionInvocation invocationType)
{
    Q_UNUSED(invocationType);
    return CompletionStartParams();
}

void ActiveMode::setViInputModeManager(InputModeManager *viInputModeManager)
{
    m_viInputModeManager = viInputModeManager;
}

ActiveMode::~ActiveMode() = default;

void ActiveMode::hideAllWidgetsExcept(QWidget *widgetToKeepVisible)
{
    m_emulatedCommandBar->hideAllWidgetsExcept(widgetToKeepVisible);
}

void ActiveMode::updateMatchHighlight(KTextEditor::Range matchRange)
{
    m_matchHighligher->updateMatchHighlight(matchRange);
}

void ActiveMode::close(bool wasAborted)
{
    m_emulatedCommandBar->m_wasAborted = wasAborted;
    m_emulatedCommandBar->hideMe();
}

void ActiveMode::closeWithStatusMessage(const QString &exitStatusMessage)
{
    m_emulatedCommandBar->closeWithStatusMessage(exitStatusMessage);
}

void ActiveMode::startCompletion(const CompletionStartParams &completionStartParams)
{
    m_emulatedCommandBar->m_completer->startCompletion(completionStartParams);
}

void ActiveMode::moveCursorTo(const KTextEditor::Cursor cursorPos)
{
    m_view->setCursorPosition(cursorPos);
    if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualMode || m_viInputModeManager->getCurrentViMode() == ViMode::VisualLineMode) {
        m_viInputModeManager->getViVisualMode()->goToPos(cursorPos);
    }
}

EmulatedCommandBar *ActiveMode::emulatedCommandBar()
{
    return m_emulatedCommandBar;
}

KTextEditor::ViewPrivate *ActiveMode::view()
{
    return m_view;
}

InputModeManager *ActiveMode::viInputModeManager()
{
    return m_viInputModeManager;
}
