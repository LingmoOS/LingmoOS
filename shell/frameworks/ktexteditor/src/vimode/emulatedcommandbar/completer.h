/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_EMULATED_COMMAND_BAR_COMPLETER_H
#define KATEVI_EMULATED_COMMAND_BAR_COMPLETER_H

#include <functional>

#include <QStringList>

namespace KTextEditor
{
class ViewPrivate;
}

class QLineEdit;
class QCompleter;
class QStringListModel;
class QKeyEvent;

namespace KateVi
{
class ActiveMode;
class EmulatedCommandBar;

struct CompletionStartParams {
    static CompletionStartParams createModeSpecific(const QStringList &completions,
                                                    int wordStartPos,
                                                    std::function<QString(const QString &)> completionTransform = std::function<QString(const QString &)>())
    {
        CompletionStartParams completionStartParams;
        completionStartParams.completionType = ModeSpecific;
        completionStartParams.completions = completions;
        completionStartParams.wordStartPos = wordStartPos;
        completionStartParams.completionTransform = completionTransform;
        return completionStartParams;
    }
    static CompletionStartParams invalid()
    {
        CompletionStartParams completionStartParams;
        completionStartParams.completionType = None;
        return completionStartParams;
    }
    enum CompletionType { None, ModeSpecific, WordFromDocument };
    CompletionType completionType = None;
    int wordStartPos = -1;
    QStringList completions;
    std::function<QString(const QString &)> completionTransform;
};

class Completer
{
public:
    enum class CompletionInvocation { ExtraContext, NormalContext };
    Completer(EmulatedCommandBar *emulatedCommandBar, KTextEditor::ViewPrivate *view, QLineEdit *edit);
    void startCompletion(const CompletionStartParams &completionStartParams);
    void deactivateCompletion();
    bool isCompletionActive() const;
    bool isNextTextChangeDueToCompletionChange() const;
    bool completerHandledKeypress(const QKeyEvent *keyEvent);
    void editTextChanged(const QString &newText);
    void setCurrentMode(ActiveMode *currentMode);

private:
    QLineEdit *m_edit;
    KTextEditor::ViewPrivate *m_view;
    ActiveMode *m_currentMode = nullptr;

    void setCompletionIndex(int index);
    void currentCompletionChanged();
    void updateCompletionPrefix();
    CompletionStartParams activateWordFromDocumentCompletion();
    QString wordBeforeCursor();
    int wordBeforeCursorBegin();
    void abortCompletionAndResetToPreCompletion();

    QCompleter *m_completer;
    QStringListModel *m_completionModel;
    QString m_textToRevertToIfCompletionAborted;
    int m_cursorPosToRevertToIfCompletionAborted = 0;
    bool m_isNextTextChangeDueToCompletionChange = false;
    CompletionStartParams m_currentCompletionStartParams;
    CompletionStartParams::CompletionType m_currentCompletionType = CompletionStartParams::None;
};
}
#endif
