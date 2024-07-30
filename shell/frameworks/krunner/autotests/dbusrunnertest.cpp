/*
    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KRunner/Action>
#include <KRunner/RunnerManager>
#include <QObject>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>
#include <QTime>
#include <QTimer>

#include "abstractrunnertest.h"
#include "kpluginmetadata_utils_p.h"

using namespace KRunner;

Q_DECLARE_METATYPE(KRunner::QueryMatch)
Q_DECLARE_METATYPE(QList<KRunner::QueryMatch>)

class DBusRunnerTest : public AbstractRunnerTest
{
    Q_OBJECT
public:
    DBusRunnerTest();
    ~DBusRunnerTest() override;

private Q_SLOTS:
    void cleanup();
    void testMatch();
    void testMulti();
    void testFilterProperties();
    void testFilterProperties_data();
    void testRequestActionsOnce();
    void testDBusRunnerSyntaxIntegration();
    void testIconData();
    void testLifecycleMethods();
    void testRequestActionsWildcards();
};

DBusRunnerTest::DBusRunnerTest()
    : AbstractRunnerTest()
{
    qRegisterMetaType<QList<KRunner::QueryMatch>>();
    QStandardPaths::setTestModeEnabled(true);
}

DBusRunnerTest::~DBusRunnerTest()
{
}

void DBusRunnerTest::cleanup()
{
    // Make sure kill the running processes after each test
    killRunningDBusProcesses();
}

void DBusRunnerTest::testMatch()
{
    QProcess *process = startDBusRunnerProcess({QStringLiteral("net.krunnertests.dave")});
    initProperties();
    const auto matches = launchQuery(QStringLiteral("foo"));

    // verify matches
    QCOMPARE(matches.count(), 1);
    auto result = matches.first();

    // see testremoterunner.cpp
    QCOMPARE(result.id(), QStringLiteral("dbusrunnertest_id1")); // note the runner name is prepended
    QCOMPARE(result.text(), QStringLiteral("Match 1"));
    QCOMPARE(result.iconName(), QStringLiteral("icon1"));
    QCOMPARE(result.categoryRelevance(), qToUnderlying(KRunner::QueryMatch::CategoryRelevance::Highest));
    QCOMPARE(result.isMultiLine(), true);
    // relevance can't be compared easily because RunnerContext meddles with it

    // verify actions
    QCOMPARE(result.actions().size(), 1);
    auto action = result.actions().constFirst();

    QCOMPARE(action.text(), QStringLiteral("Action 1"));

    QSignalSpy processSpy(process, &QProcess::readyRead);
    manager->run(result);
    processSpy.wait();
    QCOMPARE(process->readAllStandardOutput().trimmed().split('\n').constLast(), QByteArray("Running:id1:"));

    manager->run(result, action);
    processSpy.wait();
    QCOMPARE(process->readAllStandardOutput().trimmed().split('\n').constLast(), QByteArray("Running:id1:action1"));
}

void DBusRunnerTest::testMulti()
{
    startDBusRunnerProcess({QStringLiteral("net.krunnertests.multi.a1")}, QStringLiteral("net.krunnertests.multi.a1"));
    startDBusRunnerProcess({QStringLiteral("net.krunnertests.multi.a2")}, QStringLiteral("net.krunnertests.multi.a2"));
    manager.reset(new RunnerManager()); // This case is special, because we want to load the runners manually

    auto md = parseMetaDataFromDesktopFile(QFINDTESTDATA("plugins/dbusrunnertestmulti.desktop"));
    QVERIFY(md.isValid());
    manager->loadRunner(md);
    const auto matches = launchQuery(QStringLiteral("foo"));

    // verify matches, must be one from each
    QCOMPARE(matches.count(), 2);

    const QString first = matches.at(0).data().toList().constFirst().toString();
    const QString second = matches.at(1).data().toList().constFirst().toString();
    QVERIFY(first != second);
    QVERIFY(first == QLatin1String("net.krunnertests.multi.a1") || first == QStringLiteral("net.krunnertests.multi.a2"));
    QVERIFY(second == QLatin1String("net.krunnertests.multi.a1") || second == QStringLiteral("net.krunnertests.multi.a2"));
}

void DBusRunnerTest::testRequestActionsOnce()
{
    QProcess *process = startDBusRunnerProcess({QStringLiteral("net.krunnertests.dave")});
    initProperties();

    launchQuery(QStringLiteral("foo"));
    QVERIFY(!manager->matches().constFirst().actions().isEmpty());
    manager->matchSessionComplete();
    launchQuery(QStringLiteral("fooo"));
    const QString processOutput(process->readAllStandardOutput());
    QCOMPARE(processOutput.count("Matching"), 2);
    QCOMPARE(processOutput.count("Actions"), 1);
    QVERIFY(!manager->matches().constFirst().actions().isEmpty());
}

void DBusRunnerTest::testFilterProperties_data()
{
    QTest::addColumn<QString>("rejectedQuery");
    QTest::addColumn<QString>("acceptedQuery");

    QTest::newRow("min-letter-count") << "fo"
                                      << "foo";
    QTest::newRow("match-regex") << "barfoo"
                                 << "foobar";
}

void DBusRunnerTest::testFilterProperties()
{
    QFETCH(QString, rejectedQuery);
    QFETCH(QString, acceptedQuery);
    QProcess *process = startDBusRunnerProcess({QStringLiteral("net.krunnertests.dave")});
    initProperties();

    launchQuery(rejectedQuery);
    // Match method was not called, because of the min letter count or match regex property
    QVERIFY(process->readAllStandardOutput().isEmpty());
    // accepted query fits those constraints
    launchQuery(acceptedQuery);
    QCOMPARE(QString(process->readAllStandardOutput()).remove("Actions").trimmed(), QStringLiteral("Matching:") + acceptedQuery);
}

void DBusRunnerTest::testDBusRunnerSyntaxIntegration()
{
    startDBusRunnerProcess({QStringLiteral("net.krunnertests.dave")});
    initProperties();
    const QList<RunnerSyntax> syntaxes = runner->syntaxes();
    QCOMPARE(syntaxes.size(), 2);

    QCOMPARE(syntaxes.at(0).exampleQueries().size(), 1);
    QCOMPARE(syntaxes.at(0).exampleQueries().constFirst(), QStringLiteral("syntax1"));
    QCOMPARE(syntaxes.at(0).description(), QStringLiteral("description1"));
    QCOMPARE(syntaxes.at(1).exampleQueries().size(), 1);
    QCOMPARE(syntaxes.at(1).exampleQueries().constFirst(), QStringLiteral("syntax2"));
    QCOMPARE(syntaxes.at(1).description(), QStringLiteral("description2"));
}

void DBusRunnerTest::testIconData()
{
    startDBusRunnerProcess({QStringLiteral("net.krunnertests.dave")});
    initProperties();

    const auto matches = launchQuery(QStringLiteral("fooCostomIcon"));
    QCOMPARE(matches.count(), 1);
    auto result = matches.first();

    QImage expectedIcon(10, 10, QImage::Format_RGBA8888);
    expectedIcon.fill(Qt::blue);

    QCOMPARE(result.icon().availableSizes().first(), QSize(10, 10));
    QCOMPARE(result.icon().pixmap(QSize(10, 10)), QPixmap::fromImage(expectedIcon));
}

void DBusRunnerTest::testLifecycleMethods()
{
    QProcess *process = startDBusRunnerProcess({QStringLiteral("net.krunnertests.dave"), QString()});
    manager.reset(new RunnerManager()); // This case is special, because we want to load the runners manually
    auto md = parseMetaDataFromDesktopFile(QFINDTESTDATA("plugins/dbusrunnertestruntimeconfig.desktop"));
    manager->loadRunner(md);
    QCOMPARE(manager->runners().count(), 1);
    // Match session should be set up automatically
    launchQuery(QStringLiteral("fooo"));

    // Make sure we got our match, end the match session and give the process a bit of time to get the DBus signal
    QTRY_COMPARE_WITH_TIMEOUT(manager->matches().count(), 1, 2000);
    manager->matchSessionComplete();
    QTest::qWait(500);

    const QStringList lifeCycleSteps = QString::fromLocal8Bit(process->readAllStandardOutput()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    const QStringList expectedLifeCycleSteps = {
        QStringLiteral("Config"),
        QStringLiteral("Actions"),
        QStringLiteral("Matching:fooo"),
        QStringLiteral("Teardown"),
    };
    QCOMPARE(lifeCycleSteps, expectedLifeCycleSteps);

    // The query does not match our min letter count we set at runtime
    launchQuery(QStringLiteral("foo"));
    QVERIFY(manager->matches().isEmpty());
    // The query does not match our match regex we set at runtime
    launchQuery(QStringLiteral("barfoo"));
    QVERIFY(manager->matches().isEmpty());
}

void DBusRunnerTest::testRequestActionsWildcards()
{
    initProperties();
    manager.reset(new RunnerManager()); // This case is special, because we want to load the runners manually
    auto md = parseMetaDataFromDesktopFile(QFINDTESTDATA("plugins/dbusrunnertestmulti.desktop"));
    QVERIFY(md.isValid());
    manager->loadRunner(md);
    QCOMPARE(manager->runners().count(), 1);

    startDBusRunnerProcess({QStringLiteral("net.krunnertests.multi.a1")}, QStringLiteral("net.krunnertests.multi.a1"));
    startDBusRunnerProcess({QStringLiteral("net.krunnertests.multi.a2")}, QStringLiteral("net.krunnertests.multi.a2"));
    const auto matches = launchQuery("foo");
    QCOMPARE(matches.count(), 2);

    QCOMPARE(matches.at(0).actions().count(), 1);
    QCOMPARE(matches.at(0).actions(), matches.at(1).actions());
}

QTEST_MAIN(DBusRunnerTest)

#include "dbusrunnertest.moc"
