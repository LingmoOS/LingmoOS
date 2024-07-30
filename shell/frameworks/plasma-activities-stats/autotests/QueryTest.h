/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OFFLINETEST_H
#define OFFLINETEST_H

#include <common/test.h>

#include <PlasmaActivities/Controller>

class QueryTest : public Test
{
    Q_OBJECT
public:
    QueryTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testDefaults();
    void testDebuggingOutput();

    void testDerivationFromDefault();
    void testDerivationFromCustom();

    void testNormalSyntaxAgentManipulation();
    void testNormalSyntaxTypeManipulation();
    void testNormalSyntaxActivityManipulation();
    void testNormalSyntaxOrderingManipulation();
    void testNormalSyntaxDateDefinition();

    void testFancySyntaxBasic();
    void testFancySyntaxAgentDefinition();
    void testFancySyntaxTypeDefinition();
    void testFancySyntaxActivityDefinition();
    void testFancySyntaxOrderingDefinition();

    void cleanupTestCase();

private:
};

#endif /* OFFLINETEST_H */
