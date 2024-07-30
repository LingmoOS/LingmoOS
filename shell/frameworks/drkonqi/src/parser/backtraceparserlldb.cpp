/*
    SPDX-FileCopyrightText: 2014 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backtraceparserlldb.h"
#include "backtraceparser_p.h"

// BEGIN BacktraceParserLldb

class BacktraceLineLldb : public BacktraceLine
{
public:
    BacktraceLineLldb(const QString &line);
};

BacktraceLineLldb::BacktraceLineLldb(const QString &line)
    : BacktraceLine()
{
    d->m_line = line;
    // For now we'll have faith that lldb provides useful information, and that it would
    // be unwarranted to give it a rating of "MissingEverything".
    d->m_rating = Good;
}

// END BacktraceLineLldb

// BEGIN BacktraceParserLldb

BacktraceParserLldb::BacktraceParserLldb(QObject *parent)
    : BacktraceParser(parent)
{
}

BacktraceParserPrivate *BacktraceParserLldb::constructPrivate() const
{
    BacktraceParserPrivate *d = BacktraceParser::constructPrivate();
    d->m_usefulness = MayBeUseful;
    return d;
}

void BacktraceParserLldb::newLine(const QString &lineStr)
{
    d_ptr->m_linesList.append(BacktraceLineLldb(lineStr));
}

// END BacktraceParserLldb

#include "moc_backtraceparserlldb.cpp"
