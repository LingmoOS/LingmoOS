/*
    This file is part of the Nepomuk KDE project.
    SPDX-FileCopyrightText: 2011 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef FILEINDEXERCONFIGUTILS_H
#define FILEINDEXERCONFIGUTILS_H

#include <KConfig>
#include <KConfigGroup>

#include <memory>
#include <QDir>
#include <QTextStream>
#include <QTemporaryDir>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Baloo
{
namespace Test
{
void writeIndexerConfig(const QStringList& includeFolders,
                        const QStringList& excludeFolders,
                        const QStringList& excludeFilters = QStringList(),
                        bool indexHidden = false)
{
    QStandardPaths::setTestModeEnabled(true);
    KConfig fileIndexerConfig(QStringLiteral("baloofilerc"));
    fileIndexerConfig.group(QStringLiteral("General")).writePathEntry("folders", includeFolders);
    fileIndexerConfig.group(QStringLiteral("General")).writePathEntry("exclude folders", excludeFolders);
    fileIndexerConfig.group(QStringLiteral("General")).writeEntry("exclude filters", excludeFilters);
    fileIndexerConfig.group(QStringLiteral("General")).writeEntry("index hidden folders", indexHidden);
    fileIndexerConfig.sync();
}

std::unique_ptr<QTemporaryDir> createTmpFolders(const QStringList& folders)
{
    auto tmpDir = std::make_unique<QTemporaryDir>();
    // If the temporary directory is in a hidden folder, then the tests will fail,
    // so we use /tmp/ instead.
    // TODO: Find a better solution
    if (QFileInfo(tmpDir->path()).isHidden()) {
        tmpDir = std::make_unique<QTemporaryDir>(QStringLiteral("/tmp/"));
    }
    for (const QString & f : folders) {
        QDir dir(tmpDir->path());
        const auto lst = f.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        for (const QString & sf : lst) {
            if (!dir.exists(sf)) {
                dir.mkdir(sf);
            }
#ifdef Q_OS_WIN
            if(sf.startsWith(QLatin1Char('.'))) {
                if(!SetFileAttributesW(reinterpret_cast<const WCHAR*>((dir.path() + "/" + sf).utf16()), FILE_ATTRIBUTE_HIDDEN)) {
                    qWarning("failed to set 'hidden' attribute!");
                }
            }
#endif
            dir.cd(sf);
        }
    }
    return tmpDir;
}

std::unique_ptr<QTemporaryDir> createTmpFilesAndFolders(const QStringList& list)
{
    auto tmpDir = std::make_unique<QTemporaryDir>();
    // If the temporary directory is in a hidden folder, then the tests will fail,
    // so we use /tmp/ instead.
    // TODO: Find a better solution
    if (QFileInfo(tmpDir->path()).isHidden()) {
        tmpDir = std::make_unique<QTemporaryDir>(QStringLiteral("/tmp/"));
    }
    for (const QString& f : list) {
        if (f.endsWith(QLatin1Char('/'))) {
            QDir dir(tmpDir->path());
            const auto lst = f.split(QLatin1Char('/'), Qt::SkipEmptyParts);
            for (const QString & sf : lst) {
                if (!dir.exists(sf)) {
                    dir.mkdir(sf);
                }
#ifdef Q_OS_WIN
                if(sf.startsWith(QLatin1Char('.'))) {
                    if(!SetFileAttributesW(reinterpret_cast<const WCHAR*>((dir.path() + "/" + sf).utf16()), FILE_ATTRIBUTE_HIDDEN)) {
                        qWarning("failed to set 'hidden' attribute!");
                    }
                }
#endif
                dir.cd(sf);
            }
        }
        else {
            QFile file(tmpDir->path() + QLatin1Char('/') + f);
            file.open(QIODevice::WriteOnly);

            QTextStream stream(&file);
            stream << "test";
        }
    }
    return tmpDir;
}
//
// Trying to put all cases into one folder tree:
// |- indexedRootDir
//   |- indexedSubDir
//     |- indexedSubSubDir
//     |- excludedSubSubDir
//     |- .hiddenSubSubDir
//       |- ignoredSubFolderToIndexedHidden
//       |- indexedSubFolderToIndexedHidden
//   |- excludedSubDir
//     |- indexedSubDirToExcluded
//     |- .indexedHiddenSubDirToExcluded
//   |- .hiddenSubDir
//   |- .indexedHiddenSubDir
// |- ignoredRootDir
// |- excludedRootDir
//
const QString indexedRootDir = QStringLiteral("d1");
const QString indexedSubDir = QStringLiteral("d1/sd1");
const QString indexedSubSubDir = QStringLiteral("d1/sd1/ssd1");
const QString excludedSubSubDir = QStringLiteral("d1/sd1/ssd2");
const QString hiddenSubSubDir = QStringLiteral("d1/sd1/.ssd3");
const QString ignoredSubFolderToIndexedHidden = QStringLiteral("d1/sd1/.ssd3/isfh1");
const QString indexedSubFolderToIndexedHidden = QStringLiteral("d1/sd1/.ssd3/isfh2");
const QString excludedSubDir = QStringLiteral("d1/sd2");
const QString indexedSubDirToExcluded = QStringLiteral("d1/sd2/isde1");
const QString indexedHiddenSubDirToExcluded = QStringLiteral("d1/sd2/.isde2");
const QString hiddenSubDir = QStringLiteral("d1/.sd3");
const QString indexedHiddenSubDir = QStringLiteral("d1/.sd4");
const QString ignoredRootDir = QStringLiteral("d2");
const QString excludedRootDir = QStringLiteral("d3");
}
}

#endif // FILEINDEXERCONFIGUTILS_H
