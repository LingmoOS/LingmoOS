/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012 Vegard Øye
    SPDX-FileCopyrightText: 2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecommandrangeexpressionparser.h"

#include "katedocument.h"
#include "kateview.h"

#include <QStringList>

using KTextEditor::Cursor;
using KTextEditor::Range;

CommandRangeExpressionParser::CommandRangeExpressionParser()
{
    m_line = QStringLiteral("\\d+");
    m_lastLine = QStringLiteral("\\$");
    m_thisLine = QStringLiteral("\\.");

    m_forwardSearch = QStringLiteral("/([^/]*)/?");
    m_forwardSearch2 = QStringLiteral("/[^/]*/?"); // no group
    m_backwardSearch = QStringLiteral("\\?([^?]*)\\??");
    m_backwardSearch2 = QStringLiteral("\\?[^?]*\\??"); // no group
    m_base = QLatin1String("(?:%1)").arg(m_mark) + QLatin1String("|(?:%1)").arg(m_line) + QLatin1String("|(?:%1)").arg(m_thisLine)
        + QLatin1String("|(?:%1)").arg(m_lastLine) + QLatin1String("|(?:%1)").arg(m_forwardSearch2) + QLatin1String("|(?:%1)").arg(m_backwardSearch2);
    m_offset = QLatin1String("[+-](?:%1)?").arg(m_base);

    // The position regexp contains two groups: the base and the offset.
    // The offset may be empty.
    m_position = QStringLiteral("(%1)((?:%2)*)").arg(m_base, m_offset);

    // The range regexp contains seven groups: the first is the start position, the second is
    // the base of the start position, the third is the offset of the start position, the
    // fourth is the end position including a leading comma, the fifth is end position
    // without the comma, the sixth is the base of the end position, and the seventh is the
    // offset of the end position. The third and fourth groups may be empty, and the
    // fifth, sixth and seventh groups are contingent on the fourth group.
    m_cmdRangeRegex.setPattern(QStringLiteral("^(%1)((?:,(%1))?)").arg(m_position));
}

Range CommandRangeExpressionParser::parseRangeExpression(const QString &command,
                                                         KTextEditor::ViewPrivate *view,
                                                         QString &destRangeExpression,
                                                         QString &destTransformedCommand)
{
    CommandRangeExpressionParser rangeExpressionParser;
    return rangeExpressionParser.parseRangeExpression(command, destRangeExpression, destTransformedCommand, view);
}

Range CommandRangeExpressionParser::parseRangeExpression(const QString &command,
                                                         QString &destRangeExpression,
                                                         QString &destTransformedCommand,
                                                         KTextEditor::ViewPrivate *view)
{
    Range parsedRange(0, -1, 0, -1);
    if (command.isEmpty()) {
        return parsedRange;
    }
    QString commandTmp = command;
    bool leadingRangeWasPercent = false;
    // expand '%' to '1,$' ("all lines") if at the start of the line
    if (commandTmp.at(0) == QLatin1Char('%')) {
        commandTmp.replace(0, 1, QStringLiteral("1,$"));
        leadingRangeWasPercent = true;
    }

    const auto match = m_cmdRangeRegex.match(commandTmp);
    if (match.hasMatch() && match.capturedLength(0) > 0) {
        commandTmp.remove(m_cmdRangeRegex);

        const QString position_string1 = match.captured(1);
        QString position_string2 = match.captured(4);
        int position1 = calculatePosition(position_string1, view);

        int position2;
        if (!position_string2.isEmpty()) {
            // remove the comma
            position_string2 = match.captured(5);
            position2 = calculatePosition(position_string2, view);
        } else {
            position2 = position1;
        }

        // special case: if the command is just a number with an optional +/- prefix, rewrite to "goto"
        if (commandTmp.isEmpty()) {
            commandTmp = QStringLiteral("goto %1").arg(position1);
        } else {
            parsedRange.setRange(KTextEditor::Range(position1 - 1, 0, position2 - 1, 0));
        }

        destRangeExpression = leadingRangeWasPercent ? QStringLiteral("%") : match.captured(0);
        destTransformedCommand = commandTmp;
    }

    return parsedRange;
}

int CommandRangeExpressionParser::calculatePosition(const QString &string, KTextEditor::ViewPrivate *view)
{
    int pos = 0;
    std::vector<bool> operators_list;
    const QStringList split = string.split(QRegularExpression(QStringLiteral("[-+](?!([+-]|$))")));
    std::vector<int> values;

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

        static const auto lineRe = QRegularExpression(QRegularExpression::anchoredPattern(m_line));
        static const auto lastLineRe = QRegularExpression(QRegularExpression::anchoredPattern(m_lastLine));
        static const auto thisLineRe = QRegularExpression(QRegularExpression::anchoredPattern(m_thisLine));
        static const auto forwardSearchRe = QRegularExpression(QRegularExpression::anchoredPattern(m_forwardSearch));
        static const auto backwardSearchRe = QRegularExpression(QRegularExpression::anchoredPattern(m_backwardSearch));

        QRegularExpressionMatch rmatch;
        if (lineRe.match(line).hasMatch()) {
            values.push_back(line.toInt());
        } else if (lastLineRe.match(line).hasMatch()) {
            values.push_back(view->doc()->lines());
        } else if (thisLineRe.match(line).hasMatch()) {
            values.push_back(view->cursorPosition().line() + 1);
        } else if (line.contains(forwardSearchRe, &rmatch)) {
            const QString pattern = rmatch.captured(1);
            const int matchLine =
                view->doc()->searchText(Range(view->cursorPosition(), view->doc()->documentEnd()), pattern, KTextEditor::Regex).first().start().line();
            values.push_back((matchLine < 0) ? -1 : matchLine + 1);
        } else if (line.contains(backwardSearchRe, &rmatch)) {
            const QString pattern = rmatch.captured(1);
            const int matchLine = view->doc()->searchText(Range(Cursor(0, 0), view->cursorPosition()), pattern, KTextEditor::Regex).first().start().line();
            values.push_back((matchLine < 0) ? -1 : matchLine + 1);
        }
    }

    if (values.empty()) {
        return -1;
    }

    int result = values.at(0);
    for (size_t i = 0; i < operators_list.size(); ++i) {
        if (operators_list.at(i) == true) {
            result += values.at(i + 1);
        } else {
            result -= values.at(i + 1);
        }
    }

    return result;
}
