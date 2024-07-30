/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RESULTSETTEST_H
#define RESULTSETTEST_H

#include <common/test.h>

#include <PlasmaActivities/Controller>
#include <memory>

class ResultSetTest : public Test
{
    Q_OBJECT
public:
    ResultSetTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testConcat(); ///< Tests test-implementation-details
    void testLinkedResources();
    void testUsedResources();

    void cleanupTestCase();
};

#endif /* RESULTSETTEST_H */
