/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_EMULATED_COMMAND_BAR_COMMANDMODE_H
#define KATEVI_EMULATED_COMMAND_BAR_COMMANDMODE_H

#include "activemode.h"

#include <QHash>

#include <KCompletion>

namespace KTextEditor
{
class ViewPrivate;
class Command;
}

namespace KateVi
{
class EmulatedCommandBar;
class MatchHighlighter;
class InteractiveSedReplaceMode;
class Completer;
class InputModeManager;

class CommandMode : public ActiveMode
{
public:
    CommandMode(EmulatedCommandBar *emulatedCommandBar,
                MatchHighlighter *matchHighlighter,
                InputModeManager *viInputModeManager,
                KTextEditor::ViewPrivate *view,
                QLineEdit *edit,
                InteractiveSedReplaceMode *interactiveSedReplaceMode,
                Completer *completer);
    ~CommandMode() override
    {
    }
    bool handleKeyPress(const QKeyEvent *keyEvent) override;
    void editTextChanged(const QString &newText) override;
    CompletionStartParams completionInvoked(Completer::CompletionInvocation invocationType) override;
    void completionChosen() override;
    void deactivate(bool wasAborted) override;
    QString executeCommand(const QString &commandToExecute);

private:
    CompletionStartParams activateCommandCompletion();
    CompletionStartParams activateCommandHistoryCompletion();
    CompletionStartParams activateSedFindHistoryCompletion();
    CompletionStartParams activateSedReplaceHistoryCompletion();
    QString withoutRangeExpression();
    QString rangeExpression();
    QString withSedFindTermReplacedWith(const QString &newFindTerm);
    QString withSedDelimiterEscaped(const QString &text);
    bool isCursorInFindTermOfSed();
    bool isCursorInReplaceTermOfSed();
    QString sedFindTerm();
    QString sedReplaceTerm();
    /**
     * Stuff to do with expressions of the form:
     *
     *   s/find/replace/<sedflags>
     */
    struct ParsedSedExpression {
        bool parsedSuccessfully;
        int findBeginPos;
        int findEndPos;
        int replaceBeginPos;
        int replaceEndPos;
        QChar delimiter;
    };
    /**
     * The "range expression" is the (optional) expression before the command that describes
     * the range over which the command should be run e.g. '<,'>.  @see CommandRangeExpressionParser
     */
    CommandMode::ParsedSedExpression parseAsSedExpression();
    int commandBeforeCursorBegin();
    QLineEdit *m_edit;
    InteractiveSedReplaceMode *m_interactiveSedReplaceMode;
    Completer *m_completer;
    KCompletion m_cmdCompletion;
    QHash<QString, KTextEditor::Command *> m_cmdDict;
    KTextEditor::Command *queryCommand(const QString &cmd) const;
};
}

#endif
