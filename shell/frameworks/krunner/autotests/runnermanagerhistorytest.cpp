/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "runnermanager.h"

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QObject>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>
#include <memory>
namespace KRunner
{
extern int __changeCountBeforeSaving;
}
using namespace KRunner;
class RunnerManagerHistoryTest : public QObject
{
    Q_OBJECT
public:
    RunnerManagerHistoryTest()
    {
        __changeCountBeforeSaving = 1;
        QStandardPaths::setTestModeEnabled(true);
        stateConfigFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() + "krunnerstaterc";
    }

private:
    QString stateConfigFile;
    void addToHistory(const QStringList &queries, RunnerManager &manager)
    {
        QCOMPARE(manager.runners().count(), 1);
        for (const QString &query : queries) {
            QueryMatch match(manager.runners().constFirst());
            // Make sure internally the term and untrimmedTerm are set
            manager.launchQuery(query, "thisrunnerdoesnotexist");
            manager.searchContext()->setQuery(query);
            manager.run(match);
        }
    }
    void launchQuery(const QString &query, RunnerManager *manager)
    {
        QSignalSpy spy(manager, &KRunner::RunnerManager::queryFinished);
        manager->launchQuery(query);
        QVERIFY2(spy.wait(), "RunnerManager did not emit the queryFinished signal");
    }

private Q_SLOTS:
    void init()
    {
        if (QFileInfo::exists(stateConfigFile)) {
            QFile::remove(stateConfigFile);
        }
    }
    void testRunnerHistory();
    void testRunnerHistory_data();
    void testHistorySuggestionsAndRemoving();
    void testRelevanceForOftenLaunched();
};

void RunnerManagerHistoryTest::testRunnerHistory()
{
    QFETCH(const QStringList, queries);
    QFETCH(const QStringList, expectedEntries);

    RunnerManager manager;
    manager.setAllowedRunners({QStringLiteral("fakerunnerplugin")});
    manager.loadRunner(KPluginMetaData::findPluginById(QStringLiteral("krunnertest"), QStringLiteral("fakerunnerplugin")));
    addToHistory(queries, manager);
    QCOMPARE(manager.history(), expectedEntries);
}

void RunnerManagerHistoryTest::testRunnerHistory_data()
{
    QTest::addColumn<QStringList>("queries");
    QTest::addColumn<QStringList>("expectedEntries");

    QTest::newRow("should add simple entry to history") << QStringList{"test"} << QStringList{"test"};
    QTest::newRow("should not add entry that starts with space") << QStringList{" test"} << QStringList{};
    QTest::newRow("should not add duplicate entries") << QStringList{"test", "test"} << QStringList{"test"};
    QTest::newRow("should not add duplicate entries but put last run at beginning") << QStringList{"test", "test2", "test"} << QStringList{"test", "test2"};
}

void RunnerManagerHistoryTest::testHistorySuggestionsAndRemoving()
{
    RunnerManager manager;
    manager.setAllowedRunners({QStringLiteral("fakerunnerplugin")});
    manager.loadRunner(KPluginMetaData::findPluginById(QStringLiteral("krunnertest"), QStringLiteral("fakerunnerplugin")));
    const QStringList queries = {"test1", "test2", "test3"};
    addToHistory(queries, manager);
    QStringList expectedBeforeRemoval = QStringList{"test3", "test2", "test1"};
    QCOMPARE(manager.history(), expectedBeforeRemoval);
    QCOMPARE(manager.getHistorySuggestion("t"), "test3");
    QCOMPARE(manager.getHistorySuggestion("doesnotexist"), QString());

    manager.removeFromHistory(42);
    QCOMPARE(manager.history(), expectedBeforeRemoval);
    manager.removeFromHistory(0);
    QStringList expectedAfterRemoval = QStringList{"test2", "test1"};
    QCOMPARE(manager.history(), expectedAfterRemoval);
    QCOMPARE(manager.getHistorySuggestion("t"), "test2");
}

void RunnerManagerHistoryTest::testRelevanceForOftenLaunched()
{
    {
        KConfig cfg(stateConfigFile);
        cfg.group("PlasmaRunnerManager").writeEntry("LaunchCounts", "5 foo");
        cfg.sync();
    }
    std::unique_ptr<RunnerManager> manager(new RunnerManager());
    manager->setAllowedRunners({QStringLiteral("fakerunnerplugin")});
    manager->loadRunner(KPluginMetaData::findPluginById(QStringLiteral("krunnertest"), QStringLiteral("fakerunnerplugin")));

    launchQuery(QStringLiteral("foo"), manager.get());

    const auto matches = manager->matches();
    QCOMPARE(matches.size(), 2);
    QCOMPARE(matches.at(0).id(), QStringLiteral("foo"));
    QCOMPARE(matches.at(1).id(), QStringLiteral("bar"));
    QCOMPARE(matches.at(1).relevance(), 0.2);

    QVERIFY(matches.at(0).relevance() > matches.at(1).relevance());
    QVERIFY(matches.at(0).relevance() < 0.6); // 0.5 is the max we add as a bonus, 0.1 comes from the runner
    {
        KConfig cfg(stateConfigFile);
        cfg.group("PlasmaRunnerManager").writeEntry("LaunchCounts", QStringList{"5 foo", "5 bar"});
        cfg.sync();
        KSharedConfig::openConfig(QStringLiteral("krunnerstaterc"), KConfig::NoGlobals, QStandardPaths::GenericDataLocation)->reparseConfiguration();
    }
    manager = std::make_unique<RunnerManager>();
    manager->setAllowedRunners({QStringLiteral("fakerunnerplugin")});
    manager->loadRunner(KPluginMetaData::findPluginById(QStringLiteral("krunnertest"), QStringLiteral("fakerunnerplugin")));

    launchQuery(QStringLiteral("foo"), manager.get());
    const auto newMatches = manager->matches();
    QCOMPARE(newMatches.size(), 2);
    QVERIFY(newMatches.at(0).relevance() < newMatches.at(1).relevance());
}

QTEST_MAIN(RunnerManagerHistoryTest)

#include "runnermanagerhistorytest.moc"
