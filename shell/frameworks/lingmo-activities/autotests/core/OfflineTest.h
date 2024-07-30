/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OFFLINETEST_H
#define OFFLINETEST_H

#include <common/test.h>

#include <controller.h>

#include <QScopedPointer>

class OfflineTest : public Test
{
    Q_OBJECT
public:
    explicit OfflineTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testOfflineActivityListing();
    void testOfflineActivityControl();

    void cleanupTestCase();

private:
    QScopedPointer<KActivities::Controller> activities;
};

#endif /* OFFLINETEST_H */
