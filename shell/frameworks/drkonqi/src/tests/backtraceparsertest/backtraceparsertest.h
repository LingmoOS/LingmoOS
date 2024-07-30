/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BACKTRACEPARSERTEST_H
#define BACKTRACEPARSERTEST_H

#include "../../parser/backtraceparser.h"
#include "fakebacktracegenerator.h"
#include <QSettings>
#include <QTest>

class BacktraceParserTest : public QObject
{
    Q_OBJECT
public:
    BacktraceParserTest(QObject *parent = nullptr);

private Q_SLOTS:
    void btParserUsefulnessTest_data();
    void btParserUsefulnessTest();
    void btParserBenchmark_data();
    void btParserBenchmark();
    void btParserCompositorCrashTest_data();
    void btParserCompositorCrashTest();

private:
    void fetchData(const QString &group);

    QSettings m_settings;
    FakeBacktraceGenerator *m_generator = nullptr;
};

#endif
