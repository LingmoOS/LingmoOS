/*
    SPDX-FileCopyrightText: 2017 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katescripteditor.h"

#include "kateglobal.h"

#include <QApplication>
#include <QClipboard>
#include <QJSEngine>

QString KateScriptEditor::clipboardText() const
{
    return QApplication::clipboard()->text();
}

QStringList KateScriptEditor::clipboardHistory() const
{
    const auto clipboardHistory = KTextEditor::EditorPrivate::self()->clipboardHistory();

    QStringList res;
    for (const auto &entry : clipboardHistory) {
        res << entry.text;
    }

    return res;
}

void KateScriptEditor::setClipboardText(const QString &text)
{
    KTextEditor::EditorPrivate::self()->copyToClipboard(text, QString());
}

#include "moc_katescripteditor.cpp"
