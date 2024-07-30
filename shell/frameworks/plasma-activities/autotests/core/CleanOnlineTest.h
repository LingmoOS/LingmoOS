/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLEANONLINETEST_H
#define CLEANONLINETEST_H

#include <common/test.h>

#include <controller.h>

#include <QScopedPointer>

class CleanOnlineTest : public Test
{
    Q_OBJECT
public:
    CleanOnlineTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testCleanOnlineActivityListing();

    void cleanupTestCase();

private:
    QScopedPointer<KActivities::Controller> activities;
    QString id1;
    QString id2;
};

class CleanOnlineSetup : public Test
{
    Q_OBJECT
public:
    explicit CleanOnlineSetup(QObject *parent = nullptr);

private Q_SLOTS:
    void testCleanOnlineActivityControl();

    void cleanupTestCase();

private:
    QScopedPointer<KActivities::Controller> activities;

public:
    static QString id1;
    static QString id2;
};

class OnlineTest : public Test
{
    Q_OBJECT
public:
    explicit OnlineTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testOnlineActivityListing();

    void cleanupTestCase();

private:
    QScopedPointer<KActivities::Controller> activities;
};

#endif /* CLEANONLINETEST_H */
