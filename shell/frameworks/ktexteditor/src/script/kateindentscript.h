/*
    SPDX-FileCopyrightText: 2008 Paul Giannaros <paul@giannaros.org>
    SPDX-FileCopyrightText: 2009-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_INDENT_SCRIPT_H
#define KATE_INDENT_SCRIPT_H

#include "katescript.h"

#include <KTextEditor/Cursor>

namespace KTextEditor
{
class ViewPrivate;
}

class KateIndentScriptHeader
{
public:
    KateIndentScriptHeader() = default;

    inline void setName(const QString &name)
    {
        m_name = name;
    }
    inline const QString &name() const
    {
        return m_name;
    }

    inline void setRequiredStyle(const QString &requiredStyle)
    {
        m_requiredStyle = requiredStyle;
    }
    inline const QString &requiredStyle() const
    {
        return m_requiredStyle;
    }

    inline void setIndentLanguages(const QStringList &indentLanguages)
    {
        m_indentLanguages = indentLanguages;
    }
    inline const QStringList &indentLanguages() const
    {
        return m_indentLanguages;
    }

    inline void setPriority(int priority)
    {
        m_priority = priority;
    }
    inline int priority() const
    {
        return m_priority;
    }

    inline void setBaseName(const QString &baseName)
    {
        m_baseName = baseName;
    }
    inline const QString &baseName() const
    {
        return m_baseName;
    }

private:
    QString m_name; ///< indenter name, e.g. Python

    /**
     * If this is an indenter, then this specifies the required syntax
     * highlighting style that must be used for this indenter to work properly.
     * If this property is empty, the indenter doesn't require a specific style.
     */
    QString m_requiredStyle;
    /**
     * If this script is an indenter, then the indentLanguages member specifies
     * which languages this is an indenter for. The values must correspond with
     * the name of a programming language given in a highlighting file (e.g "TI Basic")
     */
    QStringList m_indentLanguages;
    /**
     * If this script is an indenter, this value controls the priority it will take
     * when an indenter for one of the supported languages is requested and multiple
     * indenters are found
     */
    int m_priority = 0;

    /**
     * basename of script
     */
    QString m_baseName;
};

/**
 * A specialized class for scripts that are of type ScriptType::Indentation.
 */
class KateIndentScript : public KateScript
{
public:
    explicit KateIndentScript(const QString &url, const KateIndentScriptHeader &header);

    const QString &triggerCharacters();

    const KateIndentScriptHeader &indentHeader() const;

    /**
     * Returns a pair where the first value is the indent amount, and the second
     * value is the alignment.
     */
    QPair<int, int> indent(KTextEditor::ViewPrivate *view, const KTextEditor::Cursor position, QChar typedCharacter, int indentWidth);

private:
    QString m_triggerCharacters;
    bool m_triggerCharactersSet = false;
    KateIndentScriptHeader m_indentHeader;
};

#endif
