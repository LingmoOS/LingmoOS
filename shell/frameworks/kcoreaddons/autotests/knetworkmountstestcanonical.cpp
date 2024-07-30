/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knetworkmountstestcanonical.h"

#include <KNetworkMounts>

#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(KNetworkMountsTestCanonical)

void KNetworkMountsTestCanonical::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_configFileName = QStringLiteral("%1/network_mounts").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));

    QFile::remove(m_configFileName);
    QVERIFY(!QFile::exists(m_configFileName));

    // create directory structure
    QVERIFY(m_tmpDir.isValid());

    const QString relLinkToPath = QStringLiteral("dir");
    const QString relSymlinkDirectory = QStringLiteral("symlinkDirectory");
    const QStringList relPaths = {relLinkToPath,
                                  QStringLiteral("dir/subdir1"),
                                  QStringLiteral("dir/subdir1/subdir1"),
                                  QStringLiteral("dir/subdir1/subdir2"),
                                  QStringLiteral("dir/subdir1/subdir3"),
                                  QStringLiteral("dir/subdir2"),
                                  QStringLiteral("dir/subdir2/subdir1"),
                                  QStringLiteral("dir/subdir2/subdir2"),
                                  QStringLiteral("dir/subdir2/subdir3"),
                                  relSymlinkDirectory};

    const QString relSymlinkToSmbPath = QStringLiteral("symlinkToSmbPath");

    QDir dir(m_tmpDir.path());
    for (const QString &relPath : relPaths) {
        QVERIFY(dir.mkpath(relPath));
        QVERIFY(QFile::exists(m_tmpDir.path() + QLatin1Char('/') + relPath));

        const QString fileName = m_tmpDir.path() + QLatin1Char('/') + relPath + QLatin1String("/file.txt");
        QFile file(fileName);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.close();
        QVERIFY(QFile::exists(fileName));
    }

    const QString linkToPath = m_tmpDir.path() + QLatin1Char('/') + relLinkToPath;

    // SymlinkToNetworkMount
    const QString symlinkToSmbPath = dir.path() + QLatin1Char('/') + relSymlinkToSmbPath;

    QVERIFY(QFile::link(linkToPath, symlinkToSmbPath));
    qDebug() << "linkToPath=" << linkToPath << ", symlinkToSmbPath=" << symlinkToSmbPath;

    // SymlinkDirectory
    QVERIFY(dir.cd(relSymlinkDirectory));
    const QString symlinkDirectory = dir.path();
    const QString linkStr = symlinkDirectory + QLatin1Char('/') + relLinkToPath;

    QVERIFY(QFile::link(linkToPath, linkStr));
    qDebug() << "linkToPath=" << linkToPath << ", symlinkDirectory=" << symlinkDirectory << ", linkStr=" << linkStr;

    // setup config
    KNetworkMounts::self()->setEnabled(true);

    const QStringList paths = {linkToPath};
    KNetworkMounts::self()->setPaths(paths, KNetworkMounts::SmbPaths);

    const QStringList savedPaths = {linkToPath + QLatin1Char('/')};
    QCOMPARE(KNetworkMounts::self()->paths(), savedPaths);

    // SymlinkDirectory
    const QStringList symlinkDirectories = {symlinkDirectory};
    KNetworkMounts::self()->setPaths(symlinkDirectories, KNetworkMounts::SymlinkDirectory);

    const QStringList savedSymlinkDirectories = {symlinkDirectory + QLatin1Char('/')};
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SmbPaths), savedPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkDirectory), savedSymlinkDirectories);

    // SymlinkToNetworkMount
    // addPath
    KNetworkMounts::self()->addPath(symlinkToSmbPath, KNetworkMounts::SymlinkToNetworkMount);

    const QString savedSymlinkToSmbPath = symlinkToSmbPath + QLatin1Char('/');
    const QStringList savedSymlinkToSmbPaths = {savedSymlinkToSmbPath};
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkToNetworkMount), savedSymlinkToSmbPaths);

    // setPaths
    const QStringList symlinkToSmbPaths = {symlinkToSmbPath};
    KNetworkMounts::self()->setPaths(symlinkToSmbPaths, KNetworkMounts::SymlinkToNetworkMount);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkToNetworkMount), savedSymlinkToSmbPaths);

    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SmbPaths), savedPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkDirectory), savedSymlinkDirectories);
}

void KNetworkMountsTestCanonical::cleanupTestCase()
{
    KNetworkMounts::self()->sync();
    QFile::remove(m_configFileName);
}

void KNetworkMountsTestCanonical::testCanonicalSymlinkPath_data()
{
    QTest::addColumn<QString>("relPath");
    QTest::addColumn<QString>("symlinkedRelPath");

    // SymlinkDirectory
    QTest::newRow("symlinkDirectory/dir") << "dir"
                                          << "symlinkDirectory/dir";
    QTest::newRow("symlinkDirectory/dir/file.txt") << "dir/file.txt"
                                                   << "symlinkDirectory/dir/file.txt";
    QTest::newRow("symlinkDirectory/dir/subdir1") << "dir/subdir1"
                                                  << "symlinkDirectory/dir/subdir1";
    QTest::newRow("symlinkDirectory/dir/subdir1/file.txt") << "dir/subdir1/file.txt"
                                                           << "symlinkDirectory/dir/subdir1/file.txt";
    QTest::newRow("symlinkDirectory/dir/subdir1/subdir1") << "dir/subdir1/subdir1"
                                                          << "symlinkDirectory/dir/subdir1/subdir1";
    QTest::newRow("symlinkDirectory/dir/subdir1/subdir1/file.txt") << "dir/subdir1/subdir1/file.txt"
                                                                   << "symlinkDirectory/dir/subdir1/subdir1/file.txt";
    QTest::newRow("symlinkDirectory/dir/subdir1/subdir2") << "dir/subdir1/subdir2"
                                                          << "symlinkDirectory/dir/subdir1/subdir2";
    QTest::newRow("symlinkDirectory/dir/subdir1/subdir2/file.txt") << "dir/subdir1/subdir2/file.txt"
                                                                   << "symlinkDirectory/dir/subdir1/subdir2/file.txt";
    QTest::newRow("symlinkDirectory/dir/subdir1/subdir3") << "dir/subdir1/subdir3"
                                                          << "symlinkDirectory/dir/subdir1/subdir3";
    QTest::newRow("symlinkDirectory/dir/subdir1/subdir3/file.txt") << "dir/subdir1/subdir3/file.txt"
                                                                   << "symlinkDirectory/dir/subdir1/subdir3/file.txt";
    QTest::newRow("symlinkDirectory/dir/subdir2") << "dir/subdir2"
                                                  << "symlinkDirectory/dir/subdir2";
    QTest::newRow("symlinkDirectory/dir/subdir2/file.txt") << "dir/subdir2/file.txt"
                                                           << "symlinkDirectory/dir/subdir2/file.txt";
    QTest::newRow("symlinkDirectory/dir/subdir2/subdir1") << "dir/subdir2/subdir1"
                                                          << "symlinkDirectory/dir/subdir2/subdir1";
    QTest::newRow("symlinkDirectory/dir/subdir2/subdir1/file.txt") << "dir/subdir2/subdir1/file.txt"
                                                                   << "symlinkDirectory/dir/subdir2/subdir1/file.txt";
    QTest::newRow("symlinkDirectory/dir/subdir2/subdir2") << "dir/subdir2/subdir2"
                                                          << "symlinkDirectory/dir/subdir2/subdir2";
    QTest::newRow("symlinkDirectory/dir/subdir2/subdir2/file.txt") << "dir/subdir2/subdir2/file.txt"
                                                                   << "symlinkDirectory/dir/subdir2/subdir2/file.txt";
    QTest::newRow("symlinkDirectory/dir/subdir2/subdir3") << "dir/subdir2/subdir3"
                                                          << "symlinkDirectory/dir/subdir2/subdir3";
    QTest::newRow("symlinkDirectory/dir/subdir2/subdir3/file.txt") << "dir/subdir2/subdir3/file.txt"
                                                                   << "symlinkDirectory/dir/subdir2/subdir3/file.txt";
    QTest::newRow("symlinkDirectory") << "symlinkDirectory"
                                      << "symlinkDirectory";
    QTest::newRow("symlinkDirectory/file.txt") << "symlinkDirectory/file.txt"
                                               << "symlinkDirectory/file.txt";

    // SymlinkToNetworkMount
    QTest::newRow("symlinkToSmbPath") << "dir"
                                      << "symlinkToSmbPath";
    QTest::newRow("symlinkToSmbPath/file.txt") << "dir/file.txt"
                                               << "symlinkToSmbPath/file.txt";
    QTest::newRow("symlinkToSmbPath/subdir1") << "dir/subdir1"
                                              << "symlinkToSmbPath/subdir1";
    QTest::newRow("symlinkToSmbPath/subdir1/file.txt") << "dir/subdir1/file.txt"
                                                       << "symlinkToSmbPath/subdir1/file.txt";
    QTest::newRow("symlinkToSmbPath/subdir1/subdir1") << "dir/subdir1/subdir1"
                                                      << "symlinkToSmbPath/subdir1/subdir1";
    QTest::newRow("symlinkToSmbPath/subdir1/subdir1/file.txt") << "dir/subdir1/subdir1/file.txt"
                                                               << "symlinkToSmbPath/subdir1/subdir1/file.txt";
    QTest::newRow("symlinkToSmbPath/subdir1/subdir2") << "dir/subdir1/subdir2"
                                                      << "symlinkToSmbPath/subdir1/subdir2";
    QTest::newRow("symlinkToSmbPath/subdir1/subdir2/file.txt") << "dir/subdir1/subdir2/file.txt"
                                                               << "symlinkToSmbPath/subdir1/subdir2/file.txt";
    QTest::newRow("symlinkToSmbPath/subdir1/subdir3") << "dir/subdir1/subdir3"
                                                      << "symlinkToSmbPath/subdir1/subdir3";
    QTest::newRow("symlinkToSmbPath/subdir1/subdir3/file.txt") << "dir/subdir1/subdir3/file.txt"
                                                               << "symlinkToSmbPath/subdir1/subdir3/file.txt";
    QTest::newRow("symlinkToSmbPath/subdir2") << "dir/subdir2"
                                              << "symlinkToSmbPath/subdir2";
    QTest::newRow("symlinkToSmbPath/subdir2/file.txt") << "dir/subdir2/file.txt"
                                                       << "symlinkToSmbPath/subdir2/file.txt";
    QTest::newRow("symlinkToSmbPath/subdir2/subdir1") << "dir/subdir2/subdir1"
                                                      << "symlinkToSmbPath/subdir2/subdir1";
    QTest::newRow("symlinkToSmbPath/subdir2/subdir1/file.txt") << "dir/subdir2/subdir1/file.txt"
                                                               << "symlinkToSmbPath/subdir2/subdir1/file.txt";
    QTest::newRow("symlinkToSmbPath/subdir2/subdir2") << "dir/subdir2/subdir2"
                                                      << "symlinkToSmbPath/subdir2/subdir2";
    QTest::newRow("symlinkToSmbPath/subdir2/subdir2/file.txt") << "dir/subdir2/subdir2/file.txt"
                                                               << "symlinkToSmbPath/subdir2/subdir2/file.txt";
    QTest::newRow("symlinkToSmbPath/subdir2/subdir3") << "dir/subdir2/subdir3"
                                                      << "symlinkToSmbPath/subdir2/subdir3";
    QTest::newRow("symlinkToSmbPath/subdir2/subdir3/file.txt") << "dir/subdir2/subdir3/file.txt"
                                                               << "symlinkToSmbPath/subdir2/subdir3/file.txt";
}

void KNetworkMountsTestCanonical::testCanonicalSymlinkPath()
{
    QFETCH(QString, relPath);
    QFETCH(QString, symlinkedRelPath);

#ifdef Q_OS_WIN
    QSKIP("QFile::link creates a shortcut on Windows, not a symlink, so no effect on canonical paths, skipped");
#endif

    const QString path = m_tmpDir.path() + QLatin1Char('/') + relPath;
    const QString symlinkedPath = m_tmpDir.path() + QLatin1Char('/') + symlinkedRelPath;
    const QString canonicalPath = QFileInfo(symlinkedPath).canonicalFilePath();

    // default with cache
    QCOMPARE(KNetworkMounts::self()->canonicalSymlinkPath(symlinkedPath), canonicalPath);
    QCOMPARE(path, canonicalPath);
    qDebug() << "path=" << path << ", canonicalPath=" << canonicalPath << ", symlinkedPath=" << symlinkedPath;

    // from cache
    QCOMPARE(KNetworkMounts::self()->canonicalSymlinkPath(symlinkedPath), canonicalPath);

    // no cache
    KNetworkMounts::self()->clearCache();
    QCOMPARE(KNetworkMounts::self()->canonicalSymlinkPath(symlinkedPath), canonicalPath);

    KNetworkMounts::self()->clearCache();
    KNetworkMounts::self()->setOption(KNetworkMounts::SymlinkPathsUseCache, false);
    QCOMPARE(KNetworkMounts::self()->canonicalSymlinkPath(symlinkedPath), canonicalPath);

    // with cache
    KNetworkMounts::self()->setOption(KNetworkMounts::SymlinkPathsUseCache, true);
    QCOMPARE(KNetworkMounts::self()->canonicalSymlinkPath(symlinkedPath), canonicalPath);

    QCOMPARE(KNetworkMounts::self()->canonicalSymlinkPath(symlinkedPath), canonicalPath);
}

#include "moc_knetworkmountstestcanonical.cpp"
