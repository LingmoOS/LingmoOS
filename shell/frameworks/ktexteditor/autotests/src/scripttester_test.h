/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2024 Jonathan Poelen <jonathan.poelen@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SCRIPT_TESTER_TEST_H
#define KATE_SCRIPT_TESTER_TEST_H

#include <QObject>

#include <ktexteditor/range.h>

class ScriptTesterTest : public QObject
{
    Q_OBJECT

public:
    ScriptTesterTest();
    ~ScriptTesterTest() override;

private Q_SLOTS:
    void testDebug();
    void testPrintExpression();
};

#endif
