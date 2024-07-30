/*
    SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_VARIABLE_H
#define KTEXTEDITOR_VARIABLE_H

#include <QStringList>
#include <functional>

namespace KTextEditor
{
class View;

/**
 * @brief Variable for variable expansion.
 *
 * @section variable_intro Introduction
 *
 * A Variable is used by the KTextEditor::Editor to expand variables, also
 * know as expanding macros. A Variable itself is defined by the variable
 * name() as well as a description() and a function that replaces the variable
 * by its value.
 *
 * To register a Variable in the Editor use either Editor::registerVariableMatch()
 * or Editor::registerPrefixMatch().
 *
 * @see KTextEditor::Editor, KTextEditor::Editor::registerVariableMatch(),
 *      KTextEditor::Editor::registerPrefixMatch()
 * @author Dominik Haumann \<dhaumann@kde.org\>
 */
class Variable
{
public:
    /**
     * Function that is called to expand a variable in @p text.
     * @param text
     */
    using ExpandFunction = std::function<QString(const QStringView &text, KTextEditor::View *view)>;

    /**
     * Constructs an invalid Variable, see isValid().
     */
    Variable() = default;

    /**
     * Constructor defining a Variable by its @p name, its @p description, and
     * its function @p expansionFunc to expand a variable to its corresponding
     * value. The parameter @p isPrefixMatch indicates whether this Variable
     * represents an exact match (false) or a prefix match (true).
     *
     * @note The @p name should @e not be translated.
     */
    Variable(const QString &name, const QString &description, ExpandFunction expansionFunc, bool isPrefixMatch);

    /**
     * Copy constructor.
     */
    Variable(const Variable &copy) = default;

    /**
     * Assignment operator.
     */
    Variable &operator=(const Variable &copy) = default;

    /**
     * Returns true, if the name is non-empty and the function provided in the
     * constructor is not a nullptr.
     */
    bool isValid() const;

    /**
     * Returns whether this Variable represents an exact match (false) or a
     * prefix match (true).
     */
    bool isPrefixMatch() const;

    /**
     * Returns the @p name that was provided in the constructor.
     * Depending on where the Variable is registered, this name is used to
     * identify an exact match or a prefix match.
     */
    QString name() const;

    /**
     * Returns the description that was provided in the constructor.
     */
    QString description() const;

    /**
     * Expands the Variable to its value.
     *
     * As example for an exact match, a variable "CurerntDocument:Cursor:Line"
     * uses the @p view to return the current line of the text cursor. In this
     * case @p prefix equals the text of the variable itself, i.e.
     * "CurerntDocument:Cursor:Line".
     *
     * As example of a prefix match, a variable "ENV:value" expands the
     * environment value @e value, e.g. "ENV:HOME". In this case, the @p prefix
     * equals the text "ENV:HOME" and @p view would be unused.
     *
     * @return the expanded variable.
     */
    QString evaluate(const QStringView &prefix, KTextEditor::View *view) const;

private:
    QString m_name;
    QString m_description;
    ExpandFunction m_function;
    bool m_isPrefixMatch = false;
};

}

#endif
