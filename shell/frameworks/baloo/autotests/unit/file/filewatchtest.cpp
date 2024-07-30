/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filewatch.h"
#include "fileindexerconfigutils.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "pendingfilequeue.h"

#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <KFileMetaData/UserMetaData>

namespace Baloo {

class FileWatchTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testFileCreation();
    void testDirCreation();
    void testConfigChange();
    void testFileMoved();

    void init()
    {
        m_tmpDir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_tmpDir->isValid());

        m_dbDir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_dbDir->isValid());

        m_db = std::make_unique<Baloo::Database>(m_dbDir->path());
    }

private:
    std::unique_ptr<QTemporaryDir> m_dbDir;
    std::unique_ptr<QTemporaryDir> m_tmpDir;
    std::unique_ptr<Baloo::Database> m_db;
};

} // namespace Baloo

using namespace Baloo;

namespace {
    bool createFile(const QString& fileUrl) {
        QFile f1(fileUrl);
        f1.open(QIODevice::WriteOnly);
        f1.close();
        return QFile::exists(fileUrl);
    }

    bool createDir(const QString &dirPath)
    {
        return QDir().mkpath(dirPath);
    }

    void modifyFile(const QString& fileUrl) {
        QFile f1(fileUrl);
        f1.open(QIODevice::Append | QIODevice::Text);

        QTextStream stream(&f1);
        stream << "1";
    }
}

void FileWatchTest::testFileCreation()
{
    m_db->open(Baloo::Database::CreateDatabase);
    QVERIFY(m_db->isOpen());

    Test::writeIndexerConfig({m_tmpDir->path()}, {});
    FileIndexerConfig config;

    FileWatch fileWatch(m_db.get(), &config);
    fileWatch.m_pendingFileQueue->setMaximumTimeout(0);
    fileWatch.m_pendingFileQueue->setMinimumTimeout(0);
    fileWatch.m_pendingFileQueue->setTrackingTime(0);

    QSignalSpy spy(&fileWatch, &FileWatch::installedWatches);
    QVERIFY(spy.isValid());

    fileWatch.updateIndexedFoldersWatches();
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    QSignalSpy spyIndexNew(&fileWatch, &FileWatch::indexNewFile);
    QSignalSpy spyIndexModified(&fileWatch, &FileWatch::indexModifiedFile);
    QSignalSpy spyIndexXattr(&fileWatch, &FileWatch::indexXAttr);

    QVERIFY(spyIndexNew.isValid());
    QVERIFY(spyIndexModified.isValid());
    QVERIFY(spyIndexXattr.isValid());

    // Create a file and see if it is indexed
    QString fileUrl(m_tmpDir->path() + QStringLiteral("/t1"));
    QVERIFY(createFile(fileUrl));

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 1);
    QCOMPARE(spyIndexModified.count(), 0);
    QCOMPARE(spyIndexXattr.count(), 0);
    QCOMPARE(spyIndexNew.takeFirst().at(0), fileUrl);

    // Modify the file
    modifyFile(fileUrl);

    QVERIFY(spyIndexModified.wait());
    QCOMPARE(spyIndexNew.count(), 0);
    QCOMPARE(spyIndexModified.count(), 1);
    QCOMPARE(spyIndexXattr.count(), 0);
    QCOMPARE(spyIndexModified.takeFirst().at(0), fileUrl);

    // Set an Xattr
    KFileMetaData::UserMetaData umd(fileUrl);
    if (!umd.isSupported()) {
        qWarning() << "Xattr not supported on this filesystem:" << fileUrl;
    } else {
        const QString userComment(QStringLiteral("UserComment"));
        QVERIFY(umd.setUserComment(userComment) == KFileMetaData::UserMetaData::NoError);

        QVERIFY(spyIndexXattr.wait());
        QCOMPARE(spyIndexNew.count(), 0);
        QCOMPARE(spyIndexModified.count(), 0);
        QCOMPARE(spyIndexXattr.count(), 1);
        QCOMPARE(spyIndexXattr.takeFirst().at(0), fileUrl);
    }

    // Change permisssions
    QFile f(fileUrl);
    auto permissions = f.permissions();
    f.setPermissions(permissions & ~QFileDevice::WriteOwner);

    QVERIFY(spyIndexXattr.wait());
    QCOMPARE(spyIndexNew.count(), 0);
    QCOMPARE(spyIndexModified.count(), 0);
    QCOMPARE(spyIndexXattr.count(), 1);
    QCOMPARE(spyIndexXattr.takeFirst().at(0), fileUrl);
}

void FileWatchTest::testDirCreation()
{
    m_db->open(Baloo::Database::CreateDatabase);
    QVERIFY(m_db->isOpen());

    Test::writeIndexerConfig({m_tmpDir->path()}, {});
    FileIndexerConfig config;

    FileWatch fileWatch(m_db.get(), &config);
    fileWatch.m_pendingFileQueue->setMaximumTimeout(0);
    fileWatch.m_pendingFileQueue->setMinimumTimeout(0);
    fileWatch.m_pendingFileQueue->setTrackingTime(0);

    QSignalSpy spy(&fileWatch, &FileWatch::installedWatches);
    QVERIFY(spy.isValid());

    fileWatch.updateIndexedFoldersWatches();
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    QSignalSpy spyIndexNew(&fileWatch, &FileWatch::indexNewFile);
    QSignalSpy spyIndexModified(&fileWatch, &FileWatch::indexModifiedFile);
    QSignalSpy spyIndexXattr(&fileWatch, &FileWatch::indexXAttr);

    QVERIFY(spyIndexNew.isValid());
    QVERIFY(spyIndexModified.isValid());
    QVERIFY(spyIndexXattr.isValid());

    // Create a dir and see if it is indexed
    QString dirUrl(m_tmpDir->path() + QStringLiteral("/d1"));
    QVERIFY(createDir(dirUrl));
    QString dirUrlTrailingSlash = QStringLiteral("%1/").arg(dirUrl);

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 1);
    QCOMPARE(spyIndexModified.count(), 0);
    QCOMPARE(spyIndexXattr.count(), 0);
    QCOMPARE(spyIndexNew.takeFirst().at(0), dirUrlTrailingSlash);

    // Set an Xattr
    KFileMetaData::UserMetaData umd(dirUrl);
    if (!umd.isSupported()) {
        qWarning() << "Xattr not supported on this filesystem:" << dirUrl;
    } else {
        const QString userComment(QStringLiteral("UserComment"));
        QVERIFY(umd.setUserComment(userComment) == KFileMetaData::UserMetaData::NoError);

        QVERIFY(spyIndexXattr.wait());
        QCOMPARE(spyIndexNew.count(), 0);
        QCOMPARE(spyIndexModified.count(), 0);
        QCOMPARE(spyIndexXattr.count(), 1);
        QCOMPARE(spyIndexXattr.takeFirst().at(0), dirUrlTrailingSlash);
    }

    // Change permisssions
    QFile f(dirUrl);
    auto permissions = f.permissions();
    f.setPermissions(permissions & ~QFileDevice::WriteOwner);

    QVERIFY(spyIndexXattr.wait());
    QCOMPARE(spyIndexNew.count(), 0);
    QCOMPARE(spyIndexModified.count(), 0);
    QCOMPARE(spyIndexXattr.count(), 1);
    QCOMPARE(spyIndexXattr.takeFirst().at(0), dirUrlTrailingSlash);
}

void FileWatchTest::testConfigChange()
{
    m_db->open(Baloo::Database::CreateDatabase);

    QString d1 = m_tmpDir->path() + QStringLiteral("/d1");
    QString d2 = m_tmpDir->path() + QStringLiteral("/d2");
    QString d11 = m_tmpDir->path() + QStringLiteral("/d1/d11");
    QString d21 = m_tmpDir->path() + QStringLiteral("/d2/d21");
    QString d22 = m_tmpDir->path() + QStringLiteral("/d2/d22");

    QDir().mkpath(d11);
    QDir().mkpath(d21);
    QDir().mkpath(d22);

    // parameters: includeFolders list, excludeFolders list
    Test::writeIndexerConfig({d1, d2}, {d11, d21});
    FileIndexerConfig config;

    FileWatch fileWatch(m_db.get(), &config);
    fileWatch.m_pendingFileQueue->setMaximumTimeout(0);
    fileWatch.m_pendingFileQueue->setMinimumTimeout(0);
    fileWatch.m_pendingFileQueue->setTrackingTime(0);

    QSignalSpy spy(&fileWatch, &FileWatch::installedWatches);
    QVERIFY(spy.isValid());

    fileWatch.updateIndexedFoldersWatches();
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 2);

    QSignalSpy spyIndexNew(&fileWatch, &FileWatch::indexNewFile);
    QVERIFY(spyIndexNew.isValid());
    QVERIFY(createFile(d1 + QStringLiteral("/t1")));
    QVERIFY(createFile(d2 + QStringLiteral("/t2")));

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 2);
    spyIndexNew.clear();

    // dir d22 is not yet excluded, so one event is expected
    QVERIFY(createFile(d11 + QStringLiteral("/tx1")));
    QVERIFY(createFile(d21 + QStringLiteral("/tx2")));
    QVERIFY(createFile(d22 + QStringLiteral("/tx3")));

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 1);
    QList<QVariant> event = spyIndexNew.at(0);
    QCOMPARE(event.at(0).toString(), d22 + QStringLiteral("/tx3"));
    spyIndexNew.clear();

    Test::writeIndexerConfig({d2}, {d22});
    config.forceConfigUpdate();
    fileWatch.updateIndexedFoldersWatches();

    // dir d1 is no longer included
    QVERIFY(createFile(d1 + QStringLiteral("/tx1a")));
    QVERIFY(createFile(d2 + QStringLiteral("/tx2a")));
    QVERIFY(spyIndexNew.wait());
    QList<QString> result;
    for (const QList<QVariant>& event : std::as_const(spyIndexNew)) {
	result.append(event.at(0).toString());
    }
    QCOMPARE(result, {d2 + QStringLiteral("/tx2a")});
    spyIndexNew.clear();
    result.clear();

    // d11 is implicitly excluded, as d1 is no longer included
    // d22 is explicitly excluded now, d21 is included
    QVERIFY(createFile(d11 + QStringLiteral("/tx1b")));
    QVERIFY(createFile(d21 + QStringLiteral("/tx2b")));
    QVERIFY(createFile(d22 + QStringLiteral("/tx3b")));

    QVERIFY(spyIndexNew.wait(500));
    for (const QList<QVariant>& event : std::as_const(spyIndexNew)) {
	result.append(event.at(0).toString());
    }
    QCOMPARE(result, {d21 + QStringLiteral("/tx2b")});
}

void FileWatchTest::testFileMoved()
{
    m_db->open(Baloo::Database::CreateDatabase);
    QVERIFY(m_db->isOpen());

    Test::writeIndexerConfig({m_tmpDir->path()}, {});
    FileIndexerConfig config;

    FileWatch fileWatch(m_db.get(), &config);
    fileWatch.m_pendingFileQueue->setMaximumTimeout(0);
    fileWatch.m_pendingFileQueue->setMinimumTimeout(0);
    fileWatch.m_pendingFileQueue->setTrackingTime(0);

    QSignalSpy spy(&fileWatch, &FileWatch::installedWatches);
    QVERIFY(spy.isValid());

    fileWatch.updateIndexedFoldersWatches();
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    QSignalSpy spyIndexNew(&fileWatch, &FileWatch::indexNewFile);
    QSignalSpy spyIndexModified(&fileWatch, &FileWatch::indexModifiedFile);
    QSignalSpy spyIndexFileRemoved(&fileWatch, &FileWatch::fileRemoved);

    QVERIFY(spyIndexNew.isValid());
    QVERIFY(spyIndexModified.isValid());
    QVERIFY(spyIndexFileRemoved.isValid());

    // Create a file and see if it is indexed
    QString fileUrl(m_tmpDir->path() + QStringLiteral("/t1"));
    QString fileDestUrl(m_tmpDir->path() + QStringLiteral("/t2"));
    QVERIFY(createFile(fileUrl));

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 1);
    QCOMPARE(spyIndexModified.count(), 0);
    QCOMPARE(spyIndexFileRemoved.count(), 0);
    QCOMPARE(spyIndexNew.takeFirst().at(0), fileUrl);

    // Move the file
    QVERIFY(QFile::rename(fileUrl, fileDestUrl));

    QVERIFY(spyIndexFileRemoved.wait());
    QCOMPARE(spyIndexNew.count(), 1);
    QCOMPARE(spyIndexNew.takeFirst().at(0), fileDestUrl);
    QCOMPARE(spyIndexModified.count(), 0);
    QCOMPARE(spyIndexFileRemoved.count(), 1);
    QCOMPARE(spyIndexFileRemoved.takeFirst().at(0), fileDestUrl); // called to clean dest metadata
}

QTEST_MAIN(FileWatchTest)

#include "filewatchtest.moc"
