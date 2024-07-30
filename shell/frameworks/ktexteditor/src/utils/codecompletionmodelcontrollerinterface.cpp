/*
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codecompletionmodelcontrollerinterface.h"

#include <QModelIndex>
#include <QRegularExpression>

#include <kateconfig.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

namespace KTextEditor
{
CodeCompletionModelControllerInterface::CodeCompletionModelControllerInterface()
{
}

CodeCompletionModelControllerInterface::~CodeCompletionModelControllerInterface() = default;

bool CodeCompletionModelControllerInterface::shouldStartCompletion(View *view, const QString &insertedText, bool userInsertion, const Cursor &position)
{
    Q_UNUSED(view);
    Q_UNUSED(position);
    if (insertedText.isEmpty()) {
        return false;
    }

    QChar lastChar = insertedText.at(insertedText.size() - 1);
    if ((userInsertion && (lastChar.isLetter() || lastChar.isNumber() || lastChar == QLatin1Char('_'))) || lastChar == QLatin1Char('.')
        || insertedText.endsWith(QLatin1String("->"))) {
        return true;
    }
    return false;
}

Range CodeCompletionModelControllerInterface::completionRange(View *view, const Cursor &position)
{
    Cursor end = position;
    const int line = end.line();

    const QString lineText = view->document()->line(line);
    QStringView text = lineText;

    static constexpr auto options = QRegularExpression::UseUnicodePropertiesOption | QRegularExpression::DontCaptureOption;
    static const QRegularExpression findWordStart(QStringLiteral("\\b[_\\w]+$"), options);
    static const QRegularExpression findWordEnd(QStringLiteral("^[_\\w]*\\b"), options);

    Cursor start = end;

    int pos = text.left(end.column()).lastIndexOf(findWordStart);
    if (pos >= 0) {
        start.setColumn(pos);
    }

    if (!KateViewConfig::global()->wordCompletionRemoveTail()) {
        // We are not removing tail, range only contains the word left of the cursor
        return Range(start, position);
    } else {
        // Removing tail, find the word end
        QRegularExpressionMatch match;
        pos = text.mid(end.column()).indexOf(findWordEnd, 0, &match);
        if (pos >= 0) {
            end.setColumn(end.column() + match.capturedLength());
        }

        return Range(start, end);
    }
}

Range CodeCompletionModelControllerInterface::updateCompletionRange(View *view, const Range &range)
{
    QStringList text = view->document()->textLines(range, false);
    if (!text.isEmpty() && text.count() == 1 && text.first().trimmed().isEmpty())
    // When inserting a newline behind an empty completion-range,, move the range forward to its end
    {
        return Range(range.end(), range.end());
    }

    return range;
}

QString CodeCompletionModelControllerInterface::filterString(View *view, const Range &range, const Cursor &position)
{
    return view->document()->text(KTextEditor::Range(range.start(), position));
}

bool CodeCompletionModelControllerInterface::shouldAbortCompletion(View *view, const Range &range, const QString &currentCompletion)
{
    if (view->cursorPosition() < range.start() || view->cursorPosition() > range.end()) {
        return true; // Always abort when the completion-range has been left
    }
    // Do not abort completions when the text has been empty already before and a newline has been entered

    static const QRegularExpression allowedText(QStringLiteral("^\\w*$"), QRegularExpression::UseUnicodePropertiesOption);
    return !allowedText.match(currentCompletion).hasMatch();
}

void CodeCompletionModelControllerInterface::aborted(KTextEditor::View *view)
{
    Q_UNUSED(view);
}

bool CodeCompletionModelControllerInterface::shouldExecute(const QModelIndex &index, QChar inserted)
{
    Q_UNUSED(index);
    Q_UNUSED(inserted);
    return false;
}

KTextEditor::CodeCompletionModelControllerInterface::MatchReaction CodeCompletionModelControllerInterface::matchingItem(const QModelIndex &selected)
{
    Q_UNUSED(selected)
    return HideListIfAutomaticInvocation;
}

bool CodeCompletionModelControllerInterface::shouldHideItemsWithEqualNames() const
{
    return false;
}

}
