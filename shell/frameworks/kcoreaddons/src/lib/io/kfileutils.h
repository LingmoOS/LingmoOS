/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KFILEUTILS_H
#define KFILEUTILS_H

#include "kcoreaddons_export.h"

#include <QString>
#include <QUrl>

/**
 * @short A namespace for KFileUtils globals
 *
 */
namespace KFileUtils
{
/**
 * Given a directory path and a string representing a file or directory
 * (which usually exist already), this function returns a suggested name
 * for a file/directory that doesn't exist in @p baseURL.
 *
 * The suggested file name is of the form "foo (1)", "foo (2)" etc.
 *
 * For local URLs, this function will check if there is already a file/directory
 * with the new suggested name and will keep incrementing the number in the above
 * format until it finds one that doesn't exist. Note that this function uses a
 * blocking I/O call (using QFileInfo) to check the existence of the file/directory,
 * this could be problematic for network mounts (e.g. SMB, NFS) as these are treated
 * as local files by the upstream QFile code. An alternative is to use makeSuggestedName()
 * and use KIO to stat the new file/directory in an asynchronous way.
 *
 * @since 5.61
 */
KCOREADDONS_EXPORT QString suggestName(const QUrl &baseURL, const QString &oldName);

/**
 * Given a string, "foo", representing a file/directory (which usually exists already),
 * this function returns a suggested name for a file/directory in the form "foo (1)",
 * "foo (2)" etc.
 *
 * Unlike the suggestName() method, this function doesn't check if there is a file/directory
 * with the newly suggested name; the idea being that this responsibility falls on
 * the caller, e.g. one can use KIO::stat() to check asynchronously whether the new
 * name already exists (in its parent directory) or not.
 *
 * @since 5.76
 */
KCOREADDONS_EXPORT QString makeSuggestedName(const QString &oldName);

/**
 * Locates all files matching the @p nameFilters in the given @p dirs
 * The returned list does not contain duplicate file names.
 * In case there are multiple files the one which comes first in the dirs list is returned.
 * For example:
 * @code
    QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("krunner/dbusplugins"), QStandardPaths::LocateDirectory);
    KFileUtils::findAllUniqueFiles(dirs, QStringList{QStringLiteral("*.desktop")});
 * @endcode
 * @param location standard location for the dir
 * @param dir directory in which the files are located
 * @param nameFilters filters that get passed to the QDirIterator that is used internally to
 * iterate over the files in each dir in the list
 * @return list of absolute file paths
 * @since 5.85
 */
KCOREADDONS_EXPORT QStringList findAllUniqueFiles(const QStringList &dirs, const QStringList &nameFilters = {});
}
#endif
