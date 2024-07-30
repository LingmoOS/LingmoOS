/*
    SPDX-FileCopyrightText: 2014-2016 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BACKTRACEPARSERLLDB_H
#define BACKTRACEPARSERLLDB_H

#include "backtraceparser.h"

class BacktraceParserLldb : public BacktraceParser
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BacktraceParser)
public:
    explicit BacktraceParserLldb(QObject *parent = nullptr);

protected Q_SLOTS:
    virtual void newLine(const QString &lineStr) override;

protected:
    virtual BacktraceParserPrivate *constructPrivate() const override;
};

#endif // BACKTRACEPARSERLLDB_H
