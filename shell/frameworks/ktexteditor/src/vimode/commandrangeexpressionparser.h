/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012 Vegard Øye
    SPDX-FileCopyrightText: 2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_COMMAND_RANGE_EXPRESSION_PARSER
#define KATEVI_COMMAND_RANGE_EXPRESSION_PARSER

#include <ktexteditor/range.h>

namespace KateVi
{
class InputModeManager;

class CommandRangeExpressionParser
{
public:
    explicit CommandRangeExpressionParser(InputModeManager *vimanager);

    /**
     * Attempt to parse any leading range expression (e.g. "%", "'<,'>", ".,+6" etc) in @c command and
     * return it as a Range.  If parsing was successful, the range will be valid and the command with
     * the range stripped will be placed in @c destTransformedCommand.  In some special cases,
     * the @c destTransformedCommand will be further re-written e.g. a command in the form of just a number
     * will be rewritten as "goto <number>".
     *
     * An invalid Range is returned if no leading range expression could be found.
     */
    KTextEditor::Range parseRange(const QString &command, QString &destTransformedCommand) const;

    /**
     * Attempts to find range expression for vi command
     * @returns range sub string of passed command or empty if not found
     */
    static QString parseRangeString(const QString &command);

private:
    int calculatePosition(const QString &string) const;

    static bool matchLineNumber(const QString &line, QList<int> &values);
    bool matchLastLine(const QString &line, QList<int> &values) const;
    bool matchThisLine(const QString &line, QList<int> &values) const;
    bool matchMark(const QString &line, QList<int> &values) const;
    bool matchForwardSearch(const QString &line, QList<int> &values) const;
    bool matchBackwardSearch(const QString &line, QList<int> &values) const;

private:
    InputModeManager *m_viInputModeManager;
};

}

#endif
