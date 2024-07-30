/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kfileutilstest.h"

#include <KFileUtils>
#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(KFileUtilsTest)

void KFileUtilsTest::testSuggestName_data()
{
    QTest::addColumn<QString>("oldName");
    QTest::addColumn<QStringList>("existingFiles");
    QTest::addColumn<QString>("expectedOutput");

    QTest::newRow("non-existing") << "foobar" << QStringList() << "foobar (1)";
    QTest::newRow("existing") << "foobar" << QStringList(QStringLiteral("foobar")) << "foobar (1)";
    QTest::newRow("existing_1") << "foobar" << (QStringList() << QStringLiteral("foobar") << QStringLiteral("foobar (1)")) << "foobar (2)";
    QTest::newRow("extension") << "foobar.txt" << QStringList() << "foobar (1).txt";
    QTest::newRow("extension_exists") << "foobar.txt" << (QStringList() << QStringLiteral("foobar.txt")) << "foobar (1).txt";
    QTest::newRow("extension_exists_1") << "foobar.txt" << (QStringList() << QStringLiteral("foobar.txt") << QStringLiteral("foobar (1).txt"))
                                        << "foobar (2).txt";
    QTest::newRow("two_extensions") << "foobar.tar.gz" << QStringList() << "foobar (1).tar.gz";
    QTest::newRow("two_extensions_exists") << "foobar.tar.gz" << (QStringList() << QStringLiteral("foobar.tar.gz")) << "foobar (1).tar.gz";
    QTest::newRow("two_extensions_exists_1") << "foobar.tar.gz" << (QStringList() << QStringLiteral("foobar.tar.gz") << QStringLiteral("foobar (1).tar.gz"))
                                             << "foobar (2).tar.gz";
    QTest::newRow("with_space") << "foo bar" << QStringList(QStringLiteral("foo bar")) << "foo bar (1)";
    QTest::newRow("dot_at_beginning") << ".aFile.tar.gz" << QStringList() << ".aFile (1).tar.gz";
    QTest::newRow("dots_at_beginning") << "..aFile.tar.gz" << QStringList() << "..aFile (1).tar.gz";
    QTest::newRow("empty_basename") << ".txt" << QStringList() << ". (1).txt";
    QTest::newRow("empty_basename_2dots") << "..txt" << QStringList() << ". (1).txt";
    QTest::newRow("basename_with_dots") << "filename.5.3.2.tar.gz" << QStringList() << "filename.5.3.2 (1).tar.gz";
    QTest::newRow("unknown_extension_trashinfo") << "fileFromHome.trashinfo" << QStringList() << "fileFromHome (1).trashinfo";
}

void KFileUtilsTest::testSuggestName()
{
    QFETCH(QString, oldName);
    QFETCH(QStringList, existingFiles);
    QFETCH(QString, expectedOutput);

    QTemporaryDir dir;
    const QUrl baseUrl = QUrl::fromLocalFile(dir.path());
    for (const QString &localFile : std::as_const(existingFiles)) {
        QFile file(dir.path() + QLatin1Char('/') + localFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
    }
    QCOMPARE(KFileUtils::suggestName(baseUrl, oldName), expectedOutput);
}

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC) && !defined(Q_OS_ANDROID)
#define XDG_PLATFORM
#endif

void KFileUtilsTest::testfindAllUniqueFiles()
{
#ifndef XDG_PLATFORM
    QSKIP("This test requires XDG_DATA_DIRS; no way to configure QStandardPaths on Windows");
#endif
    const QString testBaseDirPath = QDir::currentPath() + QLatin1String("/kfileutilstestdata/");
    QDir testDataBaseDir(testBaseDirPath);
    testDataBaseDir.mkpath(QStringLiteral("."));
    testDataBaseDir.mkpath(QStringLiteral("testdir1/testDirName"));
    testDataBaseDir.mkpath(QStringLiteral("testdir2/testDirName"));
    testDataBaseDir.mkpath(QStringLiteral("testdir3/testDirName"));

    QFile file1(testBaseDirPath + QLatin1String("/testdir1/testDirName/testfile.test"));
    file1.open(QFile::WriteOnly);
    QFile file2(testBaseDirPath + QLatin1String("/testdir2/testDirName/testfile.test"));
    file2.open(QFile::WriteOnly);
    QFile file3(testBaseDirPath + QLatin1String("/testdir3/testDirName/differentfile.test"));
    file3.open(QFile::WriteOnly);
    QFile file4(testBaseDirPath + QLatin1String("/testdir3/testDirName/nomatch.txt"));
    file4.open(QFile::WriteOnly);

    qputenv("XDG_DATA_DIRS", qPrintable(QStringLiteral("%1testdir1:%1testdir2:%1testdir3").arg(testBaseDirPath)));

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("testDirName"), QStandardPaths::LocateDirectory);
    const QStringList expected = {testDataBaseDir.filePath(QStringLiteral("testdir1/testDirName/testfile.test")),
                                  testDataBaseDir.filePath(QStringLiteral("testdir3/testDirName/differentfile.test"))};

    const QStringList actual = KFileUtils::findAllUniqueFiles(dirs, QStringList{QStringLiteral("*.test")});
    QCOMPARE(actual, expected);
}

#include "moc_kfileutilstest.cpp"
