/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ResultSetQuickCheckTest.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QRandomGenerator>
#include <QString>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/numeric.hpp>

#include <query.h>
#include <resultset.h>

#include <iostream>

#include <common/database/Database.h>
#include <common/database/schema/ResourcesDatabaseSchema.h>

#include <utils/qsqlquery_iterator.h>

#define NUMBER_ACTIVITIES 10
#define NUMBER_AGENTS 10
#define NUMBER_RESOURCES 50
#define NUMBER_CACHES 200

namespace KAStats = KActivities::Stats;

static ResultSetQuickCheckTest *instance;

ResultSetQuickCheckTest::ResultSetQuickCheckTest(QObject *parent)
    : Test(parent)
    , activities(std::make_unique<KActivities::Consumer>())
{
    instance = this;
}

namespace
{
QString resourceTitle(const QString &resource)
{
    // We need to find the title
    ResourceInfo::Item key;
    key.targettedResource = resource;

    auto &infos = instance->resourceInfos;

    auto ri = infos.lower_bound(key);

    return (ri != infos.cend() && ri->targettedResource == resource) ? ri->title : resource;
}

QString toQString(const ResourceScoreCache::Item &item)
{
    return item.targettedResource + QLatin1Char(':') //
        + resourceTitle(item.targettedResource) + QLatin1Char('(') + QString::number(item.cachedScore) + QLatin1Char(')');
}

QString toQString(const ResourceLink::Item &item)
{
    return item.targettedResource + QLatin1Char(':') + resourceTitle(item.targettedResource);
    // + '(' + QString::number(0/* item.score */) + ')'
}

QString toQString(const KAStats::ResultSet::Result &item)
{
    return item.resource() + QLatin1Char(':') + item.title() + QLatin1Char('(') + QString::number(item.score()) + QLatin1Char(')');
}

bool operator==(const ResourceScoreCache::Item &left, const KAStats::ResultSet::Result &right)
{
    return left.targettedResource == right.resource() && resourceTitle(left.targettedResource) == right.title()
        && qFuzzyCompare(left.cachedScore, right.score());
}

bool operator==(const ResourceLink::Item &left, const KAStats::ResultSet::Result &right)
{
    return left.targettedResource == right.resource() //
        && resourceTitle(left.targettedResource) == right.title();
    // && qFuzzyCompare(left.cachedScore, right.score);
}

template<typename Left>
void assert_range_equal(const Left &left, const KAStats::ResultSet &right, const char *file, int line)
{
    auto leftIt = left.cbegin();
    auto rightIt = right.cbegin();
    auto leftEnd = left.cend();
    auto rightEnd = right.cend();

    bool equal = true;

    QString leftLine;
    QString rightLine;

    for (; leftIt != leftEnd && rightIt != rightEnd; ++leftIt, ++rightIt) {
        auto leftString = toQString(*leftIt);
        auto rightString = toQString(*rightIt);

        if (*leftIt == *rightIt) {
            rightString.fill(QLatin1Char('.'));

        } else {
            equal = false;
        }

        int longer = qMax(leftString.length(), rightString.length());
        leftString = leftString.leftJustified(longer);
        rightString = rightString.leftJustified(longer, QLatin1Char('.'));

        leftLine += QStringLiteral(" ") + leftString;
        rightLine += QStringLiteral(" ") + rightString;
    }

    // So far, we are equal, but do we have the same number
    // of elements - did we reach the end of both ranges?
    if (leftIt != leftEnd) {
        for (; leftIt != leftEnd; ++leftIt) {
            auto item = toQString(*leftIt);
            leftLine += QStringLiteral(" ") + item;
            item.fill(QLatin1Char('X'));
            rightLine += QStringLiteral(" ") + item;
        }
        equal = false;

    } else if (rightIt != rightEnd) {
        for (; rightIt != rightEnd; ++rightIt) {
            auto item = toQString(*leftIt);
            rightLine += QStringLiteral(" ") + item;
            item.fill(QLatin1Char('X'));
            leftLine += QStringLiteral(" ") + item;
        }
        equal = false;
    }

    if (!equal) {
        qDebug() << "Ranges differ:\n"
                 << "MEM: " << leftLine << '\n'
                 << "SQL: " << rightLine;
        QTest::qFail("Results do not match", file, line);
    }
}

#define ASSERT_RANGE_EQUAL(L, R) assert_range_equal(L, R, __FILE__, __LINE__)
}

//_ Data init
void ResultSetQuickCheckTest::initTestCase()
{
    QString databaseFile;

    int dbArgIndex = QCoreApplication::arguments().indexOf(QStringLiteral("--ResultSetQuickCheckDatabase"));
    if (dbArgIndex > 0) {
        databaseFile = QCoreApplication::arguments()[dbArgIndex + 1];

        qDebug() << "Using an existing database: " << databaseFile;
        Common::ResourcesDatabaseSchema::overridePath(databaseFile);

        pullFromDatabase();

    } else {
        QTemporaryDir dir(QDir::tempPath() + QStringLiteral("/KActivitiesStatsTest_ResultSetQuickCheckTest_XXXXXX"));
        dir.setAutoRemove(false);

        if (!dir.isValid()) {
            qFatal("Can not create a temporary directory");
        }

        databaseFile = dir.path() + QStringLiteral("/database");

        qDebug() << "Creating database in " << databaseFile;
        Common::ResourcesDatabaseSchema::overridePath(databaseFile);

        while (activities->serviceStatus() == KActivities::Consumer::Unknown) {
            QCoreApplication::processEvents();
        }

        generateActivitiesList();
        generateAgentsList();
        generateTypesList();
        generateResourcesList();

        generateResourceInfos();
        generateResourceScoreCaches();
        generateResourceLinks();

        pushToDatabase();
    }

    if (QCoreApplication::arguments().contains(QLatin1String("--show-data"))) {
        QString rscs;
        for (const auto &rsc : resourceScoreCaches) {
            rscs += QLatin1Char('(') + rsc.targettedResource //
                + QLatin1Char(',') + rsc.usedActivity //
                + QLatin1Char(',') + rsc.initiatingAgent //
                + QLatin1Char(',') + QString::number(rsc.cachedScore) + QLatin1Char(')');
        }

        QString ris;
        for (const auto &ri : resourceInfos) {
            ris += QLatin1Char('(') + ri.targettedResource + QLatin1Char(',') + ri.title + QLatin1Char(',') + ri.mimetype + QLatin1Char(')');
        }

        QString rls;
        for (const auto &rl : resourceLinks) {
            rls += QLatin1Char('(') + rl.targettedResource //
                + QLatin1Char(',') + rl.usedActivity //
                + QLatin1Char(',') + rl.initiatingAgent + QLatin1Char(')');
        }

        qDebug() << "\nUsed data: -----------------------------"
                 << "\nActivities: " << activitiesList << "\nAgents: " << agentsList << "\nTypes: " << typesList << "\nResources: " << resourcesList
                 << "\n----------------------------------------";
        qDebug() << "\n RSCs: " << rscs;
        qDebug() << "\n RIs:  " << ris;
        qDebug() << "\n RLs:  " << rls << "\n----------------------------------------";
    }
}

void ResultSetQuickCheckTest::generateActivitiesList()
{
    activitiesList = activities->activities();

    while (activitiesList.size() < NUMBER_ACTIVITIES) {
        activitiesList << QUuid::createUuid().toString().mid(1, 36);
    }
}

void ResultSetQuickCheckTest::generateAgentsList()
{
    for (int i = 0; i < NUMBER_AGENTS; ++i) {
        agentsList << QStringLiteral("Agent_") + QString::number(i);
    }
}

void ResultSetQuickCheckTest::generateTypesList()
{
    typesList = {
        QStringLiteral("application/postscript"),
        QStringLiteral("application/pdf"),
        QStringLiteral("image/x-psd"),
        QStringLiteral("image/x-sgi"),
        QStringLiteral("image/x-tga"),
        QStringLiteral("image/x-xbitmap"),
        QStringLiteral("image/x-xwindowdump"),
        QStringLiteral("image/x-xcf"),
        QStringLiteral("image/x-compressed-xcf"),
        QStringLiteral("image/tiff"),
        QStringLiteral("image/jpeg"),
        QStringLiteral("image/x-psp"),
        QStringLiteral("image/png"),
        QStringLiteral("image/x-icon"),
        QStringLiteral("image/x-xpixmap"),
        QStringLiteral("image/svg+xml"),
        QStringLiteral("application/pdf"),
        QStringLiteral("image/x-wmf"),
        QStringLiteral("image/jp2"),
        QStringLiteral("image/jpeg2000"),
        QStringLiteral("image/jpx"),
        QStringLiteral("image/x-xcursor"),
    };
}

void ResultSetQuickCheckTest::generateResourcesList()
{
    for (int i = 0; i < NUMBER_RESOURCES; ++i) {
        resourcesList << (QStringLiteral("/r") //
                          + (i < 10 ? QStringLiteral("0") : QString()) + QString::number(i));
    }
}

void ResultSetQuickCheckTest::generateResourceInfos()
{
    auto *generator = QRandomGenerator::global();
    for (const QString &resource : std::as_const(resourcesList)) {
        // We want every n-th or so to be without the title
        if (generator->bounded(3)) {
            continue;
        }

        ResourceInfo::Item ri;
        ri.targettedResource = resource;
        ri.title = QStringLiteral("Title_") + QString::number(generator->bounded(100));
        ri.mimetype = randItem(typesList);

        resourceInfos.insert(ri);
    }
}

void ResultSetQuickCheckTest::generateResourceScoreCaches()
{
    auto *generator = QRandomGenerator::global();
    for (int i = 0; i < NUMBER_CACHES; ++i) {
        ResourceScoreCache::Item rsc;

        rsc.usedActivity = randItem(activitiesList);
        rsc.initiatingAgent = randItem(agentsList);
        rsc.targettedResource = randItem(resourcesList);

        rsc.cachedScore = generator->bounded(1000);
        rsc.firstUpdate = generator->generate();
        rsc.lastUpdate = generator->generate();

        resourceScoreCaches.insert(rsc);
    }
}

void ResultSetQuickCheckTest::generateResourceLinks()
{
    auto *generator = QRandomGenerator::global();
    for (const QString &resource : std::as_const(resourcesList)) {
        // We don't want all the resources to be linked
        // to something
        if (generator->bounded(2)) {
            continue;
        }

        ResourceLink::Item rl;

        rl.targettedResource = resource;
        rl.usedActivity = randItem(activitiesList);
        rl.initiatingAgent = randItem(agentsList);

        resourceLinks.insert(rl);
    }
}

void ResultSetQuickCheckTest::pushToDatabase()
{
    auto database = Common::Database::instance(Common::Database::ResourcesDatabase, Common::Database::ReadWrite);

    Common::ResourcesDatabaseSchema::initSchema(*database);

    // Inserting activities, so that a test can be replicated
    database->execQuery(QStringLiteral("CREATE TABLE Activity (activity TEXT)"));
    for (const auto &activity : activitiesList) {
        database->execQuery(QStringLiteral("INSERT INTO Activity VALUES ('%1')").arg(activity));
    }

    // Inserting agent, so that a test can be replicated
    database->execQuery(QStringLiteral("CREATE TABLE Agent (agent TEXT)"));
    for (const auto &agent : agentsList) {
        database->execQuery(QStringLiteral("INSERT INTO Agent VALUES ('%1')").arg(agent));
    }

    // Inserting types, so that a test can be replicated
    database->execQuery(QStringLiteral("CREATE TABLE Type (type TEXT)"));
    for (const auto &type : typesList) {
        database->execQuery(QStringLiteral("INSERT INTO Type VALUES ('%1')").arg(type));
    }

    // Inserting resources, so that a test can be replicated
    database->execQuery(QStringLiteral("CREATE TABLE Resource (resource TEXT)"));
    for (const auto &resource : resourcesList) {
        database->execQuery(QStringLiteral("INSERT INTO Resource VALUES ('%1')").arg(resource));
    }

    // Inserting resource score caches
    qDebug() << "Inserting" << resourceScoreCaches.size() << "items into ResourceScoreCache";
    int i = 0;

    for (const auto &rsc : std::as_const(resourceScoreCaches)) {
        std::cerr << '.';

        if (++i % 10 == 0) {
            std::cerr << i;
        }

        database->execQuery( //
            QStringLiteral("INSERT INTO ResourceScoreCache ("
                           "  usedActivity"
                           ", initiatingAgent"
                           ", targettedResource"
                           ", scoreType"
                           ", cachedScore"
                           ", firstUpdate"
                           ", lastUpdate"
                           ") VALUES ("
                           "  '%1'" // usedActivity
                           ", '%2'" // initiatingAgent
                           ", '%3'" // targettedResource
                           ",   0 " // scoreType
                           ",  %4 " // cachedScore
                           ",  %5 " // firstUpdate
                           ",  %6 " // lastUpdate
                           ")")
                .arg(rsc.usedActivity, rsc.initiatingAgent, rsc.targettedResource)
                .arg(rsc.cachedScore)
                .arg(rsc.firstUpdate)
                .arg(rsc.lastUpdate));
    }
    std::cerr << std::endl;

    // Inserting resource infos
    qDebug() << "Inserting" << resourceInfos.size() << "items into ResourceInfo";
    i = 0;

    for (const auto &ri : std::as_const(resourceInfos)) {
        std::cerr << '.';

        if (++i % 10 == 0) {
            std::cerr << i;
        }

        database->execQuery( //
            QStringLiteral("INSERT INTO ResourceInfo ("
                           "  targettedResource"
                           ", title"
                           ", mimetype"
                           ", autoTitle"
                           ", autoMimetype"
                           ") VALUES ("
                           "  '%1' " // targettedResource
                           ", '%2' " // title
                           ", '%3' " // mimetype
                           ",   1  " // autoTitle
                           ",   1  " // autoMimetype
                           ")")
                .arg(ri.targettedResource, ri.title, ri.mimetype));
    }
    std::cerr << std::endl;

    // Inserting resource links
    qDebug() << "Inserting" << resourceLinks.size() << "items into ResourceLink";
    i = 0;

    for (const auto &rl : std::as_const(resourceLinks)) {
        std::cerr << '.';

        if (++i % 10 == 0) {
            std::cerr << i;
        }

        database->execQuery( //
            QStringLiteral("INSERT INTO ResourceLink ("
                           "  targettedResource"
                           ", usedActivity"
                           ", initiatingAgent"
                           ") VALUES ("
                           "  '%1' " // targettedResource
                           ", '%2' " // usedActivity
                           ", '%3' " // initiatingAgent
                           ")")
                .arg(rl.targettedResource, rl.usedActivity, rl.initiatingAgent));
    }
    std::cerr << std::endl;
}

void ResultSetQuickCheckTest::pullFromDatabase()
{
    auto database = Common::Database::instance(Common::Database::ResourcesDatabase, Common::Database::ReadWrite);

    auto activityQuery = database->execQuery(QStringLiteral("SELECT * FROM Activity"));
    for (const auto &activity : activityQuery) {
        activitiesList << activity[0].toString();
    }

    auto agentQuery = database->execQuery(QStringLiteral("SELECT * FROM Agent"));
    for (const auto &agent : agentQuery) {
        agentsList << agent[0].toString();
    }

    auto typeQuery = database->execQuery(QStringLiteral("SELECT * FROM Type"));
    for (const auto &type : typeQuery) {
        typesList << type[0].toString();
    }

    auto resourceQuery = database->execQuery(QStringLiteral("SELECT * FROM Resource"));
    for (const auto &resource : resourceQuery) {
        resourcesList << resource[0].toString();
    }

    auto rscQuery = database->execQuery(QStringLiteral("SELECT * FROM ResourceScoreCache"));

    for (const auto &rsc : rscQuery) {
        ResourceScoreCache::Item item;
        item.usedActivity = rsc[QStringLiteral("usedActivity")].toString();
        item.initiatingAgent = rsc[QStringLiteral("initiatingAgent")].toString();
        item.targettedResource = rsc[QStringLiteral("targettedResource")].toString();
        item.cachedScore = rsc[QStringLiteral("cachedScore")].toDouble();
        item.firstUpdate = rsc[QStringLiteral("firstUpdate")].toInt();
        item.lastUpdate = rsc[QStringLiteral("lastUpdate")].toInt();
        resourceScoreCaches.insert(item);
    }

    auto riQuery = database->execQuery(QStringLiteral("SELECT * FROM ResourceInfo"));

    for (const auto &ri : riQuery) {
        ResourceInfo::Item item;
        item.targettedResource = ri[QStringLiteral("targettedResource")].toString();
        item.title = ri[QStringLiteral("title")].toString();
        item.mimetype = ri[QStringLiteral("mimetype")].toString();
        resourceInfos.insert(item);
    }

    auto rlQuery = database->execQuery(QStringLiteral("SELECT * FROM ResourceLink"));

    for (const auto &rl : rlQuery) {
        ResourceLink::Item item;
        item.targettedResource = rl[QStringLiteral("targettedResource")].toString();
        item.usedActivity = rl[QStringLiteral("usedActivity")].toString();
        item.initiatingAgent = rl[QStringLiteral("initiatingAgent")].toString();
        resourceLinks.insert(item);
    }
}

void ResultSetQuickCheckTest::cleanupTestCase()
{
    Q_EMIT testFinished();
}

QString ResultSetQuickCheckTest::randItem(const QStringList &choices) const
{
    return choices[QRandomGenerator::global()->bounded(choices.size())];
}
//^ Data init

void ResultSetQuickCheckTest::testUsedResourcesForAgents()
{
    using namespace KAStats;
    using namespace KAStats::Terms;
    using boost::sort;
    using boost::adaptors::filtered;

    for (const auto &agent : std::as_const(agentsList)) {
        auto memItems = ResourceScoreCache::groupByResource(resourceScoreCaches | filtered(ResourceScoreCache::initiatingAgent() == agent));

        auto baseTerm = UsedResources | Agent{agent} | Activity::any();

#define ORDERING_TEST(Column, Dir, OrderFlag)                                                                                                                  \
    {                                                                                                                                                          \
        sort(memItems, ResourceScoreCache::Column().Dir() | ResourceScoreCache::targettedResource().asc());                                                    \
        ResultSet dbItems = baseTerm | OrderFlag | Limit(100);                                                                                                 \
        ASSERT_RANGE_EQUAL(memItems, dbItems);                                                                                                                 \
    }

        ORDERING_TEST(targettedResource, asc, OrderByUrl)
        ORDERING_TEST(cachedScore, desc, HighScoredFirst);
        ORDERING_TEST(lastUpdate, desc, RecentlyUsedFirst);
        ORDERING_TEST(firstUpdate, desc, RecentlyCreatedFirst);

#undef ORDERING_TEST
    }
}

void ResultSetQuickCheckTest::testUsedResourcesForActivities()
{
}

void ResultSetQuickCheckTest::testLinkedResourcesForAgents()
{
    using namespace KAStats;
    using namespace KAStats::Terms;
    using boost::sort;
    using boost::adaptors::filtered;

    for (const auto &agent : std::as_const(agentsList)) {
        auto memItems = ResourceLink::groupByResource(resourceLinks | filtered(ResourceLink::initiatingAgent() == agent));

        auto baseTerm = LinkedResources | Agent{agent} | Activity::any();

#define ORDERING_TEST(Column, Dir, OrderFlag)                                                                                                                  \
    {                                                                                                                                                          \
        sort(memItems, ResourceLink::Column().Dir() | ResourceLink::targettedResource().asc());                                                                \
        ResultSet dbItems = baseTerm | OrderFlag;                                                                                                              \
        ASSERT_RANGE_EQUAL(memItems, dbItems);                                                                                                                 \
    }

        ORDERING_TEST(targettedResource, asc, OrderByUrl)
        // ORDERING_TEST(cachedScore,       desc, HighScoredFirst);
        // ORDERING_TEST(lastUpdate,        desc, RecentlyUsedFirst);
        // ORDERING_TEST(firstUpdate,       desc, RecentlyCreatedFirst);

#undef ORDERING_TEST
    }
}

#include "moc_ResultSetQuickCheckTest.cpp"

// vim: set foldmethod=marker:
