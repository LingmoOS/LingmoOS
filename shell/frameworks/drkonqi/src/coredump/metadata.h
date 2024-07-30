/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2019-2021 Harald Sitter <sitter@kde.org>
*/

#pragma once

#include <QDebug> // Don't use categorized logging here to make the header easy to use by the helper daemon
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>

namespace Metadata
{
static QString drkonqiMetadataPath(const QString &exe, const QString &bootId, const QString &timestamp, int pid)
{
    const QString command = QFileInfo(exe).fileName();

    const QString name = QStringLiteral("drkonqi/crashes/%1.%2.%3.%4.ini").arg(command, bootId, QString::number(pid), timestamp);
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1Char('/') + name;
}

// Defined in the header so it can be used by the coredump-helper as well.
static QString resolveKCrashMetadataPath(const QString &exe, const QString &bootId, int pid)
{
    const QString command = QFileInfo(exe).fileName();

    const QString nameV2 = QStringLiteral("kcrash-metadata/%1.%2.%3.ini").arg(command, bootId, QString::number(pid));
    const QString pathV2 = QStandardPaths::locate(QStandardPaths::GenericCacheLocation, nameV2);

    // Backwards compat for a while. Can be dropped before/after Lingmo 6.0
    const QString nameV1 = QStringLiteral("kcrash-metadata/%1.ini").arg(pid);
    const QString pathV1 = QStandardPaths::locate(QStandardPaths::GenericCacheLocation, nameV1);

    for (const auto &path : {pathV2, pathV1}) {
        if (!path.isEmpty() && QFile::exists(path)) {
            return path;
        }
    }

    qWarning() << "Unable to find file for pid" << pid << "expected at" << nameV2;
    return {};
}
} // namespace Metadata
