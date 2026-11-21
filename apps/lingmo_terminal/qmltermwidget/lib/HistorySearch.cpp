/*
    Copyright 2013 Christian Surlykke

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/
#include "TerminalCharacterDecoder.h"
#include "Emulation.h"
#include "HistorySearch.h"
#include "tools.h"

HistorySearch::HistorySearch(Emulation* emulation, const RegExp& regExp,
        bool forwards, int startColumn, int startLine) :
m_emulation(emulation),
m_regExp(regExp),
m_forwards(forwards),
m_startColumn(startColumn),
m_startLine(startLine) {
}

HistorySearch::~HistorySearch() {
}

std::optional<HistorySearch::Result> HistorySearch::search() {
    bool found = false;

    if (! m_regExp.pattern().empty())
    {
        if (m_forwards) {
            found = search(m_startColumn, m_startLine, -1, m_emulation->lineCount()) || search(0, 0, m_startColumn, m_startLine);
        } else {
            found = search(0, 0, m_startColumn, m_startLine) || search(m_startColumn, m_startLine, -1, m_emulation->lineCount());
        }

        if (found) {
            return Result{m_foundStartColumn, m_foundStartLine, m_foundEndColumn, m_foundEndLine};
        }
    }

    return std::nullopt;
}

bool HistorySearch::search(int startColumn, int startLine, int endColumn, int endLine) {
    int linesRead = 0;
    int linesToRead = endLine - startLine + 1;

    // We read process history from (and including) startLine to (and including) endLine in
    // blocks of at most 10K lines so that we do not use unhealthy amounts of memory
    int blockSize;
    while ((blockSize = kMin(10000, linesToRead - linesRead)) > 0) {

        std::u16string string;
        PlainTextDecoder decoder;
        decoder.begin(&string);
        decoder.setRecordLinePositions(true);

        // Calculate lines to read and read them
        int blockStartLine = m_forwards ? startLine + linesRead : endLine - linesRead - blockSize + 1;
        int chunkEndLine = blockStartLine + blockSize - 1;
        m_emulation->writeToStream(&decoder, blockStartLine, chunkEndLine);

        // We search between startColumn in the first line of the string and endColumn in the last
        // line of the string. First we calculate the position (in the string) of endColumn in the
        // last line of the string
        int endPosition;

        // The String that Emulator.writeToStream produces has a newline at the end, and so ends with an
        // empty line - we ignore that.
        const std::vector<int> linePositions = decoder.linePositions();
        int numberOfLinesInString = int(linePositions.size()) - 1;
        if (numberOfLinesInString > 0 && endColumn > -1 )
        {
            endPosition = linePositions.at(numberOfLinesInString - 1) + endColumn;
        }
        else
        {
            endPosition = int(string.size());
        }

        // So now we can log for m_regExp in the string between startColumn and endPosition
        int matchStart;
        RegExp::Match match;
        if (m_forwards)
        {
            match = m_regExp.match(string, startColumn);
            matchStart = match.hasMatch ? int(match.capturedStart()) : -1;
            if (matchStart >= endPosition)
                matchStart = -1;
        }
        else
        {
            matchStart = int(m_regExp.lastIndexIn(string, endPosition - 1, &match));
            if (matchStart < startColumn)
                matchStart = -1;
        }

        if (matchStart > -1)
        {
            int matchEnd = matchStart + int(match.capturedLength()) - 1;

            // Translate startPos and endPos to startColum, startLine, endColumn and endLine in history.
            int startLineNumberInString = findLineNumberInString(linePositions, matchStart);
            m_foundStartColumn = matchStart - linePositions.at(startLineNumberInString);
            m_foundStartLine = startLineNumberInString + startLine + linesRead;

            int endLineNumberInString = findLineNumberInString(linePositions, matchEnd);
            m_foundEndColumn = matchEnd - linePositions.at(endLineNumberInString);
            m_foundEndLine = endLineNumberInString + startLine + linesRead;

            return true;
        }


        linesRead += blockSize;
    }

    return false;
}


int HistorySearch::findLineNumberInString(const std::vector<int>& linePositions, int position) {
    int lineNum = 0;
    while (lineNum + 1 < int(linePositions.size()) && linePositions[lineNum + 1] <= position)
        lineNum++;

    return lineNum;
}
