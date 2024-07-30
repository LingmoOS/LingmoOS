/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_COMPLETIONRECORDER_H
#define KATEVI_COMPLETIONRECORDER_H

#include "completion.h"

namespace KateVi
{
class InputModeManager;

class CompletionRecorder
{
public:
    explicit CompletionRecorder(InputModeManager *viInputModeManager);
    ~CompletionRecorder();

    void logCompletionEvent(const Completion &completion);

    void start();
    CompletionList stop();

    CompletionList currentChangeCompletionsLog();
    void clearCurrentChangeCompletionsLog();

private:
    InputModeManager *m_viInputModeManager;

    CompletionList m_currentMacroCompletionsLog;
    CompletionList m_currentChangeCompletionsLog;
};

}

#endif // KATEVI_COMPLETIONRECORDER_H
