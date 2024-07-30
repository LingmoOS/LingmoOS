/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Jacob R Rideout <kde@jacobrideout.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kautosavefiletest.h"

#include <QFile>
#include <QTextStream>
#include <QtAlgorithms>

#include <QTemporaryFile>
#include <kautosavefile.h>

#include <QTest>

QTEST_MAIN(KAutoSaveFileTest)

void KAutoSaveFileTest::initTestCase()
{
    QCoreApplication::instance()->setApplicationName(QLatin1String("qttest")); // TODO do this in qtestlib itself
}

void KAutoSaveFileTest::cleanupTestCase()
{
    for (const QString &fileToRemove : std::as_const(filesToRemove)) {
        QFile::remove(fileToRemove);
    }
}

void KAutoSaveFileTest::test_readWrite()
{
    QTemporaryFile file;

    QVERIFY(file.open());

    QUrl normalFile = QUrl::fromLocalFile(QFileInfo(file).absoluteFilePath());

    // Test basic functionality
    KAutoSaveFile saveFile(normalFile);

    QVERIFY(!QFile::exists(saveFile.fileName()));
    QVERIFY(saveFile.open(QIODevice::ReadWrite));

    QString inText = QString::fromLatin1("This is test data one.\n");

    {
        QTextStream ts(&saveFile);
        ts << inText;
        ts.flush();
    }

    saveFile.close();

    {
        QFile testReader(saveFile.fileName());
        testReader.open(QIODevice::ReadWrite);
        QTextStream ts(&testReader);

        QString outText = ts.readAll();

        QCOMPARE(outText, inText);
    }

    filesToRemove << file.fileName();
}

void KAutoSaveFileTest::test_fileNameMaxLength()
{
    // In KAutoSaveFilePrivate::tempFile() the name of the kautosavefile that's going to be created
    // is concatanated in the form:
    // fileName + junk.truncated + protocol + _ + path.truncated + junk
    // see tempFile() for details.
    //
    // Make sure that the generated filename (e.g. as you would get from QUrl::fileName()) doesn't
    // exceed NAME_MAX (the maximum length allowed for filenames, see e.g. /usr/include/linux/limits.h)
    // otherwise the file can't be opened.
    //
    // see https://phabricator.kde.org/D24489

    QString s;
    s.fill(QLatin1Char('b'), 80);
    // create a long path that:
    // - exceeds NAME_MAX (255)
    // - is less than the maximum allowed path length, PATH_MAX (4096)
    //   see e.g. /usr/include/linux/limits.h
    const QString path = QDir::tempPath() + QLatin1Char('/') + s + QLatin1Char('/') + s + QLatin1Char('/') + s + QLatin1Char('/') + s;

    QFile file(path + QLatin1Char('/') + QLatin1String("testFile.txt"));

    QUrl normalFile = QUrl::fromLocalFile(file.fileName());

    KAutoSaveFile saveFile(normalFile);

    QVERIFY(!QFile::exists(saveFile.fileName()));
    QVERIFY(saveFile.open(QIODevice::ReadWrite));

    filesToRemove << file.fileName();
}

void KAutoSaveFileTest::test_fileStaleFiles()
{
    QUrl normalFile = QUrl::fromLocalFile(QDir::temp().absoluteFilePath(QStringLiteral("test directory/tîst me.txt")));

    KAutoSaveFile saveFile(normalFile);
    QVERIFY(saveFile.open(QIODevice::ReadWrite));
    saveFile.write("testdata");

    // Make sure the stale file is found

    const auto listOfStaleFiles = saveFile.staleFiles(normalFile, QStringLiteral("qttest"));
    QVERIFY(listOfStaleFiles.count() == 1);
    saveFile.releaseLock();
    qDeleteAll(listOfStaleFiles);

    // Make sure the stale file is deleted

    QVERIFY(saveFile.staleFiles(normalFile, QStringLiteral("qttest")).isEmpty());
}

void KAutoSaveFileTest::test_applicationStaleFiles()
{
    // TODO
}

void KAutoSaveFileTest::test_locking()
{
    QUrl normalFile(QString::fromLatin1("fish://user@example.com/home/remote/test.txt"));

    KAutoSaveFile saveFile(normalFile);

    QVERIFY(!QFile::exists(saveFile.fileName()));
    QVERIFY(saveFile.open(QIODevice::ReadWrite));

    const QList<KAutoSaveFile *> staleFiles(KAutoSaveFile::staleFiles(normalFile));

    QVERIFY(!staleFiles.isEmpty());

    KAutoSaveFile *saveFile2 = staleFiles.at(0);

    const QString fn = saveFile2->fileName();
    // It looks like $XDG_DATA_HOME/stalefiles/qttest/test.txtXXXfish_%2Fhome%2FremoteXXXXXXX
    QVERIFY2(fn.contains(QLatin1String("stalefiles/qttest/test.txt")), qPrintable(fn));
    QVERIFY2(fn.contains(QLatin1String("fish_%2Fhome%2Fremote")), qPrintable(fn));

    QVERIFY(QFile::exists(saveFile2->fileName()));
    QVERIFY(!saveFile2->open(QIODevice::ReadWrite));

    saveFile.releaseLock();

    QVERIFY(saveFile2->open(QIODevice::ReadWrite));

    qDeleteAll(staleFiles);
}

#include "moc_kautosavefiletest.cpp"
