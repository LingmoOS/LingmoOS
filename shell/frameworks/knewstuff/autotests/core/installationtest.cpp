/*
    This file is part of KNewStuff3
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <KSharedConfig>
#include <QDir>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTest>
#include <QtGlobal>

#include "entry.h"
#include "installation_p.h"
#include "itemsmodel.h"
#include "question.h"
#include "questionmanager.h"

using namespace KNSCore;

class InstallationTest : public QObject
{
    Q_OBJECT
public:
    Installation *installation = nullptr;
    const QString dataDir = QStringLiteral(DATA_DIR);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testConfigFileReading();
    void testInstallCommand();
    void testInstallCommandArchive();
    void testInstallCommandTopLevelFilesInArchive();
    void testUninstallCommand();
    void testUninstallCommandDirectory();
    void testCopyError();
};

void InstallationTest::initTestCase()
{
    // Just in case a previous test crashed
    cleanupTestCase();
    qRegisterMetaType<Entry>();
    QStandardPaths::setTestModeEnabled(true);
    installation = new Installation(this);
    KConfigGroup grp = KSharedConfig::openConfig(dataDir + QLatin1String("installationtest.knsrc"))->group(QStringLiteral("KNewStuff"));
    QString err;
    QVERIFY(installation->readConfig(grp, err));
    connect(KNSCore::QuestionManager::instance(), &KNSCore::QuestionManager::askQuestion, this, [](KNSCore::Question *q) {
        q->setResponse(KNSCore::Question::YesResponse);
    });
}

void InstallationTest::cleanupTestCase()
{
    QFile::remove(QStringLiteral("installed.txt"));
    QFile::remove(QStringLiteral("uninstalled.txt"));
    const QString dataPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("demo/"), QStandardPaths::LocateDirectory);
    if (!dataPath.isEmpty()) {
        QDir(dataPath).removeRecursively();
    }
}

void InstallationTest::testConfigFileReading()
{
    QCOMPARE(installation->uncompressionSetting(), Installation::UncompressIntoSubdirIfArchive);
    const QString actualPath = installation->targetInstallationPath();
    const QString expectedPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/demo/");
    QCOMPARE(actualPath, expectedPath);
}

void InstallationTest::testInstallCommand()
{
    Entry entry;
    entry.setUniqueId(QStringLiteral("0"));
    entry.setName(QStringLiteral("testInstallCommand"));
    entry.setPayload(QUrl::fromLocalFile(QFINDTESTDATA("data/testfile.txt")).toString());
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    installation->install(entry);
    QVERIFY(spy.wait());
    QCOMPARE(entry.status(), KNSCore::Entry::Installed);
    QVERIFY(QFileInfo::exists(QStringLiteral("installed.txt")));
}

void InstallationTest::testUninstallCommand()
{
    Entry entry;
    entry.setUniqueId(QStringLiteral("0"));
    QFile file(QStringLiteral("testFile.txt"));
    file.open(QIODevice::WriteOnly);
    file.close();
    entry.setStatus(KNSCore::Entry::Installed);
    entry.setInstalledFiles(QStringList(file.fileName()));
    QVERIFY(QFileInfo(file).exists());
    QVERIFY(!QFileInfo::exists(QStringLiteral("uninstalled.txt")));

    installation->uninstall(entry);
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QVERIFY(spy.wait());
    QCOMPARE(entry.status(), KNSCore::Entry::Deleted);
    QVERIFY(!QFileInfo(file).exists());
    QVERIFY(QFileInfo::exists(QStringLiteral("uninstalled.txt")));
}

void InstallationTest::testUninstallCommandDirectory()
{
    static const QLatin1String testDir{"testDirectory"};

    // This will be left over from the previous test, so clean it up first
    QFile::remove(QStringLiteral("uninstalled.txt"));

    Entry entry;
    entry.setUniqueId(QStringLiteral("0"));
    QDir().mkdir(testDir);
    entry.setStatus(KNSCore::Entry::Installed);
    entry.setInstalledFiles(QStringList(testDir));
    QVERIFY(QFileInfo::exists(testDir));
    QVERIFY(!QFileInfo::exists(QStringLiteral("uninstalled.txt")));

    installation->uninstall(entry);
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QVERIFY(spy.wait());
    QCOMPARE(entry.status(), KNSCore::Entry::Deleted);
    QVERIFY(!QFileInfo::exists(testDir));
    QVERIFY(QFileInfo::exists(QStringLiteral("uninstalled.txt")));
}

void InstallationTest::testInstallCommandArchive()
{
    Entry entry;
    entry.setUniqueId(QStringLiteral("0"));
    entry.setStatus(KNSCore::Entry::Downloadable);
    entry.setPayload(QUrl::fromLocalFile(QFINDTESTDATA("data/archive_dir.tar.gz")).toString());

    installation->install(entry);
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QVERIFY(spy.wait());

    QCOMPARE(entry.installedFiles().count(), 1);
    const QString file = entry.installedFiles().constFirst();
    const QFileInfo fileInfo(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("demo/data"), QStandardPaths::LocateDirectory));
    QCOMPARE(file, fileInfo.absoluteFilePath() + QLatin1String("/*"));
    QVERIFY(fileInfo.exists());
    QVERIFY(fileInfo.isDir());

    // Check if the files that are in the archive exist
    const QStringList files = QDir(fileInfo.absoluteFilePath()).entryList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(files, QStringList({QStringLiteral("test1.txt"), QStringLiteral("test2.txt")}));
}

void InstallationTest::testInstallCommandTopLevelFilesInArchive()
{
    Entry entry;
    entry.setUniqueId(QStringLiteral("0"));
    entry.setStatus(KNSCore::Entry::Downloadable);
    entry.setPayload(QUrl::fromLocalFile(QFINDTESTDATA("data/archive_toplevel_files.tar.gz")).toString());

    installation->install(entry);
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QVERIFY(spy.wait());

    QCOMPARE(entry.installedFiles().count(), 1);
    const QString file = entry.installedFiles().constFirst();
    QVERIFY(file.endsWith(QLatin1String("/*")));

    // The file is given a random name, so we can't easily check that
    const QFileInfo fileOnDisk(file.left(file.size() - 2));
    QVERIFY(fileOnDisk.exists());
    QVERIFY(fileOnDisk.isDir());
    // The by checking the parent dir we can check if it is properly in a subdir uncompressed
    QCOMPARE(fileOnDisk.absoluteDir().path(),
             QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("demo"), QStandardPaths::LocateDirectory));
    // Check if the files that are in the archive exist
    const QStringList files = QDir(fileOnDisk.absoluteFilePath()).entryList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(files, QStringList({QStringLiteral("test1.txt"), QStringLiteral("test2.txt")}));
}

void InstallationTest::testCopyError()
{
    Entry entry;
    entry.setUniqueId(QStringLiteral("0"));
    entry.setPayload(QUrl::fromLocalFile(QStringLiteral("data/does_not_exist.txt")).toString());
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QSignalSpy errorSpy(installation, &Installation::signalInstallationFailed);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Download of .* failed, error: Could not open data/does_not_exist.txt for reading")));
    installation->install(entry);
    QVERIFY(errorSpy.wait());
    QCOMPARE(spy.count(), 0);
    QCOMPARE(int(entry.status()), int(KNSCore::Entry::Invalid));
}

QTEST_MAIN(InstallationTest)

#include "installationtest.moc"
