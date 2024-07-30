/*
    SPDX-FileCopyrightText: 2008 Paul Giannaros <paul@giannaros.org>
    SPDX-FileCopyrightText: 2009-2018 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2010 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SCRIPT_MANAGER_H
#define KATE_SCRIPT_MANAGER_H

#include <KTextEditor/Command>
#include <ktexteditor/cursor.h>

#include <QHash>
#include <QList>

class QString;
class KateIndentScript;
class KateCommandLineScript;

/**
 * Manage the scripts on disks -- find them and query them.
 * Provides access to loaded scripts too.
 */
class KateScriptManager : public KTextEditor::Command
{
    Q_OBJECT

    KateScriptManager();
    static KateScriptManager *m_instance;

public:
    ~KateScriptManager() override;

    /** Get all scripts available in the command line */
    const QList<KateCommandLineScript *> &commandLineScripts()
    {
        return m_commandLineScripts;
    }

    /**
     * Get an indentation script for the given language -- if there is more than
     * one result, this will return the script with the highest priority. If
     * both have the same priority, an arbitrary script will be returned.
     * If no scripts are found, 0 is returned.
     */
    KateIndentScript *indenter(const QString &language);

    //
    // KTextEditor::Command
    //
public:
    /**
     * execute command
     * @param view view to use for execution
     * @param cmd cmd string
     * @param errorMsg error to return if no success
     * @return success
     */
    bool exec(KTextEditor::View *view, const QString &cmd, QString &errorMsg, const KTextEditor::Range &) override;

    /**
     * get help for a command
     * @param view view to use
     * @param cmd cmd name
     * @param msg help message
     * @return help available or not
     */
    bool help(KTextEditor::View *view, const QString &cmd, QString &msg) override;

    static KateScriptManager *self()
    {
        if (m_instance == nullptr) {
            m_instance = new KateScriptManager();
        }
        return m_instance;
    }

    //
    // Helper methods
    //
public:
    /**
     * Collect all scripts.
     */
    void collect();

public:
    KateIndentScript *indentationScript(const QString &scriptname)
    {
        return m_indentationScriptMap.value(scriptname);
    }

    int indentationScriptCount()
    {
        return m_indentationScripts.size();
    }
    KateIndentScript *indentationScriptByIndex(int index)
    {
        return m_indentationScripts[index];
    }

public:
    /** explicitly reload all scripts */
    void reload();

Q_SIGNALS:
    /** this signal is emitted when all scripts are _deleted_ and reloaded again. */
    void reloaded();

private:
    /** List of all command line scripts */
    QList<KateCommandLineScript *> m_commandLineScripts;

    /** list of all indentation scripts */
    QList<KateIndentScript *> m_indentationScripts;

    /** hash of all existing indenter scripts, hashes basename -> script */
    QHash<QString, KateIndentScript *> m_indentationScriptMap;

    /** Map of language to indent scripts */
    QHash<QString, QList<KateIndentScript *>> m_languageToIndenters;
};

#endif
