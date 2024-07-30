/*
    SPDX-FileCopyrightText: 2009-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_COMMANDLINE_SCRIPT_H
#define KATE_COMMANDLINE_SCRIPT_H

#include "katescript.h"

#include <KTextEditor/Command>

#include <QJsonArray>

namespace KTextEditor
{
class View;
}

class KateCommandLineScriptHeader
{
public:
    KateCommandLineScriptHeader()
    {
    }

    inline void setFunctions(const QStringList &functions)
    {
        m_functions = functions;
    }
    inline const QStringList &functions() const
    {
        return m_functions;
    }

    inline void setActions(const QJsonArray &actions)
    {
        m_actions = actions;
    }
    inline const QJsonArray &actions() const
    {
        return m_actions;
    }

private:
    QStringList m_functions; ///< the functions the script contains
    QJsonArray m_actions; ///< the action for this script
};

/**
 * A specialized class for scripts that are of type ScriptType::Indentation.
 */
class KateCommandLineScript : public KateScript, public KTextEditor::Command
{
public:
    KateCommandLineScript(const QString &url, const KateCommandLineScriptHeader &header);

    const KateCommandLineScriptHeader &commandHeader();

    bool callFunction(const QString &cmd, const QStringList &args, QString &errorMessage);

    //
    // KTextEditor::Command interface
    //
public:
    bool help(KTextEditor::View *view, const QString &cmd, QString &msg) override;
    bool exec(KTextEditor::View *view, const QString &cmd, QString &msg, const KTextEditor::Range &range = KTextEditor::Range::invalid()) override;
    bool supportsRange(const QString &cmd) override;

private:
    KateCommandLineScriptHeader m_commandHeader;
};

#endif
