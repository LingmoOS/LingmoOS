/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012 Vegard Øye
    SPDX-FileCopyrightText: 2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "commandrangeexpressionparser.h"

#include "katedocument.h"
#include "kateview.h"
#include "marks.h"
#include <vimode/inputmodemanager.h>

#include <QRegularExpression>
#include <QStringList>

using namespace KateVi;

#define RegExp(name, pattern)                                                                                                                                  \
    inline const QRegularExpression &name()                                                                                                                    \
    {                                                                                                                                                          \
        static const QRegularExpression regex(QStringLiteral(pattern), QRegularExpression::UseUnicodePropertiesOption);                                        \
        return regex;                                                                                                                                          \
    }

namespace
{
#define RE_MARK "\\'[0-9a-z><\\+\\*\\_]"
#define RE_THISLINE "\\."
#define RE_LASTLINE "\\$"
#define RE_LINE "\\d+"
#define RE_FORWARDSEARCH "/[^/]*/?"
#define RE_BACKWARDSEARCH "\\?[^?]*\\??"
#define RE_BASE "(?:" RE_MARK ")|(?:" RE_LINE ")|(?:" RE_THISLINE ")|(?:" RE_LASTLINE ")|(?:" RE_FORWARDSEARCH ")|(?:" RE_BACKWARDSEARCH ")"
#define RE_OFFSET "[+-](?:" RE_BASE ")?"
#define RE_POSITION "(" RE_BASE ")((?:" RE_OFFSET ")*)"

RegExp(RE_Line, RE_LINE) RegExp(RE_LastLine, RE_LASTLINE) RegExp(RE_ThisLine, RE_THISLINE) RegExp(RE_Mark, RE_MARK) RegExp(RE_ForwardSearch, "^/([^/]*)/?$")
    RegExp(RE_BackwardSearch, "^\\?([^?]*)\\??$") RegExp(RE_CalculatePositionSplit, "[-+](?!([+-]|$))")
    // The range regexp contains seven groups: the first is the start position, the second is
    // the base of the start position, the third is the offset of the start position, the
    // fourth is the end position including a leading comma, the fifth is end position
    // without the comma, the sixth is the base of the end position, and the seventh is the
    // offset of the end position. The third and fourth groups may be empty, and the
    // fifth, sixth and seventh groups are contingent on the fourth group.
    inline const QRegularExpression &RE_CmdRange()
{
    static const QRegularExpression regex(QStringLiteral("^(" RE_POSITION ")((?:,(" RE_POSITION "))?)"));
    return regex;
}
}

CommandRangeExpressionParser::CommandRangeExpressionParser(InputModeManager *vimanager)
    : m_viInputModeManager(vimanager)
{
}

QString CommandRangeExpressionParser::parseRangeString(const QString &command)
{
    if (command.isEmpty()) {
        return QString();
    }

    if (command.at(0) == QLatin1Char('%')) {
        return QStringLiteral("%");
    }

    QRegularExpressionMatch rangeMatch = RE_CmdRange().match(command);

    return rangeMatch.hasMatch() ? rangeMatch.captured() : QString();
}

KTextEditor::Range CommandRangeExpressionParser::parseRange(const QString &command, QString &destTransformedCommand) const
{
    if (command.isEmpty()) {
        return KTextEditor::Range::invalid();
    }

    QString commandTmp = command;

    // expand '%' to '1,$' ("all lines") if at the start of the line
    if (commandTmp.at(0) == QLatin1Char('%')) {
        commandTmp.replace(0, 1, QStringLiteral("1,$"));
    }

    QRegularExpressionMatch rangeMatch = RE_CmdRange().match(commandTmp);

    if (!rangeMatch.hasMatch()) {
        return KTextEditor::Range::invalid();
    }

    QString position_string1 = rangeMatch.captured(1);
    QString position_string2 = rangeMatch.captured(4);
    int position1 = calculatePosition(position_string1);
    int position2 = (position_string2.isEmpty()) ? position1 : calculatePosition(rangeMatch.captured(5));

    commandTmp.remove(RE_CmdRange());

    // Vi indexes lines starting from 1; however, it does accept 0 as a valid line index
    // and treats it as 1
    position1 = (position1 == 0) ? 1 : position1;
    position2 = (position2 == 0) ? 1 : position2;

    // special case: if the command is just a number with an optional +/- prefix, rewrite to "goto"
    if (commandTmp.isEmpty()) {
        destTransformedCommand = QStringLiteral("goto %1").arg(position1);
        return KTextEditor::Range::invalid();
    } else {
        destTransformedCommand = commandTmp;
        return KTextEditor::Range(KTextEditor::Range(position1 - 1, 0, position2 - 1, 0));
    }
}

int CommandRangeExpressionParser::calculatePosition(const QString &string) const
{
    int pos = 0;
    QList<bool> operators_list;
    const QStringList split = string.split(RE_CalculatePositionSplit());
    QList<int> values;

    for (const QString &line : split) {
        pos += line.size();

        if (pos < string.size()) {
            if (string.at(pos) == QLatin1Char('+')) {
                operators_list.push_back(true);
            } else if (string.at(pos) == QLatin1Char('-')) {
                operators_list.push_back(false);
            } else {
                Q_ASSERT(false);
            }
        }

        ++pos;

        matchLineNumber(line, values) || matchLastLine(line, values) || matchThisLine(line, values) || matchMark(line, values)
            || matchForwardSearch(line, values) || matchBackwardSearch(line, values);
    }

    if (values.isEmpty()) {
        return -1;
    }

    int result = values.at(0);
    for (int i = 0; i < operators_list.size(); ++i) {
        if (operators_list.at(i)) {
            result += values.at(i + 1);
        } else {
            result -= values.at(i + 1);
        }
    }

    return result;
}

bool CommandRangeExpressionParser::matchLineNumber(const QString &line, QList<int> &values)
{
    QRegularExpressionMatch match = RE_Line().match(line);

    if (!match.hasMatch() || match.capturedLength() != line.length()) {
        return false;
    }

    values.push_back(line.toInt());
    return true;
}

bool CommandRangeExpressionParser::matchLastLine(const QString &line, QList<int> &values) const
{
    QRegularExpressionMatch match = RE_LastLine().match(line);

    if (!match.hasMatch() || match.capturedLength() != line.length()) {
        return false;
    }

    values.push_back(m_viInputModeManager->view()->doc()->lines());
    return true;
}

bool CommandRangeExpressionParser::matchThisLine(const QString &line, QList<int> &values) const
{
    QRegularExpressionMatch match = RE_ThisLine().match(line);

    if (!match.hasMatch() || match.capturedLength() != line.length()) {
        return false;
    }

    values.push_back(m_viInputModeManager->view()->cursorPosition().line() + 1);
    return true;
}

bool CommandRangeExpressionParser::matchMark(const QString &line, QList<int> &values) const
{
    QRegularExpressionMatch match = RE_Mark().match(line);

    if (!match.hasMatch() || match.capturedLength() != line.length()) {
        return false;
    }

    values.push_back(m_viInputModeManager->marks()->getMarkPosition(line.at(1)).line() + 1);
    return true;
}

bool CommandRangeExpressionParser::matchForwardSearch(const QString &line, QList<int> &values) const
{
    QRegularExpressionMatch match = RE_ForwardSearch().match(line);

    if (!match.hasMatch()) {
        return false;
    }

    QString pattern = match.captured(1);
    KTextEditor::Range range(m_viInputModeManager->view()->cursorPosition(), m_viInputModeManager->view()->doc()->documentEnd());
    QList<KTextEditor::Range> matchingLines = m_viInputModeManager->view()->doc()->searchText(range, pattern, KTextEditor::Regex);

    if (matchingLines.isEmpty()) {
        return true;
    }

    int lineNumber = matchingLines.first().start().line();

    values.push_back(lineNumber + 1);
    return true;
}

bool CommandRangeExpressionParser::matchBackwardSearch(const QString &line, QList<int> &values) const
{
    QRegularExpressionMatch match = RE_BackwardSearch().match(line);

    if (!match.hasMatch()) {
        return false;
    }

    QString pattern = match.captured(1);
    KTextEditor::Range range(KTextEditor::Cursor(0, 0), m_viInputModeManager->view()->cursorPosition());
    QList<KTextEditor::Range> matchingLines = m_viInputModeManager->view()->doc()->searchText(range, pattern, KTextEditor::Regex);

    if (matchingLines.isEmpty()) {
        return true;
    }

    int lineNumber = matchingLines.first().start().line();

    values.push_back(lineNumber + 1);
    return true;
}
