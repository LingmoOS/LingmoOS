/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "querytest.h"

#include <KLocalizedString>
#include <QCoreApplication>
#include <QStandardPaths>

#include "packagejob.h"
#include "packageloader.h"
#include "packagestructure.h"

#include "config.h"

void QueryTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    // Remove any eventual installed package globally on the system
    qputenv("XDG_DATA_DIRS", "/not/valid");
    qputenv("KPACKAGE_DEP_RESOLVERS_PATH", KPACKAGE_DEP_RESOLVERS);

    m_dataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));

    // verify that the test plugin if found
    QVERIFY(KPackage::PackageLoader::self()->loadPackageStructure("Plasma/TestKPackageInternalPlasmoid"));
}

static bool checkedInstall(const QString &packageFormat, const QString &source, int expectedError)
{
    auto job = KPackage::PackageJob::install(packageFormat, source, {});
    QEventLoop l;
    QObject::connect(job, &KJob::result, &l, [&l]() {
        l.quit();
    });
    l.exec();
    if (job->error() == expectedError) {
        return true;
    }
    qWarning() << "Unexpected error" << job->error() << "while installing" << source << job->errorString();
    return false;
}

void QueryTest::installAndQuery()
{
    m_dataDir.removeRecursively();
    // verify that no packages are installed
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 0);

    // install some packages
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testpackage"), KJob::NoError));
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testfallbackpackage"), KJob::NoError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 2);

    // installing package with invalid metadata should not be possible
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testinvalidmetadata"), KPackage::PackageJob::PluginIdInvalidError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 2);

    // package with valid dep information should be installed
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testpackagesdep"), KJob::NoError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 3);

    // package with invalid dep information should not be installed
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testpackagesdepinvalid"), KPackage::PackageJob::JobError::PackageCopyError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 3);
}

void QueryTest::queryCustomPlugin()
{
    m_dataDir.removeRecursively();

    // verify that no packages are installed
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 0);

    auto testPackageStructure = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/TestKPackageInternalPlasmoid"));
    // install some packages
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testpackage"), KJob::NoError));
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testfallbackpackage"), KJob::NoError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 2);

    // installing package with invalid metadata should not be possible
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testinvalidmetadata"), KPackage::PackageJob::JobError::PluginIdInvalidError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 2);

    // package with valid dep information should be installed
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testpackagesdep"), KJob::NoError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 3);

    // package with invalid dep information should not be installed
    QVERIFY(checkedInstall(packageFormat, QFINDTESTDATA("data/testpackagesdepinvalid"), KPackage::PackageJob::JobError::PackageCopyError));

    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 3);
}

QTEST_MAIN(QueryTest)

#include "moc_querytest.cpp"
