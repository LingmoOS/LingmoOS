/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "macrorecorder.h"
#include "completionrecorder.h"
#include "completionreplayer.h"
#include "globalstate.h"
#include "kateview.h"
#include "lastchangerecorder.h"
#include "macros.h"
#include <vimode/inputmodemanager.h>
#include <vimode/keymapper.h>

namespace
{
const QChar LastPlayedRegister = QLatin1Char('@');
}

using namespace KateVi;

MacroRecorder::MacroRecorder(InputModeManager *viInputModeManager)
    : m_viInputModeManager(viInputModeManager)
    , m_isRecording(false)
    , m_macrosBeingReplayedCount(0)
    , m_lastPlayedMacroRegister(QChar::Null)
{
}

void MacroRecorder::start(const QChar &macroRegister)
{
    Q_ASSERT(!m_isRecording);
    m_isRecording = true;
    m_register = macroRegister;
    m_viInputModeManager->globalState()->macros()->remove(macroRegister);
    m_eventsLog.clear();
    m_viInputModeManager->completionRecorder()->start();
}

void MacroRecorder::stop()
{
    Q_ASSERT(m_isRecording);
    m_isRecording = false;
    CompletionList completions = m_viInputModeManager->completionRecorder()->stop();
    m_viInputModeManager->globalState()->macros()->store(m_register, m_eventsLog, completions);
}

bool MacroRecorder::isRecording() const
{
    return m_isRecording;
}

void MacroRecorder::record(const QKeyEvent &event)
{
    if (isRepeatOfLastShortcutOverrideAsKeyPress(event, m_eventsLog)) {
        return;
    }
    m_eventsLog.append(KeyEvent::fromQKeyEvent(event));
}

void MacroRecorder::dropLast()
{
    if (m_isRecording) {
        Q_ASSERT(!m_eventsLog.isEmpty());
        m_eventsLog.pop_back();
    }
}

void MacroRecorder::replay(const QChar &macroRegister)
{
    const QChar reg = (macroRegister == LastPlayedRegister) ? m_lastPlayedMacroRegister : macroRegister;

    m_lastPlayedMacroRegister = reg;
    const QString macroAsFeedableKeypresses = m_viInputModeManager->globalState()->macros()->get(reg);

    std::shared_ptr<KeyMapper> mapper(new KeyMapper(m_viInputModeManager, m_viInputModeManager->view()->doc()));
    CompletionList completions = m_viInputModeManager->globalState()->macros()->getCompletions(reg);

    m_macrosBeingReplayedCount++;
    m_viInputModeManager->completionReplayer()->start(completions);
    m_viInputModeManager->pushKeyMapper(mapper);
    m_viInputModeManager->feedKeyPresses(macroAsFeedableKeypresses);
    m_viInputModeManager->popKeyMapper();
    m_viInputModeManager->completionReplayer()->stop();
    m_macrosBeingReplayedCount--;
}

bool MacroRecorder::isReplaying() const
{
    return m_macrosBeingReplayedCount > 0;
}
