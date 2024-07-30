/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "runnermanager.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <QCoreApplication>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>
#include <memory>

#include "abstractrunnertest.h"
#include "kpluginmetadata_utils_p.h"

using namespace KRunner;

class RunnerManagerSingleRunnerModeTest : public AbstractRunnerTest
{
    Q_OBJECT
private Q_SLOTS:
    void loadTwoRunners()
    {
        startDBusRunnerProcess({QStringLiteral("net.krunnertests.dave")});
        startDBusRunnerProcess({QStringLiteral("net.krunnertests.multi.a1")}, QStringLiteral("net.krunnertests.multi.a1"));
        qputenv("XDG_DATA_DIRS", QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).toLocal8Bit());
        QCoreApplication::setLibraryPaths(QStringList());
        initProperties();
        auto md = parseMetaDataFromDesktopFile(QFINDTESTDATA("plugins/dbusrunnertestmulti.desktop"));
        QVERIFY(md.isValid());
        manager->loadRunner(md);
    }

    void cleanup()
    {
        killRunningDBusProcesses();
        const QString configFile = QStandardPaths::locate(QStandardPaths::ConfigLocation, "runnermanagersinglerunnermodetestrcm");
        if (!configFile.isEmpty()) {
            QFile::remove(configFile);
        }
    }

    void testAllRunnerResults();
    void testSingleRunnerResults();
    void testNonExistentRunnerId();
    void testLoadingDisabledRunner();
};

void RunnerManagerSingleRunnerModeTest::testAllRunnerResults()
{
    loadTwoRunners();
    launchQuery("foo");
    const auto matches = manager->matches();
    QCOMPARE(matches.count(), 2);
    const QStringList ids = {matches.at(0).runner()->id(), matches.at(1).runner()->id()};
    // Both runners should have produced one result
    QVERIFY(ids.contains("dbusrunnertest"));
    QVERIFY(ids.contains("dbusrunnertestmulti"));
}

void RunnerManagerSingleRunnerModeTest::testSingleRunnerResults()
{
    loadTwoRunners();
    launchQuery("foo", "dbusrunnertest");
    QCOMPARE(manager->matches().count(), 1);
    QCOMPARE(manager->matches().constFirst().runner()->id(), "dbusrunnertest");
    launchQuery("foo", "dbusrunnertestmulti");
    QCOMPARE(manager->matches().count(), 1);
    QCOMPARE(manager->matches().constFirst().runner()->id(), "dbusrunnertestmulti");
}

void RunnerManagerSingleRunnerModeTest::testNonExistentRunnerId()
{
    loadTwoRunners();
    manager->launchQuery("foo", "bla"); // This internally calls reset, we just wait a bit and make sure there are no matches
    QTest::qSleep(250);
    QVERIFY(manager->matches().isEmpty());
}

void RunnerManagerSingleRunnerModeTest::testLoadingDisabledRunner()
{
    loadTwoRunners();
    // Make sure the runner is disabled in the config
    auto config = KSharedConfig::openConfig();
    config->deleteGroup("Plugins");
    config->group("Plugins").writeEntry("dbusrunnertestEnabled", false);
    // reset our manager to start clean
    manager = std::make_unique<RunnerManager>(config->group("Plugins"), config->group("State"), this);
    // Ensure no system runners are picked up
    manager->setAllowedRunners(QStringList{"dbusrunnertest", "dbusrunnertestmulti"});
    // Copy the service files to the appropriate location and only load runners from there
    QString location = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/krunner/dbusplugins/";
    QDir().mkpath(location);
    QFile::copy(QFINDTESTDATA("plugins/dbusrunnertest.desktop"), location + "dbusrunnertest.desktop");
    QFile::copy(QFINDTESTDATA("plugins/dbusrunnertestmulti.desktop"), location + "dbusrunnertestmulti.desktop");

    // Only enabled runner should be loaded and have results
    auto matches = launchQuery("foo");
    QCOMPARE(manager->runners().count(), 1);
    QCOMPARE(manager->runners().constFirst()->id(), "dbusrunnertestmulti");
    QCOMPARE(matches.count(), 1);
    QCOMPARE(matches.constFirst().runner()->id(), "dbusrunnertestmulti");

    // Only enabled runner should be loaded and have results
    matches = launchQuery("foo", "dbusrunnertest");
    QCOMPARE(manager->runners().count(), 2);
    const QStringList ids{manager->runners().at(0)->id(), manager->runners().at(1)->id()};
    QVERIFY(ids.contains("dbusrunnertestmulti"));
    QVERIFY(ids.contains("dbusrunnertest"));
    QCOMPARE(matches.count(), 1);
    QCOMPARE(matches.constFirst().runner()->id(), "dbusrunnertest");

    // Only enabled runner should used for querying
    matches = launchQuery("foo");
    QCOMPARE(manager->runners().count(), 2);
    QCOMPARE(matches.count(), 1);
    QCOMPARE(matches.constFirst().runner()->id(), "dbusrunnertestmulti");
}

QTEST_MAIN(RunnerManagerSingleRunnerModeTest)

#include "runnermanagersinglerunnermodetest.moc"
