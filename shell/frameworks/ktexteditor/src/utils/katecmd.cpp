/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecmd.h"
#include "kateglobal.h"

#include "katepartdebug.h"
#include <KCompletionMatches>

#include <ktexteditor/command.h>

// BEGIN KateCmd
#define CMD_HIST_LENGTH 256

KateCmd::KateCmd()
{
    m_cmdCompletion.addItem(QStringLiteral("help"));
}

KateCmd::~KateCmd() = default;

bool KateCmd::registerCommand(KTextEditor::Command *cmd)
{
    const QStringList &l = cmd->cmds();

    for (int z = 0; z < l.count(); z++) {
        if (m_dict.contains(l[z])) {
            qCDebug(LOG_KTE) << "Command already registered: " << l[z] << ". Aborting.";
            return false;
        }
    }

    for (int z = 0; z < l.count(); z++) {
        m_dict.insert(l[z], cmd);
        // qCDebug(LOG_KTE)<<"Inserted command:"<<l[z];
    }

    m_cmds += l;
    m_cmdCompletion.insertItems(l);

    return true;
}

bool KateCmd::unregisterCommand(KTextEditor::Command *cmd)
{
    QStringList l;

    QHash<QString, KTextEditor::Command *>::const_iterator i = m_dict.constBegin();
    while (i != m_dict.constEnd()) {
        if (i.value() == cmd) {
            l << i.key();
        }
        ++i;
    }

    for (QStringList::Iterator it1 = l.begin(); it1 != l.end(); ++it1) {
        m_dict.remove(*it1);
        m_cmdCompletion.removeItem(*it1);
        // qCDebug(LOG_KTE)<<"Removed command:"<<*it1;
    }

    return true;
}

KTextEditor::Command *KateCmd::queryCommand(const QString &cmd) const
{
    // a command can be named ".*[\w\-]+" with the constrain that it must
    // contain at least one letter.
    int f = 0;
    bool b = false;

    // special case: '-' and '_' can be part of a command name, but if the
    // command is 's' (substitute), it should be considered the delimiter and
    // should not be counted as part of the command name
    if (cmd.length() >= 2 && cmd.at(0) == QLatin1Char('s') && (cmd.at(1) == QLatin1Char('-') || cmd.at(1) == QLatin1Char('_'))) {
        return m_dict.value(QStringLiteral("s"));
    }

    for (; f < cmd.length(); f++) {
        if (cmd[f].isLetter()) {
            b = true;
        }
        if (b && (!cmd[f].isLetterOrNumber() && cmd[f] != QLatin1Char('-') && cmd[f] != QLatin1Char('_'))) {
            break;
        }
    }
    return m_dict.value(cmd.left(f));
}

QList<KTextEditor::Command *> KateCmd::commands() const
{
    return m_dict.values();
}

QStringList KateCmd::commandList() const
{
    return m_cmds;
}

KateCmd *KateCmd::self()
{
    return KTextEditor::EditorPrivate::self()->cmdManager();
}

void KateCmd::appendHistory(const QString &cmd)
{
    if (!m_history.isEmpty()) { // this line should be backported to 3.x
        if (m_history.last() == cmd) {
            return;
        }
    }

    if (m_history.count() == CMD_HIST_LENGTH) {
        m_history.removeFirst();
    }

    m_history.append(cmd);
}

const QString KateCmd::fromHistory(int index) const
{
    if (index < 0 || index > m_history.count() - 1) {
        return QString();
    }
    return m_history[index];
}

KCompletion *KateCmd::commandCompletionObject()
{
    return &m_cmdCompletion;
}
// END KateCmd

// BEGIN KateCmdShellCompletion
/*
   A lot of the code in the below class is copied from
   kdelibs/kio/kio/kshellcompletion.cpp
   SPDX-FileCopyrightText: 2000 David Smith <dsmith@algonet.se>
   SPDX-FileCopyrightText: 2004 Anders Lund <anders@alweb.dk>
*/
KateCmdShellCompletion::KateCmdShellCompletion()
    : KCompletion()
{
    m_word_break_char = QLatin1Char(' ');
    m_quote_char1 = QLatin1Char('\"');
    m_quote_char2 = QLatin1Char('\'');
    m_escape_char = QLatin1Char('\\');
}

QString KateCmdShellCompletion::makeCompletion(const QString &text)
{
    // Split text at the last unquoted space
    //
    splitText(text, m_text_start, m_text_compl);

    // Make completion on the last part of text
    //
    return KCompletion::makeCompletion(m_text_compl);
}

void KateCmdShellCompletion::postProcessMatch(QString *match) const
{
    if (match->isNull()) {
        return;
    }

    match->prepend(m_text_start);
}

void KateCmdShellCompletion::postProcessMatches(QStringList *matches) const
{
    for (QStringList::Iterator it = matches->begin(); it != matches->end(); it++) {
        if (!(*it).isNull()) {
            (*it).prepend(m_text_start);
        }
    }
}

void KateCmdShellCompletion::postProcessMatches(KCompletionMatches *matches) const
{
    for (KCompletionMatches::Iterator it = matches->begin(); it != matches->end(); it++) {
        if (!(*it).value().isNull()) {
            (*it).value().prepend(m_text_start);
        }
    }
}

void KateCmdShellCompletion::splitText(const QString &text, QString &text_start, QString &text_compl) const
{
    bool in_quote = false;
    bool escaped = false;
    QChar p_last_quote_char;
    int last_unquoted_space = -1;

    for (int pos = 0; pos < text.length(); pos++) {
        if (escaped) {
            escaped = false;
        } else if (in_quote && text[pos] == p_last_quote_char) {
            in_quote = false;
        } else if (!in_quote && text[pos] == m_quote_char1) {
            p_last_quote_char = m_quote_char1;
            in_quote = true;
        } else if (!in_quote && text[pos] == m_quote_char2) {
            p_last_quote_char = m_quote_char2;
            in_quote = true;
        } else if (text[pos] == m_escape_char) {
            escaped = true;
        } else if (!in_quote && text[pos] == m_word_break_char) {
            while (pos + 1 < text.length() && text[pos + 1] == m_word_break_char) {
                pos++;
            }

            if (pos + 1 == text.length()) {
                break;
            }

            last_unquoted_space = pos;
        }
    }

    text_start = text.left(last_unquoted_space + 1);

    // the last part without trailing blanks
    text_compl = text.mid(last_unquoted_space + 1);
}

// END KateCmdShellCompletion
