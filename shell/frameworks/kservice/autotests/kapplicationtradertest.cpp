/*
    SPDX-FileCopyrightText: 2006-2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "setupxdgdirs.h"

#include <locale.h>

#include <QTest>

#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>
#include <kbuildsycoca_p.h>
#include <ksycoca.h>

#include <kapplicationtrader.h>
#include <kservicegroup.h>

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QThread>

#include <QDebug>
#include <QLoggingCategory>
#include <QMimeDatabase>

enum class ExpectedResult {
    NoResults,
    FakeApplicationOnly,
    FakeSchemeHandlerOnly,
    FakeApplicationAndOthers,
    NotFakeApplication,
};
Q_DECLARE_METATYPE(ExpectedResult)

class KApplicationTraderTest : public QObject
{
    Q_OBJECT
public:
private Q_SLOTS:
    void initTestCase();
    void testTraderConstraints_data();
    void testTraderConstraints();
    void testQueryByMimeType();
    void testThreads();
    void testTraderQueryMustRebuildSycoca();
    void testSetPreferredService();
    void cleanupTestCase();

private:
    QString createFakeApplication(const QString &filename, const QString &name, const QMap<QString, QString> &extraFields = {});
    void checkResult(const KService::List &offers, ExpectedResult expectedResult);

    QString m_fakeApplication;
    QString m_fakeSchemeHandler;
    QString m_fakeGnomeApplication;
    QStringList m_createdDesktopFiles;
};

QTEST_MAIN(KApplicationTraderTest)

extern KSERVICE_EXPORT int ksycoca_ms_between_checks;

void KApplicationTraderTest::initTestCase()
{
    // Set up a layer in the bin dir so ksycoca finds the desktop files created by createFakeApplication
    // Note that we still need /usr in there so that mimetypes are found
    setupXdgDirs();

    qputenv("XDG_CURRENT_DESKTOP", "KDE");

    QStandardPaths::setTestModeEnabled(true);

    // A non-C locale is necessary for some tests.
    // This locale must have the following properties:
    //   - some character other than dot as decimal separator
    // If it cannot be set, locale-dependent tests are skipped.
    setlocale(LC_ALL, "fr_FR.utf8");
    bool hasNonCLocale = (setlocale(LC_ALL, nullptr) == QByteArray("fr_FR.utf8"));
    if (!hasNonCLocale) {
        qDebug() << "Setting locale to fr_FR.utf8 failed";
    }

    // Ensure no leftovers from other tests
    QDir(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)).removeRecursively();

    // Create some fake services for the tests below, and ensure they are in ksycoca.

    // fakeservice_deleteme: deleted and recreated by testDeletingService, don't use in other tests
    createFakeApplication(QStringLiteral("fakeservice_deleteme.desktop"), QStringLiteral("DeleteMe"));

    // fakeapplication
    m_fakeApplication = createFakeApplication(QStringLiteral("fakeapplication.desktop"), QStringLiteral("FakeApplication"));
    m_fakeApplication = QFileInfo(m_fakeApplication).canonicalFilePath();

    m_fakeSchemeHandler = createFakeApplication(QStringLiteral("fakeschemehandler.desktop"),
                                                QStringLiteral("FakeSchemeHandler"),
                                                {{QStringLiteral("MimeType"), QStringLiteral("text/plain;x-scheme-handler/someprotocol")}});
    m_fakeSchemeHandler = QFileInfo(m_fakeSchemeHandler).canonicalFilePath();

    // fakegnomeapplication (do not show in Plasma). Should never be returned. To test the filtering code in queryByMimeType.
    QMap<QString, QString> fields;
    fields.insert(QStringLiteral("OnlyShowIn"), QStringLiteral("Gnome"));
    m_fakeGnomeApplication = createFakeApplication(QStringLiteral("fakegnomeapplication.desktop"), QStringLiteral("FakeApplicationGnome"), fields);
    m_fakeGnomeApplication = QFileInfo(m_fakeGnomeApplication).canonicalFilePath();

    ksycoca_ms_between_checks = 0;
}

void KApplicationTraderTest::cleanupTestCase()
{
    for (const QString &file : std::as_const(m_createdDesktopFiles)) {
        QFile::remove(file);
    }
}

// Helper method for all the trader tests
static bool offerListHasService(const KService::List &offers, const QString &entryPath)
{
    bool found = false;
    for (const auto &service : offers) {
        if (service->entryPath() == entryPath) {
            if (found) { // should be there only once
                qWarning("ERROR: %s was found twice in the list", qPrintable(entryPath));
                return false; // make test fail
            }
            found = true;
        }
    }
    return found;
}

void KApplicationTraderTest::checkResult(const KService::List &offers, ExpectedResult expectedResult)
{
    switch (expectedResult) {
    case ExpectedResult::NoResults:
        if (!offers.isEmpty()) {
            qWarning() << "Got" << offers.count() << "unexpected results, including" << offers.at(0)->entryPath();
        }
        QCOMPARE(offers.count(), 0);
        break;
    case ExpectedResult::FakeApplicationOnly:
        if (offers.count() != 1) {
            for (const auto &service : offers) {
                qWarning() << "    " << service->entryPath();
            }
        }
        QCOMPARE(offers.count(), 1);
        QCOMPARE(offers.at(0)->entryPath(), m_fakeApplication);
        break;
    case ExpectedResult::FakeSchemeHandlerOnly:
        if (offers.count() != 1) {
            for (const auto &service : offers) {
                qWarning() << "    " << service->entryPath();
            }
        }
        QCOMPARE(offers.count(), 1);
        QCOMPARE(offers.at(0)->entryPath(), m_fakeSchemeHandler);
        break;
    case ExpectedResult::FakeApplicationAndOthers:
        QVERIFY(!offers.isEmpty());
        if (!offerListHasService(offers, m_fakeApplication)) {
            qWarning() << m_fakeApplication << "not found. Here's what we have:";
            for (const auto &service : offers) {
                qWarning() << "    " << service->entryPath();
            }
        }
        QVERIFY(offerListHasService(offers, m_fakeApplication));
        break;
    case ExpectedResult::NotFakeApplication:
        QVERIFY(!offerListHasService(offers, m_fakeApplication));
        break;
    }
    QVERIFY(!offerListHasService(offers, m_fakeGnomeApplication));
}

using FF = KApplicationTrader::FilterFunc;
Q_DECLARE_METATYPE(KApplicationTrader::FilterFunc)

void KApplicationTraderTest::testTraderConstraints_data()
{
    QTest::addColumn<KApplicationTrader::FilterFunc>("filterFunc");
    QTest::addColumn<ExpectedResult>("expectedResult");

    QTest::newRow("no_constraint") << FF([](const KService::Ptr &) {
        return true;
    }) << ExpectedResult::FakeApplicationAndOthers;

    // == tests
    FF name_comparison = [](const KService::Ptr &serv) {
        return serv->name() == QLatin1String("FakeApplication");
    };
    QTest::newRow("name_comparison") << name_comparison << ExpectedResult::FakeApplicationOnly;
    FF isDontExist = [](const KService::Ptr &serv) {
        return serv->name() == QLatin1String("IDontExist");
    };
    QTest::newRow("no_such_name") << isDontExist << ExpectedResult::NoResults;
    FF no_such_name_by_case = [](const KService::Ptr &serv) {
        return serv->name() == QLatin1String("fakeapplication");
    };
    QTest::newRow("no_such_name_by_case") << no_such_name_by_case << ExpectedResult::NoResults;

    // Name =~ 'fAkEaPPlicaTion'
    FF match_case_insensitive = [](const KService::Ptr &serv) {
        return serv->name().compare(QLatin1String{"fAkEaPPlicaTion"}, Qt::CaseInsensitive) == 0;
    };
    QTest::newRow("match_case_insensitive") << match_case_insensitive << ExpectedResult::FakeApplicationOnly;

    // 'FakeApp' ~ Name
    FF is_contained_in = [](const KService::Ptr &serv) {
        return serv->name().contains(QLatin1String{"FakeApp"});
    };
    QTest::newRow("is_contained_in") << is_contained_in << ExpectedResult::FakeApplicationOnly;

    // 'FakeApplicationNot' ~ Name
    FF is_contained_in_fail = [](const KService::Ptr &serv) {
        return serv->name().contains(QLatin1String{"FakeApplicationNot"});
    };
    QTest::newRow("is_contained_in_fail") << is_contained_in_fail << ExpectedResult::NoResults;

    // 'faKeApP' ~~ Name
    FF is_contained_in_case_insensitive = [](const KService::Ptr &serv) {
        return serv->name().contains(QLatin1String{"faKeApP"}, Qt::CaseInsensitive);
    };
    QTest::newRow("is_contained_in_case_insensitive") << is_contained_in_case_insensitive << ExpectedResult::FakeApplicationOnly;

    // 'faKeApPp' ~ Name
    FF is_contained_in_case_in_fail = [](const KService::Ptr &serv) {
        return serv->name().contains(QLatin1String{"faKeApPp"}, Qt::CaseInsensitive);
    };
    QTest::newRow("is_contained_in_case_in_fail") << is_contained_in_case_in_fail << ExpectedResult::NoResults;

    // 'FkApli' subseq Name
    FF subseq = [](const KService::Ptr &serv) {
        return KApplicationTrader::isSubsequence(QStringLiteral("FkApli"), serv->name());
    };
    QTest::newRow("subseq") << subseq << ExpectedResult::FakeApplicationOnly;

    // 'fkApli' subseq Name
    FF subseq_fail = [](const KService::Ptr &serv) {
        return KApplicationTrader::isSubsequence(QStringLiteral("fkApli"), serv->name());
    };
    QTest::newRow("subseq_fail") << subseq_fail << ExpectedResult::NoResults;

    // 'fkApLI' ~subseq Name
    FF subseq_case_insensitive = [](const KService::Ptr &serv) {
        return KApplicationTrader::isSubsequence(QStringLiteral("fkApLI"), serv->name(), Qt::CaseInsensitive);
    };
    QTest::newRow("subseq_case_insensitive") << subseq_case_insensitive << ExpectedResult::FakeApplicationOnly;

    // 'fk_Apli' ~subseq Name
    FF subseq_case_insensitive_fail = [](const KService::Ptr &serv) {
        return KApplicationTrader::isSubsequence(QStringLiteral("fk_Apli"), serv->name(), Qt::CaseInsensitive);
    };
    QTest::newRow("subseq_case_insensitive_fail") << subseq_case_insensitive_fail << ExpectedResult::NoResults;

    // Test another property, parsed as a double
    FF testVersion = [](const KService::Ptr &serv) {
        double d = serv->property<double>(QStringLiteral("X-KDE-Version"));
        return d > 5.559 && d < 5.561;
    };
    QTest::newRow("float_parsing") << testVersion << ExpectedResult::FakeApplicationAndOthers;
}

void KApplicationTraderTest::testTraderConstraints()
{
    QFETCH(KApplicationTrader::FilterFunc, filterFunc);
    QFETCH(ExpectedResult, expectedResult);

    const KService::List offers = KApplicationTrader::query(filterFunc);
    checkResult(offers, expectedResult);
}

void KApplicationTraderTest::testQueryByMimeType()
{
    KService::List offers;

    // Without constraint

    offers = KApplicationTrader::queryByMimeType(QStringLiteral("text/plain"));
    checkResult(offers, ExpectedResult::FakeApplicationAndOthers);

    offers = KApplicationTrader::queryByMimeType(QStringLiteral("image/png"));
    checkResult(offers, ExpectedResult::NotFakeApplication);

    offers = KApplicationTrader::queryByMimeType(QStringLiteral("x-scheme-handler/someprotocol"));
    checkResult(offers, ExpectedResult::FakeSchemeHandlerOnly);

    QTest::ignoreMessage(QtWarningMsg, "KApplicationTrader: mimeType \"no/such/mimetype\" not found");
    offers = KApplicationTrader::queryByMimeType(QStringLiteral("no/such/mimetype"));
    checkResult(offers, ExpectedResult::NoResults);

    // With constraint

    FF isFakeApplication = [](const KService::Ptr &serv) {
        return serv->name() == QLatin1String("FakeApplication");
    };
    offers = KApplicationTrader::queryByMimeType(QStringLiteral("text/plain"), isFakeApplication);
    checkResult(offers, ExpectedResult::FakeApplicationOnly);

    FF isDontExist = [](const KService::Ptr &serv) {
        return serv->name() == QLatin1String("IDontExist");
    };
    offers = KApplicationTrader::queryByMimeType(QStringLiteral("text/plain"), isDontExist);
    checkResult(offers, ExpectedResult::NoResults);
}

QString KApplicationTraderTest::createFakeApplication(const QString &filename, const QString &name, const QMap<QString, QString> &extraFields)
{
    const QString fakeService = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1Char('/') + filename;
    QFile::remove(fakeService);
    m_createdDesktopFiles << fakeService;
    KDesktopFile file(fakeService);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", name);
    group.writeEntry("Type", "Application");
    group.writeEntry("Exec", "ls");
    group.writeEntry("Categories", "FakeCategory");
    group.writeEntry("X-KDE-Version", "5.56");
    group.writeEntry("MimeType", "text/plain;");
    for (auto it = extraFields.begin(); it != extraFields.end(); ++it) {
        group.writeEntry(it.key(), it.value());
    }
    return fakeService;
}

#include <QFutureSynchronizer>
#include <QThreadPool>
#include <QtConcurrentRun>

// Testing for concurrent access to ksycoca from multiple threads
// Use thread-sanitizer to see the data races

void KApplicationTraderTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    QFutureSynchronizer<void> sync;
    // Can't use data-driven tests here, QTestLib isn't threadsafe.
    sync.addFuture(QtConcurrent::run(&KApplicationTraderTest::testQueryByMimeType, this));
    sync.addFuture(QtConcurrent::run(&KApplicationTraderTest::testQueryByMimeType, this));
    sync.waitForFinished();
}

void KApplicationTraderTest::testTraderQueryMustRebuildSycoca()
{
    auto filter = [](const KService::Ptr &serv) {
        return serv->name() == QLatin1String("MustRebuild");
    };
    QCOMPARE(KApplicationTrader::query(filter).count(), 0);
    createFakeApplication(QStringLiteral("fakeservice_querymustrebuild.desktop"), QStringLiteral("MustRebuild"));
    KService::List offers = KApplicationTrader::query(filter);
    QCOMPARE(offers.count(), 1);
}

void KApplicationTraderTest::testSetPreferredService()
{
    const KService::Ptr oldPref = KApplicationTrader::preferredService(QLatin1String("text/plain"));
    const KService::Ptr newPref = KService::serviceByDesktopPath(m_fakeApplication);
    KApplicationTrader::setPreferredService(QLatin1String("text/plain"), newPref);
    QCOMPARE(KApplicationTrader::preferredService(QLatin1String("text/plain"))->entryPath(), m_fakeApplication);
    KApplicationTrader::setPreferredService(QLatin1String("text/plain"), oldPref);
    QCOMPARE(KApplicationTrader::preferredService(QLatin1String("text/plain"))->storageId(), oldPref->storageId());
}

#include "kapplicationtradertest.moc"
