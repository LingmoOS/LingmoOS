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
#ifndef TASK_H
#define	TASK_H

#include <optional>
#include <vector>

#include "Emulation.h"
#include "RegExp.h"
#include "TerminalCharacterDecoder.h"

using namespace Konsole;

class HistorySearch
{
public:
    struct Result
    {
        int startColumn = 0;
        int startLine = 0;
        int endColumn = 0;
        int endLine = 0;
    };

    explicit HistorySearch(Emulation* emulation, const RegExp& regExp, bool forwards,
                           int startColumn, int startLine);
    ~HistorySearch();

    // The match, or nothing when there is none or the pattern is empty.
    std::optional<Result> search();

private:
    bool search(int startColumn, int startLine, int endColumn, int endLine);
    int findLineNumberInString(const std::vector<int>& linePositions, int position);


    Emulation* m_emulation;
    RegExp m_regExp;
    bool m_forwards = false;
    int m_startColumn = 0;
    int m_startLine = 0;

    int m_foundStartColumn = 0;
    int m_foundStartLine = 0;
    int m_foundEndColumn = 0;
    int m_foundEndLine = 0;
};

#endif	/* TASK_H */
