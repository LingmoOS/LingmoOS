/*
    SPDX-FileCopyrightText: 2009-2010 George Kiagiadakis <kiagiadakis.george@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BACKTRACEPARSERGDB_H
#define BACKTRACEPARSERGDB_H

#include "backtraceparser.h"
class BacktraceParserGdbPrivate;

class BacktraceLineGdb : public BacktraceLine
{
public:
    BacktraceLineGdb(const QString &line);

private:
    void parse();
    void rate();
};

class BacktraceParserGdb : public BacktraceParser
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BacktraceParserGdb)
public:
    explicit BacktraceParserGdb(QObject *parent = nullptr);

    QString parsedBacktrace() const override;
    QList<BacktraceLine> parsedBacktraceLines() const override;
    static const QLatin1String KCRASH_INFO_MESSAGE;

protected:
    BacktraceParserPrivate *constructPrivate() const override;

protected Q_SLOTS:
    void newLine(const QString &lineStr) override;

private:
    void parseLine(const QString &lineStr);
};

#endif // BACKTRACEPARSERGDB_H
