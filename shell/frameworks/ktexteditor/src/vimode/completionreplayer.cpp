/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "completionreplayer.h"
#include "completionrecorder.h"
#include "katedocument.h"
#include "katepartdebug.h"
#include "kateview.h"
#include "lastchangerecorder.h"
#include "macrorecorder.h"
#include <vimode/inputmodemanager.h>

#include <QKeyEvent>
#include <QRegularExpression>

using namespace KateVi;

CompletionReplayer::CompletionReplayer(InputModeManager *viInputModeManager)
    : m_viInputModeManager(viInputModeManager)
{
}

CompletionReplayer::~CompletionReplayer() = default;

void CompletionReplayer::start(const CompletionList &completions)
{
    m_nextCompletionIndex.push(0);
    m_CompletionsToReplay.push(completions);
}

void CompletionReplayer::stop()
{
    m_CompletionsToReplay.pop();
    m_nextCompletionIndex.pop();
}

void CompletionReplayer::replay()
{
    const Completion completion = nextCompletion();
    KTextEditor::ViewPrivate *m_view = m_viInputModeManager->view();
    KTextEditor::DocumentPrivate *doc = m_view->doc();

    // Find beginning of the word.
    KTextEditor::Cursor cursorPos = m_view->cursorPosition();
    KTextEditor::Cursor wordStart = KTextEditor::Cursor::invalid();
    if (!doc->characterAt(cursorPos).isLetterOrNumber() && doc->characterAt(cursorPos) != QLatin1Char('_')) {
        cursorPos.setColumn(cursorPos.column() - 1);
    }
    while (cursorPos.column() >= 0 && (doc->characterAt(cursorPos).isLetterOrNumber() || doc->characterAt(cursorPos) == QLatin1Char('_'))) {
        wordStart = cursorPos;
        cursorPos.setColumn(cursorPos.column() - 1);
    }
    // Find end of current word.
    cursorPos = m_view->cursorPosition();
    KTextEditor::Cursor wordEnd = KTextEditor::Cursor(cursorPos.line(), cursorPos.column() - 1);
    while (cursorPos.column() < doc->lineLength(cursorPos.line())
           && (doc->characterAt(cursorPos).isLetterOrNumber() || doc->characterAt(cursorPos) == QLatin1Char('_'))) {
        wordEnd = cursorPos;
        cursorPos.setColumn(cursorPos.column() + 1);
    }
    QString completionText = completion.completedText();
    const KTextEditor::Range currentWord = KTextEditor::Range(wordStart, KTextEditor::Cursor(wordEnd.line(), wordEnd.column() + 1));
    // Should we merge opening brackets? Yes, if completion is a function with arguments and after the cursor
    // there is (optional whitespace) followed by an open bracket.
    int offsetFinalCursorPosBy = 0;
    if (completion.completionType() == Completion::FunctionWithArgs) {
        const int nextMergableBracketAfterCursorPos = findNextMergeableBracketPos(currentWord.end());
        if (nextMergableBracketAfterCursorPos != -1) {
            if (completionText.endsWith(QLatin1String("()"))) {
                // Strip "()".
                completionText.chop(2);
            } else if (completionText.endsWith(QLatin1String("();"))) {
                // Strip "();".
                completionText.chop(3);
            }
            // Ensure cursor ends up after the merged open bracket.
            offsetFinalCursorPosBy = nextMergableBracketAfterCursorPos + 1;
        } else {
            if (!completionText.endsWith(QLatin1String("()")) && !completionText.endsWith(QLatin1String("();"))) {
                // Original completion merged with an opening bracket; we'll have to add our own brackets.
                completionText.append(QLatin1String("()"));
            }
            // Position cursor correctly i.e. we'll have added "functionname()" or "functionname();"; need to step back by
            // one or two to be after the opening bracket.
            offsetFinalCursorPosBy = completionText.endsWith(QLatin1Char(';')) ? -2 : -1;
        }
    }
    KTextEditor::Cursor deleteEnd =
        completion.removeTail() ? currentWord.end() : KTextEditor::Cursor(m_view->cursorPosition().line(), m_view->cursorPosition().column() + 0);

    if (currentWord.isValid()) {
        doc->removeText(KTextEditor::Range(currentWord.start(), deleteEnd));
        doc->insertText(currentWord.start(), completionText);
    } else {
        doc->insertText(m_view->cursorPosition(), completionText);
    }

    if (offsetFinalCursorPosBy != 0) {
        m_view->setCursorPosition(KTextEditor::Cursor(m_view->cursorPosition().line(), m_view->cursorPosition().column() + offsetFinalCursorPosBy));
    }

    if (!m_viInputModeManager->lastChangeRecorder()->isReplaying()) {
        Q_ASSERT(m_viInputModeManager->macroRecorder()->isReplaying());
        // Post the completion back: it needs to be added to the "last change" list ...
        m_viInputModeManager->completionRecorder()->logCompletionEvent(completion);
        // ... but don't log the ctrl-space that led to this call to replayCompletion(), as
        // a synthetic ctrl-space was just added to the last change keypresses by logCompletionEvent(), and we don't
        // want to duplicate them!
        m_viInputModeManager->doNotLogCurrentKeypress();
    }
}

Completion CompletionReplayer::nextCompletion()
{
    Q_ASSERT(m_viInputModeManager->lastChangeRecorder()->isReplaying() || m_viInputModeManager->macroRecorder()->isReplaying());

    if (m_nextCompletionIndex.top() >= m_CompletionsToReplay.top().length()) {
        qCDebug(LOG_KTE) << "Something wrong here: requesting more completions for macro than we actually have.  Returning dummy.";
        return Completion(QString(), false, Completion::PlainText);
    }

    return m_CompletionsToReplay.top()[m_nextCompletionIndex.top()++];
}

int CompletionReplayer::findNextMergeableBracketPos(const KTextEditor::Cursor startPos) const
{
    KTextEditor::DocumentPrivate *doc = m_viInputModeManager->view()->doc();
    const QString lineAfterCursor = doc->text(KTextEditor::Range(startPos, KTextEditor::Cursor(startPos.line(), doc->lineLength(startPos.line()))));
    static const QRegularExpression whitespaceThenOpeningBracket(QStringLiteral("^\\s*(\\()"), QRegularExpression::UseUnicodePropertiesOption);
    QRegularExpressionMatch match = whitespaceThenOpeningBracket.match(lineAfterCursor);
    int nextMergableBracketAfterCursorPos = -1;
    if (match.hasMatch()) {
        nextMergableBracketAfterCursorPos = match.capturedStart(1);
    }
    return nextMergableBracketAfterCursorPos;
}
