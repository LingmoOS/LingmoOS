/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "OfflineTest.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>
#include <QTest>

QString nulluuid = QStringLiteral("00000000-0000-0000-0000-000000000000");

OfflineTest::OfflineTest(QObject *parent)
    : Test(parent)
{
    QCoreApplication::instance()->setProperty("org.kde.KActivities.core.disableAutostart", true);

    activities.reset(new KActivities::Controller());
}

void OfflineTest::testOfflineActivityListing()
{
    // The service is not running

    TEST_WAIT_UNTIL(activities->serviceStatus() == KActivities::Consumer::NotRunning);
    QCOMPARE(activities->currentActivity(), nulluuid);

    QCOMPARE(activities->activities(), QStringList() << nulluuid);
    QCOMPARE(activities->activities(KActivities::Info::Running), QStringList() << nulluuid);
    QCOMPARE(activities->activities(KActivities::Info::Stopped), QStringList());
}

void OfflineTest::testOfflineActivityControl()
{
    continue_future(activities->addActivity(QStringLiteral("Activity")), [](const QString &newid) {
        QCOMPARE(newid, QString());
    });

    continue_future(activities->setCurrentActivity(QStringLiteral("Activity")), [](bool success) {
        QCOMPARE(success, false);
    });

    // Test whether the responses are immediate
    static bool inMethod = false;

    inMethod = true;

    continue_future(activities->setActivityName(QStringLiteral("Activity"), QStringLiteral("Activity")), []() {
        QCOMPARE(inMethod, true);
    });
    continue_future(activities->setActivityIcon(QStringLiteral("Activity"), QStringLiteral("Activity")), []() {
        QCOMPARE(inMethod, true);
    });
    continue_future(activities->removeActivity(QStringLiteral("Activity")), []() {
        QCOMPARE(inMethod, true);
    });
    continue_future(activities->startActivity(QStringLiteral("Activity")), []() {
        QCOMPARE(inMethod, true);
    });
    continue_future(activities->stopActivity(QStringLiteral("Activity")), []() {
        QCOMPARE(inMethod, true);
    });

    inMethod = false;
}

void OfflineTest::initTestCase()
{
    CHECK_CONDITION(inEmptySession, FailIfFalse);
    CHECK_CONDITION(isActivityManagerRunning, FailIfTrue);
}

void OfflineTest::cleanupTestCase()
{
    Q_EMIT testFinished();
}

#include "moc_OfflineTest.cpp"
