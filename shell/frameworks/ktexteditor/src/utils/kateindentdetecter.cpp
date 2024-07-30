/*
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kateindentdetecter.h"

#include "katedocument.h"

KateIndentDetecter::KateIndentDetecter(KTextEditor::DocumentPrivate *doc)
    : m_doc(doc)
{
}

struct SpacesDiffResult {
    int spacesDiff = 0;
    bool looksLikeAlignment = false;
};

static SpacesDiffResult spacesDiff(const QString &a, int aLength, const QString &b, int bLength)
{
    SpacesDiffResult result;
    result.spacesDiff = 0;
    result.looksLikeAlignment = false;

    // This can go both ways (e.g.):
    //  - a: "\t"
    //  - b: "\t    "
    //  => This should count 1 tab and 4 spaces

    int i = 0;

    for (i = 0; i < aLength && i < bLength; i++) {
        const auto aCharCode = a.at(i);
        const auto bCharCode = b.at(i);

        if (aCharCode != bCharCode) {
            break;
        }
    }

    int aSpacesCnt = 0;
    int aTabsCount = 0;
    for (int j = i; j < aLength; j++) {
        const auto aCharCode = a.at(j);
        if (aCharCode == QLatin1Char(' ')) {
            aSpacesCnt++;
        } else {
            aTabsCount++;
        }
    }

    int bSpacesCnt = 0;
    int bTabsCount = 0;
    for (int j = i; j < bLength; j++) {
        const auto bCharCode = b.at(j);
        if (bCharCode == QLatin1Char(' ')) {
            bSpacesCnt++;
        } else {
            bTabsCount++;
        }
    }

    if (aSpacesCnt > 0 && aTabsCount > 0) {
        return result;
    }
    if (bSpacesCnt > 0 && bTabsCount > 0) {
        return result;
    }

    const auto tabsDiff = std::abs(aTabsCount - bTabsCount);
    const auto spacesDiff = std::abs(aSpacesCnt - bSpacesCnt);

    if (tabsDiff == 0) {
        // check if the indentation difference might be caused by alignment reasons
        // sometime folks like to align their code, but this should not be used as a hint
        result.spacesDiff = spacesDiff;

        if (spacesDiff > 0 && 0 <= bSpacesCnt - 1 && bSpacesCnt - 1 < a.length() && bSpacesCnt < b.length()) {
            if (b.at(bSpacesCnt) != QLatin1Char(' ') && a.at(bSpacesCnt - 1) == QLatin1Char(' ')) {
                if (a.at(a.length() - 1) == QLatin1Char(',')) {
                    // This looks like an alignment desire: e.g.
                    // const a = b + c,
                    //       d = b - c;
                    result.looksLikeAlignment = true;
                }
            }
        }
        return result;
    }
    if (spacesDiff % tabsDiff == 0) {
        result.spacesDiff = spacesDiff / tabsDiff;
        return result;
    }
    return result;
}

KateIndentDetecter::Result KateIndentDetecter::detect(int defaultTabSize, bool defaultInsertSpaces)
{
    // Look at most at the first 10k lines
    const int linesCount = std::min(m_doc->lines(), 10000);

    int linesIndentedWithTabsCount = 0; // number of lines that contain at least one tab in indentation
    int linesIndentedWithSpacesCount = 0; // number of lines that contain only spaces in indentation

    QString previousLineText; // content of latest line that contained non-whitespace chars
    int previousLineIndentation = 0; // index at which latest line contained the first non-whitespace char

    constexpr int ALLOWED_TAB_SIZE_GUESSES[] = {2, 4, 6, 8, 3, 5, 7}; // prefer even guesses for `tabSize`, limit to [2, 8].
    constexpr int MAX_ALLOWED_TAB_SIZE_GUESS = 8; // max(ALLOWED_TAB_SIZE_GUESSES) = 8

    int spacesDiffCount[MAX_ALLOWED_TAB_SIZE_GUESS + 1] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // `tabSize` scores
    SpacesDiffResult tmp;

    for (int lineNumber = 0; lineNumber < linesCount; lineNumber++) {
        const QString currentLineText = m_doc->line(lineNumber);
        const int currentLineLength = currentLineText.length();

        bool currentLineHasContent = false; // does `currentLineText` contain non-whitespace chars
        int currentLineIndentation = 0; // index at which `currentLineText` contains the first non-whitespace char
        int currentLineSpacesCount = 0; // count of spaces found in `currentLineText` indentation
        int currentLineTabsCount = 0; // count of tabs found in `currentLineText` indentation
        for (int j = 0, lenJ = currentLineLength; j < lenJ; j++) {
            const auto charCode = currentLineText.at(j);

            if (charCode == QLatin1Char('\t')) {
                currentLineTabsCount++;
            } else if (charCode == QLatin1Char(' ')) {
                currentLineSpacesCount++;
            } else {
                // Hit non whitespace character on this line
                currentLineHasContent = true;
                currentLineIndentation = j;
                break;
            }
        }

        // Ignore empty or only whitespace lines
        if (!currentLineHasContent) {
            continue;
        }

        if (currentLineTabsCount > 0) {
            linesIndentedWithTabsCount++;
        } else if (currentLineSpacesCount > 1) {
            linesIndentedWithSpacesCount++;
        }

        tmp = spacesDiff(previousLineText, previousLineIndentation, currentLineText, currentLineIndentation);

        if (tmp.looksLikeAlignment) {
            // if defaultInsertSpaces === true && the spaces count == tabSize, we may want to count it as valid indentation
            //
            // - item1
            //   - item2
            //
            // otherwise skip this line entirely
            //
            // const a = 1,
            //       b = 2;

            if (!(defaultInsertSpaces && defaultTabSize == tmp.spacesDiff)) {
                continue;
            }
        }

        const int currentSpacesDiff = tmp.spacesDiff;
        if (currentSpacesDiff <= MAX_ALLOWED_TAB_SIZE_GUESS) {
            spacesDiffCount[currentSpacesDiff]++;
        }

        previousLineText = currentLineText;
        previousLineIndentation = currentLineIndentation;
    }

    bool insertSpaces = defaultInsertSpaces;
    if (linesIndentedWithTabsCount != linesIndentedWithSpacesCount) {
        insertSpaces = (linesIndentedWithTabsCount < linesIndentedWithSpacesCount);
    }

    int tabSize = defaultTabSize;

    // Guess tabSize only if inserting spaces...
    if (insertSpaces) {
        int tabSizeScore = 0;
        for (const int possibleTabSize : ALLOWED_TAB_SIZE_GUESSES) {
            // prefer multiples of two, if ever found one, see bug 474505 and autotests/input/indent_detect/bogus7spaces.md
            const int possibleTabSizeScore = spacesDiffCount[possibleTabSize];
            if (possibleTabSizeScore > tabSizeScore && (possibleTabSize % 2 == 0 || tabSizeScore == 0)) {
                tabSizeScore = possibleTabSizeScore;
                tabSize = possibleTabSize;
            }
        }

        // Let a tabSize of 2 win even if it is not the maximum
        // (only in case 4 was guessed)
        if (tabSize == 4 && spacesDiffCount[4] > 0 && spacesDiffCount[2] > 0 && spacesDiffCount[2] >= spacesDiffCount[4] / 2) {
            tabSize = 2;
        }

        // If no indent detected, check if the file is 1 space indented
        if (tabSizeScore == 0) {
            const auto it = std::max_element(spacesDiffCount, spacesDiffCount + 9);
            const auto maxIdx = std::distance(spacesDiffCount, it);
            if (maxIdx == 1) {
                tabSize = 1;
            }
        }
    }

    return {.indentWidth=tabSize, .indentUsingSpaces=insertSpaces};
}
