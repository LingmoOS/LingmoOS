/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KATE_CMD_H
#define _KATE_CMD_H

#include <KCompletion>

#include <QHash>
#include <QStringList>

namespace KTextEditor
{
class Command;
}

class KateCmd
{
public:
    KateCmd();
    ~KateCmd();

    static KateCmd *self();

    bool registerCommand(KTextEditor::Command *cmd);
    bool unregisterCommand(KTextEditor::Command *cmd);
    KTextEditor::Command *queryCommand(const QString &cmd) const;
    QList<KTextEditor::Command *> commands() const;
    QStringList commandList() const;

    QStringList cmds();
    void appendHistory(const QString &cmd);
    const QString fromHistory(int i) const;
    uint historyLength() const
    {
        return m_history.count();
    }

    KCompletion *commandCompletionObject();

private:
    QHash<QString, KTextEditor::Command *> m_dict;
    QStringList m_cmds;
    QStringList m_history;
    KCompletion m_cmdCompletion; // shared completion object for all KateCmdLineEdits in each KTE::View
};

/**
 * A KCompletion object that completes last ?unquoted? word in the string
 * passed. Do not mistake "shell" for anything related to quoting, this
 * simply mimics shell tab completion by completing the last word in the
 * provided text.
 */
class KateCmdShellCompletion : public KCompletion
{
public:
    KateCmdShellCompletion();

    /**
     * Finds completions to the given text.
     * The first match is returned and emitted in the signal match().
     * @param text the text to complete
     * @return the first match, or QString() if not found
     */
    QString makeCompletion(const QString &text) override;

protected:
    // Called by KCompletion
    void postProcessMatch(QString *match) const override;
    void postProcessMatches(QStringList *matches) const override;
    void postProcessMatches(KCompletionMatches *matches) const override;

private:
    /**
     * Split text at the last unquoted space
     *
     * @param text_start will be set to the text at the left, including the space
     * @param text_compl Will be set to the text at the right. This is the text to complete.
     */
    void splitText(const QString &text, QString &text_start, QString &text_compl) const;

    QChar m_word_break_char;
    QChar m_quote_char1;
    QChar m_quote_char2;
    QChar m_escape_char;

    QString m_text_start;
    QString m_text_compl;
};

#endif
