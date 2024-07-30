/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RESULTWATCHERTEST_H
#define RESULTWATCHERTEST_H

#include <common/test.h>

#include <PlasmaActivities/Controller>
#include <memory>

class ResultWatcherTest : public Test
{
    Q_OBJECT
public:
    ResultWatcherTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testLinkedResources();

    void cleanupTestCase();
};

#endif /* RESULTWATCHERTEST_H */
