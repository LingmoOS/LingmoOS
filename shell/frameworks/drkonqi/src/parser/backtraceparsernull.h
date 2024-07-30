/*
    SPDX-FileCopyrightText: 2009-2010 George Kiagiadakis <kiagiadakis.george@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BACKTRACEPARSERNULL_H
#define BACKTRACEPARSERNULL_H

#include "backtraceparser.h"

class BacktraceParserNull : public BacktraceParser
{
    Q_OBJECT
public:
    explicit BacktraceParserNull(QObject *parent = nullptr);

protected Q_SLOTS:
    void newLine(const QString &lineStr) override;

protected:
    BacktraceParserPrivate *constructPrivate() const override;
};

#endif // BACKTRACEPARSERNULL_H
