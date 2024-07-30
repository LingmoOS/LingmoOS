/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fileindexerconfigutils.h"
#include "filtereddiriterator.h"
#include "fileindexerconfig.h"

#include <QTemporaryDir>
#include <QTest>

namespace {
    const QStringList dataSet() {
        static QStringList dataSet = {
            QStringLiteral("home/"),
            QStringLiteral("home/1"),
            QStringLiteral("home/2"),
            QStringLiteral("home/kde/"),
            QStringLiteral("home/kde/1"),
            QStringLiteral("home/docs/"),
            QStringLiteral("home/docs/1"),
            QStringLiteral("home/docs/.fire"),
            QStringLiteral("home/.hiddenDir/"),
            QStringLiteral("home/.hiddenFile"),
            QStringLiteral("home/.includedHidden/"),
            QStringLiteral("home/.includedHidden/dir/"),
            QStringLiteral("home/.includedHidden/file"),
            QStringLiteral("home/.includedHidden/.hidden"),
        };
        return dataSet;
    }
}

class FilteredDirIteratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFiles();
    void testIndexHidden();
    void testFolders();
    void testAddingExcludedFolder();
    void testNoConfig();
};

using namespace Baloo;

void FilteredDirIteratorTest::testFiles()
{
    std::unique_ptr<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dataSet()));

    QStringList includeFolders = {
        dir->path() + QLatin1String("/home"),
        dir->path() + QLatin1String("/home/.includedHidden"),
    };

    QStringList excludeFolders = {
        dir->path() + QLatin1String("/home/kde")
    };

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    FileIndexerConfig config;
    FilteredDirIterator it(&config, includeFolders.first());

    QSet<QString> expected = {
        QStringLiteral("/home"),
        QStringLiteral("/home/docs"),
        QStringLiteral("/home/docs/1"),
        QStringLiteral("/home/1"),
        QStringLiteral("/home/2"),
        QStringLiteral("/home/.includedHidden"),
        QStringLiteral("/home/.includedHidden/dir"),
        QStringLiteral("/home/.includedHidden/file"),
    };

    QSet<QString> list;
    while (!it.next().isEmpty()) {
        QString path = it.filePath();
        // Strip temporary dir prefix
        path = path.mid(dir->path().length());

        list << path;
        QVERIFY(config.shouldFolderBeIndexed(it.filePath()));
        QVERIFY(expected.contains(path));
    }
    QCOMPARE(list, expected);
}

void FilteredDirIteratorTest::testIndexHidden()
{
    std::unique_ptr<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dataSet()));

    QStringList includeFolders = {
        dir->path() + QLatin1String("/home"),
        dir->path() + QLatin1String("/home/.includedHidden"),
    };

    QStringList excludeFolders = {
        dir->path() + QLatin1String("/home/kde")
    };

    Test::writeIndexerConfig(includeFolders, excludeFolders, {}, true);

    FileIndexerConfig config;
    FilteredDirIterator it(&config, includeFolders.first());

    QSet<QString> expected = {
        QStringLiteral("/home"),
        QStringLiteral("/home/docs"),
        QStringLiteral("/home/docs/1"),
        QStringLiteral("/home/1"),
        QStringLiteral("/home/2"),
        QStringLiteral("/home/docs/.fire"),
        QStringLiteral("/home/.hiddenDir"),
        QStringLiteral("/home/.hiddenFile"),
        QStringLiteral("/home/.includedHidden"),
        QStringLiteral("/home/.includedHidden/dir"),
        QStringLiteral("/home/.includedHidden/file"),
        QStringLiteral("/home/.includedHidden/.hidden"),
    };

    QSet<QString> list;
    while (!it.next().isEmpty()) {
        QString path = it.filePath();
        // Strip temporary dir prefix
        path = path.mid(dir->path().length());

        list << path;
        QVERIFY(config.shouldFolderBeIndexed(it.filePath()));
        QVERIFY(expected.contains(path));
    }
    QCOMPARE(list, expected);
}

void FilteredDirIteratorTest::testFolders()
{
    std::unique_ptr<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dataSet()));

    QStringList includeFolders = {
        dir->path() + QLatin1String("/home"),
    };

    QStringList excludeFolders = {
        dir->path() + QLatin1String("/home/kde")
    };

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    FileIndexerConfig config;
    FilteredDirIterator it(&config, includeFolders.first(), FilteredDirIterator::DirsOnly);

    QSet<QString> expected = {
        QStringLiteral("/home"),
        QStringLiteral("/home/docs"),
    };

    QSet<QString> list;
    while (!it.next().isEmpty()) {
        QString path = it.filePath();
        path = path.mid(dir->path().length());

        list << path;
        QVERIFY(config.shouldFolderBeIndexed(it.filePath()));
        QVERIFY(expected.contains(path));
    }
    QCOMPARE(list, expected);
}

void FilteredDirIteratorTest::testAddingExcludedFolder()
{
    std::unique_ptr<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dataSet()));

    QStringList includeFolders = {
        dir->path() + QLatin1String("/home"),
    };

    QStringList excludeFolders = {
        dir->path() + QLatin1String("/home/kde")
    };

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    FileIndexerConfig config;
    FilteredDirIterator it(&config, excludeFolders.first());

    QVERIFY(it.next().isEmpty());
}

void FilteredDirIteratorTest::testNoConfig()
{
    std::unique_ptr<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dataSet()));

    FilteredDirIterator it(nullptr, dir->path() + QLatin1String("/home"));

    QSet<QString> expected = {
        QStringLiteral("/home"),
        QStringLiteral("/home/1"),
        QStringLiteral("/home/2"),
        QStringLiteral("/home/kde"),
        QStringLiteral("/home/kde/1"),
        QStringLiteral("/home/docs"),
        QStringLiteral("/home/docs/1"),
    };

    QSet<QString> list;
    while (!it.next().isEmpty()) {
        QString path = it.filePath();
        path = path.mid(dir->path().length());

        list << path;
        QVERIFY(expected.contains(path));
    }
    QCOMPARE(list, expected);
}

QTEST_GUILESS_MAIN(FilteredDirIteratorTest)

#include "filtereddiriteratortest.moc"
