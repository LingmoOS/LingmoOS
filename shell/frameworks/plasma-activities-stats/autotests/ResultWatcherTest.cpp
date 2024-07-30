/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ResultWatcherTest.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QString>
#include <QTemporaryDir>
#include <QTest>
#include <QTime>

#include <PlasmaActivities/ResourceInstance>

#include <query.h>
#include <resultset.h>
#include <resultwatcher.h>

#include <common/database/Database.h>
#include <common/database/schema/ResourcesDatabaseSchema.h>

namespace KAStats = KActivities::Stats;

ResultWatcherTest::ResultWatcherTest(QObject *parent)
    : Test(parent)
{
}

namespace
{
inline void liveSleep(int seconds)
{
    qDebug() << "Sleeping for " << seconds << " seconds";
    auto start = QTime::currentTime();
    while (start.secsTo(QTime::currentTime()) < seconds) {
        QCoreApplication::processEvents();
    }
}

#define CHECK_SIGNAL_RESULT(OBJ, SIGN, SECS, TESTARGS, TESTBODY)                                                                                               \
    {                                                                                                                                                          \
        QObject context;                                                                                                                                       \
        bool executed = false;                                                                                                                                 \
                                                                                                                                                               \
        QObject::connect(OBJ, SIGN, &context, [&] TESTARGS {                                                                                                   \
            TESTBODY;                                                                                                                                          \
            executed = true;                                                                                                                                   \
            qDebug() << "Signal processed";                                                                                                                    \
        });                                                                                                                                                    \
                                                                                                                                                               \
        qDebug() << "Waiting  for the signal at most " << SECS << " seconds";                                                                                  \
        auto start = QTime::currentTime();                                                                                                                     \
        while (start.secsTo(QTime::currentTime()) < SECS && !executed) {                                                                                       \
            QCoreApplication::processEvents();                                                                                                                 \
        }                                                                                                                                                      \
        QCOMPARE(executed, true);                                                                                                                              \
    }
}

void ResultWatcherTest::testLinkedResources()
{
    using namespace KAStats;
    using namespace KAStats::Terms;

    KAStats::ResultWatcher watcher(LinkedResources | Agent::global() | Activity::any());

    watcher.linkToActivity(QUrl(QStringLiteral("test://link1")), Activity::current());

    // A signal should arrive soon, waiting for 5 seconds at most
    CHECK_SIGNAL_RESULT(&watcher, &KAStats::ResultWatcher::resultLinked, 5, (const QString &uri), QCOMPARE(QStringLiteral("test://link1"), uri));

    watcher.unlinkFromActivity(QUrl(QStringLiteral("test://link1")), Activity::current());

    // A signal should arrive soon, waiting for 5 seconds at most
    CHECK_SIGNAL_RESULT(&watcher, &KAStats::ResultWatcher::resultUnlinked, 5, (const QString &uri), QCOMPARE(QStringLiteral("test://link1"), uri));
}

void ResultWatcherTest::initTestCase()
{
}

void ResultWatcherTest::cleanupTestCase()
{
    Q_EMIT testFinished();
}

#include "moc_ResultWatcherTest.cpp"
