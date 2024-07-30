/*
    This file is part of KDE Frameworks
    SPDX-FileCopyrightText: 2018 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SETUP_XDG_DIRS_H
#define SETUP_XDG_DIRS_H

#include <QCoreApplication>
#include <QFile>
#include <QStandardPaths>

static void setupXdgDirs()
{
    // We need to keep the system XDG_DATA_DIRS to find mimetypes
    const QByteArray defaultDataDirs = qEnvironmentVariableIsSet("XDG_DATA_DIRS") ? qgetenv("XDG_DATA_DIRS") : QByteArray("/usr/local/share:/usr/share");
    const QByteArray modifiedDataDirs = QFile::encodeName(QCoreApplication::applicationDirPath()) + "/data:" + defaultDataDirs;
    qputenv("XDG_DATA_DIRS", modifiedDataDirs);

    // We don't need the system config dirs, we provide our own applications.menu
    const QByteArray modifiedConfigDirs = QFile::encodeName(QCoreApplication::applicationDirPath()) + "/data";
    qputenv("XDG_CONFIG_DIRS", modifiedConfigDirs);
}

#endif
