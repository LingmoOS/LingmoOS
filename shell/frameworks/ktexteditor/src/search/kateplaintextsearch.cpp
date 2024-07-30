/*
    SPDX-FileCopyrightText: 2009-2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2007 Sebastian Pipping <webmaster@hartwork.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN includes
#include "kateplaintextsearch.h"

#include "katepartdebug.h"
#include "kateregexpsearch.h"
#include <ktexteditor/document.h>

#include <QRegularExpression>
// END  includes

// BEGIN d'tor, c'tor
//
// KateSearch Constructor
//
KatePlainTextSearch::KatePlainTextSearch(const KTextEditor::Document *document, Qt::CaseSensitivity caseSensitivity, bool wholeWords)
    : m_document(document)
    , m_caseSensitivity(caseSensitivity)
    , m_wholeWords(wholeWords)
{
}

// END

KTextEditor::Range KatePlainTextSearch::search(const QString &text, KTextEditor::Range inputRange, bool backwards)
{
    // abuse regex for whole word plaintext search
    if (m_wholeWords) {
        // escape dot and friends
        const QString workPattern = QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(text));

        QRegularExpression::PatternOptions options;
        options |= m_caseSensitivity == Qt::CaseInsensitive ? QRegularExpression::CaseInsensitiveOption : QRegularExpression::NoPatternOption;

        return KateRegExpSearch(m_document).search(workPattern, inputRange, backwards, options).at(0);
    }

    if (text.isEmpty() || !inputRange.isValid() || (inputRange.start() == inputRange.end())) {
        return KTextEditor::Range::invalid();
    }

    // split multi-line needle into single lines
    const QList<QStringView> needleLines = QStringView(text).split(QLatin1Char('\n'));

    if (needleLines.count() > 1) {
        // multi-line plaintext search (both forwards or backwards)
        const int forMin = inputRange.start().line(); // first line in range
        const int forMax = inputRange.end().line() + 1 - needleLines.count(); // last line in range
        const int forInit = backwards ? forMax : forMin;
        const int forInc = backwards ? -1 : +1;

        for (int j = forInit; (forMin <= j) && (j <= forMax); j += forInc) {
            // try to match all lines
            const int startCol = m_document->lineLength(j) - needleLines[0].length();
            for (int k = 0; k < needleLines.count(); k++) {
                // which lines to compare
                const auto &needleLine = needleLines[k];
                const QString &hayLine = m_document->line(j + k);

                // position specific comparison (first, middle, last)
                if (k == 0) {
                    // first line
                    if (forMin == j && startCol < inputRange.start().column()) {
                        break;
                    }

                    // NOTE: QString("")::endsWith("") is false in Qt, therefore we need the additional checks.
                    const bool endsWith = hayLine.endsWith(needleLine, m_caseSensitivity) || (hayLine.isEmpty() && needleLine.isEmpty());
                    if (!endsWith) {
                        break;
                    }
                } else if (k == needleLines.count() - 1) {
                    // last line
                    const int maxRight = (j + k == inputRange.end().line()) ? inputRange.end().column() : hayLine.length();

                    // NOTE: QString("")::startsWith("") is false in Qt, therefore we need the additional checks.
                    const bool startsWith = hayLine.startsWith(needleLine, m_caseSensitivity) || (hayLine.isEmpty() && needleLine.isEmpty());
                    if (startsWith && needleLine.length() <= maxRight) {
                        return KTextEditor::Range(j, startCol, j + k, needleLine.length());
                    }
                } else {
                    // mid lines
                    if (hayLine.compare(needleLine, m_caseSensitivity) != 0) {
                        break;
                    }
                }
            }
        }

        // not found
        return KTextEditor::Range::invalid();
    } else {
        // single-line plaintext search (both forward of backward mode)
        const int startCol = inputRange.start().column();
        const int endCol = inputRange.end().column(); // first not included
        const int startLine = inputRange.start().line();
        const int endLine = inputRange.end().line();
        const int forInc = backwards ? -1 : +1;

        for (int line = backwards ? endLine : startLine; (startLine <= line) && (line <= endLine); line += forInc) {
            if ((line < 0) || (m_document->lines() <= line)) {
                qCWarning(LOG_KTE) << "line " << line << " is not within interval [0.." << m_document->lines() << ") ... returning invalid range";
                return KTextEditor::Range::invalid();
            }

            const QString textLine = m_document->line(line);

            const int offset = (line == startLine) ? startCol : 0;
            const int line_end = (line == endLine) ? endCol : textLine.length();
            const int foundAt =
                backwards ? textLine.lastIndexOf(text, line_end - text.length(), m_caseSensitivity) : textLine.indexOf(text, offset, m_caseSensitivity);

            if ((offset <= foundAt) && (foundAt + text.length() <= line_end)) {
                return KTextEditor::Range(line, foundAt, line, foundAt + text.length());
            }
        }
    }
    return KTextEditor::Range::invalid();
}
