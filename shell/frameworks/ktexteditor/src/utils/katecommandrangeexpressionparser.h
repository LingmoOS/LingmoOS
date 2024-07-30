/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012 Vegard Øye
    SPDX-FileCopyrightText: 2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_COMMAND_RANGE_EXPRESSION_PARSER_INCLUDED
#define KATE_COMMAND_RANGE_EXPRESSION_PARSER_INCLUDED

#include <ktexteditor/range.h>

#include <QRegularExpression>

namespace KTextEditor
{
class ViewPrivate;
}

class CommandRangeExpressionParser
{
public:
    CommandRangeExpressionParser();
    /**
     * Attempt to parse any leading range expression (e.g. "%", "'<,'>", ".,+6" etc) in @c command and
     * return it as a Range.  If parsing was successful, the range will be valid, the string
     * making up the range expression will be placed in @c destRangeExpression, and the command with
     * the range stripped will be placed in @c destTransformedCommand.  In some special cases,
     * the @c destTransformedCommand will be further re-written e.g. a command in the form of just a number
     * will be rewritten as "goto <number>".
     *
     * An invalid Range is returned if no leading range expression could be found.
     */
    static KTextEditor::Range
    parseRangeExpression(const QString &command, KTextEditor::ViewPrivate *view, QString &destRangeExpression, QString &destTransformedCommand);

private:
    KTextEditor::Range
    parseRangeExpression(const QString &command, QString &destRangeExpression, QString &destTransformedCommand, KTextEditor::ViewPrivate *view);
    int calculatePosition(const QString &string, KTextEditor::ViewPrivate *view);
    QString m_line;
    QString m_lastLine;
    QString m_thisLine;
    QString m_mark;
    QString m_forwardSearch;
    QString m_forwardSearch2;
    QString m_backwardSearch;
    QString m_backwardSearch2;
    QString m_base;
    QString m_offset;
    QString m_position;
    QRegularExpression m_cmdRangeRegex;
};

#endif
