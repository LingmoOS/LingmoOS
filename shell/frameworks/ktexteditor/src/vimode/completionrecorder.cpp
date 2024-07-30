/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "completionrecorder.h"
#include "katepartdebug.h"
#include "lastchangerecorder.h"
#include "macrorecorder.h"
#include "vimode/definitions.h"
#include <vimode/inputmodemanager.h>

#include <QKeyEvent>

using namespace KateVi;

CompletionRecorder::CompletionRecorder(InputModeManager *viInputModeManager)
    : m_viInputModeManager(viInputModeManager)
{
}

CompletionRecorder::~CompletionRecorder() = default;

void CompletionRecorder::logCompletionEvent(const Completion &completion)
{
    // Ctrl-space is a special code that means: if you're replaying a macro, fetch and execute
    // the next logged completion.
    static const QKeyEvent CompletionEvent(QKeyEvent::KeyPress, Qt::Key_Space, CONTROL_MODIFIER, QStringLiteral(" "));

    if (m_viInputModeManager->macroRecorder()->isRecording()) {
        m_viInputModeManager->macroRecorder()->record(CompletionEvent);
        m_currentMacroCompletionsLog.append(completion);
    }

    m_viInputModeManager->lastChangeRecorder()->record(CompletionEvent);
    m_currentChangeCompletionsLog.append(completion);
}

void CompletionRecorder::start()
{
    m_currentMacroCompletionsLog.clear();
}

CompletionList CompletionRecorder::stop()
{
    return m_currentMacroCompletionsLog;
}

void CompletionRecorder::clearCurrentChangeCompletionsLog()
{
    m_currentChangeCompletionsLog.clear();
}

CompletionList CompletionRecorder::currentChangeCompletionsLog()
{
    return m_currentChangeCompletionsLog;
}
