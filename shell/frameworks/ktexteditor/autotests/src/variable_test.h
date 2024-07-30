/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_VARIABLE_TEST_H
#define KTEXTEDITOR_VARIABLE_TEST_H

#include <QObject>

class VariableTest : public QObject
{
    Q_OBJECT

public:
    VariableTest();
    ~VariableTest() override;

private Q_SLOTS:
    void testReturnValues();
    void testExactMatch_data();
    void testExactMatch();
    void testPrefixMatch();
    void testRecursiveMatch();
    void testBuiltins();
};

#endif // KTEXTEDITOR_VARIABLE_TEST_H
