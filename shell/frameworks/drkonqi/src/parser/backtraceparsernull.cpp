/*
    SPDX-FileCopyrightText: 2009-2010 George Kiagiadakis <kiagiadakis.george@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backtraceparsernull.h"
#include "backtraceparser_p.h"

// BEGIN BacktraceLineNull

class BacktraceLineNull : public BacktraceLine
{
public:
    BacktraceLineNull(const QString &line);
};

BacktraceLineNull::BacktraceLineNull(const QString &line)
    : BacktraceLine()
{
    d->m_line = line;
    d->m_rating = MissingEverything;
}

// END BacktraceLineNull

// BEGIN BacktraceParserNull

BacktraceParserNull::BacktraceParserNull(QObject *parent)
    : BacktraceParser(parent)
{
}

BacktraceParserPrivate *BacktraceParserNull::constructPrivate() const
{
    BacktraceParserPrivate *d = BacktraceParser::constructPrivate();
    d->m_usefulness = MayBeUseful;
    return d;
}

void BacktraceParserNull::newLine(const QString &lineStr)
{
    d_ptr->m_linesList.append(BacktraceLineNull(lineStr));
}

// END BacktraceParserNull

#include "moc_backtraceparsernull.cpp"
